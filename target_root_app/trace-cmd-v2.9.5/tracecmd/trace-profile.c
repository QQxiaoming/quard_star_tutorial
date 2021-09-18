// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2014 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 */

/** FIXME: Convert numbers based on machine and file */
#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef NO_AUDIT
#include <libaudit.h>
#endif
#include "trace-local.h"
#include "trace-hash.h"
#include "trace-hash-local.h"
#include "list.h"

#include <linux/time64.h>

#ifdef WARN_NO_AUDIT
# warning "lib audit not found, using raw syscalls "	\
	"(install libaudit-devel(for fedora) or libaudit-dev(for debian/ubuntu) and try again)"
#endif

#define TASK_STATE_TO_CHAR_STR "RSDTtXZxKWP"
#define TASK_STATE_MAX		1024

#define task_from_item(item)	container_of(item, struct task_data, hash)
#define start_from_item(item)	container_of(item, struct start_data, hash)
#define event_from_item(item)	container_of(item, struct event_hash, hash)
#define stack_from_item(item)	container_of(item, struct stack_data, hash)
#define group_from_item(item)	container_of(item, struct group_data, hash)
#define event_data_from_item(item)	container_of(item, struct event_data, hash)

static unsigned long long nsecs_per_sec(unsigned long long ts)
{
	return ts / NSEC_PER_SEC;
}

static unsigned long long mod_to_usec(unsigned long long ts)
{
	return ((ts % NSEC_PER_SEC) + NSEC_PER_USEC / 2) / NSEC_PER_USEC;
}

struct handle_data;
struct event_hash;
struct event_data;

typedef void (*event_data_print)(struct trace_seq *s, struct event_hash *hash);
typedef int (*handle_event_func)(struct handle_data *h, unsigned long long pid,
				 struct event_data *data,
				 struct tep_record *record, int cpu);

enum event_data_type {
	EVENT_TYPE_UNDEFINED,
	EVENT_TYPE_STACK,
	EVENT_TYPE_SCHED_SWITCH,
	EVENT_TYPE_WAKEUP,
	EVENT_TYPE_FUNC,
	EVENT_TYPE_SYSCALL,
	EVENT_TYPE_IRQ,
	EVENT_TYPE_SOFTIRQ,
	EVENT_TYPE_SOFTIRQ_RAISE,
	EVENT_TYPE_PROCESS_EXEC,
	EVENT_TYPE_USER_MATE,
};

struct event_data {
	struct trace_hash_item	hash;
	int			id;
	int			trace;
	struct tep_event	*event;

	struct event_data	*end;
	struct event_data	*start;

	struct tep_format_field	*pid_field;
	struct tep_format_field	*start_match_field;	/* match with start */
	struct tep_format_field	*end_match_field;	/* match with end */
	struct tep_format_field	*data_field;	/* optional */

	event_data_print	print_func;
	handle_event_func	handle_event;
	void			*private;
	int			migrate;	/* start/end pairs can migrate cpus */
	int			global;		/* use global tasks */
	enum event_data_type	type;
};

struct stack_data {
	struct trace_hash_item  hash;
	unsigned long long	count;
	unsigned long long	time;
	unsigned long long	time_min;
	unsigned long long	ts_min;
	unsigned long long	time_max;
	unsigned long long	ts_max;
	unsigned long long	time_avg;
	unsigned long		size;
	char			caller[];
};

struct stack_holder {
	unsigned long		size;
	void			*caller;
	struct tep_record	*record;
};

struct start_data {
	struct trace_hash_item	hash;
	struct event_data	*event_data;
	struct list_head	list;
	struct task_data	*task;
	unsigned long long 	timestamp;
	unsigned long long 	search_val;
	unsigned long long	val;
	int			cpu;

	struct stack_holder	stack;
};

struct event_hash {
	struct trace_hash_item	hash;
	struct event_data	*event_data;
	unsigned long long	search_val;
	unsigned long long	val;
	unsigned long long	count;
	unsigned long long	time_total;
	unsigned long long	time_avg;
	unsigned long long	time_max;
	unsigned long long	ts_max;
	unsigned long long	time_min;
	unsigned long long	ts_min;
	unsigned long long	time_std;
	unsigned long long	last_time;

	struct trace_hash	stacks;
};

struct group_data {
	struct trace_hash_item	hash;
	char			*comm;
	struct trace_hash	event_hash;
};

struct task_data {
	struct trace_hash_item	hash;
	int			pid;
	int			sleeping;

	char			*comm;

	struct trace_hash	start_hash;
	struct trace_hash	event_hash;

	struct task_data	*proxy;
	struct start_data	*last_start;
	struct event_hash	*last_event;
	struct tep_record	*last_stack;
	struct handle_data	*handle;
	struct group_data	*group;
};

struct cpu_info {
	int			current;
};

struct sched_switch_data {
	struct tep_format_field	*prev_state;
	int			match_state;
};

struct handle_data {
	struct handle_data	*next;
	struct tracecmd_input	*handle;
	struct tep_handle	*pevent;

	struct trace_hash	events;
	struct trace_hash	group_hash;

	struct cpu_info		**cpu_data;

	struct tep_format_field	*common_pid;
	struct tep_format_field	*wakeup_comm;
	struct tep_format_field	*switch_prev_comm;
	struct tep_format_field	*switch_next_comm;

	struct sched_switch_data sched_switch_blocked;
	struct sched_switch_data sched_switch_preempt;

	struct trace_hash	task_hash;
	struct list_head	*cpu_starts;
	struct list_head	migrate_starts;

	struct task_data	*global_task;
	struct task_data	*global_percpu_tasks;

	int			cpus;
};

static struct handle_data *handles;
static struct event_data *stacktrace_event;
static bool merge_like_comms = false;

void trace_profile_set_merge_like_comms(void)
{
	merge_like_comms = true;
}

static struct start_data *
add_start(struct task_data *task,
	  struct event_data *event_data, struct tep_record *record,
	  unsigned long long search_val, unsigned long long val)
{
	struct start_data *start;

	start = malloc(sizeof(*start));
	if (!start)
		return NULL;
	memset(start, 0, sizeof(*start));
	start->hash.key = trace_hash(search_val);
	start->search_val = search_val;
	start->val = val;
	start->timestamp = record->ts;
	start->event_data = event_data;
	start->cpu = record->cpu;
	start->task = task;
	trace_hash_add(&task->start_hash, &start->hash);
	if (event_data->migrate)
		list_add(&start->list, &task->handle->migrate_starts);
	else
		list_add(&start->list, &task->handle->cpu_starts[record->cpu]);
	return start;
}

struct event_data_match {
	struct event_data	*event_data;
	unsigned long long	search_val;
	unsigned long long	val;
};

static int match_start(struct trace_hash_item *item, void *data)
{
	struct start_data *start = start_from_item(item);
	struct event_data_match *edata = data;

	return start->event_data == edata->event_data &&
		start->search_val == edata->search_val;
}

static int match_event(struct trace_hash_item *item, void *data)
{
	struct event_data_match *edata = data;
	struct event_hash *event = event_from_item(item);

	return event->event_data == edata->event_data &&
		event->search_val == edata->search_val &&
		event->val == edata->val;
}

static struct event_hash *
find_event_hash(struct task_data *task, struct event_data_match *edata)
{
	struct event_hash *event_hash;
	struct trace_hash_item *item;
	unsigned long long key;

	key = (unsigned long)edata->event_data +
		(unsigned long)edata->search_val +
		(unsigned long)edata->val;
	key = trace_hash(key);
	item = trace_hash_find(&task->event_hash, key, match_event, edata);
	if (item)
		return event_from_item(item);

	event_hash = malloc(sizeof(*event_hash));
	if (!event_hash)
		return NULL;
	memset(event_hash, 0, sizeof(*event_hash));

	event_hash->event_data = edata->event_data;
	event_hash->search_val = edata->search_val;
	event_hash->val = edata->val;
	event_hash->hash.key = key;
	trace_hash_init(&event_hash->stacks, 32);

	trace_hash_add(&task->event_hash, &event_hash->hash);

	return event_hash;
}

static struct event_hash *
find_start_event_hash(struct task_data *task, struct event_data *event_data,
		      struct start_data *start)
{
	struct event_data_match edata;

	edata.event_data = event_data;
	edata.search_val = start->search_val;
	edata.val = start->val;

