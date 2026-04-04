/**
 * @file x.c
 * @brief Core implementation -- error reporting and debug output.
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

#include "x-lib.h"

/**
 * Output an error message to a file descriptor, then exit.
 *
 * Writes `"*** ERROR: "` followed by @p message. If @p symbol is
 * non-NULL, appends `" '"` and the symbol string. Then terminates
 * the process with X_SYS_EXIT_FAILURE.
 */
void x_error(int fd, x_char_t *message, x_char_t *symbol)
{
	x_sys_write(fd, X_STR_LITERAL("*** ERROR: "));
	x_sys_write(fd, message, x_lib_strlen(message));

	if (symbol) {
		x_sys_write(fd, X_STR_LITERAL(" '"));
		x_sys_write(fd, symbol, x_lib_strlen(symbol));
	}

	x_sys_exit(X_SYS_EXIT_FAILURE);
}


#ifdef DEBUG

/**
 * @internal
 * Write a formatted debug message with va_list.
 *
 * Prepends source location and wraps in `*** DEBUG(...): ... ***` markers.
 *
 * @param file Source file name (__FILE__).
 * @param line Source line number (__LINE__).
 * @param fd   File descriptor to write to.
 * @param fmt  printf-style format string.
 * @param ap   Variable argument list.
 */
void _x_debug_va(char *file, long unsigned line, int fd, char *fmt, va_list ap)
{
	x_char_t buffer[X_DEBUG_BUFFER_SIZE], *s;
	int n;

	s = (x_char_t *)"\n*** DEBUG(%s:%lu): ";
	sprintf((char *)buffer, s, file, line);
	x_sys_write(fd, buffer, x_lib_strlen(buffer));

	n = vsprintf((char *)buffer, fmt, ap);

	x_sys_write(fd, buffer, n);

	s = (x_char_t *)" ***\n";
	x_sys_write(fd, s, x_lib_strlen(s));
}

/**
 * @internal
 * Write a formatted debug message with variadic arguments.
 *
 * @param file Source file name (__FILE__).
 * @param line Source line number (__LINE__).
 * @param fd   File descriptor to write to.
 * @param fmt  printf-style format string.
 * @param ...  Format arguments.
 */
void _x_debug(char *file, long unsigned line, int fd, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	_x_debug_va(file, line, fd, fmt, ap);
	va_end(ap);
}

#else /* DEBUG */

#endif /* DEBUG */
