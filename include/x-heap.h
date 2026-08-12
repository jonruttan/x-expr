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
 * -# **Mark**: call x_heap_tree_mark() on known roots and
 *    x_heap_root_chain_mark() for the registered stack roots.
 * -# **Sweep**: call x_heap_sweep() to free all unmarked objects.
 *    Objects with X_OBJ_FLAG_SHARED are always retained. Mark flags
 *    are cleared during sweep, preparing for the next cycle.
 *
 * **Rooting rules** -- an object survives GC if any of these apply:
 * - It has X_OBJ_FLAG_SHARED set (permanent; used for base tree nodes).
 * - It is reachable from a pair tree passed to x_heap_tree_mark().
 * - It is reachable from an off-chain object registered on the root
 *   chain (x_heap_root_push()), marked each collection by
 *   x_heap_root_chain_mark().
 * - It is stored in a base environment field (reachable when the
 *   base tree is marked).
 *
 * A C local holding the only reference to a heap object is NOT a root
 * by itself: the frame must register it (see the root chain below).
 *
 * Objects not reachable by any of the above are collected during sweep.
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
 *
 * @warning These functions assume single-threaded execution. The heap
 * chain and mark flags are not synchronized.
 * @{
 */

/** Walk a pair tree, setting mark flags on each reachable object. */
x_obj_t *x_heap_tree_mark(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags);

/** Sweep the heap, freeing unmarked objects. */
x_obj_t *x_heap_sweep(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags);

/** @} */

/**
 * @name Hook & Root Registration
 *
 * The base's heap-group carries three extensible lists used by callers
 * to extend GC behaviour at runtime:
 * - **mark-hooks** -- callables invoked once per mark phase (subscribers
 *   that want to mark additional reachable objects).
 * - **free-hooks** -- callables invoked once per sweep phase, before
 *   reclamation (subscribers that need to react to a collection).
 * - **mark-roots** -- objects to mark on every collection, ensuring they
 *   survive GC regardless of reachability.
 *
 * x-expr stores the lists and provides these registration helpers; the
 * consuming layer walks the lists during its mark/sweep primitives and
 * dispatches the callables per its own conventions (x-expr has no
 * callable-dispatch).
 * @{
 */

/** Push a callable onto the mark-hooks list. */
void x_heap_mark_hook_add(x_obj_t *p_base, x_obj_t *p_hook);

/** Push a callable onto the free-hooks list. */
void x_heap_free_hook_add(x_obj_t *p_base, x_obj_t *p_hook);

/** Push an object onto the mark-roots list. */
void x_heap_mark_root_add(x_obj_t *p_base, x_obj_t *p_root);

/** @} */

/**
 * @name Root Chain (precise stack roots)
 *
 * Track-on-push registration for off-chain objects: C frames push the
 * stack-storage objects they create (x_satom_t / x_spair_t locals,
 * x_obj_set-initialized) as GC roots and pop them on exit, LIFO,
 * mirroring the call structure. Registered objects link through the
 * same x_obj_heap() header slot the allocation chain uses -- dormant
 * (NULL) in every off-chain object -- but from a different head
 * (x_base_field_heap_root_chain()), so any object is on exactly one
 * chain: the allocation chain is swept, the root chain is marked.
 *
 * Rules the registering frame must keep:
 * - Only off-chain objects may be pushed. Pushing a heap-allocated
 *   object overwrites its allocation-chain link and truncates the
 *   sweep chain.
 * - Registered pairs must carry the built-in pair type
 *   (#x_type_pair_obj): the mark walk traverses payloads through
 *   x_heap_tree_mark(), which descends only spair-typed pairs.
 * - Pops must run on every exit path. Nonlocal exits (longjmp) must
 *   restore the head from a snapshot taken at the setjmp point --
 *   the cut-away frames' nodes are dead stack memory.
 * - Push each node at most once: the chain has no cycle guard.
 * @{
 */

/** Head of the root chain on @p B (lvalue; nil when empty). */
#define x_heap_root_chain(B) \
	x_firstobj(x_base_field_heap_root_chain(B))

/**
 * Address of the root-chain head slot of @p B, for hoisting into a
 * frame local (`x_obj_t **`) so pushes and pops skip the base-tree
 * field chase.
 *
 * Yields nil when the base is not set: an unset base has no chain to
 * register on -- and no collector to need it -- so x_heap_root_push()
 * and x_heap_root_pop() degrade to no-ops and frames keep working for
 * NULL/partial-base callers (unit specs, embedder scratch), the same
 * x_base_isset() contract the mark/sweep walkers follow.
 */
#define x_heap_root_slot(B) \
	(x_base_isset(B) ? &x_heap_root_chain(B) : (x_obj_t **)NULL)

/**
 * Register off-chain object @p node as a GC root (two stores; no-op
 * when @p p_slot is nil).
 *
 * @param p_slot `x_obj_t **` -- the head slot (see x_heap_root_slot()).
 * @param node   The off-chain object; evaluated twice.
 */
#define x_heap_root_push(p_slot, node) \
	((p_slot) != NULL \
		? (void)(x_obj_heap((x_obj_t *)(node)) = *(p_slot), \
			*(p_slot) = (x_obj_t *)(node)) \
		: (void)0)

/**
 * Unregister the most recently pushed root (one store; no-op when
 * @p p_slot is nil).
 *
 * @param p_slot `x_obj_t **` -- the head slot (see x_heap_root_slot()).
 */
#define x_heap_root_pop(p_slot) \
	((p_slot) != NULL \
		? (void)(*(p_slot) = x_obj_heap(*(p_slot))) \
		: (void)0)

/** Mark every object registered on the root chain (two passes). */
x_obj_t *x_heap_root_chain_mark(x_obj_t *p_base, x_obj_flag_t flags);

/** @} */

#endif /* X_HEAP */

#endif /* X_HEAP_H */
