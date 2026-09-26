/*
 * test-helper-system.c -- Mock system-call layer for unit tests
 *
 * Redefines X_SYS_FUNC so x-sys routes through helper_sys_* wrappers, and
 * provides mock_* implementations (exit, malloc, free, read, write, open,
 * close; clock under X_SYS_CLOCK, sigaction/sigemptyset/signal under
 * X_SYS_SIGNAL) dispatched through a swappable function table. Lets tests
 * intercept system calls without touching the real OS.
 */
#ifndef HELPER_SYSTEM_FUNCTIONS
#define HELPER_SYSTEM_FUNCTIONS

#include <fcntl.h>			/* open() */
#include <unistd.h>			/* read(), write() */
#ifdef X_SYS_SIGNAL
#include <signal.h>			/* sigaction(), signal() */
#endif /* X_SYS_SIGNAL */

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

#ifdef X_SYS_CLOCK
clock_t mock_clock(void);
#endif /* X_SYS_CLOCK */

#ifdef X_SYS_SIGNAL
int mock_sigaction(int sig, const struct sigaction *p_act, struct sigaction *p_old);
void (*mock_signal(int sig, void (*handler)(int)))(int);
#endif /* X_SYS_SIGNAL */


/* Field names avoid the libc names that a fortified <signal.h> defines as
 * function-like macros (sigemptyset on Darwin): a member access
 * `funcs.sigemptyset(...)` would be macro-expanded. */
struct
{
	void (*exit)(int status);

	void *(*malloc)(size_t size);
	void (*free)(void *ptr);

	ssize_t (*read)(int fd, void *p_buf, size_t size);
	ssize_t (*write)(int fd, const void *p_buf, size_t size);
	int (*open)(const char *path, int flags, ...);
	int (*close)(int fd);

#ifdef X_SYS_CLOCK
	clock_t (*clock)(void);
#endif /* X_SYS_CLOCK */

#ifdef X_SYS_SIGNAL
	int (*sigaction)(int sig, const struct sigaction *p_act, struct sigaction *p_old);
	int (*sigempty)(sigset_t *p_set);
	void (*(*signal)(int sig, void (*handler)(int)))(int);
#endif /* X_SYS_SIGNAL */
} helper_sys_funcs = {
	exit,

	malloc,
	free,

	read,
	write,
	open,
	close
#ifdef X_SYS_CLOCK
	,mock_clock
#endif /* X_SYS_CLOCK */
#ifdef X_SYS_SIGNAL
	,sigaction
	,sigemptyset
	,signal
#endif /* X_SYS_SIGNAL */
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


#ifdef X_SYS_CLOCK
int mock_clock_value = 0;

clock_t mock_clock(void)
{
	return mock_clock_value++;
}

clock_t helper_sys_clock(void)
{
	return helper_sys_funcs.clock();
}
#endif /* X_SYS_CLOCK */


#ifdef X_SYS_SIGNAL
int mock_sigaction_status = 0;
int mock_sigaction_sig = 0;
struct sigaction mock_sigaction_act;

int mock_sigaction(int sig, const struct sigaction *p_act, struct sigaction *p_old)
{
	mock_sigaction_sig = sig;
	mock_sigaction_act = *p_act;

	return mock_sigaction_status;
}

int helper_sys_sigaction(int sig, const struct sigaction *p_act, struct sigaction *p_old)
{
	return helper_sys_funcs.sigaction(sig, p_act, p_old);
}

int helper_sys_sigemptyset(sigset_t *p_set)
{
	return helper_sys_funcs.sigempty(p_set);
}


int mock_signal_sig = 0;
void (*mock_signal_handler)(int) = NULL;

void (*mock_signal(int sig, void (*handler)(int)))(int)
{
	mock_signal_sig = sig;
	mock_signal_handler = handler;

	return NULL;
}

void (*helper_sys_signal(int sig, void (*handler)(int)))(int)
{
	return helper_sys_funcs.signal(sig, handler);
}
#endif /* X_SYS_SIGNAL */

#endif /* HELPER_SYSTEM_FUNCTIONS */
