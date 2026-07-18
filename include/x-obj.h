#ifndef X_OBJ_H
#define X_OBJ_H

/**
 * @file x-obj.h
 * @brief The x-expr object system -- atoms, pairs, flags, and type dispatch.
 *
 * Defines the universal object representation used throughout x-expr along
 * with the macros and functions for creating, inspecting, and traversing
 * objects.
 *
 * @details
 * Every value is an @b object: a contiguous array of @ref x_obj_t units (a
 * tagged-union datum, see @ref x_datum_union) laid out as a small metadata
 * header followed by one or more data units. A pointer to an object
 * addresses the start of its metadata; the data units begin at offset
 * #X_OBJ_META_LEN, reached via the accessor macros x_obj_type(),
 * x_obj_flags() and x_obj_data_ptr().
 *
 * There are two fundamental shapes:
 * - **atom** -- one data unit (#X_OBJ_UNITS_ATOM): a single datum such as an
 *   integer, character, string, pointer, or function pointer.
 * - **pair** -- two data units (#X_OBJ_UNITS_PAIR): a @e first and a @e rest.
 *   Pairs are the building block for lists and trees.
 *
 * `nil` is represented by a `NULL` object pointer (see x_obj_isnil()).
 *
 * @code
 *   atom                          pair
 *   +---------+                   +---------+
 *   | heap    | (X_HEAP only)     | heap    | (X_HEAP only)
 *   | type    |                   | type    |
 *   | flags   |                   | flags   |
 *   +---------+ <- data start     +---------+ <- data start
 *   | datum   | first             | first   |
 *   +---------+                   | rest    |
 *                                 +---------+
 * @endcode
 *
 * When the base configures extra metadata units (see
 * x_base_field_obj_meta_extra()), x_obj_alloc() prepends them before the
 * standard header and advances the object pointer past them, so the layout
 * above still holds; the extras sit at negative offsets reached via
 * x_obj_meta_i() and the object is flagged #X_OBJ_FLAG_META.
 *
 * **Type system.** An object carries type information two ways:
 * -# The @e type metadata slot (x_obj_type()) points at a type object whose
 *    atom value is the type-name string. The built-in static types
 *    #x_type_atom_obj and #x_type_pair_obj are matched by pointer identity
 *    (x_obj_type_issatom(), x_obj_type_isspair()); for any other type the
 *    base environment's hooks are consulted (see x_obj_prim_type_name()).
 * -# A lightweight @e simple type tag may be carried in the type nibble of
 *    the object flags (#X_OBJ_FLAG_PRIM ... #X_OBJ_FLAG_PTR) for consumers
 *    that want a cheap datum-kind tag without allocating a type object.
 *    The x-expr core does not dispatch on it; it is provided for downstream
 *    use.
 *
 * **Primitive vs. wrapper functions.** Type-introspection operations come in
 * two forms. The `x_*_prim_*` functions follow the @ref x_fn_t calling
 * convention -- `(p_base, p_args)` where `p_args` is a pair whose first
 * element is the subject object -- so they can be stored and dispatched as
 * callables (e.g. installed as base hooks). The matching non-`prim`
 * wrappers (x_obj_type_name(), x_obj_units(), x_obj_length()) take a plain
 * object pointer, build the argument pair, invoke the primitive, and unwrap
 * the result for C callers.
 *
 * @note See @ref x-base.h for how an object's type/units/length/error hooks
 * are stored in the base tree, and @ref x-lisp.h for Lisp-style
 * `cons`/`car`/`cdr` aliases over these pair operations.
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
/*
 * # Includes
 */
#include "x-lib.h"

/**
 * @name Type Names and Literal Text
 *
 * All of these are plain C strings; x-expr has no interned-symbol type
 * (that concept lives in the embedding layer), so nothing here is a
 * "symbol".
 * @{
 */

/** Type-name string returned for nil (untyped) objects. */
#define X_TYPE_NIL_NAME				"NIL"
/** Type-name string for the built-in atom type (#x_type_atom_obj). */
#define X_TYPE_ATOM_NAME			"ATOM"
/** Type-name string for the built-in pair type (#x_type_pair_obj). */
#define X_TYPE_PAIR_NAME			"PAIR"

