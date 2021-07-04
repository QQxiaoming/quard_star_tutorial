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

#ifndef __TRACE_H
#define __TRACE_H
#include <ccan/short_types/short_types.h>
#include <stddef.h>
#include <lock.h>
#include <trace_types.h>


struct cpu_thread;

/* Here's one we prepared earlier. */
void init_boot_tracebuf(struct cpu_thread *boot_cpu);

struct trace_info {
	/* Lock for writers. Exposed to kernel. */
	struct lock lock;
	/* Exposed to kernel. */
	struct tracebuf tb;
};

#define TBUF_SZ ((1024 * 1024) - sizeof(struct trace_info) - sizeof(union trace))

/* Allocate trace buffers once we know memory topology */
void init_trace_buffers(void);

/* This will fill in timestamp and cpu; you must do type and len. */
void trace_add(union trace *trace, u8 type, u16 len);

/* Put trace node into dt. */
void trace_add_node(void);
#endif /* __TRACE_H */
