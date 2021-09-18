// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

 /**
 *  @file    libkshark.c
 *  @brief   API for processing of FTRACE (trace-cmd) data.
 */

/** Use GNU C Library. */
#define _GNU_SOURCE 1

// C
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// KernelShark
#include "libkshark.h"

static __thread struct trace_seq seq;

static struct kshark_context *kshark_context_handler = NULL;

static bool kshark_default_context(struct kshark_context **context)
{
	struct kshark_context *kshark_ctx;

	kshark_ctx = calloc(1, sizeof(*kshark_ctx));
	if (!kshark_ctx)
		return false;

	kshark_ctx->event_handlers = NULL;
	kshark_ctx->collections = NULL;
	kshark_ctx->plugins = NULL;

	kshark_ctx->show_task_filter = tracecmd_filter_id_hash_alloc();
	kshark_ctx->hide_task_filter = tracecmd_filter_id_hash_alloc();

	kshark_ctx->show_event_filter = tracecmd_filter_id_hash_alloc();
	kshark_ctx->hide_event_filter = tracecmd_filter_id_hash_alloc();

	kshark_ctx->show_cpu_filter = tracecmd_filter_id_hash_alloc();
	kshark_ctx->hide_cpu_filter = tracecmd_filter_id_hash_alloc();

	kshark_ctx->filter_mask = 0x0;

	/* Will free kshark_context_handler. */
	kshark_free(NULL);

	/* Will do nothing if *context is NULL. */
	kshark_free(*context);

	*context = kshark_context_handler = kshark_ctx;

	return true;
}

static bool init_thread_seq(void)
{
	if (!seq.buffer)
		trace_seq_init(&seq);

	return seq.buffer != NULL;
}

/**
 * @brief Initialize a kshark session. This function must be called before
 *	  calling any other kshark function. If the session has been
 *	  initialized, this function can be used to obtain the session's
 *	  context.
 *
 * @param kshark_ctx: Optional input/output location for context pointer.
 *		      If it points to a context, that context will become
 *		      the new session. If it points to NULL, it will obtain
 *		      the current (or new) session. The result is only
 *		      valid on return of true.
 *
 * @returns True on success, or false on failure.
 */
bool kshark_instance(struct kshark_context **kshark_ctx)
{
	if (*kshark_ctx != NULL) {
		/* Will free kshark_context_handler */
		kshark_free(NULL);

		/* Use the context provided by the user. */
		kshark_context_handler = *kshark_ctx;
	} else {
		if (kshark_context_handler) {
			/*
			 * No context is provided by the user, but the context
			 * handler is already set. Use the context handler.
			 */
			*kshark_ctx = kshark_context_handler;
		} else {
			/* No kshark_context exists. Create a default one. */
			if (!kshark_default_context(kshark_ctx))
				return false;
		}
	}

	if (!init_thread_seq())
		return false;

	return true;
}

static void kshark_free_task_list(struct kshark_context *kshark_ctx)
{
	struct kshark_task_list *task;
	int i;

	if (!kshark_ctx)
		return;

	for (i = 0; i < KS_TASK_HASH_SIZE; ++i) {
		while (kshark_ctx->tasks[i]) {
			task = kshark_ctx->tasks[i];
			kshark_ctx->tasks[i] = task->next;
			free(task);
		}
	}
}

/**
 * @brief Open and prepare for reading a trace data file specified by "file".
 *	  If the specified file does not exist, or contains no trace data,
 *	  the function returns false.
 *
 * @param kshark_ctx: Input location for context pointer.
 * @param file: The file to load.
 *
 * @returns True on success, or false on failure.
 */
bool kshark_open(struct kshark_context *kshark_ctx, const char *file)
{
	struct tracecmd_input *handle;

	kshark_free_task_list(kshark_ctx);

	handle = tracecmd_open_head(file, 0);
	if (!handle)
		return false;

	/* Read the tracing data from the file. */
	if (tracecmd_init_data(handle) < 0)
		return false;

	if (pthread_mutex_init(&kshark_ctx->input_mutex, NULL) != 0) {
		tracecmd_close(handle);
		return false;
	}

	kshark_ctx->handle = handle;
	kshark_ctx->pevent = tracecmd_get_tep(handle);

	kshark_ctx->advanced_event_filter =
		tep_filter_alloc(kshark_ctx->pevent);

	/*
	 * Turn off function trace indent and turn on show parent
	 * if possible.
	 */
	tep_plugin_add_option("ftrace:parent", "1");
	tep_plugin_add_option("ftrace:indent", "0");

	return true;
}

/**
 * @brief Close the trace data file and free the trace data handle.
 *
 * @param kshark_ctx: Input location for the session context pointer.
 */
void kshark_close(struct kshark_context *kshark_ctx)
{
	if (!kshark_ctx || !kshark_ctx->handle)
		return;

	/*
	 * All filters are file specific. Make sure that the Pids and Event Ids
	 * from this file are not going to be used with another file.
	 */
	tracecmd_filter_id_clear(kshark_ctx->show_task_filter);
	tracecmd_filter_id_clear(kshark_ctx->hide_task_filter);
	tracecmd_filter_id_clear(kshark_ctx->show_event_filter);
	tracecmd_filter_id_clear(kshark_ctx->hide_event_filter);
	tracecmd_filter_id_clear(kshark_ctx->show_cpu_filter);
	tracecmd_filter_id_clear(kshark_ctx->hide_cpu_filter);

	if (kshark_ctx->advanced_event_filter) {
		tep_filter_reset(kshark_ctx->advanced_event_filter);
		tep_filter_free(kshark_ctx->advanced_event_filter);
		kshark_ctx->advanced_event_filter = NULL;
	}

	/*
	 * All data collections are file specific. Make sure that collections
	 * from this file are not going to be used with another file.
	 */
	kshark_free_collection_list(kshark_ctx->collections);
	kshark_ctx->collections = NULL;

	tracecmd_close(kshark_ctx->handle);
	kshark_ctx->handle = NULL;
	kshark_ctx->pevent = NULL;

	pthread_mutex_destroy(&kshark_ctx->input_mutex);
}

/**
 * @brief Deinitialize kshark session. Should be called after closing all
 *	  open trace data files and before your application terminates.
 *
 * @param kshark_ctx: Optional input location for session context pointer.
 *		      If it points to a context of a sessuin, that sessuin
 *		      will be deinitialize. If it points to NULL, it will
 *		      deinitialize the current session.
 */
