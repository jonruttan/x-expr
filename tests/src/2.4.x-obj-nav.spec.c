/*
 * # Unit Tests: *x-obj navigational macros*
 *
 * Tests x_0..x_1111 macros using a 4-deep nested pair tree.
 */

#define TEST_RUNNER_OVERHEAD
#include "test-runner.h"

#include "src/x-sys.c"
#include "src/x-lib.c"
#include "src/x.c"
#include "src/x-obj.c"

#include "helper-system-functions.c"


/*
 * ## Test Overhead
 */
static void _setup(void)
{
	helper_set_alloc(MEM_GUARANTEED);
}

static void _teardown(void)
{
}


/*
 * Build a 4-deep binary tree of pairs with atoms at leaves.
 * Leaf values encode the path: e.g. x_0101 -> atom with value 0x0101.
 *
 * Tree structure (depth 4):
 *   tree = pair(L2a, L2b) where
 *   L2a  = pair(L3a, L3b)
 *   L2b  = pair(L3c, L3d)
 *   L3a  = pair(atom(0x0000), atom(0x0001))   <- x_00, x_01
 *   L3b  = pair(atom(0x0010), atom(0x0011))   <- x_10, x_11
 *   L3c  = pair(atom(0x0100), atom(0x0101))
 *   L3d  = pair(atom(0x0110), atom(0x0111))
 *
 * For depth-4 accessors we need pairs containing pairs of pairs.
 */

/*
 * ## Test Runners
 */
static char *test_nav_depth1(void)
{
	x_obj_t *p_a, *p_b, *p;

	helper_alloc_reset();

	p_a = x_mksatom(NULL, 0);
	p_b = x_mksatom(NULL, 1);
	p = x_mkspair(NULL, p_a, p_b);

	_it_should("x_0 returns first", p_a == x_0(p));
	_it_should("x_1 returns rest", p_b == x_1(p));

	x_obj_free(p);
	x_obj_free(p_b);
	x_obj_free(p_a);

	return NULL;
}

static char *test_nav_depth2(void)
{
	x_obj_t *a[4], *p[2], *root;

	helper_alloc_reset();

	a[0] = x_mksatom(NULL, 0x00);
	a[1] = x_mksatom(NULL, 0x01);
	a[2] = x_mksatom(NULL, 0x10);
	a[3] = x_mksatom(NULL, 0x11);
	p[0] = x_mkspair(NULL, a[0], a[1]);
	p[1] = x_mkspair(NULL, a[2], a[3]);
	root = x_mkspair(NULL, p[0], p[1]);

	_it_should("x_00 returns a[0]", a[0] == x_00(root));
	_it_should("x_01 returns a[2]", a[2] == x_01(root));
	_it_should("x_10 returns a[1]", a[1] == x_10(root));
	_it_should("x_11 returns a[3]", a[3] == x_11(root));

	x_obj_free(root);
	x_obj_free(p[1]);
	x_obj_free(p[0]);
	x_obj_free(a[3]);
	x_obj_free(a[2]);
	x_obj_free(a[1]);
	x_obj_free(a[0]);

	return NULL;
}

static char *test_nav_depth3(void)
{
	x_obj_t *a[8], *d2[4], *d1[2], *root;
	int i;

	helper_alloc_reset();

	for (i = 0; i < 8; i++)
		a[i] = x_mksatom(NULL, i);

	d2[0] = x_mkspair(NULL, a[0], a[1]);
	d2[1] = x_mkspair(NULL, a[2], a[3]);
	d2[2] = x_mkspair(NULL, a[4], a[5]);
	d2[3] = x_mkspair(NULL, a[6], a[7]);
	d1[0] = x_mkspair(NULL, d2[0], d2[1]);
	d1[1] = x_mkspair(NULL, d2[2], d2[3]);
	root = x_mkspair(NULL, d1[0], d1[1]);

	_it_should("x_000 = a[0]", a[0] == x_000(root));
	_it_should("x_001 = a[4]", a[4] == x_001(root));
	_it_should("x_010 = a[2]", a[2] == x_010(root));
	_it_should("x_011 = a[6]", a[6] == x_011(root));
	_it_should("x_100 = a[1]", a[1] == x_100(root));
	_it_should("x_101 = a[5]", a[5] == x_101(root));
	_it_should("x_110 = a[3]", a[3] == x_110(root));
	_it_should("x_111 = a[7]", a[7] == x_111(root));

	x_obj_free(root);
	x_obj_free(d1[1]);
	x_obj_free(d1[0]);
	for (i = 3; i >= 0; i--) x_obj_free(d2[i]);
	for (i = 7; i >= 0; i--) x_obj_free(a[i]);

	return NULL;
}

static char *test_nav_depth4(void)
{
	x_obj_t *a[16], *d3[8], *d2[4], *d1[2], *root;
	int i;

	helper_alloc_reset();

	for (i = 0; i < 16; i++)
		a[i] = x_mksatom(NULL, i);

	for (i = 0; i < 8; i++)
		d3[i] = x_mkspair(NULL, a[i*2], a[i*2+1]);

	for (i = 0; i < 4; i++)
		d2[i] = x_mkspair(NULL, d3[i*2], d3[i*2+1]);

	d1[0] = x_mkspair(NULL, d2[0], d2[1]);
	d1[1] = x_mkspair(NULL, d2[2], d2[3]);
	root = x_mkspair(NULL, d1[0], d1[1]);

	_it_should("x_0000 = a[0]",   a[0]  == x_0000(root));
	_it_should("x_0001 = a[8]",   a[8]  == x_0001(root));
	_it_should("x_0010 = a[4]",   a[4]  == x_0010(root));
	_it_should("x_0011 = a[12]",  a[12] == x_0011(root));
	_it_should("x_0100 = a[2]",   a[2]  == x_0100(root));
	_it_should("x_0101 = a[10]",  a[10] == x_0101(root));
	_it_should("x_0110 = a[6]",   a[6]  == x_0110(root));
	_it_should("x_0111 = a[14]",  a[14] == x_0111(root));
	_it_should("x_1000 = a[1]",   a[1]  == x_1000(root));
	_it_should("x_1001 = a[9]",   a[9]  == x_1001(root));
	_it_should("x_1010 = a[5]",   a[5]  == x_1010(root));
	_it_should("x_1011 = a[13]",  a[13] == x_1011(root));
	_it_should("x_1100 = a[3]",   a[3]  == x_1100(root));
	_it_should("x_1101 = a[11]",  a[11] == x_1101(root));
	_it_should("x_1110 = a[7]",   a[7]  == x_1110(root));
	_it_should("x_1111 = a[15]",  a[15] == x_1111(root));

	x_obj_free(root);
	x_obj_free(d1[1]);
	x_obj_free(d1[0]);
	for (i = 3; i >= 0; i--) x_obj_free(d2[i]);
	for (i = 7; i >= 0; i--) x_obj_free(d3[i]);
	for (i = 15; i >= 0; i--) x_obj_free(a[i]);

	return NULL;
}

static char *run_tests()
{
	_run_test(test_nav_depth1);
	_run_test(test_nav_depth2);
	_run_test(test_nav_depth3);
	_run_test(test_nav_depth4);

	return NULL;
}