/** Literal text of the canonical true atom (#x_true_obj). */
#define X_OBJ_TRUE_TEXT				"#t"
/** Literal text of the canonical false atom (#x_false_obj). */
#define X_OBJ_FALSE_TEXT			"#f"

/** @} */

/*
 * # Data Structures
 */

/**
 * Object flags -- a bitfield stored in each object's flags metadata slot.
 *
 * The low nibble (#X_OBJ_FLAG_ATTR_MASK) holds general-purpose attribute
 * bits. The high nibble (#X_OBJ_FLAG_TYPE_MASK) encodes an optional
 * "simple type" tag. The remaining bits are storage- and GC-related.
 *
 * @note The simple-type enumeration (#X_OBJ_FLAG_PRIM ... #X_OBJ_FLAG_PTR)
 * and the attribute bits share the low-order bits, so they are alternative
 * interpretations rather than independently combinable. The storage bits
 * #X_OBJ_FLAG_OWN, #X_OBJ_FLAG_RO and #X_OBJ_FLAG_META -- and, when X_HEAP
 * is enabled, #X_OBJ_FLAG_SHARED and #X_OBJ_FLAG_MARK -- are independent
 * and may be OR-combined.
 */
typedef enum x_obj_flag_enum
{
	/** No flags set. */
	X_OBJ_FLAG_NONE=0x0,

	/* Object type system flags. */
	/** Full object typed via the type pointer; the default kind. */
	X_OBJ_FLAG_OBJ=0x0,

	/** General-purpose attribute bit 1 (application defined). */
	X_OBJ_FLAG_1=0x1,
	/** General-purpose attribute bit 2 (application defined). */
	X_OBJ_FLAG_2=0x2,
	/** General-purpose attribute bit 3 (application defined). */
	X_OBJ_FLAG_3=0x4,
	/** General-purpose attribute bit 4 (application defined). */
	X_OBJ_FLAG_4=0x8,

	/** Mask selecting the four attribute bits. */
	X_OBJ_FLAG_ATTR_MASK=0xF,

	/* Simple type system enumeration 0x10 - 0x1F. */
	/** Base value of the simple-type enumeration. */
	X_OBJ_FLAG_SIMPLE_TYPE=0x10,
	/** Simple type: primitive / special form. */
	X_OBJ_FLAG_PRIM=0x10,
	/** Simple type: function pointer (x_fn_t). */
	X_OBJ_FLAG_FN,
	/** Simple type: integer (x_int_t). */
	X_OBJ_FLAG_INT,
	/** Simple type: character (x_char_t). */
	X_OBJ_FLAG_CHAR,
	/** Simple type: string (x_char_t *). */
	X_OBJ_FLAG_STR,
	/** Simple type: opaque pointer (void *). */
	X_OBJ_FLAG_PTR,

	/** Mask selecting the simple-type nibble. */
	X_OBJ_FLAG_TYPE_MASK=0xF0,

	/** Object owns the allocation in its first datum; x_obj_free() releases it. */
	X_OBJ_FLAG_OWN=0x20,
	/** Read-only attribute (advisory; not enforced by the core). */
	X_OBJ_FLAG_RO=0x40,
	/** Object has extra metadata units prepended (see x_obj_meta_i()). */
	X_OBJ_FLAG_META=0x80,

#ifndef X_HEAP
	/** Mask of all defined flag bits. */
	X_OBJ_FLAG_MASK=0xFF
#else /* X_HEAP */
	/** Permanently retained across garbage collection (e.g. base-tree nodes). */
	X_OBJ_FLAG_SHARED=0x100,
	/** Conventional mark bit used by the mark-sweep collector. */
	X_OBJ_FLAG_MARK=0x200,

	/** Mask of all defined flag bits. */
	X_OBJ_FLAG_MASK=0x3FF
#endif /* X_HEAP */
} x_obj_flag_t;

/** A single object datum and the unit type from which objects are built. */
typedef union x_datum_union x_obj_t;

/**
 * Calling convention for x-expr primitive functions.
 *
 * @param p_base Base (execution context).
 * @param p_args A pair list of arguments (nil when there are none).
 * @return The result object, or NULL.
 */
typedef x_obj_t * (*x_fn_t)(x_obj_t *p_base, x_obj_t *p_args);

