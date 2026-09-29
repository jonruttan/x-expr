/*
 * # Unit Tests: *x-base*
 */

#define TEST_RUNNER_OVERHEAD
#include "test-runner.h"

/* Include Heap structures. */
#ifndef X_HEAP
#define X_HEAP
#endif /* X_HEAP */

#include "test-helper-system.c"

#include "src/x-sys.c"
#include "src/x-stdlib.c"
#include "src/x-lib.c"
#include "src/x.c"
#include "src/x-obj.c"
#include "src/x-base.c"


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
 * ## Test Helpers
 */
static struct x_base_t test_base_defaults(void)
{
	struct x_base_t base = {
		0, 0, 0,
		0,
		0, NULL
	};
	return base;
}

static x_obj_t *_slot_fn_a(x_obj_t *p_base, x_obj_t *p_args)
{
	return p_args;
}

static x_obj_t *_slot_fn_b(x_obj_t *p_base, x_obj_t *p_args)
{
	return NULL;
}

static x_obj_t *test_make_base(x_obj_t *p_parent)
{
	struct x_base_t base = test_base_defaults();

	base.filein = STDIN_FILENO;
	base.fileout = STDOUT_FILENO;
	base.fileerr = STDERR_FILENO;

	return x_base_make(p_parent, base);
}


/*
 * ## Test Runners
 */
static char *test_base_isset(void)
{
	x_obj_t *p_base;

	helper_alloc_reset();

	p_base = NULL;
	_it_should("return false when base is NULL",
		! x_base_isset(p_base)
	);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("return false when base data is nil",
		! x_base_isset(p_base)
	);
	x_obj_free(NULL, p_base);

	p_base = test_make_base(NULL);
	_it_should("return true when base is set",
		x_base_isset(p_base)
	);
	x_obj_free(NULL, p_base);

	return NULL;
}

static char *test_base_make(void)
{
	struct x_base_t base = test_base_defaults();
	x_obj_t *p_base;
	x_int_t filein = rand(), fileout = rand(), fileerr = rand(),
		obj_meta_extra = rand();

	helper_alloc_reset();

	base.filein = filein;
	base.fileout = fileout;
	base.fileerr = fileerr;
	base.obj_meta_extra = obj_meta_extra;

	p_base = x_base_make(NULL, base);

	_it_should("set filein",
		filein == x_atomint(x_firstobj(x_base_field_filein(p_base)))
	);

	_it_should("set fileout",
		fileout == x_atomint(x_firstobj(x_base_field_fileout(p_base)))
	);

	_it_should("set fileerr",
		fileerr == x_atomint(x_firstobj(x_base_field_fileerr(p_base)))
	);

	_it_should("set obj_meta_extra",
		obj_meta_extra == x_atomint(x_firstobj(x_base_field_obj_meta_extra(p_base)))
	);

	x_obj_free(NULL, p_base);

	return NULL;
}

