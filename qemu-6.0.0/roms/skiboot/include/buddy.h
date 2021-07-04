/* Copyright 2016 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Simple power-of-two buddy allocation mechanism.
 *
 */
#ifndef __BUDDY_H
#define __BUDDY_H

#include "bitmap.h"

#define BUDDY_MAX_ORDER	30

struct buddy {
	/* max_order is both the height of the tree - 1 and the ^2 of the
	 * size of the lowest level.
	 *
	 * So if we have 512k elements, max_order is 19, which gives us
	 * a 20 levels tree.
	 *
	 * The max supported order is 30 for now. We can increase that
	 * later if really needed but the performance is going to be
	 * already pretty bad if we go near that limit.
	 */
	unsigned int max_order;

	/* For each order, we keep track of how many free modes we
	 * have there to speed up searches.
	 */
	unsigned int freecounts[BUDDY_MAX_ORDER + 1];
	bitmap_elem_t     map[];
};

extern struct buddy *buddy_create(unsigned int max_order);
extern void buddy_destroy(struct buddy *b);

extern int buddy_alloc(struct buddy *b, unsigned int order);
extern bool buddy_reserve(struct buddy *b, unsigned int index, unsigned int order);
extern void buddy_free(struct buddy *b, unsigned int index, unsigned int order);
extern void buddy_reset(struct buddy *b);

#endif /* __BUDDY_H */
