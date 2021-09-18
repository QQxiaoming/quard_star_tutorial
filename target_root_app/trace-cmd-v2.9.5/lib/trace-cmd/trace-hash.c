// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2014, Steven Rostedt <srostedt@redhat.com>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "trace-hash.h"

int trace_hash_init(struct trace_hash *hash, int buckets)
{
	memset(hash, 0, sizeof(*hash));

	hash->buckets = calloc(sizeof(*hash->buckets), buckets);
	if (!hash->buckets)
		return -ENOMEM;
	hash->nr_buckets = buckets;

	/* If a power of two then we can shortcut */
	if (!(buckets & (buckets - 1)))
		hash->power = buckets - 1;

	return 0;
}

void trace_hash_free(struct trace_hash *hash)
{
	free(hash->buckets);
}

int trace_hash_empty(struct trace_hash *hash)
{
	struct trace_hash_item **bucket;

	trace_hash_for_each_bucket(bucket, hash)
		if (*bucket)
			return 0;
	return 1;
}

int trace_hash_add(struct trace_hash *hash, struct trace_hash_item *item)
{
	struct trace_hash_item *next;
	int bucket = hash->power ? item->key & hash->power :
		item->key % hash->nr_buckets;

	if (hash->buckets[bucket]) {
		next = hash->buckets[bucket];
		next->prev = item;
	} else
		next = NULL;

	item->next = next;
	item->prev = (struct trace_hash_item *)&hash->buckets[bucket];

	hash->buckets[bucket] = item;

	return 1;
}

struct trace_hash_item *
trace_hash_find(struct trace_hash *hash, unsigned long long key,
		trace_hash_func match, void *data)
{
	struct trace_hash_item *item;
	int bucket = hash->power ? key & hash->power :
		key % hash->nr_buckets;

	for (item = hash->buckets[bucket]; item; item = item->next) {
		if (item->key == key) {
			if (!match)
				return item;
			if (match(item, data))
				return item;
		}
	}

	return NULL;
}
