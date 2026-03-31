/*
 * # Unit Tests: *x-obj*
 */

#define TEST_RUNNER_OVERHEAD
#include "test-runner.h"

/* Include Heap structures. */
#ifndef X_HEAP
#define X_HEAP
#endif /* X_HEAP */

#include "test-helper-system.c"

#include "src/x-sys.c"
#include "src/x-lib.c"
#include "src/x.c"
#include "src/x-obj.c"


/*
 * ## Test Overhead
 */
static void _setup(void)
{
	helper_set_alloc(MEM_GUARANTEED);
	helper_sys_funcs.exit = mock_exit;
	helper_sys_funcs.malloc = helper_malloc;
	helper_sys_funcs.free = helper_free;
}

static void _teardown(void)
{
}

#define TEST_TYPE_ID		0x1d
#define TEST_TYPE_UNITS		0x1e
#define TEST_TYPE_LENGTH	0x1f
#define TEST_TYPE_STR_NAME	"NAME"

x_obj_t * (test_prim_fn)(x_obj_t *p_base, x_obj_t *p_args)
{
	return NULL;
}


/*
 * ## Test Helpers
 */
#define nil			p_base
#define pair(X,Y)	(x_mkspair(p_base, X_OBJ_FLAG_NONE, (X), (Y)))
#define atom(X)		(x_mksatom(p_base, X_OBJ_FLAG_NONE, (X)))


/*
 * ## Test Runners
 */
static char *test_meta_layout(void)
{
	_it_should("have heap at index 0",
		0 == X_OBJ_META_HEAP
	);

	_it_should("have type at index 1",
		1 == X_OBJ_META_TYPE
	);

	_it_should("have flags at index 2",
		2 == X_OBJ_META_FLAGS
	);

	_it_should("have meta length of 3",
		3 == X_OBJ_META_LEN
	);

	_it_should("have atom unit count of 1",
		1 == X_OBJ_UNITS_ATOM
	);

	_it_should("have pair unit count of 2",
		2 == X_OBJ_UNITS_PAIR
	);

	return NULL;
}

static char *test_flag_constants(void)
{
	_it_should("define X_OBJ_FLAG_NONE",       0x00 == X_OBJ_FLAG_NONE);
	_it_should("define X_OBJ_FLAG_OBJ",        0x00 == X_OBJ_FLAG_OBJ);

	_it_should("define X_OBJ_FLAG_1",          0x01 == X_OBJ_FLAG_1);
	_it_should("define X_OBJ_FLAG_2",          0x02 == X_OBJ_FLAG_2);
	_it_should("define X_OBJ_FLAG_3",          0x04 == X_OBJ_FLAG_3);
	_it_should("define X_OBJ_FLAG_4",          0x08 == X_OBJ_FLAG_4);

	_it_should("define X_OBJ_FLAG_ATTR_MASK",  0x0F == X_OBJ_FLAG_ATTR_MASK);

	_it_should("define X_OBJ_FLAG_PRIM",       0x10 == X_OBJ_FLAG_PRIM);
	_it_should("define X_OBJ_FLAG_FN",         0x11 == X_OBJ_FLAG_FN);
	_it_should("define X_OBJ_FLAG_INT",        0x12 == X_OBJ_FLAG_INT);
	_it_should("define X_OBJ_FLAG_CHAR",       0x13 == X_OBJ_FLAG_CHAR);
	_it_should("define X_OBJ_FLAG_STR",        0x14 == X_OBJ_FLAG_STR);
	_it_should("define X_OBJ_FLAG_PTR",        0x15 == X_OBJ_FLAG_PTR);

	_it_should("define X_OBJ_FLAG_TYPE_MASK",  0xF0 == X_OBJ_FLAG_TYPE_MASK);

	_it_should("define X_OBJ_FLAG_OWN",        0x20 == X_OBJ_FLAG_OWN);
	_it_should("define X_OBJ_FLAG_RO",         0x40 == X_OBJ_FLAG_RO);
	_it_should("define X_OBJ_FLAG_META",       0x80 == X_OBJ_FLAG_META);
	_it_should("define X_OBJ_FLAG_SHARED",     0x100 == X_OBJ_FLAG_SHARED);
	_it_should("define X_OBJ_FLAG_HEAP",       0x200 == X_OBJ_FLAG_HEAP);
	_it_should("define X_OBJ_FLAG_MASK",       0x3FF == X_OBJ_FLAG_MASK);

	return NULL;
}

static char *test_obj_heap(void)
{
	x_satom_t obj[2];

	obj[0][X_OBJ_META_HEAP].p = obj[1];
	_it_should("return an object's heap", obj[1] == x_obj_heap(obj[0]));

	return NULL;
}

static char *test_obj_type(void)
{
	x_satom_t obj[2];

	obj[0][X_OBJ_META_TYPE].p = obj[1];
	_it_should("return an object's type", obj[1] == x_obj_type(obj[0]));

	return NULL;
}

static char *test_obj_flags(void)
{
	x_satom_t obj;
	x_int_t i = (x_int_t)rand();

	obj[X_OBJ_META_FLAGS].i = i;
	_it_should("return an object's flags", i == x_obj_flags(obj));

	return NULL;
}

static char *test_obj_data_ptr(void)
{
	x_satom_t obj;
	x_obj_t *p = &obj[X_OBJ_META_LEN];

	_it_should("return an object's data pointer", p == x_obj_data_ptr(obj));

	return NULL;
}

static char *test_obj_data_i(void)
{
	x_spair_t obj;
	x_obj_t *p = &obj[X_OBJ_META_LEN];

	_it_should("return an object's data at index 0", p == &x_obj_data_i(obj, 0));
	_it_should("return an object's data at index 1", (p + 1) == &x_obj_data_i(obj, 1));

	return NULL;
}

