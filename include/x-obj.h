#ifndef X_OBJ_H
#define X_OBJ_H

/**
 * @file x-obj.h
 * @brief Core object system -- types, flags, accessors, and constructors.
 *
 * Defines the tagged-union object representation used throughout x-expr.
 * Objects are arrays of x_obj_t units preceded by metadata (type pointer
 * and flags). Atoms hold one data unit; pairs hold two (first and rest).
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

#include "x-lib.h"

/**
 * @name Type Symbol Strings
 * @{
 */
#define X_TYPE_NIL_SYMBOL			"NIL"	/**< Display name for nil/NULL. */
#define X_TYPE_ATOM_SYMBOL			"ATOM"	/**< Display name for atom objects. */
#define X_TYPE_PAIR_SYMBOL			"PAIR"	/**< Display name for pair objects. */

#define X_OBJ_TRUE_SYMBOL			"#t"	/**< Display name for the true object. */
#define X_OBJ_FALSE_SYMBOL			"#f"	/**< Display name for the false object. */
/** @} */

/**
 * @name Data Structures
 * @{
 */

/**
 * Object flag and type tag bitfield.
 *
 * Layout:
 * - Bits 0--3: attribute flags (X_OBJ_FLAG_1 through X_OBJ_FLAG_4)
 * - Bits 4--7: simple type tag (PRIM, FN, INT, CHAR, STR, PTR)
 * - Bit  5:    OWN -- object owns its data (freed on x_obj_free)
 * - Bit  6:    RO  -- read-only
 * - Bit  7:    META -- extra metadata units were allocated
 * - Bits 8--9: (X_HEAP only) SHARED and HEAP flags
 */
typedef enum x_obj_flag_enum
{
	X_OBJ_FLAG_NONE=0x0,		/**< No flags set. */

	X_OBJ_FLAG_OBJ=0x0,		/**< Base object type (no type tag). */

	X_OBJ_FLAG_1=0x1,			/**< Attribute bit 0. */
	X_OBJ_FLAG_2=0x2,			/**< Attribute bit 1. */
	X_OBJ_FLAG_3=0x4,			/**< Attribute bit 2. */
	X_OBJ_FLAG_4=0x8,			/**< Attribute bit 3. */

	X_OBJ_FLAG_ATTR_MASK=0xF,	/**< Mask for the attribute nibble. */

	X_OBJ_FLAG_SIMPLE_TYPE=0x10,/**< Base value for the simple type range. */
	X_OBJ_FLAG_PRIM=0x10,		/**< Native primitive function. */
	X_OBJ_FLAG_FN,				/**< User-defined function. */
	X_OBJ_FLAG_INT,				/**< Integer datum. */
	X_OBJ_FLAG_CHAR,			/**< Character datum. */
	X_OBJ_FLAG_STR,				/**< String pointer datum. */
	X_OBJ_FLAG_PTR,				/**< Void pointer datum. */

	X_OBJ_FLAG_TYPE_MASK=0xF0,	/**< Mask for the type nibble. */

	X_OBJ_FLAG_OWN=0x20,		/**< Object owns its data (freed with object). */
	X_OBJ_FLAG_RO=0x40,		/**< Read-only flag. */
	X_OBJ_FLAG_META=0x80,		/**< Extra metadata units were allocated. */

#ifndef X_HEAP
	X_OBJ_FLAG_MASK=0xFF		/**< Mask covering all valid flags. */
#else /* X_HEAP */
	X_OBJ_FLAG_SHARED=0x100,	/**< Object is shared (not swept by GC). */
	X_OBJ_FLAG_HEAP=0x200,		/**< Object was allocated on the tracked heap. */

	X_OBJ_FLAG_MASK=0x3FF		/**< Mask covering all valid flags (with heap). */
#endif /* X_HEAP */
} x_obj_flag_t;

/**
 * The fundamental object type.
 *
 * A tagged union that can hold a pointer, function, integer, character,
 * string, or void pointer. Objects are arrays of x_obj_t units; metadata
 * (type and flags) occupies the units preceding the data pointer.
 */
