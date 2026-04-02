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
		NULL, NULL, NULL, NULL,
		0,
		NULL, NULL,
		NULL
	};
	return base;
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
	int stack_anchor;
	struct x_base_t base = test_base_defaults();
	x_obj_t *p_base, *p_hook;
	x_int_t filein = rand(), fileout = rand(), fileerr = rand(),
		obj_meta_extra = rand();

	helper_alloc_reset();

	base.p_stack_base = &stack_anchor;

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

	_it_should("capture stack_base",
		NULL != x_atomptr(x_firstobj(x_base_field_stack_base(p_base)))
	);

	_it_should("set hooks to nil",
		NULL == x_firstobj(x_base_field_hook_type_name(p_base))
		&& NULL == x_firstobj(x_base_field_hook_units(p_base))
		&& NULL == x_firstobj(x_base_field_hook_length(p_base))
		&& NULL == x_firstobj(x_base_field_hook_error(p_base))
	);

	/* verify hooks with non-nil values */
	p_hook = x_mksatom(p_base, X_OBJ_FLAG_NONE, 42);
	base.p_hook_type_name = p_hook;
	base.p_hook_units = p_hook;
	base.p_hook_length = p_hook;
	base.p_hook_error = p_hook;
	x_obj_free(NULL, p_base);

	p_base = x_base_make(NULL, base);

	_it_should("set hook_type_name",
		p_hook == x_firstobj(x_base_field_hook_type_name(p_base))
	);

	_it_should("set hook_units",
		p_hook == x_firstobj(x_base_field_hook_units(p_base))
	);

	_it_should("set hook_length",
		p_hook == x_firstobj(x_base_field_hook_length(p_base))
	);

	_it_should("set hook_error",
		p_hook == x_firstobj(x_base_field_hook_error(p_base))
	);

	x_obj_free(NULL, p_base);

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
	_run_test(test_base_read);
	_run_test(test_base_write);
	_run_test(test_base_write_buf);

	return NULL;
}