void kshark_free(struct kshark_context *kshark_ctx)
{
	if (kshark_ctx == NULL) {
		if (kshark_context_handler == NULL)
			return;

		kshark_ctx = kshark_context_handler;
		/* kshark_ctx_handler will be set to NULL below. */
	}

	tracecmd_filter_id_hash_free(kshark_ctx->show_task_filter);
	tracecmd_filter_id_hash_free(kshark_ctx->hide_task_filter);

	tracecmd_filter_id_hash_free(kshark_ctx->show_event_filter);
	tracecmd_filter_id_hash_free(kshark_ctx->hide_event_filter);

	tracecmd_filter_id_hash_free(kshark_ctx->show_cpu_filter);
	tracecmd_filter_id_hash_free(kshark_ctx->hide_cpu_filter);

	if (kshark_ctx->plugins) {
		kshark_handle_plugins(kshark_ctx, KSHARK_PLUGIN_CLOSE);
		kshark_free_plugin_list(kshark_ctx->plugins);
		kshark_free_event_handler_list(kshark_ctx->event_handlers);
	}

	kshark_free_task_list(kshark_ctx);

	if (seq.buffer)
		trace_seq_destroy(&seq);

	if (kshark_ctx == kshark_context_handler)
		kshark_context_handler = NULL;

	free(kshark_ctx);
}

static struct kshark_task_list *
kshark_find_task(struct kshark_context *kshark_ctx, uint32_t key, int pid)
{
	struct kshark_task_list *list;

	for (list = kshark_ctx->tasks[key]; list; list = list->next) {
		if (list->pid == pid)
			return list;
	}

	return NULL;
}

static struct kshark_task_list *
kshark_add_task(struct kshark_context *kshark_ctx, int pid)
{
	struct kshark_task_list *list;
	uint32_t key;

	key = tracecmd_quick_hash(pid, KS_TASK_HASH_SHIFT);

	list = kshark_find_task(kshark_ctx, key, pid);
	if (list)
		return list;

	list = malloc(sizeof(*list));
	if (!list)
		return NULL;

	list->pid = pid;
	list->next = kshark_ctx->tasks[key];
	kshark_ctx->tasks[key] = list;

	return list;
}

/**
 * @brief Get an array containing the Process Ids of all tasks presented in
 *	  the loaded trace data file.
 *
 * @param kshark_ctx: Input location for context pointer.
 * @param pids: Output location for the Pids of the tasks. The user is
 *		responsible for freeing the elements of the outputted array.
 *
 * @returns The size of the outputted array of Pids in the case of success,
 *	    or a negative error code on failure.
 */
ssize_t kshark_get_task_pids(struct kshark_context *kshark_ctx, int **pids)
{
	size_t i, pid_count = 0, pid_size = KS_TASK_HASH_SIZE;
	struct kshark_task_list *list;
	int *temp_pids;

	*pids = calloc(pid_size, sizeof(int));
	if (!*pids)
		goto fail;

	for (i = 0; i < KS_TASK_HASH_SIZE; ++i) {
		list = kshark_ctx->tasks[i];
		while (list) {
			(*pids)[pid_count] = list->pid;
			list = list->next;
			if (++pid_count >= pid_size) {
				pid_size *= 2;
				temp_pids = realloc(*pids, pid_size * sizeof(int));
				if (!temp_pids) {
					goto fail;
				}
				*pids = temp_pids;
			}
		}
	}

	if (pid_count) {
		temp_pids = realloc(*pids, pid_count * sizeof(int));
		if (!temp_pids)
			goto fail;

		/* Paranoid: In the unlikely case of shrinking *pids, realloc moves it */
		*pids = temp_pids;
	} else {
		free(*pids);
		*pids = NULL;
	}

	return pid_count;

fail:
	fprintf(stderr, "Failed to allocate memory for Task Pids.\n");
	free(*pids);
	*pids = NULL;
	return -ENOMEM;
}

static bool filter_find(struct tracecmd_filter_id *filter, int pid,
			bool test)
{
	return !filter || !filter->count ||
		!!(unsigned long)tracecmd_filter_id_find(filter, pid) == test;
}

static bool kshark_show_task(struct kshark_context *kshark_ctx, int pid)
{
	return filter_find(kshark_ctx->show_task_filter, pid, true) &&
	       filter_find(kshark_ctx->hide_task_filter, pid, false);
}

static bool kshark_show_event(struct kshark_context *kshark_ctx, int pid)
{
	return filter_find(kshark_ctx->show_event_filter, pid, true) &&
	       filter_find(kshark_ctx->hide_event_filter, pid, false);
}

static bool kshark_show_cpu(struct kshark_context *kshark_ctx, int cpu)
{
	return filter_find(kshark_ctx->show_cpu_filter, cpu, true) &&
	       filter_find(kshark_ctx->hide_cpu_filter, cpu, false);
}

/**
 * @brief Add an Id value to the filster specified by "filter_id".
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param filter_id: Identifier of the filter.
 * @param id: Id value to be added to the filter.
 */
void kshark_filter_add_id(struct kshark_context *kshark_ctx,
			  int filter_id, int id)
{
	struct tracecmd_filter_id *filter;

	switch (filter_id) {
		case KS_SHOW_CPU_FILTER:
			filter = kshark_ctx->show_cpu_filter;
			break;
		case KS_HIDE_CPU_FILTER:
			filter = kshark_ctx->hide_cpu_filter;
			break;
		case KS_SHOW_EVENT_FILTER:
			filter = kshark_ctx->show_event_filter;
			break;
		case KS_HIDE_EVENT_FILTER:
			filter = kshark_ctx->hide_event_filter;
			break;
		case KS_SHOW_TASK_FILTER:
			filter = kshark_ctx->show_task_filter;
			break;
		case KS_HIDE_TASK_FILTER:
			filter = kshark_ctx->hide_task_filter;
			break;
		default:
			return;
	}

	tracecmd_filter_id_add(filter, id);
}

/**
 * @brief Clear (reset) the filster specified by "filter_id".
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param filter_id: Identifier of the filter.
 */
void kshark_filter_clear(struct kshark_context *kshark_ctx, int filter_id)
{
	struct tracecmd_filter_id *filter;

	switch (filter_id) {
		case KS_SHOW_CPU_FILTER:
			filter = kshark_ctx->show_cpu_filter;
			break;
		case KS_HIDE_CPU_FILTER:
			filter = kshark_ctx->hide_cpu_filter;
			break;
		case KS_SHOW_EVENT_FILTER:
			filter = kshark_ctx->show_event_filter;
			break;
		case KS_HIDE_EVENT_FILTER:
			filter = kshark_ctx->hide_event_filter;
			break;
		case KS_SHOW_TASK_FILTER:
			filter = kshark_ctx->show_task_filter;
			break;
		case KS_HIDE_TASK_FILTER:
			filter = kshark_ctx->hide_task_filter;
			break;
		default:
			return;
	}

	tracecmd_filter_id_clear(filter);
}

