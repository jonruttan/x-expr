/*
 * # Unit Tests: *x-obj*
 */

#include "test-runner.h"

/* Include Garbage Collection structures. */
#ifndef X_GC
#define X_GC
#endif /* X_GC */

/* Include Type structures. */
#ifndef X_TYPE
#define X_TYPE
#endif /* X_TYPE */

#include "src/x-sys.c"
#include "src/x-lib.c"
#include "src/x.c"
#include "src/x-obj.c"

#include "helper-system-functions.c"


/*
 * ## Test Overhead
 */
static void setup(void)
{
	helper_set_alloc(MEM_GUARANTEED);
}

static void teardown(void)
{
}

static char *run_tests()
{
	return NULL;
}
