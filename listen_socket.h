#ifndef WEBSERVER_LISTEN_SOCKET_H
#define WEBSERVER_LISTEN_SOCKET_H
#include "error.h"

typedef struct listen_socket_t
{
	int refs;
	int fd;
} ListenSocket;

Errable ListenSocket *
listen_socket_new(const struct sockaddr_in *addr);

ListenSocket *
listen_socket_ref(ListenSocket *listen_sock);

void
listen_socket_unref(ListenSocket *listen_sock);

#endif // WEBSERVER_LISTEN_SOCKET_H
