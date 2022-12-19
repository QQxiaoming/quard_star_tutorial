// SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
/*
 * Trace various things into in-memory buffers
 *
 * Copyright 2013-2019 IBM Corp.
 */

#include <trace.h>
#include <timebase.h>
#include <lock.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cpu.h>
#include <device.h>
#include <libfdt.h>
#include <processor.h>
#include <skiboot.h>
#include <opal-api.h>
#include <debug_descriptor.h>
#include <nvram.h>

#define DEBUG_TRACES

#define MAX_SIZE sizeof(union trace)

/* Smaller trace buffer for early booting */
#define BOOT_TBUF_SZ 65536
static struct {
	struct trace_info trace_info;
	char buf[BOOT_TBUF_SZ + MAX_SIZE];
} boot_tracebuf __section(".data.boot_trace");

void init_boot_tracebuf(struct cpu_thread *boot_cpu)
{
	init_lock(&boot_tracebuf.trace_info.lock);
	boot_tracebuf.trace_info.tb.buf_size = cpu_to_be64(BOOT_TBUF_SZ);
	boot_tracebuf.trace_info.tb.max_size = cpu_to_be32(MAX_SIZE);

	boot_cpu->trace = &boot_tracebuf.trace_info;
}

static size_t tracebuf_extra(void)
{
	/* We make room for the largest possible record */
	return TBUF_SZ + MAX_SIZE;
}

/* To avoid bloating each entry, repeats are actually specific entries.
 * tb->last points to the last (non-repeat) entry. */
static bool handle_repeat(struct tracebuf *tb, const union trace *trace)
{
	struct trace_hdr *prev;
	struct trace_repeat *rpt;
	u32 len;

	prev = (void *)tb->buf + be64_to_cpu(tb->last) % be64_to_cpu(tb->buf_size);

	if (prev->type != trace->hdr.type
	    || prev->len_div_8 != trace->hdr.len_div_8
	    || prev->cpu != trace->hdr.cpu)
		return false;

	len = prev->len_div_8 << 3;
	if (memcmp(prev + 1, &trace->hdr + 1, len - sizeof(*prev)) != 0)
		return false;

	/* If they've consumed prev entry, don't repeat. */
	if (be64_to_cpu(tb->last) < be64_to_cpu(tb->start))
		return false;

	/* OK, it's a duplicate.  Do we already have repeat? */
	if (be64_to_cpu(tb->last) + len != be64_to_cpu(tb->end)) {
		u64 pos = be64_to_cpu(tb->last) + len;
		/* FIXME: Reader is not protected from seeing this! */
		rpt = (void *)tb->buf + pos % be64_to_cpu(tb->buf_size);
		assert(pos + rpt->len_div_8*8 == be64_to_cpu(tb->end));
		assert(rpt->type == TRACE_REPEAT);

		/* If this repeat entry is full, don't repeat. */
		if (be16_to_cpu(rpt->num) == 0xFFFF)
			return false;

		rpt->num = cpu_to_be16(be16_to_cpu(rpt->num) + 1);
		rpt->timestamp = trace->hdr.timestamp;
		return true;
	}

	/*
	 * Generate repeat entry: it's the smallest possible entry, so we
	 * must have eliminated old entries.
	 */
	assert(trace->hdr.len_div_8 * 8 >= sizeof(*rpt));

	rpt = (void *)tb->buf + be64_to_cpu(tb->end) % be64_to_cpu(tb->buf_size);
	rpt->timestamp = trace->hdr.timestamp;
	rpt->type = TRACE_REPEAT;
	rpt->len_div_8 = sizeof(*rpt) >> 3;
	rpt->cpu = trace->hdr.cpu;
	rpt->prev_len = cpu_to_be16(trace->hdr.len_div_8 << 3);
	rpt->num = cpu_to_be16(1);
	lwsync(); /* write barrier: complete repeat record before exposing */
	tb->end = cpu_to_be64(be64_to_cpu(tb->end) + sizeof(*rpt));
	return true;
}

void trace_add(union trace *trace, u8 type, u16 len)
{
	struct trace_info *ti = this_cpu()->trace;
	unsigned int tsz;

	trace->hdr.type = type;
	trace->hdr.len_div_8 = (len + 7) >> 3;

	tsz = trace->hdr.len_div_8 << 3;

#ifdef DEBUG_TRACES
	assert(tsz >= sizeof(trace->hdr));
	assert(tsz <= sizeof(*trace));
	assert(trace->hdr.type != TRACE_REPEAT);
	assert(trace->hdr.type != TRACE_OVERFLOW);
#endif
	/* Skip traces not enabled in the debug descriptor */
	if (trace->hdr.type < (8 * sizeof(debug_descriptor.trace_mask)) &&
	    !((1ul << trace->hdr.type) & be64_to_cpu(debug_descriptor.trace_mask)))
		return;

	trace->hdr.timestamp = cpu_to_be64(mftb());
	trace->hdr.cpu = cpu_to_be16(this_cpu()->server_no);

	lock(&ti->lock);

	/* Throw away old entries before we overwrite them. */
	while ((be64_to_cpu(ti->tb.start) + be64_to_cpu(ti->tb.buf_size))
	       < (be64_to_cpu(ti->tb.end) + tsz)) {
		struct trace_hdr *hdr;

		hdr = (void *)ti->tb.buf +
			be64_to_cpu(ti->tb.start) % be64_to_cpu(ti->tb.buf_size);
		ti->tb.start = cpu_to_be64(be64_to_cpu(ti->tb.start) +
					   (hdr->len_div_8 << 3));
	}

	/* Must update ->start before we rewrite new entries. */
	lwsync(); /* write barrier */

	/* Check for duplicates... */
	if (!handle_repeat(&ti->tb, trace)) {
		/* This may go off end, and that's why ti->tb.buf is oversize */
		memcpy(ti->tb.buf + be64_to_cpu(ti->tb.end) % be64_to_cpu(ti->tb.buf_size),
		       trace, tsz);
		ti->tb.last = ti->tb.end;
		lwsync(); /* write barrier: write entry before exposing */
		ti->tb.end = cpu_to_be64(be64_to_cpu(ti->tb.end) + tsz);
	}
	unlock(&ti->lock);
}

