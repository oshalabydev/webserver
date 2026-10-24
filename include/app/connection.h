#ifndef WEBSERVER_CONNECTION_H
#define WEBSERVER_CONNECTION_H

#include <stdbool.h>
#include <netinet/in.h>

#include "error.h"
#include "channel.h"

#define CONN_INPUT_BUF_SZ 8192

#define CONN(x) ((Connection *) (x))

typedef struct connection_t
{
	Channel base;
	struct sockaddr_in addr;
	char input_buf[CONN_INPUT_BUF_SZ];
	size_t n_input_buf;
	bool client_eof; // When true, connection should close after response is sent.
} Connection;

Errable Connection *
connection_new_from_socket(int sock_fd, struct sockaddr_in addr);

#endif // WEBSERVER_CONNECTION_H
