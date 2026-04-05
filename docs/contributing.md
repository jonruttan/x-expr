# Contributing to x-expr

## Development Setup

```bash
git clone --recursive git@github.com:jonruttan/x-expr.git
cd x-expr
make        # Build the library
make test   # Run the test suite
```

## Code Style

  - **Language**: ANSI C (`-ansi` flag). No C99+ features.
  - **Indentation**: Tabs (width 4) for C and Makefiles, spaces (width 2) for Markdown. See `.editorconfig`.
  - **Braces**: K&R style -- opening brace on the same line as the control statement.
  - **Comments**: Doxygen `/** */` for public API, `/* */` for internal notes.

## Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Functions | `x_module_name()` | `x_obj_alloc()`, `x_lib_strlen()` |
| Types | `x_name_t` | `x_obj_t`, `x_int_t` |
| Macros | `X_MODULE_NAME` | `X_OBJ_FLAG_NONE`, `X_VERSION` |
| Static objects | `x_name_obj` | `x_type_atom_obj`, `x_true_obj` |
| Internal functions | `_x_name()` | `_x_debug()` |

## Commit Messages

Follow the Angular-style format described in [CONVENTIONS.md](../CONVENTIONS.md):

```
type(scope): short description

Longer explanation if needed.
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`.

## Testing

  - All tests must pass before submitting a PR: `make test`
  - Add tests for new features. Test files live in `tests/src/` and follow the pattern `N.module.spec.c`.
  - Run a single test file with: `TESTS=tests/src/3.0.x-obj.spec.c make test`
  - Run without Valgrind for faster iteration: `make test-quick`

## Pull Requests

  - One logical change per PR.
  - Reference related issues in the description.
  - Ensure `make test` passes and `make doc` generates without warnings.
  - Add or update Doxygen comments for any new or changed public API.