static char *test_obj_data(void)
{
	x_satom_t obj;
	x_obj_t *p = &obj[X_OBJ_META_LEN];

	_it_should("return an object's data", p == &x_obj_data(obj));

	return NULL;
}

static char *test_obj_meta_i(void)
{
	x_obj_t obj[2 + X_OBJ_META_LEN], *p_obj = &obj[2];
	x_obj_t *p = &obj[1];

	_it_should("return an object's meta data at index 0", p == &x_obj_meta_i(p_obj, 0));
	_it_should("return an object's meta data at index 1", (p - 1) == &x_obj_meta_i(p_obj, 1));

	return NULL;
}

static char *test_obj_set(void)
{
	enum x_obj_flag_enum flags = rand();
	int data[3] = { rand(), rand(), rand() };
	x_obj_t type,
		obj[X_OBJ_META_LEN + 3] = x_obj_set(
			&type, flags, { .i = data[0] }, { .i = data[1] }, { .i = data[2] }
		);

	_it_should("set an object's heap, type, flags, and data",
		NULL == x_obj_heap(obj)
		&& &type == x_obj_type(obj)
		&& flags == x_obj_flags(obj)
		&& data[0] == x_obj_data_i(obj, 0).i
		&& data[1] == x_obj_data_i(obj, 1).i
		&& data[2] == x_obj_data_i(obj, 2).i
	);

	return NULL;
}

static char *test_obj_type_isnil(void)
{
	x_obj_t type, obj[2][X_OBJ_META_LEN] = {
		x_obj_set(NULL, X_OBJ_FLAG_NONE),
		x_obj_set(&type, X_OBJ_FLAG_NONE)
	};

	_it_should("return true when object type is NULL",
		1 == x_obj_type_isnil(NULL, obj[0])
	);

	_it_should("return false when object type is not NULL",
		0 == x_obj_type_isnil(NULL, obj[1])
	);

	return NULL;
}

static char *test_obj_is_type(void)
{
	x_satom_t obj = x_obj_set(x_type_atom_obj, X_OBJ_FLAG_NONE, {.i = 0});

	_it_should("return true when type name matches",
		1 == x_obj_is_type(NULL, obj, X_TYPE_ATOM_NAME)
	);

	_it_should("return false when type name does not match",
		0 == x_obj_is_type(NULL, obj, X_TYPE_PAIR_NAME)
	);

	return NULL;
}

static char *test_obj_type_issatom(void)
{
	x_satom_t atom = x_obj_set(x_type_atom_obj, X_OBJ_FLAG_NONE, {.i = 0});
	x_spair_t pair = x_obj_set(x_type_pair_obj, X_OBJ_FLAG_NONE, {.i = 0}, {.i = 0});

	_it_should("return true when object is an atom",
		1 == x_obj_type_issatom(atom)
	);

	_it_should("return false when object is not an atom",
		0 == x_obj_type_issatom(pair)
	);

	return NULL;
}

static char *test_obj_type_isspair(void)
{
	x_spair_t pair = x_obj_set(x_type_pair_obj, X_OBJ_FLAG_NONE, {.i = 0}, {.i = 0});
	x_satom_t atom = x_obj_set(x_type_atom_obj, X_OBJ_FLAG_NONE, {.i = 0});

	_it_should("return true when object is a pair",
		1 == x_obj_type_isspair(pair)
	);

	_it_should("return false when object is not a pair",
		0 == x_obj_type_isspair(atom)
	);

	return NULL;
}

static char *test_mksatom(void)
{
	x_obj_t *p_base, *p_obj;
	unsigned int flags = rand();
	x_int_t value = rand();

	helper_alloc_reset();

	p_obj = x_mksatom(NULL, flags, value);
	_it_should("make an atom with the supplied flags and value",
		x_obj_type_issatom(p_obj)
		&& flags == x_obj_flags(p_obj)
		&& value == x_int(x_obj_data(p_obj))
	);
	x_obj_free(NULL, p_obj);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mksatom(p_base, flags, value);
	_it_should("make an atom with the supplied base, flags and value",
		x_obj_type_issatom(p_obj)
		&& p_obj == x_obj_heap(p_base)
		&& flags == x_obj_flags(p_obj)
		&& value == x_int(x_obj_data(p_obj))
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	return NULL;
}

static char *test_mksatomown(void)
{
	x_obj_t *p_base, *p_obj;
	unsigned int flags = rand();
	x_int_t value = rand();

	helper_alloc_reset();

	p_obj = x_mksatomown(NULL, flags, value);
	_it_should("make an atom with the supplied flags and value",
		x_obj_type_issatom(p_obj)
		&& (flags | X_OBJ_FLAG_OWN) == x_obj_flags(p_obj)
		&& value == x_int(x_obj_data(p_obj))
	);
	x_obj_free(NULL, p_obj);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mksatomown(p_base, flags, value);
	_it_should("make an atom with the supplied base, flags and value",
		x_obj_type_issatom(p_obj)
		&& p_obj == x_obj_heap(p_base)
		&& (flags | X_OBJ_FLAG_OWN) == x_obj_flags(p_obj)
		&& value == x_int(x_obj_data(p_obj))
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	return NULL;
}

static char *test_mkspair(void)
{
	x_obj_t *p_base, *p_obj;
	unsigned int flags = rand();
	x_int_t values[2] = { rand(), rand() };

	helper_alloc_reset();

	p_obj = x_mkspair(NULL, flags, values[0], values[1]);
	_it_should("make a pair with the supplied flags and values",
		x_obj_type_isspair(p_obj)
		&& flags == x_obj_flags(p_obj)
		&& values[0] == x_int(x_obj_data(p_obj))
		&& values[1] == x_int(x_obj_data_ptr(p_obj)[1])
	);
	x_obj_free(NULL, p_obj);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mkspair(p_base, flags, values[0], values[1]);
	_it_should("make a pair with the supplied base, flags and values",
		x_obj_type_isspair(p_obj)
		&& p_obj == x_obj_heap(p_base)
		&& flags == x_obj_flags(p_obj)
		&& values[0] == x_int(x_obj_data(p_obj))
		&& values[1] == x_int(x_obj_data_ptr(p_obj)[1])
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	return NULL;
}

static char *test_obj(void)
{
	x_obj_t other;
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &other});

	_it_should("return the object pointer value",
		&other == x_obj(x_obj_data(obj))
	);

	return NULL;
}

static char *test_fn(void)
{
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.fn = test_prim_fn});

	_it_should("return the function pointer value",
		test_prim_fn == x_fn(x_obj_data(obj))
	);

	return NULL;
}