static char *test_base_slots(void)
{
	struct x_base_t base = test_base_defaults();
	x_obj_t *p_base;
	x_satom_t seven = x_obj_set(x_type_atom_obj, X_OBJ_FLAG_NONE, { .i = 7 }),
		nine = x_obj_set(x_type_atom_obj, X_OBJ_FLAG_NONE, { .i = 9 });
	x_obj_t args[x_vector_storage(2)] =
		x_vector_set(NULL, 2, { (x_obj_t *)seven }, { (x_obj_t *)nine });
	x_fn_t slots[X_SLOT_EXPR_LEN + 2];
	x_int_t i, empty;

	helper_alloc_reset();

	/* No slots asked for. */
	p_base = x_base_make(NULL, base);

	_it_should("hold the slot vector in the base's first unit",
		x_base_slots(p_base) == x_firstobj(p_base)
		&& NULL != x_base_slots(p_base)
	);

	_it_should("make the slot vector a vector, with its length in its first unit",
		X_SLOT_EXPR_LEN == x_vectorlength(x_base_slots(p_base))
	);

	_it_should("hold the tree in the base's second unit",
		x_base(p_base) == x_restobj(p_base)
		&& NULL != x_base(p_base)
	);

	for (empty = 0, i = 0; i < X_SLOT_EXPR_LEN; i++) {
		if (NULL == x_base_slot(p_base, i)) {
			empty++;
		}
	}

	_it_should("make every slot x-expr owns, each empty",
		X_SLOT_EXPR_LEN == empty
	);

	_it_should("say an empty slot is not set",
		! x_base_slot_isset(p_base, X_SLOT_TYPE_NAME)
	);

	x_obj_free(NULL, x_vectorlengthobj(x_base_slots(p_base)));
	x_obj_free(NULL, x_base_slots(p_base));
	x_obj_free(NULL, p_base);

	p_base = NULL;
	_it_should("say no slot is set on a NULL base",
		! x_base_slot_isset(p_base, X_SLOT_TYPE_NAME)
	);

	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("say no slot is set on a base of one unit with nothing in it",
		! x_base_isset(p_base)
		&& ! x_base_slot_isset(p_base, X_SLOT_TYPE_NAME)
	);
	x_obj_free(NULL, p_base);

	/* Slots given, past the ones x-expr owns. */
	for (i = 0; i < X_SLOT_EXPR_LEN + 2; i++) {
		slots[i] = NULL;
	}

	slots[X_SLOT_UNITS] = _slot_fn_a;
	slots[X_SLOT_EXPR_LEN + 1] = _slot_fn_b;
	base.slots = X_SLOT_EXPR_LEN + 2;
	base.p_slots = slots;

	p_base = x_base_make(NULL, base);

	_it_should("fill a slot from the parameters",
		_slot_fn_a == x_base_slot(p_base, X_SLOT_UNITS)
		&& x_base_slot_isset(p_base, X_SLOT_UNITS)
	);

	_it_should("fill a slot past the ones x-expr owns",
		_slot_fn_b == x_base_slot(p_base, X_SLOT_EXPR_LEN + 1)
	);

	_it_should("leave a slot given as NULL empty",
		NULL == x_base_slot(p_base, X_SLOT_TYPE_NAME)
		&& NULL == x_base_slot(p_base, X_SLOT_EXPR_LEN)
	);

	_it_should("call the function in a slot with an argument vector",
		(x_obj_t *)args == x_base_slot(p_base, X_SLOT_UNITS)(p_base, args)
	);

	_it_should("read the arguments from the vector by position",
		2 == x_vectorlength(args)
		&& 7 == x_atomint(x_vectorobj(args, 0))
		&& 9 == x_atomint(x_vectorobj(args, 1))
	);

	/* A slot is replaced by storing another function in it. */
	x_base_slot(p_base, X_SLOT_UNITS) = _slot_fn_b;

	_it_should("replace a slot's function",
		_slot_fn_b == x_base_slot(p_base, X_SLOT_UNITS)
		&& NULL == x_base_slot(p_base, X_SLOT_UNITS)(p_base, args)
	);

	x_base_slot(p_base, X_SLOT_UNITS) = NULL;

	_it_should("empty a slot",
		! x_base_slot_isset(p_base, X_SLOT_UNITS)
	);

	x_obj_free(NULL, x_vectorlengthobj(x_base_slots(p_base)));
	x_obj_free(NULL, x_base_slots(p_base));
	x_obj_free(NULL, p_base);

	return NULL;
}

static x_obj_t *_hook_object;
static x_char_t *_hook_message;
static x_satom_t _hook_answer = x_obj_set(NULL, X_OBJ_FLAG_NONE, { .i = 3 });

static x_obj_t *_hook_fn(x_obj_t *p_base, x_obj_t *p_args)
{
	_hook_object = x_vectorobj(p_args, 0);

	return (x_obj_t *)_hook_answer;
}

static x_obj_t *_hook_error_fn(x_obj_t *p_base, x_obj_t *p_args)
{
	_hook_message = x_atomstr(x_vectorobj(p_args, 0));
	_hook_object = x_vectorobj(p_args, 1);

	return NULL;
}

static char *test_base_slot_hooks(void)
{
	x_obj_t *p_base, *p_obj, *p_ret;
	x_satom_t type = x_obj_set(NULL, X_OBJ_FLAG_NONE, { .s = (x_char_t *)"OTHER" });
	x_spair_t args = x_obj_set(NULL, X_OBJ_FLAG_NONE, { NULL }, { NULL });
	x_char_t *message = (x_char_t *)"message";

	helper_alloc_reset();

	p_base = test_make_base(NULL);
	p_obj = x_obj_make(p_base, (x_obj_t *)type, X_OBJ_FLAG_NONE,
		X_OBJ_LENGTH_ATOM, NULL);
	/* The primitives take a pair; each hands its hook an argument vector. */
	x_firstobj((x_obj_t *)args) = p_obj;

	_it_should("answer NULL for an object of another type while the hooks are empty",
		NULL == x_obj_prim_type_name(p_base, (x_obj_t *)args)
		&& NULL == x_obj_prim_units(p_base, (x_obj_t *)args)
		&& NULL == x_obj_prim_length(p_base, (x_obj_t *)args)
	);

	x_base_slot(p_base, X_SLOT_TYPE_NAME) = _hook_fn;
	_hook_object = NULL;
	p_ret = x_obj_prim_type_name(p_base, (x_obj_t *)args);
	_it_should("call the type-name hook with the object",
		(x_obj_t *)_hook_answer == p_ret && p_obj == _hook_object
	);

	x_base_slot(p_base, X_SLOT_UNITS) = _hook_fn;
	_hook_object = NULL;
	p_ret = x_obj_prim_units(p_base, (x_obj_t *)args);
	_it_should("call the units hook with the object",
		(x_obj_t *)_hook_answer == p_ret && p_obj == _hook_object
	);

	x_base_slot(p_base, X_SLOT_LENGTH) = _hook_fn;
	_hook_object = NULL;
	p_ret = x_obj_prim_length(p_base, (x_obj_t *)args);
	_it_should("call the length hook with the object",
		(x_obj_t *)_hook_answer == p_ret && p_obj == _hook_object
	);

	x_base_slot(p_base, X_SLOT_ERROR) = _hook_error_fn;
	_hook_object = NULL;
	_hook_message = NULL;
	x_obj_error(p_base, message, p_obj);
	_it_should("call the error hook with the message and the object",
		message == _hook_message && p_obj == _hook_object
	);

	return NULL;
}

