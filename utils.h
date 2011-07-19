#ifndef UTILS_H
#define UTILS_H 1
#include <stddef.h>

// Debugging
#ifdef DB
#define DEBUG(...) fprintf(stderr, __VA_ARGS__); fflush(stderr);
#else
#define DEBUG(...)
#endif

/**
 * Get memory space and check, wheter I have it
 */
void *
xmalloc(size_t size);

/**
 * reallocate space and check it
 */
void *
xrealloc(void * pointer, size_t size);

/**
 * safe version of strcat
 */
void
cat(char **to, char * from);
#endif
