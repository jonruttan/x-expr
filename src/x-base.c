/**
 * @file x-base.c
 * @brief Implementation of the base environment object.
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

/** @internal Shorthand for NULL used in the base tree construction. */
#define nil			NULL
/** @internal Shorthand for creating a shared pair in the base tree. */
#define pair(X,Y)	(x_mkspair(p_base, X_OBJ_FLAG_SHARED, (X), (Y)))
/** @internal Shorthand for creating a shared atom in the base tree. */
#define atom(X)		(x_mksatom(p_base, X_OBJ_FLAG_SHARED, (X)))

/**
 * Make a slot vector of @p length slots, every one empty.
 *
 * The vector is an object with no type, so the collector does not look
 * inside it, and it is allocated with X_OBJ_FLAG_SHARED, so a sweep keeps
 * it for as long as the base lives.
 *
 * @param p_base Base the vector belongs to (allocation context).
 * @param length The number of slots.
 * @return The slot vector, or NULL when it could not be allocated.
 */
x_obj_t *x_slots_make(x_obj_t *p_base, x_int_t length)
{
	x_obj_t *p_slots;
	x_int_t i;

	p_slots = x_obj_alloc(p_base, NULL, X_OBJ_FLAG_SHARED, (size_t)length);

	if (p_slots == NULL) {
		return NULL;
	}

	for (i = 0; i < length; i++) {
		x_slot(p_slots, i) = NULL;
	}

	return p_slots;
}

/**
 * Create a new base environment object.
 *
 * The base has two data units. The first holds the slot vector, made
 * here and filled from @p base (see x-slots.h). The second holds the
 * nested pair tree with the rest of the environment's state.
 *
 * Every node of the tree is allocated with X_OBJ_FLAG_SHARED (via the
 * local pair/atom macros) so the base tree is immune to garbage
 * collection. Each leaf value is wrapped as `pair(atom(value), nil)` to
 * form a one-element field stack (see x-base.h). The tree layout matches
 * the field accessor macros: x_base_field_filein() navigates to the
 * filein leaf, x_base_field_obj_meta_extra() to the metadata width, etc.
 *
 * @param p_base Existing base for allocation context, or NULL for bootstrap.
 * @param base   Initialization parameters (file descriptors, slots, etc.).
 * @return The newly created base object.
 */
x_obj_t *x_base_make(x_obj_t *p_base, struct x_base_t base)
{
	x_int_t length, i;

	p_base = x_obj_make(p_base, NULL, X_OBJ_FLAG_NONE,
		X_OBJ_LENGTH_PAIR, NULL, NULL);

	/* The slot vector: as long as the caller asked for, and never too
	 * short for the positions x-expr owns. */
	length = base.slots > X_SLOT_EXPR_LEN ? base.slots : X_SLOT_EXPR_LEN;
	x_base_slots(p_base) = x_slots_make(p_base, length);

	if (base.p_slots != NULL) {
		for (i = 0; i < base.slots; i++) {
			x_base_slot(p_base, i) = base.p_slots[i];
		}
	}

	x_base(p_base) = pair(
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
				/* meta: profile, heap, alloc
				 * -- tail past alloc extended by the embedding layer */
				pair(
					/* profile: allocs */
					pair(pair(atom(0), nil),
					nil),
					nil),
				pair(
					/* heap: obj-meta-extra
					 * + mark-hooks, free-hooks, mark-roots (extensible
					 *   lists, grown at runtime via x_heap_*_add)
					 * + root-chain (head of the precise stack-root chain,
					 *   pushed/popped by frames via x_heap_root_push/pop). */
					pair(pair(atom(base.obj_meta_extra), nil),
					pair(pair(nil, nil),
					pair(pair(nil, nil),
					pair(pair(nil, nil),
					pair(pair(nil, nil),
					nil))))),
				/* alloc fields: count, limit, error -- objects currently
				 * allocated (x_obj_alloc increments, x_obj_free decrements),
				 * the ceiling x_obj_alloc enforces (0 = unlimited), and the
				 * embedder-supplied message atom reported when the ceiling
				 * trips (nil = stop without reporting; x-expr holds no
				 * message text).  Allocation accounting, independent of
				 * X_HEAP garbage collection.  The tail past these fields is
				 * the embedding layer's extension point (x-eval builds its
				 * state fields there). */
				pair(
					pair(pair(atom(0), nil),
					pair(pair(atom(0), nil),
					pair(pair(nil, nil),
					nil))),
				nil)))));

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
 *
 * @param p_base Base (execution context).
 * @param p_args Pair list: (destination-atom byte-count).
 * @return The destination atom on success, or NULL on read failure.
 *         On failure the byte-count atom is overwritten with the raw
 *         x_sys_read() result so the caller can tell end-of-stream (0)
 *         from a read error (negative): this layer has no error
 *         channel, and conflating the two turned transient read errors
 *         into silent input truncation (x-lang#170).
 */
x_obj_t *x_base_read(x_obj_t *p_base, x_obj_t *p_args)
{
	int fd = x_base_isset(p_base)
		? x_atomint(x_firstobj(x_base_field_filein(p_base)))
		: STDIN_FILENO;
	x_obj_t *p_atom = x_firstobj(p_args);
	x_int_t size = x_atomint(x_firstobj(x_restobj(p_args)));
	ssize_t result = x_sys_read(fd, &x_atomchar(p_atom), size);

	if (result == size) {
		return p_atom;
	}

	x_atomint(x_firstobj(x_restobj(p_args))) = result;

	return NULL;
}

/**
 * Write data from an atom to the base's output or write buffer.
 *
 * Checks the write_buf field first: if it holds a non-nil pair
 * `(pointer . position)`, data is memcpy'd into the buffer at the
 * current position and the position is advanced. Otherwise, writes
 * to the fileout file descriptor, falling back to STDOUT_FILENO
 * if the base is not initialized.
 *
 * @param p_base Base (execution context).
 * @param p_args Pair list: (source-atom byte-count).
 * @return The source atom on success, or NULL on write failure.
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
