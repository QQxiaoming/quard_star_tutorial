// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2009, 2010 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 */
#define _LARGEFILE64_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "trace-local.h"
#include "trace-hash.h"
#include "trace-hash-local.h"
#include "kbuffer.h"
#include "list.h"

static struct filter_str {
	struct filter_str	*next;
	char			*filter;
	int			neg;
} *filter_strings;
static struct filter_str **filter_next = &filter_strings;

struct filter {
	struct filter		*next;
	struct tep_event_filter	*filter;
};

struct event_str {
	struct event_str	*next;
	const char		*event;
};

struct handle_list {
	struct list_head	list;
	struct tracecmd_input	*handle;
	const char		*file;
	int			cpus;
	int			done;
	struct tep_record	*record;
	struct filter		*event_filters;
	struct filter		*event_filter_out;
	unsigned long long	*last_timestamp;
};
static struct list_head handle_list;

struct input_files {
	struct list_head	list;
	const char		*file;
	long long		tsoffset;
	unsigned long long	ts2secs;
};
static struct list_head input_files;
static struct input_files *last_input_file;

struct pid_list {
	struct pid_list		*next;
	char			*pid;
	int			free;
} *pid_list;

struct pid_list *comm_list;

static unsigned int page_size;
static int input_fd;
static const char *default_input_file = DEFAULT_INPUT_FILE;
static const char *input_file;
static int multi_inputs;
static int max_file_size;

static int instances;

static int *filter_cpus;
static int nr_filter_cpus;
static int test_filters_mode;

static int show_wakeup;
static int wakeup_id;
static int wakeup_new_id;
static int sched_id;
static int stacktrace_id;

static int profile;

static int buffer_breaks = 0;

static int no_irqs;
static int no_softirqs;

static int tsdiff;
static int tscheck;

static int latency_format;
static bool raw_format;
static const char *format_type = TEP_PRINT_INFO;

static struct tep_format_field *wakeup_task;
static struct tep_format_field *wakeup_success;
static struct tep_format_field *wakeup_new_task;
static struct tep_format_field *wakeup_new_success;
static struct tep_format_field *sched_task;
static struct tep_format_field *sched_prio;

static unsigned long long total_wakeup_lat;
static unsigned long wakeup_lat_count;

static unsigned long long total_wakeup_rt_lat;
static unsigned long wakeup_rt_lat_count;

struct wakeup_info {
	struct trace_hash_item	hash;
	unsigned long long	start;
	int			pid;
};

static struct hook_list *hooks;
static struct hook_list *last_hook;

#define WAKEUP_HASH_SIZE 1024
static struct trace_hash wakeup_hash;

static void print_event_name(struct trace_seq *s, struct tep_event *event)
{
	static const char *spaces = "                    "; /* 20 spaces */
	int len;

	trace_seq_printf(s, " %s: ", event->name);

	/* Space out the event names evenly. */
	len = strlen(event->name);
	if (len < 20)
		trace_seq_printf(s, "%.*s", 20 - len, spaces);
}

enum time_fmt {
	TIME_FMT_LAT		= 1,
	TIME_FMT_NORMAL		= 2,
};

static const char *time_format(struct tracecmd_input *handle, enum time_fmt tf)
{
	struct tep_handle *tep = tracecmd_get_tep(handle);

	switch (tf) {
	case TIME_FMT_LAT:
		if (latency_format)
			return "%8.8s-%-5d %3d";
		return "%16s-%-5d [%03d]";
	default:
		if (tracecmd_get_flags(handle) & TRACECMD_FL_IN_USECS) {
			if (tep_test_flag(tep, TEP_NSEC_OUTPUT))
				return " %9.1d:";
			else
				return " %6.1000d:";
		} else
			return "%12d:";
	}
}

static void print_event(struct trace_seq *s, struct tracecmd_input *handle,
			struct tep_record *record)
{
	struct tep_handle *tep = tracecmd_get_tep(handle);
	struct tep_event *event;
	const char *lfmt = time_format(handle, TIME_FMT_LAT);
	const char *tfmt = time_format(handle, TIME_FMT_NORMAL);

	event = tep_find_event_by_record(tep, record);
	tep_print_event(tep, s, record, lfmt, TEP_PRINT_COMM,
			TEP_PRINT_PID, TEP_PRINT_CPU);
	tep_print_event(tep, s, record, tfmt, TEP_PRINT_TIME);
	print_event_name(s, event);
	tep_print_event(tep, s, record, "%s", format_type);
}

/* Debug variables for testing tracecmd_read_at */
#define TEST_READ_AT 0
#if TEST_READ_AT
#define DO_TEST
static off64_t test_read_at_offset;
static int test_read_at_copy = 100;
static int test_read_at_index;
static void show_test(struct tracecmd_input *handle)
{
	struct tep_record *record;
	struct trace_seq s;

	if (!test_read_at_offset) {
		printf("\nNO RECORD COPIED\n");
		return;
	}

	record = tracecmd_read_at(handle, test_read_at_offset, NULL);
	printf("\nHERE'S THE COPY RECORD\n");
	trace_seq_init(&s);
	print_event(&s, handle, record);
	trace_seq_do_printf(&s);
	trace_seq_destroy(&s);
	printf("\n");

	tracecmd_free_record(record);
}

static void test_save(struct tep_record *record, int cpu)
{
	if (test_read_at_index++ == test_read_at_copy) {
		test_read_at_offset = record->offset;
		printf("\nUSING THIS RECORD\n");
	}
}
#endif /* TEST_READ_AT */

/* Debug variables for testing tracecmd_set_cpu_at_timestamp */
#define TEST_AT_TIMESTAMP 0
#if TEST_AT_TIMESTAMP
#define DO_TEST
static unsigned long long test_at_timestamp_ts;
static int test_at_timestamp_copy = 100;
static int test_at_timestamp_cpu = -1;
static int test_at_timestamp_index;
static void show_test(struct tracecmd_input *handle)
{
	struct tep_record *record;
	struct trace_seq s;
	int cpu = test_at_timestamp_cpu;

	if (!test_at_timestamp_ts) {
		printf("\nNO RECORD COPIED\n");
		return;
	}

	if (tracecmd_set_cpu_to_timestamp(handle, cpu, test_at_timestamp_ts))
		return;

	record = tracecmd_read_data(handle, cpu);
	printf("\nHERE'S THE COPY RECORD with page %p offset=%p\n",
	       (void *)(record->offset & ~(page_size - 1)),
	       (void *)record->offset);
	trace_seq_init(&s);
	print_event(&s, handle, record);
	trace_seq_do_printf(&s);
	trace_seq_destroy(&s);
	printf("\n");

	tracecmd_free_record(record);
}

