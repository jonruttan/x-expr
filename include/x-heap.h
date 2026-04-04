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

/**
 * Walk a pair tree, setting mark flags on each reachable object.
 *
 * Recursively marks the first branch and tail-iterates the rest branch.
 * For non-pair objects, calls the mark hook from the base if set.
 *
 * @param p_base The base environment.
 * @param p_obj  Root object to start marking from.
 * @param flags  Flags to set on each marked object.
 * @return The last object visited (for tail-iteration chaining).
 */
x_obj_t *x_heap_tree_mark(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags);

/**
 * Scan a memory region for heap object pointers and mark them.
 *
 * Treats each pointer-sized value in the region as a potential object
 * pointer, checking it against the heap chain. Matches are marked via
 * x_heap_tree_mark().
 *
 * @param p_base  The base environment.
 * @param p_start Start of the memory region to scan.
 * @param size    Size of the region in bytes.
 * @param flags   Flags to set on each marked object.
 * @return NULL.
 */
x_obj_t *x_heap_vector_mark(x_obj_t *p_base, void *p_start, size_t size, x_obj_flag_t flags);

/**
 * Sweep the heap, freeing unmarked objects.
 *
 * Walks the heap chain and frees objects that do not have the given
 * mark flags set. Objects with X_OBJ_FLAG_SHARED are always retained.
 * Calls the free hook if set before freeing each object.
 *
 * @note If the top object on the heap is deleted, the heap structure
 *       will fragment.
 *
 * @param p_base The base environment.
 * @param p_obj  Start of the heap chain to sweep.
 * @param flags  Mark flags to check (objects with these flags are retained).
 * @return The base object.
 */
x_obj_t *x_heap_sweep(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags);

/**
 * Mark objects referenced from the C call stack.
 *
 * Scans the memory between the current stack position and the stack base
 * stored in the base object, marking any heap objects found. Uses
 * X_NO_OPTIMIZE to prevent the compiler from optimizing away the
 * stack frame.
 *
 * @param p_base The base environment.
 * @param flags  Flags to set on each marked object.
 * @return Result of x_heap_vector_mark() on the stack region.
 */
x_obj_t *x_heap_callstack_mark(x_obj_t *p_base, x_obj_flag_t flags);

/** @} */

#endif /* X_HEAP */

#endif /* X_HEAP_H */
