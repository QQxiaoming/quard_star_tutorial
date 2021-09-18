/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2014 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 */
#ifndef _TRACE_HASH_H
#define _TRACE_HASH_H

struct trace_hash_item {
	struct trace_hash_item	*next;
	struct trace_hash_item	*prev;
	unsigned long long	key;
};

struct trace_hash {
	struct trace_hash_item	**buckets;
	int			nr_buckets;
	int			power;
};

int trace_hash_init(struct trace_hash *hash, int buckets);
void trace_hash_free(struct trace_hash *hash);
int trace_hash_add(struct trace_hash *hash, struct trace_hash_item *item);
int trace_hash_empty(struct trace_hash *hash);

static inline void trace_hash_del(struct trace_hash_item *item)
{
	struct trace_hash_item *prev = item->prev;

	prev->next = item->next;
	if (item->next)
		item->next->prev = prev;
}

#define trace_hash_for_each_bucket(bucket, hash)			\
	for (bucket = (hash)->buckets;					\
	     (bucket) < (hash)->buckets + (hash)->nr_buckets; (bucket)++)

#define trace_hash_for_each_item(item, bucket)				\
	for ((item = *(bucket)); item; item = (item)->next)

#define trace_hash_for_each_item_safe(item, n, bucket)		\
	for ((item = *(bucket)), n = item ? item->next : NULL; item; \
	     item = n, n = item ? (item)->next : NULL)

#define trace_hash_while_item(item, bucket)	\
	while ((item = *(bucket)))

typedef int (*trace_hash_func)(struct trace_hash_item *item, void *data);

struct trace_hash_item *
trace_hash_find(struct trace_hash *hash, unsigned long long key,
		trace_hash_func match, void *data);

#endif /* _TRACE_HASH_H */
