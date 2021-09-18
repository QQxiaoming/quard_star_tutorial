// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2013 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 *
 * This code was inspired by Ezequiel Garcia's trace_analyze program:
 *   git://github.com/ezequielgarcia/trace_analyze.git
 *
 * Unfortuntately, I hate working with Python, and I also had trouble
 * getting it to work, as I had an old python on my Fedora 13, and it
 * was written for the newer version. I decided to do some of it here
 * in C.
 */
#define _LARGEFILE64_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>

#include "trace-local.h"
#include "trace-hash-local.h"
#include "list.h"

static int kmalloc_type;
static int kmalloc_node_type;
static int kfree_type;
static int kmem_cache_alloc_type;
static int kmem_cache_alloc_node_type;
static int kmem_cache_free_type;

static struct tep_format_field *common_type_mem;

static struct tep_format_field *kmalloc_callsite_field;
static struct tep_format_field *kmalloc_bytes_req_field;
static struct tep_format_field *kmalloc_bytes_alloc_field;
static struct tep_format_field *kmalloc_ptr_field;

static struct tep_format_field *kmalloc_node_callsite_field;
static struct tep_format_field *kmalloc_node_bytes_req_field;
static struct tep_format_field *kmalloc_node_bytes_alloc_field;
static struct tep_format_field *kmalloc_node_ptr_field;

static struct tep_format_field *kfree_ptr_field;

static struct tep_format_field *kmem_cache_callsite_field;
static struct tep_format_field *kmem_cache_bytes_req_field;
static struct tep_format_field *kmem_cache_bytes_alloc_field;
static struct tep_format_field *kmem_cache_ptr_field;

static struct tep_format_field *kmem_cache_node_callsite_field;
static struct tep_format_field *kmem_cache_node_bytes_req_field;
static struct tep_format_field *kmem_cache_node_bytes_alloc_field;
static struct tep_format_field *kmem_cache_node_ptr_field;

static struct tep_format_field *kmem_cache_free_ptr_field;

static void *zalloc(size_t size)
{
	return calloc(1, size);
}

static struct tep_event *
update_event(struct tep_handle *pevent,
	     const char *sys, const char *name, int *id)
{
	struct tep_event *event;

	event = tep_find_event_by_name(pevent, sys, name);
	if (!event)
		return NULL;

	*id = event->id;

	return event;
}

static void update_kmalloc(struct tep_handle *pevent)
{
	struct tep_event *event;

	event = update_event(pevent, "kmem", "kmalloc", &kmalloc_type);
	if (!event)
		return;

	kmalloc_callsite_field = tep_find_field(event, "call_site");
	kmalloc_bytes_req_field = tep_find_field(event, "bytes_req");
	kmalloc_bytes_alloc_field = tep_find_field(event, "bytes_alloc");
	kmalloc_ptr_field = tep_find_field(event, "ptr");
}

static void update_kmalloc_node(struct tep_handle *pevent)
{
	struct tep_event *event;

	event = update_event(pevent, "kmem", "kmalloc_node", &kmalloc_node_type);
	if (!event)
		return;

	kmalloc_node_callsite_field = tep_find_field(event, "call_site");
	kmalloc_node_bytes_req_field = tep_find_field(event, "bytes_req");
	kmalloc_node_bytes_alloc_field = tep_find_field(event, "bytes_alloc");
	kmalloc_node_ptr_field = tep_find_field(event, "ptr");
}

static void update_kfree(struct tep_handle *pevent)
{
	struct tep_event *event;

	event = update_event(pevent, "kmem", "kfree", &kfree_type);
	if (!event)
		return;

	kfree_ptr_field = tep_find_field(event, "ptr");
}

static void update_kmem_cache_alloc(struct tep_handle *pevent)
{
	struct tep_event *event;

	event = update_event(pevent, "kmem", "kmem_cache_alloc", &kmem_cache_alloc_type);
	if (!event)
		return;

	kmem_cache_callsite_field = tep_find_field(event, "call_site");
	kmem_cache_bytes_req_field = tep_find_field(event, "bytes_req");
	kmem_cache_bytes_alloc_field = tep_find_field(event, "bytes_alloc");
	kmem_cache_ptr_field = tep_find_field(event, "ptr");
}

