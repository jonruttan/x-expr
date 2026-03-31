/*
 * # Unit Tests: *x-lisp alias macros*
 */

#define TEST_RUNNER_OVERHEAD
#include "test-runner.h"

#include "test-helper-system.c"

#include "src/x-sys.c"
#include "src/x-lib.c"
#include "src/x.c"
#include "src/x-obj.c"

#include "x-lisp.h"


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
 * ## Test Runners
 */
static char *test_cons(void)
{
	x_obj_t *p_a, *p_b, *p;

	helper_alloc_reset();

	p_a = x_mksatom(NULL, X_OBJ_FLAG_NONE, 1);
	p_b = x_mksatom(NULL, X_OBJ_FLAG_NONE, 2);
	p = x_cons(NULL, X_OBJ_FLAG_NONE, p_a, p_b);

	_it_should("create a pair with car and cdr",
		x_obj_type_isspair(p)
		&& p_a == x_car(p)
		&& p_b == x_cdr(p)
	);

	x_obj_free(NULL, p);
	x_obj_free(NULL, p_b);
	x_obj_free(NULL, p_a);

	return NULL;
}

static char *test_cXr(void)
{
	x_obj_t n[16];

	x_spair_t d3[8] = {
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[0]},  {.p = &n[1]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[2]},  {.p = &n[3]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[4]},  {.p = &n[5]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[6]},  {.p = &n[7]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[8]},  {.p = &n[9]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[10]}, {.p = &n[11]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[12]}, {.p = &n[13]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &n[14]}, {.p = &n[15]})
	};

	x_spair_t d2[4] = {
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d3[0]}, {.p = d3[1]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d3[2]}, {.p = d3[3]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d3[4]}, {.p = d3[5]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d3[6]}, {.p = d3[7]})
	};

	x_spair_t d1[2] = {
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d2[0]}, {.p = d2[1]}),
		x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d2[2]}, {.p = d2[3]})
	};

	x_spair_t root = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = d1[0]}, {.p = d1[1]});

	/* depth 1 */
	_it_should("x_car",  d1[0] == x_car(root));
	_it_should("x_cdr",  d1[1] == x_cdr(root));

	/* depth 2 */
	_it_should("x_caar", d2[0] == x_caar(root));
	_it_should("x_cadr", d2[2] == x_cadr(root));
	_it_should("x_cdar", d2[1] == x_cdar(root));
	_it_should("x_cddr", d2[3] == x_cddr(root));

	/* depth 3 */
	_it_should("x_caaar", d3[0] == x_caaar(root));
	_it_should("x_caadr", d3[4] == x_caadr(root));
	_it_should("x_cadar", d3[2] == x_cadar(root));
	_it_should("x_caddr", d3[6] == x_caddr(root));
	_it_should("x_cdaar", d3[1] == x_cdaar(root));
	_it_should("x_cdadr", d3[5] == x_cdadr(root));
	_it_should("x_cddar", d3[3] == x_cddar(root));
	_it_should("x_cdddr", d3[7] == x_cdddr(root));

	/* depth 4 */
	_it_should("x_caaaar", &n[0]  == x_caaaar(root));
	_it_should("x_caaadr", &n[8]  == x_caaadr(root));
	_it_should("x_caadar", &n[4]  == x_caadar(root));
	_it_should("x_caaddr", &n[12] == x_caaddr(root));
	_it_should("x_cadaar", &n[2]  == x_cadaar(root));
	_it_should("x_cadadr", &n[10] == x_cadadr(root));
	_it_should("x_caddar", &n[6]  == x_caddar(root));
	_it_should("x_cadddr", &n[14] == x_cadddr(root));
	_it_should("x_cdaaar", &n[1]  == x_cdaaar(root));
	_it_should("x_cdaadr", &n[9]  == x_cdaadr(root));
	_it_should("x_cdadar", &n[5]  == x_cdadar(root));
	_it_should("x_cdaddr", &n[13] == x_cdaddr(root));
	_it_should("x_cddaar", &n[3]  == x_cddaar(root));
	_it_should("x_cddadr", &n[11] == x_cddadr(root));
	_it_should("x_cdddar", &n[7]  == x_cdddar(root));
	_it_should("x_cddddr", &n[15] == x_cddddr(root));

	return NULL;
}

static char *test_setcar_setcdr(void)
{
	x_obj_t a, b, c;
	x_spair_t p = x_obj_set(NULL, X_OBJ_FLAG_NONE, {.p = &a}, {.p = &b});

	x_setcar(p, &c);
	_it_should("setcar changes car", &c == x_car(p));

	x_setcdr(p, &a);
	_it_should("setcdr changes cdr", &a == x_cdr(p));

	return NULL;
}

static char *run_tests()
{
	_run_test(test_cons);
	_run_test(test_cXr);
	_run_test(test_setcar_setcdr);

	return NULL;
}
