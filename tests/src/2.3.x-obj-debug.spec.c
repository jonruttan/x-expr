/*
 * # Unit Tests: *x-obj debug functions*
 */

#define TEST_RUNNER_OVERHEAD
#include "test-runner.h"

#include "src/x-sys.c"
#include "src/x-lib.c"
#include "src/x.c"
#include "src/x-obj.c"

#include "helper-system-functions.c"


/*
 * ## Test Overhead
 */
static void _setup(void)
{
	helper_set_alloc(MEM_GUARANTEED);
}

static void _teardown(void)
{
}


/*
 * ## Test Runners
 */
static char *test_x_debug(void)
{
	char buffer[4096];

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();
	memset(buffer, 0, sizeof(buffer));

	_x_debug(__FILE__, __LINE__, TEST_HELPER_FILE_STDERR, "hello %s", "world");
	_it_should("have written debug output", x_lib_strlen((x_char_t *)buffer) > 0);
	_it_should("contain DEBUG marker", NULL != strstr(buffer, "DEBUG"));
	_it_should("contain the message", NULL != strstr(buffer, "hello world"));
	_it_should("X_DEBUG_BUFFER_SIZE is defined", X_DEBUG_BUFFER_SIZE > 0);

	return NULL;
}

static char *test_obj_debug(void)
{
	char buffer[4096];

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();
	memset(buffer, 0, sizeof(buffer));

	_x_obj_debug(__FILE__, __LINE__, NULL, "obj %d", 42);
	_it_should("have written debug output", x_lib_strlen((x_char_t *)buffer) > 0);
	_it_should("contain the message", NULL != strstr(buffer, "obj 42"));

	return NULL;
}

static char *test_obj_dump_nil(void)
{
	char buffer[4096];

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();
	memset(buffer, 0, sizeof(buffer));

	_x_obj_dump(__FILE__, __LINE__, NULL, NULL, "dump-nil");
	_it_should("have written debug output", x_lib_strlen((x_char_t *)buffer) > 0);
	_it_should("contain the label", NULL != strstr(buffer, "dump-nil"));
	_it_should("contain NIL type", NULL != strstr(buffer, X_TYPE_NIL_NAME));
	_it_should("X_OBJ_DUMP_BUFFER_SIZE is defined", X_OBJ_DUMP_BUFFER_SIZE > 0);

	return NULL;
}

static char *test_obj_dump_atom(void)
{
	x_obj_t *p_obj;
	char buffer[4096];

	helper_alloc_reset();

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();
	memset(buffer, 0, sizeof(buffer));

	p_obj = x_mksatom(NULL, 42);
	_x_obj_dump(__FILE__, __LINE__, NULL, p_obj, "dump-atom");
	_it_should("have written debug output", x_lib_strlen((x_char_t *)buffer) > 0);
	_it_should("contain the label", NULL != strstr(buffer, "dump-atom"));
	_it_should("contain ATOM type", NULL != strstr(buffer, X_TYPE_ATOM_NAME));

	x_obj_free(p_obj);

	return NULL;
}

static char *test_obj_dump_pair(void)
{
	x_obj_t *p_a, *p_b, *p_obj;
	char buffer[4096];

	helper_alloc_reset();

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();
	memset(buffer, 0, sizeof(buffer));

	p_a = x_mksatom(NULL, 1);
	p_b = x_mksatom(NULL, 2);
	p_obj = x_mkspair(NULL, p_a, p_b);
	_x_obj_dump(__FILE__, __LINE__, NULL, p_obj, "dump-pair");
	_it_should("have written debug output", x_lib_strlen((x_char_t *)buffer) > 0);
	_it_should("contain the label", NULL != strstr(buffer, "dump-pair"));
	_it_should("contain PAIR type", NULL != strstr(buffer, X_TYPE_PAIR_NAME));

	x_obj_free(p_obj);
	x_obj_free(p_b);
	x_obj_free(p_a);

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
	_it_should("x_debug_va wrote output", x_lib_strlen((x_char_t *)buffer) > 0);

	helper_file_reset();
	memset(buffer, 0, sizeof(buffer));

	helper_x_obj_debug_va(NULL, "va-obj-%s", "test");
	_it_should("x_obj_debug_va wrote output", x_lib_strlen((x_char_t *)buffer) > 0);

	return NULL;
}

static char *test_constants(void)
{
	_it_should("X_VERSION is defined", NULL != X_VERSION);
	_it_should("X_MACHINE is defined", NULL != X_MACHINE);
	_it_should("X_ARCH is defined", NULL != X_ARCH);
	_it_should("X_OBJ_UNITS_TYPE is defined", X_OBJ_UNITS_TYPE > 0);
	_it_should("X_OBJ_UNITS_FLAGS is defined", X_OBJ_UNITS_FLAGS > 0);
	_it_should("X_OBJ_UNITS_META is defined", X_OBJ_UNITS_META > 0);
	_it_should("X_OBJ_UNITS_BASE is defined", X_OBJ_UNITS_BASE > 0);
	_it_should("X_OBJ_UNITS_HEAP is defined", X_OBJ_UNITS_HEAP >= 0);
	_it_should("X_INT_STR_PRINTF_CONV is defined", NULL != X_INT_STR_PRINTF_CONV);
	_it_should("X_INT_CHAR_PRINTF_CONV is defined", NULL != X_INT_CHAR_PRINTF_CONV);

	return NULL;
}

static char *run_tests()
{
	_run_test(test_x_debug);
	_run_test(test_obj_debug);
	_run_test(test_obj_dump_nil);
	_run_test(test_obj_dump_atom);
	_run_test(test_obj_dump_pair);
	_run_test(test_debug_va_macros);
	_run_test(test_constants);

	return NULL;
}
