#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "error.h"
#include "event_loop.h"
#include "listen_socket.h"

#define DEFAULT_ADDR 0 // Listen on all devices
#define DEFAULT_PORT 8080

#define ACCEPT_BACKLOG 16 // Maximum number of pending connections
#define MAX_EPOLL_EVENTS 64 // Maximum number of epoll events

// For application lifecycle. Simplifies cleanup. Invisible to other files.
typedef struct app_t
{
	EventLoop *event_loop;
	ListenSocket *listen_sock;
} App;

// Global error object
Error global_err;

static int app_init(App *app);
static int app_run(App *app);
static void app_destroy(const App *app);

int
main(void)
{
	App app = {0};
	int ret = 0;

	if (app_init(&app) < 0)
	{
		ret = 1;
		fprintf(stderr, "[ERR] %s: %s\n", global_err.e_ctx, strerror(global_err.e_errno));
		goto cleanup;
	};

	if (app_run(&app) < 0)
	{
		ret = 1;
		fprintf(stderr, "[ERR] %s: %s\n", global_err.e_ctx, strerror(global_err.e_errno));
	}

	cleanup:
		app_destroy(&app);

	return ret;
}

int
app_init(App *app)
{
	const struct sockaddr_in listen_sock_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(DEFAULT_PORT),
		.sin_addr = { htonl(DEFAULT_ADDR) },
	};

	app->event_loop = event_loop_new();
	app->listen_sock = listen_socket_new(&listen_sock_addr);

	return 0;
}

int
app_run(App *app)
{
	if (event_loop_run(app->event_loop) < 0)
		return -1;

	return 0;
}

void
app_destroy(const App *app)
{
	if (app->listen_sock)
	{
		listen_socket_unref(app->listen_sock);
	}

	if (app->event_loop)
	{
		event_loop_unref(app->event_loop);
	}
}
