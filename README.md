# x-expr -- Computational Expressions in C

A minimal C library providing a foundation object system with atoms, pairs, garbage collection, and a light type system. Designed for building interpreters and expression evaluators.

## Features

  - Minimal, self-contained object system (atoms and pairs)
  - Light type system with runtime type checking
  - Optional garbage collection (`-DX_GC`)
  - No stdlib dependency (provides its own system abstractions)
  - ANSI C compliant, portable across architectures
  - Suitable as a foundation for Lisp-like interpreters

## Project Structure

```
include/     C headers (x.h, x-obj.h, x-lib.h, x-sys.h, x-gc.h, x-lisp.h)
src/         C source files
tests/       Unit tests using test-runner submodule
```

## Building

```bash
make
```

## Running Tests

```bash
make test
```

## Usage as a Submodule

Add x-expr to your project:

```bash
git submodule add git@github.com:jonruttan/x-expr.git ext/x-expr
```

In your Makefile, add the include path and sources:

```makefile
X_EXPR_DIR=ext/x-expr
CFLAGS+=-I$(X_EXPR_DIR)/include
X_EXPR_SOURCES=$(wildcard $(X_EXPR_DIR)/src/*.c)
```

## License

MIT No Attribution (MIT-0) -- see [LICENSE](LICENSE).
