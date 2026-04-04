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
 * Every leaf field is a stack: `(current-value . saved-values)`.
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
 */

/*
 *     ., .,
 *     {O,O}
 *     (   )
 *      " "
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

/** Get the filein field (standard input file descriptor stack). */
#define x_base_field_filein(X)				x_firstobj(x_base_field_files(X))

/** Get the fileout field (standard output file descriptor stack). */
#define x_base_field_fileout(X)				x_firstobj(x_restobj(x_base_field_files(X)))

/** Get the fileerr field (standard error file descriptor stack). */
#define x_base_field_fileerr(X)				x_firstobj(x_restobj(x_restobj(x_base_field_files(X))))

/** Get the write buffer field (buffered output pointer + position). */
#define x_base_field_write_buf(X)			x_firstobj(x_restobj(x_restobj(x_restobj(x_base_field_files(X)))))

/** Get the input buffer field. */
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

/** Get the allocation counter field. */
#define x_base_field_profile_allocs(X)		x_firstobj(x_base_field_profile(X))

/** Get the hooks list (type-name, units, length, error). */
#define x_base_field_hooks(X)				x_restobj(x_firstobj(x_base_field_meta_group(X)))

/** Get the type_name hook field (called to resolve custom type names). */
#define x_base_field_hook_type_name(X)		x_firstobj(x_base_field_hooks(X))

/** Get the units hook field (called to determine custom type unit counts). */
#define x_base_field_hook_units(X)			x_firstobj(x_restobj(x_base_field_hooks(X)))

/** Get the length hook field (called to determine custom type lengths). */
#define x_base_field_hook_length(X)			x_firstobj(x_restobj(x_restobj(x_base_field_hooks(X))))

/** Get the error hook field (called for error handling). */
#define x_base_field_hook_error(X)			x_firstobj(x_restobj(x_restobj(x_restobj(x_base_field_hooks(X)))))

/** @} */

/**
 * @defgroup base_heap Base Heap Field Accessors
 * @brief Navigate the heap/GC section of the base object tree.
 * @{
 */

/** Get the heap group pair. */
#define x_base_field_heap_group(X)			x_firstobj(x_restobj(x_base_field_meta_group(X)))

/** Get the extra metadata units count field. */
#define x_base_field_obj_meta_extra(X)		x_firstobj(x_base_field_heap_group(X))

/** Get the heap mark function hook field. */
#define x_base_field_heap_mark(X)			x_firstobj(x_restobj(x_base_field_heap_group(X)))

/** Get the heap free function hook field. */
#define x_base_field_heap_free(X)			x_firstobj(x_restobj(x_restobj(x_base_field_heap_group(X))))

/** Get the C call stack base pointer field (for GC stack scanning). */
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

	x_obj_t *p_hook_type_name;	/**< Type name resolution hook, or NULL. */
	x_obj_t *p_hook_units;		/**< Units count hook, or NULL. */
	x_obj_t *p_hook_length;		/**< Length hook, or NULL. */
	x_obj_t *p_hook_error;		/**< Error handling hook, or NULL. */

	x_int_t obj_meta_extra;		/**< Number of extra metadata units per object. */
	x_obj_t *p_heap_mark;		/**< Heap mark function hook, or NULL. */
	x_obj_t *p_heap_free;		/**< Heap free function hook, or NULL. */
	void *p_stack_base;			/**< C call stack base address for GC scanning. */
};

/**
 * @name Base Functions
 * @{
 */

/**
 * Create a new base environment object.
 *
 * Allocates and assembles the nested pair tree described in the file
 * header from the given initialization parameters.
 *
 * @param p_base Existing base for allocation context, or NULL for bootstrap.
 * @param base   Initialization parameters.
 * @return The new base object.
 */
x_obj_t *x_base_make(x_obj_t *p_base, struct x_base_t base);

/**
 * Read data from the base's input file descriptor.
 *
 * @param p_base The base environment.
 * @param p_args Pair list: (destination-atom byte-count).
 * @return The destination atom on success, or NULL on failure.
 */
x_obj_t *x_base_read(x_obj_t *p_base, x_obj_t *p_args);

/**
 * Write data to the base's output file descriptor or write buffer.
 *
 * If the base has a write buffer set, data is copied there. Otherwise
 * it is written to the output file descriptor.
 *
 * @param p_base The base environment.
 * @param p_args Pair list: (source-atom byte-count).
 * @return The source atom on success, or NULL on failure.
 */
x_obj_t *x_base_write(x_obj_t *p_base, x_obj_t *p_args);

/** @} */

#endif /* X_BASE_H */
