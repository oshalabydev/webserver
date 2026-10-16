#include <stdlib.h>

#include "source.h"

Source *
source_new(int fd, enum SourceType type)
{
	Source *src = malloc(sizeof(Source));
	src->refs = 1;
	src->type = type;
	src->fd = fd;

	return src;
}

Source *
source_ref(Source *src)
{
	src->refs++;
	return src;
}

void
source_unref(Source *src)
{
	if (--src->refs == 0)
	{
		free(src);
	}
}
