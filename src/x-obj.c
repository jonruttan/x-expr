/**
 * @file x-obj.c
 * @brief Implementation of the x-expr object system.
 *
 * Defines the built-in static type and value objects and the routines for
 * allocating, freeing, introspecting, and stack-manipulating objects. See
 * @ref x-obj.h for the object model and the primitive/wrapper function
 * convention.
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


/*
 * Built-in static objects, shared by all base environments:
 *   - the atom and pair type objects (their values are the type-name
 *     strings returned by x_obj_type_name());
 *   - the units/length atoms returned by the per-type units/length
 *     primitives;
 *   - the canonical #t / #f boolean atoms.
 */
x_satom_t x_type_atom_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)X_TYPE_ATOM_NAME}),
	x_type_pair_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)X_TYPE_PAIR_NAME}),
	x_type_units_atom_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = X_OBJ_UNITS_ATOM}),
	x_type_units_pair_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = X_OBJ_UNITS_PAIR}),
	x_type_length_atom_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = X_OBJ_LENGTH_ATOM}),
	x_type_length_pair_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = X_OBJ_LENGTH_PAIR}),
	x_true_obj  = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)X_OBJ_TRUE_TEXT}),
	x_false_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)X_OBJ_FALSE_TEXT});


/*
 * # Object Functions
 */
/**
 * Test whether an object is nil.
 *
 * nil is represented by a NULL object pointer. @p p_base is accepted for
 * calling-convention uniformity but is not used.
 *
 * @param p_base Base (execution context; unused).
 * @param p_obj  The object to test.
 * @return Non-zero if @p p_obj is nil (NULL), zero otherwise.
 */
int x_obj_isnil(x_obj_t *p_base, x_obj_t *p_obj)
{
	return p_obj == NULL;
}

/**
 * Allocate an uninitialized object.
 *
 * Allocates space for @p units data units plus the standard metadata header.
 * When the base configures extra metadata units (see
 * x_base_field_obj_meta_extra()), that many leading units plus one SIZE
 * ELEMENT are also reserved; the returned pointer is advanced past them,
 * the size element (x_obj_meta_size()) records the payload count, and
 * #X_OBJ_FLAG_META is set so x_obj_free() can recover the original
 * allocation from the object itself -- never from the base, whose
 * obj-meta-extra may have changed since (x-engine-c#21). When X_HEAP is enabled
 * the object is linked onto the base's heap chain (and, under X_PROFILE, the
 * allocation counter is incremented). The type and flags slots are set; the
 * data units are left uninitialized.
 *
 * @param p_base Base (execution context), or NULL to allocate without a base.
 * @param p_type The type object to assign, or NULL.
 * @param flags  Initial object flags.
 * @param units  Number of data units to allocate.
 * @return The new object, or NULL on allocation failure when no full
 *         base is attached (scratch/fixture use: the caller owns the
 *         null-check).  With a full base attached the failure does not
 *         return -- the runtime does not null-check allocations, so
 *         this function reports through the embedder-armed message
 *         (when set) and stops the process instead of handing back a
 *         NULL that would surface as a SIGSEGV in whatever code was
 *         running.
 * @note When the base arms an allocation ceiling (see
 *       x_base_field_alloc_limit()), reaching it makes this function report
 *       through the base error path and stop the process rather than
 *       allocate past it -- the runaway-memory guard.
 */
