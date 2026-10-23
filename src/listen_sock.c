#define _GNU_SOURCE

#include <app/listen_sock.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include <app/event_loop.h>

#define DEFAULT_ADDR 0 // Listen on all devices
#define DEFAULT_PORT 8080

#define ACCEPT_BACKLOG 16 // Maximum number of pending connections

static Errable int
listen_sock_handle_event(Channel *self, const uint32_t events);

static void
listen_sock_close(Channel *self);

Errable ListenSock *
listen_sock_new()
{
	const struct sockaddr_in listen_sock_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(DEFAULT_PORT),
		.sin_addr = { htonl(DEFAULT_ADDR) },
	};

	const int listen_sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (listen_sock_fd < 0)
		THROW_ERROR_NULL("Failed to create listening socket");

#ifdef DEBUG
	// Enable addresses to be reused rapidly, making the server start faster.
	if (setsockopt(listen_sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		THROW_ERROR_NULL("Failed to initialize listening socket");
#endif

	if (bind(listen_sock_fd, (struct sockaddr *) &listen_sock_addr, sizeof(listen_sock_addr)) < 0)
		THROW_ERROR_NULL("Failed to initialize listening socket");

	ListenSock *listen_sock = malloc(sizeof(ListenSock));
	listen_sock->base.type = CHANNEL_TYPE_LISTEN_SOCK;
	listen_sock->base.fd = listen_sock_fd;
	listen_sock->base.events = EPOLLIN;
	listen_sock->base.handle_event = listen_sock_handle_event;
	listen_sock->base.close = listen_sock_close;

	return listen_sock;
}

Errable int
listen_sock_listen(const ListenSock *listen_sock)
{
	if (listen(listen_sock->base.fd, ACCEPT_BACKLOG) < 0)
		THROW_ERROR("Failed to initialize listening socket");

	return EXIT_SUCCESS;
}

static Errable int
listen_sock_handle_event(Channel *self, const uint32_t events)
{
	if (events & EPOLLHUP)
	{
		printf("[INFO] Listen socket hang up.\n");
		self->close(self);
	}

	if (events & EPOLLIN)
	{
		printf("[INFO] Client wants to connect.\n");

		struct sockaddr_in client_addr;
		socklen_t client_addr_sz = sizeof(struct sockaddr_in);
		int client_fd;

		if ((client_fd = accept4(self->fd, (struct sockaddr *) &client_addr, &client_addr_sz,
			SOCK_NONBLOCK | SOCK_CLOEXEC)) < 0)
			THROW_ERROR("Failed to accept client (accept4)");

		char addr_str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, INET_ADDRSTRLEN);
		printf("[INFO] Client %s:%d connected.\n", addr_str, ntohs(client_addr.sin_port));
	}

	return EXIT_SUCCESS;
}

static void
listen_sock_close(Channel *self)
{
	event_loop_unsubscribe(self->subscribed_to, self);
	close(self->fd);
	free(self);
}