void trace_add_dt_props(void)
{
	uint64_t boot_buf_phys = (uint64_t) &boot_tracebuf.trace_info;
	struct dt_node *exports, *traces;
	unsigned int i;
	fdt64_t *prop;
	u64 tmask;
	char tname[256];

	exports = dt_find_by_path(opal_node, "firmware/exports");
	if (!exports)
		return;

	/*
	 * nvram hack to put all the trace buffer exports in the exports
	 * node. This is useful if the kernel doesn't also export subnodes.
	 */
	if (nvram_query_safe("flat-trace-buf"))
		traces = exports;
	else
		traces = dt_new(exports, "traces");

	prop = malloc(sizeof(u64) * 2 * be32_to_cpu(debug_descriptor.num_traces));

	for (i = 0; i < be32_to_cpu(debug_descriptor.num_traces); i++) {
		uint64_t addr = be64_to_cpu(debug_descriptor.trace_phys[i]);
		uint64_t size = be32_to_cpu(debug_descriptor.trace_size[i]);
		uint32_t pir = be16_to_cpu(debug_descriptor.trace_pir[i]);

		prop[i * 2]     = cpu_to_fdt64(addr);
		prop[i * 2 + 1] = cpu_to_fdt64(size);

		if (addr == boot_buf_phys)
			snprintf(tname, sizeof(tname), "boot-%x", pir);
		else
			snprintf(tname, sizeof(tname), "trace-%x", pir);

		dt_add_property_u64s(traces, tname, addr, size);
	}

	dt_add_property(opal_node, "ibm,opal-traces",
			prop, sizeof(u64) * 2 * i);
	free(prop);

	tmask = (uint64_t)&debug_descriptor.trace_mask;
	dt_add_property_u64(opal_node, "ibm,opal-trace-mask", tmask);
}

static void trace_add_desc(struct trace_info *t, uint64_t size, uint16_t pir)
{
	unsigned int i = be32_to_cpu(debug_descriptor.num_traces);

	if (i >= DEBUG_DESC_MAX_TRACES) {
		prerror("TRACE: Debug descriptor trace list full !\n");
		return;
	}

	debug_descriptor.num_traces = cpu_to_be32(i + 1);
	debug_descriptor.trace_phys[i] = cpu_to_be64((uint64_t)t);
	debug_descriptor.trace_tce[i] = 0; /* populated later */
	debug_descriptor.trace_size[i] = cpu_to_be32(size);
	debug_descriptor.trace_pir[i] = cpu_to_be16(pir);
}

/* Allocate trace buffers once we know memory topology */
void init_trace_buffers(void)
{
	struct cpu_thread *t;
	struct trace_info *any = &boot_tracebuf.trace_info;
	uint64_t size;

	/* Boot the boot trace in the debug descriptor */
	trace_add_desc(any, sizeof(boot_tracebuf), this_cpu()->pir);

	/* Allocate a trace buffer for each primary cpu. */
	for_each_cpu(t) {
		if (t->is_secondary)
			continue;

		/* Use a 64K alignment for TCE mapping */
		size = ALIGN_UP(sizeof(*t->trace) + tracebuf_extra(), 0x10000);
		t->trace = local_alloc(t->chip_id, size, 0x10000);
		if (t->trace) {
			any = t->trace;
			memset(t->trace, 0, size);
			init_lock(&t->trace->lock);
			t->trace->tb.max_size = cpu_to_be32(MAX_SIZE);
			t->trace->tb.buf_size = cpu_to_be64(TBUF_SZ);
			trace_add_desc(any, sizeof(t->trace->tb) +
				       tracebuf_extra(), t->pir);
		} else
			prerror("TRACE: cpu 0x%x allocation failed\n", t->pir);
	}

	/* In case any allocations failed, share trace buffers. */
	for_each_cpu(t) {
		if (!t->is_secondary && !t->trace)
			t->trace = any;
	}

	/* And copy those to the secondaries. */
	for_each_cpu(t) {
		if (!t->is_secondary)
			continue;
		t->trace = t->primary->trace;
	}
}