x_obj_t *x_obj_alloc(x_obj_t *p_base, x_obj_t *p_type, x_obj_flag_t flags, size_t units)
{
	x_obj_t *p_obj;
	/* Chasing base fields below requires a full, TYPED base -- the same
	 * predicate the obj-meta-extra fetch has always used.  x_base_isset
	 * alone is too weak: minimal/partial bases (x-expr test fixtures,
	 * embedder scratch objects) pass it and the field chase derefs
	 * garbage. */
	int base_full = (p_base != NULL
		&& ! x_obj_isnil(p_base, x_obj_type(p_base))
		&& x_base_isset(p_base));
	size_t extra = base_full
		? (size_t)x_atomint(x_firstobj(x_base_field_obj_meta_extra(p_base))) : 0;
	/* The prefix is the payload plus its size element -- the element is
	 * what lets x_obj_free() step back without consulting the (mutable)
	 * base field this width came from. */
	size_t prefix = extra > 0 ? extra + 1 : 0;
	/* Allocation ceiling (0 = disarmed, < 0 = already tripped) and the
	 * embedder-supplied trip message (nil = stop without reporting; x-expr
	 * holds no message text of its own). */
	x_int_t limit = base_full
		? x_atomint(x_firstobj(x_base_field_alloc_limit(p_base))) : 0;
	x_obj_t *p_msg = base_full
		? x_firstobj(x_base_field_alloc_error(p_base)) : NULL;

	if (limit < 0) {
		/* Ceiling already tripped once and the report was intercepted
		 * by an error handler (longjmp): an in-language handler cannot
		 * run without allocating, so re-reporting would longjmp forever
		 * (a zero-progress livelock).  Print directly and stop. */
		if ( ! x_obj_isnil(p_base, p_msg)) {
			x_error(STDERR_FILENO, x_atomstr(p_msg), NULL);
		}

		x_sys_exit(2);
	}

	/* Allocation ceiling (runaway-memory guard): report through the
	 * standard error path and stop rather than allocate past it.  Exit
	 * rather than return NULL: the runtime does not null-check
	 * allocations, so a NULL here segfaults (the allocation-failure
	 * path below has the same latent crash, just rarely reached).
	 * Latch (-1) BEFORE reporting: x_obj_error may longjmp to a guard
	 * handler and never return. */
	if (limit > 0 && x_atomint(x_firstobj(x_base_field_alloc_count(p_base))) >= limit) {
		x_atomint(x_firstobj(x_base_field_alloc_limit(p_base))) = -1;

		if ( ! x_obj_isnil(p_base, p_msg)) {
			x_obj_error(p_base, x_atomstr(p_msg), NULL);
		}

		x_sys_exit(2);
	}

	p_obj = (x_obj_t *)x_sys_malloc(sizeof(x_obj_t) * (prefix + X_OBJ_META_LEN + units));

	if (p_obj == NULL) {
		/* Real allocation failure, two contracts by context.  With a
		 * FULL base -- the running language -- the runtime does not
		 * null-check allocations, so returning NULL surfaced as a
		 * SIGSEGV wherever the caller happened to be standing (under
		 * machine-wide memory exhaustion that was deep inside
		 * JIT-compiled code, x-lang#201): report through the
		 * embedder-armed message when one exists (x-expr holds no
		 * prose of its own) and stop cleanly.  No x_obj_error here:
		 * an in-language guard handler cannot run without allocating,
		 * and allocation is precisely what just failed -- the same
		 * zero-progress argument as the tripped ceiling's latched
		 * branch above.  WITHOUT a full base -- x-expr test fixtures,
		 * embedder scratch use -- the historic NULL-return contract
		 * stands: those callers own their null-checks (and the
		 * allocation-failure spec pins exactly that). */
		if (base_full) {
			if ( ! x_obj_isnil(p_base, p_msg)) {
				x_error(STDERR_FILENO, x_atomstr(p_msg), NULL);
			}

			x_sys_exit(2);
		}

		return NULL;
	}

	/* Objects currently allocated; x_obj_free decrements. */
	if (base_full) {
		x_atomint(x_firstobj(x_base_field_alloc_count(p_base)))++;
	}

#ifdef X_HEAP
	if (extra > 0) {
		p_obj += prefix;
		/* Stamp the size element before anything can free this object:
		 * the prefix describes its own width from birth. */
		x_obj_meta_size(p_obj) = (x_int_t)extra;
		flags |= X_OBJ_FLAG_META;
	}

	if (p_base) {
		x_obj_heap(p_obj) = x_obj_heap(p_base);
		x_obj_heap(p_base) = p_obj;

#ifdef X_PROFILE
		if (x_base_isset(p_base)) {
			x_atomint(x_firstobj(x_base_field_profile_allocs(p_base)))++;
		}
#endif /* X_PROFILE */
	} else {
		x_obj_heap(p_obj) = NULL;
	}
#endif /* X_HEAP */

	x_obj_type(p_obj) = p_type;
	x_obj_flags(p_obj) = flags;

	return p_obj;
}

