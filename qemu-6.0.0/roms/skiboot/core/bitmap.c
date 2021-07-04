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
 */
#include "bitmap.h"

static int __bitmap_find_bit(bitmap_t map, unsigned int start, unsigned int count,
			     bool value)
{
	unsigned int el, first_bit;
	unsigned int end = start + count;
	bitmap_elem_t e, ev;
	int b;

	ev = value ? -1ul : 0;
	el = BITMAP_ELEM(start);
	first_bit = BITMAP_BIT(start);

	while (start < end) {
		e = map[el] ^ ev;
		e |= ((1ul << first_bit) - 1);
		if (~e)
			break;
		start = (start + BITMAP_ELSZ) & ~(BITMAP_ELSZ - 1);
		first_bit = 0;
		el++;
	}
	for (b = first_bit; b < BITMAP_ELSZ && start < end; b++,start++) {
		if ((e & (1ull << b)) == 0)
			return start;
	}

	return -1;
}

int bitmap_find_zero_bit(bitmap_t map, unsigned int start, unsigned int count)
{
	return __bitmap_find_bit(map, start, count, false);
}

int bitmap_find_one_bit(bitmap_t map, unsigned int start, unsigned int count)
{
	return __bitmap_find_bit(map, start, count, true);
}

