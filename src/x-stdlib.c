/**
 * @file x-stdlib.c
 * @brief Implementation of the C standard library replacements.
 *
 * The C standard defines the behaviour here, not us (see x-stdlib.h for
 * the charter). Comparison functions compare bytes as `unsigned char`
 * (C99 7.24.4) regardless of the platform's plain-`char` signedness,
 * and strchr converts @c c to `char` before comparing (C99 7.24.5.2) --
 * both exactly as the standard specifies, so the freestanding path and
 * an X_USE_STDLIB build agree byte-for-byte.
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

#include "x-stdlib.h"

#ifdef X_USE_STDLIB

#include <stdlib.h>
#include <string.h>

#endif /* X_USE_STDLIB */

/**
 * Compute the absolute value of an integer.
 *
 * @param i The integer value.
 * @return The absolute value of @p i.
 */
int x_lib_abs(int i)
{
#ifdef X_USE_STDLIB
	return abs(i);
#else
	return i >= 0 ? i : -i;
#endif /* X_USE_STDLIB */
}

/**
 * Copy bytes from a source buffer to a destination buffer.
 *
 * @param p_dest Pointer to the destination buffer.
 * @param p_src  Pointer to the source buffer.
 * @param size   Number of bytes to copy.
 * @return Pointer to @p p_dest.
 */
void *x_lib_memcpy(void *p_dest, const void *p_src, size_t size)
{
#ifdef X_USE_STDLIB
	return memcpy(p_dest, p_src, size);
#else
	unsigned char *pd = (unsigned char *)p_dest;
	const unsigned char *ps = (const unsigned char *)p_src;

	while (size--) {
		*pd++ = *ps++;
	}

	return p_dest;
#endif /* X_USE_STDLIB */
}

/**
 * Fill a memory region with a constant byte.
 *
 * @param p_dest Pointer to the destination buffer.
 * @param byte   The byte value to fill with.
 * @param size   Number of bytes to set.
 * @return Pointer to @p p_dest.
 */
void *x_lib_memset(void *p_dest, int byte, size_t size)
{
#ifdef X_USE_STDLIB
	return memset(p_dest, byte, size);
#else
	unsigned char *pd = (unsigned char *)p_dest;

	while (size--) {
		*pd++ = (unsigned char)byte;
	}

	return p_dest;
#endif /* X_USE_STDLIB */
}

/**
 * Locate the first occurrence of a character in a string.
 *
 * @p c is converted to `char` before comparing, and the terminating
 * NUL is part of the string (C99 7.24.5.2): searching for '\0' returns
 * the terminator.
 *
 * @param p_str The string to search.
 * @param c     The character to find (as an int).
 * @return Pointer to the first occurrence, or NULL if not found.
 */
char *x_lib_strchr(const char *p_str, int c)
{
#ifdef X_USE_STDLIB
	return strchr(p_str, c);
#else
	const char *ps = p_str;
	char c_conv = (char)c;

	for (; *ps && *ps != c_conv; ps++) ;

	return *ps == c_conv ? (char *)ps : NULL;
#endif /* X_USE_STDLIB */
}

/**
 * Compare two null-terminated strings.
 *
 * Bytes compare as `unsigned char` (C99 7.24.4), independent of the
 * platform's plain-`char` signedness.
 *
 * @param p_str1 First string.
 * @param p_str2 Second string.
 * @return Negative, zero, or positive value indicating ordering.
 */
int x_lib_strcmp(const char *p_str1, const char *p_str2)
{
#ifdef X_USE_STDLIB
	return strcmp(p_str1, p_str2);
#else
	const unsigned char *ps1 = (const unsigned char *)p_str1;
	const unsigned char *ps2 = (const unsigned char *)p_str2;

	for (;*ps1 && *ps2 && *ps2 == *ps1; ps1++, ps2++) ;

	return *ps1 - *ps2;
#endif /* X_USE_STDLIB */
}

/**
 * Calculate the length of a null-terminated string.
 *
 * @note This function does not handle wide characters.
 *
 * @param p_str The string to measure.
 * @return The length in bytes, excluding the null terminator.
 */
size_t x_lib_strlen(const char *p_str)
{
#ifdef X_USE_STDLIB
	return strlen(p_str);
#else
	size_t size;

	for (size=0; *p_str++; size++) ;

	return size;
#endif /* X_USE_STDLIB */
}

/**
 * Compare two strings up to a maximum of @p n characters.
 *
 * Bytes compare as `unsigned char` (C99 7.24.4); n == 0 compares
 * nothing and returns 0.
 *
 * @param p_str1 First string.
 * @param p_str2 Second string.
 * @param n      Maximum number of characters to compare.
 * @return Negative, zero, or positive value indicating ordering.
 */
int x_lib_strncmp(const char *p_str1, const char *p_str2, size_t n)
{
#ifdef X_USE_STDLIB
	return strncmp(p_str1, p_str2, n);
#else
	const unsigned char *ps1 = (const unsigned char *)p_str1;
	const unsigned char *ps2 = (const unsigned char *)p_str2;

	/* n == 0 compares nothing; the pre-decrement idiom below would
	 * wrap the size_t and compare unbounded. */
	if (n == 0) {
		return 0;
	}

	for (;--n && *ps1 && *ps2 && *ps2 == *ps1; ps1++, ps2++) ;

	return *ps1 - *ps2;
#endif /* X_USE_STDLIB */
}

/**
 * Duplicate at most @p size characters of a string.
 *
 * The copy stops at the first NUL or after @p size characters,
 * whichever comes first, and is always NUL-terminated; at most
 * @p size source bytes are read.
 *
 * @param p_str The source string.
 * @param size  Maximum number of characters to copy.
 * @return Pointer to the new string, or NULL on allocation failure.
 */
char *x_lib_strndup(const char *p_str, size_t size)
{
#ifdef X_USE_STDLIB
	/* strndup is POSIX.1-2008, not ISO C: under -ansi glibc/musl do
	 * not declare it (and a feature macro here would be too late --
	 * string.h is already in by way of earlier includes), so declare
	 * it directly. */
	extern char *strndup(const char *, size_t);

	return strndup(p_str, size);
#else
	size_t len;
	char *p_clone;

	for (len = 0; len < size && p_str[len]; len++) ;

	if ( ! (p_clone = (char *)x_sys_malloc(len + 1))) {
		return NULL;
	}

	x_lib_memcpy(p_clone, p_str, len);
	p_clone[len] = 0;

	return p_clone;
#endif /* X_USE_STDLIB */
}
