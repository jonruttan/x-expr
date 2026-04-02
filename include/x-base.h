#ifndef X_BASE_H
#define X_BASE_H

/*
 * # Computational Expressions in C
 *
 * ## x-base.h -- Header - Base
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

/*
 * # The Base Object
 *
 * Every leaf field is a stack: `(current-value . saved-values)`.
 *
 * ```
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
 * ```
 */

/* TODO: Add name and version fields. */
#define x_base(X)							x_firstobj(X)
#define x_base_isset(B)						((B) != NULL && x_base((B)) != NULL)

/*
 * ## IO Group
 */
#define x_base_field_io_group(X)			x_firstobj(x_restobj(x_base(X)))
#define x_base_field_files(X)				x_restobj(x_firstobj(x_base_field_io_group(X)))
#define x_base_field_filein(X)				x_firstobj(x_base_field_files(X))
#define x_base_field_fileout(X)				x_firstobj(x_restobj(x_base_field_files(X)))
#define x_base_field_fileerr(X)				x_firstobj(x_restobj(x_restobj(x_base_field_files(X))))
#define x_base_field_write_buf(X)			x_firstobj(x_restobj(x_restobj(x_restobj(x_base_field_files(X)))))
#define x_base_field_buffer(X)				x_firstobj(x_restobj(x_restobj(x_restobj(x_restobj(x_base_field_files(X))))))

/*
 * ## Meta Group
 */
#define x_base_field_meta_group(X)			x_restobj(x_restobj(x_base(X)))

/* -- profile -- */
#define x_base_field_profile(X)				x_firstobj(x_firstobj(x_base_field_meta_group(X)))
#define x_base_field_profile_allocs(X)		x_firstobj(x_base_field_profile(X))

/* -- hooks -- */
#define x_base_field_hooks(X)				x_restobj(x_firstobj(x_base_field_meta_group(X)))
#define x_base_field_hook_type_name(X)		x_firstobj(x_base_field_hooks(X))
#define x_base_field_hook_units(X)			x_firstobj(x_restobj(x_base_field_hooks(X)))
#define x_base_field_hook_length(X)			x_firstobj(x_restobj(x_restobj(x_base_field_hooks(X))))
#define x_base_field_hook_error(X)			x_firstobj(x_restobj(x_restobj(x_restobj(x_base_field_hooks(X)))))

/*
 * ## Heap Group
 */
#define x_base_field_heap_group(X)			x_firstobj(x_restobj(x_base_field_meta_group(X)))
#define x_base_field_obj_meta_extra(X)		x_firstobj(x_base_field_heap_group(X))
#define x_base_field_heap_mark(X)			x_firstobj(x_restobj(x_base_field_heap_group(X)))
#define x_base_field_heap_free(X)			x_firstobj(x_restobj(x_restobj(x_base_field_heap_group(X))))
#define x_base_field_stack_base(X)			x_firstobj(x_restobj(x_restobj(x_restobj(x_base_field_heap_group(X)))))

/*
 * # Base Struct
 */
struct x_base_t
{
	/* io */
	x_int_t filein;
	x_int_t fileout;
	x_int_t fileerr;

	/* meta: hooks */
	x_obj_t *p_hook_type_name;
	x_obj_t *p_hook_units;
	x_obj_t *p_hook_length;
	x_obj_t *p_hook_error;

	/* heap */
	x_int_t obj_meta_extra;
	x_obj_t *p_heap_mark;
	x_obj_t *p_heap_free;
	void *p_stack_base;
};

/*
 * # Functions
 */
x_obj_t *x_base_make(x_obj_t *p_base, struct x_base_t base);
x_obj_t *x_base_read(x_obj_t *p_base, x_obj_t *p_args);
x_obj_t *x_base_write(x_obj_t *p_base, x_obj_t *p_args);

#endif /* X_BASE_H */
