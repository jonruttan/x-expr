#ifndef X_H
#define X_H

/**
 * @file x.h
 * @brief Core definitions for computational expressions.
 *
 * Provides fundamental types, architecture detection, debug utilities,
 * and error handling for the x-expr library.
 *
 * @author Jon Ruttan (jonruttan@gmail.com)
 * @copyright 2021 Jon Ruttan
 * @license MIT No Attribution (MIT-0)
 */

/*
 *     ., .,
 *     {O,O}
 *     (   )
 *      " "
 */

#include <stdarg.h>		/* va_list */

/*#define X_USE_STDLIB*/
/*#define X_USE_STDLIB_NONSTD*/

/**
 * @name Constants
 * @{
 */

/**
 * The current version of the x-expr library.
 *
 * Version numbering conforms to the [Semantic Versioning](http://semver.org/)
 * specification.
 */
#define X_VERSION "0.1.0"

/**
 * Enable heap management and garbage collection support.
 *
 * When defined, objects are allocated on a tracked heap chain and
 * additional metadata (heap pointer, shared/heap flags) is stored
 * per object. Required by x-heap.h functions.
 */
/*#define X_HEAP*/

/**
 * The C compiler's target machine identifier.
 *
 * For example, `i686-pc-linux-gnu`. To override the default value of
 * `"undefined"`, pass a value to the C preprocessor via the command line.
 * When supported by the compiler, this is typically the output of
 * the `-dumpmachine` switch.
 */
#ifndef X_MACHINE
#define X_MACHINE "undefined"
#endif /* X_MACHINE */

/**
 * Compiler attribute to disable optimization for a function.
 *
 * Applied to functions that depend on stack layout (e.g. call-stack
 * scanning in the garbage collector). Expands to the appropriate
 * compiler-specific attribute, or nothing if unsupported.
 */
#if defined(__clang__)
#define X_NO_OPTIMIZE __attribute__((optnone))
#elif defined(__GNUC__)
#define X_NO_OPTIMIZE __attribute__((optimize("O0")))
#else
#define X_NO_OPTIMIZE
#endif

#ifdef DEBUG
/** Size in bytes of the debug output buffer. */
#define X_DEBUG_BUFFER_SIZE	65536
#endif /* DEBUG */

/**
 * The C compiler's target CPU architecture as a string literal.
 *
 * Determined by probing compiler predefined macros for known architecture
 * values. Resolves to one of: `"alpha"`, `"amd64"`, `"arm"`, `"arm64"`,
 * `"blackfin"`, `"convex"`, `"epiphany"`, `"pa_risc"`, `"x86"`, `"ia64"`,
 * `"m68k"`, `"mips"`, `"powerpc"`, `"pyramid9810"`, `"rs6000"`, `"sparc"`,
 * `"superh"`, `"systemz"`, `"tms320"`, `"tms470"`, or `"undefined"`.
 *
 * Adapted from https://sourceforge.net/p/predef/wiki/Architectures/
 */