static void test_save(struct tep_record *record, int cpu)
{
	if (test_at_timestamp_index++ == test_at_timestamp_copy) {
		test_at_timestamp_ts = record->ts;
		test_at_timestamp_cpu = cpu;
		printf("\nUSING THIS RECORD page=%p offset=%p\n",
		       (void *)(record->offset & ~(page_size - 1)),
		       (void *)record->offset);
	}
}
#endif /* TEST_AT_TIMESTAMP */

#define TEST_FIRST_LAST 0
#if TEST_FIRST_LAST
#define DO_TEST
static void show_test(struct tracecmd_input *handle)
{
	struct tep_record *record;
	struct trace_seq s;
	int cpu = 0;

	record = tracecmd_read_cpu_first(handle, cpu);
	if (!record) {
		printf("No first record?\n");
		return;
	}

	printf("\nHERE'S THE FIRST RECORD with offset %p\n",
	       (void *)record->offset);
	trace_seq_init(&s);
	print_event(&s, handle, record);
	trace_seq_do_printf(&s);
	trace_seq_destroy(&s);
	printf("\n");

	tracecmd_free_record(record);

	record = tracecmd_read_cpu_last(handle, cpu);
	if (!record) {
		printf("No last record?\n");
		return;
	}

	printf("\nHERE'S THE LAST RECORD with offset %p\n",
	       (void *)record->offset);
	trace_seq_init(&s);
	print_event(&s, handle, record);
	trace_seq_do_printf(&s);
	trace_seq_destroy(&s);
	printf("\n");

	tracecmd_free_record(record);
}
static void test_save(struct tep_record *record, int cpu)
{
}
#endif /* TEST_FIRST_LAST */

#ifndef DO_TEST
static void show_test(struct tracecmd_input *handle)
{
	/* quiet the compiler */
	if (0)
		print_event(NULL, NULL, NULL);
}
static void test_save(struct tep_record *record, int cpu)
{
}
#endif

static void add_input(const char *file)
{
	struct input_files *item;

	item = malloc(sizeof(*item));
	if (!item)
		die("Failed to allocate for %s", file);
	memset(item, 0, sizeof(*item));
	item->file = file;
	list_add_tail(&item->list, &input_files);
	last_input_file = item;
}

static void add_handle(struct tracecmd_input *handle, const char *file)
{
	struct handle_list *item;

	item = malloc(sizeof(*item));
	if (!item)
		die("Failed ot allocate for %s", file);
	memset(item, 0, sizeof(*item));
	item->handle = handle;
	if (file) {
		item->file = file + strlen(file);
		/* we want just the base name */
		while (item->file >= file && *item->file != '/')
			item->file--;
		item->file++;
		if (strlen(item->file) > max_file_size)
			max_file_size = strlen(item->file);
	}
	list_add_tail(&item->list, &handle_list);
}

static void free_inputs(void)
{
	struct input_files *item;

	while (!list_empty(&input_files)) {
		item = container_of(input_files.next, struct input_files, list);
		list_del(&item->list);
		free(item);
	}
}

static void free_handles(void)
{
	struct handle_list *item;

	while (!list_empty(&handle_list)) {
		item = container_of(handle_list.next, struct handle_list, list);
		list_del(&item->list);
		free(item);
	}
}

static void add_filter(const char *filter, int neg)
{
	struct filter_str *ftr;

	ftr = malloc(sizeof(*ftr));
	if (!ftr)
		die("Failed to allocate for filter %s", filter);
	ftr->filter = strdup(filter);
	if (!ftr->filter)
		die("malloc");
	ftr->next = NULL;
	ftr->neg = neg;

	/* must maintain order of command line */
	*filter_next = ftr;
	filter_next = &ftr->next;
}

static void __add_filter(struct pid_list **head, const char *arg)
{
	struct pid_list *list;
	char *pids = strdup(arg);
	char *pid;
	char *sav;
	int free = 1;

	if (!pids)
		die("malloc");

	pid = strtok_r(pids, ",", &sav);
	while (pid) {
		list = malloc(sizeof(*list));
		if (!list)
			die("Failed to allocate for arg %s", arg);
		list->pid = pid;
		list->free = free;
		list->next = *head;
		*head = list;
		/* The first pid needs to be freed */
		free = 0;
		pid = strtok_r(NULL, ",", &sav);
	}
}

static void add_comm_filter(const char *arg)
{
	__add_filter(&comm_list, arg);
}

static void add_pid_filter(const char *arg)
{
	__add_filter(&pid_list, arg);
}

static char *append_pid_filter(char *curr_filter, char *pid)
{
	char *filter;
	int len, curr_len;

#define FILTER_FMT "(common_pid==" __STR ")||(pid==" __STR ")||(next_pid==" __STR ")"

#undef __STR
#define __STR ""

	/* strlen(".*:") > strlen("||") */
	len = strlen(".*:" FILTER_FMT) + strlen(pid) * 3 + 1;

#undef __STR
#define __STR "%s"

	if (!curr_filter) {
		filter = malloc(len);
		if (!filter)
			die("Failed to allocate for filter %s", curr_filter);
		sprintf(filter, ".*:" FILTER_FMT, pid, pid, pid);
	} else {
		curr_len = strlen(curr_filter);
		len += curr_len;

		filter = realloc(curr_filter, len);
		if (!filter)
			die("realloc");
		sprintf(filter + curr_len, "||" FILTER_FMT, pid, pid, pid);
	}

	return filter;
}

static void convert_comm_filter(struct tracecmd_input *handle)
{
	struct tep_cmdline *cmdline;
	struct tep_handle *pevent;
	struct pid_list *list;

	char pidstr[100];

	if (!comm_list)
		return;

	pevent = tracecmd_get_tep(handle);

	/* Seach for comm names and get their pids */
	for (list = comm_list; list; list = list->next) {
		cmdline = tep_data_pid_from_comm(pevent, list->pid, NULL);
		if (!cmdline) {
			warning("comm: %s not in cmdline list", list->pid);
			continue;
		}
		do {
			sprintf(pidstr, "%d", tep_cmdline_pid(pevent, cmdline));
			add_pid_filter(pidstr);
			cmdline = tep_data_pid_from_comm(pevent, list->pid,
							    cmdline);
		} while (cmdline);
	}

	while (comm_list) {
		list = comm_list;
		comm_list = comm_list->next;
		if (list->free)
			free(list->pid);
		free(list);
	}
}

