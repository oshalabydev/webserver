#ifndef WEBSERVER_EVENTLOOP_H
#define WEBSERVER_EVENTLOOP_H

#include "error.h"

typedef struct event_loop_t
{
	int refs;
	int epoll_fd;
} EventLoop;

Errable EventLoop *
event_loop_new(void);

EventLoop *
event_loop_ref(EventLoop *event_loop);

void
event_loop_unref(EventLoop *event_loop);

Errable int
event_loop_subscribe(EventLoop *event_loop, Source *src, int events);

Errable int
event_loop_run(EventLoop *event_loop);

#endif // WEBSERVER_EVENTLOOP_H
