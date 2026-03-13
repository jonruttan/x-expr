/*
 * # Unit Tests: *x-obj error functions*
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
static char *test_x_error(void)
{
	char buffer[4096];

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();

	x_sys_exit_status = X_SYS_EXIT_SUCCESS;
	memset(buffer, 0, sizeof(buffer));

	x_error(TEST_HELPER_FILE_STDERR, (x_char_t *)"test error", NULL);
	_it_should("have set exit status to failure", X_SYS_EXIT_FAILURE == x_sys_exit_status);
	_it_should("have written error prefix", 0 == strncmp(buffer, "*** ERROR: ", 11));
	_it_should("have written the message", NULL != strstr(buffer, "test error"));

	return NULL;
}

static char *test_x_error_with_symbol(void)
{
	char buffer[4096];

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();

	x_sys_exit_status = X_SYS_EXIT_SUCCESS;
	memset(buffer, 0, sizeof(buffer));

	x_error(TEST_HELPER_FILE_STDERR, (x_char_t *)"bad value", (x_char_t *)"foo");
	_it_should("have set exit status to failure", X_SYS_EXIT_FAILURE == x_sys_exit_status);
	_it_should("have written the symbol", NULL != strstr(buffer, "'foo"));

	return NULL;
}

static char *test_obj_error_nil(void)
{
	char buffer[4096];

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();

	x_sys_exit_status = X_SYS_EXIT_SUCCESS;
	memset(buffer, 0, sizeof(buffer));

	x_obj_error(NULL, (x_char_t *)"nil error", NULL);
	_it_should("have set exit status to failure", X_SYS_EXIT_FAILURE == x_sys_exit_status);
	_it_should("have written error prefix", 0 == strncmp(buffer, "*** ERROR: ", 11));

	return NULL;
}

static char *test_obj_error_atom(void)
{
	x_obj_t *p_obj;
	char buffer[4096];

	helper_alloc_reset();

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();

	x_sys_exit_status = X_SYS_EXIT_SUCCESS;
	memset(buffer, 0, sizeof(buffer));

	p_obj = x_mksatom(NULL, "mysym");
	x_obj_error(NULL, (x_char_t *)"symbol error", p_obj);
	_it_should("have set exit status to failure", X_SYS_EXIT_FAILURE == x_sys_exit_status);
	_it_should("have included the symbol name", NULL != strstr(buffer, "'mysym"));

	x_obj_free(p_obj);

	return NULL;
}

static char *run_tests()
{
	_run_test(test_x_error);
	_run_test(test_x_error_with_symbol);
	_run_test(test_obj_error_nil);
	_run_test(test_obj_error_atom);

	return NULL;
}