/**
 * The fundamental datum -- one unit of an object's metadata or data.
 *
 * A datum is reinterpreted through the member matching its content; the
 * x_obj(), x_fn(), x_int(), x_char(), x_str() and x_ptr() macros select the
 * appropriate member.
 */
union x_datum_union
{
	x_obj_t *p;		/**< Object pointer (e.g. type slot, pair element). */
	x_fn_t fn;		/**< Primitive function pointer. */
	x_int_t i;		/**< Signed integer (e.g. flags slot). */
	x_char_t c;		/**< Character. */
	x_char_t *s;	/**< Null-terminated string. */
	void *v;		/**< Opaque pointer. */
};


/**
 * @name Object Layout
 * @brief Metadata sizing and the offsets of an object's data units.
 * @{
 */

#ifdef X_HEAP
/** Metadata units consumed by the heap-chain link (1 when X_HEAP, else 0). */
#define X_OBJ_UNITS_HEAP			1
#else /* X_HEAP */
/** Metadata units consumed by the heap-chain link (1 when X_HEAP, else 0). */
#define X_OBJ_UNITS_HEAP			0
#endif /* X_HEAP */

#ifndef X_OBJ_UNITS_TYPE
/** Metadata units consumed by the type slot. */
#define X_OBJ_UNITS_TYPE			1
#endif /* X_OBJ_UNITS_TYPE */

#ifndef X_OBJ_UNITS_FLAGS
/** Metadata units consumed by the flags slot. */
#define X_OBJ_UNITS_FLAGS			1
#endif /* X_OBJ_UNITS_FLAGS */

/** Indices of the metadata units; X_OBJ_META_LEN is where data begins. */
enum {
#ifdef X_HEAP
	X_OBJ_META_HEAP = 0,
#endif /* X_HEAP */
	X_OBJ_META_TYPE = X_OBJ_UNITS_HEAP,
	X_OBJ_META_FLAGS = (X_OBJ_UNITS_HEAP + X_OBJ_UNITS_TYPE),
	X_OBJ_META_LEN = (X_OBJ_UNITS_HEAP + X_OBJ_UNITS_TYPE + X_OBJ_UNITS_FLAGS)
};

#ifndef X_OBJ_UNITS_META
/** Total number of standard metadata units preceding an object's data. */
#define X_OBJ_UNITS_META			X_OBJ_META_LEN
#endif /* X_OBJ_UNITS_META */

/** Number of data units in an atom. */
#define X_OBJ_UNITS_ATOM			1
/** Number of data units in a pair. */
#define X_OBJ_UNITS_PAIR			2

/** Logical length of an atom. */
#define X_OBJ_LENGTH_ATOM			X_OBJ_UNITS_ATOM
/** Logical length of a pair. */
#define X_OBJ_LENGTH_PAIR			X_OBJ_UNITS_PAIR


/**
 * Fixed-size storage for a statically allocated atom (metadata + 1 unit).
 *
 * @note The `s` prefix carries two senses in this header. Here (and in
 * #x_spair_t) it is @e storage: room for an object in static or stack
 * storage instead of the heap. In x_obj_type_issatom() /
 * x_obj_type_isspair() and in the x_mksatom() / x_mkspair() constructor
 * family it is the built-in @e static @e type (#x_type_atom_obj /
 * #x_type_pair_obj) -- a property of the type slot, independent of where
 * the object lives.
 */
typedef x_obj_t x_satom_t[X_OBJ_META_LEN + X_OBJ_UNITS_ATOM];
/** Fixed-size storage for a statically allocated pair (metadata + 2 units;
 *  see the `s`-prefix note at #x_satom_t). */
typedef x_obj_t x_spair_t[X_OBJ_META_LEN + X_OBJ_UNITS_PAIR];

/** Built-in static atom type; its value is the "ATOM" type-name string. */
extern x_satom_t x_type_atom_obj;
/** Built-in static pair type; its value is the "PAIR" type-name string. */
extern x_satom_t x_type_pair_obj;
/** Static atom holding the atom data-unit count, returned by x_atom_prim_units(). */
extern x_satom_t x_type_units_atom_obj;
/** Static atom holding the pair data-unit count, returned by x_pair_prim_units(). */
extern x_satom_t x_type_units_pair_obj;
/** Static atom holding the atom length, returned by x_atom_prim_length(). */
extern x_satom_t x_type_length_atom_obj;
/** Static atom holding the pair length, returned by x_pair_prim_length(). */
extern x_satom_t x_type_length_pair_obj;