typedef union x_datum_union x_obj_t;

/**
 * Native function pointer type.
 *
 * All built-in primitive functions share this signature. The first argument
 * is the base/environment object; the second is a pair list of arguments.
 *
 * @param p_base The base environment object.
 * @param p_args A pair list of arguments.
 * @return The function result as an object pointer, or NULL.
 */
typedef x_obj_t * (*x_fn_t)(x_obj_t *p_base, x_obj_t *p_args);

/**
 * Datum union -- the storage unit for all object values.
 *
 * Every x_obj_t is one instance of this union. Which member is valid
 * depends on the object's type flags.
 */
union x_datum_union
{
	x_obj_t *p;		/**< Object pointer (for pairs and linked structures). */
	x_fn_t fn;			/**< Native function pointer. */
	x_int_t i;			/**< Integer value. */
	x_char_t c;		/**< Character value. */
	x_char_t *s;		/**< String pointer. */
	void *v;			/**< Generic void pointer. */
};

/** @} */

/**
 * @defgroup obj_layout Object Memory Layout
 * @brief Constants defining the metadata and data layout of objects.
 *
 * Each object in memory is an array of x_obj_t units:
 *
 * ```
 * [heap?] [type] [flags] [data_0] [data_1] ...
 * |<--- metadata ------->|<----- data ------->|
 * ```
 *
 * The data pointer returned by x_obj_alloc() points at `data_0`.
 * Metadata fields are at negative offsets from the data pointer.
 * @{
 */

#ifdef X_HEAP
/** Number of extra metadata units for heap tracking. */
#define X_OBJ_UNITS_HEAP			1
#else /* X_HEAP */
#define X_OBJ_UNITS_HEAP			0
#endif /* X_HEAP */

#ifndef X_OBJ_UNITS_TYPE
/** Number of metadata units for the type pointer. */
#define X_OBJ_UNITS_TYPE			1
#endif /* X_OBJ_UNITS_TYPE */

#ifndef X_OBJ_UNITS_FLAGS
/** Number of metadata units for the flags word. */
#define X_OBJ_UNITS_FLAGS			1
#endif /* X_OBJ_UNITS_FLAGS */

/** Indices into the metadata region (relative to the allocation start). */
enum {
#ifdef X_HEAP
	X_OBJ_META_HEAP = 0,		/**< Index of the heap chain pointer. */
#endif /* X_HEAP */
	X_OBJ_META_TYPE = X_OBJ_UNITS_HEAP,					/**< Index of the type pointer. */
	X_OBJ_META_FLAGS = (X_OBJ_UNITS_HEAP + X_OBJ_UNITS_TYPE),	/**< Index of the flags word. */
	X_OBJ_META_LEN = (X_OBJ_UNITS_HEAP + X_OBJ_UNITS_TYPE + X_OBJ_UNITS_FLAGS) /**< Total metadata length. */
};

#ifndef X_OBJ_UNITS_META
/** Total number of metadata units per object. */
#define X_OBJ_UNITS_META			X_OBJ_META_LEN
#endif /* X_OBJ_UNITS_META */

/** Number of data units in an atom (1). */
#define X_OBJ_UNITS_ATOM			1
/** Number of data units in a pair (2). */
#define X_OBJ_UNITS_PAIR			2

/** Logical length of an atom. */
#define X_OBJ_LENGTH_ATOM			X_OBJ_UNITS_ATOM
/** Logical length of a pair. */
#define X_OBJ_LENGTH_PAIR			X_OBJ_UNITS_PAIR

/** @} */

/**
 * @name Static Object Types
 * @{
 */

/** Static atom array type -- sized for metadata + one data unit. */
typedef x_obj_t x_satom_t[X_OBJ_META_LEN + X_OBJ_UNITS_ATOM];

/** Static pair array type -- sized for metadata + two data units. */
typedef x_obj_t x_spair_t[X_OBJ_META_LEN + X_OBJ_UNITS_PAIR];