/**
 * @brief Check if a given Id filter is set.
 *
 * @param filter: Input location for the Id filster.
 *
 * @returns True if the Id filter is set, otherwise False.
 */
bool kshark_this_filter_is_set(struct tracecmd_filter_id *filter)
{
	return filter && filter->count;
}

/**
 * @brief Check if an Id filter is set.
 *
 * @param kshark_ctx: Input location for the session context pointer.
 *
 * @returns True if at least one Id filter is set, otherwise False.
 */
bool kshark_filter_is_set(struct kshark_context *kshark_ctx)
{
	return kshark_this_filter_is_set(kshark_ctx->show_task_filter) ||
-              kshark_this_filter_is_set(kshark_ctx->hide_task_filter) ||
-              kshark_this_filter_is_set(kshark_ctx->show_cpu_filter) ||
-              kshark_this_filter_is_set(kshark_ctx->hide_cpu_filter) ||
-              kshark_this_filter_is_set(kshark_ctx->show_event_filter) ||
-              kshark_this_filter_is_set(kshark_ctx->hide_event_filter);
}

static inline void unset_event_filter_flag(struct kshark_context *kshark_ctx,
					   struct kshark_entry *e)
{
	/*
	 * All entries, filtered-out by the event filters, will be treated
	 * differently, when visualized. Because of this, ignore the value
	 * of the GRAPH_VIEW flag provided by the user via
	 * kshark_ctx->filter_mask. The value of the EVENT_VIEW flag in
	 * kshark_ctx->filter_mask will be used instead.
	 */
	int event_mask = kshark_ctx->filter_mask & ~KS_GRAPH_VIEW_FILTER_MASK;

	e->visible &= ~event_mask;
}

static void set_all_visible(uint16_t *v) {
	/*  Keep the original value of the PLUGIN_UNTOUCHED bit flag. */
	*v |= 0xFF & ~KS_PLUGIN_UNTOUCHED_MASK;
}

/**
 * @brief This function loops over the array of entries specified by "data"
 *	  and "n_entries" and sets the "visible" fields of each entry
 *	  according to the criteria provided by the filters of the session's
 *	  context. The field "filter_mask" of the session's context is used to
 *	  control the level of visibility/invisibility of the entries which
 *	  are filtered-out.
 *	  WARNING: Do not use this function if the advanced filter is set.
 *	  Applying the advanced filter requires access to prevent_record,
 *	  hence the data has to be reloaded using kshark_load_data_entries().
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param data: Input location for the trace data to be filtered.
 * @param n_entries: The size of the inputted data.
 */
void kshark_filter_entries(struct kshark_context *kshark_ctx,
			   struct kshark_entry **data,
			   size_t n_entries)
{
	int i;

	if (kshark_ctx->advanced_event_filter->filters) {
		/* The advanced filter is set. */
		fprintf(stderr,
			"Failed to filter!\n");
		fprintf(stderr,
			"Reset the Advanced filter or reload the data.\n");
		return;
	}

	if (!kshark_filter_is_set(kshark_ctx))
		return;

	/* Apply only the Id filters. */
	for (i = 0; i < n_entries; ++i) {
		/* Start with and entry which is visible everywhere. */
		set_all_visible(&data[i]->visible);

		/* Apply event filtering. */
		if (!kshark_show_event(kshark_ctx, data[i]->event_id))
			unset_event_filter_flag(kshark_ctx, data[i]);

		/* Apply CPU filtering. */
		if (!kshark_show_cpu(kshark_ctx, data[i]->cpu))
			data[i]->visible &= ~kshark_ctx->filter_mask;

		/* Apply task filtering. */
		if (!kshark_show_task(kshark_ctx, data[i]->pid))
			data[i]->visible &= ~kshark_ctx->filter_mask;
	}
}

/**
 * @brief This function loops over the array of entries specified by "data"
 *	  and "n_entries" and resets the "visible" fields of each entry to
 *	  the default value of "0xFF" (visible everywhere).
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param data: Input location for the trace data to be unfiltered.
 * @param n_entries: The size of the inputted data.
 */
void kshark_clear_all_filters(struct kshark_context *kshark_ctx,
			      struct kshark_entry **data,
			      size_t n_entries)
{
	int i;
	for (i = 0; i < n_entries; ++i)
		set_all_visible(&data[i]->visible);
}

static void kshark_set_entry_values(struct kshark_context *kshark_ctx,
				    struct tep_record *record,
				    struct kshark_entry *entry)
{
	/* Offset of the record */
	entry->offset = record->offset;

	/* CPU Id of the record */
	entry->cpu = record->cpu;

	/* Time stamp of the record */
	entry->ts = record->ts;

	/* Event Id of the record */
	entry->event_id = tep_data_type(kshark_ctx->pevent, record);

	/*
	 * Is visible mask. This default value means that the entry
	 * is visible everywhere.
	 */
	entry->visible = 0xFF;

	/* Process Id of the record */
	entry->pid = tep_data_pid(kshark_ctx->pevent, record);
}

/** Prior time offset of the "missed_events" entry. */
#define ME_ENTRY_TIME_SHIFT	10

static void missed_events_action(struct kshark_context *kshark_ctx,
				 struct tep_record *record,
				 struct kshark_entry *entry)
{
	/*
	 * Use the offset field of the entry to store the number of missed
	 * events.
	 */
	entry->offset = record->missed_events;

	entry->cpu = record->cpu;

	/*
	 * Position the "missed_events" entry a bit before (in time)
	 * the original record.
	 */
	entry->ts = record->ts - ME_ENTRY_TIME_SHIFT;

	/* All custom entries must have negative event Identifiers. */
	entry->event_id = KS_EVENT_OVERFLOW;

	entry->visible = 0xFF;

	entry->pid = tep_data_pid(kshark_ctx->pevent, record);
}

static const char* missed_events_dump(struct kshark_context *kshark_ctx,
				      const struct kshark_entry *entry,
				      bool get_info)
{
	int size = 0;
	static char *buffer;

	if (get_info)
		size = asprintf(&buffer, "missed_events=%i", (int) entry->offset);
	else
		size = asprintf(&buffer, "missed_events");
	if (size > 0)
		return buffer;

	return NULL;
}

/**
 * rec_list is used to pass the data to the load functions.
 * The rec_list will contain the list of entries from the source,
 * and will be a link list of per CPU entries.
 */
struct rec_list {
	union {
		/* Used by kshark_load_data_records */
		struct {
			/** next pointer, matches entry->next */
			struct rec_list		*next;
			/** pointer to the raw record data */
			struct tep_record	*rec;
		};
		/** entry - Used for kshark_load_data_entries() */
		struct kshark_entry		entry;
	};
};