static void make_pid_filter(struct tracecmd_input *handle)
{
	struct pid_list *list;
	char *str = NULL;

	convert_comm_filter(handle);

	if (!pid_list)
		return;

	/* First do all common pids */
	for (list = pid_list; list; list = list->next) {
		str = append_pid_filter(str, list->pid);
	}

	add_filter(str, 0);
	free(str);

	while (pid_list) {
		list = pid_list;
		pid_list = pid_list->next;
		if (list->free)
			free(list->pid);
		free(list);
	}
}

static void process_filters(struct handle_list *handles)
{
	struct filter **filter_next = &handles->event_filters;
	struct filter **filter_out_next = &handles->event_filter_out;
	struct filter *event_filter;
	struct filter_str *filter;
	struct tep_handle *pevent;
	char errstr[200];
	int filters = 0;
	int ret;

	pevent = tracecmd_get_tep(handles->handle);

	make_pid_filter(handles->handle);

	while (filter_strings) {
		filter = filter_strings;
		filter_strings = filter->next;

		event_filter = malloc(sizeof(*event_filter));
		if (!event_filter)
			die("Failed to allocate for event filter");
		event_filter->next = NULL;
		event_filter->filter = tep_filter_alloc(pevent);
		if (!event_filter->filter)
			die("malloc");

		ret = tep_filter_add_filter_str(event_filter->filter,
						   filter->filter);
		if (ret < 0) {
			tep_strerror(pevent, ret, errstr, sizeof(errstr));
			die("Error filtering: %s\n%s",
			    filter->filter, errstr);
		}

		if (filter->neg) {
			*filter_out_next = event_filter;
			filter_out_next = &event_filter->next;
		} else {
			*filter_next = event_filter;
			filter_next = &event_filter->next;
		}
		filters++;
		free(filter->filter);
		free(filter);
	}
	if (filters && test_filters_mode)
		exit(0);
}

static void init_wakeup(struct tracecmd_input *handle)
{
	struct tep_handle *pevent;
	struct tep_event *event;

	if (!show_wakeup)
		return;

	pevent = tracecmd_get_tep(handle);

	trace_hash_init(&wakeup_hash, WAKEUP_HASH_SIZE);

	event = tep_find_event_by_name(pevent, "sched", "sched_wakeup");
	if (!event)
		goto fail;
	wakeup_id = event->id;
	wakeup_task = tep_find_field(event, "pid");
	if (!wakeup_task)
		goto fail;
	wakeup_success = tep_find_field(event, "success");

	event = tep_find_event_by_name(pevent, "sched", "sched_switch");
	if (!event)
		goto fail;
	sched_id = event->id;
	sched_task = tep_find_field(event, "next_pid");
	if (!sched_task)
		goto fail;

	sched_prio = tep_find_field(event, "next_prio");
	if (!sched_prio)
		goto fail;


	wakeup_new_id = -1;

	event = tep_find_event_by_name(pevent, "sched", "sched_wakeup_new");
	if (!event)
		goto skip;
	wakeup_new_id = event->id;
	wakeup_new_task = tep_find_field(event, "pid");
	if (!wakeup_new_task)
		goto fail;
	wakeup_new_success = tep_find_field(event, "success");

 skip:
	return;

 fail:
	show_wakeup = 0;
}

static void add_wakeup(unsigned int val, unsigned long long start)
{
	unsigned int key = trace_hash(val);
	struct wakeup_info *info;
	struct trace_hash_item *item;

	item = trace_hash_find(&wakeup_hash, key, NULL, NULL);
	if (item) {
		info = container_of(item, struct wakeup_info, hash);
		/* Hmm, double wakeup? */
		info->start = start;
		return;
	}

	info = malloc(sizeof(*info));
	if (!info)
		die("Failed to allocate wakeup info");
	info->hash.key = key;
	info->start = start;
	trace_hash_add(&wakeup_hash, &info->hash);
}

static unsigned long long max_lat = 0;
static unsigned long long max_time;
static unsigned long long min_lat = -1;
static unsigned long long min_time;

static unsigned long long max_rt_lat = 0;
static unsigned long long max_rt_time;
static unsigned long long min_rt_lat = -1;
static unsigned long long min_rt_time;

static void add_sched(unsigned int val, unsigned long long end, int rt)
{
	struct trace_hash_item *item;
	unsigned int key = trace_hash(val);
	struct wakeup_info *info;
	unsigned long long cal;

	item = trace_hash_find(&wakeup_hash, key, NULL, NULL);
	if (!item)
		return;

	info = container_of(item, struct wakeup_info, hash);

	cal = end - info->start;

	if (cal > max_lat) {
		max_lat = cal;
		max_time = end;
	}
	if (cal < min_lat) {
		min_lat = cal;
		min_time = end;
	}

	if (rt) {
		if (cal > max_rt_lat) {
			max_rt_lat = cal;
			max_rt_time = end;
		}
		if (cal < min_rt_lat) {
			min_rt_lat = cal;
			min_rt_time = end;
		}
	}

	printf(" Latency: %llu.%03llu usecs", cal / 1000, cal % 1000);

	total_wakeup_lat += cal;
	wakeup_lat_count++;

	if (rt) {
		total_wakeup_rt_lat += cal;
		wakeup_rt_lat_count++;
	}

	trace_hash_del(item);
	free(info);
}

static void process_wakeup(struct tep_handle *pevent, struct tep_record *record)
{
	unsigned long long val;
	int id;

	if (!show_wakeup)
		return;

	id = tep_data_type(pevent, record);
	if (id == wakeup_id) {
		if (tep_read_number_field(wakeup_success, record->data, &val) == 0) {
			if (!val)
				return;
		}
		if (tep_read_number_field(wakeup_task, record->data, &val))
			return;
		add_wakeup(val, record->ts);
	} else if (id == wakeup_new_id) {
		if (tep_read_number_field(wakeup_new_success, record->data, &val) == 0) {
			if (!val)
				return;
		}
		if (tep_read_number_field(wakeup_new_task, record->data, &val))
			return;
		add_wakeup(val, record->ts);
	} else if (id == sched_id) {
		int rt = 1;
		if (tep_read_number_field(sched_prio, record->data, &val))
			return;
		if (val > 99)
			rt = 0;
		if (tep_read_number_field(sched_task, record->data, &val))
			return;
		add_sched(val, record->ts, rt);
	}
}