/** @} */

/**
 * @name Global Type Objects
 *
 * Statically allocated type descriptor atoms. The data unit of each
 * holds a string identifying the type (e.g. "ATOM") or an integer
 * holding a size constant.
 * @{
 */
extern x_satom_t x_type_atom_obj;			/**< Type descriptor for atoms. */
extern x_satom_t x_type_pair_obj;			/**< Type descriptor for pairs. */
extern x_satom_t x_type_base_obj;			/**< Type descriptor for the base object. */
extern x_satom_t x_type_units_atom_obj;		/**< Units constant for atoms (1). */
extern x_satom_t x_type_units_pair_obj;		/**< Units constant for pairs (2). */
extern x_satom_t x_type_length_atom_obj;	/**< Length constant for atoms (1). */
extern x_satom_t x_type_length_pair_obj;	/**< Length constant for pairs (2). */

extern x_satom_t x_true_obj;				/**< The canonical true object (\#t). */
extern x_satom_t x_false_obj;				/**< The canonical false object (\#f). */
/** @} */

/**
 * @defgroup obj_meta Object Metadata Accessors
 * @brief Macros to access the metadata fields of an object.
 *
 * These macros index into the metadata region that precedes the data
 * pointer. @p X must point to the start of the object's allocation
 * (i.e. the metadata array, not the data pointer).
 * @{
 */

#ifdef X_HEAP
/** Get/set the heap chain pointer of object @p X. */
#define x_obj_heap(X)				((X)[X_OBJ_META_HEAP].p)
#endif /* X_HEAP */

/** Get/set the type pointer of object @p X. */
#define x_obj_type(X)				((X)[X_OBJ_META_TYPE].p)

/** Get/set the flags word of object @p X. */
#define x_obj_flags(X)				((X)[X_OBJ_META_FLAGS].i)

/** Get a pointer to the first data unit of object @p X. */
#define x_obj_data_ptr(X)			((X) + X_OBJ_META_LEN)

/** Get the data unit at index @p I of object @p X. */
#define x_obj_data_i(X,I)			(x_obj_data_ptr((X))[(I)])

/** Get the first data unit of object @p X (shorthand for index 0). */
#define x_obj_data(X)				x_obj_data_i((X), 0)

/**
 * Access extra metadata at negative offset @p I from the data pointer.
 *
 * Index 0 is the unit immediately before the standard metadata.
 * Used by extensions that allocate additional metadata via obj_meta_extra.
 *
 * @param X The object pointer.
 * @param I Zero-based index into the extra metadata region.
 */
#define x_obj_meta_i(X, I)			((X)[-((I) + 1)])

/**
 * Static initializer for an object's metadata and data units.
 *
 * Produces a brace-enclosed initializer list suitable for x_satom_t
 * or x_spair_t arrays.
 *
 * @param T Type pointer (x_obj_t * or NULL).
 * @param F Flags value (x_obj_flag_t).
 * @param ... Data unit initializers.
 */
#ifdef X_HEAP
#define x_obj_set(T,F,...)			{ { .v = NULL }, { .p = (T) }, { .i = (F) }, __VA_ARGS__ }
#else /* X_HEAP */
#define x_obj_set(T,F,...)			{ { .p = (T) }, { .i = (F) }, __VA_ARGS__ }
#endif /* X_HEAP */

/** @} */

/**
 * @defgroup obj_typecheck Type Checking Macros
 * @brief Macros to test an object's type.
 * @{
 */

/** Test whether the type pointer of @p X is nil. */
#define x_obj_type_isnil(B,X)		x_obj_isnil((B), x_obj_type(X))

/** Test whether @p X has the named type @p T (string comparison). */
#define x_obj_is_type(B,X,T)		(x_lib_strcmp(x_obj_type_name((B), (X)), (T)) == 0)

/** Test whether @p X is a static atom (type pointer == x_type_atom_obj). */
#define x_obj_type_issatom(X)		(x_obj_type((X)) == x_type_atom_obj)

