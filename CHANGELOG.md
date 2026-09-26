# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]
### Added
  - `x_sys_signal_install` / `x_sys_signal_restore` under `X_SYS_SIGNAL`,
    with `X_SYS_SIGINT`.
  - `make check-libc` (tools/check/libc.sh): refuses a libc header, a libc
    call, a feature-test macro or a silenced diagnostic anywhere outside
    x-sys and x-stdlib. `<ctype.h>`, `<setjmp.h>` and the freestanding
    headers are allowed everywhere; code inside `#ifdef DEBUG` is exempt.
    Runs first in `make test` and `make test-quick`. The signal wrappers
    exist because a consumer had called sigaction directly.

### Changed
  - `x-sys.c` defines `_GNU_SOURCE` before its first header, and the test
    build passes it, so the POSIX names are declared under `-ansi`.

## [0.1.0] - 2026-03-06
### Added
  - Light type system baked into object layout (atom/pair type checking).
  - Type object externs and type checking macros.
  - Stack-allocated object typedefs (`x_satom_t`, `x_spair_t`).
  - Convenience macros (`x_mksatom`, `x_mkspair`, etc.).
  - Published to GitHub as standalone library.

### Changed
  - Object layout always includes type field.
  - `x_obj_set` uses initializer syntax instead of compound literals.

## [0.0.1] - 2021-09-26
### Added
  - Initial object system (atoms, pairs).
  - Garbage collection support (`-DX_HEAP`).
  - System abstraction layer (`x-sys`).
  - Standard library abstractions (`x-lib`).
  - Unit tests and test-runner submodule.
  - Makefile with portable compiler detection.
