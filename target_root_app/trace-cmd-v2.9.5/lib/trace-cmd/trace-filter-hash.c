// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2009, Steven Rostedt <srostedt@redhat.com>
 * Copyright (C) 2018 VMware Inc, Steven Rostedt <rostedt@goodmis.org>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "trace-filter-hash.h"

#define FILTER_HASH_BITS	8
#define FILTER_HASH_SIZE	(1 << FILTER_HASH_BITS)

struct tracecmd_filter_id_item *
tracecmd_filter_id_find(struct tracecmd_filter_id *hash, int id)
{
	int key = tracecmd_quick_hash(id, FILTER_HASH_BITS);
	struct tracecmd_filter_id_item *item = hash->hash[key];

	while (item) {
		if (item->id == id)
			break;
		item = item->next;
	}

	return item;
}

void tracecmd_filter_id_add(struct tracecmd_filter_id *hash, int id)
{
	int key = tracecmd_quick_hash(id, FILTER_HASH_BITS);
	struct tracecmd_filter_id_item *item;

	item = calloc(1, sizeof(*item));
	assert(item);

	item->id = id;
	item->next = hash->hash[key];
	hash->hash[key] = item;

	hash->count++;
}

void tracecmd_filter_id_remove(struct tracecmd_filter_id *hash, int id)
{
	int key = tracecmd_quick_hash(id, FILTER_HASH_BITS);
	struct tracecmd_filter_id_item **next = &hash->hash[key];
	struct tracecmd_filter_id_item *item;

	while (*next) {
		if ((*next)->id == id)
			break;
		next = &(*next)->next;
	}

	if (!*next)
		return;

	assert(hash->count);
	hash->count--;

	item = *next;

	*next = item->next;

	free(item);
}

void tracecmd_filter_id_clear(struct tracecmd_filter_id *hash)
{
	struct tracecmd_filter_id_item *item, *next;
	int i;

	for (i = 0; i < FILTER_HASH_SIZE; i++) {
		next = hash->hash[i];
		if (!next)
			continue;

		hash->hash[i] = NULL;
		while (next) {
			item = next;
			next = item->next;
			free(item);
		}
	}

	hash->count = 0;
}

struct tracecmd_filter_id *tracecmd_filter_id_hash_alloc(void)
{
	struct tracecmd_filter_id *hash;

	hash = calloc(1, sizeof(*hash));
	assert(hash);
	hash->hash = calloc(FILTER_HASH_SIZE, sizeof(*hash->hash));
	hash->count = 0;

	return hash;
}

void tracecmd_filter_id_hash_free(struct tracecmd_filter_id *hash)
{
	if (!hash)
		return;

	tracecmd_filter_id_clear(hash);
	free(hash->hash);
	free(hash);
}

struct tracecmd_filter_id *
tracecmd_filter_id_hash_copy(struct tracecmd_filter_id *hash)
{
	struct tracecmd_filter_id *new_hash;
	struct tracecmd_filter_id_item *item, **pitem;
	int i;

	if (!hash)
		return NULL;

	new_hash = tracecmd_filter_id_hash_alloc();
	assert(new_hash);

	for (i = 0; i < FILTER_HASH_SIZE; i++) {
		item = hash->hash[i];
		if (!item)
			continue;

		pitem = &new_hash->hash[i];

		while (item) {
			*pitem = calloc(1, sizeof(*item));
			assert(*pitem);
			**pitem = *item;

			pitem = &(*pitem)->next;
			item = item->next;
		}
	}

	new_hash->count = hash->count;
	return new_hash;
}

int *tracecmd_filter_ids(struct tracecmd_filter_id *hash)
{
	struct tracecmd_filter_id_item *item;
	int *ids;
	int count = 0;
	int i;

	if (!hash->count)
		return NULL;

	ids = malloc(sizeof(*ids) * (hash->count + 1));
	if (!ids)
		return NULL;

	for (i = 0; i < FILTER_HASH_SIZE; i++) {
		item = hash->hash[i];
		while (item) {
			ids[count++] = item->id;
			item = item->next;
		}
	}

	ids[count] = -1;
	return ids;
}

/**
 * filter_id_compare - compare two id hashes to see if they are equal
 * @hash1: one hash to compare
 * @hash2: another hash to compare to @hash1
 *
 * Returns 1 if the two hashes are the same, 0 otherwise.
 */
int tracecmd_filter_id_compare(struct tracecmd_filter_id *hash1,
			       struct tracecmd_filter_id *hash2)
{
	int *ids;
	int ret = 0;
	int i;

	/* If counts don't match, then they obviously are not the same */
	if (hash1->count != hash2->count)
		return 0;

	/* If both hashes are empty, they are the same */
	if (!hash1->count && !hash2->count)
		return 1;

	/* Now compare the pids of one hash with the other */
	ids = tracecmd_filter_ids(hash1);
	for (i = 0; ids[i] >= 0; i++) {
		if (!tracecmd_filter_id_find(hash2, ids[i]))
			break;
	}

	if (ids[i] == -1)
		ret = 1;

	free(ids);

	return ret;
}