	return find_event_hash(task, &edata);
}

static struct start_data *
find_start(struct task_data *task, struct event_data *event_data,
	   unsigned long long search_val)
{
	unsigned long long key = trace_hash(search_val);
	struct event_data_match edata;
	void *data = &edata;
	struct trace_hash_item *item;
	struct start_data *start;

	edata.event_data = event_data;
	edata.search_val = search_val;

	item = trace_hash_find(&task->start_hash, key, match_start, data);
	if (!item)
		return NULL;

	start = start_from_item(item);
	return start;
}

struct stack_match {
	void		*caller;
	unsigned long	size;
};

static int match_stack(struct trace_hash_item *item, void *data)
{
	struct stack_data *stack = stack_from_item(item);
	struct stack_match *match = data;

	if (match->size != stack->size)
		return 0;

	return memcmp(stack->caller, match->caller, stack->size) == 0;
}


static void add_event_stack(struct event_hash *event_hash,
			    void *caller, unsigned long size,
			    unsigned long long time, unsigned long long ts)
{
	unsigned long long key;
	struct stack_data *stack;
	struct stack_match match;
	struct trace_hash_item *item;
	int i;

	match.caller = caller;
	match.size = size;

	if (size < sizeof(int))
		die("Stack size of less than sizeof(int)??");

	for (key = 0, i = 0; i <= size - sizeof(int); i += sizeof(int))
		key += trace_hash(*(int *)(caller + i));

	item = trace_hash_find(&event_hash->stacks, key, match_stack, &match);
	if (!item) {
		stack = malloc(sizeof(*stack) + size);
		if (!stack) {
			warning("Could not allocate stack");
			return;
		}
		memset(stack, 0, sizeof(*stack));
		memcpy(&stack->caller, caller, size);
		stack->size = size;
		stack->hash.key = key;
		trace_hash_add(&event_hash->stacks, &stack->hash);
	} else
		stack = stack_from_item(item);

	stack->count++;
	stack->time += time;
	if (stack->count == 1 || time < stack->time_min) {
		stack->time_min = time;
		stack->ts_min = ts;
	}
	if (time > stack->time_max) {
		stack->time_max = time;
		stack->ts_max = ts;
	}
}

static void free_start(struct start_data *start)
{
	if (start->task->last_start == start)
		start->task->last_start = NULL;
	if (start->stack.record)
		tracecmd_free_record(start->stack.record);
	trace_hash_del(&start->hash);
	list_del(&start->list);
	free(start);
}

static struct event_hash *
add_and_free_start(struct task_data *task, struct start_data *start,
		   struct event_data *event_data, unsigned long long ts)
{
	struct event_hash *event_hash;
	long long delta;

	delta = ts - start->timestamp;

	/*
	 * It's possible on a live trace, because of timestamps being
	 * different on different CPUs, we can go back in time. When
	 * that happens, just zero out the delta.
	 */
	if (delta < 0)
		delta = 0;

	event_hash = find_start_event_hash(task, event_data, start);
	if (!event_hash)
		return NULL;
	event_hash->count++;
	event_hash->time_total += delta;
	event_hash->last_time = delta;

	if (delta > event_hash->time_max) {
		event_hash->time_max = delta;
		event_hash->ts_max = ts;
	}

	if (event_hash->count == 1 || delta < event_hash->time_min) {
		event_hash->time_min = delta;
		event_hash->ts_min = ts;
	}

	if (start->stack.record) {
		unsigned long size;
		void *caller;

		size = start->stack.size;
		caller = start->stack.caller;

		add_event_stack(event_hash, caller, size, delta,
				start->stack.record->ts);
		tracecmd_free_record(start->stack.record);
		start->stack.record = NULL;
	}

	free_start(start);

	return event_hash;
}

static struct event_hash *
find_and_update_start(struct task_data *task, struct event_data *event_data,
		      unsigned long long ts, unsigned long long search_val)
{
	struct start_data *start;

	start = find_start(task, event_data, search_val);
	if (!start)
		return NULL;
	return add_and_free_start(task, start, event_data, ts);
}

static int match_task(struct trace_hash_item *item, void *data)
{
	struct task_data *task = task_from_item(item);
	int pid = *(unsigned long *)data;

	return task->pid == pid;
}

static void init_task(struct handle_data *h, struct task_data *task)
{
	task->handle = h;

	trace_hash_init(&task->start_hash, 16);
	trace_hash_init(&task->event_hash, 32);
}

static struct task_data *
add_task(struct handle_data *h, int pid)
{
	unsigned long long key = trace_hash(pid);
	struct task_data *task;

	task = malloc(sizeof(*task));
	if (!task) {
		warning("Could not allocate task");
		return NULL;
	}
	memset(task, 0, sizeof(*task));

	task->pid = pid;
	task->hash.key = key;
	trace_hash_add(&h->task_hash, &task->hash);

	init_task(h, task);

	return task;
}

static struct task_data *
find_task(struct handle_data *h, int pid)
{
	unsigned long long key = trace_hash(pid);
	struct trace_hash_item *item;
	static struct task_data *last_task;
	void *data = (unsigned long *)&pid;

	if (last_task && last_task->pid == pid)
		return last_task;

	item = trace_hash_find(&h->task_hash, key, match_task, data);

	if (item)
		last_task = task_from_item(item);
	else
		last_task = add_task(h, pid);

	return last_task;
}

static int match_group(struct trace_hash_item *item, void *data)
{
	struct group_data *group = group_from_item(item);

	return strcmp(group->comm, (char *)data) == 0;
}


static void
add_task_comm(struct task_data *task, struct tep_format_field *field,
	      struct tep_record *record)
{
	const char *comm;

	task->comm = malloc(field->size + 1);
	if (!task->comm) {
		warning("Could not allocate task comm");
		return;
	}
	comm = record->data + field->offset;
	memcpy(task->comm, comm, field->size);
	task->comm[field->size] = 0;
}

/* Account for tasks that don't have starts */
static void account_task(struct task_data *task, struct event_data *event_data,
			 struct tep_record *record)
{
	struct event_data_match edata;
	struct event_hash *event_hash;
	struct task_data *proxy = NULL;
	unsigned long long search_val = 0;
	unsigned long long val = 0;
	unsigned long long pid;

	/*
	 * If an event has the pid_field set, then find that task for
	 * this event instead. Let this task proxy for it to handle
	 * stack traces on this event.
	 */
	if (event_data->pid_field) {
		tep_read_number_field(event_data->pid_field,
				      record->data, &pid);
		proxy = task;
		task = find_task(task->handle, pid);
		if (!task)
			return;
		proxy->proxy = task;
	}

	/*
	 * If data_field is defined, use that for val,
	 * if the start_field is defined, use that for search_val.
	 */
	if (event_data->data_field) {
		tep_read_number_field(event_data->data_field,
				      record->data, &val);
	}
	if (event_data->start_match_field) {
		tep_read_number_field(event_data->start_match_field,
				      record->data, &search_val);
	}

	edata.event_data = event_data;
	edata.search_val = val;
	edata.val = val;

	event_hash = find_event_hash(task, &edata);
	if (!event_hash) {
		warning("failed to allocate event_hash");
		return;
	}

	event_hash->count++;
	task->last_event = event_hash;
}

static struct task_data *
find_event_task(struct handle_data *h, struct event_data *event_data,
		struct tep_record *record, unsigned long long pid)
{
	if (event_data->global) {
		if (event_data->migrate)
			return h->global_task;
		else
			return &h->global_percpu_tasks[record->cpu];
	}

	/* If pid_field is defined, use that to find the task */
	if (event_data->pid_field)
		tep_read_number_field(event_data->pid_field,
				      record->data, &pid);
	return find_task(h, pid);
}

static struct task_data *
handle_end_event(struct handle_data *h, struct event_data *event_data,
		 struct tep_record *record, int pid)
{
	struct event_hash *event_hash;
	struct task_data *task;
	unsigned long long val;

	task = find_event_task(h, event_data, record, pid);
	if (!task)
		return NULL;

	tep_read_number_field(event_data->start_match_field, record->data,
			      &val);
	event_hash = find_and_update_start(task, event_data->start, record->ts, val);
	task->last_start = NULL;
	task->last_event = event_hash;

	return task;
}

static struct task_data *
handle_start_event(struct handle_data *h, struct event_data *event_data,
		   struct tep_record *record, unsigned long long pid)
{
	struct start_data *start;
	struct task_data *task;
	unsigned long long val;

