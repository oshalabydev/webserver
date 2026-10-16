#include "listen_socket.h"

#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "error.h"

// TODO listen_socket_listen()

ListenSocket *
listen_socket_new(const struct sockaddr_in *addr)
{
	ListenSocket *listen_sock = malloc(sizeof(ListenSocket));
	listen_sock->refs = 1;

	listen_sock->fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (listen_sock < 0)
		THROW_ERROR_NULL("Failed to create listening socket");

#ifdef DEBUG
	// Makes the server restart quickly
	if (setsockopt(listen_sock->fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		THROW_ERROR_NULL("Failed to configure socket");
#endif

	if (bind(listen_sock->fd, (struct sockaddr *) addr, sizeof(struct sockaddr_in)) < 0)
		THROW_ERROR_NULL("Failed to bind listening socket to Internet address");

	return listen_sock;
}

ListenSocket *
listen_socket_ref(ListenSocket *listen_sock)
{
	listen_sock->refs++;
	return listen_sock;
}

void
listen_socket_unref(ListenSocket *listen_sock)
{
	if (--listen_sock->refs == 0)
	{
		if (close(listen_sock->fd) < 0)
			THROW_WARNING("Failed to close listening socket");

		free(listen_sock);
	}
}
