#ifndef X_HEAP_H
#define X_HEAP_H

/*
 * # Computational Expressions in C
 *
 * ## x-heap.h -- Header - Heap Management
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
#include "x-obj.h"

#ifndef X_HEAP
#warning X_HEAP is required
#else /* X_HEAP */

/*
 * # Function Pointer Types
 */
/* Mark hook: called for non-pair objects. Return non-NULL p_obj
 * to continue tail-iteration, NULL to break. */
typedef x_obj_t *(*x_heap_mark_fn_t)(x_obj_t *, x_obj_t *, x_obj_flag_t);

/* Free hook: called before x_obj_free for type-specific cleanup. */
typedef void (*x_heap_free_fn_t)(x_obj_t *, x_obj_t *);

/*
 * # Heap Management Functions
 */
x_obj_t *x_heap_mark(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags,
	x_heap_mark_fn_t p_mark_fn);
x_obj_t *x_heap_sweep(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags,
	x_heap_free_fn_t p_free_fn);

/* Conservative stack scanning: mark heap objects referenced from C stack.
 * Call x_heap_stack_init() once at startup to record the stack base.
 * x_heap_mark_stack() scans the C stack during GC mark phase. */
extern void *x_heap_stack_base;
void x_heap_mark_stack(x_obj_t *p_base, x_obj_flag_t flags,
	x_heap_mark_fn_t p_mark_fn);

#endif /* X_HEAP */

#endif /* X_HEAP_H */
