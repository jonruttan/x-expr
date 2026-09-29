#ifndef X_BASE_H
#define X_BASE_H

/**
 * @file x-base.h
 * @brief Base environment object -- the root context for evaluation.
 *
 * The base object has two data units. The first holds the slot vector:
 * the routines the engine calls through, each at a fixed position (see
 * @ref x-slots.h). The second holds a nested pair tree with the I/O
 * descriptors, profiling counters, and heap/GC state. Field accessor
 * macros navigate the tree without hard-coding offsets.
 *
 * @details
 * Every leaf field is stored as a pair -- its @e field @e cell:
 * `(current-value . saved-values)`. Fields are initialized with
 * `(value . nil)`. Any field can be used as
 * a stack via x_obj_push() / x_obj_pop() to save and restore values.
 * ("Field cell" always means this slot; a plain "cell" is a cons cell,
 * see @ref x-lisp.h.)
 * To read a field's current value, use `x_firstobj(field)` then extract
 * the appropriate datum (e.g. `x_atomint()` for integers). To
 * temporarily override a field (e.g. redirect I/O for a nested scope),
 * push a new value before entering the scope and pop it on exit.
 *
 * @note The tree layout below is stable API. Field positions will not
 * change within a major version. Extension points (env+ctrl, type-alist,
 * io-state) are reserved for downstream use.
 *
 * The tree has three extension points (marked below) where downstream
 * projects can attach additional state: `env+ctrl` for environment
 * bindings, `type-alist` for type dispatch tables, and `io-state`
 * for I/O subsystem extensions. These start as nil.
 *
 * @code
 * (lit base-data
 *   (env+ctrl
 *     . ((; io
 *         (type-alist
 *           . (filein fileout fileerr write-buf buffer))
 *         . io-state)
 *       . ((; meta
 *           (; profile
 *             (allocs)
 *             . ())
 *           . ((; heap
 *               (obj-meta-extra
 *                mark-hooks free-hooks mark-roots root-chain)
 *               . ())
 *             . ()))
 *         . ()))))
 * @endcode
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

#include "x-obj.h"
#include "x-slots.h"

/**
 * @name Base Object Root
 * @{
 */

/* TODO: Add name and version fields. */

/** Get the slot vector of the base object @p X: its first data unit. */
#define x_base_slots(X)						x_firstobj(X)

/** Get the root of the base object @p X's tree: its second data unit. */
#define x_base(X)							x_restobj(X)

/**
 * Test whether the base object @p B is initialized and has data: it has
 * a slot vector and a tree.
 *
 * The slot vector is tested first. An object of one unit may stand as an
 * allocation context with nothing in it, and testing its first unit finds
 * it unset without reading a second unit it does not have.
 */
#define x_base_isset(B) \
	((B) != NULL \
		&& x_base_slots((B)) != NULL \
		&& x_base((B)) != NULL)

/** @} */

/**
 * @name Base Slots
 * @{
 */

/** The function pointer in slot @p I of base @p B (an lvalue). */
#define x_base_slot(B,I)					x_slot(x_base_slots((B)), (I))

/**
 * The type a vector made for base @p B carries: the type of its slot
 * vector, which the embedding layer sets when it has a vector type to
 * give. One load, from a vector a call through a slot loads in any case.
 */
#define x_base_vector_type(B)				x_obj_type(x_base_slots((B)))

/**
 * Call the function in slot @p I of base @p B with argument vector @p A.
 * The base must be set and the slot must hold a function.
 */
#define x_base_call(B,I,A)					(x_base_slot((B), (I))((B), (A)))

/**
 * Test whether slot @p I of base @p B holds a function: the base is
 * set, it has a slot vector, and the slot is not empty.
 */
#define x_base_slot_isset(B,I) \
	(x_base_isset((B)) && x_base_slot((B), (I)) != NULL)

/** @} */

