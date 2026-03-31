/*
 * # Unit Tests: *x-obj*
 */

#define TEST_RUNNER_OVERHEAD
#include "test-runner.h"

/* No Garbage Collection structures. */
#ifdef X_HEAP
#undef X_HEAP
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


/*
 * ## Test Runners
 */
static char *test_meta_layout(void)
{
	_it_should("have no heap unit",
		0 == X_OBJ_UNITS_HEAP
	);

	_it_should("have type at index 0",
		0 == X_OBJ_META_TYPE
	);

	_it_should("have flags at index 1",
		1 == X_OBJ_META_FLAGS
	);

	_it_should("have meta length of 2",
		2 == X_OBJ_META_LEN
	);

	return NULL;
}

static char *test_flag_constants(void)
{
	_it_should("not define X_OBJ_FLAG_SHARED or X_OBJ_FLAG_HEAP",
		0xFF == X_OBJ_FLAG_MASK
	);

	return NULL;
}

static char *test_obj_set(void)
{
	x_obj_t *type = (x_obj_t *)(x_int_t)rand();
	x_int_t flags = rand(), d0 = rand();
	x_satom_t obj = x_obj_set(type, flags, {.i = d0});

	_it_should("initialize without a heap slot",
		type == obj[0].p
		&& flags == obj[1].i
		&& d0 == obj[2].i
	);

	return NULL;
}

static char *test_obj_alloc(void)
{
	x_obj_t *p_obj;
	x_int_t flags = rand();

	helper_alloc_reset();

	p_obj = x_obj_alloc(NULL, (x_obj_t *)x_type_atom_obj, flags, X_OBJ_UNITS_ATOM);
	_it_should("allocate an object with type and flags",
		NULL != p_obj
		&& (x_obj_t *)x_type_atom_obj == x_obj_type(p_obj)
		&& flags == x_obj_flags(p_obj)
	);
	x_obj_free(NULL, p_obj);

	return NULL;
}

static char *test_obj_make(void)
{
	x_obj_t *p_obj;
	x_int_t flags = rand(), d0 = rand(), d1 = rand();

	helper_alloc_reset();

	p_obj = x_obj_make(NULL, (x_obj_t *)x_type_pair_obj, flags, X_OBJ_LENGTH_PAIR, d0, d1);
	_it_should("make a pair with type, flags, and data",
		NULL != p_obj
		&& (x_obj_t *)x_type_pair_obj == x_obj_type(p_obj)
		&& flags == x_obj_flags(p_obj)
		&& d0 == x_firstint(p_obj)
		&& d1 == x_restint(p_obj)
	);
	x_obj_free(NULL, p_obj);

	return NULL;
}

static char *test_obj_free(void)
{
	x_obj_t *p_obj;

	helper_alloc_reset();

	p_obj = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("allocate an object",
		NULL != p_obj
	);

	x_obj_free(NULL, p_obj);
	_it_should("free the object",
		1 == helper_free_count()
	);

	return NULL;
}

static char *run_tests() {
	_run_test(test_meta_layout);
	_run_test(test_flag_constants);
	_run_test(test_obj_set);
	_run_test(test_obj_alloc);
	_run_test(test_obj_make);
	_run_test(test_obj_free);

	return NULL;
}