#if __alpha__ || __alpha || _M_ALPHA
#define X_ARCH "alpha"
#elif __amd64__ || __amd64 || __x86_64__ || __x86_64 || _M_X64 || _M_AMD64
#define X_ARCH "amd64"
#elif __arm__ || __thumb__ || __TARGET_ARCH_ARM || __TARGET_ARCH_THUMB || _ARM || _M_ARM || _M_ARMT || __arm
#define X_ARCH "arm"
#elif __aarch64__
#define X_ARCH "arm64"
#elif __bfin || __BFIN__
#define X_ARCH "blackfin"
#elif __convex__
#define X_ARCH "convex"
#elif __epiphany__
#define X_ARCH "epiphany"
#elif __hppa__ || __HPPA__ || __hppa
#define X_ARCH "pa_risc"
#elif i386 || __i386 || __i386__ || __i386 || __i386 || __IA32__ || _M_I86 || _M_IX86 || __X86__ || _X86_ || __THW_INTEL__ || __I86__ || __INTEL__ || __386
#define X_ARCH "x86"
#elif __ia64__ || _IA64 || __IA64__ || __ia64 || _M_IA64 || _M_IA64 || __itanium__
#define X_ARCH "ia64"
#elif __m68k__ || M68000 || __MC68K__
#define X_ARCH "m68k"
#elif __mips__ || mips || __mips || __MIPS__
#define X_ARCH "mips"
#elif  __powerpc || __powerpc__ || __powerpc64__ || __POWERPC__ || __ppc__ || __ppc64__ || __PPC__ || __PPC64__ || _ARCH_PPC || _ARCH_PPC64 || _M_PPC || _ARCH_PPC || _ARCH_PPC64 || __ppc
#define X_ARCH "powerpc"
#elif pyr
#define X_ARCH "pyramid9810"
#elif __THW_RS6000 || _IBMR2 || _POWER || _ARCH_PWR || _ARCH_PWR2 || _ARCH_PWR3 || _ARCH_PWR4
#define X_ARCH "rs6000"
#elif __sparc__ || __sparc
#define X_ARCH "sparc"
#elif __sh__
#define X_ARCH "superh"
#elif __370__ || __THW_370__ || __s390__ || __s390x__ || __zarch__ || __SYSC_ZARCH__
#define X_ARCH "systemz"
#elif _TMS320C2XX || __TMS320C2000__ || _TMS320C5X || __TMS320C55X__ || TMS320C6X || __TMS320C6X__
#define X_ARCH "tms320"
#elif __TMS470__
#define X_ARCH "tms470"
#else
#define X_ARCH "undefined"
#endif

/** @} */

/**
 * @name Basic Types
 * @{
 */

/** The integer type used throughout x-expr. Sized to match pointer width. */
typedef long x_int_t;

/** Printf length modifier for x_int_t values (e.g. `"l"` for `long`). */
#define X_INT_STR_PRINTF_CONV	"l"

/** Compile-time assertion that x_int_t and pointers are the same size. */
typedef char x_assert_int_ptr_size[sizeof(x_int_t) == sizeof(void *) ? 1 : -1];

/** The character type used throughout x-expr. */
typedef char x_char_t;

/** Printf conversion specifier for x_char_t values. */
#define X_INT_CHAR_PRINTF_CONV	"c"

/**
 * Expand a string literal into a pointer and length pair.
 *
 * Produces two arguments: a `(x_char_t *)` pointer and the string length
 * (excluding the null terminator). Useful with x_sys_write().
 *
 * @param s A string literal.
 */
#define X_STR_LITERAL(s) (x_char_t *)(s), (sizeof(s) - 1)

/** @} */

/**
 * @name Debug Utilities
 * @{
 */

#ifdef DEBUG

/** @internal */
void _x_debug_va(char *file, long unsigned line, int fd, char *fmt, va_list ap);

/**
 * Write a formatted debug message with va_list arguments.
 *
 * Automatically prepends the source file name and line number.
 * Compiles to a no-op when DEBUG is not defined.
 *
 * @param fd   File descriptor to write to.
 * @param fmt  printf-style format string.
 * @param ap   Variable argument list.
 */
#define x_debug_va(fd, fmt, ap) _x_debug_va(__FILE__, __LINE__, fd, fmt, ap)

/** @internal */
void _x_debug(char *file, long unsigned line, int fd, char *fmt, ...);

/**
 * Write a formatted debug message.
 *
 * Automatically prepends the source file name and line number.
 * Compiles to a no-op when DEBUG is not defined.
 *
 * @param fd   File descriptor to write to.
 * @param fmt  printf-style format string.
 * @param ...  Format arguments.
 */
#define x_debug(fd, fmt, ...) _x_debug(__FILE__, __LINE__, fd, fmt, __VA_ARGS__)

#else /* DEBUG */

/** @copydoc x_debug_va */
#define x_debug_va(fd, fmt, ap)		{}
/** @copydoc x_debug */
#define x_debug(fd, fmt, ...)		{}

#endif /* DEBUG */

/** @} */

/**
 * Output an error message and terminate the process.
 *
 * Writes `"*** ERROR: "` followed by the message (and optionally the
 * symbol in quotes) to the given file descriptor, then calls x_sys_exit()
 * with X_SYS_EXIT_FAILURE.
 *
 * @param fd      File descriptor to write the error message to.
 * @param message A C string error message.
 * @param symbol  A C string with additional context, or NULL.
 */
void x_error(int fd, x_char_t *message, x_char_t *symbol);

#endif /* X_H */
