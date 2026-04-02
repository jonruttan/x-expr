/*
 * # Unit Tests: *x-heap*
 */

#define TEST_RUNNER_OVERHEAD
#include "test-runner.h"

#ifndef X_HEAP
#define X_HEAP
#endif /* X_HEAP */

#include "test-helper-system.c"

#include "src/x-sys.c"
#include "src/x-lib.c"
#include "src/x-obj.c"
#include "src/x.c"
#include "src/x-base.c"
#include "src/x-heap.c"


/*
 * ## Test Overhead
 */

static void _setup(void)
{
	helper_set_alloc(MEM_GUARANTEED);
	helper_sys_funcs.exit = mock_exit;
	helper_sys_funcs.malloc = helper_malloc;
	helper_sys_funcs.free = helper_free;
}

static void _teardown(void)
{
}


/*
 * ## Test Helpers
 */
static x_obj_t *_mark_fn_continue(x_obj_t *p_base, x_obj_t *p_obj,
	x_obj_flag_t flags)
{
	return p_obj;
}

static x_obj_t *_mark_fn_stop(x_obj_t *p_base, x_obj_t *p_obj,
	x_obj_flag_t flags)
{
	return NULL;
}

static int _free_fn_count;

static void _free_fn(x_obj_t *p_base, x_obj_t *p_obj)
{
	_free_fn_count++;
}

static x_obj_t *test_make_heap_base(x_obj_t *p_heap_mark, x_obj_t *p_heap_free,
	void *p_stack_base)
{
	struct x_base_t base = {
		0, 0, 0,
		NULL, NULL, NULL, NULL,
		0,
		p_heap_mark, p_heap_free,
		p_stack_base
	};
	return x_base_make(NULL, base);
}


/*
 * ## Test Runners
 */
static char *test_heap_tree_mark(void)
{
	x_obj_t *p_base, *p_obj[3], *p_ret;
	x_satom_t mark_fn = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.v = (void *)_mark_fn_continue});

	helper_alloc_reset();

	/* Atoms */
	p_obj[0] = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);

	p_ret = x_heap_tree_mark(NULL, p_obj[0], X_OBJ_FLAG_HEAP);
	_it_should("return the object", p_ret == p_obj[0]);
	_it_should("mark the atom",
		X_OBJ_FLAG_HEAP == x_obj_flags(p_obj[0]));

	x_sys_free(p_obj[0]);

	/* Pairs */
	p_obj[0] = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("set the pair's first object's flags to 0", x_obj_flags(p_obj[0]) == 0);
	p_obj[1] = x_mksatom(NULL, X_OBJ_FLAG_NONE, 1);
	_it_should("set the pair's rest object's flags to 0", x_obj_flags(p_obj[1]) == 0);

	p_obj[2] = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_obj[0], p_obj[1]);
	_it_should("set the pair object's flags to 0", x_obj_flags(p_obj[2]) == 0);

	p_ret = x_heap_tree_mark(NULL, p_obj[2], X_OBJ_FLAG_HEAP);
	_it_should("mark the pair's first object", X_OBJ_FLAG_HEAP == x_obj_flags(p_obj[0]));
	_it_should("mark the pair's rest object", X_OBJ_FLAG_HEAP == x_obj_flags(p_obj[1]));
	_it_should("mark the pair object itself", X_OBJ_FLAG_HEAP == x_obj_flags(p_obj[2]));

	x_sys_free(p_obj[2]);
	x_sys_free(p_obj[1]);
	x_sys_free(p_obj[0]);

	/* Pairs (Recursive) */
	p_obj[0] = x_mkspair(NULL, X_OBJ_FLAG_NONE, NULL, NULL);
	_it_should("set the recursive pair's flags to 0", x_obj_flags(p_obj[0]) == 0);
	p_obj[1] = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_obj[0], NULL);
	_it_should("set the second pair's flags to 0", x_obj_flags(p_obj[1]) == 0);

	p_obj[2] = x_mkspair(NULL, X_OBJ_FLAG_NONE, p_obj[0], p_obj[1]);
	_it_should("set the root pair's flags to 0", x_obj_flags(p_obj[2]) == 0);

	x_firstobj(p_obj[0]) = (x_obj_t *)p_obj[2];
	x_restobj(p_obj[0]) = (x_obj_t *)p_obj[2];
	x_restobj(p_obj[1]) = (x_obj_t *)p_obj[2];

	p_ret = x_heap_tree_mark(NULL, p_obj[2], X_OBJ_FLAG_HEAP);
	_it_should("mark the root pair", X_OBJ_FLAG_HEAP == x_obj_flags(p_obj[2]));
	_it_should("mark the recursive pair", X_OBJ_FLAG_HEAP == x_obj_flags(p_obj[0]));
	_it_should("mark the second pair", X_OBJ_FLAG_HEAP == x_obj_flags(p_obj[1]));

	x_sys_free(p_obj[2]);
	x_sys_free(p_obj[1]);
	x_sys_free(p_obj[0]);

	/* Mark hook: continue path */
	helper_alloc_reset();

	p_base = test_make_heap_base((x_obj_t *)mark_fn, NULL, NULL);
	p_obj[0] = x_mksatom(p_base, X_OBJ_FLAG_NONE, 0);
	p_ret = x_heap_tree_mark(p_base, p_obj[0], X_OBJ_FLAG_HEAP);
	_it_should("mark the object via mark_fn continue path",
		X_OBJ_FLAG_HEAP == (x_obj_flags(p_obj[0]) & X_OBJ_FLAG_HEAP));
	_it_should("return the object after already-marked check",
		p_ret == p_obj[0]);

	/* Mark hook: stop path */
	x_atomptr((x_obj_t *)mark_fn) = (void *)_mark_fn_stop;
	p_obj[0] = x_mksatom(p_base, X_OBJ_FLAG_NONE, 0);
	p_ret = x_heap_tree_mark(p_base, p_obj[0], X_OBJ_FLAG_HEAP);
	_it_should("mark the object via mark_fn stop path",
		X_OBJ_FLAG_HEAP == (x_obj_flags(p_obj[0]) & X_OBJ_FLAG_HEAP));
	_it_should("return NULL when mark_fn stops", p_ret == NULL);

	return NULL;
}

