// ReSharper disable CppUseInternalLinkage
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "app/channel.h"
#include "app/connection.h"
#include "app/event_loop.h"

void
conn_handle_event(Channel *self, uint32_t events)
{
	if (events & EPOLLIN)
	{
		assert(CONN_INPUT_BUF_SZ - CONN(self)->n_input_buf >= 0);

		read:
			const size_t n_bytes = recv(self->fd, CONN(self)->input_buf + CONN(self)->n_input_buf,
				CONN_INPUT_BUF_SZ - CONN(self)->n_input_buf, 0);

		if (n_bytes < 0)
		{
			if (errno == ECONNRESET || errno == ETIMEDOUT || errno == ENOTCONN)
			{
				printf("[INFO] Client connection reset.\n");
				self->close(self);
				return;
			}
			else if (errno == EINTR)
				goto read; // Reperform the read
			else if (errno == EAGAIN || errno == EWOULDBLOCK)
				return;
			else
			{
				THROW_WARNING("Fatal error (recv)");
				self->close(self);
				return;
			}
		}

		// EOF, client disconnected
		if (n_bytes == 0)
		{
			/*	NOTE When writing the response, if you get EPIPE (suppress SIGPIPE
				with MSG_NOSIGPIPE or similar), then know that the client has completely
				disconnected both it's writing and reading ends. In this case, just close
				the connection immediately.

				Also, always check with the parser, if the request is incomplete, close
				immediately. */
			printf("[INFO] Client disconnected.\n");
			CONN(self)->client_eof = true;

			if (event_loop_unsubscribe(self->subscribed_to, self) < 0)
				THROW_WARNING("Failed to unsubscribe connection");
		}

		CONN(self)->n_input_buf += n_bytes;

		/* consuming pattern */
		// const size_t n_bytes_written = write(STDOUT_FILENO, CONN(self)->input_buf, CONN(self)->n_input_buf);
		// memmove(CONN(self)->input_buf, CONN(self)->input_buf + n_bytes_written,
		// 	CONN(self)->n_input_buf - n_bytes_written);
		// CONN(self)->n_input_buf -= n_bytes_written;
	}

	if (events & EPOLLHUP)
	{
		printf("[INFO] Client hang up.\n");
		self->close(self);
	}
}

void
conn_close(Channel *self)
{
	if (self->subscribed_to)
	{
		if (event_loop_unsubscribe(self->subscribed_to, self) < 0)
			THROW_WARNING("Failed to unsubscribe connection");
	}

	close(self->fd);
	free(self);
}