	task = find_event_task(h, event_data, record, pid);
	if (!task)
		return NULL;

	tep_read_number_field(event_data->end_match_field, record->data,
				 &val);
	start = add_start(task, event_data, record, val, val);
	if (!start) {
		warning("Failed to allocate start of task");
		return NULL;
	}
		
	task->last_start = start;
	task->last_event = NULL;

	return task;
}

static int handle_event_data(struct handle_data *h,
			     unsigned long long pid,
			     struct event_data *event_data,
			     struct tep_record *record, int cpu)
{
	struct task_data *task = NULL;

	/* If this is the end of a event pair (start is set) */
	if (event_data->start)
		task = handle_end_event(h, event_data, record, pid);

	/* If this is the start of a event pair (end is set) */
	if (event_data->end) {
		task = handle_start_event(h, event_data, record, pid);
		/* handle_start_event only returns NULL on error */
		if (!task)
			return -1;
	}

	if (!task) {
		task = find_task(h, pid);
		if (!task)
			return -1;
		task->proxy = NULL;
		task->last_start = NULL;
		task->last_event = NULL;
		account_task(task, event_data, record);
	}

	return 0;
}

static void handle_missed_events(struct handle_data *h, int cpu)
{
	struct start_data *start;
	struct start_data *n;

	/* Clear all starts on this CPU */
	list_for_each_entry_safe(start, n, &h->cpu_starts[cpu], list) {
		free_start(start);
	}

	/* Now clear all starts whose events can migrate */
	list_for_each_entry_safe(start, n, &h->migrate_starts, list) {
		free_start(start);
	}
}

static int match_event_data(struct trace_hash_item *item, void *data)
{
	struct event_data *event_data = event_data_from_item(item);
	int id = (int)(unsigned long)data;

	return event_data->id == id;
}

static struct event_data *
find_event_data(struct handle_data *h, int id)
{
	struct trace_hash_item *item;
	unsigned long long key = trace_hash(id);
	void *data = (void *)(unsigned long)id;

	item = trace_hash_find(&h->events, key, match_event_data, data);
	if (item)
		return event_data_from_item(item);
	return NULL;
}

static void trace_profile_record(struct tracecmd_input *handle,
				 struct tep_record *record)
{
	static struct handle_data *last_handle;
	struct tep_record *stack_record;
	struct event_data *event_data;
	struct task_data *task;
	struct handle_data *h;
	struct tep_handle *pevent;
	unsigned long long pid;
	int cpu = record->cpu;
	int id;

	if (last_handle && last_handle->handle == handle)
		h = last_handle;
	else {
		for (h = handles; h; h = h->next) {
			if (h->handle == handle)
				break;
		}
		if (!h)
			die("Handle not found?");
		last_handle = h;
	}

	if (record->missed_events)
		handle_missed_events(h, cpu);

	pevent = h->pevent;

	id = tep_data_type(pevent, record);

	event_data = find_event_data(h, id);

	if (!event_data)
		return;


	/* Get this current PID */
	tep_read_number_field(h->common_pid, record->data, &pid);

	task = find_task(h, pid);
	if (!task)
		return;
	stack_record = task->last_stack;

	if (event_data->handle_event)
		event_data->handle_event(h, pid, event_data, record, cpu);
	else
		handle_event_data(h, pid, event_data, record, cpu);

	/* If the last stack hasn't changed, free it */
	if (stack_record && task->last_stack == stack_record) {
		tracecmd_free_record(stack_record);
		task->last_stack = NULL;
	}
}

static struct event_data *
add_event(struct handle_data *h, const char *system, const char *event_name,
	  enum event_data_type type)
{
	struct event_data *event_data;
	struct tep_event *event;

	event = tep_find_event_by_name(h->pevent, system, event_name);
	if (!event)
		return NULL;

	if (!h->common_pid) {
		h->common_pid = tep_find_common_field(event, "common_pid");
		if (!h->common_pid)
			die("No 'common_pid' found in event");
	}

	event_data = malloc(sizeof(*event_data));
	if (!event_data) {
		warning("Could not allocate event_data");
		return NULL;
	}
	memset(event_data, 0, sizeof(*event_data));
	event_data->id = event->id;
	event_data->event = event;
	event_data->type = type;
	event_data->hash.key = trace_hash(event_data->event->id);

	trace_hash_add(&h->events, &event_data->hash);

	return event_data;
}

static void
mate_events(struct handle_data *h, struct event_data *start,
	    const char *pid_field, const char *end_match_field,
	    struct event_data *end, const char *start_match_field,
	    int migrate, int global)
{
	start->end = end;
	end->start = start;

	if (pid_field) {
		start->pid_field = tep_find_field(start->event, pid_field);
		if (!start->pid_field)
			die("Event: %s does not have field %s",
			    start->event->name, pid_field);
	}

	/* Field to match with end */
	start->end_match_field = tep_find_field(start->event, end_match_field);
	if (!start->end_match_field)
		die("Event: %s does not have field %s",
		    start->event->name, end_match_field);

	/* Field to match with start */
	end->start_match_field = tep_find_field(end->event, start_match_field);
	if (!end->start_match_field)
		die("Event: %s does not have field %s",
		    end->event->name, start_match_field);

	start->migrate = migrate;
	start->global = global;
	end->migrate = migrate;
	end->global = global;
}

/**
 * tracecmd_mate_events - match events to profile against
 * @handle: The input handle where the events exist.
 * @start_event: The event that starts the transaction
 * @pid_field: Use this over common_pid (may be NULL to use common_pid)
 * @end_match_field: The field that matches the end events @start_match_field
 * @end_event: The event that ends the transaction
 * @start_match_field: The end event field that matches start's @end_match_field
 * @migrate: Can the transaction switch CPUs? 1 for yes, 0 for no
 * @global: The events are global and not per task
 */
void tracecmd_mate_events(struct tracecmd_input *handle,
			  struct tep_event *start_event,
			  const char *pid_field, const char *end_match_field,
			  struct tep_event *end_event,
			  const char *start_match_field,
			  int migrate, int global)
{
	struct handle_data *h;
	struct event_data *start;
	struct event_data *end;

	for (h = handles; h; h = h->next) {
		if (h->handle == handle)
			break;
	}
	if (!h)
		die("Handle not found for trace profile");

	start = add_event(h, start_event->system, start_event->name,
			  EVENT_TYPE_USER_MATE);

	end = add_event(h, end_event->system, end_event->name,
			EVENT_TYPE_USER_MATE);

	if (!start || !end)
		return;

	mate_events(h, start, pid_field, end_match_field, end, start_match_field,
		    migrate, global);
}

static void func_print(struct trace_seq *s, struct event_hash *event_hash)
{
	const char *func;

	func = tep_find_function(event_hash->event_data->event->tep,
				 event_hash->val);
	if (func)
		trace_seq_printf(s, "func: %s()", func);
	else
		trace_seq_printf(s, "func: 0x%llx", event_hash->val);
}

static void syscall_print(struct trace_seq *s, struct event_hash *event_hash)
{
#ifndef NO_AUDIT
	const char *name = NULL;
	int machine;

	machine = audit_detect_machine();
	if (machine < 0)
		goto fail;
	name = audit_syscall_to_name(event_hash->val, machine);
	if (!name)
		goto fail;
	trace_seq_printf(s, "syscall:%s", name);
	return;
fail:
#endif
	trace_seq_printf(s, "%s:%d", event_hash->event_data->event->name,
			 (int)event_hash->val);
}

/* From Linux include/linux/interrupt.h */
#define SOFTIRQS				\
		C(HI),				\
		C(TIMER),			\
		C(NET_TX),			\
		C(NET_RX),			\
		C(BLOCK),			\
		C(BLOCK_IOPOLL),		\
		C(TASKLET),			\
		C(SCHED),			\
		C(HRTIMER),			\
		C(RCU),				\
		C(NR),

#undef C
#define C(a)	a##_SOFTIRQ

enum { SOFTIRQS };

#undef C
#define C(a)	#a

static const char *softirq_map[] = { SOFTIRQS };

