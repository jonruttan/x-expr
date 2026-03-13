/*
 * # Unit Tests: *x-lisp alias macros*
 */

#define TEST_RUNNER_OVERHEAD
#include "test-runner.h"

#include "src/x-sys.c"
#include "src/x-lib.c"
#include "src/x.c"
#include "src/x-obj.c"

#include "x-lisp.h"

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
 * ## Test Runners
 */
static char *test_cons(void)
{
	x_obj_t *p_a, *p_b, *p;

	helper_alloc_reset();

	p_a = x_mksatom(NULL, 1);
	p_b = x_mksatom(NULL, 2);
	p = x_cons(NULL, p_a, p_b);

	_it_should("create a pair", x_obj_type_isspair(p));
	_it_should("first is p_a", p_a == x_car(p));
	_it_should("rest is p_b", p_b == x_cdr(p));

	x_obj_free(p);
	x_obj_free(p_b);
	x_obj_free(p_a);

	return NULL;
}

static char *test_car_cdr_depth2(void)
{
	x_obj_t *a[4], *p[2], *root;

	helper_alloc_reset();

	a[0] = x_mksatom(NULL, 0);
	a[1] = x_mksatom(NULL, 1);
	a[2] = x_mksatom(NULL, 2);
	a[3] = x_mksatom(NULL, 3);
	p[0] = x_cons(NULL, a[0], a[1]);
	p[1] = x_cons(NULL, a[2], a[3]);
	root = x_cons(NULL, p[0], p[1]);

	_it_should("x_caar = a[0]", a[0] == x_caar(root));
	_it_should("x_cadr = a[2]", a[2] == x_cadr(root));
	_it_should("x_cdar = a[1]", a[1] == x_cdar(root));
	_it_should("x_cddr = a[3]", a[3] == x_cddr(root));

	x_obj_free(root);
	x_obj_free(p[1]);
	x_obj_free(p[0]);
	x_obj_free(a[3]);
	x_obj_free(a[2]);
	x_obj_free(a[1]);
	x_obj_free(a[0]);

	return NULL;
}

static char *test_car_cdr_depth3(void)
{
	x_obj_t *a[8], *d2[4], *d1[2], *root;
	int i;

	helper_alloc_reset();

	for (i = 0; i < 8; i++)
		a[i] = x_mksatom(NULL, i);

	d2[0] = x_cons(NULL, a[0], a[1]);
	d2[1] = x_cons(NULL, a[2], a[3]);
	d2[2] = x_cons(NULL, a[4], a[5]);
	d2[3] = x_cons(NULL, a[6], a[7]);
	d1[0] = x_cons(NULL, d2[0], d2[1]);
	d1[1] = x_cons(NULL, d2[2], d2[3]);
	root = x_cons(NULL, d1[0], d1[1]);

	_it_should("x_caaar = a[0]", a[0] == x_caaar(root));
	_it_should("x_caadr = a[4]", a[4] == x_caadr(root));
	_it_should("x_cadar = a[2]", a[2] == x_cadar(root));
	_it_should("x_caddr = a[6]", a[6] == x_caddr(root));
	_it_should("x_cdaar = a[1]", a[1] == x_cdaar(root));
	_it_should("x_cdadr = a[5]", a[5] == x_cdadr(root));
	_it_should("x_cddar = a[3]", a[3] == x_cddar(root));
	_it_should("x_cdddr = a[7]", a[7] == x_cdddr(root));

	x_obj_free(root);
	x_obj_free(d1[1]);
	x_obj_free(d1[0]);
	for (i = 3; i >= 0; i--) x_obj_free(d2[i]);
	for (i = 7; i >= 0; i--) x_obj_free(a[i]);

	return NULL;
}

static char *test_car_cdr_depth4(void)
{
	x_obj_t *a[16], *d3[8], *d2[4], *d1[2], *root;
	int i;

	helper_alloc_reset();

	for (i = 0; i < 16; i++)
		a[i] = x_mksatom(NULL, i);

	for (i = 0; i < 8; i++)
		d3[i] = x_cons(NULL, a[i*2], a[i*2+1]);

	for (i = 0; i < 4; i++)
		d2[i] = x_cons(NULL, d3[i*2], d3[i*2+1]);

	d1[0] = x_cons(NULL, d2[0], d2[1]);
	d1[1] = x_cons(NULL, d2[2], d2[3]);
	root = x_cons(NULL, d1[0], d1[1]);

	_it_should("x_caaaar = a[0]",   a[0]  == x_caaaar(root));
	_it_should("x_caaadr = a[8]",   a[8]  == x_caaadr(root));
	_it_should("x_caadar = a[4]",   a[4]  == x_caadar(root));
	_it_should("x_caaddr = a[12]",  a[12] == x_caaddr(root));
	_it_should("x_cadaar = a[2]",   a[2]  == x_cadaar(root));
	_it_should("x_cadadr = a[10]",  a[10] == x_cadadr(root));
	_it_should("x_caddar = a[6]",   a[6]  == x_caddar(root));
	_it_should("x_cadddr = a[14]",  a[14] == x_cadddr(root));
	_it_should("x_cdaaar = a[1]",   a[1]  == x_cdaaar(root));
	_it_should("x_cdaadr = a[9]",   a[9]  == x_cdaadr(root));
	_it_should("x_cdadar = a[5]",   a[5]  == x_cdadar(root));
	_it_should("x_cdaddr = a[13]",  a[13] == x_cdaddr(root));
	_it_should("x_cddaar = a[3]",   a[3]  == x_cddaar(root));
	_it_should("x_cddadr = a[11]",  a[11] == x_cddadr(root));
	_it_should("x_cdddar = a[7]",   a[7]  == x_cdddar(root));
	_it_should("x_cddddr = a[15]",  a[15] == x_cddddr(root));

	x_obj_free(root);
	x_obj_free(d1[1]);
	x_obj_free(d1[0]);
	for (i = 3; i >= 0; i--) x_obj_free(d2[i]);
	for (i = 7; i >= 0; i--) x_obj_free(d3[i]);
	for (i = 15; i >= 0; i--) x_obj_free(a[i]);

	return NULL;
}

static char *test_setcar_setcdr(void)
{
	x_obj_t *p_a, *p_b, *p_c, *p;

	helper_alloc_reset();

	p_a = x_mksatom(NULL, 1);
	p_b = x_mksatom(NULL, 2);
	p_c = x_mksatom(NULL, 3);
	p = x_cons(NULL, p_a, p_b);

	_it_should("car is p_a", p_a == x_car(p));
	_it_should("cdr is p_b", p_b == x_cdr(p));

	x_setcar(p, p_c);
	_it_should("setcar changes car to p_c", p_c == x_car(p));

	x_setcdr(p, p_a);
	_it_should("setcdr changes cdr to p_a", p_a == x_cdr(p));

	x_obj_free(p);
	x_obj_free(p_c);
	x_obj_free(p_b);
	x_obj_free(p_a);

	return NULL;
}

static char *run_tests()
{
	_run_test(test_cons);
	_run_test(test_car_cdr_depth2);
	_run_test(test_car_cdr_depth3);
	_run_test(test_car_cdr_depth4);
	_run_test(test_setcar_setcdr);

	return NULL;
}