/** Test whether @p X is a static pair (type pointer == x_type_pair_obj). */
#define x_obj_type_isspair(X)		(x_obj_type((X)) == x_type_pair_obj)

/** @} */

/**
 * @defgroup obj_constructors Object Constructor Macros
 * @brief Convenience macros for creating atoms and pairs.
 * @{
 */

/** Allocate a static atom with flags @p F and data @p X. */
#define x_mksatom(B,F,X)			x_obj_make((B), x_type_atom_obj, (F), X_OBJ_LENGTH_ATOM, (X))

/** Allocate a static atom with OWN flag set. */
#define x_mksatomown(B,F,X)			x_obj_make((B), x_type_atom_obj, (F) | X_OBJ_FLAG_OWN, X_OBJ_LENGTH_ATOM, (X))

/** Allocate a static pair with first @p X and rest @p Y. */
#define x_mkspair(B,F,X,Y)			x_obj_make((B), x_type_pair_obj, (F), X_OBJ_LENGTH_PAIR, (X), (Y))

/** @} */

/**
 * @defgroup obj_datum Datum Accessor Macros
 * @brief Extract a typed value from a datum union.
 *
 * Each macro accesses the corresponding member of an x_obj_t (x_datum_union).
 * @{
 */
#define x_obj(X)					((X).p)		/**< Extract object pointer. */
#define x_fn(X)						((X).fn)	/**< Extract function pointer. */
#define x_int(X)					((X).i)		/**< Extract integer. */
#define x_char(X)					((X).c)		/**< Extract character. */
#define x_str(X)					((X).s)		/**< Extract string pointer. */
#define x_ptr(X)					((X).v)		/**< Extract void pointer. */
/** @} */

/**
 * @defgroup pair_access Pair Element Accessors
 * @brief Access the first and rest (second) data units of a pair, with typed variants.
 *
 * - x_first() / x_second() / x_rest() -- raw datum access
 * - x_first**TYPE**() / x_second**TYPE**() -- typed extraction (ptr, obj, int, char, str, fn)
 * - x_atom**TYPE**() -- aliases for x_first**TYPE**() (atom has only one data unit)
 * - x_rest**TYPE**() -- aliases for x_second**TYPE**()
 * @{
 */

/** Get the first data unit of pair @p X (raw datum). */
#define x_first(X)					x_obj_data_i((X),0)
/** Get the second data unit of pair @p X (raw datum). */
#define x_second(X)					x_obj_data_i((X),1)
/** Alias for x_second(). */
#define x_rest(X)					x_second((X))

#define x_firstptr(X)				x_ptr(x_first((X)))	/**< First as void pointer. */
#define x_firstobj(X)				x_obj(x_first((X)))	/**< First as object pointer. */
#define x_firstint(X)				x_int(x_first((X)))	/**< First as integer. */
#define x_firstchar(X)				x_char(x_first((X)))	/**< First as character. */
#define x_firststr(X)				x_str(x_first((X)))	/**< First as string pointer. */
#define x_firstfn(X)				x_fn(x_first((X)))		/**< First as function pointer. */

#define x_secondptr(X)				x_ptr(x_rest((X)))		/**< Rest as void pointer. */
#define x_secondobj(X)				x_obj(x_rest((X)))		/**< Rest as object pointer. */
#define x_secondint(X)				x_int(x_rest((X)))		/**< Rest as integer. */
#define x_secondchar(X)				x_char(x_rest((X)))	/**< Rest as character. */
#define x_secondstr(X)				x_str(x_rest((X)))		/**< Rest as string pointer. */
#define x_secondfn(X)				x_fn(x_rest((X)))		/**< Rest as function pointer. */

#define x_atomptr(X)				x_firstptr((X))		/**< Atom data as void pointer. */
#define x_atomobj(X)				x_firstobj((X))		/**< Atom data as object pointer. */
#define x_atomint(X)				x_firstint((X))		/**< Atom data as integer. */
#define x_atomchar(X)				x_firstchar((X))	/**< Atom data as character. */
#define x_atomstr(X)				x_firststr((X))		/**< Atom data as string pointer. */
#define x_atomfn(X)					x_firstfn((X))		/**< Atom data as function pointer. */