/**
 * Allocate an object and initialize its data units from a va_list.
 *
 * Calls x_obj_alloc(), then assigns @p units consecutive data units from
 * @p ap (each consumed as an x_obj_t *).
 *
 * @param p_base Base (execution context), or NULL.
 * @param p_type The type object to assign, or NULL.
 * @param flags  Initial object flags.
 * @param units  Number of data units to allocate and initialize.
 * @param ap     Variable argument list supplying the data units.
 * @return The new object, or NULL on allocation failure.
 */
x_obj_t *x_obj_make_va(x_obj_t *p_base, x_obj_t *p_type, x_obj_flag_t flags, size_t units, va_list ap)
{
	x_obj_t *p_obj = x_obj_alloc(p_base, p_type, flags, units);
	x_obj_t **p;

	if (p_obj == NULL) {
		return NULL;
	}

	p = &x_firstobj(p_obj);

	for (; units--; *p++ = va_arg(ap, x_obj_t *));

	return p_obj;
}

/**
 * Allocate an object and initialize its data units from varargs.
 *
 * Variadic wrapper around x_obj_make_va(); the x_mksatom() and x_mkspair()
 * macros build on it.
 *
 * @param p_base Base (execution context), or NULL.
 * @param p_type The type object to assign, or NULL.
 * @param flags  Initial object flags.
 * @param units  Number of data units, followed by that many x_obj_t * args.
 * @return The new object, or NULL on allocation failure.
 */
x_obj_t *x_obj_make(x_obj_t *p_base, x_obj_t *p_type, x_obj_flag_t flags, size_t units, ...)
{
	x_obj_t *p_obj;
	va_list ap;

	va_start(ap, units);
	p_obj = x_obj_make_va(p_base, p_type, flags, units, ap);
	va_end(ap);

	return p_obj;
}

/**
 * Free an object and any resources it owns.
 *
 * If the object has #X_OBJ_FLAG_OWN set, the allocation referenced by its
 * first datum is freed first. When X_HEAP is enabled and #X_OBJ_FLAG_META is
 * set, the original allocation address is recovered by stepping back over
 * the prefix, whose width the object's own size element records
 * (x_obj_meta_size()) -- the base's obj-meta-extra is mutable, so it is
 * never consulted here. This does not unlink the object
 * from the heap chain -- x_heap_sweep() relinks the chain around freed
 * objects.
 *
 * @param p_base Base (execution context; allocation accounting only).
 * @param p_obj  The object to free.
 */
void x_obj_free(x_obj_t *p_base, x_obj_t *p_obj)
{
	x_obj_t *p_alloc = p_obj;
	/* Full, typed base -- the only kind whose fields may be chased (see
	 * x_obj_alloc: x_base_isset alone admits minimal test/embedder bases
	 * whose "fields" are garbage). */
	int base_full = (p_base != NULL
		&& ! x_obj_isnil(p_base, x_obj_type(p_base))
		&& x_base_isset(p_base));

	/* Mirror the x_obj_alloc increment: every freed object (including GC
	 * sweeps, which free through here) decrements the allocated count. */
	if (base_full) {
		x_atomint(x_firstobj(x_base_field_alloc_count(p_base)))--;
	}

	if (x_obj_flags(p_obj) & X_OBJ_FLAG_OWN) {
		x_sys_free(x_firstobj(p_obj));
	}

#ifdef X_HEAP
	if (x_obj_flags(p_obj) & X_OBJ_FLAG_META) {
		/* Step back over THIS object's prefix: its payload units plus
		 * the size element that counted them. */
		p_alloc = p_obj - ((size_t)x_obj_meta_size(p_obj) + 1);
	}
#endif /* X_HEAP */

	x_sys_free(p_alloc);
}

