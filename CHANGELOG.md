# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [0.2.0] - 2026-11-13
### Added
  - `x-base`: the base object, made by `x_base_make()` from a
    `struct x_base_t` of I/O descriptors, hooks and heap configuration,
    with field accessors and `x_base_read()`.
  - `x-stdlib.h`: the standard C library stand-ins, split out of `x-lib`.
  - A precise root chain for the collector (`x_heap_root_push()`,
    `x_heap_root_pop()`, `x_heap_root_chain_mark()`), and per-pass mark and
    free hooks and root lists (`x_heap_mark_hook_add()`,
    `x_heap_free_hook_add()`, `x_heap_mark_root_add()`).
  - `x_heap_chain_clear()`.
  - An object-allocation ceiling in the base, which stops a runaway
    allocation.
  - Extended object metadata: extra metadata units per object
    (`obj_meta_extra`), zeroed when an object is made.
  - `x_obj_push()` and `x_obj_pop()`.
  - `X_OBJ_FLAG_SIMPLE_TYPE`.
  - `x_sys_open()`, `x_sys_close()`, and `x_sys_clock()` under
    `X_SYS_CLOCK`.
  - `X_SYS_FUNC`, a macro that redirects system calls at compile time.
  - Out-of-memory reporting.
  - Doxygen documentation for every header and source, and `make doc`.
  - CI on macOS, Alpine and Debian, on x86, arm and riscv.
  - Example programs under `examples/`.

### Changed
  - The collector no longer scans the C stack. A frame registers what it
    holds on the root chain, and `struct x_base_t` no longer takes a stack
    address.
  - Nil is `NULL`, not the base.
  - `x_obj_isnil` is a macro.
  - `x_error` is not fatal.
  - A failed allocation of a base stops cleanly and no longer returns
    `NULL`.
  - `x_base_read()` reports a read error apart from the end of the stream.
  - The base's sets of fields are fields, not groups: the base layout
    changed.
  - "GC" is "heap" throughout: the header is `x-heap.h` and the functions
    are `x_heap_*`.
  - `X_OBJ_FLAG_HEAP` is `X_OBJ_FLAG_MARK`.
  - `x_prim_fn` is `x_fn_t`.
  - `X_CLOCK` is `X_SYS_CLOCK`.
  - `x-lisp.h` is an optional include.
  - Object hooks are function pointers held in the base; the global hooks
    and the `X_OBJ_OVERRIDE` guards are gone.

### Fixed
  - `x_lib_strncmp()`, `x_lib_strchr()` and `x_lib_strndup()` conform to
    their standard library contracts.
  - `x_sys_read_char()` returns the right type where `char` is unsigned.
  - Registering a heap hook pushes onto the hook list and no longer
    replaces it.
  - The mark and the sweep traverse registered type slots.
  - The examples initialize `struct x_base_t` with its ten fields.

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