/** Canonical true atom (value #X_OBJ_TRUE_TEXT). */
extern x_satom_t x_true_obj;
/** Canonical false atom (value #X_OBJ_FALSE_TEXT). */
extern x_satom_t x_false_obj;

/** @} */

/**
 * @name Metadata Accessors
 * @brief Read or assign an object's metadata slots and locate its data.
 *
 * Each macro is an lvalue, so it may appear on either side of an assignment.
 * @{
 */

#ifdef X_HEAP
/** Heap-chain link to the next object (X_HEAP only). */
#define x_obj_heap(X)				((X)[X_OBJ_META_HEAP].p)
#endif /* X_HEAP */

/** The object's type slot: a pointer to its type object, or NULL (nil). */
#define x_obj_type(X)				((X)[X_OBJ_META_TYPE].p)

/** The object's flags slot (an #x_obj_flag_t bitfield held as an integer). */
#define x_obj_flags(X)				((X)[X_OBJ_META_FLAGS].i)

/** Pointer to the first data unit of object @p X. */
#define x_obj_data_ptr(X)			((X) + X_OBJ_META_LEN)
/** The data unit at index @p I of object @p X. */
#define x_obj_data_i(X,I)			(x_obj_data_ptr((X))[(I)])
/** The first data unit of object @p X. */
#define x_obj_data(X)				x_obj_data_i((X), 0)

/** The extra metadata unit at index @p I (negative offset before @p X). */
#define x_obj_meta_i(X, I)			((X)[-((I) + 1)])

#ifdef X_HEAP
/** Brace initializer for a static object: type @p T, flags @p F, then data. */
#define x_obj_set(T,F,...)			{ { .v = NULL }, { .p = (T) }, { .i = (F) }, __VA_ARGS__ }
#else /* X_HEAP */
/** Brace initializer for a static object: type @p T, flags @p F, then data. */
#define x_obj_set(T,F,...)			{ { .p = (T) }, { .i = (F) }, __VA_ARGS__ }
#endif /* X_HEAP */

/** @} */

/**
 * @name Type Predicates and Constructors
 *
 * The `s` in `issatom`/`isspair` and `x_mks*` is the built-in @e static
 * @e type sense (see the note at #x_satom_t): the predicates match
 * #x_type_atom_obj / #x_type_pair_obj by pointer identity, and the
 * constructors heap-allocate objects carrying those types -- `s` says
 * nothing about where the object is stored.
 * @{
 */

/** True if object @p X has a nil (NULL) type slot. */
#define x_obj_type_isnil(B,X)		x_obj_isnil((B), x_obj_type(X))
/** True if the type name of object @p X equals the C string @p T. */
#define x_obj_is_type(B,X,T)		(x_lib_strcmp(x_obj_type_name((B), (X)), (T)) == 0)

/** True if @p X is typed as the built-in atom type (by pointer identity). */
#define x_obj_type_issatom(X)		(x_obj_type((X)) == x_type_atom_obj)
/** True if @p X is typed as the built-in pair type (by pointer identity). */
#define x_obj_type_isspair(X)		(x_obj_type((X)) == x_type_pair_obj)

/** Heap-allocate an atom of the built-in atom type with flags @p F holding
 *  datum @p X. */
#define x_mksatom(B,F,X)			x_obj_make((B), x_type_atom_obj, (F), X_OBJ_LENGTH_ATOM, (X))
/** Heap-allocate an atom of the built-in atom type that owns its datum @p X
 *  (adds #X_OBJ_FLAG_OWN). */
#define x_mksatomown(B,F,X)			x_obj_make((B), x_type_atom_obj, (F) | X_OBJ_FLAG_OWN, X_OBJ_LENGTH_ATOM, (X))
/** Heap-allocate a pair of the built-in pair type with flags @p F holding
 *  first @p X and rest @p Y. */
#define x_mkspair(B,F,X,Y)			x_obj_make((B), x_type_pair_obj, (F), X_OBJ_LENGTH_PAIR, (X), (Y))

/** @} */