static void
show_wakeup_timings(unsigned long long total, unsigned long count,
		    unsigned long long lat_max, unsigned long long time_max,
		    unsigned long long lat_min, unsigned long long time_min)
{

	total /= count;

	printf("\nAverage wakeup latency: %llu.%03llu usecs\n",
	       total / 1000,
	       total % 1000);
	printf("Maximum Latency: %llu.%03llu usecs at ", lat_max / 1000, lat_max % 1000);
	printf("timestamp: %llu.%06llu\n",
	       time_max / 1000000000, ((time_max + 500) % 1000000000) / 1000);
	printf("Minimum Latency: %llu.%03llu usecs at ", lat_min / 1000, lat_min % 1000);
	printf("timestamp: %llu.%06llu\n\n", time_min / 1000000000,
	       ((time_min + 500) % 1000000000) / 1000);
}

static void finish_wakeup(void)
{
	struct wakeup_info *info;
	struct trace_hash_item **bucket;
	struct trace_hash_item *item;

	if (!show_wakeup || !wakeup_lat_count)
		return;

	show_wakeup_timings(total_wakeup_lat, wakeup_lat_count,
			    max_lat, max_time,
			    min_lat, min_time);


	if (wakeup_rt_lat_count) {
		printf("RT task timings:\n");
		show_wakeup_timings(total_wakeup_rt_lat, wakeup_rt_lat_count,
				    max_rt_lat, max_rt_time,
				    min_rt_lat, min_rt_time);
	}

	trace_hash_for_each_bucket(bucket, &wakeup_hash) {
		trace_hash_while_item(item, bucket) {
			trace_hash_del(item);
			info = container_of(item, struct wakeup_info, hash);
			free(info);
		}
	}

	trace_hash_free(&wakeup_hash);
}

void trace_show_data(struct tracecmd_input *handle, struct tep_record *record)
{
	tracecmd_show_data_func func = tracecmd_get_show_data_func(handle);
	const char *tfmt = time_format(handle, TIME_FMT_NORMAL);
	const char *cfmt = latency_format ? "%8.8s-%-5d %3d" : "%16s-%-5d [%03d]";
	struct tep_handle *pevent;
	struct tep_event *event;
	struct trace_seq s;
	int cpu = record->cpu;
	bool use_trace_clock;
	static unsigned long long last_ts;
	unsigned long long diff_ts;
	unsigned long page_size;
	char buf[50];

	page_size = tracecmd_page_size(handle);

	test_save(record, cpu);

	if (func) {
		func(handle, record);
		return;
	}

	pevent = tracecmd_get_tep(handle);
	event = tep_find_event_by_record(pevent, record);
	use_trace_clock = tracecmd_get_use_trace_clock(handle);

	trace_seq_init(&s);
	if (record->missed_events > 0)
		trace_seq_printf(&s, "CPU:%d [%lld EVENTS DROPPED]\n",
				 cpu, record->missed_events);
	else if (record->missed_events < 0)
		trace_seq_printf(&s, "CPU:%d [EVENTS DROPPED]\n", cpu);
	if (buffer_breaks || tracecmd_get_debug()) {
		if (tracecmd_record_at_buffer_start(handle, record)) {
			trace_seq_printf(&s, "CPU:%d [SUBBUFFER START]", cpu);
			if (tracecmd_get_debug())
				trace_seq_printf(&s, " [%lld:0x%llx]",
						 tracecmd_page_ts(handle, record),
						 record->offset & ~(page_size - 1));
			trace_seq_putc(&s, '\n');
		}
	}

	tep_print_event(pevent, &s, record, cfmt,
			TEP_PRINT_COMM,
			TEP_PRINT_PID,
			TEP_PRINT_CPU);

	if (latency_format) {
		if (raw_format)
			trace_seq_printf(&s, "-0x%x",
					 tep_data_flags(pevent, record));
		else
			tep_print_event(pevent, &s, record, "%s",
					TEP_PRINT_LATENCY);
	}

	tep_print_event(pevent, &s, record, tfmt, TEP_PRINT_TIME);

	if (tsdiff) {
		unsigned long long rec_ts = record->ts;

		buf[0] = 0;
		if (use_trace_clock && !tep_test_flag(pevent, TEP_NSEC_OUTPUT))
			rec_ts = (rec_ts + 500) / 1000;
		if (last_ts) {
			diff_ts = rec_ts - last_ts;
			snprintf(buf, 50, "(+%lld)", diff_ts);
			buf[49] = 0;
		}
		last_ts = rec_ts;
		trace_seq_printf(&s, " %-8s", buf);
	}

	print_event_name(&s, event);
	tep_print_event(pevent, &s, record, "%s", format_type);

	if (s.len && *(s.buffer + s.len - 1) == '\n')
		s.len--;
	if (tracecmd_get_debug()) {
		struct kbuffer *kbuf;
		struct kbuffer_raw_info info;
		void *page;
		void *offset;

		trace_seq_printf(&s, " [%d:0x%llx:%d]",
				 tracecmd_record_ts_delta(handle, record),
				 record->offset & (page_size - 1), record->size);
		kbuf = tracecmd_record_kbuf(handle, record);
		page = tracecmd_record_page(handle, record);
		offset = tracecmd_record_offset(handle, record);

		if (kbuf && page && offset) {
			struct kbuffer_raw_info *pi = &info;

			/* We need to get the record raw data to get next */
			pi->next = offset;
			pi = kbuffer_raw_get(kbuf, page, pi);
			while ((pi = kbuffer_raw_get(kbuf, page, pi))) {
				if (pi->type < KBUFFER_TYPE_PADDING)
					break;
				switch (pi->type) {
				case KBUFFER_TYPE_PADDING:
					trace_seq_printf(&s, "\n PADDING: ");
					break;
				case KBUFFER_TYPE_TIME_EXTEND:
					trace_seq_printf(&s, "\n TIME EXTEND: ");
					break;
				case KBUFFER_TYPE_TIME_STAMP:
					trace_seq_printf(&s, "\n TIME STAMP: ");
					break;
				}
				if (pi->type == KBUFFER_TYPE_TIME_STAMP)
					trace_seq_printf(&s, "timestamp:%lld length:%d",
							 pi->delta,
							 pi->length);
				else
					trace_seq_printf(&s, "delta:%lld length:%d",
							 pi->delta,
							 pi->length);
			}
		}
	}

	trace_seq_do_printf(&s);
	trace_seq_destroy(&s);

	process_wakeup(pevent, record);

	printf("\n");
}

