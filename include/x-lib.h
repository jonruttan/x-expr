#ifndef X_LIB_H
#define X_LIB_H

/**
 * @file x-lib.h
 * @brief Portable library utility functions.
 *
 * Self-contained implementations of common string, memory, and math
 * operations. When X_USE_STDLIB is defined, these delegate to the
 * standard C library instead.
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
 * @name Library Functions
 * @{
 */

/** Compute the absolute value of an integer. */
int x_lib_abs(int i);

/** Convert an integer to a string representation. */
x_char_t *x_lib_inttostr(x_int_t num, x_char_t *p_str, unsigned short base);

/** Copy bytes from a source buffer to a destination buffer. */
void *x_lib_memcpy(void *p_dest, const void *p_src, size_t n);

/** Duplicate a memory region. */
void *x_lib_memdup(const void *p_src, size_t size);

/** Fill a memory region with a constant byte. */
void *x_lib_memset(void *p_dest, int byte, size_t size);

/** Locate the first occurrence of a character in a string. */
x_char_t *x_lib_strchr(const x_char_t *p_str, int c);

/** Compare two null-terminated strings. */
int x_lib_strcmp(const x_char_t *p_str1, const x_char_t *p_str2);

/** Calculate the length of a null-terminated string. */
size_t x_lib_strlen(const x_char_t *p_str);

/** Compare two strings up to a maximum number of characters. */
int x_lib_strncmp(const x_char_t *p_str1, const x_char_t *p_str2, size_t n);

/** Duplicate a string up to a given size. */
x_char_t *x_lib_strndup(const x_char_t *p_str, size_t size);

/** Convert a string to an integer. */
x_int_t x_lib_strtoint(const x_char_t *p_str, x_char_t **pp_end, unsigned short base);

/** @} */

#endif /* X_LIB_H */
