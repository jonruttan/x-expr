/**
 * @file x-sys.c
 * @brief Implementation of system-level function wrappers.
 *
 * @author Jon Ruttan (jonruttan@gmail.com)
 * @copyright 2021 Jon Ruttan
 * @license MIT No Attribution (MIT-0)
 */

/*
 *     ., .,
 *     {O,O}
 *     (   )
 *      " "
 */

#include <stdio.h>			/* vsprintf() */
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef X_CLOCK
#include <time.h>			/* clock() */
#endif /* X_CLOCK */

#include "x-sys.h"
#include "x-lib.h"

#ifndef X_SYS_FUNC
/**
 * @internal
 * Indirection macro for system function calls.
 *
 * Allows all system calls to be redirected at compile time by
 * redefining this macro.
 *
 * @param F The system function name.
 */
#define X_SYS_FUNC(F)		F
#endif /* X_SYS_FUNC */


/** Cause normal process termination. */
void x_sys_exit(int status)
{
	X_SYS_FUNC(exit)(status);
}

/** Allocate a memory block from the heap. */
void *x_sys_malloc(size_t size)
{
	return X_SYS_FUNC(malloc)(size);
}

/** Free a memory block back to the heap. */
void x_sys_free(void *ptr)
{
	X_SYS_FUNC(free)(ptr);
}

/** Read from a file descriptor. */
ssize_t x_sys_read(int fd, void *p_buf, size_t size)
{
	return X_SYS_FUNC(read)(fd, p_buf, size);
}

/** Write to a file descriptor. */
ssize_t x_sys_write(int fd, const void *p_buf, size_t size)
{
	return X_SYS_FUNC(write)(fd, p_buf, size);
}

/** Open a file. */
int x_sys_open(const char *path, int flags)
{
	return X_SYS_FUNC(open)(path, flags);
}

/** Close a file descriptor. */
int x_sys_close(int fd)
{
	return X_SYS_FUNC(close)(fd);
}

/** Read a single character from a file descriptor. */
/* NOTE: Compositional, move to library? */
x_char_t x_sys_read_char(int fd)
{
	x_char_t c;

	if (x_sys_read(fd, &c, sizeof(c)) <= 0) {
		return X_SYS_EOF;
	}

	return c;
}

#ifdef X_CLOCK
/**
 * Read the CPU clock in microseconds.
 *
 * Converts the value from clock() into microseconds.
 */
x_int_t x_sys_clock(void)
{
	return (x_int_t)(X_SYS_FUNC(clock)() * 1000000 / CLOCKS_PER_SEC);
}
#endif /* X_CLOCK */