/**
 * @defgroup base_io Base IO Field Accessors
 * @brief Navigate the I/O section of the base object tree.
 * @{
 */

/** Get the IO fields pair (type-alist . files, io-state). */
#define x_base_field_io_fields(X)			x_firstobj(x_restobj(x_base(X)))

/** Get the files list (filein, fileout, fileerr, write-buf, buffer). */
#define x_base_field_files(X)				x_restobj(x_firstobj(x_base_field_io_fields(X)))

/** Get the filein field. Value: integer atom (file descriptor). */
#define x_base_field_filein(X)				x_firstobj(x_base_field_files(X))

/** Get the fileout field. Value: integer atom (file descriptor). */
#define x_base_field_fileout(X)				x_firstobj(x_restobj(x_base_field_files(X)))

/** Get the fileerr field. Value: integer atom (file descriptor). */
#define x_base_field_fileerr(X)				x_firstobj(x_restobj(x_restobj(x_base_field_files(X))))

/**
 * Get the write buffer field.
 * Value: pair `(pointer . position)` when active, or nil when disabled.
 * When set, x_base_write() copies data here instead of writing to fileout.
 */
#define x_base_field_write_buf(X)			x_firstobj(x_restobj(x_restobj(x_restobj(x_base_field_files(X)))))

/** Get the input buffer field. Reserved for extension use. */
#define x_base_field_buffer(X)				x_firstobj(x_restobj(x_restobj(x_restobj(x_restobj(x_base_field_files(X))))))

/** @} */

/**
 * @defgroup base_meta Base Meta Field Accessors
 * @brief Navigate the metadata section (profile) of the base tree.
 * @{
 */

/** Get the meta fields list (profile, heap, alloc; the tail past alloc
 *  is the embedding layer's extension point). */
#define x_base_field_meta_fields(X)			x_restobj(x_restobj(x_base(X)))

/** Get the profile pair (counters). */
#define x_base_field_profile(X)				x_firstobj(x_firstobj(x_base_field_meta_fields(X)))

/** Get the allocation counter field. Value: integer atom, incremented by x_obj_alloc(). */
#define x_base_field_profile_allocs(X)		x_firstobj(x_base_field_profile(X))

/** @} */

/**
 * @defgroup base_heap Base Heap Field Accessors
 * @brief Navigate the heap/GC section of the base object tree.
 * @{
 */

/** Get the heap fields pair. */
#define x_base_field_heap_fields(X)			x_firstobj(x_restobj(x_base_field_meta_fields(X)))

/**
 * Get the extra metadata units count field. Value: integer atom.
 * When > 0, x_obj_alloc() prepends this many extra units before each
 * object's standard metadata, accessible via x_obj_meta_i().
 */
#define x_base_field_obj_meta_extra(X)		x_firstobj(x_base_field_heap_fields(X))

/**
 * Get the mark-hooks list field. Value: a list of callables, each
 * invoked once per garbage collection mark phase (fan-out subscribers).
 * Extended at runtime via x_heap_mark_hook_add() rather than reserved
 * as a single fixed slot. The collector itself does not invoke the
 * hooks (it has no callable-dispatch); the consuming layer walks the
 * list and dispatches per its own conventions.
 */
#define x_base_field_heap_mark_hooks(X)		x_firstobj(x_restobj(x_base_field_heap_fields(X)))

/**
 * Get the free-hooks list field. Value: a list of callables, each
 * invoked once per sweep phase before objects are reclaimed.
 * Extended at runtime via x_heap_free_hook_add().
 */
#define x_base_field_heap_free_hooks(X)		x_firstobj(x_restobj(x_restobj(x_base_field_heap_fields(X))))

/**
 * Get the mark-roots list field. Value: a list of objects to mark on
 * every collection (so they survive GC even when not reachable from the
 * base tree or the root chain). Extended at runtime via x_heap_mark_root_add().
 */
