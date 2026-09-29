# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]
### Added
  - The base's slot vector (`x-slots.h`): the routines the engine calls
    through, each at a fixed position. A slot holds a function pointer with
    the signature `x_fn_t`, and its arguments arrive as an argument vector
    built in stack storage.
  - Slot functions for `x_obj_alloc`, `x_obj_free`, `x_heap_tree_mark`,
    `x_heap_sweep` and `x_heap_root_chain_mark`.

### Changed
  - The base object has two data units: the slot vector, then the tree.
    `x_base()` reads the second, and `x_base_isset()` tests both.
  - The type-name, units, length, error, mark and free hooks are slots.
    Their field cells and accessors are removed, and the mark-hooks,
    free-hooks, mark-roots and root-chain fields are each two steps nearer.
  - `struct x_base_t` takes a table of slot functions in place of six hook
    atoms.
  - The error, mark and free hooks have the signature `x_fn_t`; the types
    `x_heap_mark_fn_t` and `x_heap_free_fn_t` are removed.

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
