/* Copyright 2013-2014 IBM Corp.
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

#include "../../core/test/stubs.c"
#include "../phys-map.c"

enum proc_gen proc_gen;

static inline void print_entry(const struct phys_map_entry *e)
{
	printf("type:%i index:%i addr:%016lx size:%016lx",
	       e->type, e->index, e->addr, e->size);
}

/* Check table directly for overlaps */
static void check_table_directly(void)
{
	const struct phys_map_entry *e, *prev;
	uint64_t start, end, pstart, pend;
	bool passed;

	/* Loop over table entries ...  */
	for (e = phys_map->table; !phys_map_entry_null(e); e++) {

		start = e->addr;
		end = e->addr + e->size;
		/* ... see if they overlap with previous entries */
		for (prev = phys_map->table; prev != e; prev++) {
			passed = true;
			/* Check for overlaping regions */
			pstart = prev->addr;
			pend = prev->addr + prev->size;
			if ((start > pstart) && (start < pend))
				passed = false;
			if ((end > pstart) && (end < pend))
				passed = false;

			/* Check for duplicate entries */
			if ((e->type == prev->type) &&
			    (e->index == prev->index))
				passed = false;

			if (passed)
				continue;

			printf("Phys map direct test FAILED: Entry overlaps\n");
			printf("First:  ");
			print_entry(prev);
			printf("\n");
			printf("Second: ");
			print_entry(e);
			printf("\n");
			assert(0);
		}
	}
}

struct map_call_entry {
	uint64_t start;
	uint64_t end;
};

static inline bool map_call_entry_null(const struct map_call_entry *t)
{
	if ((t->start == 0) &&
	    (t->end == 0))
		return true;
	return false;
}

/* Check calls to map to see if they overlap.
 * Creates a new table for each of the entries it gets to check against
 */

/* Pick a chip ID, any ID. */
#define FAKE_CHIP_ID 8

static void check_map_call(void)
{
	uint64_t start, size, end;
	const struct phys_map_entry *e;
	struct map_call_entry *tbl, *t, *tnext;
	int tbl_size = 0;
	bool passed;

	for (e = phys_map->table; !phys_map_entry_null(e); e++)
		tbl_size++;

	tbl_size++; /* allow for null entry at end */
	tbl_size *= sizeof(struct map_call_entry);
	tbl = malloc(tbl_size);
	assert(tbl != NULL);
	memset(tbl, 0, tbl_size);

	/* Loop over table entries ...  */
	for (e = phys_map->table; !phys_map_entry_null(e); e++) {
		phys_map_get(FAKE_CHIP_ID, e->type, e->index, &start, &size);

		/* Check for alignment */
		if ((e->type != SYSTEM_MEM) && (e->type != RESV)) {
			/* Size is power of 2? */
			assert(__builtin_popcountl(size) == 1);
			/* Start is aligned to size? */
			assert((start % size) == 0);
		}

		end = start + size;
		for (t = tbl; !map_call_entry_null(t); t++) {
			passed = true;

			/* Check for overlaping regions */
			if ((start > t->start) && (start < t->end))
				passed = false;
			if ((end > t->start) && (end < t->end))
				passed = false;

			if (passed)
				continue;

			printf("Phys map call test FAILED: Entry overlaps\n");
			printf("First:  addr:%016lx size:%016lx\n",
			       t->start, t->end - t->start);
			printf("Second: addr:%016lx size:%016lx\n  ",
			       start, size);
			print_entry(e);
			printf("\n");
			assert(0);
		}
		/* Insert entry at end of table */
		t->start = start;
		t->end = end;
	}

	for (t = tbl; !map_call_entry_null(t + 1); t++) {
		tnext = t + 1;
		/* Make sure the table is sorted */
		if (t->start > tnext->start) {
			printf("Phys map test FAILED: Entry not sorted\n");
			printf("First:  addr:%016lx size:%016lx\n",
			       t->start, t->end - t->start);
			printf("Second:  addr:%016lx size:%016lx\n",
			       tnext->start, tnext->end - tnext->start);
			assert(0);
		}

		/* Look for holes in the table in MMIO region */
		/* We assume over 1PB is MMIO. */
		if ((t->end != tnext->start) &&
		    (t->start > 0x0004000000000000)) {
			printf("Phys map test FAILED: Hole in map\n");
			printf("First:  addr:%016lx size:%016lx\n",
			       t->start, t->end - t->start);
			printf("Second:  addr:%016lx size:%016lx\n",
			       tnext->start, tnext->end - tnext->start);
			assert(0);
		}
	}

	free(tbl);
}

int main(void)
{
	/* Fake we are POWER9 */
	proc_gen = proc_gen_p9;
	phys_map_init();

	/* Run tests */
	check_table_directly();
	check_map_call();

	return(0);
}
