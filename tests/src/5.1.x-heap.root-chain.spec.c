/*
 * # Unit Tests: *x-heap* root chain (precise stack roots)
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
static x_obj_t *test_make_heap_base(void)
{
	struct x_base_t base = {
		0, 0, 0,
		NULL, NULL, NULL, NULL,
		0,
		NULL, NULL
	};
	return x_base_make(NULL, base);
}


/*
 * ## Test Runners
 */
static char *test_root_chain_push_pop(void)
{
	x_obj_t *p_base, **p_slot;
	x_spair_t root_a = x_obj_set((x_obj_t *)x_type_pair_obj,
		X_OBJ_FLAG_NONE, { NULL }, { NULL });
	x_spair_t root_b = x_obj_set((x_obj_t *)x_type_pair_obj,
		X_OBJ_FLAG_NONE, { NULL }, { NULL });

	helper_alloc_reset();

	/* Unset base: no chain, no collector -- registration must degrade
	 * to a no-op so NULL-base callers (e.g. x_eval(NULL, ...)) work.
	 * Pass a TYPED null base (what x_eval(NULL, ...) holds internally),
	 * not the bare NULL token: x_base_isset() is a runtime test, so the
	 * compiler still type-checks the macro's dead &x_heap_root_chain(B)
	 * branch -- and that field chase on a void* literal will not compile. */
	p_base = NULL;
	p_slot = x_heap_root_slot(p_base);
	_it_should("yield a nil slot without a base", NULL == p_slot);
	x_heap_root_push(p_slot, root_a);
	x_heap_root_pop(p_slot);
	_it_should("leave a nil-slot push/pop inert",
		NULL == x_obj_heap((x_obj_t *)root_a));

	p_base = test_make_heap_base();
	p_slot = x_heap_root_slot(p_base);

	_it_should("start with an empty root chain",
		NULL == x_heap_root_chain(p_base));

	x_heap_root_push(p_slot, root_a);
	_it_should("head the chain at the pushed node",
		(x_obj_t *)root_a == x_heap_root_chain(p_base));

	x_heap_root_push(p_slot, root_b);
	_it_should("head the chain at the newer node",
		(x_obj_t *)root_b == x_heap_root_chain(p_base));
	_it_should("link the newer node to the older one",
		(x_obj_t *)root_a == x_obj_heap((x_obj_t *)root_b));

	x_heap_root_pop(p_slot);
	_it_should("pop back to the older node",
		(x_obj_t *)root_a == x_heap_root_chain(p_base));

	x_heap_root_pop(p_slot);
	_it_should("pop back to the empty chain",
		NULL == x_heap_root_chain(p_base));

	return NULL;
}

static char *test_root_chain_mark_survives_sweep(void)
{
	x_obj_t *p_base, *p_a, *p_b, **p_slot;
	x_spair_t root = x_obj_set((x_obj_t *)x_type_pair_obj,
		X_OBJ_FLAG_NONE, { NULL }, { NULL });
	int n;

	helper_alloc_reset();

	p_base = test_make_heap_base();
	p_slot = x_heap_root_slot(p_base);

	/* Two heap atoms whose only references live in the stack-storage pair. */
	p_a = x_mksatom(p_base, X_OBJ_FLAG_NONE, 41);
	p_b = x_mksatom(p_base, X_OBJ_FLAG_NONE, 42);
	x_firstobj((x_obj_t *)root) = p_a;
	x_restobj((x_obj_t *)root) = p_b;
	x_heap_root_push(p_slot, root);

	n = helper_free_count();
	x_heap_root_chain_mark(p_base, X_OBJ_FLAG_MARK);
	x_heap_sweep(p_base, x_obj_heap(p_base), X_OBJ_FLAG_MARK);
	_it_should("retain both registered referents across a sweep",
		0 == helper_free_count() - n);
	_it_should("keep the referents' values intact",
		41 == x_atomint(p_a) && 42 == x_atomint(p_b));

	/* Second cycle with the node still registered: the stale-mark
	 * regression.  The node is off the allocation chain, so the sweep
	 * did not clear its mark flag; without the walk's pre-clear pass
	 * the node is skipped as already-marked and its referents are
	 * freed while live. */
	n = helper_free_count();
	x_heap_root_chain_mark(p_base, X_OBJ_FLAG_MARK);
	x_heap_sweep(p_base, x_obj_heap(p_base), X_OBJ_FLAG_MARK);
	_it_should("retain the referents across a second cycle",
		0 == helper_free_count() - n);

	/* Popped: the referents are unreachable and must be reclaimed. */
	x_heap_root_pop(p_slot);
	n = helper_free_count();
	x_heap_root_chain_mark(p_base, X_OBJ_FLAG_MARK);
	x_heap_sweep(p_base, x_obj_heap(p_base), X_OBJ_FLAG_MARK);
	_it_should("reclaim both referents once unregistered",
		2 == helper_free_count() - n);

	return NULL;
}

static char *test_root_chain_mark_walks_all_nodes(void)
{
	x_obj_t *p_base, *p_a, *p_b, **p_slot;
	/* root_b's payload rest links root_a -- the stack-built argument
	 * list shape (stack pair chaining to stack pair). */
	x_spair_t root_a = x_obj_set((x_obj_t *)x_type_pair_obj,
		X_OBJ_FLAG_NONE, { NULL }, { NULL });
	x_spair_t root_b = x_obj_set((x_obj_t *)x_type_pair_obj,
		X_OBJ_FLAG_NONE, { NULL }, { NULL });
	int n;

	helper_alloc_reset();

	_it_should("no-op without a base",
		NULL == x_heap_root_chain_mark(NULL, X_OBJ_FLAG_MARK));

	p_base = test_make_heap_base();
	p_slot = x_heap_root_slot(p_base);

	p_a = x_mksatom(p_base, X_OBJ_FLAG_NONE, 1);
	p_b = x_mksatom(p_base, X_OBJ_FLAG_NONE, 2);
	x_firstobj((x_obj_t *)root_a) = p_a;
	x_heap_root_push(p_slot, root_a);
	x_firstobj((x_obj_t *)root_b) = p_b;
	x_restobj((x_obj_t *)root_b) = (x_obj_t *)root_a;
	x_heap_root_push(p_slot, root_b);

	/* Two cycles: the second exercises every node's stale mark, both
	 * as a chain node (pre-clear pass) and as another node's payload
	 * (tree-mark traversal into an already-registered node). */
	n = helper_free_count();
	x_heap_root_chain_mark(p_base, X_OBJ_FLAG_MARK);
	x_heap_sweep(p_base, x_obj_heap(p_base), X_OBJ_FLAG_MARK);
	x_heap_root_chain_mark(p_base, X_OBJ_FLAG_MARK);
	x_heap_sweep(p_base, x_obj_heap(p_base), X_OBJ_FLAG_MARK);
	_it_should("retain referents of every chain node across two cycles",
		0 == helper_free_count() - n);

	x_heap_root_pop(p_slot);
	x_heap_root_pop(p_slot);
	n = helper_free_count();
	x_heap_root_chain_mark(p_base, X_OBJ_FLAG_MARK);
	x_heap_sweep(p_base, x_obj_heap(p_base), X_OBJ_FLAG_MARK);
	_it_should("reclaim every referent once the chain unwinds",
		2 == helper_free_count() - n);

	return NULL;
}

static char *run_tests()
{
	_run_test(test_root_chain_push_pop);
	_run_test(test_root_chain_mark_survives_sweep);
	_run_test(test_root_chain_mark_walks_all_nodes);

	return NULL;
}
