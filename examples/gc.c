/*
 * gc.c -- garbage collection example
 *
 * Demonstrates allocating objects on the heap, marking roots,
 * and sweeping unreachable objects.
 *
 * Build:  make gc
 * Run:    ./gc
 */

#include <stdio.h>
#include <unistd.h>

#include "x-heap.h"
#include "x-lisp.h"

int main(int argc, char **argv)
{
	x_obj_t *p_base, *p_root, *p_garbage;
	struct x_base_t base = {
		STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO,
		NULL, NULL, NULL, NULL,
		0, NULL, NULL, &argc
	};

	p_base = x_base_make(NULL, base);

	/* Allocate objects -- all land on the heap chain */
	p_root = x_cons(p_base, X_OBJ_FLAG_NONE,
		x_mksatom(p_base, X_OBJ_FLAG_NONE, (x_obj_t *)100),
		x_mksatom(p_base, X_OBJ_FLAG_NONE, (x_obj_t *)200));

	p_garbage = x_mksatom(p_base, X_OBJ_FLAG_NONE, (x_obj_t *)999);
	(void)p_garbage; /* intentionally unreachable */

	printf("before GC: root=(%ld . %ld), garbage=%ld\n",
		x_atomint(x_car(p_root)),
		x_atomint(x_cdr(p_root)),
		(long)999);

	/* Mark phase: only mark the root tree */
	x_heap_tree_mark(p_base, p_root, X_OBJ_FLAG_1);
	x_heap_tree_mark(p_base, p_base, X_OBJ_FLAG_1);

	/* Sweep phase: free anything unmarked */
	x_heap_sweep(p_base, x_obj_heap(p_base), X_OBJ_FLAG_1);

	printf("after GC:  root=(%ld . %ld), garbage collected\n",
		x_atomint(x_car(p_root)),
		x_atomint(x_cdr(p_root)));

	return 0;
}