static void softirq_print(struct trace_seq *s, struct event_hash *event_hash)
{
	int softirq = (int)event_hash->val;

	if (softirq < NR_SOFTIRQ)
		trace_seq_printf(s, "%s:%s", event_hash->event_data->event->name,
				 softirq_map[softirq]);
	else
		trace_seq_printf(s, "%s:%d", event_hash->event_data->event->name,
				 softirq);
}

static void sched_switch_print(struct trace_seq *s, struct event_hash *event_hash)
{
	const char states[] = TASK_STATE_TO_CHAR_STR;
	int i;

	trace_seq_printf(s, "%s:", event_hash->event_data->event->name);

	if (event_hash->val) {
		int val = event_hash->val;

		for (i = 0; val && i < sizeof(states) - 1; i++, val >>= 1) {
			if (val & 1)
				trace_seq_putc(s, states[i+1]);
		}
	} else
		trace_seq_putc(s, 'R');
}

static int handle_sched_switch_event(struct handle_data *h,
				     unsigned long long pid,
				     struct event_data *event_data,
				     struct tep_record *record, int cpu)
{
	struct task_data *task;
	unsigned long long prev_pid;
	unsigned long long prev_state;
	unsigned long long next_pid;
	struct start_data *start;

	/* pid_field holds prev_pid, data_field holds prev_state */
	tep_read_number_field(event_data->pid_field,
			      record->data, &prev_pid);

	tep_read_number_field(event_data->data_field,
				 record->data, &prev_state);

	/* only care about real states */
	prev_state &= TASK_STATE_MAX - 1;

	/* end_match_field holds next_pid */
	tep_read_number_field(event_data->end_match_field,
			      record->data, &next_pid);

	task = find_task(h, prev_pid);
	if (!task)
		return -1;
	if (!task->comm)
		add_task_comm(task, h->switch_prev_comm, record);

	if (prev_state)
		task->sleeping = 1;
	else
		task->sleeping = 0;

	/* task is being scheduled out. prev_state tells why */
	start = add_start(task, event_data, record, prev_pid, prev_state);
	task->last_start = start;
	task->last_event = NULL;

	task = find_task(h, next_pid);
	if (!task)
		return -1;

	if (!task->comm)
		add_task_comm(task, h->switch_next_comm, record);

	/*
	 * If the next task was blocked, it required a wakeup to
	 * restart, and there should be one.
	 * But if it was preempted, we look for the previous sched switch.
	 * Unfortunately, we have to look for both types of events as
	 * we do not know why next_pid scheduled out.
	 *
	 * event_data->start holds the sched_wakeup event data.
	 */
	find_and_update_start(task, event_data->start, record->ts, next_pid);

	/* Look for this task if it was preempted (no wakeup found). */
	find_and_update_start(task, event_data, record->ts, next_pid);

	return 0;
}

static int handle_stacktrace_event(struct handle_data *h,
				   unsigned long long pid,
				   struct event_data *event_data,
				   struct tep_record *record, int cpu)
{
	struct task_data *orig_task;
	struct task_data *proxy;
	struct task_data *task;
	unsigned long long size;
	struct event_hash *event_hash;
	struct start_data *start;
	void *caller;

	task = find_task(h, pid);
	if (!task)
		return -1;

	if (task->last_stack) {
		tracecmd_free_record(task->last_stack);
		task->last_stack = NULL;
	}

	if ((proxy = task->proxy)) {
		task->proxy = NULL;
		orig_task = task;
		task = proxy;
	}

	if (!task->last_start && !task->last_event) {
		/*
		 * Save this stack in case function graph needs it.
		 * Need the original task, not a proxy.
		 */
		if (proxy)
			task = orig_task;
		tracecmd_record_ref(record);
		task->last_stack = record;
		return 0;
	}

	/*
	 * start_match_field holds the size.
	 * data_field holds the caller location.
	 */
	size = record->size - event_data->data_field->offset;
	caller = record->data + event_data->data_field->offset;

	/*
	 * If there's a "start" then don't add the stack until
	 * it finds a matching "end".
	 */
	if ((start = task->last_start)) {
		tracecmd_record_ref(record);
		start->stack.record = record;
		start->stack.size = size;
		start->stack.caller = caller;
		task->last_start = NULL;
		task->last_event = NULL;
		return 0;
	}

	event_hash = task->last_event;
	task->last_event = NULL;

	add_event_stack(event_hash, caller, size, event_hash->last_time,
			record->ts);
	
	return 0;
}

static int handle_fgraph_entry_event(struct handle_data *h,
				    unsigned long long pid,
				    struct event_data *event_data,
				    struct tep_record *record, int cpu)
{
	unsigned long long size;
	struct start_data *start;
	struct task_data *task;
	void *caller;

	task = handle_start_event(h, event_data, record, pid);
	if (!task)
		return -1;

	/*
	 * If a stack trace hasn't been used for a previous task,
	 * then it could be a function trace that we can use for
	 * the function graph. But stack traces come before the function
	 * graph events (unfortunately). So we need to attach the previous
	 * stack trace (if there is one) to this start event.
	 */
	if (task->last_stack) {
		start = task->last_start;
		record = task->last_stack;
		size = record->size - stacktrace_event->data_field->offset;
		caller = record->data + stacktrace_event->data_field->offset;
		start->stack.record = record;
		start->stack.size = size;
		start->stack.caller = caller;
		task->last_stack = NULL;
		task->last_event = NULL;
	}

	/* Do not map stacks after this event to this event */
	task->last_start = NULL;

	return 0;
}

static int handle_fgraph_exit_event(struct handle_data *h,
				    unsigned long long pid,
				    struct event_data *event_data,
				    struct tep_record *record, int cpu)
{
	struct task_data *task;

	task = handle_end_event(h, event_data, record, pid);
	if (!task)
		return -1;
	/* Do not match stacks with function graph exit events */
	task->last_event = NULL;

	return 0;
}

static int handle_process_exec(struct handle_data *h,
			       unsigned long long pid,
			       struct event_data *event_data,
			       struct tep_record *record, int cpu)
{
	struct task_data *task;
	unsigned long long val;

	/* Task has execed, remove the comm for it */
	if (event_data->data_field) {
		tep_read_number_field(event_data->data_field,
				      record->data, &val);
		pid = val;
	}

	task = find_task(h, pid);
	if (!task)
		return -1;

	free(task->comm);
	task->comm = NULL;

	return 0;
}

static int handle_sched_wakeup_event(struct handle_data *h,
				     unsigned long long pid,
				     struct event_data *event_data,
				     struct tep_record *record, int cpu)
{
	struct task_data *proxy;
	struct task_data *task = NULL;
	struct start_data *start;
	unsigned long long success;

	proxy = find_task(h, pid);
	if (!proxy)
		return -1;

	/* If present, data_field holds "success" */
	if (event_data->data_field) {
		tep_read_number_field(event_data->data_field,
				      record->data, &success);

		/* If not a successful wakeup, ignore this */
		if (!success)
			return 0;
	}

	tep_read_number_field(event_data->pid_field,
			      record->data, &pid);

	task = find_task(h, pid);
	if (!task)
		return -1;

	if (!task->comm)
		add_task_comm(task, h->wakeup_comm, record);

	/* if the task isn't sleeping, then ignore the wake up */
	if (!task->sleeping) {
		/* Ignore any following stack traces */
		proxy->proxy = NULL;
		proxy->last_start = NULL;
		proxy->last_event = NULL;
		return 0;
	}

	/* It's being woken up */
	task->sleeping = 0;

	/*
	 * We need the stack trace to be hooked to the woken up
	 * task, not the waker.
	 */
	proxy->proxy = task;

	/* There should be a blocked schedule out of this task */
	find_and_update_start(task, event_data->start, record->ts, pid);

	/* Set this up for timing how long the wakeup takes */
	start = add_start(task, event_data, record, pid, pid);
	task->last_event = NULL;
	task->last_start = start;

	return 0;
}