static void read_rest(void)
{
	char buf[BUFSIZ + 1];
	int r;

	do {
		r = read(input_fd, buf, BUFSIZ);
		if (r > 0) {
			buf[r] = 0;
			printf("%s", buf);
		}
	} while (r > 0);
}

static int
test_filters(struct tep_handle *pevent, struct filter *event_filters,
	     struct tep_record *record, int neg)
{
	int found = 0;
	int ret = FILTER_NONE;
	int flags;

	if (no_irqs || no_softirqs) {
		flags = tep_data_flags(pevent, record);
		if (no_irqs && (flags & TRACE_FLAG_HARDIRQ))
			return FILTER_MISS;
		if (no_softirqs && (flags & TRACE_FLAG_SOFTIRQ))
			return FILTER_MISS;
	}

	while (event_filters) {
		ret = tep_filter_match(event_filters->filter, record);
		switch (ret) {
			case FILTER_NONE:
			case FILTER_MATCH: 
				found = 1;
		}
		/* We need to test all negative filters */
		if (!neg && found)
			break;
		event_filters = event_filters->next;
	}

	return ret;
}

struct stack_info_cpu {
	int			cpu;
	int			last_printed;
};

struct stack_info {
	struct stack_info	*next;
	struct handle_list	*handles;
	struct stack_info_cpu	*cpus;
	int			stacktrace_id;
	int			nr_cpus;
};

static int
test_stacktrace(struct handle_list *handles, struct tep_record *record,
		int last_printed)
{
	static struct stack_info *infos;
	struct stack_info *info;
	struct stack_info_cpu *cpu_info;
	struct handle_list *h;
	struct tracecmd_input *handle;
	struct tep_handle *pevent;
	struct tep_event *event;
	static int init;
	int ret;
	int id;

	if (!init) {
		init = 1;

		list_for_each_entry(h, &handle_list, list) {
			info = malloc(sizeof(*info));
			if (!info)
				die("Failed to allocate handle");
			info->handles = h;
			info->nr_cpus = tracecmd_cpus(h->handle);

			info->cpus = malloc(sizeof(*info->cpus) * info->nr_cpus);
			if (!info->cpus)
				die("Failed to allocate for %d cpus", info->nr_cpus);
			memset(info->cpus, 0, sizeof(*info->cpus));

			pevent = tracecmd_get_tep(h->handle);
			event = tep_find_event_by_name(pevent, "ftrace",
						       "kernel_stack");
			if (event)
				info->stacktrace_id = event->id;
			else
				info->stacktrace_id = 0;

			info->next = infos;
			infos = info;
		}


	}

	handle = handles->handle;
	pevent = tracecmd_get_tep(handle);

	for (info = infos; info; info = info->next)
		if (info->handles == handles)
			break;

	if (!info->stacktrace_id)
		return 0;

	cpu_info = &info->cpus[record->cpu];

	id = tep_data_type(pevent, record);

	/*
	 * Print the stack trace if the previous event was printed.
	 * But do not print the stack trace if it is explicitly
	 * being filtered out.
	 */
	if (id == info->stacktrace_id) {
		ret = test_filters(pevent, handles->event_filter_out, record, 1);
		if (ret != FILTER_MATCH)
			return cpu_info->last_printed;
		return 0;
	}

	cpu_info->last_printed = last_printed;
	return 0;
}

static struct tep_record *get_next_record(struct handle_list *handles)
{
	struct tep_record *record;
	struct tep_handle *pevent;
	int found = 0;
	int cpu;
	int ret;

	if (handles->record)
		return handles->record;

	if (handles->done)
		return NULL;

	pevent = tracecmd_get_tep(handles->handle);

	do {
		if (filter_cpus) {
			long long last_stamp = -1;
			struct tep_record *precord;
			int first_record = 1;
			int next_cpu = -1;
			int i;

			for (i = 0; (cpu = filter_cpus[i]) >= 0; i++) {
				precord = tracecmd_peek_data(handles->handle, cpu);
				if (precord &&
				    (first_record || precord->ts < last_stamp)) {
					next_cpu = cpu;
					last_stamp = precord->ts;
					first_record = 0;
				}
			}
			if (!first_record)
				record = tracecmd_read_data(handles->handle, next_cpu);
			else
				record = NULL;
		} else
			record = tracecmd_read_next_data(handles->handle, &cpu);

		if (record) {
			ret = test_filters(pevent, handles->event_filters, record, 0);
			switch (ret) {
			case FILTER_NOEXIST:
				/* Stack traces may still filter this */
				if (stacktrace_id &&
				    test_stacktrace(handles, record, 0))
					found = 1;
				else
					tracecmd_free_record(record);
				break;
			case FILTER_NONE:
			case FILTER_MATCH:
				/* Test the negative filters (-v) */
				ret = test_filters(pevent, handles->event_filter_out,
						   record, 1);
				if (ret != FILTER_MATCH) {
					found = 1;
					break;
				}
				/* fall through */
			default:
				tracecmd_free_record(record);
			}
		}
	} while (record && !found);

	if (record && stacktrace_id)
		test_stacktrace(handles, record, 1);

	handles->record = record;
	if (!record)
		handles->done = 1;

	return record;
}

static void free_handle_record(struct handle_list *handles)
{
	if (!handles->record)
		return;

	tracecmd_free_record(handles->record);
	handles->record = NULL;
}

static void print_handle_file(struct handle_list *handles)
{
	/* Only print file names if more than one file is read */
	if (!multi_inputs && !instances)
		return;
	if (handles->file)
		printf("%*s: ", max_file_size, handles->file);
	else
		printf("%*s  ", max_file_size, "");
}

static void free_filters(struct filter *event_filter)
{
	struct filter *filter;

	while (event_filter) {
		filter = event_filter;
		event_filter = filter->next;

		tep_filter_free(filter->filter);
		free(filter);
	}
}