/**
 * @name Datum Member Accessors
 * @brief Reinterpret a single datum through the chosen union member.
 *
 * Each is an lvalue selecting one member of an #x_datum_union.
 * @{
 */

#define x_obj(X)					((X).p)		/**< Datum as an object pointer. */
#define x_fn(X)						((X).fn)	/**< Datum as a function pointer. */
#define x_int(X)					((X).i)		/**< Datum as an integer. */
#define x_char(X)					((X).c)		/**< Datum as a character. */
#define x_str(X)					((X).s)		/**< Datum as a string. */
#define x_ptr(X)					((X).v)		/**< Datum as an opaque pointer. */

/** @} */

/**
 * @name Data Unit Accessors
 * @brief Read an object's data units, optionally typed.
 *
 * The prefix selects the unit -- `first`/`atom` are data unit 0, while
 * `second`/`rest` are data unit 1 -- and the suffix selects the union member
 * (`ptr`, `obj`, `int`, `char`, `str`, `fn`). The `atom*` names are aliases
 * of the `first*` names (an atom's single value); the `rest*` names are
 * aliases of the `second*` names.
 * @{
 */

#define x_first(X)					x_obj_data_i((X),0)	/**< Data unit 0 (raw datum, lvalue). */
#define x_second(X)					x_obj_data_i((X),1)	/**< Data unit 1 (raw datum, lvalue). */
#define x_rest(X)					x_second((X))		/**< Alias for x_second(): data unit 1. */

#define x_firstptr(X)				x_ptr(x_first((X)))		/**< Data unit 0 as a pointer. */
#define x_firstobj(X)				x_obj(x_first((X)))		/**< Data unit 0 as an object. */
#define x_firstint(X)				x_int(x_first((X)))		/**< Data unit 0 as an integer. */
#define x_firstchar(X)				x_char(x_first((X)))	/**< Data unit 0 as a character. */
#define x_firststr(X)				x_str(x_first((X)))		/**< Data unit 0 as a string. */
#define x_firstfn(X)				x_fn(x_first((X)))		/**< Data unit 0 as a function. */

#define x_secondptr(X)				x_ptr(x_rest((X)))		/**< Data unit 1 as a pointer. */
#define x_secondobj(X)				x_obj(x_rest((X)))		/**< Data unit 1 as an object. */
#define x_secondint(X)				x_int(x_rest((X)))		/**< Data unit 1 as an integer. */
#define x_secondchar(X)				x_char(x_rest((X)))		/**< Data unit 1 as a character. */
#define x_secondstr(X)				x_str(x_rest((X)))		/**< Data unit 1 as a string. */
#define x_secondfn(X)				x_fn(x_rest((X)))		/**< Data unit 1 as a function. */

#define x_atomptr(X)				x_firstptr((X))		/**< Atom value as a pointer. */
#define x_atomobj(X)				x_firstobj((X))		/**< Atom value as an object. */
#define x_atomint(X)				x_firstint((X))		/**< Atom value as an integer. */
#define x_atomchar(X)				x_firstchar((X))	/**< Atom value as a character. */
#define x_atomstr(X)				x_firststr((X))		/**< Atom value as a string. */
#define x_atomfn(X)					x_firstfn((X))		/**< Atom value as a function. */

#define x_restptr(X)				x_secondptr((X))	/**< Rest value as a pointer. */
#define x_restobj(X)				x_secondobj((X))	/**< Rest value as an object. */
#define x_restint(X)				x_secondint((X))	/**< Rest value as an integer. */
#define x_restchar(X)				x_secondchar((X))	/**< Rest value as a character. */
#define x_reststr(X)				x_secondstr((X))	/**< Rest value as a string. */
#define x_restfn(X)					x_secondfn((X))		/**< Rest value as a function. */

/** @} */

/**
 * @defgroup obj_bit_accessors Bit-Path Pair Accessors
 * @brief Navigate nested pairs by a binary path.
 *
 * Each bit selects a branch -- `0` = first, `1` = rest -- read left to right
 * as a chain of accessors. For example x_011() is "first of rest of rest",
 * expanding to `x_0(x_1(x_1(X)))`. The leftmost bit is the outermost (last
 * applied) accessor. All results are object pointers.
 * Provided up to four levels deep.
 * @{
 */