static char *test_int(void)
{
	x_int_t i = rand();
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = i});

	_it_should("return the integer value",
		i == x_int(x_obj_data(obj))
	);

	return NULL;
}

static char *test_char(void)
{
	x_char_t c = rand();
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.c = c});

	_it_should("return the character value",
		c == x_char(x_obj_data(obj))
	);

	return NULL;
}

static char *test_str(void)
{
	char *s = "test";
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)s});

	_it_should("return the string value",
		s == x_str(x_obj_data(obj))
	);

	return NULL;
}

static char *test_ptr(void)
{
	int dummy;
	void *ptr = &dummy;
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.v = ptr});

	_it_should("return the void pointer value",
		ptr == x_ptr(x_obj_data(obj))
	);

	return NULL;
}

static char *test_first(void)
{
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0});

	_it_should("return the first data element",
		x_obj_data_ptr(obj) == &x_first(obj)
	);

	return NULL;
}

static char *test_second(void)
{
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.i = 0});

	_it_should("return the second data element",
		x_obj_data_ptr(obj) + 1 == &x_second(obj)
	);

	return NULL;
}

static char *test_rest(void)
{
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.i = 0});

	_it_should("return the rest data element",
		x_obj_data_ptr(obj) + 1 == &x_rest(obj)
	);

	return NULL;
}

static char *test_firstptr(void)
{
	int dummy;
	void *ptr = &dummy;
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.v = ptr});

	_it_should("return the void pointer value",
		ptr == x_firstptr(obj)
	);

	return NULL;
}

static char *test_firstobj(void)
{
	x_obj_t other;
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &other});

	_it_should("return the object pointer value",
		&other == x_firstobj(obj)
	);

	return NULL;
}

static char *test_firstint(void)
{
	x_int_t i = rand();
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = i});

	_it_should("return the integer value",
		i == x_firstint(obj)
	);

	return NULL;
}

static char *test_firstchar(void)
{
	x_char_t c = rand();
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.c = c});

	_it_should("return the character value",
		c == x_firstchar(obj)
	);

	return NULL;
}

static char *test_firststr(void)
{
	char *s = "test";
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)s});

	_it_should("return the string value",
		s == x_firststr(obj)
	);

	return NULL;
}

static char *test_firstfn(void)
{
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.fn = test_prim_fn});

	_it_should("return the function pointer value",
		test_prim_fn == x_firstfn(obj)
	);

	return NULL;
}

static char *test_secondptr(void)
{
	int dummy;
	void *ptr = &dummy;
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.v = ptr});

	_it_should("return the void pointer value",
		ptr == x_secondptr(obj)
	);

	return NULL;
}

static char *test_secondobj(void)
{
	x_obj_t other;
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.p = &other});

	_it_should("return the object pointer value",
		&other == x_secondobj(obj)
	);

	return NULL;
}

static char *test_secondint(void)
{
	x_int_t i = rand();
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.i = i});

	_it_should("return the integer value",
		i == x_secondint(obj)
	);

	return NULL;
}

static char *test_secondchar(void)
{
	x_char_t c = rand();
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.c = c});

	_it_should("return the character value",
		c == x_secondchar(obj)
	);

	return NULL;
}

static char *test_secondstr(void)
{
	char *s = "test";
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.s = (x_char_t *)s});

	_it_should("return the string value",
		s == x_secondstr(obj)
	);

	return NULL;
}

static char *test_secondfn(void)
{
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.fn = test_prim_fn});

	_it_should("return the function pointer value",
		test_prim_fn == x_secondfn(obj)
	);

	return NULL;
}

static char *test_atomptr(void)
{
	int dummy;
	void *ptr = &dummy;
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.v = ptr});

	_it_should("return the void pointer value",
		ptr == x_atomptr(obj)
	);

	return NULL;
}

static char *test_atomobj(void)
{
	x_obj_t other;
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &other});

	_it_should("return the object pointer value",
		&other == x_atomobj(obj)
	);

	return NULL;
}

static char *test_atomint(void)
{
	x_int_t i = rand();
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = i});

	_it_should("return the integer value",
		i == x_atomint(obj)
	);

	return NULL;
}

static char *test_atomchar(void)
{
	x_char_t c = rand();
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.c = c});

	_it_should("return the character value",
		c == x_atomchar(obj)
	);

	return NULL;
}

static char *test_atomstr(void)
{
	char *s = "test";
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.s = (x_char_t *)s});

	_it_should("return the string value",
		s == x_atomstr(obj)
	);

	return NULL;
}

static char *test_atomfn(void)
{
	x_satom_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.fn = test_prim_fn});

	_it_should("return the function pointer value",
		test_prim_fn == x_atomfn(obj)
	);

	return NULL;
}