/**
 * Primitive: resolve an object's type object.
 *
 * Follows the #x_fn_t convention: @p p_args is a pair whose first element is
 * the object to inspect. Built-in atoms and pairs, and any object with a nil
 * type slot, return their type slot directly; for other types the base's
 * type-name hook (x_base_field_hook_type_name()) is consulted.
 *
 * @param p_base Base (execution context).
 * @param p_args Pair list whose first element is the subject object.
 * @return The type object, or NULL if the subject is nil or no type is found.
 */
x_obj_t *x_obj_prim_type_name(x_obj_t *p_base, x_obj_t *p_args)
{
	x_obj_t *p_obj;

	if (x_obj_isnil(p_base, p_args) || x_obj_isnil(p_base, (p_obj = x_firstobj(p_args)))) {
		return NULL;
	}

	if (x_obj_type_issatom(p_obj)
			|| x_obj_type_isspair(p_obj)
			|| x_obj_isnil(p_base, x_obj_type(p_obj))) {
		return x_obj_type(p_obj);
	}

	if (x_base_isset(p_base)
		&& ! x_obj_isnil(p_base, x_firstobj(x_base_field_hook_type_name(p_base)))) {
		return x_atomfn(x_firstobj(x_base_field_hook_type_name(p_base)))(p_base, p_args);
	}

	return NULL;
}

/**
 * Get an object's type name as a C string.
 *
 * Builds the argument pair, calls x_obj_prim_type_name(), and unwraps the
 * resulting type object's string value.
 *
 * @param p_base Base (execution context).
 * @param p_obj  The object to inspect.
 * @return The type-name string, or #X_TYPE_NIL_NAME if the object has no
 *         resolvable type.
 */
x_char_t *x_obj_type_name(x_obj_t *p_base, x_obj_t *p_obj)
{
	x_obj_t *p_name;
	x_spair_t args = x_obj_set(NULL, X_OBJ_FLAG_NONE, { p_obj }, { p_base });

	p_name = x_obj_prim_type_name(p_base, (x_obj_t *)args);

	if (x_obj_isnil(p_base, p_name)) {
		return X_TYPE_NIL_NAME;
	}

	return x_atomstr(p_name);
}

/**
 * Primitive: the data-unit count of an atom.
 *
 * @param p_base Base (execution context; unused).
 * @param p_args Argument pair (unused).
 * @return The static atom #x_type_units_atom_obj (value #X_OBJ_UNITS_ATOM).
 */
x_obj_t *x_atom_prim_units(x_obj_t *p_base, x_obj_t *p_args)
{
	return x_type_units_atom_obj;
}

/**
 * Primitive: the data-unit count of a pair.
 *
 * @param p_base Base (execution context; unused).
 * @param p_args Argument pair (unused).
 * @return The static atom #x_type_units_pair_obj (value #X_OBJ_UNITS_PAIR).
 */
x_obj_t *x_pair_prim_units(x_obj_t *p_base, x_obj_t *p_args)
{
	return x_type_units_pair_obj;
}

/**
 * Primitive: dispatch an object's data-unit count by type.
 *
 * Returns x_pair_prim_units() for pairs and x_atom_prim_units() for atoms or
 * nil-typed objects; for other types the base's units hook
 * (x_base_field_hook_units()) is consulted.
 *
 * @param p_base Base (execution context).
 * @param p_args Pair list whose first element is the subject object.
 * @return An integer atom with the unit count, or NULL.
 */
x_obj_t *x_obj_prim_units(x_obj_t *p_base, x_obj_t *p_args)
{
	x_obj_t *p_obj;

	if (x_obj_isnil(p_base, p_args) || x_obj_isnil(p_base, (p_obj = x_firstobj(p_args)))) {
		return NULL;
	}

	if (x_obj_type_isspair(p_obj)) {
		return x_pair_prim_units(p_base, p_args);
	}

	if (x_obj_type_issatom(p_obj) || x_obj_isnil(p_base, x_obj_type(p_obj))) {
		return x_atom_prim_units(p_base, p_args);
	}

	if (x_base_isset(p_base)
		&& ! x_obj_isnil(p_base, x_firstobj(x_base_field_hook_units(p_base)))) {
		return x_atomfn(x_firstobj(x_base_field_hook_units(p_base)))(p_base, p_args);
	}

	return NULL;
}