static void update_kmem_cache_alloc_node(struct tep_handle *pevent)
{
	struct tep_event *event;

	event = update_event(pevent, "kmem", "kmem_cache_alloc_node",
			     &kmem_cache_alloc_node_type);
	if (!event)
		return;

	kmem_cache_node_callsite_field = tep_find_field(event, "call_site");
	kmem_cache_node_bytes_req_field = tep_find_field(event, "bytes_req");
	kmem_cache_node_bytes_alloc_field = tep_find_field(event, "bytes_alloc");
	kmem_cache_node_ptr_field = tep_find_field(event, "ptr");
}

static void update_kmem_cache_free(struct tep_handle *pevent)
{
	struct tep_event *event;

	event = update_event(pevent, "kmem", "kmem_cache_free", &kmem_cache_free_type);
	if (!event)
		return;

	kmem_cache_free_ptr_field = tep_find_field(event, "ptr");
}

struct func_descr {
	struct func_descr	*next;
	const char		*func;
	unsigned long		total_alloc;
	unsigned long		total_req;
	unsigned long		current_alloc;
	unsigned long		current_req;
	unsigned long		max_alloc;
	unsigned long		max_req;
	unsigned long		waste;
	unsigned long		max_waste;
};

struct ptr_descr {
	struct ptr_descr	*next;
	struct func_descr	*func;
	unsigned long long	ptr;
	unsigned long		alloc;
	unsigned long		req;
};

#define HASH_BITS	12
#define HASH_SIZE	(1 << HASH_BITS)
#define HASH_MASK	(HASH_SIZE - 1);

static struct func_descr *func_hash[HASH_SIZE];
static struct ptr_descr *ptr_hash[HASH_SIZE];
static struct func_descr **func_list;

static unsigned func_count;

static int make_key(const void *ptr, int size)
{
	int key = 0;
	int i;
	char *kp = (char *)&key;
	const char *indx = ptr;

	for (i = 0; i < size; i++)
		kp[i & 3] ^= indx[i];

	return trace_hash(key);
}

static struct func_descr *find_func(const char *func)
{
	struct func_descr *funcd;
	int key = make_key(func, strlen(func)) & HASH_MASK;

	for (funcd = func_hash[key]; funcd; funcd = funcd->next) {
		/*
		 * As func is always a constant to one pointer,
		 * we can use a direct compare instead of strcmp.
		 */
		if (funcd->func == func)
			return funcd;
	}

	return NULL;
}

static struct func_descr *create_func(const char *func)
{
	struct func_descr *funcd;
	int key = make_key(func, strlen(func)) & HASH_MASK;

	funcd = zalloc(sizeof(*funcd));
	if (!funcd)
		die("malloc");

	funcd->func = func;
	funcd->next = func_hash[key];
	func_hash[key] = funcd;

	func_count++;

	return funcd;
}

static struct ptr_descr *find_ptr(unsigned long long ptr)
{
	struct ptr_descr *ptrd;
	int key = make_key(&ptr, sizeof(ptr)) & HASH_MASK;

	for (ptrd = ptr_hash[key]; ptrd; ptrd = ptrd->next) {
		if (ptrd->ptr == ptr)
			return ptrd;
	}

	return NULL;
}

static struct ptr_descr *create_ptr(unsigned long long ptr)
{
	struct ptr_descr *ptrd;
	int key = make_key(&ptr, sizeof(ptr)) & HASH_MASK;

	ptrd = zalloc(sizeof(*ptrd));
	if (!ptrd)
		die("malloc");

	ptrd->ptr = ptr;
	ptrd->next = ptr_hash[key];
	ptr_hash[key] = ptrd;

	return ptrd;
}

