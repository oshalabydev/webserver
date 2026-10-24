#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#include <app/error.h>
#include <app/channel.h>
#include <app/event_loop.h>
#include <app/listen_sock.h>

typedef struct app_t
{
	ListenSock *listen_sock;
	EventLoop *event_loop;
} App;

// Global error object
Error global_err;

// Application lifecycle
static Errable int app_run(App *app);
static void app_destroy(const App *app);

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
	app->listen_sock = listen_sock_new();

	app->event_loop = event_loop_new();
	if (app->event_loop == NULL)
		THROW_ERROR("Failed to create event loop");

	if (event_loop_subscribe(app->event_loop, CHANNEL(app->listen_sock)) < 0)
		return -EXIT_FAILURE;

	if (listen_sock_listen(app->listen_sock) < 0)
		return -EXIT_FAILURE;

	if (event_loop_run(app->event_loop) < 0)
		return -EXIT_FAILURE;

	return EXIT_SUCCESS;
}

void
app_destroy(const App *app)
{
	app->listen_sock->base.close(CHANNEL(app->listen_sock));
	event_loop_free(app->event_loop);
}
