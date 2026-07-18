/**
 * @file x-heap.c
 * @brief Implementation of heap management and mark-sweep garbage collection.
 *
 * @author Jon Ruttan (jonruttan@gmail.com)
 * @copyright 2021 Jon Ruttan
 * @license MIT No Attribution (MIT-0)
 *
 *         ., .,
 *         {O,O}
 *         (   )
 *          " "
 */

#include "x-heap.h"

#ifdef X_HEAP

/**
 * Walk a pair tree, setting mark flags on each reachable object.
 *
 * Traversal strategy: for each pair, recursively mark the first branch
 * (depth-first), then tail-iterate the rest branch (avoids stack overflow
 * on long lists). For non-pair objects, calls the mark hook
 * (x_heap_mark_fn_t) from the base if set -- the hook can return a next
 * object to continue tail-iteration, or NULL to stop. Already-marked
 * objects (those with @p flags already set) are skipped to avoid cycles.
 *
 * @param p_base Base (execution context).
 * @param p_obj  Root object to start marking from.
 * @param flags  Flags to set on each marked object.
 * @return The last object visited.
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
 * Sweep the heap, freeing unmarked objects.
 *
 * Walks the heap chain starting from @p p_obj. For each object:
 * - **Retained** if it has the mark @p flags set, or X_OBJ_FLAG_SHARED.
 *   The mark flags are then cleared for the next GC cycle.
 * - **Freed** otherwise: the free hook (x_heap_free_fn_t) is called
 *   first for type-specific cleanup, then x_obj_free(). The heap chain
 *   is relinked to skip the freed object.
 *
 * @note If the top object on the heap is deleted, the heap structure
 *       will fragment.
 *
 * @param p_base Base (execution context).
 * @param p_obj  Start of the heap chain to sweep.
 * @param flags  Mark flags to check (objects with these flags are retained).
 * @return The base object.
 */
x_obj_t *x_heap_sweep(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags)
{
	x_heap_free_fn_t p_free_fn = (x_base_isset(p_base)
		&& ! x_obj_isnil(p_base, x_firstobj(x_base_field_heap_free(p_base))))
		? (x_heap_free_fn_t)x_firstptr(x_firstobj(x_base_field_heap_free(p_base)))
		: NULL;
	x_obj_t *p_ret = p_base;
	x_obj_t *gc = p_obj, *tmp,
		*prev = x_obj_heap(p_base) == p_obj ? p_base : p_obj;

	while (gc) {
		if ((flags && x_obj_flags(gc) & flags)
			|| (x_obj_flags(gc) & X_OBJ_FLAG_SHARED))
		{
			x_obj_flags(gc) &= ~flags;
			prev = gc;
			gc = x_obj_heap(gc);
		} else {
			if (p_free_fn != NULL) {
				p_free_fn(p_base, gc);
			}

			tmp = x_obj_heap(prev) = x_obj_heap(gc);
			x_obj_free(p_base, gc);

			/* The teardown idiom -- x_heap_sweep(base, base, NONE) -- frees
			 * the base itself partway through the walk.  From then on the
			 * base is dead memory: stop handing it to x_obj_free (which
			 * chases base fields for the allocation accounting) and to the
			 * free hook.  The caller still gets the original pointer back. */
			if (gc == p_base) {
				p_base = NULL;
			}

			gc = tmp;
		}
	}

	return p_ret;
}

/**
 * Mark every object registered on the root chain.
 *
 * The root chain is the precise mirror of the C call stack: frames push
 * the off-chain (stack-storage) objects they create and pop them on
 * exit (x_heap_root_push() / x_heap_root_pop()), linked LIFO through
 * the same x_obj_heap() header slot the allocation chain uses, from a
 * different head -- so the chains never share a node, and registered
 * objects are marked here but never reached by x_heap_sweep().
 *
 * Two passes. Pass 1 clears the mark flags of every registered node:
 * x_heap_sweep() clears flags on the allocation chain only, so a node
 * that stays registered across two collections would otherwise keep a
 * stale mark and be skipped -- its referents missed -- by the second
 * one. The pre-clear also makes pass 2 order-independent when one
 * registered pair's payload references a node further along the chain
 * (stack-built argument lists), and when the base-tree mark has already
 * visited a node through a field that stores it.
 *
 * @param p_base Base (execution context).
 * @param flags  Flags to set on each marked object.
 * @return NULL.
 */
x_obj_t *x_heap_root_chain_mark(x_obj_t *p_base, x_obj_flag_t flags)
{
	x_obj_t *p_node;

	if ( ! x_base_isset(p_base)) {
		return NULL;
	}

	for (p_node = x_heap_root_chain(p_base); p_node != NULL; p_node = x_obj_heap(p_node)) {
		x_obj_flags(p_node) &= ~flags;
	}

	for (p_node = x_heap_root_chain(p_base); p_node != NULL; p_node = x_obj_heap(p_node)) {
		x_heap_tree_mark(p_base, p_node, flags);
	}

	return NULL;
}

/*
 * # Hook & Root Registration
 *
 * Each of these lists lives in a field cell -- the (current . saved)
 * pair at its base-tree field accessor (see x-base.h).  We prepend onto
 * the *current* list -- the first of the field cell -- so the saved
 * slot stays available for future masking use, and the collector walk
 * (which reads `x_firstobj(field)` to get the list) sees a proper list
 * head.  Passing &field directly to x_obj_push_field would replace the
 * whole field cell with (value . field-cell), which the walk would then
 * mis-interpret as a one-element list whose first IS value (rather than
 * a list containing value).
 */
/**
 * Register a mark hook on the base.
 *
 * Prepends @p p_hook to the base's mark-hooks list (see
 * x_base_field_heap_mark_hooks()). Each registered callable is intended to be
 * invoked once per garbage-collection mark phase by the consuming layer.
 *
 * @param p_base Base (execution context).
 * @param p_hook The callable to register.
 */
void x_heap_mark_hook_add(x_obj_t *p_base, x_obj_t *p_hook)
{
	x_obj_t *p_field = x_base_field_heap_mark_hooks(p_base);

	x_obj_push_field(p_base, &x_firstobj(p_field), p_hook, X_OBJ_FLAG_NONE);
}

/**
 * Register a free hook on the base.
 *
 * Prepends @p p_hook to the base's free-hooks list (see
 * x_base_field_heap_free_hooks()). Each registered callable is intended to be
 * invoked once per sweep phase, before objects are reclaimed.
 *
 * @param p_base Base (execution context).
 * @param p_hook The callable to register.
 */
void x_heap_free_hook_add(x_obj_t *p_base, x_obj_t *p_hook)
{
	x_obj_t *p_field = x_base_field_heap_free_hooks(p_base);

	x_obj_push_field(p_base, &x_firstobj(p_field), p_hook, X_OBJ_FLAG_NONE);
}

/**
 * Register a mark root on the base.
 *
 * Prepends @p p_root to the base's mark-roots list (see
 * x_base_field_heap_mark_roots()), so it is marked on every collection and
 * survives GC regardless of other reachability.
 *
 * @param p_base Base (execution context).
 * @param p_root The object to keep reachable.
 */
void x_heap_mark_root_add(x_obj_t *p_base, x_obj_t *p_root)
{
	x_obj_t *p_field = x_base_field_heap_mark_roots(p_base);

	x_obj_push_field(p_base, &x_firstobj(p_field), p_root, X_OBJ_FLAG_NONE);
}

#endif /* X_HEAP */
