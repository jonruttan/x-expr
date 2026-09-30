# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]
### Added
  - The vector layout (`x-vector.h`): an object whose first data unit holds
    its length and whose other data units are its elements, with accessors,
    static length atoms for the lengths 0 to 8, and the constructor
    `x_vector_make`.
  - The base's slot vector (`x-slots.h`): a vector of function pointers, one
    per slot, the routines the engine calls through. A routine in a slot
    has the signature `x_fn_t`, and its arguments arrive as an argument run:
    a run of datum words, one per argument, written in stack storage, with
    no header and no length. An integer or a string travels as a word.
  - `x_base_call`, a call through a slot; `x_base_call_or`, which calls the
    routine named when there is no base or the base has no slot vector; and
    `x_argrun`, an argument run written as an expression.

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
  - `x_obj_alloc`, `x_obj_free`, `x_heap_tree_mark`, `x_heap_sweep` and
    `x_heap_root_chain_mark` have the signature `x_fn_t` and take an
    argument run. `x_base_make` puts each in its slot, a table entry of the
    caller's replaces it, and x-expr's own calls go through the slots.
    `x_obj_free` returns NULL.

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
