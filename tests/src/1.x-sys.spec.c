/*
 * # Unit Tests: *x-sys*
 */

#define X_CLOCK

#define TEST_RUNNER_OVERHEAD
#include "test-runner.h"
#include "test-helper-mem.h"
#include "test-helper-file.h"
#include "test-helper-system.c"

#include "src/x-sys.c"


/*
 * ## Test Overhead
 */
static void _setup(void)
{
	srand(time(NULL));
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
static char *test_sys_exit(void)
{
	int status = rand();

	_it_should("define X_SYS_EXIT_SUCCESS", 0 == X_SYS_EXIT_SUCCESS);
	_it_should("define X_SYS_EXIT_FAILUTRE", 1 == X_SYS_EXIT_FAILURE);

	x_sys_exit(status);
	_it_should("have set the exit status", status == helper_sys_exit_status);

	return NULL;
}

static char *test_sys_malloc(void)
{
	size_t bytes = 16;
	x_char_t *p_dst;

	p_dst = x_sys_malloc(bytes);
	_it_should("have called alloc", 1 == helper_alloc_count());
	_it_should("have requested the number of bytes", bytes == helper_sys_malloc_size);
	_it_should("create an uninitialized memory vector", NULL != p_dst);

	x_sys_free(p_dst);

	return NULL;
}

static char *test_sys_free(void)
{
	x_char_t *p_dst;

	helper_alloc_reset();

	p_dst = x_sys_malloc(16);
	x_sys_free(p_dst);
	_it_should("have called free", 1 == helper_free_count());

	return NULL;
}

static char *test_sys_read(void)
{
	char *test_string = "test_sys_read", buffer[4096];
	ssize_t size;

	helper_sys_funcs.read = mock_read;

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDIN] = test_string;

	helper_file_reset();

	memset(buffer, 0, 4096);

	size = x_sys_read(TEST_HELPER_FILE_STDIN, buffer, strlen(test_string));
	_it_should("have returned the number of bytes read", (ssize_t)strlen(test_string) == size);
	_it_should("have read the test data", 0 == strcmp(buffer, test_string));

	return NULL;
}

static char *test_sys_write(void)
{
	char *test_string = "test_sys_read", buffer[4096];
	ssize_t size;

	helper_sys_funcs.write = mock_write;

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDOUT] = buffer;

	helper_file_reset();

	memset(buffer, 0, 4096);

	size = x_sys_write(TEST_HELPER_FILE_STDOUT, test_string, strlen(test_string));
	_it_should("have returned the number of bytes written", (ssize_t)strlen(test_string) == size);
	_it_should("have written the test data", 0 == strcmp(buffer, test_string));

	return NULL;
}

static char *test_sys_open(void)
{
	char *path = "test_sys_open";
	int flags = rand(), fd;

	helper_sys_funcs.open = mock_open;

	fd = x_sys_open(path, flags);
	_it_should("have returned a file descriptor", mock_open_fd == fd);
	_it_should("have passed the path", helper_sys_open_path == path);
	_it_should("have passed the flags", helper_sys_open_flags == flags);

	return NULL;
}

static char *test_sys_close(void)
{
	int fd = 1, status;

	helper_sys_funcs.close = mock_close;

	status = x_sys_close(fd);
	_it_should("have returned the status", mock_close_status == status);

	return NULL;
}

static char *test_sys_read_char(void)
{
	char *test_string = "AB";

	helper_sys_funcs.read = mock_read_failure;

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDIN] = test_string;
	helper_file_reset();

	_it_should("define X_SYS_EOF", EOF == X_SYS_EOF);
	_it_should("read the first character", 'A' == x_sys_read_char(TEST_HELPER_FILE_STDIN));
	_it_should("read the second character", 'B' == x_sys_read_char(TEST_HELPER_FILE_STDIN));

	helper_file_buffer_ptr[TEST_HELPER_FILE_STDIN] = NULL;
	helper_file_reset();

	_it_should("return EOF on short read", X_SYS_EOF == x_sys_read_char(TEST_HELPER_FILE_STDIN));

	return NULL;
}

static char *test_sys_clock(void)
{
	x_int_t t[2];

	helper_sys_funcs.clock = mock_clock;

	t[0] = x_sys_clock();
	_it_should("return a non-negative value", 0 <= t[0]);

	t[1] = x_sys_clock();
	_it_should("increment the value", t[0] < t[1]);

	return NULL;
}

static char *run_tests() {
	_run_test(test_sys_exit);

	_run_test(test_sys_malloc);
	_run_test(test_sys_free);

	_run_test(test_sys_read);
	_run_test(test_sys_write);

	_run_test(test_sys_open);
	_run_test(test_sys_close);

	_run_test(test_sys_read_char);
	_run_test(test_sys_clock);

	return NULL;
}
