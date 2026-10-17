#ifndef WEBSERVER_ERROR_H
#define WEBSERVER_ERROR_H

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#define ERROR_CTX_SIZE 512	// Maximum size of an error message

/*
 Functions labelled Errable may return the type's error sentinel:
   pointer -> NULL
   integer -> -1

 Errors must be propagated unless explicitly handled.
*/
#define Errable

// Writes to global error object: errno and error message (context),
// then returns -1.
#define THROW_ERROR(ctx) \
	do { \
		global_err.e_errno = errno; \
		strncpy(global_err.e_ctx, ctx, ERROR_CTX_SIZE); \
		return -1; \
	} while (0)

// Returns NULL instead of -1. Used for functions where the return type
// is a pointer to an object.
#define THROW_ERROR_NULL(ctx) \
	do { \
		global_err.e_errno = errno; \
		strncpy(global_err.e_ctx, ctx, ERROR_CTX_SIZE); \
		return NULL; \
	} while (0)

// Used instead of THROW_ERROR if you only want to print to stderr,
// but no returning or error propagation. Useful in app_destroy.
#define THROW_WARNING(ctx) \
	do { \
		fprintf(stderr, "[WARN] %s: %s\n", ctx, strerror(errno)); \
	} while (0)

typedef struct error_t
{
	int e_errno;
	char e_ctx[ERROR_CTX_SIZE];
} Error;

// Global error object
extern Error global_err;

#endif // WEBSERVER_ERROR_H