/**
 * Get an object's data-unit count as an integer.
 *
 * Builds the argument pair, calls x_obj_prim_units(), and unwraps the result.
 *
 * @param p_base Base (execution context).
 * @param p_obj  The object to inspect.
 * @return The data-unit count, defaulting to #X_OBJ_UNITS_ATOM when unknown.
 */
x_int_t x_obj_units(x_obj_t *p_base, x_obj_t *p_obj)
{
	x_obj_t *p_units;
	x_spair_t args = x_obj_set(NULL, X_OBJ_FLAG_NONE, { p_obj }, { p_base });

	p_units = x_obj_prim_units(p_base, (x_obj_t *)args);

	if (x_obj_isnil(p_base, p_units)) {
		return X_OBJ_UNITS_ATOM;
	}

	return x_atomint(p_units);
}

/**
 * Primitive: the logical length of an atom.
 *
 * @param p_base Base (execution context; unused).
 * @param p_args Argument pair (unused).
 * @return The static atom #x_type_length_atom_obj (value #X_OBJ_LENGTH_ATOM).
 */
x_obj_t *x_atom_prim_length(x_obj_t *p_base, x_obj_t *p_args)
{
	return x_type_length_atom_obj;
}

/**
 * Primitive: the logical length of a pair.
 *
 * @param p_base Base (execution context; unused).
 * @param p_args Argument pair (unused).
 * @return The static atom #x_type_length_pair_obj (value #X_OBJ_LENGTH_PAIR).
 */
x_obj_t *x_pair_prim_length(x_obj_t *p_base, x_obj_t *p_args)
{
	return x_type_length_pair_obj;
}

/**
 * Primitive: dispatch an object's logical length by type.
 *
 * Returns x_pair_prim_length() for pairs and x_atom_prim_length() for atoms
 * or nil-typed objects; for other types the base's length hook
 * (x_base_field_hook_length()) is consulted.
 *
 * @param p_base Base (execution context).
 * @param p_args Pair list whose first element is the subject object.
 * @return An integer atom with the logical length, or NULL.
 */
x_obj_t *x_obj_prim_length(x_obj_t *p_base, x_obj_t *p_args)
{
	x_obj_t *p_obj;

	if (x_obj_isnil(p_base, p_args) || x_obj_isnil(p_base, (p_obj = x_firstobj(p_args)))) {
		return NULL;
	}

	if (x_obj_type_isspair(p_obj)) {
		return x_pair_prim_length(p_base, p_args);
	}

	if (x_obj_type_issatom(p_obj) || x_obj_isnil(p_base, x_obj_type(p_obj))) {
		return x_atom_prim_length(p_base, p_args);
	}

	if (x_base_isset(p_base)
		&& ! x_obj_isnil(p_base, x_firstobj(x_base_field_hook_length(p_base)))) {
		return x_atomfn(x_firstobj(x_base_field_hook_length(p_base)))(p_base, p_args);
	}

	return NULL;
}

/**
 * Get an object's logical length as an integer.
 *
 * Builds the argument pair, calls x_obj_prim_length(), and unwraps the result.
 *
 * @param p_base Base (execution context).
 * @param p_obj  The object to inspect.
 * @return The logical length, defaulting to 0 when unknown.
 */
x_int_t x_obj_length(x_obj_t *p_base, x_obj_t *p_obj)
{
	x_obj_t *p_length;
	x_spair_t args = x_obj_set(NULL, X_OBJ_FLAG_NONE, { p_obj }, { p_base });

	p_length = x_obj_prim_length(p_base, (x_obj_t *)args);

	if (x_obj_isnil(p_base, p_length)) {
		return 0;
	}

	return x_atomint(p_length);
}

