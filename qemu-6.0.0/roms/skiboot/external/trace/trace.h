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
#ifndef E_TRACE_H
#define E_TRACE_H

#include <stdbool.h>
#include <types.h>
#include <trace.h>
#include <trace_types.h>

struct trace_reader {
	/* This is where the reader is up to. */
	u64 rpos;
	/* If the last one we read was a repeat, this shows how many. */
	u32 last_repeat;
	struct list_head traces;
	struct tracebuf *tb;
};

/* Is this tracebuf empty? */
bool trace_empty(const struct trace_reader *tr);

/* Get the next trace from this buffer (false if empty). */
bool trace_get(union trace *t, struct trace_reader *tr);

#endif
