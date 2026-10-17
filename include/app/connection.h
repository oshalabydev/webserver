#ifndef WEBSERVER_CONNECTION_H
#define WEBSERVER_CONNECTION_H

#include "channel.h"

#define CONNECTION_INPUT_BUFFER_SIZE 4096

typedef struct connection_t
{
	Channel base;
	char input_buf[CONNECTION_INPUT_BUFFER_SIZE];
} Connection;

#endif // WEBSERVER_CONNECTION_H