/**
 * Push a value onto a pair-list stack.
 *
 * Replaces @p *p_field with a new pair `(p_value . old)`, so the field
 * becomes a stack whose head is @p p_value. Used to save and restore base
 * fields and to grow the heap hook/root lists.
 *
 * @param p_base  Base (execution context) for allocation.
 * @param p_field Address of the field to push onto (updated in place).
 * @param p_value The value to push.
 * @param flags   Flags for the new pair.
 * @return The pushed value (@p p_value).
 */
x_obj_t *x_obj_push_field(x_obj_t *p_base, x_obj_t **p_field, x_obj_t *p_value, x_obj_flag_t flags)
{
	*p_field = x_mkspair(p_base, flags, p_value, *p_field);

	return x_firstobj(*p_field);
}

/**
 * Pop the head value from a pair-list stack.
 *
 * Returns the current head (`x_firstobj(*p_field)`) and advances @p *p_field
 * to its rest. The popped pair is not freed; under X_HEAP the collector
 * reclaims it.
 *
 * @param p_base  Base (execution context; unused).
 * @param p_field Address of the field to pop from (updated in place).
 * @return The former head value.
 */
x_obj_t *x_obj_pop_field(x_obj_t *p_base, x_obj_t **p_field)
{
	x_obj_t *p_top = x_firstobj(*p_field);

	*p_field = x_restobj(*p_field);

	return p_top;
}

/**
 * Callable wrapper for x_obj_push_field().
 *
 * Follows the #x_fn_t convention. @p p_args is `(field-ptr value [flags])`
 * where field-ptr is an atom wrapping the `x_obj_t **` field address, value
 * is the object to push, and the optional third element is an integer atom
 * of flags (default #X_OBJ_FLAG_NONE).
 *
 * @param p_base Base (execution context).
 * @param p_args Argument pair list as described above.
 * @return The pushed value.
 */
x_obj_t *x_obj_push(x_obj_t *p_base, x_obj_t *p_args)
{
	x_obj_t **p_field = (x_obj_t **)x_atomptr(x_firstobj(p_args));
	x_obj_t *p_value = x_firstobj(x_restobj(p_args));
	x_obj_flag_t flags = x_obj_isnil(p_base, x_restobj(x_restobj(p_args)))
		? X_OBJ_FLAG_NONE
		: (x_obj_flag_t)x_atomint(x_firstobj(x_restobj(x_restobj(p_args))));

	return x_obj_push_field(p_base, p_field, p_value, flags);
}

/**
 * Callable wrapper for x_obj_pop_field().
 *
 * Follows the #x_fn_t convention. @p p_args is `(field-ptr)` where field-ptr
 * is an atom wrapping the `x_obj_t **` field address.
 *
 * @param p_base Base (execution context).
 * @param p_args Argument pair whose first element wraps the field address.
 * @return The popped value.
 */
x_obj_t *x_obj_pop(x_obj_t *p_base, x_obj_t *p_args)
{
	x_obj_t **p_field = (x_obj_t **)x_atomptr(x_firstobj(p_args));

	return x_obj_pop_field(p_base, p_field);
}

/**
 * Output an error message to stderr.
 *
 * If the error hook is set in the base, delegates to it via a cast
 * to `void (*)(x_obj_t *, x_char_t *, x_obj_t *)` -- this differs from
 * x_fn_t because error handlers take raw arguments (not a pair list).
 * Otherwise, extracts the object's string text (if it is a static
 * atom) and calls x_error().
 *
 * @param p_base  Base (execution context).
 * @param message Error message string.
 * @param p_obj   Related object for context, or NULL.
 */
void x_obj_error(x_obj_t *p_base, x_char_t *message, x_obj_t *p_obj)
{
	x_char_t *p_text = NULL;

	if (x_base_isset(p_base)
		&& ! x_obj_isnil(p_base, x_firstobj(x_base_field_hook_error(p_base)))) {
		((void (*)(x_obj_t *, x_char_t *, x_obj_t *))
			x_firstptr(x_firstobj(x_base_field_hook_error(p_base))))(p_base, message, p_obj);
		return;
	}

	if (p_obj != NULL && x_obj_type_issatom(p_obj)) {
		p_text = x_atomstr(p_obj);
	}

	x_error(STDERR_FILENO, message, p_text);
}