static void remove_ptr(unsigned long long ptr)
{
	struct ptr_descr *ptrd, **last;
	int key = make_key(&ptr, sizeof(ptr)) & HASH_MASK;

	last = &ptr_hash[key];
	for (ptrd = ptr_hash[key]; ptrd; ptrd = ptrd->next) {
		if (ptrd->ptr == ptr)
			break;
		last = &ptrd->next;
	}

	if (!ptrd)
		return;

	*last = ptrd->next;
	free(ptrd);
}

static void add_kmalloc(const char *func, unsigned long long ptr,
			unsigned int req, int alloc)
{
	struct func_descr *funcd;
	struct ptr_descr *ptrd;

	funcd = find_func(func);
	if (!funcd)
		funcd = create_func(func);

	funcd->total_alloc += alloc;
	funcd->total_req += req;
	funcd->current_alloc += alloc;
	funcd->current_req += req;
	if (funcd->current_alloc > funcd->max_alloc)
		funcd->max_alloc = funcd->current_alloc;
	if (funcd->current_req > funcd->max_req)
		funcd->max_req = funcd->current_req;

	ptrd = find_ptr(ptr);
	if (!ptrd)
		ptrd = create_ptr(ptr);

	ptrd->alloc = alloc;
	ptrd->req = req;
	ptrd->func = funcd;
}

static void remove_kmalloc(unsigned long long ptr)
{
	struct func_descr *funcd;
	struct ptr_descr *ptrd;

	ptrd = find_ptr(ptr);
	if (!ptrd)
		return;

	funcd = ptrd->func;
	funcd->current_alloc -= ptrd->alloc;
	funcd->current_req -= ptrd->req;

	remove_ptr(ptr);
}

static void
process_kmalloc(struct tep_handle *pevent, struct tep_record *record,
		struct tep_format_field *callsite_field,
		struct tep_format_field *bytes_req_field,
		struct tep_format_field *bytes_alloc_field,
		struct tep_format_field *ptr_field)
{
	unsigned long long callsite;
	unsigned long long val;
	unsigned long long ptr;
	unsigned int req;
	int alloc;
	const char *func;

	tep_read_number_field(callsite_field, record->data, &callsite);
	tep_read_number_field(bytes_req_field, record->data, &val);
	req = val;
	tep_read_number_field(bytes_alloc_field, record->data, &val);
	alloc = val;
	tep_read_number_field(ptr_field, record->data, &ptr);

	func = tep_find_function(pevent, callsite);

	add_kmalloc(func, ptr, req, alloc);
}

static void
process_kfree(struct tep_handle *pevent, struct tep_record *record,
	      struct tep_format_field *ptr_field)
{
	unsigned long long ptr;

	tep_read_number_field(ptr_field, record->data, &ptr);

	remove_kmalloc(ptr);
}

static void
process_record(struct tep_handle *pevent, struct tep_record *record)
{
	unsigned long long val;
	int type;

	tep_read_number_field(common_type_mem, record->data, &val);
	type = val;

	if (type == kmalloc_type)
		return process_kmalloc(pevent, record,
				       kmalloc_callsite_field,
				       kmalloc_bytes_req_field,
				       kmalloc_bytes_alloc_field,
				       kmalloc_ptr_field);
	if (type == kmalloc_node_type)
		return process_kmalloc(pevent, record,
				       kmalloc_node_callsite_field,
				       kmalloc_node_bytes_req_field,
				       kmalloc_node_bytes_alloc_field,
				       kmalloc_node_ptr_field);
	if (type == kfree_type)
		return process_kfree(pevent, record, kfree_ptr_field);

	if (type == kmem_cache_alloc_type)
		return process_kmalloc(pevent, record,
				       kmem_cache_callsite_field,
				       kmem_cache_bytes_req_field,
				       kmem_cache_bytes_alloc_field,
				       kmem_cache_ptr_field);
	if (type == kmem_cache_alloc_node_type)
		return process_kmalloc(pevent, record,
				       kmem_cache_node_callsite_field,
				       kmem_cache_node_bytes_req_field,
				       kmem_cache_node_bytes_alloc_field,
				       kmem_cache_node_ptr_field);
	if (type == kmem_cache_free_type)
		return process_kfree(pevent, record, kmem_cache_free_ptr_field);
}

