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
x_satom_t x_type_atom_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)X_TYPE_ATOM_SYMBOL}),
	x_type_pair_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)X_TYPE_PAIR_SYMBOL}),
	x_type_units_atom_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = X_OBJ_UNITS_ATOM}),
	x_type_units_pair_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = X_OBJ_UNITS_PAIR}),
	x_type_length_atom_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = X_OBJ_LENGTH_ATOM}),
	x_type_length_pair_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = X_OBJ_LENGTH_PAIR}),
	x_true_obj  = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)X_OBJ_TRUE_SYMBOL}),
	x_false_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)X_OBJ_FALSE_SYMBOL});


/*
 * # Object Functions
 */
/**
 * Test whether an object is nil.
 *
 * nil is represented by a NULL object pointer. @p p_base is accepted for
 * calling-convention uniformity but is not used.
 *
 * @param p_base The base environment (unused).
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
 * x_base_field_obj_meta_extra()), that many leading units are also reserved;
 * the returned pointer is advanced past them and #X_OBJ_FLAG_META is set so
 * x_obj_free() can recover the original allocation. When X_HEAP is enabled
 * the object is linked onto the base's heap chain (and, under X_PROFILE, the
 * allocation counter is incremented). The type and flags slots are set; the
 * data units are left uninitialized.
 *
 * @param p_base The base environment, or NULL to allocate without a base.
 * @param p_type The type object to assign, or NULL.
 * @param flags  Initial object flags.
 * @param units  Number of data units to allocate.
 * @return The new object, or NULL on allocation failure (after reporting
 *         #X_OBJ_ERROR_OOM via x_obj_error() when @p p_base is non-NULL).
 */
x_obj_t *x_obj_alloc(x_obj_t *p_base, x_obj_t *p_type, x_obj_flag_t flags, size_t units)
{
	x_obj_t *p_obj;
	size_t extra = (p_base != NULL
		&& ! x_obj_isnil(p_base, x_obj_type(p_base))
		&& x_base_isset(p_base))
		? (size_t)x_atomint(x_firstobj(x_base_field_obj_meta_extra(p_base))) : 0;

	p_obj = (x_obj_t *)x_sys_malloc(sizeof(x_obj_t) * (extra + X_OBJ_META_LEN + units));

	if (p_obj == NULL) {
		if (p_base != NULL) {
			x_obj_error(p_base, (x_char_t *)X_OBJ_ERROR_OOM, NULL);
		}
		return NULL;
	}

#ifdef X_HEAP
	if (extra > 0) {
		p_obj += extra;
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
 * @param p_base The base environment, or NULL.
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
 * @param p_base The base environment, or NULL.
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
 * the extra metadata units before freeing. This does not unlink the object
 * from the heap chain -- x_heap_sweep() relinks the chain around freed
 * objects.
 *
 * @param p_base The base environment (used to size extra metadata units).
 * @param p_obj  The object to free.
 */
void x_obj_free(x_obj_t *p_base, x_obj_t *p_obj)
{
	x_obj_t *p_alloc = p_obj;

	if (x_obj_flags(p_obj) & X_OBJ_FLAG_OWN) {
		x_sys_free(x_firstobj(p_obj));
	}

#ifdef X_HEAP
	if (x_obj_flags(p_obj) & X_OBJ_FLAG_META) {
		size_t extra = (p_base != NULL
			&& !x_obj_isnil(p_base, x_obj_type(p_base))
			&& x_base_isset(p_base))
			? (size_t)x_atomint(x_firstobj(x_base_field_obj_meta_extra(p_base))) : 0;
		p_alloc = p_obj - extra;
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
 * @param p_base The base environment.
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
 * @param p_base The base environment.
 * @param p_obj  The object to inspect.
 * @return The type-name string, or #X_TYPE_NIL_SYMBOL if the object has no
 *         resolvable type.
 */
x_char_t *x_obj_type_name(x_obj_t *p_base, x_obj_t *p_obj)
{
	x_obj_t *p_name;
	x_spair_t args = x_obj_set(NULL, X_OBJ_FLAG_NONE, { p_obj }, { p_base });

	p_name = x_obj_prim_type_name(p_base, (x_obj_t *)args);

	if (x_obj_isnil(p_base, p_name)) {
		return X_TYPE_NIL_SYMBOL;
	}

	return x_atomstr(p_name);
}

/**
 * Primitive: the data-unit count of an atom.
 *
 * @param p_base The base environment (unused).
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
 * @param p_base The base environment (unused).
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
 * @param p_base The base environment.
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
 * @param p_base The base environment.
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
 * @param p_base The base environment (unused).
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
 * @param p_base The base environment (unused).
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
 * @param p_base The base environment.
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
 * @param p_base The base environment.
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
 * @param p_base  The base environment (allocation context).
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
 * @param p_base  The base environment (unused).
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
 * @param p_base The base environment.
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
 * @param p_base The base environment.
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
 * Otherwise, extracts a string symbol from the object (if it is a
 * static atom) and calls x_error().
 *
 * @param p_base  The base environment.
 * @param message Error message string.
 * @param p_obj   Related object for context, or NULL.
 */
void x_obj_error(x_obj_t *p_base, x_char_t *message, x_obj_t *p_obj)
{
	x_char_t *symbol = NULL;

	if (x_base_isset(p_base)
		&& ! x_obj_isnil(p_base, x_firstobj(x_base_field_hook_error(p_base)))) {
		((void (*)(x_obj_t *, x_char_t *, x_obj_t *))
			x_firstptr(x_firstobj(x_base_field_hook_error(p_base))))(p_base, message, p_obj);
		return;
	}

	if (p_obj != NULL && x_obj_type_issatom(p_obj)) {
		symbol = x_atomstr(p_obj);
	}

	x_error(STDERR_FILENO, message, symbol);
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
 * @param p_base The base environment (unused).
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
 * @param p_base The base environment.
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
 * @param p_base The base environment.
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
				s += sprintf(s, X_TYPE_NIL_SYMBOL);
			} else {
				s += sprintf(s, "0x%"X_INT_STR_PRINTF_CONV"x", x_atomint(x_firstobj(p_obj)));
			}

			if (x_obj_isnil(p_base, x_restobj(p_obj))) {
				s += sprintf(s, ", "X_TYPE_NIL_SYMBOL);
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
