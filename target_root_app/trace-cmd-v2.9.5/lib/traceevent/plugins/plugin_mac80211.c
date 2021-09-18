// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2009 Johannes Berg <johannes@sipsolutions.net>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "event-parse.h"
#include "trace-seq.h"

#define INDENT 65

static void print_string(struct trace_seq *s, struct tep_event *event,
			 const char *name, const void *data)
{
	struct tep_format_field *f = tep_find_field(event, name);
	int offset;
	int length;

	if (!f) {
		trace_seq_printf(s, "NOTFOUND:%s", name);
		return;
	}

	offset = f->offset;
	length = f->size;

	if (!strncmp(f->type, "__data_loc", 10)) {
		unsigned long long v;
		if (tep_read_number_field(f, data, &v)) {
			trace_seq_printf(s, "invalid_data_loc");
			return;
		}
		offset = v & 0xffff;
		length = v >> 16;
	}

	trace_seq_printf(s, "%.*s", length, (char *)data + offset);
}

struct value_name {
	unsigned long long value;
	const char *name;
};

static void _print_enum(struct trace_seq *s, struct tep_event *event,
			const char *name, const void *data,
			const struct value_name *names, int n_names)
{
	struct tep_format_field *f = tep_find_field(event, name);
	unsigned long long val;
	int i;

	if (!f) {
		trace_seq_puts(s, "field-not-found");
		return;
	}

	if (tep_read_number_field(f, data, &val)) {
		trace_seq_puts(s, "field-invalid");
		return;
	}

	for (i = 0; i < n_names; i++) {
		if (names[i].value == val) {
			trace_seq_puts(s, names[i].name);
			return;
		}
	}

	trace_seq_printf(s, "%llu", val);
}

#define print_enum(s, ev, name, data, enums...)					\
	({ static const struct value_name __n[] = { enums };			\
	_print_enum(s, ev, name, data, __n, sizeof(__n)/sizeof(__n[0]));	\
	})

static void _print_flag(struct trace_seq *s, struct tep_event *event,
			const char *name, const void *data,
			const struct value_name *names, int n_names)
{
	struct tep_format_field *f = tep_find_field(event, name);
	unsigned long long val;
	int i, j, found, first = 1;

	if (!f) {
		trace_seq_puts(s, "field-not-found");
		return;
	}

	if (tep_read_number_field(f, data, &val)) {
		trace_seq_puts(s, "field-invalid");
		return;
	}

	for (i = 0; i < 64; i++) {
		if (!(val & (1ULL<<i)))
			continue;
		if (!first)
			trace_seq_putc(s, '|');
		first = 0;

		found = 0;
		for (j = 0; j < n_names; j++) {
			if (i == names[j].value) {
				trace_seq_puts(s, names[j].name);
				found = 1;
				break;
			}
		}
		if (!found)
			trace_seq_printf(s, "flag_%d", i);
	}
}

#define print_flag(s, ev, name, data, enums...)					\
	({ static const struct value_name __n[] = { enums };			\
	_print_flag(s, ev, name, data, __n, sizeof(__n)/sizeof(__n[0]));	\
	})

#define SF(fn)	tep_print_num_field(s, fn ":%d", event, fn, record, 0)
#define SFX(fn)	tep_print_num_field(s, fn ":%#x", event, fn, record, 0)
#define SP()	trace_seq_putc(s, ' ')

static int drv_bss_info_changed(struct trace_seq *s, struct tep_record *record,
				struct tep_event *event, void *context)
{
	void *data = record->data;

	print_string(s, event, "wiphy_name", data);
	trace_seq_printf(s, " vif:");
	print_string(s, event, "vif_name", data);
	tep_print_num_field(s, "(%d)", event, "vif_type", record, 1);

	trace_seq_printf(s, "\n%*s", INDENT, "");
	SF("assoc"); SP();
	SF("aid"); SP();
	SF("cts"); SP();
	SF("shortpre"); SP();
	SF("shortslot"); SP();
	SF("dtimper"); SP();
	trace_seq_printf(s, "\n%*s", INDENT, "");
	SF("bcnint"); SP();
	SFX("assoc_cap"); SP();
	SFX("basic_rates"); SP();
	SF("enable_beacon");
	trace_seq_printf(s, "\n%*s", INDENT, "");
	SF("ht_operation_mode");

	return 0;
}

static int drv_config(struct trace_seq *s, struct tep_record *record,
		      struct tep_event *event, void *context)
{
	void *data = record->data;

	print_string(s, event, "wiphy_name", data);
	trace_seq_putc(s, ' ');
	print_flag(s, event, "flags", data,
		{ 0, "MONITOR" },
		{ 1, "PS" },
		{ 2, "IDLE" },
		{ 3, "QOS"},
	);
	tep_print_num_field(s, " chan:%d/", event, "center_freq", record, 1);
	print_enum(s, event, "channel_type", data,
		{ 0, "noht" },
		{ 1, "ht20" },
		{ 2, "ht40-" },
		{ 3, "ht40+" });
	trace_seq_putc(s, ' ');
	SF("power_level");

	return 0;
}

int TEP_PLUGIN_LOADER(struct tep_handle *tep)
{
	tep_register_event_handler(tep, -1, "mac80211", "drv_bss_info_changed",
				   drv_bss_info_changed, NULL);
	tep_register_event_handler(tep, -1, "mac80211", "drv_config",
				   drv_config, NULL);

	return 0;
}
