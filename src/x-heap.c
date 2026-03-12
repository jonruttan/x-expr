/*
 * # Computational Expressions in C
 *
 * ## x-heap.c -- Implementation - Heap Management
 *
 * @description Computational Expressions in C
 * @author [Jon Ruttan](jonruttan@gmail.com)
 * @copyright 2021 Jon Ruttan
 * @license MIT No Attribution (MIT-0)
 *
 *     ., .,
 *     {O,O}
 *     (   )
 *      " "
 */
/*
 * # Includes
 */
#include "x-heap.h"

#ifdef X_HEAP

/*
 * # Heap Management Functions
 */
x_obj_t *x_heap_mark(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags)
{
	union x_datum_union *tmp;
	short units;

	if (
		/* Avoid recursion. */
		(x_obj_flags(p_obj) & flags) != flags

		/* Set the flags, and exit if the OBJ flag is set. */
		&& ~(x_obj_flags(p_obj) |= flags) & X_OBJ_FLAG_OBJ

		/* Get the number of units for this object, exit when 0. */
		&& (units = x_obj_units(p_base, p_obj))
	) {
		tmp = x_obj_data_ptr(p_obj) + units - 1;

		while (tmp >= x_obj_data_ptr(p_obj)) {
			x_heap_mark(p_base, x_obj(*tmp--), flags);
		}
	}


	return p_obj;
}

/*
 * NOTE: If the top object is deleted the heap structure will fragment.
 */
x_obj_t *x_heap_sweep(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags)
{
	x_obj_t *gc = p_obj, *tmp,
		*prev = x_obj_heap(p_base) == p_obj ? p_base : p_obj;

	while (gc) {
		if (flags && x_obj_flags(gc) & flags) {
			x_obj_flags(gc) &= ~flags;
			prev = gc;
			gc = x_obj_heap(gc);
		} else {
			tmp = x_obj_heap(prev) = x_obj_heap(gc);
			x_obj_free(gc);
			gc = tmp;
		}
	}

	return p_base;
}

#endif /* X_HEAP */