static char *test_restptr(void)
{
	int dummy;
	void *ptr = &dummy;
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.v = ptr});

	_it_should("return the void pointer value",
		ptr == x_restptr(obj)
	);

	return NULL;
}

static char *test_restobj(void)
{
	x_obj_t other;
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.p = &other});

	_it_should("return the object pointer value",
		&other == x_restobj(obj)
	);

	return NULL;
}

static char *test_restint(void)
{
	x_int_t i = rand();
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.i = i});

	_it_should("return the integer value",
		i == x_restint(obj)
	);

	return NULL;
}

static char *test_restchar(void)
{
	x_char_t c = rand();
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.c = c});

	_it_should("return the character value",
		c == x_restchar(obj)
	);

	return NULL;
}

static char *test_reststr(void)
{
	char *s = "test";
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.s = (x_char_t *)s});

	_it_should("return the string value",
		s == x_reststr(obj)
	);

	return NULL;
}

static char *test_restfn(void)
{
	x_spair_t obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0}, {.fn = test_prim_fn});

	_it_should("return the function pointer value",
		test_prim_fn == x_restfn(obj)
	);

	return NULL;
}

static char *test_obj_accessors(void)
{
	/* 16 leaf nodes for depth-4 tree */
	x_obj_t n[16];

	/* depth 3: pairs of leaves */
	x_spair_t d3[8] = {
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[0]},  {.p = &n[1]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[2]},  {.p = &n[3]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[4]},  {.p = &n[5]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[6]},  {.p = &n[7]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[8]},  {.p = &n[9]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[10]}, {.p = &n[11]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[12]}, {.p = &n[13]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[14]}, {.p = &n[15]})
	};

	/* depth 2 */
	x_spair_t d2[4] = {
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d3[0]}, {.p = d3[1]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d3[2]}, {.p = d3[3]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d3[4]}, {.p = d3[5]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d3[6]}, {.p = d3[7]})
	};

	/* depth 1 */
	x_spair_t d1[2] = {
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d2[0]}, {.p = d2[1]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d2[2]}, {.p = d2[3]})
	};

	/* root */
	x_spair_t root = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d1[0]}, {.p = d1[1]});

	/* depth 1 */
	_it_should("x_0",  d1[0] == x_0(root));
	_it_should("x_1",  d1[1] == x_1(root));

	/* depth 2 */
	_it_should("x_00", d2[0] == x_00(root));
	_it_should("x_01", d2[2] == x_01(root));
	_it_should("x_10", d2[1] == x_10(root));
	_it_should("x_11", d2[3] == x_11(root));

	/* depth 3 */
	_it_should("x_000", d3[0] == x_000(root));
	_it_should("x_001", d3[4] == x_001(root));
	_it_should("x_010", d3[2] == x_010(root));
	_it_should("x_011", d3[6] == x_011(root));
	_it_should("x_100", d3[1] == x_100(root));
	_it_should("x_101", d3[5] == x_101(root));
	_it_should("x_110", d3[3] == x_110(root));
	_it_should("x_111", d3[7] == x_111(root));

	/* depth 4 */
	_it_should("x_0000", &n[0]  == x_0000(root));
	_it_should("x_0001", &n[8]  == x_0001(root));
	_it_should("x_0010", &n[4]  == x_0010(root));
	_it_should("x_0011", &n[12] == x_0011(root));
	_it_should("x_0100", &n[2]  == x_0100(root));
	_it_should("x_0101", &n[10] == x_0101(root));
	_it_should("x_0110", &n[6]  == x_0110(root));
	_it_should("x_0111", &n[14] == x_0111(root));
	_it_should("x_1000", &n[1]  == x_1000(root));
	_it_should("x_1001", &n[9]  == x_1001(root));
	_it_should("x_1010", &n[5]  == x_1010(root));
	_it_should("x_1011", &n[13] == x_1011(root));
	_it_should("x_1100", &n[3]  == x_1100(root));
	_it_should("x_1101", &n[11] == x_1101(root));
	_it_should("x_1110", &n[7]  == x_1110(root));
	_it_should("x_1111", &n[15] == x_1111(root));

	return NULL;
}

static char *test_obj_is_nil(void)
{
	x_obj_t *p_base, *p_obj;

	helper_alloc_reset();

	_it_should("return true when base is NULL and value is NULL",
		1 == x_obj_isnil(NULL, NULL)
	);

	p_obj = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("return false when base is NULL and value is an object",
		0 == x_obj_isnil(NULL, p_obj)
	);
	x_obj_free(NULL, p_obj);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("return false when base is an object and value is base",
		0 == x_obj_isnil(p_base, p_base)
	);

	_it_should("return true when base is an object and value is NULL",
		1 == x_obj_isnil(p_base, NULL)
	);

	p_obj = x_mksatom(p_base, X_OBJ_FLAG_NONE, 0);
	_it_should("return false when base is an object and value is another object",
		0 == x_obj_isnil(p_base, p_obj)
	);

	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	return NULL;
}