static char *test_heap_vector_mark(void)
{
	x_obj_t *p_base, *p_obj[3];
	void *vector[4];

	helper_alloc_reset();

	/* Create a base with objects on the heap chain */
	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	p_obj[0] = x_mksatom(p_base, X_OBJ_FLAG_NONE, 0);
	p_obj[1] = x_mksatom(p_base, X_OBJ_FLAG_NONE, 1);
	p_obj[2] = x_mksatom(p_base, X_OBJ_FLAG_NONE, 2);

	/* Put some object addresses into a vector, with a non-match */
	vector[0] = (void *)p_obj[0];
	vector[1] = (void *)0xDEADBEEF;
	vector[2] = (void *)p_obj[2];
	vector[3] = NULL;

	x_heap_vector_mark(p_base, vector, sizeof(vector), X_OBJ_FLAG_HEAP);

	_it_should("mark object found in vector",
		X_OBJ_FLAG_HEAP == (x_obj_flags(p_obj[0]) & X_OBJ_FLAG_HEAP)
	);

	_it_should("not mark object not in vector",
		0 == (x_obj_flags(p_obj[1]) & X_OBJ_FLAG_HEAP)
	);

	_it_should("mark second object found in vector",
		X_OBJ_FLAG_HEAP == (x_obj_flags(p_obj[2]) & X_OBJ_FLAG_HEAP)
	);

	return NULL;
}

X_NO_OPTIMIZE static char *test_heap_callstack_mark(void)
{
	x_obj_t *p_base, *p_obj[2];

	helper_alloc_reset();

	p_base = test_make_heap_base(NULL, NULL, _stack_base);
	p_obj[0] = x_mksatom(p_base, X_OBJ_FLAG_NONE, rand());
	p_obj[1] = NULL;

	/* locals[0] is on the C stack.
	 * callstack_mark should find it and mark it. */
	x_heap_callstack_mark(p_base, X_OBJ_FLAG_HEAP);

	_it_should("mark object referenced from C stack",
		X_OBJ_FLAG_HEAP == (x_obj_flags(p_obj[0]) & X_OBJ_FLAG_HEAP)
	);

	return NULL;
}