#define x_restptr(X)				x_secondptr((X))	/**< Rest data as void pointer. */
#define x_restobj(X)				x_secondobj((X))	/**< Rest data as object pointer. */
#define x_restint(X)				x_secondint((X))	/**< Rest data as integer. */
#define x_restchar(X)				x_secondchar((X))	/**< Rest data as character. */
#define x_reststr(X)				x_secondstr((X))	/**< Rest data as string pointer. */
#define x_restfn(X)					x_secondfn((X))		/**< Rest data as function pointer. */

/** @} */

/**
 * @defgroup tree_nav Binary Tree Navigation
 * @brief Navigate nested pair structures using positional indices.
 *
 * Each digit in the macro name selects `first` (0) or `rest` (1) at
 * successive depth levels. For example:
 * - x_0(X) = first child of X
 * - x_1(X) = rest child of X
 * - x_01(X) = first child of the rest child of X
 * - x_101(X) = rest of first of rest of X
 *
 * All return an object pointer (x_obj_t *).
 * @{
 */
#define x_0(X)						x_firstobj(X)	/**< first(X) */
#define x_1(X)						x_restobj(X)	/**< rest(X) */
#define x_00(X)						x_0(x_0(X))	/**< first(first(X)) */
#define x_01(X)						x_0(x_1(X))	/**< first(rest(X)) */
#define x_10(X)						x_1(x_0(X))	/**< rest(first(X)) */
#define x_11(X)						x_1(x_1(X))	/**< rest(rest(X)) */
#define x_000(X)					x_0(x_00(X))	/**< 3-deep: first(first(first(X))) */
#define x_001(X)					x_0(x_01(X))	/**< 3-deep: first(first(rest(X))) */
#define x_010(X)					x_0(x_10(X))	/**< 3-deep: first(rest(first(X))) */
#define x_011(X)					x_0(x_11(X))	/**< 3-deep: first(rest(rest(X))) */
#define x_100(X)					x_1(x_00(X))	/**< 3-deep: rest(first(first(X))) */
#define x_101(X)					x_1(x_01(X))	/**< 3-deep: rest(first(rest(X))) */
#define x_110(X)					x_1(x_10(X))	/**< 3-deep: rest(rest(first(X))) */
#define x_111(X)					x_1(x_11(X))	/**< 3-deep: rest(rest(rest(X))) */
#define x_0000(X)					x_0(x_000(X))	/**< 4-deep navigation. */
#define x_0001(X)					x_0(x_001(X))	/**< 4-deep navigation. */
#define x_0010(X)					x_0(x_010(X))	/**< 4-deep navigation. */
#define x_0011(X)					x_0(x_011(X))	/**< 4-deep navigation. */
#define x_0100(X)					x_0(x_100(X))	/**< 4-deep navigation. */
#define x_0101(X)					x_0(x_101(X))	/**< 4-deep navigation. */
#define x_0110(X)					x_0(x_110(X))	/**< 4-deep navigation. */
#define x_0111(X)					x_0(x_111(X))	/**< 4-deep navigation. */
#define x_1000(X)					x_1(x_000(X))	/**< 4-deep navigation. */
#define x_1001(X)					x_1(x_001(X))	/**< 4-deep navigation. */
#define x_1010(X)					x_1(x_010(X))	/**< 4-deep navigation. */
#define x_1011(X)					x_1(x_011(X))	/**< 4-deep navigation. */
#define x_1100(X)					x_1(x_100(X))	/**< 4-deep navigation. */
#define x_1101(X)					x_1(x_101(X))	/**< 4-deep navigation. */
#define x_1110(X)					x_1(x_110(X))	/**< 4-deep navigation. */
#define x_1111(X)					x_1(x_111(X))	/**< 4-deep navigation. */
/** @} */

