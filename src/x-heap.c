/**
 * @file x-heap.c
 * @brief Implementation of heap management and mark-sweep garbage collection.
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

#include "x-heap.h"

#ifdef X_HEAP

/**
 * Walk a pair tree, setting mark flags on each reachable object.
 *
 * For pairs, recursively marks the first branch and tail-iterates
 * the rest branch. For non-pair objects, calls the mark hook
 * (x_heap_mark_fn_t) from the base if set. Already-marked objects
 * (those with @p flags already set) are skipped.
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

/**
 * Scan a memory region for heap object pointers and mark them.
 *
 * Iterates through each pointer-sized value in the region from
 * @p p_start to @p p_start + @p size, comparing against all objects
 * in the heap chain. Any match is marked via x_heap_tree_mark().
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

/**
 * Mark objects referenced from the C call stack.
 *
 * Determines the stack bounds between the current position and the
 * stack base stored in the base object, then scans that region with
 * x_heap_vector_mark(). Uses X_NO_OPTIMIZE to prevent the compiler
 * from optimizing away the stack frame variable.
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

/**
 * Sweep the heap, freeing unmarked objects.
 *
 * Walks the heap chain starting from @p p_obj. Objects that have the
 * given @p flags set (or X_OBJ_FLAG_SHARED) are retained and their
 * mark flags are cleared. Unmarked objects are freed via x_obj_free(),
 * with the free hook called first if set.
 *
 * @note If the top object on the heap is deleted, the heap structure
 *       will fragment.
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
