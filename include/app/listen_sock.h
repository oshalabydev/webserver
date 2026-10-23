#ifndef WEBSERVER_LISTEN_SOCK_H
#define WEBSERVER_LISTEN_SOCK_H

#include <netinet/in.h>

#include "error.h"
#include "channel.h"

typedef struct listen_sock_t
{
	Channel base;
	struct sockaddr_in addr;
} ListenSock;

Errable ListenSock *
listen_sock_new();

Errable int
listen_sock_listen(const ListenSock *listen_sock);

#endif // WEBSERVER_LISTEN_SOCK_H
