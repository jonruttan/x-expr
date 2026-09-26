/**
 * @file x-sys.c
 * @brief Implementation of system-level function wrappers.
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

/* sigaction is POSIX; under -ansi glibc's <features.h> hides it unless a
 * feature-test macro asks.  This file is the one place libc is spoken to,
 * so it asks -- before the first header, which is the only place a
 * feature-test macro works.  (No effect on macOS or musl, whose headers do
 * not gate on it.) */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <stdio.h>			/* vsprintf() */
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef X_SYS_CLOCK
#include <time.h>			/* clock() */
#endif /* X_SYS_CLOCK */
#ifdef X_SYS_SIGNAL
#include <signal.h>			/* sigaction(), signal() */
#endif /* X_SYS_SIGNAL */

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


/**
 * Terminate the process.
 *
 * @param status Exit status code returned to the parent process.
 */
void x_sys_exit(int status)
{
	X_SYS_FUNC(exit)(status);
}

/**
 * Allocate a memory block from the heap.
 *
 * @param size The number of bytes to allocate.
 * @return A pointer to the allocated memory, or NULL on failure.
 */
void *x_sys_malloc(size_t size)
{
	return X_SYS_FUNC(malloc)(size);
}

/**
 * Free a previously allocated memory block.
 *
 * @param ptr Pointer to the memory block to free.
 */
void x_sys_free(void *ptr)
{
	X_SYS_FUNC(free)(ptr);
}

/**
 * Read from a file descriptor.
 *
 * @param fd    File descriptor to read from.
 * @param p_buf Pointer to the destination buffer.
 * @param size  Maximum number of bytes to read.
 * @return The number of bytes read, or -1 on error.
 */
ssize_t x_sys_read(int fd, void *p_buf, size_t size)
{
	/* No EINTR retry here: an interrupted read is the ctrl-c cancel
	 * protocol (SIGINT handler sets a flag, the aborted read surfaces
	 * it -- see the REPL loop).  The embedding layer separates that
	 * from real read errors (x-lang#170). */
	return X_SYS_FUNC(read)(fd, p_buf, size);
}

/**
 * Write to a file descriptor.
 *
 * @param fd    File descriptor to write to.
 * @param p_buf Pointer to the source buffer.
 * @param size  Number of bytes to write.
 * @return The number of bytes written, or -1 on error.
 */
ssize_t x_sys_write(int fd, const void *p_buf, size_t size)
{
	return X_SYS_FUNC(write)(fd, p_buf, size);
}

/**
 * Open a file.
 *
 * @param path The file path to open.
 * @param flags Open flags (e.g. O_RDONLY).
 * @return A file descriptor on success, or -1 on error.
 */
int x_sys_open(const char *path, int flags)
{
	return X_SYS_FUNC(open)(path, flags);
}

/**
 * Close a file descriptor.
 *
 * @param fd The file descriptor to close.
 * @return 0 on success, -1 on error.
 */
int x_sys_close(int fd)
{
	return X_SYS_FUNC(close)(fd);
}

/**
 * Read a single character from a file descriptor.
 *
 * @param fd File descriptor to read from.
 * @return The character read, or X_SYS_EOF on error or end-of-file.
 */
/* NOTE: Compositional, move to library? */
int x_sys_read_char(int fd)
{
	x_char_t c;

	if (x_sys_read(fd, &c, sizeof(c)) <= 0) {
		return X_SYS_EOF;
	}

	return c;
}

#ifdef X_SYS_CLOCK
/**
 * Read the CPU clock in microseconds.
 *
 * Converts the value from clock() into microseconds.
 *
 * @return Elapsed CPU time in microseconds since process start.
 */
x_int_t x_sys_clock(void)
{
	return (x_int_t)(X_SYS_FUNC(clock)() * 1000000 / CLOCKS_PER_SEC);
}
#endif /* X_SYS_CLOCK */

#ifdef X_SYS_SIGNAL
/**
 * Install a signal handler.
 *
 * No flags, deliberately no SA_RESTART: a handler installed this way
 * interrupts a blocked read, which is how ctrl-c reaches an idle REPL.
 *
 * @param sig     The signal number, e.g. X_SYS_SIGINT.
 * @param handler The function to run when @p sig arrives.
 * @return 0 on success, -1 on error.
 */
int x_sys_signal_install(int sig, void (*handler)(int))
{
	struct sigaction sa;

	sa.sa_handler = handler;
	sa.sa_flags = 0;
	X_SYS_FUNC(sigemptyset)(&sa.sa_mask);

	return X_SYS_FUNC(sigaction)(sig, &sa, NULL);
}

/**
 * Restore a signal's default disposition.
 *
 * @param sig The signal number.
 * @return 0 on success, -1 on error.
 */
int x_sys_signal_restore(int sig)
{
	return X_SYS_FUNC(signal)(sig, SIG_DFL) == SIG_ERR ? -1 : 0;
}
#endif /* X_SYS_SIGNAL */
