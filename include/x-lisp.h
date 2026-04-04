#ifndef X_LISP_H
#define X_LISP_H

/**
 * @file x-lisp.h
 * @brief Lisp-style aliases for pair construction and traversal.
 *
 * Provides traditional Lisp names (cons, car, cdr) as macros that map
 * to the underlying x-obj pair operations.
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

#include "x-obj.h"

/**
 * Construct a new pair (cons cell).
 *
 * Alias for x_mkspair().
 *
 * @param B Base environment.
 * @param F Object flags.
 * @param X First element (car).
 * @param Y Rest element (cdr).
 */
#define x_cons(B,F,X,Y)		x_mkspair((B), (F), (X), (Y))

/**
 * @defgroup lisp_car_cdr Lisp car/cdr Accessors
 * @brief Traditional Lisp accessors for navigating pair structures.
 *
 * - `x_car(X)` returns the first element of pair X.
 * - `x_cdr(X)` returns the rest element of pair X.
 *
 * Compound accessors follow standard Lisp naming: each letter between
 * `c` and `r` is `a` (car) or `d` (cdr), applied right-to-left.
 * For example, `x_cadr(X)` = `car(cdr(X))` = first of rest.
 *
 * Provided up to four levels deep (e.g. x_caddar).
 * @{
 */

#define x_car(X)			x_firstobj(X)	/**< First element of pair X. */
#define x_cdr(X)			x_restobj(X)	/**< Rest element of pair X. */

/* 2-deep */
#define x_caar(X)			x_car(x_car(X))	/**< car(car(X)) */
#define x_cadr(X)			x_car(x_cdr(X))	/**< car(cdr(X)) */
#define x_cdar(X)			x_cdr(x_car(X))	/**< cdr(car(X)) */
#define x_cddr(X)			x_cdr(x_cdr(X))	/**< cdr(cdr(X)) */

/* 3-deep */
#define x_caaar(X)			x_car(x_caar(X))	/**< car(car(car(X))) */
#define x_caadr(X)			x_car(x_cadr(X))	/**< car(car(cdr(X))) */
#define x_cadar(X)			x_car(x_cdar(X))	/**< car(cdr(car(X))) */
#define x_caddr(X)			x_car(x_cddr(X))	/**< car(cdr(cdr(X))) */
#define x_cdaar(X)			x_cdr(x_caar(X))	/**< cdr(car(car(X))) */
#define x_cdadr(X)			x_cdr(x_cadr(X))	/**< cdr(car(cdr(X))) */
#define x_cddar(X)			x_cdr(x_cdar(X))	/**< cdr(cdr(car(X))) */
#define x_cdddr(X)			x_cdr(x_cddr(X))	/**< cdr(cdr(cdr(X))) */

/* 4-deep */
#define x_caaaar(X)			x_car(x_caaar(X))	/**< 4-deep: car(car(car(car(X)))) */
#define x_caaadr(X)			x_car(x_caadr(X))	/**< 4-deep: car(car(car(cdr(X)))) */
#define x_caadar(X)			x_car(x_cadar(X))	/**< 4-deep: car(car(cdr(car(X)))) */
#define x_caaddr(X)			x_car(x_caddr(X))	/**< 4-deep: car(car(cdr(cdr(X)))) */
#define x_cadaar(X)			x_car(x_cdaar(X))	/**< 4-deep: car(cdr(car(car(X)))) */
#define x_cadadr(X)			x_car(x_cdadr(X))	/**< 4-deep: car(cdr(car(cdr(X)))) */
#define x_caddar(X)			x_car(x_cddar(X))	/**< 4-deep: car(cdr(cdr(car(X)))) */
#define x_cadddr(X)			x_car(x_cdddr(X))	/**< 4-deep: car(cdr(cdr(cdr(X)))) */
#define x_cdaaar(X)			x_cdr(x_caaar(X))	/**< 4-deep: cdr(car(car(car(X)))) */
#define x_cdaadr(X)			x_cdr(x_caadr(X))	/**< 4-deep: cdr(car(car(cdr(X)))) */
#define x_cdadar(X)			x_cdr(x_cadar(X))	/**< 4-deep: cdr(car(cdr(car(X)))) */
#define x_cdaddr(X)			x_cdr(x_caddr(X))	/**< 4-deep: cdr(car(cdr(cdr(X)))) */
#define x_cddaar(X)			x_cdr(x_cdaar(X))	/**< 4-deep: cdr(cdr(car(car(X)))) */
#define x_cddadr(X)			x_cdr(x_cdadr(X))	/**< 4-deep: cdr(cdr(car(cdr(X)))) */
#define x_cdddar(X)			x_cdr(x_cddar(X))	/**< 4-deep: cdr(cdr(cdr(car(X)))) */
#define x_cddddr(X)			x_cdr(x_cdddr(X))	/**< 4-deep: cdr(cdr(cdr(cdr(X)))) */

/** @} */

/**
 * @name Pair Mutation
 * @{
 */

/** Set the car (first element) of pair @p X to @p Y. */
#define x_setcar(X,Y)		(x_car((X)) = (Y))

/** Set the cdr (rest element) of pair @p X to @p Y. */
#define x_setcdr(X,Y)		(x_cdr((X)) = (Y))

/** @} */

#endif /* X_LISP_H */