enum output_type {
	OUTPUT_NORMAL,
	OUTPUT_STAT_ONLY,
	OUTPUT_UNAME_ONLY,
	OUTPUT_VERSION_ONLY,
};

static void read_data_info(struct list_head *handle_list, enum output_type otype,
			   int global)
{
	struct handle_list *handles;
	struct handle_list *last_handle;
	struct tep_record *record;
	struct tep_record *last_record;
	struct tep_handle *pevent;
	struct tep_event *event;
	int ret;

	list_for_each_entry(handles, handle_list, list) {
		int cpus;

		cpus = tracecmd_cpus(handles->handle);
		handles->cpus = cpus;
		handles->last_timestamp = calloc(cpus, sizeof(*handles->last_timestamp));
		if (!handles->last_timestamp)
			die("allocating timestamps");

		/* Don't process instances that we added here */
		if (tracecmd_is_buffer_instance(handles->handle))
			continue;

		ret = tracecmd_init_data(handles->handle);
		if (ret < 0)
			die("failed to init data");

		print_handle_file(handles);
		printf("cpus=%d\n", cpus);

		/* Latency trace is just all ASCII */
		if (ret > 0) {
			if (multi_inputs)
				die("latency traces do not work with multiple inputs");
			read_rest();
			return;
		}

		switch (otype) {
		case OUTPUT_NORMAL:
			break;
		case OUTPUT_STAT_ONLY:
			printf("\nKernel buffer statistics:\n"
			       "  Note: \"entries\" are the entries left in the kernel ring buffer and are not\n"
			       "        recorded in the trace data. They should all be zero.\n\n");
			tracecmd_print_stats(handles->handle);
			continue;
		case OUTPUT_UNAME_ONLY:
			tracecmd_print_uname(handles->handle);
		case OUTPUT_VERSION_ONLY:
			tracecmd_print_version(handles->handle);
			continue;
		}

		/* Find the kernel_stacktrace if available */
		pevent = tracecmd_get_tep(handles->handle);
		event = tep_find_event_by_name(pevent, "ftrace", "kernel_stack");
		if (event)
			stacktrace_id = event->id;

		init_wakeup(handles->handle);
		if (last_hook)
			last_hook->next = tracecmd_hooks(handles->handle);
		else
			hooks = tracecmd_hooks(handles->handle);
		if (profile)
			trace_init_profile(handles->handle, hooks, global);

		process_filters(handles);

		/* If this file has buffer instances, get the handles for them */
		instances = tracecmd_buffer_instances(handles->handle);
		if (instances) {
			struct tracecmd_input *new_handle;
			const char *name;
			int i;

			for (i = 0; i < instances; i++) {
				name = tracecmd_buffer_instance_name(handles->handle, i);
				if (!name)
					die("error in reading buffer instance");
				new_handle = tracecmd_buffer_instance_handle(handles->handle, i);
				if (!new_handle) {
					warning("could not retreive handle %s", name);
					continue;
				}
				add_handle(new_handle, name);
			}
		}
	}

	if (otype != OUTPUT_NORMAL)
		return;

	do {
		last_handle = NULL;
		last_record = NULL;

		list_for_each_entry(handles, handle_list, list) {
			record = get_next_record(handles);
			if (!record)
				continue;
			if (!last_record ||
			    (record && record->ts < last_record->ts)) {
				last_record = record;
				last_handle = handles;
			}
		}
		if (last_record) {
			int cpu = last_record->cpu;
			if (cpu >= last_handle->cpus)
				die("cpu %d creater than %d\n", cpu, last_handle->cpus);
			if (tscheck &&
			    last_handle->last_timestamp[cpu] > last_record->ts) {
				errno = 0;
				warning("WARNING: Record on cpu %d went backwards: %lld to %lld delta: -%lld\n",
					cpu, last_handle->last_timestamp[cpu],
					last_record->ts,
					last_handle->last_timestamp[cpu] - last_record->ts);
			}
			last_handle->last_timestamp[cpu] = last_record->ts;
			print_handle_file(last_handle);
			trace_show_data(last_handle->handle, last_record);
			free_handle_record(last_handle);
		}
	} while (last_record);

	if (profile)
		do_trace_profile();

	list_for_each_entry(handles, handle_list, list) {
		free_filters(handles->event_filters);
		free_filters(handles->event_filter_out);
		free(handles->last_timestamp);

		show_test(handles->handle);
	}
}

struct tracecmd_input *read_trace_header(const char *file, int flags)
{
	input_fd = open(file, O_RDONLY);
	if (input_fd < 0)
		die("opening '%s'\n", file);

	return tracecmd_alloc_fd(input_fd, flags);
}

static void sig_end(int sig)
{
	fprintf(stderr, "trace-cmd: Received SIGINT\n");
	exit(0);
}

static const char *skip_space_and_test_digit(const char *p, const char *cpu_str)
{
	while (isspace(*p))
		p++;
	if (!isdigit(*p))
		die("invalid character '%c' in cpu string '%s'",
		    *p, cpu_str);
	return p;
}

static void __add_cpu(int cpu)
{
	filter_cpus = tracecmd_add_id(filter_cpus, cpu, nr_filter_cpus++);
}

static void parse_cpulist(const char *cpu_str)
{
	unsigned a, b;
	const char *s = cpu_str;

	do {
		s = skip_space_and_test_digit(s, cpu_str);
		b = a = strtoul(s, (char **)&s, 10);
		if (*s == '-') {
			s = skip_space_and_test_digit(s + 1, cpu_str);
			b = strtoul(s, (char **)&s, 10);
		}
		if (!(a <= b))
			die("range of cpu numbers must be lower to greater");
		while (a <= b) {
			__add_cpu(a);
			a++;
		}
		if (*s == ',' || *s == ':')
			s++;
	} while (*s != '\0');
}

static void read_file_fd(int fd, char *dst, int len)
{
	size_t size = 0;
	int r;

	do {
		r = read(fd, dst+size, len);
		if (r > 0) {
			size += r;
			len -= r;
		}
	} while (r > 0);
}

static void add_functions(struct tep_handle *pevent, const char *file)
{
	struct stat st;
	char *buf;
	int ret;
	int fd;

	fd = open(file, O_RDONLY);
	if (fd < 0)
		die("Can't read file %s", file);

	ret = fstat(fd, &st);
	if (ret < 0)
		die("Can't stat file %s", file);

	buf = malloc(st.st_size);
	if (!buf)
		die("Failed to allocate for function buffer");
	read_file_fd(fd, buf, st.st_size);
	close(fd);
	tracecmd_parse_proc_kallsyms(pevent, buf, st.st_size);
	free(buf);
}

