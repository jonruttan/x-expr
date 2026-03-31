/*
 * # Computational Expressions in C
 *
 * ## x-sys.c -- Implementation - System Functions
 *
 * @description Computational Expressions in C
 * @author [Jon Ruttan](jonruttan@gmail.com)
 * @copyright 2021 Jon Ruttan
 * @license MIT No Attribution (MIT-0)
 *
 *     ., .,
 *     {O,O}
 *     (   )
 *      " "
 */
/*
 * # Includes
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
#define X_SYS_FUNC(F)		F
#endif /* X_SYS_FUNC */


/*
 * # System Functions
 */

/*
 * Cause normal process termination.
 *
 * @param {int} status An integer returned to the parent.
 * @returns The exit function does not return.
 */
void x_sys_exit(int status)
{
	X_SYS_FUNC(exit)(status);
}

/*
 * Allocate a memory vector from the heap.
 *
 * @function x_sys_malloc
 * @param {size_t} size The size in bytes of the memory vector to allocate.
 * @returns {void *} A pointer to the allocated memory.
 */
void *x_sys_malloc(size_t size)
{
	return X_SYS_FUNC(malloc)(size);
}

/*
 * Free a memory vector to the heap.
 *
 * @function x_sys_free
 * @param {void *} ptr A pointer to the memory vector to free.
 * @returns {void}
 */
void x_sys_free(void *ptr)
{
	X_SYS_FUNC(free)(ptr);
}

/*
 * Read from a file descriptor.
 *
 * @function x_sys_read
 * @param {int} fd A file descriptor.
 * @param {void *} p_buf A pointer to the memory to read to.
 * @param {size_t} size The size of the buffer to read to.
 * @returns {ssize_t} The number of bytes read (may be less than size.)
 */
ssize_t x_sys_read(int fd, void *p_buf, size_t size)
{
	return X_SYS_FUNC(read)(fd, p_buf, size);
}

/*
 * Write to a file descriptor.
 *
 * @function x_sys_write
 * @param {int} fd A file descriptor.
 * @param {void *} p_buf A pointer to the memory to write from.
 * @param {size_t} size The size of the buffer to write from.
 * @returns {ssize_t} The number of bytes written (may be less than size.)
 */
ssize_t x_sys_write(int fd, const void *p_buf, size_t size)
{
	return X_SYS_FUNC(write)(fd, p_buf, size);
}

/*
 * Open a file.
 *
 * @function x_sys_open
 * @param {const char *} path The file path to open.
 * @param {int} flags The open flags (e.g. O_RDONLY).
 * @returns {int} A file descriptor, or -1 on error.
 */
int x_sys_open(const char *path, int flags)
{
	return X_SYS_FUNC(open)(path, flags);
}

/*
 * Close a file descriptor.
 *
 * @function x_sys_close
 * @param {int} fd The file descriptor to close.
 * @returns {int} 0 on success, -1 on error.
 */
int x_sys_close(int fd)
{
	return X_SYS_FUNC(close)(fd);
}

/*
 * Read a character from a file.
 *
 * @function x_sys_read_char
 * @param (int) fd The file descriptor to read from.
 * @returns {x_char_t} on success, X_SYS_EOF on error.
 */
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
x_int_t x_sys_clock(void)
{
	return (x_int_t)(X_SYS_FUNC(clock)() * 1000000 / CLOCKS_PER_SEC);
}
#endif /* X_CLOCK */