void trace_init_profile(struct tracecmd_input *handle, struct hook_list *hook,
			int global)
{
	struct tep_handle *pevent = tracecmd_get_tep(handle);
	struct tep_format_field **fields;
	struct handle_data *h;
	struct event_data *event_data;
	struct event_data *sched_switch;
	struct event_data *sched_wakeup;
	struct event_data *irq_entry;
	struct event_data *irq_exit;
	struct event_data *softirq_entry;
	struct event_data *softirq_exit;
	struct event_data *softirq_raise;
	struct event_data *fgraph_entry;
	struct event_data *fgraph_exit;
	struct event_data *syscall_enter;
	struct event_data *syscall_exit;
	struct event_data *process_exec;
	struct event_data *start_event;
	struct event_data *end_event;
	struct tep_event **events;
	int ret;
	int i;

	tracecmd_set_show_data_func(handle, trace_profile_record);
	h = malloc(sizeof(*h));
	if (!h) {
		warning("Could not allocate handle");
		return;
	};
	memset(h, 0, sizeof(*h));
	h->next = handles;
	handles = h;

	trace_hash_init(&h->task_hash, 1024);
	trace_hash_init(&h->events, 1024);
	trace_hash_init(&h->group_hash, 512);

	h->handle = handle;
	h->pevent = pevent;

	h->cpus = tracecmd_cpus(handle);

	/*
	 * For streaming profiling, cpus will not be set up yet.
	 * In this case, we simply use the number of cpus on the
	 * system.
	 */
	if (!h->cpus)
		h->cpus = tracecmd_count_cpus();

	list_head_init(&h->migrate_starts);
	h->cpu_starts = malloc(sizeof(*h->cpu_starts) * h->cpus);
	if (!h->cpu_starts)
		goto free_handle;

	for (i = 0; i < h->cpus; i++)
		list_head_init(&h->cpu_starts[i]);

	h->cpu_data = malloc(h->cpus * sizeof(*h->cpu_data));
	if (!h->cpu_data)
		goto free_starts;

	memset(h->cpu_data, 0, h->cpus * sizeof(h->cpu_data));

	h->global_task = malloc(sizeof(struct task_data));
	if (!h->global_task)
		goto free_data;

	memset(h->global_task, 0, sizeof(struct task_data));
	init_task(h, h->global_task);
	h->global_task->comm = strdup("Global Events");
	if (!h->global_task->comm)
		die("malloc");
	h->global_task->pid = -1;

	h->global_percpu_tasks = calloc(h->cpus, sizeof(struct task_data));
	if (!h->global_percpu_tasks)
		die("malloc");
	for (i = 0; i < h->cpus; i++) {
		init_task(h, &h->global_percpu_tasks[i]);
		ret = asprintf(&h->global_percpu_tasks[i].comm,
			       "Global CPU[%d] Events", i);
		if (ret < 0)
			die("malloc");
		h->global_percpu_tasks[i].pid = -1 - i;
	}

	irq_entry = add_event(h, "irq", "irq_handler_entry", EVENT_TYPE_IRQ);
	irq_exit = add_event(h, "irq", "irq_handler_exit", EVENT_TYPE_IRQ);
	softirq_entry = add_event(h, "irq", "softirq_entry", EVENT_TYPE_SOFTIRQ);
	softirq_exit = add_event(h, "irq", "softirq_exit", EVENT_TYPE_SOFTIRQ);
	softirq_raise = add_event(h, "irq", "softirq_raise", EVENT_TYPE_SOFTIRQ_RAISE);
	sched_wakeup = add_event(h, "sched", "sched_wakeup", EVENT_TYPE_WAKEUP);
	sched_switch = add_event(h, "sched", "sched_switch", EVENT_TYPE_SCHED_SWITCH);
	fgraph_entry = add_event(h, "ftrace", "funcgraph_entry", EVENT_TYPE_FUNC);
	fgraph_exit = add_event(h, "ftrace", "funcgraph_exit", EVENT_TYPE_FUNC);
	syscall_enter = add_event(h, "raw_syscalls", "sys_enter", EVENT_TYPE_SYSCALL);
	syscall_exit = add_event(h, "raw_syscalls", "sys_exit", EVENT_TYPE_SYSCALL);

	process_exec = add_event(h, "sched", "sched_process_exec",
				 EVENT_TYPE_PROCESS_EXEC);

	stacktrace_event = add_event(h, "ftrace", "kernel_stack", EVENT_TYPE_STACK);
	if (stacktrace_event) {
		stacktrace_event->handle_event = handle_stacktrace_event;

		stacktrace_event->data_field = tep_find_field(stacktrace_event->event,
							    "caller");
		if (!stacktrace_event->data_field)
			die("Event: %s does not have field caller",
			    stacktrace_event->event->name);
	}

	if (process_exec) {
		process_exec->handle_event = handle_process_exec;
		process_exec->data_field = tep_find_field(process_exec->event,
							     "old_pid");
	}

	if (sched_switch) {
		sched_switch->handle_event = handle_sched_switch_event;
		sched_switch->data_field = tep_find_field(sched_switch->event,
							     "prev_state");
		if (!sched_switch->data_field)
			die("Event: %s does not have field prev_state",
			    sched_switch->event->name);

		h->switch_prev_comm = tep_find_field(sched_switch->event,
							"prev_comm");
		if (!h->switch_prev_comm)
			die("Event: %s does not have field prev_comm",
			    sched_switch->event->name);

		h->switch_next_comm = tep_find_field(sched_switch->event,
							"next_comm");
		if (!h->switch_next_comm)
			die("Event: %s does not have field next_comm",
			    sched_switch->event->name);

		sched_switch->print_func = sched_switch_print;
	}

	if (sched_switch && sched_wakeup) {
		mate_events(h, sched_switch, "prev_pid", "next_pid", 
			    sched_wakeup, "pid", 1, 0);
		mate_events(h, sched_wakeup, "pid", "pid",
			    sched_switch, "prev_pid", 1, 0);
		sched_wakeup->handle_event = handle_sched_wakeup_event;

		/* The 'success' field may or may not be present */
		sched_wakeup->data_field = tep_find_field(sched_wakeup->event,
							     "success");

		h->wakeup_comm = tep_find_field(sched_wakeup->event, "comm");
		if (!h->wakeup_comm)
			die("Event: %s does not have field comm",
			    sched_wakeup->event->name);
	}

	if (irq_entry && irq_exit)
		mate_events(h, irq_entry, NULL, "irq", irq_exit, "irq", 0, global);

	if (softirq_entry)
		softirq_entry->print_func = softirq_print;

	if (softirq_exit)
		softirq_exit->print_func = softirq_print;

	if (softirq_raise)
		softirq_raise->print_func = softirq_print;

	if (softirq_entry && softirq_exit)
		mate_events(h, softirq_entry, NULL, "vec", softirq_exit, "vec",
			    0, global);

	if (softirq_entry && softirq_raise)
		mate_events(h, softirq_raise, NULL, "vec", softirq_entry, "vec",
			    0, global);

	if (fgraph_entry && fgraph_exit) {
		mate_events(h, fgraph_entry, NULL, "func", fgraph_exit, "func", 1, 0);
		fgraph_entry->handle_event = handle_fgraph_entry_event;
		fgraph_exit->handle_event = handle_fgraph_exit_event;
		fgraph_entry->print_func = func_print;
	}

	if (syscall_enter && syscall_exit) {
		mate_events(h, syscall_enter, NULL, "id", syscall_exit, "id", 1, 0);
		syscall_enter->print_func = syscall_print;
		syscall_exit->print_func = syscall_print;
	}

	events = tep_list_events(pevent, TEP_EVENT_SORT_ID);
	if (!events)
		die("malloc");

	/* Add some other events */
	event_data = add_event(h, "ftrace", "function", EVENT_TYPE_FUNC);
	if (event_data) {
		event_data->data_field =
			tep_find_field(event_data->event, "ip");
	}

	/* Add any user defined hooks */
	for (; hook; hook = hook->next) {
		start_event = add_event(h, hook->start_system, hook->start_event,
					EVENT_TYPE_USER_MATE);
		end_event = add_event(h, hook->end_system, hook->end_event,
				      EVENT_TYPE_USER_MATE);
		if (!start_event) {
			warning("Event %s not found", hook->start_event);
			continue;
		}
		if (!end_event) {
			warning("Event %s not found", hook->end_event);
			continue;
		}
		mate_events(h, start_event, hook->pid, hook->start_match,
			    end_event, hook->end_match, hook->migrate,
			    hook->global);
	}

	/* Now add any defined event that we haven't processed */
	for (i = 0; events[i]; i++) {
		event_data = find_event_data(h, events[i]->id);
		if (event_data)
			continue;

		event_data = add_event(h, events[i]->system, events[i]->name,
				       EVENT_TYPE_UNDEFINED);

		fields = tep_event_fields(events[i]);
		if (!fields)
			die("malloc");

		if (fields[0])
			event_data->data_field = fields[0];

		free(fields);
	}
	return;

 free_data:
	free(h->cpu_data);
 free_starts:
	free(h->cpu_starts);
 free_handle:
	handles = h->next;
	free(h);
	warning("Failed handle allocations");
}

