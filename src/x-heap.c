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
 * # Conservative Stack Scanning
 *
 * The stack base is recorded once at interpreter startup.
 * During GC mark, we scan the C stack for values that match
 * heap object addresses. This prevents sweep from freeing
 * objects that are only referenced from C local variables.
 */
void *x_heap_stack_base = NULL;

void x_heap_mark_stack(x_obj_t *p_base, x_obj_flag_t flags,
	x_heap_mark_fn_t p_mark_fn)
{
	volatile int stack_top;
	void **lo, **hi, **p;
	x_obj_t *gc;

	if (x_heap_stack_base == NULL) {
		return;
	}

	/* Determine stack scan range (stack may grow up or down) */
	lo = (void **)&stack_top;
	hi = (void **)x_heap_stack_base;

	if (lo > hi) {
		void **tmp = lo; lo = hi; hi = tmp;
	}

	/* For each aligned word on the stack, check if it matches
	 * any heap object address. O(stack * heap) but uses only
	 * the existing heap chain — no external functions. */
	for (p = lo; p < hi; p++) {
		for (gc = x_obj_heap(p_base); gc != NULL; gc = x_obj_heap(gc)) {
			if ((void *)gc == *p) {
				x_heap_mark(p_base, gc, flags, p_mark_fn);

				break;
			}
		}
	}
}

/*
 * # Heap Management Functions
 */
x_obj_t *x_heap_mark(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags,
	x_heap_mark_fn_t p_mark_fn)
{
	/* Iterate rest-slots to avoid stack overflow on long chains. */
	while (p_obj != NULL && (x_obj_flags(p_obj) & flags) != flags) {
		x_obj_flags(p_obj) |= flags;

		if (x_obj_type_isspair(p_obj)) {
			/* Simple pair — recurse into first, iterate on rest. */
			x_heap_mark(p_base, x_firstobj(p_obj), flags,
				p_mark_fn);
			p_obj = x_restobj(p_obj);
			continue;
		}

		if (p_mark_fn != NULL) {
			p_obj = p_mark_fn(p_base, p_obj, flags);
			if (p_obj != NULL)
				continue;
		}

		break;
	}

	return p_obj;
}

/*
 * NOTE: If the top object is deleted the heap structure will fragment.
 */
x_obj_t *x_heap_sweep(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags,
	x_heap_free_fn_t p_free_fn)
{
	x_obj_t *gc = p_obj, *tmp,
		*prev = x_obj_heap(p_base) == p_obj ? p_base : p_obj;

	while (gc) {
		if ((flags && x_obj_flags(gc) & flags)
			|| (x_obj_flags(gc) & (X_OBJ_FLAG_SHARED | X_OBJ_FLAG_SYSTEM))) {
			x_obj_flags(gc) &= ~flags;
			prev = gc;
			gc = x_obj_heap(gc);
		} else {
			if (p_free_fn != NULL) {
				p_free_fn(p_base, gc);
			}

			tmp = x_obj_heap(prev) = x_obj_heap(gc);
			x_obj_free(gc);
			gc = tmp;
		}
	}

	return p_base;
}

#endif /* X_HEAP */
