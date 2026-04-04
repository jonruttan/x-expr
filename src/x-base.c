/**
 * @file x-base.c
 * @brief Implementation of the base environment object.
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

/** @internal Shorthand for NULL used in the base tree construction. */
#define nil			NULL
/** @internal Shorthand for creating a shared pair in the base tree. */
#define pair(X,Y)	(x_mkspair(p_base, X_OBJ_FLAG_SHARED, (X), (Y)))
/** @internal Shorthand for creating a shared atom in the base tree. */
#define atom(X)		(x_mksatom(p_base, X_OBJ_FLAG_SHARED, (X)))

/**
 * Create a new base environment object.
 *
 * Allocates and assembles the nested pair tree that holds all environment
 * state: I/O descriptors, profiling counters, hook functions, and heap
 * management pointers. See x-base.h for the tree structure.
 */
x_obj_t *x_base_make(x_obj_t *p_base, struct x_base_t base)
{
	p_base = x_obj_make(p_base, NULL, X_OBJ_FLAG_NONE,
		X_OBJ_LENGTH_ATOM, NULL);
	x_atomobj(p_base) = pair(
		/* env+ctrl (x project extends) */
		nil,
		pair(
			/* io: (type-alist . files), io-state */
			pair(
				pair(
					/* type-alist (x project extends) */
					nil,
					/* files: filein, fileout, fileerr, write-buf, buffer */
					pair(pair(atom(base.filein), nil),
					pair(pair(atom(base.fileout), nil),
					pair(pair(atom(base.fileerr), nil),
					pair(pair(nil, nil),
					pair(pair(nil, nil),
					nil)))))),
				/* io-state (x project extends) */
				nil),
			pair(
				/* meta: (profile . hooks), (heap . ()) */
				pair(
					/* profile: allocs */
					pair(pair(atom(0), nil),
					nil),
					/* hooks: type-name, units, length, error */
					pair(pair(base.p_hook_type_name, nil),
					pair(pair(base.p_hook_units, nil),
					pair(pair(base.p_hook_length, nil),
					pair(pair(base.p_hook_error, nil),
					nil))))),
				pair(
					/* heap: obj-meta-extra, mark, free, stack-base */
					pair(pair(atom(base.obj_meta_extra), nil),
					pair(pair(base.p_heap_mark, nil),
					pair(pair(base.p_heap_free, nil),
					pair(pair(atom(base.p_stack_base), nil),
					nil)))),
				nil))));

	return p_base;
}

#undef nil
#undef pair
#undef atom

/**
 * Read data from the base's input file descriptor into an atom.
 *
 * Uses the filein descriptor from the base environment, or falls back
 * to STDIN_FILENO if the base is not initialized.
 */
x_obj_t *x_base_read(x_obj_t *p_base, x_obj_t *p_args)
{
	int fd = x_base_isset(p_base)
		? x_atomint(x_firstobj(x_base_field_filein(p_base)))
		: STDIN_FILENO;
	x_obj_t *p_atom = x_firstobj(p_args);
	x_int_t size = x_atomint(x_firstobj(x_restobj(p_args)));

	if (x_sys_read(fd, &x_atomchar(p_atom), size) == size) {
		return p_atom;
	}

	return NULL;
}

/**
 * Write data from an atom to the base's output or write buffer.
 *
 * If the base has an active write buffer, data is copied into it.
 * Otherwise, writes to the fileout descriptor from the base environment,
 * falling back to STDOUT_FILENO if the base is not initialized.
 */
x_obj_t *x_base_write(x_obj_t *p_base, x_obj_t *p_args)
{
	x_obj_t *p_atom = x_firstobj(p_args), *p_buf;
	x_int_t size = x_atomint(x_firstobj(x_restobj(p_args))), pos;
	x_char_t *dst;
	int fd;

	if (x_base_isset(p_base)) {
		p_buf = x_firstobj(x_base_field_write_buf(p_base));

		if ( ! x_obj_isnil(p_base, p_buf)) {
			dst = (x_char_t *)x_firstptr(p_buf);
			pos = x_atomint(x_restobj(p_buf));

			x_lib_memcpy(dst + pos, x_atomstr(p_atom), size);
			x_atomint(x_restobj(p_buf)) = pos + size;

			return p_atom;
		}
	}

	fd = x_base_isset(p_base)
		? x_atomint(x_firstobj(x_base_field_fileout(p_base)))
		: STDOUT_FILENO;

	if (x_sys_write(fd, x_atomstr(p_atom), size) == size) {
		return p_atom;
	}

	return NULL;
}
