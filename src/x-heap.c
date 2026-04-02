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
 * # Heap Tree Mark
 *
 * Walk a pair tree, setting flags on each object.
 * For non-pair objects, calls the mark hook from the base if set.
 */
x_obj_t *x_heap_tree_mark(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags)
{
	x_heap_mark_fn_t p_mark_fn = (x_base_isset(p_base)
		&& ! x_obj_isnil(p_base, x_firstobj(x_base_field_heap_mark(p_base))))
		? (x_heap_mark_fn_t)x_firstptr(x_firstobj(x_base_field_heap_mark(p_base)))
		: NULL;

	while (p_obj != NULL && (x_obj_flags(p_obj) & flags) != flags) {
		x_obj_flags(p_obj) |= flags;

		if (x_obj_type_isspair(p_obj)) {
			x_heap_tree_mark(p_base, x_firstobj(p_obj), flags);
			p_obj = x_restobj(p_obj);

			continue;
		}

		if (p_mark_fn != NULL) {
			p_obj = p_mark_fn(p_base, p_obj, flags);

			if (p_obj != NULL) {
				continue;
			}
		}

		break;
	}

	return p_obj;
}

/*
 * # Heap Vector Mark
 *
 * Scan a memory region for pointer-sized values that match heap
 * object addresses, marking any matches.
 */
x_obj_t *x_heap_vector_mark(x_obj_t *p_base, void *p_start, size_t size,
	x_obj_flag_t flags)
{
	void **p = (void **)p_start;
	void **p_end = (void **)((char *)p_start + size);
	x_obj_t *gc;

	for (; p < p_end; p++) {
		for (gc = x_obj_heap(p_base); gc != NULL; gc = x_obj_heap(gc)) {
			if ((void *)gc == *p) {
				x_heap_tree_mark(p_base, gc, flags);

				break;
			}
		}
	}

	return NULL;
}

/*
 * # Heap Callstack Mark
 *
 * Scan the C call stack for heap object references.
 * Uses the stack base captured in the base object at creation.
 */
X_NO_OPTIMIZE x_obj_t *x_heap_callstack_mark(x_obj_t *p_base, x_obj_flag_t flags)
{
	volatile int stack_top;
	void *lo, *hi, *tmp, *stack_base;

	if ( ! x_base_isset(p_base)) {
		return NULL;
	}

	stack_base = x_atomptr(x_firstobj(x_base_field_stack_base(p_base)));

	if (stack_base == NULL) {
		return NULL;
	}

	lo = (void *)&stack_top;
	hi = stack_base;

	if (lo > hi) {
		tmp = lo;
		lo = hi;
		hi = tmp;
	}

	return x_heap_vector_mark(p_base, lo, (size_t)((char *)hi - (char *)lo), flags);
}

/*
 * # Heap Sweep
 *
 * Walk the heap chain, freeing objects that don't have the given
 * flags set. Calls the free hook from the base if set.
 *
 * NOTE: If the top object is deleted the heap structure will fragment.
 */
x_obj_t *x_heap_sweep(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags)
{
	x_heap_free_fn_t p_free_fn = (x_base_isset(p_base)
		&& ! x_obj_isnil(p_base, x_firstobj(x_base_field_heap_free(p_base))))
		? (x_heap_free_fn_t)x_firstptr(x_firstobj(x_base_field_heap_free(p_base)))
		: NULL;
	x_obj_t *gc = p_obj, *tmp,
		*prev = x_obj_heap(p_base) == p_obj ? p_base : p_obj;

	while (gc) {
		if ((flags && x_obj_flags(gc) & flags)
			|| (x_obj_flags(gc) & X_OBJ_FLAG_SHARED)
		) {
			x_obj_flags(gc) &= ~flags;
			prev = gc;
			gc = x_obj_heap(gc);
		} else {
			if (p_free_fn != NULL) {
				p_free_fn(p_base, gc);
			}

			tmp = x_obj_heap(prev) = x_obj_heap(gc);
			x_obj_free(p_base, gc);
			gc = tmp;
		}
	}

	return p_base;
}

#endif /* X_HEAP */
