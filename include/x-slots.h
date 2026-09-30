#ifndef X_SLOTS_H
#define X_SLOTS_H

/**
 * @file x-slots.h
 * @brief The base's function vector -- the routines the engine calls
 * through, each at a fixed position.
 *
 * The first data unit of a base object holds its @e slot @e vector: a
 * vector (see @ref x-vector.h) whose elements are function pointers, one
 * per slot. The engine reaches a routine by its position in the vector,
 * so a routine can be replaced while the engine runs by storing another
 * function pointer in its slot.
 *
 * @details
 * A slot holds a function pointer and nothing more. Every routine in a
 * slot has the engine's one signature, #x_fn_t:
 *
 * @code
 *   x_obj_t *fn(x_obj_t *p_base, x_obj_t *p_args);
 * @endcode
 *
 * and @p p_args is the routine's @e argument @e run: a run of datum words,
 * one per argument, in the order the slot's row in the layout contract
 * gives them. A word holds what the argument is: an object pointer, an
 * integer, a string. The run has no header and no length; the contract
 * says how many words there are and what each holds. The caller writes
 * the words, in stack storage, and the routine reads them back:
 *
 * @code
 *   x_obj_t args[2] = { { .p = p_env }, { .p = p_sym } };
 *
 *   p = x_base_call(p_base, X_SLOT_ENV_LOOKUP, args);
 *   ...
 *   p_env = x_obj(p_args[0]);
 *   p_sym = x_obj(p_args[1]);
 * @endcode
 *
 * A call through a slot allocates nothing. The elements of a heap vector
 * are a run of words too, so a caller that holds its arguments in a
 * vector passes the address of its first element.
 *
 * @code
 *   base                      slot vector              argument run
 *   +---------+               +---------+              +---------+
 *   | header  |               | header  |              | arg 0   |
 *   +---------+               +---------+              | arg 1   |
 *   | slots   | ------------> | length  |              | ...     |
 *   | tree    |               | slot 0  |              +---------+
 *   +---------+               | slot 1  |
 *                             | ...     |
 *                             +---------+
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
#include "x-vector.h"

/**
 * @name Slot Positions
 * @{
 */

/**
 * The positions x-expr owns in the slot vector.
 *
 * The first six are the hooks an embedding layer supplies; x-expr fills
 * the rest with its own routines, each of which takes the argument run
 * itself. An embedding layer replaces one by naming another function at
 * its position in the table it hands x_base_make().
 *
 * Each slot's comment gives its arguments and the type of the word each
 * travels in: an object, an integer or a string.
 */
enum x_slot_enum
{
	/** Type-name hook. Arguments: (object). Types: (object).
	 *  Returns the type object. */
	X_SLOT_TYPE_NAME = 0,

	/** Units hook. Arguments: (object). Types: (object).
	 *  Returns an integer atom. */
	X_SLOT_UNITS,

	/** Length hook. Arguments: (object). Types: (object).
	 *  Returns an integer atom. */
	X_SLOT_LENGTH,

	/** Error hook. Arguments: (message, object). Types: (string, object).
	 *  Returns NULL. */
	X_SLOT_ERROR,

	/**
	 * Mark hook, for an object that is not a pair.
	 * Arguments: (object, flags). Types: (object, integer).
	 * Returns the object to mark next, or NULL to stop at this branch.
	 */
	X_SLOT_HEAP_MARK,

	/** Free hook. Arguments: (object). Types: (object). Returns NULL. */
	X_SLOT_HEAP_FREE,

	/** Allocate an object. Arguments: (type, flags, units).
	 *  Types: (object, integer, integer). */
	X_SLOT_OBJ_ALLOC,

	/** Free an object. Arguments: (object). Types: (object).
	 *  Returns NULL. */
	X_SLOT_OBJ_FREE,

	/** Mark a tree. Arguments: (object, flags). Types: (object, integer). */
	X_SLOT_HEAP_TREE_MARK,

	/** Sweep the heap. Arguments: (object, flags). Types: (object, integer). */
	X_SLOT_HEAP_SWEEP,

	/** Mark the root chain. Arguments: (flags). Types: (integer). */
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
#define x_slot(V,I)					x_vectorfn((V), (I))

/**
 * An argument run written as an expression: the datum initializers of its
 * words, in order. It lasts as long as the block the expression is in.
 *
 * @code
 *   x_base_call(p_base, X_SLOT_OBJ_FREE, x_argrun({ .p = p_obj }));
 * @endcode
 */
#define x_argrun(...)				((x_obj_t[]){ __VA_ARGS__ })

/** Make a slot vector of @p length slots, every one empty. */
x_obj_t *x_slots_make(x_obj_t *p_base, x_int_t length);

/** @} */

#endif /* X_SLOTS_H */
