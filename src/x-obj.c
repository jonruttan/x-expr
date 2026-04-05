/**
 * @file x-obj.c
 * @brief Implementation of the core object system.
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


/** @name Global Type and Constant Objects
 *
 * Statically initialized atoms used as type descriptors and constants.
 * @{ */
x_satom_t x_type_atom_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)X_TYPE_ATOM_SYMBOL}),
	x_type_pair_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)X_TYPE_PAIR_SYMBOL}),
	x_type_units_atom_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = X_OBJ_UNITS_ATOM}),
	x_type_units_pair_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = X_OBJ_UNITS_PAIR}),
	x_type_length_atom_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = X_OBJ_LENGTH_ATOM}),
	x_type_length_pair_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = X_OBJ_LENGTH_PAIR}),
	x_true_obj  = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)X_OBJ_TRUE_SYMBOL}),
	x_false_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)X_OBJ_FALSE_SYMBOL});
/** @} */


/**
 * Test whether an object is nil (NULL).
 *
 * @param p_base The base environment (unused, reserved for hook dispatch).
 * @param p_obj  The object to test.
 * @return Non-zero if @p p_obj is NULL, zero otherwise.
 */
int x_obj_isnil(x_obj_t *p_base, x_obj_t *p_obj)
{
	return p_obj == NULL;
}

/**
 * Allocate an uninitialized object.
 *
 * Allocates `sizeof(x_obj_t) * (extra + META_LEN + units)` bytes where
 * @p extra is the obj_meta_extra count from the base. When extra > 0,
 * the returned pointer is advanced past the extension region and the
 * META flag is set so x_obj_free() can adjust back. With X_HEAP enabled,
 * the new object is prepended to the heap chain via `x_obj_heap()` and
 * the profile allocation counter is incremented.
 *
 * @param p_base The base environment (for extra metadata size and heap chain).
 * @param p_type Type descriptor pointer to store in metadata.
 * @param flags  Initial flags for the new object.
 * @param units  Number of data units to allocate.
 * @return Pointer to the new object's metadata, or NULL on failure.
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
 * Create an object with data units from a va_list.
 *
 * Allocates via x_obj_alloc() then fills @p units data slots from @p ap.
 *
 * @param p_base The base environment.
 * @param p_type Type descriptor pointer.
 * @param flags  Initial flags.
 * @param units  Number of data units.
 * @param ap     Variable argument list of x_obj_t * values for each data unit.
 * @return Pointer to the new object, or NULL on allocation failure.
 */
x_obj_t *x_obj_make_va(x_obj_t *p_base, x_obj_t *p_type, x_obj_flag_t flags, size_t units, va_list ap)
{
	x_obj_t *p_obj = x_obj_alloc(p_base, p_type, flags, units);
	x_obj_t **p = &x_firstobj(p_obj);

	for (; units--; *p++ = va_arg(ap, x_obj_t *));

	return p_obj;
}

/**
 * Create an object with data units from variadic arguments.
 *
 * Convenience wrapper that calls x_obj_make_va().
 *
 * @param p_base The base environment.
 * @param p_type Type descriptor pointer.
 * @param flags  Initial flags.
 * @param units  Number of data units.
 * @param ...    Data unit values (as x_obj_t *).
 * @return Pointer to the new object, or NULL on allocation failure.
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
 * Free an object and its owned data.
 *
 * If the OWN flag is set, the first data unit's pointer is freed first.
 * If the META flag is set, the allocation pointer is adjusted back past
 * the extra metadata units before freeing.
 *
 * @param p_base The base environment.
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
 * Primitive: get the type name object for an object.
 *
 * For built-in atoms and pairs, returns the type descriptor directly.
 * For user-defined types, delegates to the type_name hook in the base.
 *
 * @param p_base The base environment.
 * @param p_args Pair list whose first element is the object to inspect.
 * @return The type name atom, or NULL for nil objects.
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
 * Get the type name string for an object.
 *
 * Wraps x_obj_prim_type_name() to return a C string directly.
 *
 * @param p_base The base environment.
 * @param p_obj  The object to inspect.
 * @return The type name as a C string (e.g. "ATOM", "PAIR"), or "NIL".
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
 * Primitive: return the units count for an atom.
 *
 * @param p_base The base environment (unused).
 * @param p_args Unused.
 * @return The atom units constant object (X_OBJ_UNITS_ATOM).
 */
x_obj_t *x_atom_prim_units(x_obj_t *p_base, x_obj_t *p_args)
{
	return x_type_units_atom_obj;
}

/**
 * Primitive: return the units count for a pair.
 *
 * @param p_base The base environment (unused).
 * @param p_args Unused.
 * @return The pair units constant object (X_OBJ_UNITS_PAIR).
 */