static char *test_obj_sys_alloc(void)
{
	x_obj_t *p_obj[3];

	helper_alloc_reset();

	p_obj[0] = x_obj_alloc(NULL, (x_obj_t *)x_type_atom_obj, 2, 3);
	_it_should("make a new object", p_obj[0] != NULL);
	_it_should("set the new object's gc pointer to NULL", x_obj_heap(p_obj[0]) == NULL);
	_it_should("set the new object's type to the value given", x_obj_type(p_obj[0]) == (x_obj_t *)x_type_atom_obj);
	_it_should("set the new object's flags to the value given", x_obj_flags(p_obj[0]) == 2);

	p_obj[1] = x_obj_alloc(p_obj[0], (x_obj_t *)x_type_atom_obj, 3, 4);
	_it_should("make a new object", p_obj[1] != NULL);
	_it_should("set the new object's gc pointer to NULL", x_obj_heap(p_obj[1]) == NULL);
	_it_should("set the new object's type to the value given", x_obj_type(p_obj[1]) == (x_obj_t *)x_type_atom_obj);
	_it_should("set the new object's flags to the value given", x_obj_flags(p_obj[1]) == 3);
	_it_should("set the p_base object's gc pointer to the new object", x_obj_heap(p_obj[0]) == p_obj[1]);

	p_obj[2] = x_obj_alloc(p_obj[0], (x_obj_t *)x_type_atom_obj, 4, 5);
	_it_should("make a new object", p_obj[2] != NULL);
	_it_should("set the new object's gc pointer to obj1", x_obj_heap(p_obj[2]) == p_obj[1]);
	_it_should("set the new object's type to the value given", x_obj_type(p_obj[2]) == (x_obj_t *)x_type_atom_obj);
	_it_should("set the new object's flags to the value given", x_obj_flags(p_obj[2]) == 4);
	_it_should("set the p_base object's gc pointer to the new object", x_obj_heap(p_obj[0]) == p_obj[2]);

	x_sys_free(p_obj[2]);
	x_sys_free(p_obj[1]);
	x_sys_free(p_obj[0]);

	return NULL;
}

static char *test_obj_sys_make(void)
{
	x_obj_t *p_obj;

	helper_alloc_reset();

	p_obj = x_obj_make(NULL, (x_obj_t *)x_type_atom_obj, 2, 3, (void *)4, (void *)5);
	_it_should("make a new object", p_obj != NULL);
	_it_should("set the new object's gc pointer to NULL", x_obj_heap(p_obj) == NULL);
	_it_should("set the new object's type to the value given", x_obj_type(p_obj) == (x_obj_t *)x_type_atom_obj);
	_it_should("set the new object's flags to the value given", x_obj_flags(p_obj) == 2);
	_it_should("set the new object's first element", x_obj_data_ptr(p_obj)[0].p == (void *)4);
	_it_should("set the new object's second element", x_obj_data_ptr(p_obj)[1].p == (void *)5);

	x_sys_free(p_obj);

	return NULL;
}

static char *test_obj_sys_free(void)
{
	x_obj_t *p_obj, *p_own;

	helper_alloc_reset();

	p_obj = x_obj_alloc(NULL, (x_obj_t *)x_type_atom_obj, 0, 0);
	_it_should("allocate an object", helper_alloc_count() == 1);
	x_obj_free(NULL, p_obj);
	_it_should("free the object", helper_free_count() == 1);

	helper_alloc_reset();

	p_own = x_sys_malloc(16);
	_it_should("allocate some memory", helper_alloc_count() == 1);
	p_obj = x_obj_make(NULL, (x_obj_t *)x_type_atom_obj, X_OBJ_FLAG_OWN, 1, p_own);
	_it_should("allocate an object", helper_alloc_count() == 2);
	x_obj_free(NULL, p_obj);
	_it_should("free the object and OWNed data", helper_free_count() == 2);

	return NULL;
}

static char *test_obj_prim_type_name(void)
{
	x_obj_t *p_base, *p_obj, *p_args, *p_ret;

	helper_alloc_reset();

	p_ret = x_obj_prim_type_name(NULL, NULL);
	_it_should("return the base when args is NULL",
		x_obj_isnil(NULL, p_ret)
	);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_ret = x_obj_prim_type_name(p_base, p_base);
	_it_should("return the base when args is nil",
		x_obj_isnil(p_base, p_ret)
	);
	x_obj_free(NULL, p_base);


	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, NULL, NULL);
	p_ret = x_obj_prim_type_name(NULL, p_args);
	_it_should("return the base when first element of args is NULL",
		x_obj_isnil(NULL, p_ret)
	);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_args = x_mkspair(p_base, X_OBJ_FLAG_NONE, p_base, p_base);
	p_ret = x_obj_prim_type_name(p_base, p_args);
	_it_should("return atom type name when first arg is a simple atom",
		(x_obj_t *)x_type_atom_obj == p_ret
	);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_base);


	p_obj = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_obj, NULL);
	p_ret = x_obj_prim_type_name(NULL, p_args);
	_it_should("return atom's type name when base is NULL",
		 (x_obj_t *)x_type_atom_obj == p_ret
	);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_obj);

	p_obj = x_mkspair(NULL, X_OBJ_FLAG_NONE, 0, 0);
	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_obj, NULL);
	p_ret = x_obj_prim_type_name(NULL, p_args);
	_it_should("return pair's type name when base is NULL",
		(x_obj_t *)x_type_pair_obj == p_ret
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_args);


	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mksatom(p_base, X_OBJ_FLAG_NONE, 0);
	p_args = x_mkspair(p_base, X_OBJ_FLAG_NONE, p_obj, p_base);
	p_ret = x_obj_prim_type_name(p_base, p_args);
	_it_should("return atom's type name when base is empty",
		(x_obj_t *)x_type_atom_obj == p_ret
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mkspair(p_base, X_OBJ_FLAG_NONE, 0, 0);
	p_args = x_mkspair(p_base, X_OBJ_FLAG_NONE, p_obj, p_base);
	p_ret = x_obj_prim_type_name(p_base, p_args);
	_it_should("return pair's type name when base is empty",
		(x_obj_t *)x_type_pair_obj == p_ret
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_base);

	return NULL;
}

