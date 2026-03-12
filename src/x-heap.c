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
	/* Iterate rest-slots to avoid stack overflow on long chains. */
	while (p_obj != NULL && (x_obj_flags(p_obj) & flags) != flags) {
		x_obj_flags(p_obj) |= flags;

		if (x_obj_type_isspair(p_obj)) {
			/* Simple pair — recurse into first, iterate on rest. */
			x_heap_mark(p_base, x_firstobj(p_obj), flags);
			p_obj = x_restobj(p_obj);
			continue;
		}

#ifdef X_TYPE
		/* Registered type: check p_units in type structure.
		 * Type layout: (name data (make free clone units length) ...)
		 * A registered type's root node is a simple pair. */
		{
			x_obj_t *p_type = x_obj_type(p_obj);

			if (p_type != NULL && x_obj_type_isspair(p_type)) {
				x_obj_t *p_units = x_firstobj(x_restobj(
					x_restobj(x_restobj(x_firstobj(
					x_restobj(x_restobj(p_type)))))));

				if (p_units != NULL) {
					x_int_t n = x_atomint(p_units);
					x_int_t i;

					for (i = 0; i < n - 1; i++) {
						x_heap_mark(p_base,
							x_obj(x_obj_data_i(
							p_obj, i)), flags);
					}
					/* Tail-iterate on last slot. */
					p_obj = x_obj(x_obj_data_i(
						p_obj, n - 1));
					continue;
				}
			}
		}
#endif /* X_TYPE */

		break;
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