/**
 * rec_type defines what type of rec_list is being used.
 */
enum rec_type {
	REC_RECORD,
	REC_ENTRY,
};

static void free_rec_list(struct rec_list **rec_list, int n_cpus,
			  enum rec_type type)
{
	struct rec_list *temp_rec;
	int cpu;

	for (cpu = 0; cpu < n_cpus; ++cpu) {
		while (rec_list[cpu]) {
			temp_rec = rec_list[cpu];
			rec_list[cpu] = temp_rec->next;
			if (type == REC_RECORD)
				tracecmd_free_record(temp_rec->rec);
			free(temp_rec);
		}
	}
	free(rec_list);
}

static ssize_t get_records(struct kshark_context *kshark_ctx,
			   struct rec_list ***rec_list, enum rec_type type)
{
	struct kshark_event_handler *evt_handler;
	struct tep_event_filter *adv_filter;
	struct kshark_task_list *task;
	struct tep_record *rec;
	struct rec_list **temp_next;
	struct rec_list **cpu_list;
	struct rec_list *temp_rec;
	size_t count, total = 0;
	int n_cpus;
	int pid;
	int cpu;

	n_cpus = tep_get_cpus(kshark_ctx->pevent);;
	cpu_list = calloc(n_cpus, sizeof(*cpu_list));
	if (!cpu_list)
		return -ENOMEM;

	/* Just to shorten the name */
	if (type == REC_ENTRY)
		adv_filter = kshark_ctx->advanced_event_filter;

	for (cpu = 0; cpu < n_cpus; ++cpu) {
		count = 0;
		cpu_list[cpu] = NULL;
		temp_next = &cpu_list[cpu];

		rec = tracecmd_read_cpu_first(kshark_ctx->handle, cpu);
		while (rec) {
			*temp_next = temp_rec = calloc(1, sizeof(*temp_rec));
			if (!temp_rec)
				goto fail;

			temp_rec->next = NULL;

			switch (type) {
			case REC_RECORD:
				temp_rec->rec = rec;
				pid = tep_data_pid(kshark_ctx->pevent, rec);
				break;
			case REC_ENTRY: {
				struct kshark_entry *entry;
				int ret;

				if (rec->missed_events) {
					/*
					 * Insert a custom "missed_events" entry just
					 * befor this record.
					 */
					entry = &temp_rec->entry;
					missed_events_action(kshark_ctx, rec, entry);

					temp_next = &temp_rec->next;
					++count;

					/* Now allocate a new rec_list node and comtinue. */
					*temp_next = temp_rec = calloc(1, sizeof(*temp_rec));
				}

				entry = &temp_rec->entry;
				kshark_set_entry_values(kshark_ctx, rec, entry);

				/* Execute all plugin-provided actions (if any). */
				evt_handler = kshark_ctx->event_handlers;
				while ((evt_handler = kshark_find_event_handler(evt_handler,
										entry->event_id))) {
					evt_handler->event_func(kshark_ctx, rec, entry);
					evt_handler = evt_handler->next;
					entry->visible &= ~KS_PLUGIN_UNTOUCHED_MASK;
				}

				pid = entry->pid;
				/* Apply event filtering. */
				ret = FILTER_MATCH;
				if (adv_filter->filters)
					ret = tep_filter_match(adv_filter, rec);

				if (!kshark_show_event(kshark_ctx, entry->event_id) ||
				    ret != FILTER_MATCH) {
					unset_event_filter_flag(kshark_ctx, entry);
				}

				/* Apply CPU filtering. */
				if (!kshark_show_cpu(kshark_ctx, entry->pid)) {
					entry->visible &= ~kshark_ctx->filter_mask;
				}

				/* Apply task filtering. */
				if (!kshark_show_task(kshark_ctx, entry->pid)) {
					entry->visible &= ~kshark_ctx->filter_mask;
				}
				tracecmd_free_record(rec);
				break;
			} /* REC_ENTRY */
			}

			task = kshark_add_task(kshark_ctx, pid);
			if (!task) {
				tracecmd_free_record(rec);
				goto fail;
			}

			temp_next = &temp_rec->next;

			++count;
			rec = tracecmd_read_data(kshark_ctx->handle, cpu);
		}

		total += count;
	}

	*rec_list = cpu_list;
	return total;

 fail:
	free_rec_list(cpu_list, n_cpus, type);
	return -ENOMEM;
}

static int pick_next_cpu(struct rec_list **rec_list, int n_cpus,
			 enum rec_type type)
{
	uint64_t ts = 0;
	uint64_t rec_ts;
	int next_cpu = -1;
	int cpu;

	for (cpu = 0; cpu < n_cpus; ++cpu) {
		if (!rec_list[cpu])
			continue;

		switch (type) {
		case REC_RECORD:
			rec_ts = rec_list[cpu]->rec->ts;
			break;
		case REC_ENTRY:
			rec_ts = rec_list[cpu]->entry.ts;
			break;
		}
		if (!ts || rec_ts < ts) {
			ts = rec_ts;
			next_cpu = cpu;
		}
	}

	return next_cpu;
}

/**
 * @brief Load the content of the trace data file into an array of
 *	  kshark_entries. This function provides an abstraction of the
 *	  entries from the raw data that is read, however the "latency"
 *	  and the "info" fields can be accessed only via the offset
 *	  into the file. This makes the access to these two fields much
 *	  slower.
 *	  If one or more filters are set, the "visible" fields of each entry
 *	  is updated according to the criteria provided by the filters. The
 *	  field "filter_mask" of the session's context is used to control the
 *	  level of visibility/invisibility of the filtered entries.
 *
 * @param kshark_ctx: Input location for context pointer.
 * @param data_rows: Output location for the trace data. The user is
 *		     responsible for freeing the elements of the outputted
 *		     array.
 *
 * @returns The size of the outputted data in the case of success, or a
 *	    negative error code on failure.
 */
ssize_t kshark_load_data_entries(struct kshark_context *kshark_ctx,
				 struct kshark_entry ***data_rows)
{
	struct kshark_entry **rows;
	struct rec_list **rec_list;
	enum rec_type type = REC_ENTRY;
	ssize_t count, total = 0;
	int n_cpus;

	if (*data_rows)
		free(*data_rows);

	total = get_records(kshark_ctx, &rec_list, type);
	if (total < 0)
		goto fail;

	n_cpus = tep_get_cpus(kshark_ctx->pevent);;

	rows = calloc(total, sizeof(struct kshark_entry *));
	if (!rows)
		goto fail_free;

	for (count = 0; count < total; count++) {
		int next_cpu;

		next_cpu = pick_next_cpu(rec_list, n_cpus, type);

		if (next_cpu >= 0) {
			rows[count] = &rec_list[next_cpu]->entry;
			rec_list[next_cpu] = rec_list[next_cpu]->next;
		}
	}