static char *test_obj_type_name(void)
{
	x_obj_t *p_base, *p_obj;
	x_char_t *s;

	helper_alloc_reset();

	p_obj = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	s = x_obj_type_name(NULL, p_obj);
	_it_should("return atom's type name when base is NULL",
		0 == strcmp(X_TYPE_ATOM_NAME, s)
	);
	x_obj_free(NULL, p_obj);

	p_obj = x_mkspair(NULL, X_OBJ_FLAG_NONE, 0, 0);
	s = x_obj_type_name(NULL, p_obj);
	_it_should("return pair's type name when base is NULL",
		0 == strcmp(X_TYPE_PAIR_NAME, s)
	);
	x_obj_free(NULL, p_obj);


	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mksatom(p_base, X_OBJ_FLAG_NONE, 0);
	s = x_obj_type_name(p_base, p_obj);
	_it_should("return atom's type name when base is empty",
		0 == strcmp(X_TYPE_ATOM_NAME, s)
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mkspair(p_base, X_OBJ_FLAG_NONE, 0, 0);
	s = x_obj_type_name(p_base, p_obj);
	_it_should("return pair's type name when base is empty",
		0 == strcmp(X_TYPE_PAIR_NAME, s)
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	return NULL;
}

x_obj_t *test_units(x_obj_t *p_base, x_obj_t *p_args)
{
	return x_firstobj(p_args);
}

x_satom_t test_units_prim = x_obj_set((x_obj_t *)x_type_atom_obj, X_OBJ_FLAG_NONE, { .fn = test_units });

static char *test_obj_prim_units(void)
{
	x_obj_t *p_base, *p_obj, *p_args, *p_ret;

	helper_alloc_reset();

	p_ret = x_obj_prim_units(NULL, NULL);
	_it_should("return NULL when args is NULL",
		x_obj_isnil(NULL, p_ret)
	);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_ret = x_obj_prim_units(p_base, p_base);
	_it_should("return nil when args is nil",
		x_obj_isnil(p_base, p_ret)
	);
	x_obj_free(NULL, p_base);


	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, NULL, NULL);
	p_ret = x_obj_prim_units(NULL, p_args);
	_it_should("return NULL when first element of args is NULL",
		x_obj_isnil(NULL, p_ret)
	);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_args = x_mkspair(p_base, X_OBJ_FLAG_NONE, p_base, p_base);
	p_ret = x_obj_prim_units(p_base, p_args);
	_it_should("return atom units when first arg is a simple atom",
		p_ret == x_type_units_atom_obj
	);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_base);


	p_obj = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_obj, NULL);
	p_ret = x_obj_prim_units(NULL, p_args);
	_it_should("return atom's size in units when base is NULL",
		 (x_obj_t *)x_type_units_atom_obj == p_ret
	);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_obj);

	p_obj = x_mkspair(NULL, X_OBJ_FLAG_NONE, 0, 0);
	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_obj, NULL);
	p_ret = x_obj_prim_units(NULL, p_args);
	_it_should("return pair's size in units when base is NULL",
		(x_obj_t *)x_type_units_pair_obj == p_ret
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_args);


	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mksatom(p_base, X_OBJ_FLAG_NONE, 0);
	p_args = x_mkspair(p_base, X_OBJ_FLAG_NONE, p_obj, p_base);
	p_ret = x_obj_prim_units(p_base, p_args);
	_it_should("return atom's size in units when base is empty",
		(x_obj_t *)x_type_units_atom_obj == p_ret
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mkspair(p_base, X_OBJ_FLAG_NONE, 0, 0);
	p_args = x_mkspair(p_base, X_OBJ_FLAG_NONE, p_obj, p_base);
	p_ret = x_obj_prim_units(p_base, p_args);
	_it_should("return pair's size in units when base is empty",
		(x_obj_t *)x_type_units_pair_obj == p_ret
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_base);

	return NULL;
}

static char *test_obj_units(void)
{
	x_obj_t *p_base, *p_obj;
	int i;

	p_obj = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	i = x_obj_units(NULL, p_obj);
	_it_should("return atoms's size in units when base is NULL",
		X_OBJ_UNITS_ATOM == i
	);
	x_obj_free(NULL, p_obj);

	p_obj = x_mkspair(NULL, X_OBJ_FLAG_NONE, 0, 0);
	i = x_obj_units(NULL, p_obj);
	_it_should("return pair's size in units when base is NULL",
		X_OBJ_UNITS_PAIR == i
	);
	x_obj_free(NULL, p_obj);


	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mksatom(p_base, X_OBJ_FLAG_NONE, 0);
	i = x_obj_units(p_base, p_obj);
	_it_should("return atom's size in units when base is empty",
		X_OBJ_UNITS_ATOM == i
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mkspair(p_base, X_OBJ_FLAG_NONE, 0, 0);
	i = x_obj_units(p_base, p_obj);
	_it_should("return pair's size in units when base is empty",
		X_OBJ_UNITS_PAIR == i
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	return NULL;
}

x_obj_t *test_length(x_obj_t *p_base, x_obj_t *p_args)
{
	return x_firstobj(p_args);
}

x_satom_t test_length_prim = x_obj_set((x_obj_t *)x_type_atom_obj, X_OBJ_FLAG_NONE, { .fn = test_length });

static char *test_obj_prim_length(void)
{
	x_obj_t *p_base, *p_obj, *p_args, *p_ret;

	helper_alloc_reset();

	p_ret = x_obj_prim_length(NULL, NULL);
	_it_should("return NULL when args is NULL",
		x_obj_isnil(NULL, p_ret)
	);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_ret = x_obj_prim_length(p_base, p_base);
	_it_should("return nil when args is nil",
		x_obj_isnil(p_base, p_ret)
	);
	x_obj_free(NULL, p_base);


	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, NULL, NULL);
	p_ret = x_obj_prim_length(NULL, p_args);
	_it_should("return NULL when first element of args is NULL",
		x_obj_isnil(NULL, p_ret)
	);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_args = x_mkspair(p_base, X_OBJ_FLAG_NONE, p_base, p_base);
	p_ret = x_obj_prim_length(p_base, p_args);
	_it_should("return atom length when first arg is a simple atom",
		p_ret == x_type_length_atom_obj
	);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_base);


	p_obj = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_obj, NULL);
	p_ret = x_obj_prim_length(NULL, p_args);
	_it_should("return atom's length when base is NULL",
		 (x_obj_t *)x_type_length_atom_obj == p_ret
	);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_obj);

	p_obj = x_mkspair(NULL, X_OBJ_FLAG_NONE, 0, 0);
	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_obj, NULL);
	p_ret = x_obj_prim_length(NULL, p_args);
	_it_should("return pair's length when base is NULL",
		(x_obj_t *)x_type_length_pair_obj == p_ret
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_args);


	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mksatom(p_base, X_OBJ_FLAG_NONE, 0);
	p_args = x_mkspair(p_base, X_OBJ_FLAG_NONE, p_obj, p_base);
	p_ret = x_obj_prim_length(p_base, p_args);
	_it_should("return atom's length when base is empty",
		(x_obj_t *)x_type_length_atom_obj == p_ret
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mkspair(p_base, X_OBJ_FLAG_NONE, 0, 0);
	p_args = x_mkspair(p_base, X_OBJ_FLAG_NONE, p_obj, p_base);
	p_ret = x_obj_prim_length(p_base, p_args);
	_it_should("return pair's length when base is empty",
		(x_obj_t *)x_type_length_pair_obj == p_ret
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_base);

	return NULL;
}

