#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "source.h"
#include "event_loop.h"

EventLoop *
event_loop_new(void)
{
	EventLoop *event_loop = malloc(sizeof(EventLoop));
	event_loop->refs = 1;

	event_loop->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	if (event_loop->epoll_fd < 0)
		THROW_ERROR_NULL("Failed to create epoll instance");

	return event_loop;
}

EventLoop *
event_loop_ref(EventLoop *event_loop)
{
	event_loop->refs++;
	return event_loop;
}

void
event_loop_unref(EventLoop *event_loop)
{
	if (--event_loop->refs == 0)
	{
		if (close(event_loop->epoll_fd) < 0)
			THROW_WARNING("Failed to close epoll instance");

		free(event_loop);
	}
}

int
event_loop_subscribe(EventLoop *event_loop, Source *src, int events)
{
	struct epoll_event ep_ev = {
		.events = EPOLLIN,
		.data.ptr = src,
	};

	if (epoll_ctl(event_loop->epoll_fd, EPOLL_CTL_ADD,
		app->listen_sock, &listen_sock_epoll_event) < 0)
		THROW_ERROR(app, "Failed to subscribe to listening socket");

	return 0;
}

int
event_loop_run(EventLoop *event_loop)
{
	Source *listen_sock_src = source_new(event_loop->epoll_fd, SOURCE_TYPE_LISTEN_SOCK);
	struct epoll_event listen_sock_epoll_event = {
		.events = EPOLLIN,
		.data.ptr = &listen_sock_src,
	};

	if (listen(app->listen_sock, ACCEPT_BACKLOG) < 0)
		THROW_ERROR(app, "Failed to listen");

	struct epoll_event events[MAX_EPOLL_EVENTS] = {0}; // Epoll events buffer

	int client_socks[5];
	int clients = 0;

	int num_events = 0;
	while ((num_events = epoll_wait(app->epoll_fd, events, 10, -1)))
	{
		for (int i = 0; i < num_events; i++)
		{
			if (((Source *) events[i].data.ptr)->type == SOURCE_TYPE_LISTEN_SOCK)
			{
				struct sockaddr_in client_addr = {0};
				socklen_t client_addr_sz = sizeof(client_addr);
				int client_sock = accept4(app->listen_sock, (struct sockaddr *) &client_addr, &client_addr_sz, SOCK_NONBLOCK);
				client_socks[clients++] = client_sock;

				epoll_ctl(app->epoll_fd, EPOLL_CTL_ADD, client_sock, &(struct epoll_event){
					.events = EPOLLIN,
					.data.fd = client_sock,
				});

				char buf[100];
				inet_ntop(AF_INET, &client_addr.sin_addr, buf, 100);
			}
			else
			{
				int client_sock = events[i].data.fd;

				char buf[1000] = {0};
				int ret = read(client_sock, buf, 1000);
				if (ret == -1)
				{
					perror("read");
					return -1;
				}

				if (ret == 0)
				{
					printf("client hanged up\n");
					return 0;
				}

				printf("read %d bytes\n", ret);
				printf("-----------------\n");

				for (int i = 0; buf[i]; i++)
				{
					if (buf[i] == '\r')
						continue;

					putchar(buf[i]);
				}

				printf("\n-----------------\n");

				char *res =
					"HTTP/1.1 200 OK\r\n"
					"Content-Length: 12\r\n"
					"Content-Type: text/html\r\n\r\n"
					"Hello World!";
				write(client_sock, res, strlen(res));
				printf("sent!\n");

				close(client_sock);
			}
		}
	}

	return 0;
}