#ifdef DEBUG

/**
 * @internal
 * Write a formatted debug message with a va_list (see x_obj_debug_va()).
 *
 * Delegates to _x_debug_va() on STDERR. @p p_base is accepted for a uniform
 * macro signature but is not used.
 *
 * @param file   Source file name (__FILE__).
 * @param line   Source line number (__LINE__).
 * @param p_base Base (execution context; unused).
 * @param fmt    printf-style format string.
 * @param ap     Variable argument list.
 */
void _x_obj_debug_va(char *file, long unsigned line, x_obj_t *p_base, char *fmt, va_list ap)
{
	int fd = STDERR_FILENO;

	_x_debug_va(file, line, fd, fmt, ap);
}

/**
 * @internal
 * Write a formatted debug message with varargs (see x_obj_debug()).
 *
 * @param file   Source file name (__FILE__).
 * @param line   Source line number (__LINE__).
 * @param p_base Base (execution context).
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
void _x_obj_debug(char *file, long unsigned line, x_obj_t *p_base, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	_x_obj_debug_va(file, line, p_base, fmt, ap);
	va_end(ap);
}

/**
 * @internal
 * Write a one-line dump of an object (see x_obj_dump()).
 *
 * Formats the object's type name, flags (as binary), address, heap link
 * (under X_HEAP), and data, prefixed by @p msg, then emits it via
 * _x_obj_debug(). Atoms print their integer datum; pairs print their first
 * and rest data as `[first, rest]`.
 *
 * @param file   Source file name (__FILE__).
 * @param line   Source line number (__LINE__).
 * @param p_base Base (execution context).
 * @param p_obj  The object to dump.
 * @param msg    A label prefix for the output.
 */
void _x_obj_dump(char *file, long unsigned line, x_obj_t *p_base, x_obj_t *p_obj, char *msg)
{
	x_char_t *type = x_obj_type_name(p_base, p_obj);
	char data_buffer[X_DEBUG_BUFFER_SIZE], *s = "",
		flag_buffer[(sizeof(x_obj_flag_t) << 3) + 1], *flags = "-";

	if (p_obj != NULL) {
		/* Convert object flags to a string and skip leading zeros */
		flags = x_lib_strchr(x_lib_inttostr(x_obj_flags(p_obj), flag_buffer, 2), '1');

		if (flags == NULL) {
			flags = flag_buffer + x_lib_strlen(flag_buffer) - 1;
		}
	}


	if ( ! (x_obj_isnil(p_base, p_obj) || x_obj_isnil(p_base, x_obj_type(p_obj)))) {
		s = data_buffer;
		s += sprintf(s, ":[");

		if (x_obj_type_isspair(p_obj)) {
			if (x_obj_isnil(p_base, x_firstobj(p_obj))) {
				s += sprintf(s, X_TYPE_NIL_NAME);
			} else {
				s += sprintf(s, "0x%"X_INT_STR_PRINTF_CONV"x", x_atomint(x_firstobj(p_obj)));
			}

			if (x_obj_isnil(p_base, x_restobj(p_obj))) {
				s += sprintf(s, ", "X_TYPE_NIL_NAME);
			} else {
				s += sprintf(s, ", 0x%"X_INT_STR_PRINTF_CONV"x", x_atomint(x_restobj(p_obj)));
			}
		} else {
			s += sprintf(s, "0x%"X_INT_STR_PRINTF_CONV"x", x_atomint(p_obj));
		}

		s += sprintf(s, "]");
		s = data_buffer;
	}

	_x_obj_debug(file, line, p_base, "%s[%s:%s][%p] "
#ifdef X_HEAP
		"HEAP[%p]"
#endif /* X_HEAP */
		"%s"
		, msg ? (char *)msg : ""
		, type
		, flags
		, p_obj
#ifdef X_HEAP
		, p_obj ? x_obj_heap(p_obj) : NULL
#endif /* X_HEAP */
		, s
	);
}

#else /* DEBUG */

#endif /* DEBUG */