static char *test_obj_length(void)
{
	x_obj_t *p_base, *p_obj;
	int i;

	helper_alloc_reset();

	p_obj = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	i = x_obj_length(NULL, p_obj);
	_it_should("return atoms's length when base is NULL and type is an integer",
		X_OBJ_LENGTH_ATOM == i
	);
	x_obj_free(NULL, p_obj);

	p_obj = x_mkspair(NULL, X_OBJ_FLAG_NONE, 0, 0);
	i = x_obj_length(NULL, p_obj);
	_it_should("return pair's length when base is NULL and type is an integer",
		X_OBJ_LENGTH_PAIR == i
	);
	x_obj_free(NULL, p_obj);


	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mksatom(p_base, X_OBJ_FLAG_NONE, 0);
	i = x_obj_length(p_base, p_obj);
	_it_should("return atom's length when base is empty and type is an integer",
		X_OBJ_LENGTH_ATOM == i
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj = x_mkspair(p_base, X_OBJ_FLAG_NONE, 0, 0);
	i = x_obj_length(p_base, p_obj);
	_it_should("return pair's length when base is empty and type is an integer",
		X_OBJ_LENGTH_PAIR == i
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_base);

	return NULL;
}

static char *test_atom_prim_units(void)
{
	x_obj_t *p_atom, *p_args, *p_ret;

	helper_alloc_reset();

	p_atom = x_mksatom(NULL, X_OBJ_FLAG_NONE, 42);
	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_atom, NULL);
	p_ret = x_atom_prim_units(NULL, p_args);
	_it_should("return the atom units object", p_ret == x_type_units_atom_obj);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_atom);

	return NULL;
}

static char *test_pair_prim_units(void)
{
	x_obj_t *p_pair, *p_args, *p_ret;

	helper_alloc_reset();

	p_pair = x_mkspair(NULL, X_OBJ_FLAG_NONE, 1, 2);
	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_pair, NULL);
	p_ret = x_pair_prim_units(NULL, p_args);
	_it_should("return the pair units object", p_ret == x_type_units_pair_obj);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_pair);

	return NULL;
}

static char *test_atom_prim_length(void)
{
	x_obj_t *p_atom, *p_args, *p_ret;

	helper_alloc_reset();

	p_atom = x_mksatom(NULL, X_OBJ_FLAG_NONE, 42);
	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_atom, NULL);
	p_ret = x_atom_prim_length(NULL, p_args);
	_it_should("return the atom length object", p_ret == x_type_length_atom_obj);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_atom);

	return NULL;
}

static char *test_pair_prim_length(void)
{
	x_obj_t *p_pair, *p_args, *p_ret;

	helper_alloc_reset();

	p_pair = x_mkspair(NULL, X_OBJ_FLAG_NONE, 1, 2);
	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_pair, NULL);
	p_ret = x_pair_prim_length(NULL, p_args);
	_it_should("return the pair length object", p_ret == x_type_length_pair_obj);
	x_obj_free(NULL, p_args);
	x_obj_free(NULL, p_pair);

	return NULL;
}

static char *test_obj_error(void)
{
	x_obj_t *p_obj;
	char buffer[4096];

	helper_alloc_reset();
	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();

	helper_sys_exit_status = X_SYS_EXIT_SUCCESS;
	memset(buffer, 0, sizeof(buffer));
	x_obj_error(NULL, (x_char_t *)"nil error", NULL);
	_it_should("write error and exit on nil",
		X_SYS_EXIT_FAILURE == helper_sys_exit_status
		&& 0 == strncmp(buffer, "*** ERROR: ", 11)
	);

	helper_file_reset();
	helper_sys_exit_status = X_SYS_EXIT_SUCCESS;
	memset(buffer, 0, sizeof(buffer));
	p_obj = x_mksatom(NULL, X_OBJ_FLAG_NONE, "mysym");
	x_obj_error(NULL, (x_char_t *)"symbol error", p_obj);
	_it_should("include the symbol name for an atom",
		X_SYS_EXIT_FAILURE == helper_sys_exit_status
		&& NULL != strstr(buffer, "'mysym")
	);
	x_obj_free(NULL, p_obj);

	return NULL;
}

