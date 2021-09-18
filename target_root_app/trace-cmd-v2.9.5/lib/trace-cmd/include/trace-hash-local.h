/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2009, Steven Rostedt <srostedt@redhat.com>
 *
 */
#ifndef _TRACE_HASH_LOCAL_H
#define _TRACE_HASH_LOCAL_H

static inline unsigned int trace_hash(unsigned int val)
{
	unsigned int hash, tmp;

	hash = 12546869;	/* random prime */

	/*
	 * The following hash is based off of Paul Hsieh's super fast hash:
	 *  http://www.azillionmonkeys.com/qed/hash.html
	 * Note, he released this code unde the GPL 2.0 license, which
	 *  is the same as the license for the programs that use it here.
	 */

	hash +=	(val & 0xffff);
	tmp = (val >> 16) ^ hash;
	hash = (hash << 16) ^ tmp;
	hash += hash >> 11;

	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}

static inline unsigned int trace_hash_str(char *str)
{
	int val = 0;
	int i;

	for (i = 0; str[i]; i++)
		val += ((int)str[i]) << (i & 0xf);
	return trace_hash(val);
}
#endif /* _TRACE_HASH_LOCAL_H */
