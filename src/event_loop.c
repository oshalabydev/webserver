#include <app/event_loop.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>

#include <app/error.h>
#include <app/channel.h>

Errable EventLoop *
event_loop_new()
{
	EventLoop *evl = malloc(sizeof(EventLoop));

	evl->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	if (evl->epoll_fd < 0)
		THROW_ERROR_NULL("Failed to create epoll instance");

	evl->sz_chlist = EVENT_LOOP_CHLIST_INITIAL_SIZE;
	evl->chlist = malloc(evl->sz_chlist * sizeof(Channel *));
	evl->n_chan = 0;

	return evl;
}

Errable int
event_loop_subscribe(EventLoop *event_loop, Channel *channel)
{
	struct epoll_event ev = {
		.events = channel->events,
		.data.ptr = channel,
	};

	if (epoll_ctl(event_loop->epoll_fd, EPOLL_CTL_ADD, channel->fd, &ev) < 0)
		THROW_ERROR("Failed to subscribe channel to event loop");

	// Double the size of `chlist` if data can't fit.
	if (event_loop->n_chan == event_loop->sz_chlist)
	{
		event_loop->sz_chlist *= 2;
		event_loop->chlist = realloc(event_loop->chlist, event_loop->sz_chlist * sizeof(Channel *));
	}

	event_loop->chlist[event_loop->n_chan++] = channel;
	channel->subscribed_to = event_loop;

	return EXIT_SUCCESS;
}

Errable int
event_loop_update_subscriber(const EventLoop *event_loop, Channel *channel)
{
	struct epoll_event ev = {
		.events = channel->events,
		.data.ptr = channel,
	};

	if (epoll_ctl(event_loop->epoll_fd, EPOLL_CTL_MOD, channel->fd, &ev) < 0)
		THROW_ERROR("Failed to update event loop subscriber");

	return EXIT_SUCCESS;
}

Errable int
event_loop_unsubscribe(EventLoop *event_loop, Channel *channel)
{
	event_loop->n_chan--;
	channel->subscribed_to = NULL;

	if (epoll_ctl(event_loop->epoll_fd, EPOLL_CTL_DEL, channel->fd, NULL) < 0)
		THROW_ERROR("Failed to unsubscribe channel from event loop");

	return EXIT_SUCCESS;
}

Errable int
event_loop_run(const EventLoop *event_loop)
{
	struct epoll_event events[EVENT_LOOP_MAX_EPOLL_EVENTS];

	for (;;)
	{
		const int nfds = epoll_wait(event_loop->epoll_fd, events, EVENT_LOOP_MAX_EPOLL_EVENTS, -1);
		if (nfds < 0)
			THROW_ERROR("epoll_wait");

		for (int i = 0; i < nfds; i++)
		{
			Channel *chan = events[i].data.ptr;

			chan->handle_event(chan, events[i].events);
		}
	}

	return EXIT_SUCCESS;
}

void
event_loop_free(EventLoop *evl)
{
	free(evl->chlist);
	close(evl->epoll_fd);
	free(evl);
}
