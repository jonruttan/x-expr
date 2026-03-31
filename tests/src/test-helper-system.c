#ifndef HELPER_SYSTEM_FUNCTIONS
#define HELPER_SYSTEM_FUNCTIONS

#include <fcntl.h>			/* open() */
#include <unistd.h>			/* read(), write() */

#include "test-helper-mem.h"
#include "test-helper-file.h"

#include "x-sys.h"


#define X_SYS_FUNC(F)		helper_sys_##F

void mock_exit(int status);
void *mock_malloc(size_t size);
void mock_free(void *ptr);
ssize_t mock_read(int fd, void *p_buf, size_t size);
ssize_t mock_write(int fd, const void *p_buf, size_t size);
int mock_open(const char *path, int flags, ...);
int mock_close(int fd);

#ifdef X_CLOCK
clock_t mock_clock(void);
#endif /* X_CLOCK */


struct
{
	void (*exit)(int status);

	void *(*malloc)(size_t size);
	void (*free)(void *ptr);

	ssize_t (*read)(int fd, void *p_buf, size_t size);
	ssize_t (*write)(int fd, const void *p_buf, size_t size);
	int (*open)(const char *path, int flags, ...);
	int (*close)(int fd);

#ifdef X_CLOCK
	clock_t (*clock)(void);
#endif /* X_CLOCK */
} helper_sys_funcs = {
	exit,

	malloc,
	free,

	read,
	write,
	open,
	close
#ifdef X_CLOCK
	,mock_clock
#endif /* X_CLOCK */
};


/*
 * ## System Functions
 */
int helper_sys_exit_status = X_SYS_EXIT_SUCCESS;

void mock_exit(int status)
{
	return;
}

void helper_sys_exit(int status)
{
	helper_sys_exit_status = status;
	helper_sys_funcs.exit(status);
}


void *mock_malloc_value = NULL;

void *mock_malloc(size_t size)
{
	return ++mock_malloc_value;
}

size_t helper_sys_malloc_size = 0;

void *helper_sys_malloc(size_t size)
{
	helper_sys_malloc_size = size;

	return helper_sys_funcs.malloc(size);
}


void mock_free(void *ptr)
{
	return;
}

void helper_sys_free(void *ptr)
{
	helper_sys_funcs.free(ptr);
}


ssize_t mock_read_size = 0;

ssize_t mock_read(int fd, void *p_buf, size_t size)
{
	return mock_read_size ? mock_read_size : size;
}

ssize_t mock_read_success(int fd, void *p_buf, size_t size)
{
	return X_SYS_EXIT_SUCCESS;
}

ssize_t mock_read_failure(int fd, void *p_buf, size_t size)
{
	return X_SYS_EOF;
}

ssize_t helper_sys_read(int fd, void *p_buf, size_t size)
{
	if (file_buffer_ptr[fd][TEST_HELPER_FILE_READ] == NULL) {
		return helper_sys_funcs.read(fd, p_buf, size);
	}

	return helper_file_read(fd, p_buf, size);
}


ssize_t mock_write_size = 0;

ssize_t mock_write(int fd, const void *p_buf, size_t size)
{
	return mock_write_size ? mock_read_size : size;
}

ssize_t helper_sys_write(int fd, const void *p_buf, size_t size)
{
	if (file_buffer_ptr[fd][TEST_HELPER_FILE_WRITE] == NULL) {
		return helper_sys_funcs.write(fd, p_buf, size);
	}

	return helper_file_write(fd, p_buf, size);
}


int mock_open_fd = 0;

int mock_open(const char *path, int flags, ...)
{
	return ++mock_open_fd;
}

const char *helper_sys_open_path = NULL;
int helper_sys_open_flags = 0;

int helper_sys_open(const char *path, int flags)
{
	helper_sys_open_path = path;
	helper_sys_open_flags = flags;

	return helper_sys_funcs.open(path, flags);
}


int mock_close_status = 0;

int mock_close(int fd)
{
	return mock_close_status;
}

int helper_sys_close_fd = 0;

int helper_sys_close(int fd)
{
	helper_sys_close_fd = fd;

	return helper_sys_funcs.close(fd);
}


#ifdef X_CLOCK
int mock_clock_value = 0;

clock_t mock_clock(void)
{
	return mock_clock_value++;
}

clock_t helper_sys_clock(void)
{
	return helper_sys_funcs.clock();
}
#endif /* X_CLOCK */

#endif /* HELPER_SYSTEM_FUNCTIONS */
