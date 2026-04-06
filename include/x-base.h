#ifndef X_BASE_H
#define X_BASE_H

/**
 * @file x-base.h
 * @brief Base environment object -- the root context for evaluation.
 *
 * The base object is a nested pair tree that holds I/O descriptors,
 * profiling counters, extensible hook functions, and heap/GC state.
 * Field accessor macros navigate this tree without hard-coding offsets.
 *
 * @details
 * Every leaf field is stored as a pair: `(current-value . saved-values)`.
 * Fields are initialized with `(value . nil)`. Any field can be used as
 * a stack via x_obj_push() / x_obj_pop() to save and restore values.
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
 *           (; profile . hooks
 *             (allocs)
 *             . (type-name units length error))
 *           . ((; heap
 *               (obj-meta-extra mark free stack-base)
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

/**
 * @name Base Object Root
 * @{
 */

/* TODO: Add name and version fields. */

/** Get the root data of the base object @p X. */
#define x_base(X)							x_firstobj(X)

/** Test whether the base object @p B is initialized and has data. */
#define x_base_isset(B)						((B) != NULL && x_base((B)) != NULL)

/** @} */

/**
 * @defgroup base_io Base IO Field Accessors
 * @brief Navigate the I/O section of the base object tree.
 * @{
 */

/** Get the IO group pair (type-alist . files, io-state). */
#define x_base_field_io_group(X)			x_firstobj(x_restobj(x_base(X)))

/** Get the files list (filein, fileout, fileerr, write-buf, buffer). */
#define x_base_field_files(X)				x_restobj(x_firstobj(x_base_field_io_group(X)))

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
 * @brief Navigate the metadata section (profile + hooks) of the base tree.
 * @{
 */

/** Get the meta group pair (profile+hooks, heap). */
#define x_base_field_meta_group(X)			x_restobj(x_restobj(x_base(X)))

/** Get the profile pair (counters). */
#define x_base_field_profile(X)				x_firstobj(x_firstobj(x_base_field_meta_group(X)))

/** Get the allocation counter field. Value: integer atom, incremented by x_obj_alloc(). */
#define x_base_field_profile_allocs(X)		x_firstobj(x_base_field_profile(X))

/**
 * Get the hooks list (type-name, units, length, error).
 *
 * Hooks are only dispatched for objects whose type pointer is NOT one
 * of the built-in static types (x_type_atom_obj, x_type_pair_obj).
 * Dispatch priority: check static type (pointer identity, O(1)),
 * then call the hook if set, then fall back to NULL.
 */
#define x_base_field_hooks(X)				x_restobj(x_firstobj(x_base_field_meta_group(X)))

/**
 * Get the type_name hook field. Value: atom with x_fn_t function pointer.
 * Signature: `x_obj_t *(*)(x_obj_t *p_base, x_obj_t *p_args)` --
 * p_args is a pair list whose first element is the object to name.
 * Must return a type name atom (string data), or NULL.
 */
#define x_base_field_hook_type_name(X)		x_firstobj(x_base_field_hooks(X))

/**
 * Get the units hook field. Value: atom with x_fn_t function pointer.
 * Same calling convention as type_name. Must return an integer atom
 * with the number of data units, or NULL.
 */
#define x_base_field_hook_units(X)			x_firstobj(x_restobj(x_base_field_hooks(X)))

/**
 * Get the length hook field. Value: atom with x_fn_t function pointer.
 * Same calling convention as type_name. Must return an integer atom
 * with the logical length, or NULL.
 */
#define x_base_field_hook_length(X)			x_firstobj(x_restobj(x_restobj(x_base_field_hooks(X))))

/**
 * Get the error hook field. Value: atom with void pointer to function.
 *
 * @note Unlike the other hooks, the error hook does NOT follow the
 * x_fn_t signature. Its actual signature is:
 * `void (*)(x_obj_t *p_base, x_char_t *message, x_obj_t *p_obj)`.
 * It is dispatched via a cast from x_firstptr(), not x_atomfn().
 */
#define x_base_field_hook_error(X)			x_firstobj(x_restobj(x_restobj(x_restobj(x_base_field_hooks(X)))))

/** @} */

/**
 * @defgroup base_heap Base Heap Field Accessors
 * @brief Navigate the heap/GC section of the base object tree.
 * @{
 */

/** Get the heap group pair. */
#define x_base_field_heap_group(X)			x_firstobj(x_restobj(x_base_field_meta_group(X)))

/**
 * Get the extra metadata units count field. Value: integer atom.
 * When > 0, x_obj_alloc() prepends this many extra units before each
 * object's standard metadata, accessible via x_obj_meta_i().
 */
#define x_base_field_obj_meta_extra(X)		x_firstobj(x_base_field_heap_group(X))

/** Get the heap mark hook field. Value: atom with x_heap_mark_fn_t pointer. */
#define x_base_field_heap_mark(X)			x_firstobj(x_restobj(x_base_field_heap_group(X)))

/** Get the heap free hook field. Value: atom with x_heap_free_fn_t pointer. */
#define x_base_field_heap_free(X)			x_firstobj(x_restobj(x_restobj(x_base_field_heap_group(X))))

/**
 * Get the C call stack base pointer field. Value: atom with void pointer.
 * Should be captured near program entry (e.g. `&argc` in main) and
 * passed via x_base_t.p_stack_base. Used by x_heap_callstack_mark()
 * to determine the stack region to scan for object references.
 */
#define x_base_field_stack_base(X)			x_firstobj(x_restobj(x_restobj(x_restobj(x_base_field_heap_group(X)))))

/** @} */

/**
 * Initialization parameters for creating a base object.
 *
 * Passed to x_base_make() to specify I/O descriptors, hook functions,
 * and heap configuration for a new base environment.
 */
struct x_base_t
{
	x_int_t filein;				/**< Input file descriptor (e.g. STDIN_FILENO). */
	x_int_t fileout;			/**< Output file descriptor (e.g. STDOUT_FILENO). */
	x_int_t fileerr;			/**< Error file descriptor (e.g. STDERR_FILENO). */

	x_obj_t *p_hook_type_name;	/**< Type name hook (x_fn_t atom), or NULL. */
	x_obj_t *p_hook_units;		/**< Units hook (x_fn_t atom), or NULL. */
	x_obj_t *p_hook_length;		/**< Length hook (x_fn_t atom), or NULL. */
	x_obj_t *p_hook_error;		/**< Error hook (void * atom, different signature), or NULL. */

	x_int_t obj_meta_extra;		/**< Extra metadata units per object (0 for none). */
	x_obj_t *p_heap_mark;		/**< Mark hook (x_heap_mark_fn_t atom), or NULL. */
	x_obj_t *p_heap_free;		/**< Free hook (x_heap_free_fn_t atom), or NULL. */
	void *p_stack_base;			/**< C stack base address for GC scanning (e.g. `&main_local`). */
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
