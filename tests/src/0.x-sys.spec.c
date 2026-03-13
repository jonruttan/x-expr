/*
 * # Unit Tests: *x-sys*
 */

#define X_CLOCK
#define TEST_RUNNER_OVERHEAD
#include "test-runner.h"

#include "src/x-sys.c"

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
static char *test_sys_malloc(void)
{
	x_char_t *p_dst;

	p_dst = x_sys_malloc(16);
	_it_should("have called alloc", helper_alloc_count() == 1);
	_it_should("create an uninitialized memory vector", p_dst != NULL);

	x_sys_free(p_dst);

	return NULL;
}

static char *test_sys_free(void)
{
	x_char_t *p_dst;

	helper_alloc_reset();

	p_dst = x_sys_malloc(16);
	x_sys_free(p_dst);
	_it_should("have called free", helper_free_count() == 1);

	return NULL;
}

static char *test_sys_read(void)
{
	char *test_string = "test_sys_read", buffer[4096];
	ssize_t size;

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDIN] = test_string;

	helper_file_reset();

	memset(buffer, 0, 4096);

	size = x_sys_read(TEST_HELPER_FILE_STDIN, buffer, strlen(test_string));
	_it_should("have returned the number of bytes read", size == (ssize_t)strlen(test_string));
	_it_should("have read the test data", strcmp(buffer, test_string) == 0);

	return NULL;
}

static char *test_sys_write(void)
{
	char *test_string = "test_sys_read", buffer[4096];
	ssize_t size;

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDOUT] = buffer;

	helper_file_reset();

	memset(buffer, 0, 4096);

	size = x_sys_write(TEST_HELPER_FILE_STDOUT, test_string, strlen(test_string));
	_it_should("have returned the number of bytes written", size == (ssize_t)strlen(test_string));
	_it_should("have written the test data", strcmp(buffer, test_string) == 0);

	return NULL;
}

static char *test_sys_read_char(void)
{
	char *test_string = "AB";

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDIN] = test_string;
	helper_file_reset();

	_it_should("read the first character", 'A' == x_sys_read_char(TEST_HELPER_FILE_STDIN));
	_it_should("read the second character", 'B' == x_sys_read_char(TEST_HELPER_FILE_STDIN));
	_it_should("X_SYS_EOF is defined", X_SYS_EOF != 0);

	return NULL;
}

static char *test_sys_clock(void)
{
	x_int_t t;

	t = x_sys_clock();
	_it_should("return a non-negative value", t >= 0);

	return NULL;
}

static char *run_tests() {
	_run_test(test_sys_malloc);
	_run_test(test_sys_free);

	_run_test(test_sys_read);
	_run_test(test_sys_write);

	_run_test(test_sys_read_char);
	_run_test(test_sys_clock);

	return NULL;
}
