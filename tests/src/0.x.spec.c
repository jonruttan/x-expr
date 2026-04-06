/*
 * # Unit Tests: *x*
 */

#define TEST_RUNNER_OVERHEAD
#include "test-runner.h"

#define X_OPT_MEMZERO

#include "x.h"

#ifndef X_TEST_LIB
#undef X_USE_STDLIB
#endif /* X_TEST_LIB */

#include "test-helper-system.c"

#include "src/x-sys.c"
#include "src/x-lib.c"
#include "src/x.c"


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
static char *test_constants(void)
{
	_it_should("X_VERSION is defined", NULL != X_VERSION);
	_it_should("X_MACHINE is defined", NULL != X_MACHINE);
	_it_should("X_ARCH is defined", NULL != X_ARCH);
	_it_should("X_INT_STR_PRINTF_CONV is defined", NULL != X_INT_STR_PRINTF_CONV);
	_it_should("X_INT_CHAR_PRINTF_CONV is defined", NULL != X_INT_CHAR_PRINTF_CONV);

	return NULL;
}

static char *test_error(void)
{
	char buffer[4096];

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();

	memset(buffer, 0, sizeof(buffer));

	x_error(TEST_HELPER_FILE_STDERR, (x_char_t *)"test error", NULL);
	_it_should("have written error prefix", 0 == strncmp(buffer, "*** ERROR: ", 11));
	_it_should("have written the message", NULL != strstr(buffer, "test error"));

	helper_file_reset();
	memset(buffer, 0, sizeof(buffer));

	x_error(TEST_HELPER_FILE_STDERR, (x_char_t *)"bad value", (x_char_t *)"foo");
	_it_should("have written the symbol", NULL != strstr(buffer, "'foo"));

	return NULL;
}

static char *test_debug(void)
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

static void helper_debug_va(int fd, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	x_debug_va(fd, fmt, ap);
	va_end(ap);
}

static char *test_debug_va(void)
{
	char buffer[4096];

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDERR] = buffer;
	helper_file_reset();
	memset(buffer, 0, sizeof(buffer));

	helper_debug_va(TEST_HELPER_FILE_STDERR, "va-%s", "test");
	_it_should("x_debug_va wrote output", x_lib_strlen((x_char_t *)buffer) > 0);

	return NULL;
}

static char *run_tests() {
	_run_test(test_constants);
	_run_test(test_error);
	_run_test(test_debug);
	_run_test(test_debug_va);

	return NULL;
}
