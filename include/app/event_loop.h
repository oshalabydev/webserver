#ifndef WEBSERVER_EVENT_LOOP_H
#define WEBSERVER_EVENT_LOOP_H

#include <stddef.h>

#include <app/error.h>
#include <app/channel.h>

#define EVENT_LOOP_MAX_EPOLL_EVENTS 64 // Maximum number of epoll events
#define EVENT_LOOP_CHLIST_INITIAL_SIZE 64 // Initial size of chlist

typedef struct event_loop_t
{
	int epoll_fd;
	Channel **chlist;
	size_t sz_chlist;
	size_t n_chan;
} EventLoop;

Errable
EventLoop *event_loop_new();

Errable
int event_loop_subscribe(EventLoop *event_loop, Channel *channel);

Errable
int event_loop_update_subscriber(const EventLoop *event_loop, Channel *channel);

Errable
int event_loop_unsubscribe(EventLoop *event_loop, const Channel *channel);

Errable
int event_loop_run(const EventLoop *event_loop);

void event_loop_free(EventLoop *evl);

#endif // WEBSERVER_EVENT_LOOP_H
