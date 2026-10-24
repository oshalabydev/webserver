#include "app/connection.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "app/event_loop.h"

extern void
conn_handle_event(Channel *self, uint32_t events);

extern void
conn_close(Channel *self);

Errable Connection *
connection_new_from_socket(int sock_fd, struct sockaddr_in addr)
{
	Connection *conn = malloc(sizeof(Connection));

	conn->base.type = CHANNEL_TYPE_CONNECTION;
	conn->base.fd = sock_fd;
	conn->base.events = EPOLLIN;
	conn->base.subscribed_to = NULL;
	conn->addr = addr;
	conn->n_input_buf = 0;
	conn->client_eof = false;

	conn->base.handle_event = conn_handle_event;
	conn->base.close = conn_close;

	// Logging
	char addr_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &addr.sin_addr, addr_str, INET_ADDRSTRLEN);
	printf("[INFO] Client %s:%d connected.\n", addr_str, ntohs(addr.sin_port));

	return conn;
}