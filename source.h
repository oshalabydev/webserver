#ifndef WEBSERVER_SOURCE_H
#define WEBSERVER_SOURCE_H

enum SourceType
{
	SOURCE_TYPE_LISTEN_SOCK,
	SOURCE_TYPE_CLIENT_SOCK,
};

// TODO implement "socket", "fastcgi", etc. types
// TODO make "source" have a union field that can hold a "socket", "fastcgi", etc. types

typedef struct source_t
{
	int refs;
	int fd;
	enum SourceType type;
	// Used to associate a source with another parent source, e.g. a FastCGI connection
	// initiated in response to a request. Optional, can be NULL.
	struct source_t *parent;
} Source;

Source *
source_new(int fd, enum SourceType type);

Source *
source_ref(Source *src);

void
source_unref(Source *src);

#endif // WEBSERVER_SOURCE_H
