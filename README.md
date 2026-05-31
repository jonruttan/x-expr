# x-expr -- Computational Expressions in C

A minimal C library providing a foundation object system with atoms, pairs, garbage collection, and a light type system. Designed for building interpreters and expression evaluators.

## Features

  - Minimal, self-contained object system (atoms and pairs)
  - Light type system with runtime type checking
  - Optional garbage collection (`-DX_HEAP`)
  - No stdlib dependency (provides its own system abstractions)
  - ANSI C compliant, portable across architectures
  - Suitable as a foundation for Lisp-like interpreters

## Quick Start

```c
#include "x-base.h"
#include "x-lisp.h"

/* Create a base environment */
struct x_base_t conf = { STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO };
x_obj_t *base = x_base_make(NULL, conf);

/* Make atoms and pairs */
x_obj_t *a = x_mksatom(base, X_OBJ_FLAG_NONE, (x_obj_t *)42);
x_obj_t *p = x_cons(base, X_OBJ_FLAG_NONE, a, NULL);

/* Access data */
printf("%ld\n", x_atomint(a));        /* 42 */
printf("%ld\n", x_atomint(x_car(p))); /* 42 */
```

See [examples/hello.c](examples/hello.c) for a complete working program.

## Building

```bash
make          # Build static library (libx-expr.a)
make test     # Run unit tests
make doc      # Generate HTML + man page documentation
make install  # Install to /usr/local (override with PREFIX=...)
```

Run `make help` for the full list of targets.

## Compile-Time Flags

**Supported** -- tested in CI:

| Flag | Default | Description |
|------|---------|-------------|
| `X_HEAP` | on | Enable heap management and mark-sweep garbage collection |
| `X_USE_STDLIB` | off | Delegate x-lib functions to standard library equivalents |
| `DEBUG` | off | Enable debug output macros (x_debug, x_obj_dump) |

**Experimental** -- functional but not CI-tested:

| Flag | Default | Description |
|------|---------|-------------|
| `X_PROFILE` | off | Enable allocation profiling counters |
| `X_CLOCK` | off | Enable CPU clock measurement (x_sys_clock) |
| `X_USE_STDLIB_NONSTD` | off | Use non-standard library extensions (requires `X_USE_STDLIB`) |

**Internal** -- for x-expr development:

| Flag | Default | Description |
|------|---------|-------------|
| `X_OPT_MEMZERO` | off | Zero allocated memory in x_lib_memdup |
| `X_SYS_FUNC` | identity | Macro to redirect system calls at compile time |

Pass flags to the compiler: `make CFLAGS+="-DX_HEAP -DDEBUG"`

## API Overview

| Header | Purpose |
|--------|---------|
| [x.h](include/x.h) | Core types (`x_int_t`, `x_char_t`), architecture detection, error handling |
| [x-sys.h](include/x-sys.h) | System-level wrappers (malloc, free, read, write, exit) |
| [x-lib.h](include/x-lib.h) | Portable string, memory, and math utilities |
| [x-obj.h](include/x-obj.h) | Object system: atoms, pairs, flags, type checking, allocation |
| [x-base.h](include/x-base.h) | Base environment: I/O, hooks, heap config, field accessors |
| [x-heap.h](include/x-heap.h) | Mark-sweep garbage collection (requires `X_HEAP`) |
| [x-lisp.h](include/x-lisp.h) | Lisp-style aliases: cons, car, cdr, setcar, setcdr |

## Base Tree Layout

The base environment is a nested pair tree whose field positions are accessed via macros in `x-base.h`. The tree layout documented there is stable API -- field positions will not change within a major version.

## Thread Safety

x-expr assumes single-threaded execution. The heap chain, garbage collector, and field stack operations are not synchronized. If you need to use x-expr from multiple threads, you must provide your own external synchronization around all object allocation, GC, and base environment mutation.

## Usage as a Submodule

Add x-expr to your project:

```bash
git submodule add git@github.com:jonruttan/x-expr.git ext/x-expr
```

In your Makefile, add the include path and sources:

```makefile
X_EXPR_DIR=ext/x-expr
CFLAGS+=-I$(X_EXPR_DIR)/include -DX_HEAP
X_EXPR_SOURCES=$(wildcard $(X_EXPR_DIR)/src/*.c)
```

## Project Structure

```
include/     C headers
src/         C source files
tests/       Unit tests using test-runner submodule
examples/    Example programs
docs/        Documentation and contributor guide
```

## Documentation

Generate the API reference with `make doc`. Output is written to `docs/ref/html/` (HTML) and `docs/ref/man/` (man pages).

## Contributing

See [docs/contributing.md](docs/contributing.md) for development setup, code style, and pull request guidelines.

## License

MIT No Attribution (MIT-0) -- see [LICENSE](LICENSE).
