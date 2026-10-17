#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include <app/error.h>
#include <app/channel.h>
#include <app/event_loop.h>

#define DEFAULT_ADDR 0 // Listen on all devices
#define DEFAULT_PORT 8080

#define ACCEPT_BACKLOG 16 // Maximum number of pending connections

typedef struct app_t
{
	Channel *listen_sock;
	EventLoop *event_loop;
} App;

// Global error object
Error global_err;

// Application lifecycle
static Errable int app_run(App *app);
static void app_destroy(const App *app);

static void listen_sock_handle_event(Channel *self, uint32_t events);
static void listen_sock_close(Channel *self);

int
main(void)
{
	App app = {0};
	int ret = 0;

	if (app_run(&app) < 0)
	{
		ret = EXIT_FAILURE;
		fprintf(stderr, "[ERR] %s: %s\n", global_err.e_ctx, strerror(global_err.e_errno));
	};

	app_destroy(&app);

	return ret;
}

Errable int
app_run(App *app)
{
	const struct sockaddr_in listen_sock_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(DEFAULT_PORT),
		.sin_addr = { htonl(DEFAULT_ADDR) },
	};

	const int listen_sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (listen_sock_fd < 0)
		THROW_ERROR("Failed to create listening socket");

	// TODO use `Channel` constructor instead
	app->listen_sock = malloc(sizeof(Channel));
	app->listen_sock->type = CHANNEL_TYPE_LISTEN_SOCK;
	app->listen_sock->fd = listen_sock_fd;
	app->listen_sock->events = EPOLLIN;
	app->listen_sock->handle_event = listen_sock_handle_event;
	app->listen_sock->close = listen_sock_close;

#ifdef DEBUG
	// Enable addresses to be reused rapidly, making the server start faster.
	if (setsockopt(listen_sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		THROW_ERROR("Failed to initialize listening socket");
#endif

	if (bind(listen_sock_fd, (struct sockaddr *) &listen_sock_addr, sizeof(listen_sock_addr)) < 0)
		THROW_ERROR("Failed to initialize listening socket");

	if (listen(listen_sock_fd, ACCEPT_BACKLOG) < 0)
		THROW_ERROR("Failed to initialize listening socket");

	app->event_loop = event_loop_new();
	if (app->event_loop == NULL)
		THROW_ERROR("Failed to create event loop");

	event_loop_subscribe(app->event_loop, app->listen_sock);
	event_loop_run(app->event_loop);

	return EXIT_SUCCESS;
}

void listen_sock_handle_event(Channel *self, const uint32_t events)
{
	// TODO include the other hangup event mask
	if (events & EPOLLHUP)
	{
		printf("[INFO] Listen socket hang up.\n");
		self->close(self);
	}

	if (events & EPOLLIN)
	{
		printf("[INFO] Client wants to connect.\n");
	}
}

void listen_sock_close(Channel *self)
{
	event_loop_unsubscribe(self->subscribed_to, self);
	close(self->fd);
	free(self);
}

void
app_destroy(const App *app)
{
	app->listen_sock->close(app->listen_sock);
	event_loop_free(app->event_loop);
}
