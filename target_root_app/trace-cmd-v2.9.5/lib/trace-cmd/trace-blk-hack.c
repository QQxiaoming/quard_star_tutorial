// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2009 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 */
#include <stdio.h>
#include "trace-cmd.h"
#include "trace-local.h"

static const char blk_event_start[] =
	"name: blktrace\n"
	"ID: %d\n"
	"format:\n"
	"\tfield:unsigned short common_type;\toffset:0;\tsize:2;\n"
	"\tfield:unsigned char common_flags;\toffset:2;\tsize:1;\n"
	"\tfield:unsigned char common_preempt_count;\toffset:3;\tsize:1;\n"
	"\tfield:int common_pid;\toffset:4;\tsize:4;\n";

static const char blk_body[] = "\n"
	"\tfield:u64 sector;\toffset:16;\tsize:8;\n"
	"\tfield:int bytes;\toffset:24;\tsize:4;\n"
	"\tfield:int action;\toffset:28;\tsize:4;\n"
	"\tfield:int pid;\toffset:32;\tsize:4;\n"
	"\tfield:int device;\toffset:36;\tsize:4;\n"
	"\tfield:int cpu;\toffset:40;\tsize:4;\n"
	"\tfield:short error;\toffset:44;\tsize:2;\n"
	"\tfield:short pdu_len;\toffset:46;\tsize:2;\n"
	"\tfield:void data;\toffset:48;\tsize:0;\n"
	"\n"
	"print fmt: \"%%d\", REC->pid\n";

int tracecmd_blk_hack(struct tracecmd_input *handle)
{
	struct tep_handle *pevent;
	struct tep_event *event;
	struct tep_format_field *field;
	char buf[4096]; /* way more than enough! */
	int id;
	int l;
	int r;

	pevent = tracecmd_get_tep(handle);

	/*
	 * Unfortunately, the TRACE_BLK has changed a bit.
	 * We need to test if various events exist to try
	 * to guess what event id TRACE_BLK would be.
	 */

	/* It was originally behind the "power" event */
	event = tep_find_event_by_name(pevent, "ftrace", "power");
	if (event) {
		id = event->id + 1;
		goto found;
	}

	/*
	 * But the power tracer is now in perf.
	 * Then it was after kmem_free
	 */
	event = tep_find_event_by_name(pevent, "ftrace", "kmem_free");
	if (event) {
		id = event->id + 1;
		goto found;
	}

	/*
	 * But that then went away.
	 * Currently it should be behind the user stack.
	 */
	event = tep_find_event_by_name(pevent, "ftrace", "user_stack");
	if (event) {
		id = event->id + 1;
		goto found;
	}
	/* Give up :( */
	return -1;

 found:
	/*
	 * Blk events are not exported in the events directory.
	 * This is a hack to attempt to create a block event
	 * that we can read.
	 *
	 * We'll make a format file to look like this:
	 *
	 * name: blktrace
	 * ID: 13
	 * format:
	 *	field:unsigned short common_type;	offset:0;	size:2;
	 *	field:unsigned char common_flags;	offset:2;	size:1;
	 *	field:unsigned char common_preempt_count;	offset:3;	size:1;
	 *	field:int common_pid;	offset:4;	size:4;
	 *	field:int common_lock_depth;	offset:8;	size:4;
	 *
	 *	field:u64 sector;	offset:16;	size:8;
	 *	field:int bytes;	offset:32;	size:4;
	 *	field:int action;	offset:36;	size:4;
	 *	field:int pid;	offset:40;	size:4;
	 *	field:int device;	offset:44;	size:4;
	 *	field:int cpu;	offset:48;	size:4;
	 *	field:short error;	offset:52;	size:2;
	 *	field:short pdu_len;	offset:54;	size:2;
	 *	field:void data;	offset:60;	size:0;
	 *
	 * print fmt: "%d", REC->pid
	 *
	 * Note: the struct blk_io_trace is used directly and
	 * just the first parts of the struct are not used in order
	 * to not write over the ftrace data.
	 */

	/* Make sure the common fields exist */
	field = tep_find_common_field(event, "common_type");
	if (!field || field->offset != 0 || field->size != 2)
		goto fail;
	field = tep_find_common_field(event, "common_flags");
	if (!field || field->offset != 2 || field->size != 1)
		goto fail;
	field = tep_find_common_field(event, "common_preempt_count");
	if (!field || field->offset != 3 || field->size != 1)
		goto fail;
	field = tep_find_common_field(event, "common_pid");
	if (!field || field->offset != 4 || field->size != 4)
		goto fail;
	r = sprintf(buf, blk_event_start, id);
	l = r;

	/* lock depth is optional */
	field = tep_find_common_field(event, "common_lock_depth");
	if (field) {
		if (field->offset != 8 || field->size != 4)
			return -1;
		r = sprintf(buf+l, "\tfield:int common_lock_depth;\toffset:8;\tsize:4;\n");
		l += r;
	}

	r = sprintf(buf+l, blk_body);

	/* Parse this event */
	l += r;
	tep_parse_event(pevent, buf, l, "ftrace");

	return 0;

 fail:
	return -1;
}
