#ifndef X_HEAP_H
#define X_HEAP_H

/**
 * @file x-heap.h
 * @brief Heap management and mark-sweep garbage collection.
 *
 * Provides functions for marking reachable objects and sweeping
 * unreachable ones. Requires X_HEAP to be defined.
 *
 * Objects are linked into a singly-linked heap chain via x_obj_heap().
 * The GC lifecycle is:
 * -# **Mark**: call x_heap_tree_mark() on known roots and/or
 *    x_heap_callstack_mark() to scan the C stack for references.
 * -# **Sweep**: call x_heap_sweep() to free all unmarked objects.
 *    Objects with X_OBJ_FLAG_SHARED are always retained.
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
 * Called by x_heap_tree_mark() when a non-pair object is encountered.
 * The hook should mark any objects reachable from p_obj (e.g. by calling
 * x_heap_tree_mark() on them). Return a non-NULL object pointer to
 * continue tail-iteration (the returned object will be marked next),
 * or NULL to stop traversal at this branch.
 *
 * Arguments: (p_base, p_obj, flags) -- the object to mark and flags to set.
 */
typedef x_obj_t *(*x_heap_mark_fn_t)(x_obj_t *, x_obj_t *, x_obj_flag_t);

/**
 * Free hook for type-specific cleanup.
 *
 * Called by x_heap_sweep() just before x_obj_free() on each object
 * being collected. Use this to release type-specific resources (e.g.
 * external handles, extra allocations) that x_obj_free() would not
 * know about.
 *
 * Arguments: (p_base, p_obj) -- the object about to be freed.
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