	free_rec_list(rec_list, n_cpus, type);
	*data_rows = rows;
	return total;

 fail_free:
	free_rec_list(rec_list, n_cpus, type);

 fail:
	fprintf(stderr, "Failed to allocate memory during data loading.\n");
	return -ENOMEM;
}

/**
 * @brief Load the content of the trace data file into an array of
 *	  tep_records. Use this function only if you need fast access
 *	  to all fields of the record.
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param data_rows: Output location for the trace data. Use tracecmd_free_record()
 *	 	     to free the elements of the outputted array.
 *
 * @returns The size of the outputted data in the case of success, or a
 *	    negative error code on failure.
 */
ssize_t kshark_load_data_records(struct kshark_context *kshark_ctx,
				 struct tep_record ***data_rows)
{
	struct tep_record **rows;
	struct tep_record *rec;
	struct rec_list **rec_list;
	struct rec_list *temp_rec;
	enum rec_type type = REC_RECORD;
	ssize_t count, total = 0;
	int n_cpus;

	total = get_records(kshark_ctx, &rec_list, type);
	if (total < 0)
		goto fail;

	n_cpus = tep_get_cpus(kshark_ctx->pevent);;

	rows = calloc(total, sizeof(struct tep_record *));
	if (!rows)
		goto fail_free;

	for (count = 0; count < total; count++) {
		int next_cpu;

		next_cpu = pick_next_cpu(rec_list, n_cpus, type);

		if (next_cpu >= 0) {
			rec = rec_list[next_cpu]->rec;
			rows[count] = rec;

			temp_rec = rec_list[next_cpu];
			rec_list[next_cpu] = rec_list[next_cpu]->next;
			free(temp_rec);
			/* The record is still referenced in rows */
		}
	}

	/* There should be no records left in rec_list */
	free_rec_list(rec_list, n_cpus, type);
	*data_rows = rows;
	return total;

 fail_free:
	free_rec_list(rec_list, n_cpus, type);

 fail:
	fprintf(stderr, "Failed to allocate memory during data loading.\n");
	return -ENOMEM;
}

static inline void free_ptr(void *ptr)
{
	if (ptr)
		free(*(void **)ptr);
}

static bool data_matrix_alloc(size_t n_rows, uint64_t **offset_array,
					     uint16_t **cpu_array,
					     uint64_t **ts_array,
					     uint16_t **pid_array,
					     int **event_array)
{
	if (offset_array) {
		*offset_array = calloc(n_rows, sizeof(**offset_array));
		if (!*offset_array)
			return false;
	}

	if (cpu_array) {
		*cpu_array = calloc(n_rows, sizeof(**cpu_array));
		if (!*cpu_array)
			goto free_offset;
	}

	if (ts_array) {
		*ts_array = calloc(n_rows, sizeof(**ts_array));
		if (!*ts_array)
			goto free_cpu;
	}

	if (pid_array) {
		*pid_array = calloc(n_rows, sizeof(**pid_array));
		if (!*pid_array)
			goto free_ts;
	}

	if (event_array) {
		*event_array = calloc(n_rows, sizeof(**event_array));
		if (!*event_array)
			goto free_pid;
	}

	return true;

 free_pid:
	free_ptr(pid_array);
 free_ts:
	free_ptr(ts_array);
 free_cpu:
	free_ptr(cpu_array);
 free_offset:
	free_ptr(offset_array);

	fprintf(stderr, "Failed to allocate memory during data loading.\n");
	return false;
}

/**
 * @brief Load the content of the trace data file into a table / matrix made
 *	  of columns / arrays of data. The user is responsible for freeing the
 *	  elements of the outputted array
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param offset_array: Output location for the array of record offsets.
 * @param cpu_array: Output location for the array of CPU Ids.
 * @param ts_array: Output location for the array of timestamps.
 * @param pid_array: Output location for the array of Process Ids.
 * @param event_array: Output location for the array of Event Ids.
 *
 * @returns The size of the outputted arrays in the case of success, or a
 *	    negative error code on failure.
 */
size_t kshark_load_data_matrix(struct kshark_context *kshark_ctx,
			       uint64_t **offset_array,
			       uint16_t **cpu_array,
			       uint64_t **ts_array,
			       uint16_t **pid_array,
			       int **event_array)
{
	enum rec_type type = REC_ENTRY;
	struct rec_list **rec_list;
	ssize_t count, total = 0;
	bool status;
	int n_cpus;

	total = get_records(kshark_ctx, &rec_list, type);
	if (total < 0)
		goto fail;

	n_cpus = tep_get_cpus(kshark_ctx->pevent);;

	status = data_matrix_alloc(total, offset_array,
					  cpu_array,
					  ts_array,
					  pid_array,
					  event_array);
	if (!status)
		goto fail_free;

	for (count = 0; count < total; count++) {
		int next_cpu;

		next_cpu = pick_next_cpu(rec_list, n_cpus, type);
		if (next_cpu >= 0) {
			struct rec_list *rec = rec_list[next_cpu];
			struct kshark_entry *e = &rec->entry;

			if (offset_array)
				(*offset_array)[count] = e->offset;

			if (cpu_array)
				(*cpu_array)[count] = e->cpu;

			if (ts_array)
				(*ts_array)[count] = e->ts;

			if (pid_array)
				(*pid_array)[count] = e->pid;

			if (event_array)
				(*event_array)[count] = e->event_id;

			rec_list[next_cpu] = rec_list[next_cpu]->next;
			free(rec);
		}
	}

	/* There should be no entries left in rec_list. */
	free_rec_list(rec_list, n_cpus, type);
	return total;

 fail_free:
	free_rec_list(rec_list, n_cpus, type);

 fail:
	fprintf(stderr, "Failed to allocate memory during data loading.\n");
	return -ENOMEM;
}

static const char *kshark_get_latency(struct tep_handle *pe,
				      struct tep_record *record)
{
	if (!record)
		return NULL;

	trace_seq_reset(&seq);
	tep_print_event(pe, &seq, record, "%s", TEP_PRINT_LATENCY);
	return seq.buffer;
}

static const char *kshark_get_info(struct tep_handle *pe,
				   struct tep_record *record,
				   struct tep_event *event)
{
	char *pos;

	if (!record || !event)
		return NULL;

	trace_seq_reset(&seq);
	tep_print_event(pe, &seq, record, "%s", TEP_PRINT_INFO);

	/*
	 * The event info string contains a trailing newline.
	 * Remove this newline.
	 */
	if ((pos = strchr(seq.buffer, '\n')) != NULL)
		*pos = '\0';

	return seq.buffer;
}