static void process_plugin_option(char *option)
{
	char *name = option;
	char *val = NULL;
	char *p;

	if ((p = strstr(name, "="))) {
		*p = '\0';
		val = p+1;
	}
	tep_plugin_add_option(name, val);
}

static void set_event_flags(struct tep_handle *pevent, struct event_str *list,
			    unsigned int flag)
{
	struct tep_event **events;
	struct tep_event *event;
	struct event_str *str;
	regex_t regex;
	int ret;
	int i;

	if (!list)
		return;

	events = tep_list_events(pevent, 0);

	for (str = list; str; str = str->next) {
		char *match;

		match = malloc(strlen(str->event) + 3);
		if (!match)
			die("Failed to allocate for match string '%s'", str->event);
		sprintf(match, "^%s$", str->event);

		ret = regcomp(&regex, match, REG_ICASE|REG_NOSUB);
		if (ret < 0)
			die("Can't parse '%s'", str->event);
		free(match);
		for (i = 0; events[i]; i++) {
			event = events[i];
			if (!regexec(&regex, event->name, 0, NULL, 0) ||
			    !regexec(&regex, event->system, 0, NULL, 0))
				event->flags |= flag;
		}
	}
}

static void add_hook(const char *arg)
{
	struct hook_list *hook;

	hook = tracecmd_create_event_hook(arg);

	hook->next = hooks;
	hooks = hook;
	if (!last_hook)
		last_hook = hook;
}

enum {
	OPT_version	= 237,
	OPT_tscheck	= 238,
	OPT_tsdiff	= 239,
	OPT_ts2secs	= 240,
	OPT_tsoffset	= 241,
	OPT_bycomm	= 242,
	OPT_debug	= 243,
	OPT_uname	= 244,
	OPT_profile	= 245,
	OPT_event	= 246,
	OPT_comm	= 247,
	OPT_boundary	= 248,
	OPT_stat	= 249,
	OPT_pid		= 250,
	OPT_nodate	= 251,
	OPT_check_event_parsing	= 252,
	OPT_kallsyms	= 253,
	OPT_events	= 254,
	OPT_cpu		= 255,
};