x_obj_t *x_pair_prim_units(x_obj_t *p_base, x_obj_t *p_args)
{
	return x_type_units_pair_obj;
}

/**
 * Primitive: return the units count for any object.
 *
 * Dispatches to x_atom_prim_units() or x_pair_prim_units() for
 * built-in types. For user-defined types, delegates to the units hook.
 *
 * @param p_base The base environment.
 * @param p_args Pair list whose first element is the object.
 * @return The units constant object, or NULL for nil.
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
 * Get the number of data units for an object.
 *
 * Wraps x_obj_prim_units() and extracts the integer value.
 *
 * @param p_base The base environment.
 * @param p_obj  The object to measure.
 * @return The unit count, defaulting to X_OBJ_UNITS_ATOM for nil.
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
 * Primitive: return the length constant for an atom.
 *
 * @param p_base The base environment (unused).
 * @param p_args Unused.
 * @return The atom length constant object (X_OBJ_LENGTH_ATOM).
 */
x_obj_t *x_atom_prim_length(x_obj_t *p_base, x_obj_t *p_args)
{
	return x_type_length_atom_obj;
}

/**
 * Primitive: return the length constant for a pair.
 *
 * @param p_base The base environment (unused).
 * @param p_args Unused.
 * @return The pair length constant object (X_OBJ_LENGTH_PAIR).
 */
x_obj_t *x_pair_prim_length(x_obj_t *p_base, x_obj_t *p_args)
{
	return x_type_length_pair_obj;
}

/**
 * Primitive: return the length for any object.
 *
 * Dispatches to x_atom_prim_length() or x_pair_prim_length() for
 * built-in types. For user-defined types, delegates to the length hook.
 *
 * @param p_base The base environment.
 * @param p_args Pair list whose first element is the object.
 * @return The length constant object, or NULL for nil.
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
 * Get the logical length of an object.
 *
 * Wraps x_obj_prim_length() and extracts the integer value.
 *
 * @param p_base The base environment.
 * @param p_obj  The object to measure.
 * @return The length, or 0 for nil.
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
 * Push a value onto a field stack.
 *
 * A field stack is a pair chain: `(current . (prev . (prev2 . nil)))`.
 * Push creates `(value . *field)` and writes it back to `*field`,
 * saving the old value in the rest chain. Use this to temporarily
 * override a base field (e.g. redirect I/O) and restore it later
 * with x_obj_pop(). The first argument is a void pointer to the
 * field's `x_obj_t *` variable (one level of indirection).
 *
 * @param p_base The base environment.
 * @param p_args Pair list: (field-pointer value [flags]).
 * @return The pushed value.
 */
x_obj_t *x_obj_push(x_obj_t *p_base, x_obj_t *p_args)
{
	x_obj_t **p_field = (x_obj_t **)x_atomptr(x_firstobj(p_args));
	x_obj_t *p_value = x_firstobj(x_restobj(p_args));
	x_obj_flag_t flags = x_obj_isnil(p_base, x_restobj(x_restobj(p_args)))
		? X_OBJ_FLAG_NONE
		: (x_obj_flag_t)x_atomint(x_firstobj(x_restobj(x_restobj(p_args))));

	*p_field = x_mkspair(p_base, flags, p_value, *p_field);

	return x_firstobj(*p_field);
}

/**
 * Pop a value from a field stack.
 *
 * Removes the top pair from `*field` and returns its first element.
 *
 * @param p_base The base environment.
 * @param p_args Pair list: (field-pointer).
 * @return The popped value.
 */
x_obj_t *x_obj_pop(x_obj_t *p_base, x_obj_t *p_args)
{
	x_obj_t **p_field = (x_obj_t **)x_atomptr(x_firstobj(p_args));
	x_obj_t *p_top = x_firstobj(*p_field);

	*p_field = x_restobj(*p_field);

	return p_top;
}

/**
 * Output an error message to stderr and exit.
 *
 * If the error hook is set in the base, delegates to it via a cast
 * to `void (*)(x_obj_t *, x_char_t *, x_obj_t *)` -- this differs from
 * x_fn_t because error handlers take raw arguments (not a pair list)
 * and may not return (they typically exit). Otherwise, extracts a string symbol
 * from the object (if it is a static atom) and calls x_error().
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
 * Write a formatted debug message using the object system's base.
 */
void _x_obj_debug_va(char *file, long unsigned line, x_obj_t *p_base, char *fmt, va_list ap)
{
	int fd = STDERR_FILENO;

	_x_debug_va(file, line, fd, fmt, ap);
}

/**
 * @internal
 * Write a formatted debug message using the object system's base.
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
 * Dump an object's type, flags, and data for debugging.
 *
 * Outputs a formatted line showing the object's type name, flag bits
 * (in binary), memory address, and data contents.
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