static char *test_obj_debug(void)
{
	char buffer[4096];

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();
	memset(buffer, 0, sizeof(buffer));

	_x_obj_debug(__FILE__, __LINE__, NULL, "obj %d", 42);
	_it_should("write debug output",
		x_lib_strlen((x_char_t *)buffer) > 0
		&& NULL != strstr(buffer, "obj 42")
	);

	return NULL;
}

static char *test_obj_dump(void)
{
	x_obj_t *p_a, *p_b, *p_obj;
	char buffer[4096];

	helper_alloc_reset();
	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();

	memset(buffer, 0, sizeof(buffer));
	_x_obj_dump(__FILE__, __LINE__, NULL, NULL, "dump-nil");
	_it_should("dump nil with NIL type",
		NULL != strstr(buffer, "dump-nil")
		&& NULL != strstr(buffer, X_TYPE_NIL_NAME)
	);

	helper_file_reset();
	memset(buffer, 0, sizeof(buffer));
	p_obj = x_mksatom(NULL, X_OBJ_FLAG_NONE, 42);
	_x_obj_dump(__FILE__, __LINE__, NULL, p_obj, "dump-atom");
	_it_should("dump atom with ATOM type",
		NULL != strstr(buffer, "dump-atom")
		&& NULL != strstr(buffer, X_TYPE_ATOM_NAME)
	);
	x_obj_free(NULL, p_obj);

	helper_file_reset();
	memset(buffer, 0, sizeof(buffer));
	p_a = x_mksatom(NULL, X_OBJ_FLAG_NONE, 1);
	p_b = x_mksatom(NULL, X_OBJ_FLAG_NONE, 2);
	p_obj = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_a, p_b);
	_x_obj_dump(__FILE__, __LINE__, NULL, p_obj, "dump-pair");
	_it_should("dump pair with PAIR type",
		NULL != strstr(buffer, "dump-pair")
		&& NULL != strstr(buffer, X_TYPE_PAIR_NAME)
	);
	x_obj_free(NULL, p_obj);
	x_obj_free(NULL, p_b);
	x_obj_free(NULL, p_a);

	return NULL;
}

static void helper_x_debug_va(int fd, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	x_debug_va(fd, fmt, ap);
	va_end(ap);
}

static void helper_x_obj_debug_va(x_obj_t *p_base, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	x_obj_debug_va(p_base, fmt, ap);
	va_end(ap);
}

static char *test_debug_va_macros(void)
{
	char buffer[4096];

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();

	memset(buffer, 0, sizeof(buffer));
	helper_x_debug_va(TEST_HELPER_FILE_STDERR, "va-%s", "test");
	_it_should("x_debug_va write output",
		x_lib_strlen((x_char_t *)buffer) > 0
	);

	helper_file_reset();
	memset(buffer, 0, sizeof(buffer));
	helper_x_obj_debug_va(NULL, "va-obj-%s", "test");
	_it_should("x_obj_debug_va write output",
		x_lib_strlen((x_char_t *)buffer) > 0
	);

	return NULL;
}

static char *run_tests()
{
	_run_test(test_meta_layout);
	_run_test(test_flag_constants);

	_run_test(test_obj_heap);
	_run_test(test_obj_type);
	_run_test(test_obj_flags);
	_run_test(test_obj_data_ptr);
	_run_test(test_obj_data_i);
	_run_test(test_obj_data);
	_run_test(test_obj_meta_i);

	_run_test(test_obj_set);

	_run_test(test_obj_type_isnil);
	_run_test(test_obj_is_type);

	_run_test(test_obj_type_issatom);
	_run_test(test_obj_type_isspair);

	_run_test(test_mksatom);
	_run_test(test_mksatomown);
	_run_test(test_mkspair);

	_run_test(test_obj);
	_run_test(test_fn);
	_run_test(test_int);
	_run_test(test_char);
	_run_test(test_str);
	_run_test(test_ptr);

	_run_test(test_first);
	_run_test(test_second);
	_run_test(test_rest);

	_run_test(test_firstptr);
	_run_test(test_firstobj);
	_run_test(test_firstint);
	_run_test(test_firstchar);
	_run_test(test_firststr);
	_run_test(test_firstfn);

	_run_test(test_secondptr);
	_run_test(test_secondobj);
	_run_test(test_secondint);
	_run_test(test_secondchar);
	_run_test(test_secondstr);
	_run_test(test_secondfn);

	_run_test(test_atomptr);
	_run_test(test_atomobj);
	_run_test(test_atomint);
	_run_test(test_atomchar);
	_run_test(test_atomstr);
	_run_test(test_atomfn);

	_run_test(test_restptr);
	_run_test(test_restobj);
	_run_test(test_restint);
	_run_test(test_restchar);
	_run_test(test_reststr);
	_run_test(test_restfn);

	_run_test(test_obj_accessors);

	_run_test(test_obj_is_nil);

	_run_test(test_obj_sys_alloc);
	_run_test(test_obj_sys_make);
	_run_test(test_obj_sys_free);

	_run_test(test_obj_prim_type_name);
	_run_test(test_obj_type_name);

	_run_test(test_obj_prim_units);
	_run_test(test_obj_units);
	_run_test(test_obj_prim_length);
	_run_test(test_obj_length);

	_run_test(test_atom_prim_units);
	_run_test(test_pair_prim_units);
	_run_test(test_atom_prim_length);
	_run_test(test_pair_prim_length);

	_run_test(test_obj_error);

	_run_test(test_obj_debug);
	_run_test(test_obj_dump);
	_run_test(test_debug_va_macros);

	return NULL;
}
