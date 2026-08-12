/**
 * @file x-lib.c
 * @brief Implementation of the general-purpose library helpers.
 *
 * These carry x-expr's own contracts (x types are honest here). The
 * C standard library replacements live in x-stdlib.c.
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

#include "x-lib.h"

#ifdef X_USE_STDLIB

#include <stdlib.h>

#else /* X_USE_STDLIB */

#include <ctype.h>

#endif /* X_USE_STDLIB */
/**
 * Convert an integer to a string representation.
 *
 * Converts @p num into an ASCII string stored in @p p_str using the
 * given numeric @p base (2--36). If @p base is greater than 10, digits
 * after '9' use lowercase letters starting with 'a'. If @p num is
 * negative, a minus sign is prepended.
 *
 * @note The caller must provide sufficient storage in @p p_str.
 *       The minimum buffer size depends on the base. For base 2 (binary),
 *       the buffer needs at least `8 * sizeof(x_int_t) + 1` bytes.
 *
 * @param num   The integer to convert.
 * @param p_str Destination buffer for the resulting string.
 * @param base  Numeric base (2--36).
 * @return Pointer to @p p_str, or NULL if arguments are invalid.
 */
x_char_t *x_lib_inttostr(x_int_t num, x_char_t *p_str, unsigned short base)
{
#ifdef X_USE_STDLIB_NONSTD
	return ltoa(num, (char *)p_str, base);
#else
	x_char_t *p1 = p_str, *p2;
	short offset;

	if (p_str == NULL || base < 2 || base > 36) {
		return NULL;
	}

	if (num < 0) {
		*p1++ = '-';
		num = -num;
	}

	p2 = p1;
	do {
		offset = num % base;
		*p2++ = offset + (offset >= 10 ? 'a' - 10 : '0');
	} while ((num /= base));

	/* Add a trailing '0' and reverse the digits in the string */
	for (*p2-- = 0; p2 > p1; ++p1, --p2)
	{
		/* Swap the value at p1 with that at p2 */
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}

	return p_str;
#endif /* X_USE_STDLIB_NONSTD */
}

/**
 * Duplicate a memory region.
 *
 * Allocates a new buffer via x_sys_malloc() and copies @p size bytes
 * from @p p_src into it. If @p p_src is NULL, the buffer is allocated
 * but not copied (and optionally zeroed if X_OPT_MEMZERO is defined).
 *
 * @param p_src Pointer to the source memory, or NULL.
 * @param size  Number of bytes to duplicate.
 * @return Pointer to the new buffer, or NULL on allocation failure.
 */
void *x_lib_memdup(const void *p_src, size_t size)
{
	void *p_dst;

	if ((p_dst = x_sys_malloc(size)) == NULL) {
		return NULL;
	}

	if (p_src) {
		x_lib_memcpy(p_dst, p_src, size);
	}
#ifdef X_OPT_MEMZERO
	else {
		x_lib_memset(p_dst, 0, size);
	}
#endif /* X_OPT_MEMZERO */

	return p_dst;
}

/**
 * Convert a string to an integer.
 *
 * Parses @p p_str as a signed integer in the given @p base (2--36).
 * If @p base is 0, the base is auto-detected from the prefix:
 * `0x` for hexadecimal, `0` for octal, otherwise decimal.
 *
 * Leading whitespace is skipped. An optional `+` or `-` sign is accepted.
 * If @p pp_end is non-NULL, it is set to point past the last parsed character.
 *
 * @param p_str  The string to parse.
 * @param pp_end Output: pointer past the last parsed character, or NULL.
 * @param base   Numeric base (0 for auto-detect, or 2--36).
 * @return The parsed integer value.
 */
x_int_t x_lib_strtoint(const x_char_t *p_str, x_char_t **pp_end, unsigned short base)
{
#ifdef X_USE_STDLIB
	return strtol((char *)p_str, (char **)pp_end, base);
#else
	x_int_t i = 0, n, sign = 1;

	while (isspace(*p_str)) {
		p_str++;
	}

	if (*p_str == '+') {
		p_str++;
	} else if (*p_str == '-') {
		p_str++;
		sign = -1;
	}

	if (base == 0) {
		if (*p_str == '0') {
			if (toupper(*++p_str) == 'X') {
				base = 16;
				p_str++;
			} else {
				base = 8;
			}
		} else {
			base = 10;
		}
	}

	for (;*p_str; p_str++) {
		if (isdigit(*p_str)) {
			n = *p_str - '0';
#define BREAK_ON_BASE
#ifdef BREAK_ON_BASE
			if (n >= base) {
				break;
			}
			i *= base;
			i += n;
#else
			if (n < base) {
				i *= base;
				i += n;
			}
#endif
		}
		else if (toupper(*p_str) >= 'A' && toupper(*p_str) <= 'Z') {
			n = toupper(*p_str) - 'A' + 10;
#ifdef BREAK_ON_BASE
			if (n >= base) {
				break;
			}
			i *= base;
			i += n;
#else
			if (n < base) {
				i *= base;
				i += n;
			}
#endif
		} else {
			break;
		}
	}

	if (pp_end) {
		*pp_end = (x_char_t *)p_str;
	}

	return i * sign;
#endif /* X_USE_STDLIB */
}
