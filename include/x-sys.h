#ifndef X_SYS_H
#define X_SYS_H

/**
 * @file x-sys.h
 * @brief System-level function wrappers.
 *
 * Thin wrappers around OS/libc primitives for memory allocation, file I/O,
 * and process control. Each wrapper can be redirected at compile time via
 * the X_SYS_FUNC macro.
 *
 * @author Jon Ruttan (jonruttan@gmail.com)
 * @copyright 2021 Jon Ruttan
 * @license MIT No Attribution (MIT-0)
 *
 *         ., .,
 *         {O,O}
 *         (   )
 *          " "
 */

#include "x.h"

#include <stdio.h>	/* For *EOF* */
#include <unistd.h> /* For *STD*_FILENO* */

/** End-of-file sentinel value. */
#ifndef X_SYS_EOF
#define X_SYS_EOF EOF
#endif /* X_SYS_EOF */

/** Successful process exit status. */
#ifndef X_SYS_EXIT_SUCCESS
#define X_SYS_EXIT_SUCCESS 0
#endif /* X_SYS_EXIT_SUCCESS */

/** Failure process exit status. */
#ifndef X_SYS_EXIT_FAILURE
#define X_SYS_EXIT_FAILURE 1
#endif /* X_SYS_EXIT_FAILURE */

/**
 * @name System Functions
 * @{
 */

/** Allocate a memory block from the heap. */
void *x_sys_malloc(size_t size);

/** Free a previously allocated memory block. */
void x_sys_free(void *ptr);

/** Read bytes from a file descriptor. */
ssize_t x_sys_read(int fd, void *p_buf, size_t size);

/** Write bytes to a file descriptor. */
ssize_t x_sys_write(int fd, const void *p_buf, size_t size);

/** Terminate the process. */
void x_sys_exit(int status);

/** Open a file by path. */
int x_sys_open(const char *path, int flags);

/** Close a file descriptor. */
int x_sys_close(int fd);

/** Read a single character from a file descriptor, or X_SYS_EOF. */
int x_sys_read_char(int fd);

#ifdef X_CLOCK
/** Read the CPU clock in microseconds. */
x_int_t x_sys_clock(void);
#endif /* X_CLOCK */

/** @} */

#endif /* X_SYS_H */
