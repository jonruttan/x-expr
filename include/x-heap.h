#ifndef X_HEAP_H
#define X_HEAP_H

/**
 * @file x-heap.h
 * @brief Heap management and mark-sweep garbage collection.
 *
 * Provides functions for marking reachable objects and sweeping
 * unreachable ones. Requires X_HEAP to be defined.
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

#include "x-base.h"

#ifndef X_HEAP
#warning X_HEAP is required
#else /* X_HEAP */

/**
 * @name Function Pointer Types
 * @{
 */

/**
 * Mark hook for non-pair objects.
 *
 * Called during tree marking when a non-pair object is encountered.
 * Return a non-NULL object pointer to continue tail-iteration,
 * or NULL to stop.
 */
typedef x_obj_t *(*x_heap_mark_fn_t)(x_obj_t *, x_obj_t *, x_obj_flag_t);

/**
 * Free hook for type-specific cleanup.
 *
 * Called before x_obj_free() during sweeping to allow types to release
 * additional resources.
 */
typedef void (*x_heap_free_fn_t)(x_obj_t *, x_obj_t *);

/** @} */

/**
 * @name Heap Management Functions
 * @{
 */

/** Walk a pair tree, setting mark flags on each reachable object. */
x_obj_t *x_heap_tree_mark(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags);

/** Scan a memory region for heap object pointers and mark them. */
x_obj_t *x_heap_vector_mark(x_obj_t *p_base, void *p_start, size_t size, x_obj_flag_t flags);

/** Sweep the heap, freeing unmarked objects. */
x_obj_t *x_heap_sweep(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags);

/** Mark objects referenced from the C call stack. */
x_obj_t *x_heap_callstack_mark(x_obj_t *p_base, x_obj_flag_t flags);

/** @} */

#endif /* X_HEAP */

#endif /* X_HEAP_H */