void trace_report (int argc, char **argv)
{
	struct tracecmd_input *handle;
	struct tep_handle *pevent;
	struct event_str *raw_events = NULL;
	struct event_str *nohandler_events = NULL;
	struct event_str **raw_ptr = &raw_events;
	struct event_str **nohandler_ptr = &nohandler_events;
	const char *functions = NULL;
	const char *print_event = NULL;
	struct input_files *inputs;
	struct handle_list *handles;
	enum output_type otype;
	long long tsoffset = 0;
	unsigned long long ts2secs = 0;
	unsigned long long ts2sc;
	int open_flags = 0;
	int show_stat = 0;
	int show_funcs = 0;
	int show_endian = 0;
	int show_page_size = 0;
	int show_printk = 0;
	int show_uname = 0;
	int show_version = 0;
	int show_events = 0;
	int print_events = 0;
	int nanosec = 0;
	int no_date = 0;
	int global = 0;
	int neg = 0;
	int ret = 0;
	int check_event_parsing = 0;
	int c;

	list_head_init(&handle_list);
	list_head_init(&input_files);

	if (argc < 2)
		usage(argv);

	if (strcmp(argv[1], "report") != 0)
		usage(argv);

	signal(SIGINT, sig_end);

	for (;;) {
		int option_index = 0;
		static struct option long_options[] = {
			{"cpu", required_argument, NULL, OPT_cpu},
			{"events", no_argument, NULL, OPT_events},
			{"event", required_argument, NULL, OPT_event},
			{"filter-test", no_argument, NULL, 'T'},
			{"kallsyms", required_argument, NULL, OPT_kallsyms},
			{"pid", required_argument, NULL, OPT_pid},
			{"comm", required_argument, NULL, OPT_comm},
			{"check-events", no_argument, NULL,
				OPT_check_event_parsing},
			{"nodate", no_argument, NULL, OPT_nodate},
			{"stat", no_argument, NULL, OPT_stat},
			{"boundary", no_argument, NULL, OPT_boundary},
			{"debug", no_argument, NULL, OPT_debug},
			{"profile", no_argument, NULL, OPT_profile},
			{"uname", no_argument, NULL, OPT_uname},
			{"version", no_argument, NULL, OPT_version},
			{"by-comm", no_argument, NULL, OPT_bycomm},
			{"ts-offset", required_argument, NULL, OPT_tsoffset},
			{"ts2secs", required_argument, NULL, OPT_ts2secs},
			{"ts-diff", no_argument, NULL, OPT_tsdiff},
			{"ts-check", no_argument, NULL, OPT_tscheck},
			{"help", no_argument, NULL, '?'},
			{NULL, 0, NULL, 0}
		};

		c = getopt_long (argc-1, argv+1, "+hSIi:H:feGpRr:tPNn:LlEwF:VvTqO:",
			long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 'h':
			usage(argv);
			break;
		case 'i':
			if (input_file) {
				if (!multi_inputs) {
					add_input(input_file);
					if (tsoffset)
						last_input_file->tsoffset = tsoffset;
				}
				multi_inputs++;
				add_input(optarg);
			} else
				input_file = optarg;
			break;
		case 'F':
			add_filter(optarg, neg);
			break;
		case 'H':
			add_hook(optarg);
			break;
		case 'T':
			test_filters_mode = 1;
			break;
		case 'f':
			show_funcs = 1;
			break;
		case 'I':
			no_irqs = 1;
			break;
		case 'S':
			no_softirqs = 1;
			break;
		case 'P':
			show_printk = 1;
			break;
		case 'L':
			open_flags |= TRACECMD_FL_LOAD_NO_SYSTEM_PLUGINS;
			break;
		case 'N':
			open_flags |= TRACECMD_FL_LOAD_NO_PLUGINS;
			break;
		case 'n':
			*nohandler_ptr = malloc(sizeof(struct event_str));
			if (!*nohandler_ptr)
				die("Failed to allocate for '-n %s'", optarg);
			(*nohandler_ptr)->event = optarg;
			(*nohandler_ptr)->next = NULL;
			nohandler_ptr = &(*nohandler_ptr)->next;
			break;
		case 'e':
			show_endian = 1;
			break;
		case 'p':
			show_page_size = 1;
			break;
		case 'E':
			show_events = 1;
			break;
		case 'G':
			global = 1;
			break;
		case 'R':
			raw_format = true;
			break;
		case 'r':
			*raw_ptr = malloc(sizeof(struct event_str));
			if (!*raw_ptr)
				die("Failed to allocate '-r %s'", optarg);
			(*raw_ptr)->event = optarg;
			(*raw_ptr)->next = NULL;
			raw_ptr = &(*raw_ptr)->next;
			break;
		case 't':
			nanosec = 1;
			break;
		case 'w':
			show_wakeup = 1;
			break;
		case 'l':
			latency_format = 1;
			break;
		case 'O':
			process_plugin_option(optarg);
			break;
		case 'v':
			if (neg)
				die("Only 1 -v can be used");
			neg = 1;
			break;
		case 'V':
			show_status = 1;
			break;
		case 'q':
			silence_warnings = 1;
			break;
		case OPT_cpu:
			parse_cpulist(optarg);
			break;
		case OPT_events:
			print_events = 1;
			break;
		case OPT_event:
			print_event = optarg;
			break;
		case OPT_kallsyms:
			functions = optarg;
			break;
		case OPT_pid:
			add_pid_filter(optarg);
			break;
		case OPT_comm:
			add_comm_filter(optarg);
			break;
		case OPT_check_event_parsing:
			check_event_parsing = 1;
			break;
		case OPT_nodate:
			no_date = 1;
			break;
		case OPT_stat:
			show_stat = 1;
			break;
		case OPT_boundary:
			/* Debug to look at buffer breaks */
			buffer_breaks = 1;
			break;
		case OPT_debug:
			buffer_breaks = 1;
			tracecmd_set_debug(true);
			break;
		case OPT_profile:
			profile = 1;
			break;
		case OPT_uname:
			show_uname = 1;
			break;
		case OPT_version:
			show_version = 1;
			break;
		case OPT_bycomm:
			trace_profile_set_merge_like_comms();
			break;
		case OPT_ts2secs:
			ts2sc = atoll(optarg);
			if (multi_inputs)
				last_input_file->ts2secs = ts2sc;
			else
				ts2secs = ts2sc;
			break;
		case OPT_tsoffset:
			tsoffset = atoll(optarg);
			if (multi_inputs)
				last_input_file->tsoffset = tsoffset;
			if (!input_file)
				die("--ts-offset must come after -i");
			break;
		case OPT_tsdiff:
			tsdiff = 1;
			break;
		case OPT_tscheck:
			tscheck = 1;
			break;
		default:
			usage(argv);
		}
	}

	if ((argc - optind) >= 2) {
		if (input_file)
			usage(argv);
		input_file = argv[optind + 1];
	}

	if (!input_file)
		input_file = default_input_file;

	if (!multi_inputs) {
		add_input(input_file);
		if (tsoffset)
			last_input_file->tsoffset = tsoffset;
	} else if (show_wakeup)
		die("Wakeup tracing can only be done on a single input file");

	list_for_each_entry(inputs, &input_files, list) {
		handle = read_trace_header(inputs->file, open_flags);
		if (!handle)
			die("error reading header for %s", inputs->file);

		/* If used with instances, top instance will have no tag */
		add_handle(handle, multi_inputs ? inputs->file : NULL);

		if (no_date)
			tracecmd_set_flag(handle, TRACECMD_FL_IGNORE_DATE);

		page_size = tracecmd_page_size(handle);

		if (show_page_size) {
			printf("file page size is %d, and host page size is %d\n",
			       page_size,
			       getpagesize());
			return;
		}

		if (inputs->tsoffset)
			tracecmd_set_ts_offset(handle, inputs->tsoffset);

		if (inputs->ts2secs)
			tracecmd_set_ts2secs(handle, inputs->ts2secs);
		else if (ts2secs)
			tracecmd_set_ts2secs(handle, ts2secs);

		pevent = tracecmd_get_tep(handle);

		if (nanosec)
			tep_set_flag(pevent, TEP_NSEC_OUTPUT);

		if (raw_format)
			format_type = TEP_PRINT_INFO_RAW;

		if (test_filters_mode)
			tep_set_test_filters(pevent, 1);

		if (functions)
			add_functions(pevent, functions);

		if (show_endian) {
			printf("file is %s endian and host is %s endian\n",
				tep_is_file_bigendian(pevent) ? "big" : "little",
				tep_is_local_bigendian(pevent) ? "big" : "little");
			return;
		}

		if (print_events) {
			tracecmd_print_events(handle, NULL);
			return;
		}

		if (print_event) {
			tracecmd_print_events(handle, print_event);
			return;
		}

		ret = tracecmd_read_headers(handle, 0);
		if (check_event_parsing) {
			if (ret || tracecmd_get_parsing_failures(handle))
				exit(EINVAL);
			else
				exit(0);
		} else {
			if (ret)
				return;
		}

		if (show_funcs) {
			tep_print_funcs(pevent);
			return;
		}
		if (show_printk) {
			tep_print_printk(pevent);
			return;
		}

		if (show_events) {
			struct tep_event **events;
			struct tep_event *event;
			int i;

			events = tep_list_events(pevent, TEP_EVENT_SORT_SYSTEM);
			for (i = 0; events[i]; i++) {
				event = events[i];
				if (event->system)
					printf("%s:", event->system);
				printf("%s\n", event->name);
			}
			return;
		}

		set_event_flags(pevent, nohandler_events, TEP_EVENT_FL_NOHANDLE);
		set_event_flags(pevent, raw_events, TEP_EVENT_FL_PRINTRAW);
	}

	otype = OUTPUT_NORMAL;

	if (show_stat)
		otype = OUTPUT_STAT_ONLY;
	/* yeah yeah, uname overrides stat */
	if (show_uname)
		otype = OUTPUT_UNAME_ONLY;
	/* and version overrides uname! */
	if (show_version)
		otype = OUTPUT_VERSION_ONLY;
	read_data_info(&handle_list, otype, global);

	list_for_each_entry(handles, &handle_list, list) {
		tracecmd_close(handles->handle);
	}
	free_handles();
	free_inputs();

	finish_wakeup();

	return;
}
