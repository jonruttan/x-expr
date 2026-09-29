#ifndef X_SLOTS_H
#define X_SLOTS_H

/**
 * @file x-slots.h
 * @brief The base's function vector -- the routines the engine calls
 * through, each at a fixed position.
 *
 * The first data unit of a base object holds its @e slot @e vector: an
 * object whose data units are function pointers, one per slot. The engine
 * reaches a routine by its position in the vector, so a routine can be
 * replaced while the engine runs by storing another function pointer in
 * its slot.
 *
 * @details
 * A slot holds a function pointer and nothing more. Every slot function
 * has the engine's one signature, #x_fn_t:
 *
 * @code
 *   x_obj_t *fn(x_obj_t *p_base, x_obj_t *p_args);
 * @endcode
 *
 * and @p p_args is always an @e argument @e vector: an object whose data
 * units are the function's arguments, side by side. An argument that is an
 * integer travels as a plain word in its unit. The caller builds the
 * argument vector in stack storage (see x_slot_args()), so a call through
 * a slot allocates nothing.
 *
 * @code
 *   base                      slot vector              argument vector
 *   +---------+               +---------+              +---------+
 *   | header  |               | header  |              | header  |
 *   +---------+               +---------+              +---------+
 *   | slots   | ------------> | slot 0  |              | arg 0   |
 *   | tree    |               | slot 1  |              | arg 1   |
 *   +---------+               | ...     |              | ...     |
 *                             +---------+              +---------+
 * @endcode
 *
 * The positions are fixed and are part of the layout contract. x-expr owns
 * the positions below #X_SLOT_EXPR_LEN; an embedding layer numbers its own
 * slots from #X_SLOT_EXPR_LEN up and asks x_base_make() for a vector long
 * enough to hold them.
 *
 * A slot may be empty (NULL). What an empty slot means is the caller's to
 * say: the hook slots are optional and an empty one is skipped, while the
 * slots x-expr fills itself are never empty in a base x_base_make() made.
 *
 * @author Jon Ruttan (jonruttan@gmail.com)
 * @copyright 2026 Jon Ruttan
 * @license MIT No Attribution (MIT-0)
 *
 *         ., .,
 *         {O,O}
 *         (   )
 *          " "
 */

#include "x-obj.h"

/**
 * @name Slot Positions
 * @{
 */

/**
 * The positions x-expr owns in the slot vector.
 *
 * The first six are the hooks an embedding layer supplies; x-expr fills
 * the rest with its own routines.
 */
enum x_slot_enum
{
	/** Type-name hook. Arguments: (object). Returns the type object. */
	X_SLOT_TYPE_NAME = 0,

	/** Units hook. Arguments: (object). Returns an integer atom. */
	X_SLOT_UNITS,

	/** Length hook. Arguments: (object). Returns an integer atom. */
	X_SLOT_LENGTH,

	/** Error hook. Arguments: (message, object). Returns NULL. */
	X_SLOT_ERROR,

	/**
	 * Mark hook, for an object that is not a pair.
	 * Arguments: (object, flags). Returns the object to mark next, or
	 * NULL to stop at this branch.
	 */
	X_SLOT_HEAP_MARK,

	/** Free hook. Arguments: (object). Returns NULL. */
	X_SLOT_HEAP_FREE,

	/** x_obj_alloc(). Arguments: (type, flags, units). */
	X_SLOT_OBJ_ALLOC,

	/** x_obj_free(). Arguments: (object). Returns NULL. */
	X_SLOT_OBJ_FREE,

	/** x_heap_tree_mark(). Arguments: (object, flags). */
	X_SLOT_HEAP_TREE_MARK,

	/** x_heap_sweep(). Arguments: (object, flags). */
	X_SLOT_HEAP_SWEEP,

	/** x_heap_root_chain_mark(). Arguments: (flags). */
	X_SLOT_HEAP_ROOT_CHAIN_MARK,

	/** The number of slots x-expr owns, and the first position an
	 *  embedding layer may use. */
	X_SLOT_EXPR_LEN
};

/** @} */

/**
 * @name Slot Access
 * @{
 */

/**
 * The function pointer in slot @p I of slot vector @p V (an lvalue).
 */
#define x_slot(V,I)					x_fn(x_obj_data_i((V), (I)))

/** @} */

/**
 * @name Argument Vectors
 * @brief Build an argument vector in stack storage, and read one.
 * @{
 */

/**
 * The number of units of storage an argument vector of @p N arguments
 * takes: the metadata, then the arguments.
 *
 * @code
 *   x_obj_t args[x_slot_args_units(2)] = x_slot_args({ .p = p_obj }, { .i = flags });
 * @endcode
 */
#define x_slot_args_units(N)		(X_OBJ_META_LEN + (N))

/**
 * Brace initializer for an argument vector: each argument is given as a
 * datum initializer, in order.
 */
#define x_slot_args(...)			x_obj_set(NULL, X_OBJ_FLAG_NONE, __VA_ARGS__)

/** Argument @p I of argument vector @p A, as a raw datum (an lvalue). */
#define x_slot_arg(A,I)				x_obj_data_i((A), (I))

/** Argument @p I of argument vector @p A, as an object. */
#define x_slot_argobj(A,I)			x_obj(x_slot_arg((A), (I)))

/** Argument @p I of argument vector @p A, as an integer. */
#define x_slot_argint(A,I)			x_int(x_slot_arg((A), (I)))

/** Argument @p I of argument vector @p A, as a string. */
#define x_slot_argstr(A,I)			x_str(x_slot_arg((A), (I)))

/** @} */

/**
 * @name Slot Functions
 * @{
 */

/** Make a slot vector of @p length slots, every one empty. */
x_obj_t *x_slots_make(x_obj_t *p_base, x_int_t length);

/** Slot function for x_obj_alloc(). */
x_obj_t *x_slot_obj_alloc(x_obj_t *p_base, x_obj_t *p_args);

/** Slot function for x_obj_free(). */
x_obj_t *x_slot_obj_free(x_obj_t *p_base, x_obj_t *p_args);

#ifdef X_HEAP

/** Slot function for x_heap_tree_mark(). */
x_obj_t *x_slot_heap_tree_mark(x_obj_t *p_base, x_obj_t *p_args);

/** Slot function for x_heap_sweep(). */
x_obj_t *x_slot_heap_sweep(x_obj_t *p_base, x_obj_t *p_args);

/** Slot function for x_heap_root_chain_mark(). */
x_obj_t *x_slot_heap_root_chain_mark(x_obj_t *p_base, x_obj_t *p_args);

#endif /* X_HEAP */

/** @} */

#endif /* X_SLOTS_H */