#define x_0(X)						x_firstobj(X)		/**< first */
#define x_1(X)						x_restobj(X)		/**< rest */
#define x_00(X)						x_0(x_0(X))			/**< first of first */
#define x_01(X)						x_0(x_1(X))			/**< first of rest */
#define x_10(X)						x_1(x_0(X))			/**< rest of first */
#define x_11(X)						x_1(x_1(X))			/**< rest of rest */
#define x_000(X)					x_0(x_00(X))		/**< first of first of first */
#define x_001(X)					x_0(x_01(X))		/**< first of first of rest */
#define x_010(X)					x_0(x_10(X))		/**< first of rest of first */
#define x_011(X)					x_0(x_11(X))		/**< first of rest of rest */
#define x_100(X)					x_1(x_00(X))		/**< rest of first of first */
#define x_101(X)					x_1(x_01(X))		/**< rest of first of rest */
#define x_110(X)					x_1(x_10(X))		/**< rest of rest of first */
#define x_111(X)					x_1(x_11(X))		/**< rest of rest of rest */
#define x_0000(X)					x_0(x_000(X))		/**< first of first of first of first */
#define x_0001(X)					x_0(x_001(X))		/**< first of first of first of rest */
#define x_0010(X)					x_0(x_010(X))		/**< first of first of rest of first */
#define x_0011(X)					x_0(x_011(X))		/**< first of first of rest of rest */
#define x_0100(X)					x_0(x_100(X))		/**< first of rest of first of first */
#define x_0101(X)					x_0(x_101(X))		/**< first of rest of first of rest */
#define x_0110(X)					x_0(x_110(X))		/**< first of rest of rest of first */
#define x_0111(X)					x_0(x_111(X))		/**< first of rest of rest of rest */
#define x_1000(X)					x_1(x_000(X))		/**< rest of first of first of first */
#define x_1001(X)					x_1(x_001(X))		/**< rest of first of first of rest */
#define x_1010(X)					x_1(x_010(X))		/**< rest of first of rest of first */
#define x_1011(X)					x_1(x_011(X))		/**< rest of first of rest of rest */
#define x_1100(X)					x_1(x_100(X))		/**< rest of rest of first of first */
#define x_1101(X)					x_1(x_101(X))		/**< rest of rest of first of rest */
#define x_1110(X)					x_1(x_110(X))		/**< rest of rest of rest of first */
#define x_1111(X)					x_1(x_111(X))		/**< rest of rest of rest of rest */

/** @} */

/**
 * @name Object Functions
 *
 * @warning x-expr is not thread-safe. All functions that allocate objects,
 * mutate the heap chain, or modify field stacks assume single-threaded
 * execution. External synchronization is required for multi-threaded use.
 * @{
 */

/** Test whether @p p_obj is nil (a NULL object pointer). */
int x_obj_isnil(x_obj_t *p_base, x_obj_t *p_obj);

/** Allocate an uninitialized object with @p units data units. */
x_obj_t *x_obj_alloc(x_obj_t *p_base, x_obj_t *p_type, x_obj_flag_t flags, size_t units);
/** Allocate an object and initialize its data units from a va_list. */
x_obj_t *x_obj_make_va(x_obj_t *p_base, x_obj_t *p_type, x_obj_flag_t flags, size_t units, va_list ap);
/** Allocate an object and initialize its data units from varargs. */
x_obj_t *x_obj_make(x_obj_t *p_base, x_obj_t *p_type, x_obj_flag_t flags, size_t units, ...);
/** Free an object, releasing any owned datum and extra metadata units. */
void x_obj_free(x_obj_t *p_base, x_obj_t *p_obj);

/** Primitive: resolve an object's type object (#x_fn_t convention). */
x_obj_t *x_obj_prim_type_name(x_obj_t *p_base, x_obj_t *p_args);
/** Get an object's type name as a C string (#X_TYPE_NIL_NAME if nil). */
x_char_t *x_obj_type_name(x_obj_t *p_base, x_obj_t *p_obj);