/**
 * @brief This function allows for an easy access to the original value of the
 *	  Process Id as recorded in the tep_record object. The record is read
 *	  from the file only in the case of an entry being touched by a plugin.
 *	  Be aware that using the kshark_get_X_easy functions can be
 *	  inefficient if you need an access to more than one of the data fields
 *	  of the record.
 *
 * @param entry: Input location for the KernelShark entry.
 *
 * @returns The original value of the Process Id as recorded in the
 *	    tep_record object on success, otherwise negative error code.
 */
int kshark_get_pid_easy(struct kshark_entry *entry)
{
	struct kshark_context *kshark_ctx = NULL;
	struct tep_record *data;
	int pid;

	if (!kshark_instance(&kshark_ctx))
		return -ENODEV;

	if (entry->visible & KS_PLUGIN_UNTOUCHED_MASK) {
		pid = entry->pid;
	} else {
		/*
		 * The entry has been touched by a plugin callback function.
		 * Because of this we do not trust the value of "entry->pid".
		 *
		 * Currently the data reading operations are not thread-safe.
		 * Use a mutex to protect the access.
		 */
		pthread_mutex_lock(&kshark_ctx->input_mutex);

		data = tracecmd_read_at(kshark_ctx->handle, entry->offset,
					NULL);
		pid = tep_data_pid(kshark_ctx->pevent, data);
		tracecmd_free_record(data);

		pthread_mutex_unlock(&kshark_ctx->input_mutex);
	}

	return pid;
}

/**
 * @brief This function allows for an easy access to the original value of the
 *	  Task name as recorded in the tep_record object. The record is read
 *	  from the file only in the case of an entry being touched by a plugin.
 *	  Be aware that using the kshark_get_X_easy functions can be
 *	  inefficient if you need an access to more than one of the data fields
 *	  of the record.
 *
 * @param entry: Input location for the KernelShark entry.
 *
 * @returns The original name of the task, retrieved from the Process Id
 *	    recorded in the tep_record object on success, otherwise NULL.
 */
const char *kshark_get_task_easy(struct kshark_entry *entry)
{
	struct kshark_context *kshark_ctx = NULL;
	int pid = kshark_get_pid_easy(entry);

	if (pid < 0)
		return NULL;

	kshark_instance(&kshark_ctx);
	return tep_data_comm_from_pid(kshark_ctx->pevent, pid);
}

/**
 * @brief This function allows for an easy access to the latency information
 *	  recorded in the tep_record object. The record is read from the file
 *	  using the offset field of kshark_entry. Be aware that using the
 *	  kshark_get_X_easy functions can be inefficient if you need an access
 *	  to more than one of the data fields of the record.
 *
 * @param entry: Input location for the KernelShark entry.
 *
 * @returns On success the function returns a string showing the latency
 *	    information, coded into 5 fields:
 *	    interrupts disabled, need rescheduling, hard/soft interrupt,
 *	    preempt count and lock depth. On failure it returns NULL.
 */
const char *kshark_get_latency_easy(struct kshark_entry *entry)
{
	struct kshark_context *kshark_ctx = NULL;
	struct tep_record *data;
	const char *lat;

	if (!kshark_instance(&kshark_ctx))
		return NULL;

	if (entry->event_id < 0)
		return NULL;

	/*
	 * Currently the data reading operations are not thread-safe.
	 * Use a mutex to protect the access.
	 */
	pthread_mutex_lock(&kshark_ctx->input_mutex);

	data = tracecmd_read_at(kshark_ctx->handle, entry->offset, NULL);
	lat = kshark_get_latency(kshark_ctx->pevent, data);
	tracecmd_free_record(data);

	pthread_mutex_unlock(&kshark_ctx->input_mutex);

	return lat;
}

/**
 * @brief This function allows for an easy access to the original value of the
 *	  Event Id as recorded in the tep_record object. The record is read
 *	  from the file only in the case of an entry being touched by a plugin.
 *	  Be aware that using the kshark_get_X_easy functions can be
 *	  inefficient if you need an access to more than one of the data fields
 *	  of the record.
 *
 * @param entry: Input location for the KernelShark entry.
 *
 * @returns The original value of the Event Id as recorded in the
 *	    tep_record object on success, otherwise negative error code.
 */
int kshark_get_event_id_easy(struct kshark_entry *entry)
{
	struct kshark_context *kshark_ctx = NULL;
	struct tep_record *data;
	int event_id;

	if (!kshark_instance(&kshark_ctx))
		return -ENODEV;

	if (entry->visible & KS_PLUGIN_UNTOUCHED_MASK) {
		event_id = entry->event_id;
	} else {
		/*
		 * The entry has been touched by a plugin callback function.
		 * Because of this we do not trust the value of
		 * "entry->event_id".
		 *
		 * Currently the data reading operations are not thread-safe.
		 * Use a mutex to protect the access.
		 */
		pthread_mutex_lock(&kshark_ctx->input_mutex);

		data = tracecmd_read_at(kshark_ctx->handle, entry->offset,
					NULL);
		event_id = tep_data_type(kshark_ctx->pevent, data);
		tracecmd_free_record(data);

		pthread_mutex_unlock(&kshark_ctx->input_mutex);
	}

	return (event_id == -1)? -EFAULT : event_id;
}

/**
 * @brief This function allows for an easy access to the original name of the
 *	  trace event as recorded in the tep_record object. The record is read
 *	  from the file only in the case of an entry being touched by a plugin.
 *	  Be aware that using the kshark_get_X_easy functions can be
 *	  inefficient if you need an access to more than one of the data fields
 *	  of the record.
 *
 * @param entry: Input location for the KernelShark entry.
 *
 * @returns The mane of the trace event recorded in the tep_record object on
 *	    success, otherwise "[UNKNOWN EVENT]" or NULL.
 */
const char *kshark_get_event_name_easy(struct kshark_entry *entry)
{
	struct kshark_context *kshark_ctx = NULL;
	struct tep_event *event;

	int event_id = kshark_get_event_id_easy(entry);
	if (event_id == -EFAULT)
		return NULL;

	kshark_instance(&kshark_ctx);

	if (event_id < 0) {
		switch (event_id) {
		case KS_EVENT_OVERFLOW:
			return missed_events_dump(kshark_ctx, entry, false);
		default:
			return NULL;
		}
	}

	/*
	 * Currently the data reading operations are not thread-safe.
	 * Use a mutex to protect the access.
	 */
	pthread_mutex_lock(&kshark_ctx->input_mutex);
	event = tep_find_event(kshark_ctx->pevent, event_id);
	pthread_mutex_unlock(&kshark_ctx->input_mutex);

	if (event)
		return event->name;

	return "[UNKNOWN EVENT]";
}