static void output_event_stack(struct tep_handle *pevent, struct stack_data *stack)
{
	int longsize = tep_get_long_size(pevent);
	unsigned long long val;
	const char *func;
	unsigned long long stop = -1ULL;
	void *ptr;
	int i;

	if (longsize < 8)
		stop &= (1ULL << (longsize * 8)) - 1;

	if (stack->count)
		stack->time_avg = stack->time / stack->count;

	printf("     <stack> %lld total:%lld min:%lld(ts:%lld.%06lld) max:%lld(ts:%lld.%06lld) avg=%lld\n",
	       stack->count, stack->time, stack->time_min,
	       nsecs_per_sec(stack->ts_min), mod_to_usec(stack->ts_min),
	       stack->time_max,
	       nsecs_per_sec(stack->ts_max), mod_to_usec(stack->ts_max),
	       stack->time_avg);

	for (i = 0; i < stack->size; i += longsize) {
		ptr = stack->caller + i;
		switch (longsize) {
		case 4:
			/* todo, read value from pevent */
			val = *(unsigned int *)ptr;
			break;
		case 8:
			val = *(unsigned long long *)ptr;
			break;
		default:
			die("Strange long size %d", longsize);
		}
		if (val == stop)
			break;
		func = tep_find_function(pevent, val);
		if (func)
			printf("       => %s (0x%llx)\n", func, val);
		else
			printf("       => 0x%llx\n", val);
	}
}

struct stack_chain {
	struct stack_chain *children;
	unsigned long long	val;
	unsigned long long	time;
	unsigned long long	time_min;
	unsigned long long	ts_min;
	unsigned long long	time_max;
	unsigned long long	ts_max;
	unsigned long long	time_avg;
	unsigned long long	count;
	int			percent;
	int			nr_children;
};

static int compare_chains(const void *a, const void *b)
{
	const struct stack_chain * A = a;
	const struct stack_chain * B = b;

	if (A->time > B->time)
		return -1;
	if (A->time < B->time)
		return 1;
	/* If stacks don't use time, then use count */
	if (A->count > B->count)
		return -1;
	if (A->count < B->count)
		return 1;
	return 0;
}

static int calc_percent(unsigned long long val, unsigned long long total)
{
	return (val * 100 + total / 2) / total;
}

static int stack_overflows(struct stack_data *stack, int longsize, int level)
{
	return longsize * level > stack->size - longsize;
}

static unsigned long long
stack_value(struct stack_data *stack, int longsize, int level)
{
	void *ptr;

	ptr = &stack->caller[longsize * level];
	return longsize == 8 ? *(u64 *)ptr : *(unsigned *)ptr;
}

static struct stack_chain *
make_stack_chain(struct stack_data **stacks, int cnt, int longsize, int level,
		 int *nr_children)
{
	struct stack_chain *chain;
	unsigned long long	total_time = 0;
	unsigned long long	total_count = 0;
	unsigned long long	time;
	unsigned long long	time_min;
	unsigned long long	ts_min;
	unsigned long long	time_max;
	unsigned long long	ts_max;
	unsigned long long	count;
	unsigned long long	stop = -1ULL;
	int nr_chains = 0;
	u64 last = 0;
	u64 val;
	int start;
	int i;
	int x;

	if (longsize < 8)
		stop &= (1ULL << (longsize * 8)) - 1;

	/* First find out how many diffs there are */
	for (i = 0; i < cnt; i++) {
		if (stack_overflows(stacks[i], longsize, level))
			continue;

		val = stack_value(stacks[i], longsize, level);

		if (val == stop)
			continue;

		if (!nr_chains || val != last)
			nr_chains++;
		last = val;
	}

	if (!nr_chains) {
		*nr_children = 0;
		return NULL;
	}

	chain = malloc(sizeof(*chain) * nr_chains);
	if (!chain) {
		warning("Could not allocate chain");
		return NULL;
	}
	memset(chain, 0, sizeof(*chain) * nr_chains);

	x = 0;
	count = 0;
	start = 0;
	time = 0;
	time_min = 0;
	time_max = 0;

	for (i = 0; i < cnt; i++) {
		if (stack_overflows(stacks[i], longsize, level)) {
			start = i+1;
			continue;
		}

		val = stack_value(stacks[i], longsize, level);

		if (val == stop) {
			start = i+1;
			continue;
		}

		count += stacks[i]->count;
		time += stacks[i]->time;
		if (stacks[i]->time_max > time_max) {
			time_max = stacks[i]->time_max;
			ts_max = stacks[i]->ts_max;
		}
		if (i == start || stacks[i]->time_min < time_min) {
			time_min = stacks[i]->time_min;
			ts_min = stacks[i]->ts_min;
		}
		if (i == cnt - 1 ||
		    stack_overflows(stacks[i+1], longsize, level) ||
		    val != stack_value(stacks[i+1], longsize, level)) {

			total_time += time;
			total_count += count;
			chain[x].val = val;
			chain[x].time_avg = time / count;
			chain[x].count = count;
			chain[x].time = time;
			chain[x].time_min = time_min;
			chain[x].ts_min = ts_min;
			chain[x].time_max = time_max;
			chain[x].ts_max = ts_max;
			chain[x].children =
				make_stack_chain(&stacks[start], (i - start) + 1,
						 longsize, level+1,
						 &chain[x].nr_children);
			x++;
			start = i + 1;
			count = 0;
			time = 0;
			time_min = 0;
			time_max = 0;
		}
	}

	qsort(chain, nr_chains, sizeof(*chain), compare_chains);

	*nr_children = nr_chains;

	/* Should never happen */
	if (!total_time && !total_count)
		return chain;


	/* Now calculate percentage */
	time = 0;
	for (i = 0; i < nr_chains; i++) {
		if (total_time)
			chain[i].percent = calc_percent(chain[i].time, total_time);
		/* In case stacks don't have time */
		else if (total_count)
			chain[i].percent = calc_percent(chain[i].count, total_count);
	}

	return chain;
}

static void free_chain(struct stack_chain *chain, int nr_chains)
{
	int i;

	if (!chain)
		return;

	for (i = 0; i < nr_chains; i++)
		free_chain(chain[i].children, chain[i].nr_children);

	free(chain);
}

#define INDENT	5

static void print_indent(int level, unsigned long long mask)
{
	char line;
	int p;

	for (p = 0; p < level + 1; p++) {
		if (mask & (1ULL << p))
			line = '|';
		else
			line = ' ';
		printf("%*c ", INDENT, line);
	}
}

static void print_chain_func(struct tep_handle *pevent, struct stack_chain *chain)
{
	unsigned long long val = chain->val;
	const char *func;

	func = tep_find_function(pevent, val);
	if (func)
		printf("%s (0x%llx)\n", func, val);
	else
		printf("0x%llx\n", val);
}

static void output_chain(struct tep_handle *pevent, struct stack_chain *chain, int level,
			 int nr_chains, unsigned long long *mask)
{
	struct stack_chain *child;
	int nr_children;
	int i;
	char line = '|';

	if (!nr_chains)
		return;

	*mask |= (1ULL << (level + 1));
	print_indent(level + 1, *mask);
	printf("\n");

	for (i = 0; i < nr_chains; i++) {

		print_indent(level, *mask);

		printf("%*c ", INDENT, '+');

		if (i == nr_chains - 1) {
			*mask &= ~(1ULL << (level + 1));
			line = ' ';
		}

		print_chain_func(pevent, &chain[i]);

		print_indent(level, *mask);

		printf("%*c ", INDENT, line);
		printf("  %d%% (%lld)", chain[i].percent, chain[i].count);
		if (chain[i].time)
			printf(" time:%lld max:%lld(ts:%lld.%06lld) min:%lld(ts:%lld.%06lld) avg:%lld",
			       chain[i].time, chain[i].time_max,
			       nsecs_per_sec(chain[i].ts_max),
			       mod_to_usec(chain[i].ts_max),
			       chain[i].time_min,
			       nsecs_per_sec(chain[i].ts_min),
			       mod_to_usec(chain[i].ts_min),
			       chain[i].time_avg);
		printf("\n");

		for (child = chain[i].children, nr_children = chain[i].nr_children;
		     child && nr_children == 1;
		     nr_children = child->nr_children, child = child->children) {
			print_indent(level, *mask);
			printf("%*c ", INDENT, line);
			printf("   ");
			print_chain_func(pevent, child);
		}

		if (child)
			output_chain(pevent, child, level+1, nr_children, mask);

		print_indent(level + 1, *mask);
		printf("\n");
	}
	*mask &= ~(1ULL << (level + 1));
	print_indent(level, *mask);
	printf("\n");
}

