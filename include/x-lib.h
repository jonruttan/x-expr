#ifndef X_LIB_H
#define X_LIB_H

/**
 * @file x-lib.h
 * @brief General-purpose library helpers.
 *
 * Helpers with x-expr's own contracts, declared in the x layer's
 * types. The C standard library replacements -- stdlib names, stdlib
 * types, stdlib semantics -- live in x-stdlib.h; this header pulls
 * them in so the library layer is one include.
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

/**
 * @name Library Functions
 * @{
 */

/** Convert an integer to a string representation. */
x_char_t *x_lib_inttostr(x_int_t num, x_char_t *p_str, unsigned short base);

/** Duplicate a memory region. */
void *x_lib_memdup(const void *p_src, size_t size);

/** Convert a string to an integer. */
x_int_t x_lib_strtoint(const x_char_t *p_str, x_char_t **pp_end, unsigned short base);

/** @} */

#endif /* X_LIB_H */