/**
 * @brief This function allows for an easy access to the tep_record's info
 *	  streang. The record is read from the file using the offset field of
 *	  kshark_entry. Be aware that using the kshark_get_X_easy functions can
 *	  be inefficient if you need an access to more than one of the data
 *	  fields of the record.
 *
 * @param entry: Input location for the KernelShark entry.
 *
 * @returns A string showing the data output of the trace event on success,
 *	    otherwise NULL.
 */
const char *kshark_get_info_easy(struct kshark_entry *entry)
{
	struct kshark_context *kshark_ctx = NULL;
	struct tep_event *event;
	struct tep_record *data;
	const char *info = NULL;
	int event_id;

	if (!kshark_instance(&kshark_ctx))
		return NULL;

	if (entry->event_id < 0) {
		switch (entry->event_id) {
		case KS_EVENT_OVERFLOW:
			return missed_events_dump(kshark_ctx, entry, true);
		default:
			return NULL;
		}
	}

	/*
	 * Currently the data reading operations are not thread-safe.
	 * Use a mutex to protect the access.
	 */
	pthread_mutex_lock(&kshark_ctx->input_mutex);

	data = tracecmd_read_at(kshark_ctx->handle, entry->offset, NULL);
	event_id = tep_data_type(kshark_ctx->pevent, data);
	event = tep_find_event(kshark_ctx->pevent, event_id);
	if (event)
		info = kshark_get_info(kshark_ctx->pevent, data, event);

	tracecmd_free_record(data);

	pthread_mutex_unlock(&kshark_ctx->input_mutex);

	return info;
}

/**
 * @brief Convert the timestamp of the trace record (nanosecond precision) into
 *	  seconds and microseconds.
 *
 * @param time: Input location for the timestamp.
 * @param sec: Output location for the value of the seconds.
 * @param usec: Output location for the value of the microseconds.
 */
void kshark_convert_nano(uint64_t time, uint64_t *sec, uint64_t *usec)
{
	uint64_t s;

	*sec = s = time / 1000000000ULL;
	*usec = (time - s * 1000000000ULL) / 1000;
}

/**
 * @brief Dump into a string the content a custom entry. The function allocates
 *	  a null terminated string and returns a pointer to this string.
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param entry: A Kernel Shark entry to be printed.
 * @param info_func:
 *
 * @returns The returned string contains a semicolon-separated list of data
 *	    fields. The user has to free the returned string.
 */
char* kshark_dump_custom_entry(struct kshark_context *kshark_ctx,
			       const struct kshark_entry *entry,
			       kshark_custom_info_func info_func)
{
	const char *event_name, *task, *info;
	char *entry_str;
	int size = 0;

	task = tep_data_comm_from_pid(kshark_ctx->pevent, entry->pid);
	event_name = info_func(kshark_ctx, entry, false);
	info = info_func(kshark_ctx, entry, true);

	size = asprintf(&entry_str, "%" PRIu64 "; %s-%i; CPU %i; ; %s; %s",
			entry->ts,
			task,
			entry->pid,
			entry->cpu,
			event_name,
			info);

	if (size > 0)
		return entry_str;

	return NULL;
}

/**
 * @brief Dump into a string the content of one entry. The function allocates
 *	  a null terminated string and returns a pointer to this string. The
 *	  user has to free the returned string.
 *
 * @param entry: A Kernel Shark entry to be printed.
 *
 * @returns The returned string contains a semicolon-separated list of data
 *	    fields.
 */
char* kshark_dump_entry(const struct kshark_entry *entry)
{
	const char *event_name, *task, *lat, *info;
	struct kshark_context *kshark_ctx;
	char *temp_str, *entry_str;
	int size = 0;

	kshark_ctx = NULL;
	if (!kshark_instance(&kshark_ctx) || !init_thread_seq())
		return NULL;

	task = tep_data_comm_from_pid(kshark_ctx->pevent, entry->pid);

	if (entry->event_id >= 0) {
		struct tep_event *event;
		struct tep_record *data;

		data = tracecmd_read_at(kshark_ctx->handle, entry->offset,
					NULL);

		event = tep_find_event(kshark_ctx->pevent, entry->event_id);

		event_name = event? event->name : "[UNKNOWN EVENT]";
		lat = kshark_get_latency(kshark_ctx->pevent, data);

		size = asprintf(&temp_str, "%" PRIu64 "; %s-%i; CPU %i; %s;",
				entry->ts,
				task,
				entry->pid,
				entry->cpu,
				lat);

		info = kshark_get_info(kshark_ctx->pevent, data, event);

		if (size > 0) {
			size = asprintf(&entry_str, "%s %s; %s; 0x%x",
					temp_str,
					event_name,
					info,
					entry->visible);

			free(temp_str);
		}

		tracecmd_free_record(data);
		if (size < 1)
			entry_str = NULL;
	} else {
		switch (entry->event_id) {
		case KS_EVENT_OVERFLOW:
			entry_str = kshark_dump_custom_entry(kshark_ctx, entry,
							     missed_events_dump);
		default:
			entry_str = NULL;
		}
	}

	return entry_str;
}

/**
 * @brief Binary search inside a time-sorted array of kshark_entries.
 *
 * @param time: The value of time to search for.
 * @param data: Input location for the trace data.
 * @param l: Array index specifying the lower edge of the range to search in.
 * @param h: Array index specifying the upper edge of the range to search in.
 *
 * @returns On success, the first kshark_entry inside the range, having a
	    timestamp equal or bigger than "time".
	    If all entries inside the range have timestamps greater than "time"
	    the function returns BSEARCH_ALL_GREATER (negative value).
	    If all entries inside the range have timestamps smaller than "time"
	    the function returns BSEARCH_ALL_SMALLER (negative value).
 */
ssize_t kshark_find_entry_by_time(uint64_t time,
				 struct kshark_entry **data,
				 size_t l, size_t h)
{
	size_t mid;

	if (data[l]->ts > time)
		return BSEARCH_ALL_GREATER;

	if (data[h]->ts < time)
		return BSEARCH_ALL_SMALLER;

	/*
	 * After executing the BSEARCH macro, "l" will be the index of the last
	 * entry having timestamp < time and "h" will be the index of the first
	 * entry having timestamp >= time.
	 */
	BSEARCH(h, l, data[mid]->ts < time);
	return h;
}

/**
 * @brief Binary search inside a time-sorted array of tep_records.
 *
 * @param time: The value of time to search for.
 * @param data: Input location for the trace data.
 * @param l: Array index specifying the lower edge of the range to search in.
 * @param h: Array index specifying the upper edge of the range to search in.
 *
 * @returns On success, the first tep_record inside the range, having a
	    timestamp equal or bigger than "time".
	    If all entries inside the range have timestamps greater than "time"
	    the function returns BSEARCH_ALL_GREATER (negative value).
	    If all entries inside the range have timestamps smaller than "time"
	    the function returns BSEARCH_ALL_SMALLER (negative value).
 */
