#ifndef WEBSERVER_CONNECTION_H
#define WEBSERVER_CONNECTION_H

#include <netinet/in.h>

#include "channel.h"

#define CONNECTION_INPUT_BUFFER_SIZE 4096

typedef struct connection_t
{
	Channel base;
	struct sockaddr_in addr;
	char input_buf[CONNECTION_INPUT_BUFFER_SIZE];
} Connection;

#endif // WEBSERVER_CONNECTION_H