static int compare_stacks(const void *a, const void *b)
{
	struct stack_data * const *A = a;
	struct stack_data * const *B = b;
	unsigned int sa, sb;
	int size;
	int i;

	/* only compare up to the smaller size of the two */
	if ((*A)->size > (*B)->size)
		size = (*B)->size;
	else
		size = (*A)->size;

	for (i = 0; i < size; i += sizeof(sa)) {
		sa = *(unsigned *)&(*A)->caller[i];
		sb = *(unsigned *)&(*B)->caller[i];
		if (sa > sb)
			return 1;
		if (sa < sb)
			return -1;
	}

	/* They are the same up to size. Then bigger size wins */
	if ((*A)->size > (*B)->size)
		return 1;
	if ((*A)->size < (*B)->size)
		return -1;
	return 0;
}

static void output_stacks(struct tep_handle *pevent, struct trace_hash *stack_hash)
{
	struct trace_hash_item **bucket;
	struct trace_hash_item *item;
	struct stack_data **stacks;
	struct stack_chain *chain;
	unsigned long long mask = 0;
	int nr_chains;
	int longsize = tep_get_long_size(pevent);
	int nr_stacks;
	int i;

	nr_stacks = 0;
	trace_hash_for_each_bucket(bucket, stack_hash) {
		trace_hash_for_each_item(item, bucket) {
			nr_stacks++;
		}
	}

	stacks = malloc(sizeof(*stacks) * nr_stacks);
	if (!stacks) {
		warning("Could not allocate stacks");
		return;
	}

	nr_stacks = 0;
	trace_hash_for_each_bucket(bucket, stack_hash) {
		trace_hash_for_each_item(item, bucket) {
			stacks[nr_stacks++] = stack_from_item(item);
		}
	}

	qsort(stacks, nr_stacks, sizeof(*stacks), compare_stacks);

	chain = make_stack_chain(stacks, nr_stacks, longsize, 0, &nr_chains);

	output_chain(pevent, chain, 0, nr_chains, &mask);

	if (0)
		for (i = 0; i < nr_stacks; i++)
			output_event_stack(pevent, stacks[i]);

	free(stacks);
	free_chain(chain, nr_chains);
}

static void output_event(struct event_hash *event_hash)
{
	struct event_data *event_data = event_hash->event_data;
	struct tep_handle *pevent = event_data->event->tep;
	struct trace_seq s;

	trace_seq_init(&s);

	if (event_data->print_func)
		event_data->print_func(&s, event_hash);
	else if (event_data->type == EVENT_TYPE_FUNC)
		func_print(&s, event_hash);
	else
		trace_seq_printf(&s, "%s:0x%llx",
				 event_data->event->name,
				 event_hash->val);
	trace_seq_terminate(&s);

	printf("  Event: %s (%lld)",
	       s.buffer, event_hash->count);

	trace_seq_destroy(&s);

	if (event_hash->time_total) {
		event_hash->time_avg = event_hash->time_total / event_hash->count;
		printf(" Total: %lld Avg: %lld Max: %lld(ts:%lld.%06lld) Min:%lld(ts:%lld.%06lld)",
		       event_hash->time_total, event_hash->time_avg,
		       event_hash->time_max,
		       nsecs_per_sec(event_hash->ts_max),
		       mod_to_usec(event_hash->ts_max),
		       event_hash->time_min,
		       nsecs_per_sec(event_hash->ts_min),
		       mod_to_usec(event_hash->ts_min));
	}
	printf("\n");

	output_stacks(pevent, &event_hash->stacks);
}

static int compare_events(const void *a, const void *b)
{
	struct event_hash * const *A = a;
	struct event_hash * const *B = b;
	const struct event_data *event_data_a = (*A)->event_data;
	const struct event_data *event_data_b = (*B)->event_data;

	/* Schedule switch goes first */
	if (event_data_a->type == EVENT_TYPE_SCHED_SWITCH) {
		if (event_data_b->type != EVENT_TYPE_SCHED_SWITCH)
			return -1;
		/* lower the state the better */
		if ((*A)->val > (*B)->val)
			return 1;
		if ((*A)->val < (*B)->val)
			return -1;
		return 0;
	} else if (event_data_b->type == EVENT_TYPE_SCHED_SWITCH)
			return 1;

	/* Wakeups are next */
	if (event_data_a->type == EVENT_TYPE_WAKEUP) {
		if (event_data_b->type != EVENT_TYPE_WAKEUP)
			return -1;
		return 0;
	} else if (event_data_b->type == EVENT_TYPE_WAKEUP)
		return 1;

	if (event_data_a->id > event_data_b->id)
		return 1;
	if (event_data_a->id < event_data_b->id)
		return -1;
	if ((*A)->time_total > (*B)->time_total)
		return -1;
	if ((*A)->time_total < (*B)->time_total)
		return 1;
	return 0;
}

static void output_task(struct handle_data *h, struct task_data *task)
{
	struct trace_hash_item **bucket;
	struct trace_hash_item *item;
	struct event_hash **events;
	const char *comm;
	int nr_events = 0;
	int i;

	if (task->group)
		return;

	if (task->comm)
		comm = task->comm;
	else
		comm = tep_data_comm_from_pid(h->pevent, task->pid);

	if (task->pid < 0)
		printf("%s\n", task->comm);
	else
		printf("\ntask: %s-%d\n", comm, task->pid);

	trace_hash_for_each_bucket(bucket, &task->event_hash) {
		trace_hash_for_each_item(item, bucket) {
			nr_events++;
		}
	}

	events = malloc(sizeof(*events) * nr_events);
	if (!events) {
		warning("Could not allocate events");
		return;
	}

	i = 0;
	trace_hash_for_each_bucket(bucket, &task->event_hash) {
		trace_hash_for_each_item(item, bucket) {
			events[i++] = event_from_item(item);
		}
	}

	qsort(events, nr_events, sizeof(*events), compare_events);

	for (i = 0; i < nr_events; i++)
		output_event(events[i]);

	free(events);
}

static void output_group(struct handle_data *h, struct group_data *group)
{
	struct trace_hash_item **bucket;
	struct trace_hash_item *item;
	struct event_hash **events;
	int nr_events = 0;
	int i;

	printf("\ngroup: %s\n", group->comm);

	trace_hash_for_each_bucket(bucket, &group->event_hash) {
		trace_hash_for_each_item(item, bucket) {
			nr_events++;
		}
	}

	events = malloc(sizeof(*events) * nr_events);
	if (!events) {
		warning("Could not allocate events");
		return;
	}

	i = 0;
	trace_hash_for_each_bucket(bucket, &group->event_hash) {
		trace_hash_for_each_item(item, bucket) {
			events[i++] = event_from_item(item);
		}
	}

	qsort(events, nr_events, sizeof(*events), compare_events);

	for (i = 0; i < nr_events; i++)
		output_event(events[i]);

	free(events);
}

static int compare_tasks(const void *a, const void *b)
{
	struct task_data * const *A = a;
	struct task_data * const *B = b;

	if ((*A)->pid > (*B)->pid)
		return 1;
	else if ((*A)->pid < (*B)->pid)
		return -1;
	return 0;
}

static int compare_groups(const void *a, const void *b)
{
	const char *A = a;
	const char *B = b;

	return strcmp(A, B);
}

static void free_event_hash(struct event_hash *event_hash)
{
	struct trace_hash_item **bucket;
	struct trace_hash_item *item;
	struct stack_data *stack;

	trace_hash_for_each_bucket(bucket, &event_hash->stacks) {
		trace_hash_while_item(item, bucket) {
			stack = stack_from_item(item);
			trace_hash_del(&stack->hash);
			free(stack);
		}
	}
	trace_hash_free(&event_hash->stacks);
	free(event_hash);
}