ssize_t kshark_find_record_by_time(uint64_t time,
				   struct tep_record **data,
				   size_t l, size_t h)
{
	size_t mid;

	if (data[l]->ts > time)
		return BSEARCH_ALL_GREATER;

	if (data[h]->ts < time)
		return BSEARCH_ALL_SMALLER;

	/*
	 * After executing the BSEARCH macro, "l" will be the index of the last
	 * record having timestamp < time and "h" will be the index of the
	 * first record having timestamp >= time.
	 */
	BSEARCH(h, l, data[mid]->ts < time);
	return h;
}

/**
 * @brief Simple Pid matching function to be user for data requests.
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param e: kshark_entry to be checked.
 * @param pid: Matching condition value.
 *
 * @returns True if the Pid of the entry matches the value of "pid".
 *	    Else false.
 */
bool kshark_match_pid(struct kshark_context *kshark_ctx,
		      struct kshark_entry *e, int pid)
{
	if (e->pid == pid)
		return true;

	return false;
}

/**
 * @brief Simple Cpu matching function to be user for data requests.
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param e: kshark_entry to be checked.
 * @param cpu: Matching condition value.
 *
 * @returns True if the Cpu of the entry matches the value of "cpu".
 *	    Else false.
 */
bool kshark_match_cpu(struct kshark_context *kshark_ctx,
		      struct kshark_entry *e, int cpu)
{
	if (e->cpu == cpu)
		return true;

	return false;
}

/**
 * @brief Create Data request. The request defines the properties of the
 *	  requested kshark_entry.
 *
 * @param first: Array index specifying the position inside the array from
 *		 where the search starts.
 * @param n: Number of array elements to search in.
 * @param cond: Matching condition function.
 * @param val: Matching condition value, used by the Matching condition
 *	       function.
 * @param vis_only: If true, a visible entry is requested.
 * @param vis_mask: If "vis_only" is true, use this mask to specify the level
 *		    of visibility of the requested entry.
 *
 * @returns Pointer to kshark_entry_request on success, or NULL on failure.
 *	    The user is responsible for freeing the returned
 *	    kshark_entry_request.
 */
struct kshark_entry_request *
kshark_entry_request_alloc(size_t first, size_t n,
			   matching_condition_func cond, int val,
			   bool vis_only, int vis_mask)
{
	struct kshark_entry_request *req = malloc(sizeof(*req));

	if (!req) {
		fprintf(stderr,
			"Failed to allocate memory for entry request.\n");
		return NULL;
	}

	req->next = NULL;
	req->first = first;
	req->n = n;
	req->cond = cond;
	req->val = val;
	req->vis_only = vis_only;
	req->vis_mask = vis_mask;

	return req;
}

/**
 * @brief Free all Data requests in a given list.
 * @param req: Intput location for the Data request list.
 */
void kshark_free_entry_request(struct kshark_entry_request *req)
{
	struct kshark_entry_request *last;

	while (req) {
		last = req;
		req = req->next;
		free(last);
	}
}

/** Dummy entry, used to indicate the existence of filtered entries. */
const struct kshark_entry dummy_entry = {
	.next		= NULL,
	.visible	= 0x00,
	.cpu		= KS_FILTERED_BIN,
	.pid		= KS_FILTERED_BIN,
	.event_id	= -1,
	.offset		= 0,
	.ts		= 0
};

static const struct kshark_entry *
get_entry(const struct kshark_entry_request *req,
          struct kshark_entry **data,
          ssize_t *index, ssize_t start, ssize_t end, int inc)
{
	struct kshark_context *kshark_ctx = NULL;
	const struct kshark_entry *e = NULL;
	ssize_t i;

	if (index)
		*index = KS_EMPTY_BIN;

	if (!kshark_instance(&kshark_ctx))
		return e;

	/*
	 * We will do a sanity check in order to protect against infinite
	 * loops.
	 */
	assert((inc > 0 && start < end) || (inc < 0 && start > end));
	for (i = start; i != end; i += inc) {
		if (req->cond(kshark_ctx, data[i], req->val)) {
			/*
			 * Data satisfying the condition has been found.
			 */
			if (req->vis_only &&
			    !(data[i]->visible & req->vis_mask)) {
				/* This data entry has been filtered. */
				e = &dummy_entry;
			} else {
				e = data[i];
				break;
			}
		}
	}

	if (index) {
		if (e)
			*index = (e->cpu != KS_FILTERED_BIN)? i : KS_FILTERED_BIN;
		else
			*index = KS_EMPTY_BIN;
	}

	return e;
}

/**
 * @brief Search for an entry satisfying the requirements of a given Data
 *	  request. Start from the position provided by the request and go
 *	  searching in the direction of the increasing timestamps (front).
 *
 * @param req: Input location for Data request.
 * @param data: Input location for the trace data.
 * @param index: Optional output location for the index of the returned
 *		 entry inside the array.
 *
 * @returns Pointer to the first entry satisfying the matching conditionon
 *	    success, or NULL on failure.
 *	    In the special case when some entries, satisfying the Matching
 *	    condition function have been found, but all these entries have
 *	    been discarded because of the visibility criteria (filtered
 *	    entries), the function returns a pointer to a special
 *	    "Dummy entry".
 */
const struct kshark_entry *
kshark_get_entry_front(const struct kshark_entry_request *req,
                       struct kshark_entry **data,
                       ssize_t *index)
{
	ssize_t end = req->first + req->n;

	return get_entry(req, data, index, req->first, end, +1);
}

/**
 * @brief Search for an entry satisfying the requirements of a given Data
 *	  request. Start from the position provided by the request and go
 *	  searching in the direction of the decreasing timestamps (back).
 *
 * @param req: Input location for Data request.
 * @param data: Input location for the trace data.
 * @param index: Optional output location for the index of the returned
 *		 entry inside the array.
 *
 * @returns Pointer to the first entry satisfying the matching conditionon
 *	    success, or NULL on failure.
 *	    In the special case when some entries, satisfying the Matching
 *	    condition function have been found, but all these entries have
 *	    been discarded because of the visibility criteria (filtered
 *	    entries), the function returns a pointer to a special
 *	    "Dummy entry".
 */
const struct kshark_entry *
kshark_get_entry_back(const struct kshark_entry_request *req,
                      struct kshark_entry **data,
                      ssize_t *index)
{
	ssize_t end = req->first - req->n;

	if (end < 0)
		end = -1;

	return get_entry(req, data, index, req->first, end, -1);
}
