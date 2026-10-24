#ifndef WEBSERVER_CHANNEL_H
#define WEBSERVER_CHANNEL_H

#include <stdint.h>

#define CHANNEL(x) ((Channel *) (x))

struct event_loop_t;

enum ChannelType
{
	CHANNEL_TYPE_CONNECTION,
	CHANNEL_TYPE_LISTEN_SOCK,
};

typedef struct channel_t
{
	enum ChannelType type;
	struct event_loop_t *subscribed_to;
	int fd;
	uint32_t events;
	void (*handle_event)(struct channel_t *self, uint32_t events);
	void (*close)(struct channel_t *self);
} Channel;

#endif // WEBSERVER_CHANNEL_H
