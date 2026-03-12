#ifndef X_HEAP_H
#define X_HEAP_H

/*
 * # Computational Expressions in C
 *
 * ## x-heap.h -- Header - Heap Management
 *
 * @description Computational Expressions in C
 * @author [Jon Ruttan](jonruttan@gmail.com)
 * @copyright 2021 Jon Ruttan
 * @license MIT No Attribution (MIT-0)
 *
 *     ., .,
 *     {O,O}
 *     (   )
 *      " "
 */
/*
 * # Includes
 */
#include "x-obj.h"

#ifndef X_HEAP
#warning X_HEAP is required
#else /* X_HEAP */

/*
 * # Heap Management Functions
 */
x_obj_t *x_heap_mark(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags);
x_obj_t *x_heap_sweep(x_obj_t *p_base, x_obj_t *p_obj, x_obj_flag_t flags);
#endif /* X_HEAP */

#endif /* X_HEAP_H */