/**
 * @name Object Functions
 * @{
 */

/** Test whether an object is nil (NULL). */
int x_obj_isnil(x_obj_t *p_base, x_obj_t *p_obj);

/** Allocate an uninitialized object. */
x_obj_t *x_obj_alloc(x_obj_t *p_base, x_obj_t *p_type, x_obj_flag_t flags, size_t size);

/** Create an object with data units from a va_list. */
x_obj_t *x_obj_make_va(x_obj_t *p_base, x_obj_t *p_type, x_obj_flag_t flags, size_t size, va_list ap);

/** Create an object with data units from variadic arguments. */
x_obj_t *x_obj_make(x_obj_t *p_base, x_obj_t *p_type, x_obj_flag_t flags, size_t size, ...);

/** Free an object and its owned data. */
void x_obj_free(x_obj_t *p_base, x_obj_t *p_obj);

/** Primitive: get the type name object for an object. */
x_obj_t *x_obj_prim_type_name(x_obj_t *p_base, x_obj_t *p_args);

/** Get the type name string for an object. */
x_char_t *x_obj_type_name(x_obj_t *p_base, x_obj_t *p_obj);

/** Primitive: return the units count for an atom. */
x_obj_t *x_atom_prim_units(x_obj_t *p_base, x_obj_t *p_args);

/** Primitive: return the units count for a pair. */
x_obj_t *x_pair_prim_units(x_obj_t *p_base, x_obj_t *p_args);

/** Primitive: return the units count for any object. */
x_obj_t *x_obj_prim_units(x_obj_t *p_base, x_obj_t *p_args);

/** Get the number of data units for an object. */
x_int_t x_obj_units(x_obj_t *p_base, x_obj_t *p_obj);

/** Primitive: return the length constant for an atom. */
x_obj_t *x_atom_prim_length(x_obj_t *p_base, x_obj_t *p_args);

/** Primitive: return the length constant for a pair. */
x_obj_t *x_pair_prim_length(x_obj_t *p_base, x_obj_t *p_args);

/** Primitive: return the length for any object. */
x_obj_t *x_obj_prim_length(x_obj_t *p_base, x_obj_t *p_args);

/** Get the logical length of an object. */
x_int_t x_obj_length(x_obj_t *p_base, x_obj_t *p_obj);

/** Push a value onto a field stack. */
x_obj_t *x_obj_push(x_obj_t *p_base, x_obj_t *p_args);

/** Pop a value from a field stack. */
x_obj_t *x_obj_pop(x_obj_t *p_base, x_obj_t *p_args);

/** Output an error message to stderr and exit. */
void x_obj_error(x_obj_t *p_base, x_char_t *message, x_obj_t *p_obj);

/** @} */

/**
 * @name Debug Functions
 * @{
 */

#ifdef DEBUG
/** @internal */
void _x_obj_debug_va(char *file, long unsigned line, x_obj_t *p_base, char *fmt, va_list ap);

/**
 * Write a formatted debug message using the object system's base.
 *
 * @param p_base The base environment.
 * @param fmt    printf-style format string.
 * @param ap     Variable argument list.
 */
#define x_obj_debug_va(p_base, fmt, ap)\
	_x_obj_debug_va(__FILE__, __LINE__, p_base, fmt, ap)

/** @internal */
void _x_obj_debug(char *file, long unsigned line, x_obj_t *p_base, char *fmt, ...);

/**
 * Write a formatted debug message using the object system's base.
 *
 * @param p_base The base environment.
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
#define x_obj_debug(p_base, fmt, ...)\
	_x_obj_debug(__FILE__, __LINE__, p_base, fmt, __VA_ARGS__)

/** @internal */
void _x_obj_dump(char *file, long unsigned line, x_obj_t *p_base, x_obj_t *p_obj, char *msg);

/**
 * Dump an object's type, flags, and data for debugging.
 *
 * @param p_base The base environment.
 * @param p_obj  The object to dump.
 * @param msg    Optional message prefix, or NULL.
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