/** Primitive: data-unit count of an atom (#x_fn_t convention). */
x_obj_t *x_atom_prim_units(x_obj_t *p_base, x_obj_t *p_args);
/** Primitive: data-unit count of a pair (#x_fn_t convention). */
x_obj_t *x_pair_prim_units(x_obj_t *p_base, x_obj_t *p_args);
/** Primitive: dispatch an object's data-unit count by type (#x_fn_t convention). */
x_obj_t *x_obj_prim_units(x_obj_t *p_base, x_obj_t *p_args);
/** Get an object's data-unit count as an integer. */
x_int_t x_obj_units(x_obj_t *p_base, x_obj_t *p_obj);

/** Primitive: logical length of an atom (#x_fn_t convention). */
x_obj_t *x_atom_prim_length(x_obj_t *p_base, x_obj_t *p_args);
/** Primitive: logical length of a pair (#x_fn_t convention). */
x_obj_t *x_pair_prim_length(x_obj_t *p_base, x_obj_t *p_args);
/** Primitive: dispatch an object's logical length by type (#x_fn_t convention). */
x_obj_t *x_obj_prim_length(x_obj_t *p_base, x_obj_t *p_args);
/** Get an object's logical length as an integer. */
x_int_t x_obj_length(x_obj_t *p_base, x_obj_t *p_obj);

/** Push @p p_value onto the pair-list stack rooted at @p *p_field. */
x_obj_t *x_obj_push_field(x_obj_t *p_base, x_obj_t **p_field, x_obj_t *p_value, x_obj_flag_t flags);
/** Pop and return the head value of the pair-list stack at @p *p_field. */
x_obj_t *x_obj_pop_field(x_obj_t *p_base, x_obj_t **p_field);
/** Callable wrapper for x_obj_push_field() (#x_fn_t convention). */
x_obj_t *x_obj_push(x_obj_t *p_base, x_obj_t *p_args);
/** Callable wrapper for x_obj_pop_field() (#x_fn_t convention). */
x_obj_t *x_obj_pop(x_obj_t *p_base, x_obj_t *p_args);

/** Output an error message to stderr. */
void x_obj_error(x_obj_t *p_base, x_char_t *message, x_obj_t *p_obj);

/** @} */

/**
 * @name Debug Utilities
 * @brief Formatted debug output and object dumping (compiled only with DEBUG).
 * @{
 */

#ifdef DEBUG
/** @internal */
void _x_obj_debug_va(char *file, long unsigned line, x_obj_t *p_base, char *fmt, va_list ap);
/**
 * Write a formatted debug message with va_list arguments.
 *
 * Automatically prepends the source file name and line number.
 * Compiles to a no-op when DEBUG is not defined.
 *
 * @param p_base Base (execution context).
 * @param fmt    printf-style format string.
 * @param ap     Variable argument list.
 */
#define x_obj_debug_va(p_base, fmt, ap)\
	_x_obj_debug_va(__FILE__, __LINE__, p_base, fmt, ap)

/** @internal */
void _x_obj_debug(char *file, long unsigned line, x_obj_t *p_base, char *fmt, ...);
/**
 * Write a formatted debug message.
 *
 * Automatically prepends the source file name and line number.
 * Compiles to a no-op when DEBUG is not defined.
 *
 * @param p_base Base (execution context).
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
#define x_obj_debug(p_base, fmt, ...)\
	_x_obj_debug(__FILE__, __LINE__, p_base, fmt, __VA_ARGS__)

/** @internal */
void _x_obj_dump(char *file, long unsigned line, x_obj_t *p_base, x_obj_t *p_obj, char *msg);
/**
 * Write a one-line dump of an object's type, flags, address, and data.
 *
 * Automatically prepends the source file name and line number.
 * Compiles to a no-op when DEBUG is not defined.
 *
 * @param p_base Base (execution context).
 * @param p_obj  The object to dump.
 * @param msg    A label prefix for the output.
 */
#define x_obj_dump(p_base, p_obj, msg)\
	_x_obj_dump(__FILE__, __LINE__, p_base, p_obj, msg)

#else /* DEBUG */

/** @copydoc x_obj_debug_va */
#define x_obj_debug_va(p_base, fmt, ap)		;
/** @copydoc x_obj_debug */
#define x_obj_debug(p_base, fmt, ...)		;
/** @copydoc x_obj_dump */
#define x_obj_dump(p_base, p_obj, msg)		;

#endif /* DEBUG */

/** @} */

#endif /* X_OBJ_H */
