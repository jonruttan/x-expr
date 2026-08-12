#ifndef X_STDLIB_H
#define X_STDLIB_H

/**
 * @file x-stdlib.h
 * @brief C standard library replacements.
 *
 * Stand-ins for standard C library functions, for builds where the
 * stdlib is not available (freestanding targets, embedders that supply
 * their own runtime). When X_USE_STDLIB is defined each function is a
 * light wrapper delegating to the real one; otherwise the
 * self-contained implementation is used.
 *
 * The C standard owns everything about these functions: the semantics,
 * and the types in the prototypes -- plain `char`/`void`/`int`/
 * `size_t`, never the x layer's typedefs. The `x_lib_` prefix is the
 * one deliberate difference: it lets the replacements coexist with a
 * real libc in the same link. General-purpose helpers with their own
 * contracts live in x-lib.h, not here.
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

#include "x-sys.h"

/**
 * @name Standard Library Replacements
 * @{
 */

/** Compute the absolute value of an integer. */
int x_lib_abs(int i);

/** Copy bytes from a source buffer to a destination buffer. */
void *x_lib_memcpy(void *p_dest, const void *p_src, size_t n);

/** Fill a memory region with a constant byte. */
void *x_lib_memset(void *p_dest, int byte, size_t size);

/** Locate the first occurrence of a character in a string. */
char *x_lib_strchr(const char *p_str, int c);

/** Compare two null-terminated strings. */
int x_lib_strcmp(const char *p_str1, const char *p_str2);

/** Calculate the length of a null-terminated string. */
size_t x_lib_strlen(const char *p_str);

/** Compare two strings up to a maximum number of characters. */
int x_lib_strncmp(const char *p_str1, const char *p_str2, size_t n);

/** Duplicate at most @p size characters of a string (POSIX.1-2008). */
char *x_lib_strndup(const char *p_str, size_t size);

/** @} */

#endif /* X_STDLIB_H */