static int func_cmp(const void *a, const void *b)
{
	const struct func_descr *fa = *(const struct func_descr **)a;
	const struct func_descr *fb = *(const struct func_descr **)b;

	if (fa->waste > fb->waste)
		return -1;
	if (fa->waste < fb->waste)
		return 1;
	return 0;
}

static void sort_list(void)
{
	struct func_descr *funcd;
	int h;
	int i = 0;

	func_list = zalloc(sizeof(*func_list) * func_count);

	for (h = 0; h < HASH_SIZE; h++) {
		for (funcd = func_hash[h]; funcd; funcd = funcd->next) {
			funcd->waste = funcd->current_alloc - funcd->current_req;
			funcd->max_waste = funcd->max_alloc - funcd->max_req;
			if (i == func_count)
				die("more funcs than expected\n");
			func_list[i++] = funcd;
		}
	}

	qsort(func_list, func_count, sizeof(*func_list), func_cmp);
}

static void print_list(void)
{
	struct func_descr *funcd;
	int i;

	printf("                Function            \t");
	printf("Waste\tAlloc\treq\t\tTotAlloc     TotReq\t\tMaxAlloc     MaxReq\t");
	printf("MaxWaste\n");
	printf("                --------            \t");
	printf("-----\t-----\t---\t\t--------     ------\t\t--------     ------\t");
	printf("--------\n");
	
	for (i = 0; i < func_count; i++) {
		funcd = func_list[i];

		printf("%32s\t%ld\t%ld\t%ld\t\t%8ld   %8ld\t\t%8ld   %8ld\t%ld\n",
		       funcd->func, funcd->waste,
		       funcd->current_alloc, funcd->current_req,
		       funcd->total_alloc, funcd->total_req,
		       funcd->max_alloc, funcd->max_req, funcd->max_waste);
	}
}

static void do_trace_mem(struct tracecmd_input *handle)
{
	struct tep_handle *pevent = tracecmd_get_tep(handle);
	struct tep_record *record;
	struct tep_event *event;
	int missed_events = 0;
	int cpus;
	int cpu;
	int ret;

	ret = tracecmd_init_data(handle);
	if (ret < 0)
		die("failed to init data");

	if (ret > 0)
		die("trace-cmd mem does not work with latency traces\n");

	cpus = tracecmd_cpus(handle);

	/* Need to get any event */
	for (cpu = 0; cpu < cpus; cpu++) {
		record = tracecmd_peek_data(handle, cpu);
		if (record)
			break;
	}
	if (!record)
		die("No records found in file");

	ret = tep_data_type(pevent, record);
	event = tep_find_event(pevent, ret);

	common_type_mem = tep_find_common_field(event, "common_type");
	if (!common_type_mem)
		die("Can't find a 'type' field?");

	update_kmalloc(pevent);
	update_kmalloc_node(pevent);
	update_kfree(pevent);
	update_kmem_cache_alloc(pevent);
	update_kmem_cache_alloc_node(pevent);
	update_kmem_cache_free(pevent);

	while ((record = tracecmd_read_next_data(handle, &cpu))) {

		/* record missed event */
		if (!missed_events && record->missed_events)
			missed_events = 1;

		process_record(pevent, record);
		tracecmd_free_record(record);
	}

	sort_list();
	print_list();
}

void trace_mem(int argc, char **argv)
{
	struct tracecmd_input *handle;
	const char *input_file = NULL;
	int ret;

	for (;;) {
		int c;

		c = getopt(argc-1, argv+1, "+hi:");
		if (c == -1)
			break;
		switch (c) {
		case 'h':
			usage(argv);
			break;
		case 'i':
			if (input_file)
				die("Only one input for mem");
			input_file = optarg;
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
		input_file = DEFAULT_INPUT_FILE;

	handle = tracecmd_alloc(input_file, 0);
	if (!handle)
		die("can't open %s\n", input_file);

	ret = tracecmd_read_headers(handle, 0);
	if (ret)
		return;

	do_trace_mem(handle);

	tracecmd_close(handle);
}
