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
 */

/*
 *     ., .,
 *     {O,O}
 *     (   )
 *      " "
 */

#include "x-sys.h"

/**
 * @name Library Functions
 * @{
 */

/**
 * Compute the absolute value of an integer.
 *
 * @param i The integer value.
 * @return The absolute value of @p i.
 */
int x_lib_abs(int i);

/**
 * Convert an integer to a string representation.
 *
 * Writes the ASCII representation of @p num into the buffer @p p_str
 * using the given numeric @p base (2--36). The caller must ensure the
 * buffer is large enough for the result.
 *
 * @param num   The integer to convert.
 * @param p_str Destination buffer for the resulting string.
 * @param base  Numeric base for the conversion (2--36).
 * @return Pointer to @p p_str, or NULL on invalid arguments.
 */
x_char_t *x_lib_inttostr(x_int_t num, x_char_t *p_str, unsigned short base);

/**
 * Copy bytes from a source buffer to a destination buffer.
 *
 * @param p_dest Pointer to the destination buffer.
 * @param p_src  Pointer to the source buffer.
 * @param n      Number of bytes to copy.
 * @return Pointer to @p p_dest.
 */
void *x_lib_memcpy(void *p_dest, const void *p_src, size_t n);

/**
 * Duplicate a memory region.
 *
 * Allocates a new buffer via x_sys_malloc() and copies @p size bytes
 * from @p p_src into it.
 *
 * @param p_src Pointer to the source memory.
 * @param size  Number of bytes to duplicate.
 * @return Pointer to the new buffer, or NULL on allocation failure.
 */
void *x_lib_memdup(const void *p_src, size_t size);

/**
 * Fill a memory region with a constant byte.
 *
 * @param p_dest Pointer to the destination buffer.
 * @param byte   The byte value to fill with.
 * @param size   Number of bytes to set.
 * @return Pointer to @p p_dest.
 */
void *x_lib_memset(void *p_dest, int byte, size_t size);

/**
 * Locate the first occurrence of a character in a string.
 *
 * @param p_str The string to search.
 * @param c     The character to find (as an int).
 * @return Pointer to the first occurrence, or NULL if not found.
 */
x_char_t *x_lib_strchr(const x_char_t *p_str, int c);

/**
 * Compare two null-terminated strings.
 *
 * @param p_str1 First string.
 * @param p_str2 Second string.
 * @return Negative, zero, or positive value indicating ordering.
 */
int x_lib_strcmp(const x_char_t *p_str1, const x_char_t *p_str2);

/**
 * Calculate the length of a null-terminated string.
 *
 * @param p_str The string to measure.
 * @return The length in bytes, excluding the null terminator.
 */
size_t x_lib_strlen(const x_char_t *p_str);

/**
 * Compare two strings up to a maximum number of characters.
 *
 * @param p_str1 First string.
 * @param p_str2 Second string.
 * @param n      Maximum number of characters to compare.
 * @return Negative, zero, or positive value indicating ordering.
 */
int x_lib_strncmp(const x_char_t *p_str1, const x_char_t *p_str2, size_t n);

/**
 * Duplicate a string up to a given size.
 *
 * Allocates a new buffer, copies up to @p size bytes from @p p_str,
 * and null-terminates the result.
 *
 * @param p_str The source string.
 * @param size  Maximum number of characters to copy.
 * @return Pointer to the new string, or NULL on allocation failure.
 */
x_char_t *x_lib_strndup(const x_char_t *p_str, size_t size);

/**
 * Convert a string to an integer.
 *
 * Parses @p p_str as an integer in the given @p base (0 for auto-detect).
 * If @p pp_end is non-NULL, it is set to point past the last parsed character.
 *
 * @param p_str  The string to parse.
 * @param pp_end Output pointer to the first unparsed character, or NULL.
 * @param base   Numeric base (0 for auto-detect, or 2--36).
 * @return The parsed integer value.
 */
x_int_t x_lib_strtoint(const x_char_t *p_str, x_char_t **pp_end, unsigned short base);

/** @} */

#endif /* X_LIB_H */