static void __free_task(struct task_data *task)
{
	struct trace_hash_item **bucket;
	struct trace_hash_item *item;
	struct start_data *start;
	struct event_hash *event_hash;

	free(task->comm);

	trace_hash_for_each_bucket(bucket, &task->start_hash) {
		trace_hash_while_item(item, bucket) {
			start = start_from_item(item);
			if (start->stack.record)
				tracecmd_free_record(start->stack.record);
			list_del(&start->list);
			trace_hash_del(item);
			free(start);
		}
	}
	trace_hash_free(&task->start_hash);

	trace_hash_for_each_bucket(bucket, &task->event_hash) {
		trace_hash_while_item(item, bucket) {
			event_hash = event_from_item(item);
			trace_hash_del(item);
			free_event_hash(event_hash);
		}
	}
	trace_hash_free(&task->event_hash);

	if (task->last_stack)
		tracecmd_free_record(task->last_stack);
}

static void free_task(struct task_data *task)
{
	__free_task(task);
	free(task);
}

static void free_group(struct group_data *group)
{
	struct trace_hash_item **bucket;
	struct trace_hash_item *item;
	struct event_hash *event_hash;

	free(group->comm);

	trace_hash_for_each_bucket(bucket, &group->event_hash) {
		trace_hash_while_item(item, bucket) {
			event_hash = event_from_item(item);
			trace_hash_del(item);
			free_event_hash(event_hash);
		}
	}
	trace_hash_free(&group->event_hash);
	free(group);
}

static void show_global_task(struct handle_data *h,
			     struct task_data *task)
{
	if (trace_hash_empty(&task->event_hash))
		return;

	output_task(h, task);
}

static void output_tasks(struct handle_data *h)
{
	struct trace_hash_item **bucket;
	struct trace_hash_item *item;
	struct task_data **tasks;
	int nr_tasks = 0;
	int i;

	trace_hash_for_each_bucket(bucket, &h->task_hash) {
		trace_hash_for_each_item(item, bucket) {
			nr_tasks++;
		}
	}

	tasks = malloc(sizeof(*tasks) * nr_tasks);
	if (!tasks) {
		warning("Could not allocate tasks");
		return;
	}

	nr_tasks = 0;

	trace_hash_for_each_bucket(bucket, &h->task_hash) {
		trace_hash_while_item(item, bucket) {
			tasks[nr_tasks++] = task_from_item(item);
			trace_hash_del(item);
		}
	}

	qsort(tasks, nr_tasks, sizeof(*tasks), compare_tasks);

	for (i = 0; i < nr_tasks; i++) {
		output_task(h, tasks[i]);
		free_task(tasks[i]);
	}

	free(tasks);
}

static void output_groups(struct handle_data *h)
{
	struct trace_hash_item **bucket;
	struct trace_hash_item *item;
	struct group_data **groups;
	int nr_groups = 0;
	int i;

	trace_hash_for_each_bucket(bucket, &h->group_hash) {
		trace_hash_for_each_item(item, bucket) {
			nr_groups++;
		}
	}

	if (nr_groups == 0)
		return;

	groups = malloc(sizeof(*groups) * nr_groups);
	if (!groups) {
		warning("Could not allocate groups");
		return;
	}

	nr_groups = 0;

	trace_hash_for_each_bucket(bucket, &h->group_hash) {
		trace_hash_while_item(item, bucket) {
			groups[nr_groups++] = group_from_item(item);
			trace_hash_del(item);
		}
	}

	qsort(groups, nr_groups, sizeof(*groups), compare_groups);

	for (i = 0; i < nr_groups; i++) {
		output_group(h, groups[i]);
		free_group(groups[i]);
	}

	free(groups);
}

static void output_handle(struct handle_data *h)
{
	int i;

	show_global_task(h, h->global_task);
	for (i = 0; i < h->cpus; i++)
		show_global_task(h, &h->global_percpu_tasks[i]);

	output_groups(h);
	output_tasks(h);
}

static void merge_event_stack(struct event_hash *event,
			      struct stack_data *stack)
{
	struct stack_data *exist;
	struct trace_hash_item *item;
	struct stack_match match;

	match.caller = stack->caller;
	match.size = stack->size;
	item = trace_hash_find(&event->stacks, stack->hash.key, match_stack,
			       &match);
	if (!item) {
		trace_hash_add(&event->stacks, &stack->hash);
		return;
	}
	exist = stack_from_item(item);
	exist->count += stack->count;
	exist->time += stack->time;

	if (exist->time_max < stack->time_max) {
		exist->time_max = stack->time_max;
		exist->ts_max = stack->ts_max;
	}
	if (exist->time_min > stack->time_min) {
		exist->time_min = stack->time_min;
		exist->ts_min = stack->ts_min;
	}
	free(stack);
}

static void merge_stacks(struct event_hash *exist, struct event_hash *event)
{
	struct stack_data *stack;
	struct trace_hash_item *item;
	struct trace_hash_item **bucket;

	trace_hash_for_each_bucket(bucket, &event->stacks) {
		trace_hash_while_item(item, bucket) {
			stack = stack_from_item(item);
			trace_hash_del(&stack->hash);
			merge_event_stack(exist, stack);
		}
	}
}

static void merge_event_into_group(struct group_data *group,
				   struct event_hash *event)
{
	struct event_hash *exist;
	struct trace_hash_item *item;
	struct event_data_match edata;
	unsigned long long key;

	if (event->event_data->type == EVENT_TYPE_WAKEUP) {
		edata.event_data = event->event_data;
		event->search_val = 0;
		event->val = 0;
		key = trace_hash((unsigned long)event->event_data);
	} else if (event->event_data->type == EVENT_TYPE_SCHED_SWITCH) {
		edata.event_data = event->event_data;
		event->search_val = event->val;
		key = (unsigned long)event->event_data +
			((unsigned long)event->val * 2);
		key = trace_hash(key);
	} else {
		key = event->hash.key;
	}

	edata.event_data = event->event_data;
	edata.search_val = event->search_val;
	edata.val = event->val;

	item = trace_hash_find(&group->event_hash, key, match_event, &edata);
	if (!item) {
		event->hash.key = key;
		trace_hash_add(&group->event_hash, &event->hash);
		return;
	}

	exist = event_from_item(item);
	exist->count += event->count;
	exist->time_total += event->time_total;

	if (exist->time_max < event->time_max) {
		exist->time_max = event->time_max;
		exist->ts_max = event->ts_max;
	}
	if (exist->time_min > event->time_min) {
		exist->time_min = event->time_min;
		exist->ts_min = event->ts_min;
	}

	merge_stacks(exist, event);
	free_event_hash(event);
}

static void add_group(struct handle_data *h, struct task_data *task)
{
	unsigned long long key;
	struct trace_hash_item *item;
	struct group_data *grp;
	struct trace_hash_item **bucket;
	void *data = task->comm;

	if (!task->comm)
		return;

	key = trace_hash_str(task->comm);

	item = trace_hash_find(&h->group_hash, key, match_group, data);
	if (item) {
		grp = group_from_item(item);
	} else {
		grp = malloc(sizeof(*grp));
		if (!grp) {
			warning("Could not allocate group");
			return;
		}
		memset(grp, 0, sizeof(*grp));

		grp->comm = strdup(task->comm);
		if (!grp->comm)
			die("strdup");
		grp->hash.key = key;
		trace_hash_add(&h->group_hash, &grp->hash);
		trace_hash_init(&grp->event_hash, 32);
	}
	task->group = grp;

	trace_hash_for_each_bucket(bucket, &task->event_hash) {
		trace_hash_while_item(item, bucket) {
			struct event_hash *event_hash;

			event_hash = event_from_item(item);
			trace_hash_del(&event_hash->hash);
			merge_event_into_group(grp, event_hash);
		}
	}
}

static void merge_tasks(struct handle_data *h)
{
	struct trace_hash_item **bucket;
	struct trace_hash_item *item;

	if (!merge_like_comms)
		return;

	trace_hash_for_each_bucket(bucket, &h->task_hash) {
		trace_hash_for_each_item(item, bucket)
			add_group(h, task_from_item(item));
	}
}

int do_trace_profile(void)
{
	struct handle_data *h;

	for (h = handles; h; h = h->next) {
		if (merge_like_comms)
			merge_tasks(h);
		output_handle(h);
		trace_hash_free(&h->task_hash);
	}

	return 0;
}
