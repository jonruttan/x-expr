/*
 * hello.c -- x-expr introductory example
 *
 * Demonstrates creating a base environment, making atoms and pairs,
 * accessing data with car/cdr, and querying type names.
 *
 * Build:  make
 * Run:    ./hello
 */

#include <stdio.h>
#include <unistd.h>

#include "x-base.h"
#include "x-lisp.h"

int main(int argc, char **argv)
{
	x_obj_t *p_base, *p_atom, *p_pair, *p_list;
	struct x_base_t base = {
		STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO,	/* file descriptors */
		NULL, NULL, NULL, NULL,				/* hooks (none) */
		0, NULL, NULL, &argc				/* heap config + stack base */
	};

	/* Create the base environment */
	p_base = x_base_make(NULL, base);

	/* Create an integer atom */
	p_atom = x_mksatom(p_base, X_OBJ_FLAG_NONE, (x_obj_t *)42);
	printf("atom type:  %s\n", x_obj_type_name(p_base, p_atom));
	printf("atom value: %ld\n", x_atomint(p_atom));

	/* Create a pair of two atoms */
	p_pair = x_cons(p_base, X_OBJ_FLAG_NONE,
		x_mksatom(p_base, X_OBJ_FLAG_NONE, (x_obj_t *)1),
		x_mksatom(p_base, X_OBJ_FLAG_NONE, (x_obj_t *)2));
	printf("pair type:  %s\n", x_obj_type_name(p_base, p_pair));
	printf("car: %ld, cdr: %ld\n",
		x_atomint(x_car(p_pair)),
		x_atomint(x_cdr(p_pair)));

	/* Build a list: (10 20 30) = (10 . (20 . (30 . nil))) */
	p_list = x_cons(p_base, X_OBJ_FLAG_NONE,
		x_mksatom(p_base, X_OBJ_FLAG_NONE, (x_obj_t *)10),
		x_cons(p_base, X_OBJ_FLAG_NONE,
			x_mksatom(p_base, X_OBJ_FLAG_NONE, (x_obj_t *)20),
			x_cons(p_base, X_OBJ_FLAG_NONE,
				x_mksatom(p_base, X_OBJ_FLAG_NONE, (x_obj_t *)30),
				NULL)));

	printf("list: (%ld %ld %ld)\n",
		x_atomint(x_car(p_list)),
		x_atomint(x_cadr(p_list)),
		x_atomint(x_caddr(p_list)));

	return 0;
}
