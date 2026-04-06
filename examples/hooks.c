/*
 * hooks.c -- custom type hooks example
 *
 * Demonstrates installing a custom type_name hook to support
 * user-defined types beyond the built-in atom and pair.
 *
 * Build:  make hooks
 * Run:    ./hooks
 */

#include <stdio.h>
#include <unistd.h>

#include "x-base.h"
#include "x-lisp.h"

/* A custom type descriptor (just a static atom) */
static x_satom_t vec2_type_obj = x_obj_set(NULL, X_OBJ_FLAG_NONE, {0});

/* Hook: return type name for custom types */
static x_obj_t *my_type_name(x_obj_t *p_base, x_obj_t *p_args)
{
	x_obj_t *p_obj = x_firstobj(p_args);

	if (x_obj_type(p_obj) == (x_obj_t *)&vec2_type_obj) {
		return x_mksatom(p_base, X_OBJ_FLAG_NONE, "VEC2");
	}

	return NULL;
}

int main(int argc, char **argv)
{
	x_obj_t *p_base, *p_hook, *p_vec;
	struct x_base_t base = {
		STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO,
		NULL, NULL, NULL, NULL,
		0, NULL, NULL, &argc
	};

	/* Create the hook atom and wire it in */
	p_hook = x_mksatom(NULL, X_OBJ_FLAG_NONE, (x_obj_t *)my_type_name);
	base.p_hook_type_name = p_hook;
	p_base = x_base_make(NULL, base);

	/* Create a pair with our custom type */
	p_vec = x_obj_make(p_base, (x_obj_t *)&vec2_type_obj,
		X_OBJ_FLAG_NONE, X_OBJ_LENGTH_PAIR,
		(x_obj_t *)10, (x_obj_t *)20);

	printf("type: %s\n", x_obj_type_name(p_base, p_vec));
	printf("x=%ld, y=%ld\n", x_firstint(p_vec), x_restint(p_vec));

	/* Built-in types still work */
	printf("atom type: %s\n", x_obj_type_name(p_base,
		x_mksatom(p_base, X_OBJ_FLAG_NONE, (x_obj_t *)42)));

	return 0;
}