static char *test_base_read(void)
{
	x_obj_t *p_args, *p_obj;
	x_char_t *s;

	helper_alloc_reset();

	s = "";
	helper_file_buffer_ptr[TEST_HELPER_FILE_STDIN] = s;
	helper_file_buffer_length[TEST_HELPER_FILE_STDIN] = 0;
	helper_file_reset();

	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE,
		x_mksatom(NULL, X_OBJ_FLAG_NONE, s),
		x_mkspair(NULL, X_OBJ_FLAG_NONE,
			x_mksatom(NULL, X_OBJ_FLAG_NONE, 1), NULL));
	p_obj = x_base_read(NULL, p_args);
	_it_should("return NULL on empty input",
		NULL == p_obj
	);

	s = "@";
	helper_file_buffer_ptr[TEST_HELPER_FILE_STDIN] = s;
	helper_file_buffer_length[TEST_HELPER_FILE_STDIN] = TEST_HELPER_FILE_UNDEFINED;
	helper_file_reset();

	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE,
		x_mksatom(NULL, X_OBJ_FLAG_NONE, s),
		x_mkspair(NULL, X_OBJ_FLAG_NONE,
			x_mksatom(NULL, X_OBJ_FLAG_NONE, 1), NULL));
	p_obj = x_base_read(NULL, p_args);
	_it_should("read a character",
		! x_obj_isnil(NULL, p_obj)
		&& '@' == x_atomchar(p_obj)
	);

	return NULL;
}

static char *test_base_write(void)
{
	x_obj_t *p_args, *p_obj;
	x_char_t s[8];

	helper_alloc_reset();

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDOUT] = s;
	helper_file_buffer_length[TEST_HELPER_FILE_STDOUT] = 8;
	helper_file_reset();

	p_args = x_mkspair(NULL, X_OBJ_FLAG_NONE,
		x_mksatom(NULL, X_OBJ_FLAG_NONE, "@"),
		x_mkspair(NULL, X_OBJ_FLAG_NONE,
			x_mksatom(NULL, X_OBJ_FLAG_NONE, 1), NULL));
	p_obj = x_base_write(NULL, p_args);
	_it_should("write a character",
		! x_obj_isnil(NULL, p_obj)
		&& '@' == s[0]
	);

	return NULL;
}

static char *test_base_write_buf(void)
{
	x_obj_t *p_base, *p_args;
	x_char_t buf[64], out[8];
	x_satom_t buf_pos = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.i = 0});
	x_spair_t buf_obj[1] = {
		x_obj_set(NULL, X_OBJ_FLAG_NONE,
			{.v = buf}, {(x_obj_t *)&buf_pos})
	};

	helper_alloc_reset();

	p_base = test_make_base(NULL);
	x_firstobj(x_base_field_write_buf(p_base)) = (x_obj_t *)buf_obj;

	p_args = x_mkspair(p_base, X_OBJ_FLAG_NONE,
		x_mksatom(p_base, X_OBJ_FLAG_NONE, "hello"),
		x_mkspair(p_base, X_OBJ_FLAG_NONE,
			x_mksatom(p_base, X_OBJ_FLAG_NONE, 5), NULL));
	x_base_write(p_base, p_args);
	buf[x_atomint(buf_pos)] = '\0';
	_it_should("write to buffer",
		5 == x_atomint(buf_pos)
		&& 0 == strncmp(buf, "hello", 5)
	);

	/* Remove write-buffer, verify fallback to fd */
	x_firstobj(x_base_field_write_buf(p_base)) = NULL;

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDOUT] = out;
	helper_file_buffer_length[TEST_HELPER_FILE_STDOUT] = 8;
	helper_file_reset();

	p_args = x_mkspair(p_base, X_OBJ_FLAG_NONE,
		x_mksatom(p_base, X_OBJ_FLAG_NONE, "X"),
		x_mkspair(p_base, X_OBJ_FLAG_NONE,
			x_mksatom(p_base, X_OBJ_FLAG_NONE, 1), NULL));
	x_base_write(p_base, p_args);
	_it_should("write to fd when buf is nil", 'X' == out[0]);

	x_obj_free(NULL, p_base);

	return NULL;
}

static char *run_tests()
{
	_run_test(test_base_isset);
	_run_test(test_base_make);
	_run_test(test_base_slots);
	_run_test(test_base_slot_hooks);
	_run_test(test_base_read);
	_run_test(test_base_write);
	_run_test(test_base_write_buf);

	return NULL;
}