#define x_base_field_heap_mark_roots(X)		x_firstobj(x_restobj(x_restobj(x_restobj(x_base_field_heap_fields(X)))))

/**
 * Get the root-chain head field. Value: the most recently registered
 * off-chain (stack- or static-storage) object, or nil. Registered objects
 * link LIFO through the same x_obj_heap() header slot the allocation
 * chain uses -- with a different head, so any object is on exactly one
 * of the two chains. The chain is marked on every collection
 * (x_heap_root_chain_mark()) but never swept: x_heap_sweep() walks only
 * the allocation chain. See x_heap_root_push() / x_heap_root_pop().
 */
#define x_base_field_heap_root_chain(X)		x_firstobj(x_restobj(x_restobj(x_restobj(x_restobj(x_base_field_heap_fields(X))))))

/** @} */

/**
 * @defgroup base_alloc Base Allocation Field Accessors
 * @brief Object-allocation accounting (the meta fields' alloc element).
 *
 * Maintained by x_obj_alloc / x_obj_free in every build: allocation-layer
 * state, not part of X_HEAP garbage collection.
 * @{
 */

/** Get the alloc fields list (count, limit, error). */
#define x_base_field_alloc_fields(X)		x_firstobj(x_restobj(x_restobj(x_base_field_meta_fields(X))))

/**
 * Get the allocated object-count field. Value: integer atom. Incremented
 * by x_obj_alloc and decremented by x_obj_free: the number of objects
 * currently allocated (counted once the base is set, so the base tree
 * itself is excluded).
 */
#define x_base_field_alloc_count(X)			x_firstobj(x_base_field_alloc_fields(X))

/**
 * Get the allocation ceiling field. Value: integer atom. 0 = unlimited (the
 * default). When > 0, x_obj_alloc stops the process rather than allocate
 * past it -- the runaway-memory guard.  Negative values are reserved: the
 * allocator latches the cell to -1 once tripped.
 */
#define x_base_field_alloc_limit(X)			x_firstobj(x_restobj(x_base_field_alloc_fields(X)))

/**
 * Get the allocation trip-message field. Value: an atom whose string is
 * reported (via the error hook, or stderr once latched) when the ceiling
 * trips, or nil to stop without reporting.  Supplied by the embedding layer
 * when it arms the limit -- x-expr itself holds no message text.
 */
#define x_base_field_alloc_error(X)			x_firstobj(x_restobj(x_restobj(x_base_field_alloc_fields(X))))

/** @} */

/**
 * Initialization parameters for creating a base object.
 *
 * Passed to x_base_make() to specify I/O descriptors, the slot vector's
 * contents, and heap configuration for a new base environment.
 */
struct x_base_t
{
	x_int_t filein;				/**< Input file descriptor (e.g. STDIN_FILENO). */
	x_int_t fileout;			/**< Output file descriptor (e.g. STDOUT_FILENO). */
	x_int_t fileerr;			/**< Error file descriptor (e.g. STDERR_FILENO). */

	x_int_t obj_meta_extra;		/**< Extra metadata units per object (0 for none). */

	/**
	 * The number of functions at @p p_slots. The slot vector is made this
	 * long, and never shorter than #X_SLOT_EXPR_LEN.
	 */
	x_int_t slots;

	/**
	 * The functions the slot vector starts with, in position order, or
	 * NULL to leave every slot empty. A NULL among them leaves that slot
	 * empty.
	 */
	const x_fn_t *p_slots;
};

/**
 * @name Base Functions
 * @{
 */

/** Create a new base environment object. */
x_obj_t *x_base_make(x_obj_t *p_base, struct x_base_t base);

/** Read data from the base's input file descriptor. */
x_obj_t *x_base_read(x_obj_t *p_base, x_obj_t *p_args);

/** Write data to the base's output file descriptor or write buffer. */
x_obj_t *x_base_write(x_obj_t *p_base, x_obj_t *p_args);

/** @} */

#endif /* X_BASE_H */