static char *test_heap_sweep(void)
{
	x_obj_t *p_base, *p_obj[4], *p_ret;
	x_satom_t free_fn = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.v = (void *)_free_fn});
	x_char_t *s;
	int n;


	/* # Single object */
	helper_alloc_reset();

	/* Create the base object */
	n = helper_alloc_count();
	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("allocate memory for the base object", 1 == helper_alloc_count() - n);
	_it_should("set the base object's flags to 0", 0 == x_obj_flags(p_base));
	_it_should("set the base object's gc pointer to NULL", NULL == x_obj_heap(p_base));

	/* Create an object */
	n = helper_alloc_count();
	p_obj[0] = x_obj_alloc(p_base, NULL, X_OBJ_FLAG_RO|X_OBJ_FLAG_HEAP, 0);
	_it_should("allocate memory for the object", 1 == helper_alloc_count() - n);
	_it_should("set the object's flags to GC,RO", (X_OBJ_FLAG_RO|X_OBJ_FLAG_HEAP) == x_obj_flags(p_obj[0]));
	_it_should("set the object's gc pointer to NULL", NULL == x_obj_heap(p_obj[0]));
	_it_should("set the base object's gc pointer to the object", p_obj[0] == x_obj_heap(p_base));

	/* Sweep the RO flag from the object */
	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, p_obj[0], X_OBJ_FLAG_RO);
	_it_should("not have freed the object's memory", 0 == helper_free_count() - n);
	_it_should("return the base object", p_base == p_ret);
	_it_should("cleared RO bit in the object's flags", X_OBJ_FLAG_HEAP == x_obj_flags(p_obj[0]));

	/* Garbage collect everything */
	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, p_base, X_OBJ_FLAG_NONE);
	_it_should("free all of the objects", 2 == helper_free_count() - n);
	_it_should("returned the base object", p_base == p_ret);
	_it_should("have freed all allocated memory", helper_free_count() == helper_alloc_count());


	/* Add an orphaned object */

	/* Create the base object */
	helper_alloc_reset();
	n = helper_alloc_count();
	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("allocate memory for the base object", 1 == helper_alloc_count() - n);
	_it_should("set the base object's flags to 0", 0 == x_obj_flags(p_base));
	_it_should("set the base base object's gc pointer to NULL", NULL == x_obj_heap(p_base));

	/* Create an object */
	n = helper_alloc_count();
	p_obj[0] = x_obj_alloc(p_base, NULL, X_OBJ_FLAG_HEAP, 0);
	_it_should("allocate memory for the object", 1 == helper_alloc_count() - n);
	_it_should("set the object's flags to GC", X_OBJ_FLAG_HEAP == x_obj_flags(p_obj[0]));
	_it_should("set the object's gc pointer to NULL", NULL == x_obj_heap(p_obj[0]));
	_it_should("set the base object's gc pointer to the object", p_obj[0] == x_obj_heap(p_base));

	n = helper_alloc_count();
	p_obj[1] = x_obj_alloc(p_base, NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("allocate memory for the orphaned object", 1 == helper_alloc_count() - n);
	_it_should("set the orphaned object's flags to 0", 0 == x_obj_flags(p_obj[1]));
	_it_should("set the orphaned object's gc pointer to the first object", p_obj[0] == x_obj_heap(p_obj[1]));
	_it_should("set the base object's gc pointer to the orphaned object", p_obj[1] == x_obj_heap(p_base));

	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, x_obj_heap(p_base), X_OBJ_FLAG_HEAP);
	_it_should("have freed the orphaned object's memory", 1 == helper_free_count() - n);
	_it_should("return the base object", p_base == p_ret);
	_it_should("set the base object's gc pointer to the first object", p_obj[0] == x_obj_heap(p_base));

	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, p_base, X_OBJ_FLAG_NONE);
	_it_should("have freed the object's memory", 2 == helper_free_count() - n);
	_it_should("return the base object", p_base == p_ret);
	_it_should("have freed all allocated memory", helper_free_count() == helper_alloc_count());



	/* # Atom */
	helper_alloc_reset();

	/* Create the base object */
	n = helper_alloc_count();
	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("allocate memory for the base object", 1 == helper_alloc_count() - n);
	_it_should("set the base object's flags to 0", 0 == x_obj_flags(p_base));
	_it_should("set the base object's gc pointer to NULL", NULL == x_obj_heap(p_base));

	/* Create an Atom object. */
	n = helper_alloc_count();
	p_obj[0] = x_mksatom(p_base, X_OBJ_FLAG_RO|X_OBJ_FLAG_HEAP, 0);
	_it_should("allocate memory for the object", 1 == helper_alloc_count() - n);
	_it_should("set the object's flags to GC,RO", (X_OBJ_FLAG_RO|X_OBJ_FLAG_HEAP) == x_obj_flags(p_obj[0]));
	_it_should("set the object's gc pointer to NULL", NULL == x_obj_heap(p_obj[0]));
	_it_should("set the base object's gc pointer to the object", p_obj[0] == x_obj_heap(p_base));

	/* Sweep the RO flag from the object */
	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, p_obj[0], X_OBJ_FLAG_RO);
	_it_should("not have freed the object's memory", 0 == helper_free_count() - n);
	_it_should("return the base object", p_base == p_ret);
	_it_should("cleared RO bit in the object's flags", X_OBJ_FLAG_HEAP == x_obj_flags(p_obj[0]));

	/* Garbage collect everything */
	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, p_base, X_OBJ_FLAG_NONE);
	_it_should("free the object's memory", 2 == helper_free_count() - n);
	_it_should("return the base object", p_base == p_ret);
	_it_should("have freed all allocated memory", helper_free_count() == helper_alloc_count());


	/* Add an orphaned object */
	helper_alloc_reset();

	/* Create the base object */
	n = helper_alloc_count();
	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("allocate memory for the base object", 1 == helper_alloc_count() - n);
	_it_should("set the base object's flags to 0", 0 == x_obj_flags(p_base));
	_it_should("set the base object's gc pointer to NULL", NULL == x_obj_heap(p_base));

	/* Create an atom object */
	n = helper_alloc_count();
	p_obj[0] = x_mksatom(p_base, X_OBJ_FLAG_HEAP, 0);
	_it_should("allocate memory for the object", 1 == helper_alloc_count() - n);
	_it_should("set the object's flags to GC", X_OBJ_FLAG_HEAP == x_obj_flags(p_obj[0]));
	_it_should("set the object's gc pointer to NULL", NULL == x_obj_heap(p_obj[0]));
	_it_should("set the base object's gc pointer to the object", p_obj[0] == x_obj_heap(p_base));

	/* Create another atom object */
	n = helper_alloc_count();
	p_obj[1] = x_mksatom(p_base, X_OBJ_FLAG_NONE, 0);
	_it_should("allocate memory for the orphaned object", 1 == helper_alloc_count() - n);
	_it_should("set the orphaned object's flags to 0", 0 == x_obj_flags(p_obj[1]));
	_it_should("set the orphaned object's gc pointer to the previous object", p_obj[0] == x_obj_heap(p_obj[1]));
	_it_should("set the base object's gc pointer to the object", p_obj[1] == x_obj_heap(p_base));

	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, x_obj_heap(p_base), X_OBJ_FLAG_HEAP);
	_it_should("have freed the orphaned object's memory", 1 == helper_free_count() - n);
	_it_should("return the base object", p_base == p_ret);
	_it_should("set the base object's gc pointer to the first object", p_obj[0] == x_obj_heap(p_base));

	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, p_base, X_OBJ_FLAG_NONE);
	_it_should("have freed the object's memory", 2 == helper_free_count() - n);
	_it_should("return the base object", p_base == p_ret);
	_it_should("have freed all allocated memory", helper_alloc_count() == helper_free_count());


	/* Owner Atom */
	helper_alloc_reset();

	/* Create the base object */
	n = helper_alloc_count();
	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("allocate memory for the base object", 1 == helper_alloc_count() - n);
	_it_should("set the base object's flags to 0", 0 == x_obj_flags(p_base));
	_it_should("set the base object's gc pointer to NULL", NULL == x_obj_heap(p_base));

	/* Create an Owner Atom object. */
	n = helper_alloc_count();
	s = (x_char_t *)"ATOMOWN";
	p_obj[0] = x_mksatomown(p_base, X_OBJ_FLAG_RO|X_OBJ_FLAG_HEAP, (x_obj_t *)x_lib_memdup(s, strlen((char *)s) + 1));
	_it_should("allocate memory for the object", 2 == helper_alloc_count() - n);
	_it_should("set the object's flags to OWN,RO,GC", (X_OBJ_FLAG_OWN|X_OBJ_FLAG_RO|X_OBJ_FLAG_HEAP) == x_obj_flags(p_obj[0]));
	_it_should("set the object's gc pointer to NULL", NULL == x_obj_heap(p_obj[0]));
	_it_should("set the base object's gc pointer to the object", p_obj[0] == x_obj_heap(p_base));

	/* Sweep the RO flag from the object */
	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, p_obj[0], X_OBJ_FLAG_RO);
	_it_should("not have freed the object's memory and the owned resource", 0 == helper_free_count() - n);
	_it_should("return the base object",  p_base == p_ret);
	_it_should("cleared RO bit in the object's flags", (X_OBJ_FLAG_OWN|X_OBJ_FLAG_HEAP) == x_obj_flags(p_obj[0]));

	/* Garbage collect everything */
	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, p_base, X_OBJ_FLAG_NONE);
	_it_should("free the object's memory and the owned resource", 3 == helper_free_count() - n);
	_it_should("return the base object", p_base == p_ret);
	_it_should("have freed all allocated memory", helper_free_count() == helper_alloc_count());


	/* Add an orphaned object */
	helper_alloc_reset();

	/* Create the base object */
	n = helper_alloc_count();
	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("allocate memory for the base object", 1 == helper_alloc_count() - n);
	_it_should("set the base object's flags to 0", 0 == x_obj_flags(p_base));
	_it_should("set the base object's gc pointer to NULL", NULL == x_obj_heap(p_base));

	/* Create an Owner Atom object. */
	n = helper_alloc_count();
	s = (x_char_t *)"ATOMOWN1";
	p_obj[0] = x_mksatomown(p_base, X_OBJ_FLAG_HEAP, (x_obj_t *)x_lib_memdup(s, strlen((char *)s) + 1));
	_it_should("allocate memory for the object and its resource", 2 == helper_alloc_count() - n);
	_it_should("set the object's flags to OWN,GC", (X_OBJ_FLAG_OWN|X_OBJ_FLAG_HEAP) == x_obj_flags(p_obj[0]));
	_it_should("set the object's gc pointer to NULL", NULL == x_obj_heap(p_obj[0]));
	_it_should("set the base object's gc pointer to the object", p_obj[0] == x_obj_heap(p_base));

	/* Create an Owner Atom object. */
	n = helper_alloc_count();
	s = (x_char_t *)"ATOMOWN2";
	p_obj[1] = x_mksatomown(p_base, X_OBJ_FLAG_NONE, (x_obj_t *)x_lib_memdup(s, strlen((char *)s) + 1));
	_it_should("allocate memory for the orphaned object and its resource", 2 == helper_alloc_count() - n);
	_it_should("set the orphaned object's flags to OWN", X_OBJ_FLAG_OWN == x_obj_flags(p_obj[1]));
	_it_should("set the orphaned object's gc pointer to the previous object", p_obj[0] == x_obj_heap(p_obj[1]));
	_it_should("set the base object's gc pointer to the object", p_obj[1] == x_obj_heap(p_base));

	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, x_obj_heap(p_base), X_OBJ_FLAG_HEAP);
	_it_should("have freed the orphaned object's memory and the owned resource", 2 ==helper_free_count() - n);
	_it_should("return the base object",  p_base == p_ret);
	_it_should("set the base object's gc pointer to the first object", p_obj[0] == x_obj_heap(p_base));

	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, p_base, X_OBJ_FLAG_NONE);
	_it_should("have freed the object's memory and the owned resource", 3 == helper_free_count() - n);
	_it_should("return the object", p_base == p_ret);
	_it_should("have freed all allocated memory", helper_alloc_count() == helper_free_count());


	/* Pair */
	helper_alloc_reset();

	/* Create the Base object */
	n = helper_alloc_count();
	p_base = x_mksatom(NULL, X_OBJ_FLAG_NONE, 0);
	_it_should("allocate memory for the base object", 1 == helper_alloc_count() - n);
	_it_should("set the base object's flags to 0", 0 == x_obj_flags(p_base));
	_it_should("set the base object's gc pointer to NULL", NULL == x_obj_heap(p_base));

	/* Create an Atom object */
	n = helper_alloc_count();
	p_obj[0] = x_mksatom(p_base, X_OBJ_FLAG_RO|X_OBJ_FLAG_HEAP, "ATOM");
	_it_should("allocate memory for the atom object", 1 == helper_alloc_count() - n);
	_it_should("set the object's flags to RO,GC", (X_OBJ_FLAG_RO|X_OBJ_FLAG_HEAP) == x_obj_flags(p_obj[0]));
	_it_should("set the object's gc pointer to NULL", NULL == x_obj_heap(p_obj[0]));
	_it_should("set the base's gc pointer to the object", p_obj[0] == x_obj_heap(p_base));

	/* Create an Owner Atom object */
	n = helper_alloc_count();
	s = (x_char_t *)"ATOMOWN";
	p_obj[1] = x_mksatomown(p_base, X_OBJ_FLAG_RO|X_OBJ_FLAG_HEAP, x_lib_memdup(s, strlen((char *)s) + 1));
	_it_should("allocate memory for the atomown object", 2 == helper_alloc_count() - n);
	_it_should("set the object's flags to OWN,RO,GC", (X_OBJ_FLAG_OWN|X_OBJ_FLAG_RO|X_OBJ_FLAG_HEAP) == x_obj_flags(p_obj[1]));
	_it_should("set the object's gc pointer to the previous object", p_obj[0] == x_obj_heap(p_obj[1]));
	_it_should("set the base's gc pointer to the object", p_obj[1] == x_obj_heap(p_base));

	/* Create a Pair containing the two objects */
	n = helper_alloc_count();
	p_obj[2] = x_mkspair(p_base, X_OBJ_FLAG_RO|X_OBJ_FLAG_HEAP, p_obj[0], p_obj[1]);
	_it_should("allocate memory for the pair object", 1 == helper_alloc_count() -n);
	_it_should("set the object's flags to GC,RO", (X_OBJ_FLAG_RO|X_OBJ_FLAG_HEAP) == x_obj_flags(p_obj[2]));
	_it_should("set the object's gc pointer to the previous object", p_obj[1] == x_obj_heap(p_obj[2]));
	_it_should("set the base's gc pointer to the object", p_obj[2] == x_obj_heap(p_base));

	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, p_obj[2], X_OBJ_FLAG_RO);
	_it_should("not have freed the object's memory", 0 == helper_free_count() - n);
	_it_should("return the base object", p_base == p_ret);
	_it_should("cleared RO bit in the object's flags", X_OBJ_FLAG_HEAP == x_obj_flags(p_obj[2]));

	/* Add an orphaned object */
	n = helper_alloc_count();
	p_obj[3] = x_mksatom(p_base, X_OBJ_FLAG_NONE, 0);
	_it_should("allocate memory for the orphaned object", 1 == helper_alloc_count() - n);
	_it_should("set the orphaned object's flags to 0", 0 == x_obj_flags(p_obj[3]));
	_it_should("set the orphaned object's gc pointer to the pair object", p_obj[2] == x_obj_heap(p_obj[3]));
	_it_should("set the object's gc pointer to the orphaned object", p_obj[3] == x_obj_heap(p_base));

	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, x_obj_heap(p_base), X_OBJ_FLAG_HEAP);
	_it_should("have freed the orphaned object's memory", 1 == helper_free_count() - n);
	_it_should("return the base object", p_base == p_ret);
	_it_should("set the object's gc pointer to the pair object", p_obj[2] == x_obj_heap(p_base));

	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, p_base, X_OBJ_FLAG_NONE);
	_it_should("have freed the object's memory", 5 == helper_free_count() - n);
	_it_should("return the object", p_base == p_ret);
	_it_should("have freed all allocated memory", helper_free_count() == helper_alloc_count());

	/* Free hook */
	helper_alloc_reset();
	_free_fn_count = 0;

	p_base = test_make_heap_base(NULL, (x_obj_t *)free_fn, NULL);
	x_obj_alloc(p_base, NULL, X_OBJ_FLAG_NONE, 0);

	n = helper_free_count();
	p_ret = x_heap_sweep(p_base, x_obj_heap(p_base), X_OBJ_FLAG_RO);
	_it_should("call the free callback", 1 == _free_fn_count);
	_it_should("free the object via callback", 1 == helper_free_count() - n);
	_it_should("return the base object after free callback", p_base == p_ret);

	return NULL;
}

static char *run_tests()
{
	_run_test(test_heap_tree_mark);
	_run_test(test_heap_vector_mark);
	_run_test(test_heap_callstack_mark);
	_run_test(test_heap_sweep);

	return NULL;
}
