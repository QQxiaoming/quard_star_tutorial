/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

 /**
 *  @file    libkshark.h
 *  @brief   API for processing of FTRACE (trace-cmd) data.
 */

#ifndef _LIB_KSHARK_H
#define _LIB_KSHARK_H

// C
#include <stdint.h>
#include <pthread.h>
#include <errno.h>

// Json-C
#include <json-c/json.h>

#ifdef __cplusplus
extern "C" {
#endif

// trace-cmd
#include "trace-cmd/trace-cmd.h"
#include "private/trace-filter-hash.h"
#include "traceevent/event-parse.h"
#include "tracefs/tracefs.h"

// KernelShark
#include "libkshark-plugin.h"

/**
 * Kernel Shark entry contains all information from one trace record needed
 * in order to  visualize the time-series of trace records. The part of the
 * data which is not directly required for the visualization (latency, record
 * info etc.) is available on-demand via the offset into the trace file.
 */
struct kshark_entry {
	/** Pointer to the next (in time) kshark_entry on the same CPU core. */
	struct kshark_entry *next; /* MUST BE FIRST ENTRY */

	/**
	 * A bit mask controlling the visibility of the entry. A value of OxFF
	 * would mean that the entry is visible everywhere. Use
	 * kshark_filter_masks to check the level of visibility/invisibility
	 * of the entry.
	 */
	uint16_t	visible;

	/** The CPU core of the record. */
	int16_t		cpu;

	/** The PID of the task the record was generated. */
	int32_t		pid;

	/** Unique Id ot the trace event type. */
	int32_t		event_id;

	/** The offset into the trace file, used to find the record. */
	uint64_t	offset;

	/**
	 * The time of the record in nano seconds. The value is taken from
	 * the timestamps within the trace data file, which are architecture
	 * dependent. The time usually is the timestamp from when the system
	 * started.
	 */
	uint64_t	ts;
};

/** Size of the task's hash table. */
#define KS_TASK_HASH_SHIFT 16
#define KS_TASK_HASH_SIZE (1 << KS_TASK_HASH_SHIFT)

/** Linked list of tasks. */
struct kshark_task_list {
	/** Pointer to the next task's PID. */
	struct kshark_task_list	*next;

	/** PID of a task. */
	int			 pid;
};

/** Structure representing a kshark session. */
struct kshark_context {
	/** Input handle for the trace data file. */
	struct tracecmd_input	*handle;

	/** Page event used to parse the page. */
	struct tep_handle	*pevent;

	/** Hash table of task PIDs. */
	struct kshark_task_list	*tasks[KS_TASK_HASH_SIZE];

	/** A mutex, used to protect the access to the input file. */
	pthread_mutex_t		input_mutex;

	/** Hash of tasks to filter on. */
	struct tracecmd_filter_id	*show_task_filter;

	/** Hash of tasks to not display. */
	struct tracecmd_filter_id	*hide_task_filter;

	/** Hash of events to filter on. */
	struct tracecmd_filter_id	*show_event_filter;

	/** Hash of events to not display. */
	struct tracecmd_filter_id	*hide_event_filter;

	/** Hash of CPUs to filter on. */
	struct tracecmd_filter_id	*show_cpu_filter;

	/** Hash of CPUs to not display. */
	struct tracecmd_filter_id	*hide_cpu_filter;

	/**
	 * Bit mask, controlling the visibility of the entries after filtering.
	 * If given bit is set here, all entries which are filtered-out will
	 * have this bit unset in their "visible" fields.
	 */
	uint8_t				filter_mask;

	/**
	 * Filter allowing sophisticated filtering based on the content of
	 * the event.
	 */
	struct tep_event_filter		*advanced_event_filter;

	/** List of Data collections. */
	struct kshark_entry_collection *collections;

	/** List of Plugins. */
	struct kshark_plugin_list	*plugins;

	/** List of Plugin Event handlers. */
	struct kshark_event_handler	*event_handlers;
};

bool kshark_instance(struct kshark_context **kshark_ctx);

bool kshark_open(struct kshark_context *kshark_ctx, const char *file);

ssize_t kshark_load_data_entries(struct kshark_context *kshark_ctx,
				 struct kshark_entry ***data_rows);

ssize_t kshark_load_data_records(struct kshark_context *kshark_ctx,
				 struct tep_record ***data_rows);

size_t kshark_load_data_matrix(struct kshark_context *kshark_ctx,
			       uint64_t **offset_array,
			       uint16_t **cpu_array,
			       uint64_t **ts_array,
			       uint16_t **pid_array,
			       int **event_array);

ssize_t kshark_get_task_pids(struct kshark_context *kshark_ctx, int **pids);

void kshark_close(struct kshark_context *kshark_ctx);

void kshark_free(struct kshark_context *kshark_ctx);

int kshark_get_pid_easy(struct kshark_entry *entry);

const char *kshark_get_task_easy(struct kshark_entry *entry);

const char *kshark_get_latency_easy(struct kshark_entry *entry);

int kshark_get_event_id_easy(struct kshark_entry *entry);

const char *kshark_get_event_name_easy(struct kshark_entry *entry);

const char *kshark_get_info_easy(struct kshark_entry *entry);

void kshark_convert_nano(uint64_t time, uint64_t *sec, uint64_t *usec);

char* kshark_dump_entry(const struct kshark_entry *entry);

/**
 * Custom entry info function type. To be user for dumping info for custom
 * KernelShark entryes.
 */
typedef const char *(kshark_custom_info_func)(struct kshark_context *,
					      const struct kshark_entry *,
					      bool);

char* kshark_dump_custom_entry(struct kshark_context *kshark_ctx,
			       const struct kshark_entry *entry,
			       kshark_custom_info_func info_func);

/** Bit masks used to control the visibility of the entry after filtering. */
enum kshark_filter_masks {
	/**
	 * Use this mask to check the visibility of the entry in the text
	 * view.
	 */
	KS_TEXT_VIEW_FILTER_MASK	= 1 << 0,

	/**
	 * Use this mask to check the visibility of the entry in the graph
	 * view.
	 */
	KS_GRAPH_VIEW_FILTER_MASK	= 1 << 1,

	/** Special mask used whene filtering events. */
	KS_EVENT_VIEW_FILTER_MASK	= 1 << 2,

	/* The next 4 bits are reserved for more KS_X_VIEW_FILTER_MASKs. */

	/**
	 * Use this mask to check if the content of the entry has been accessed
	 * by a plugin-defined function.
	 */
	KS_PLUGIN_UNTOUCHED_MASK	= 1 << 7
};

/** Filter type identifier. */
enum kshark_filter_type {
	/** Dummy filter identifier reserved for future use. */
	KS_NO_FILTER,

	/**
	 * Identifier of the filter, used to specified the events to be shown.
	 */
	KS_SHOW_EVENT_FILTER,

	/**
	 * Identifier of the filter, used to specified the events to be
	 * filtered-out.
	 */
	KS_HIDE_EVENT_FILTER,

	/**
	 * Identifier of the filter, used to specified the tasks to be shown.
	 */
	KS_SHOW_TASK_FILTER,

	/**
	 * Identifier of the filter, used to specified the tasks to be
	 * filtered-out.
	 */
	KS_HIDE_TASK_FILTER,

	/**
	 * Identifier of the filter, used to specified the CPUs to be shown.
	 */
	KS_SHOW_CPU_FILTER,

	/**
	 * Identifier of the filter, used to specified the CPUs to be
	 * filtered-out.
	 */
	KS_HIDE_CPU_FILTER,
};

void kshark_filter_add_id(struct kshark_context *kshark_ctx,
			  int filter_id, int id);

void kshark_filter_clear(struct kshark_context *kshark_ctx, int filter_id);

bool kshark_this_filter_is_set(struct tracecmd_filter_id *filter);

bool kshark_filter_is_set(struct kshark_context *kshark_ctx);

void kshark_filter_entries(struct kshark_context *kshark_ctx,
			   struct kshark_entry **data,
			   size_t n_entries);

void kshark_clear_all_filters(struct kshark_context *kshark_ctx,
			      struct kshark_entry **data,
			      size_t n_entries);

/** Search failed identifiers. */
enum kshark_search_failed {
	/** All entries have timestamps greater timestamps. */
	BSEARCH_ALL_GREATER = -1,

	/** All entries have timestamps smaller timestamps. */
	BSEARCH_ALL_SMALLER = -2,
};

/** General purpose Binary search macro. */
#define BSEARCH(h, l, cond) 				\
	({						\
		while (h - l > 1) {			\
			mid = (l + h) / 2;		\
			if (cond)			\
				l = mid;		\
			else				\
				h = mid;		\
		}					\
	})

ssize_t kshark_find_entry_by_time(uint64_t time,
				  struct kshark_entry **data_rows,
				  size_t l, size_t h);

ssize_t kshark_find_record_by_time(uint64_t time,
				   struct tep_record **data_rows,
				   size_t l, size_t h);

bool kshark_match_pid(struct kshark_context *kshark_ctx,
		      struct kshark_entry *e, int pid);

bool kshark_match_cpu(struct kshark_context *kshark_ctx,
		      struct kshark_entry *e, int cpu);

/**
 * Empty bin identifier.
 * KS_EMPTY_BIN is used to reset entire arrays to empty with memset(), thus it
 * must be -1 for that to work.
 */
#define KS_EMPTY_BIN		-1

/** Filtered bin identifier. */
#define KS_FILTERED_BIN		-2

/** Overflow Event identifier. */
#define KS_EVENT_OVERFLOW	(-EOVERFLOW)

/** Matching condition function type. To be user for data requests */
typedef bool (matching_condition_func)(struct kshark_context*,
				       struct kshark_entry*,
				       int);

/**
 * Data request structure, defining the properties of the required
 * kshark_entry.
 */
struct kshark_entry_request {
	/** Pointer to the next Data request. */
	struct kshark_entry_request *next;

	/**
	 * Array index specifying the position inside the array from where
	 * the search starts.
	 */
	size_t first;

	/** Number of array elements to search in. */
	size_t n;

	/** Matching condition function. */
	matching_condition_func *cond;

	/**
	 * Matching condition value, used by the Matching condition function.
	 */
	int val;

	/** If true, a visible entry is requested. */
	bool vis_only;

	/**
	 * If "vis_only" is true, use this mask to specify the level of
	 * visibility of the requested entry.
	 */
	uint8_t vis_mask;
};

struct kshark_entry_request *
kshark_entry_request_alloc(size_t first, size_t n,
			   matching_condition_func cond, int val,
			   bool vis_only, int vis_mask);

void kshark_free_entry_request(struct kshark_entry_request *req);

const struct kshark_entry *
kshark_get_entry_front(const struct kshark_entry_request *req,
		       struct kshark_entry **data,
		       ssize_t *index);

const struct kshark_entry *
kshark_get_entry_back(const struct kshark_entry_request *req,
		      struct kshark_entry **data,
		      ssize_t *index);

/**
 * Data collections are used to optimize the search for an entry having an
 * abstract property, defined by a Matching condition function and a value.
 * When a collection is processed, the data which is relevant for the
 * collection is enclosed in "Data intervals", defined by pairs of "Resume" and
 * "Break" points. It is guaranteed that the data outside of the intervals
 * contains no entries satisfying the abstract matching condition. However, the
 * intervals may (will) contain data that do not satisfy the matching condition.
 * Once defined, the Data collection can be used when searching for an entry
 * having the same (ore related) abstract property. The collection allows to
 * ignore the irrelevant data, thus it eliminates the linear worst-case time
 * complexity of the search.
 */
struct kshark_entry_collection {
	/** Pointer to the next Data collection. */
	struct kshark_entry_collection *next;

	/** Matching condition function, used to define the collections. */
	matching_condition_func *cond;

	/**
	 * Matching condition value, used by the Matching condition finction
	 * to define the collections.
	 */
	int val;

	/**
	 * Array of indexes defining the beginning of each individual data
	 * interval.
	 */
	size_t *resume_points;

	/**
	 * Array of indexes defining the end of each individual data interval.
	 */
	size_t *break_points;

	/** Number of data intervals in this collection. */
	size_t size;
};

struct kshark_entry_collection *
kshark_add_collection_to_list(struct kshark_context *kshark_ctx,
			      struct kshark_entry_collection **col_list,
			      struct kshark_entry **data,
			      size_t n_rows,
			      matching_condition_func cond,
			      int val,
			      size_t margin);

struct kshark_entry_collection *
kshark_register_data_collection(struct kshark_context *kshark_ctx,
				struct kshark_entry **data, size_t n_rows,
				matching_condition_func cond, int val,
				size_t margin);

void kshark_unregister_data_collection(struct kshark_entry_collection **col,
				       matching_condition_func cond,
				       int val);

struct kshark_entry_collection *
kshark_find_data_collection(struct kshark_entry_collection *col,
			    matching_condition_func cond,
			    int val);

void kshark_reset_data_collection(struct kshark_entry_collection *col);

void kshark_free_collection_list(struct kshark_entry_collection *col);

const struct kshark_entry *
kshark_get_collection_entry_front(struct kshark_entry_request *req,
				  struct kshark_entry **data,
				  const struct kshark_entry_collection *col,
				  ssize_t *index);

const struct kshark_entry *
kshark_get_collection_entry_back(struct kshark_entry_request *req,
				 struct kshark_entry **data,
				 const struct kshark_entry_collection *col,
				 ssize_t *index);

/** Structure representing a KernelShark Configuration document. */
struct kshark_config_doc {
	/** Document format identifier. */
	int	format;

	/** Configuration document instance. */
	void	*conf_doc;
};

/** Configuration format identifiers. */
enum kshark_config_formats {
	/** Unformatted Configuration document identifier. */
	KS_CONFIG_AUTO = 0,

	/**
	 * String Configuration document identifier. The String format is
	 * meant to be used only by kshark_config_doc_add() and
	 * kshark_config_doc_get(), when adding/getting simple string fields.
	 */
	KS_CONFIG_STRING,

	/** Json Configuration document identifier. */
	KS_CONFIG_JSON,
};

/**
 * Field name for the Configuration document describing the Hide Event filter.
 */
#define KS_HIDE_EVENT_FILTER_NAME	"hide event filter"

/**
 * Field name for the Configuration document describing the Show Event filter.
 */
#define KS_SHOW_EVENT_FILTER_NAME	"show event filter"

/**
 * Field name for the Configuration document describing the Hide Task filter.
 */
#define KS_HIDE_TASK_FILTER_NAME	"hide task filter"

/**
 * Field name for the Configuration document describing the Show Task filter.
 */
#define KS_SHOW_TASK_FILTER_NAME	"show task filter"

/**
 * Field name for the Configuration document describing the Hide Task filter.
 */
#define KS_HIDE_CPU_FILTER_NAME		"hide cpu filter"

/**
 * Field name for the Configuration document describing the Show Task filter.
 */
#define KS_SHOW_CPU_FILTER_NAME		"show cpu filter"

/**
 * Field name for the Configuration document describing the Advanced event
 * filter.
 */
#define KS_ADV_EVENT_FILTER_NAME	"adv event filter"

/**
 * Field name for the Configuration document describing user-specified filter
 * mask.
 */
#define KS_USER_FILTER_MASK_NAME	"filter mask"
/**
 * Field name for the Configuration document describing the state of the Vis.
 * model.
 */
#define KS_HISTO_NAME			"vis. model"

/**
 * Field name for the Configuration document describing the currently loaded
 * trace data file.
 */
#define KS_DATA_SOURCE_NAME		"trace data"

struct kshark_config_doc *
kshark_config_alloc(enum kshark_config_formats);

struct kshark_config_doc *
kshark_config_new(const char *type, enum kshark_config_formats);

void kshark_free_config_doc(struct kshark_config_doc *conf);

struct kshark_config_doc *
kshark_record_config_new(enum kshark_config_formats);

struct kshark_config_doc *
kshark_filter_config_new(enum kshark_config_formats);

struct kshark_config_doc *kshark_string_config_alloc(void);

bool kshark_type_check(struct kshark_config_doc *conf, const char *type);

bool kshark_config_doc_add(struct kshark_config_doc *conf,
			   const char *key,
			   struct kshark_config_doc *val);

bool kshark_config_doc_get(struct kshark_config_doc *conf,
			   const char *key,
			   struct kshark_config_doc *val);

struct kshark_trace_histo;

struct kshark_config_doc *
kshark_export_trace_file(const char *file,
			 enum kshark_config_formats format);

const char *kshark_import_trace_file(struct kshark_context *kshark_ctx,
				     struct kshark_config_doc *conf);

struct kshark_config_doc *
kshark_export_model(struct kshark_trace_histo *histo,
		     enum kshark_config_formats format);


bool kshark_import_model(struct kshark_trace_histo *histo,
			 struct kshark_config_doc *conf);

bool kshark_export_adv_filters(struct kshark_context *kshark_ctx,
			       struct kshark_config_doc **conf);

bool kshark_import_adv_filters(struct kshark_context *kshark_ctx,
			       struct kshark_config_doc *conf);

bool kshark_export_event_filter(struct tep_handle *pevent,
				struct tracecmd_filter_id *filter,
				const char *filter_name,
				struct kshark_config_doc *conf);

int kshark_import_event_filter(struct tep_handle *pevent,
			       struct tracecmd_filter_id *filter,
			       const char *filter_name,
			       struct kshark_config_doc *conf);

bool kshark_export_user_mask(struct kshark_context *kshark_ctx,
			     struct kshark_config_doc **conf);

bool kshark_import_user_mask(struct kshark_context *kshark_ctx,
			     struct kshark_config_doc *conf);

bool kshark_export_filter_array(struct tracecmd_filter_id *filter,
				const char *filter_name,
				struct kshark_config_doc *conf);

bool kshark_import_filter_array(struct tracecmd_filter_id *filter,
				const char *filter_name,
				struct kshark_config_doc *conf);

bool kshark_export_all_event_filters(struct kshark_context *kshark_ctx,
				     struct kshark_config_doc **conf);

bool kshark_export_all_task_filters(struct kshark_context *kshark_ctx,
				    struct kshark_config_doc **conf);

bool kshark_export_all_cpu_filters(struct kshark_context *kshark_ctx,
				   struct kshark_config_doc **conf);

struct kshark_config_doc *
kshark_export_all_filters(struct kshark_context *kshark_ctx,
			  enum kshark_config_formats format);

bool kshark_import_all_event_filters(struct kshark_context *kshark_ctx,
				     struct kshark_config_doc *conf);

bool kshark_import_all_task_filters(struct kshark_context *kshark_ctx,
				    struct kshark_config_doc *conf);

bool kshark_import_all_cpu_filters(struct kshark_context *kshark_ctx,
				   struct kshark_config_doc *conf);

bool kshark_import_all_filters(struct kshark_context *kshark_ctx,
			       struct kshark_config_doc *conf);

bool kshark_save_config_file(const char *file_name,
			     struct kshark_config_doc *conf);

struct kshark_config_doc *kshark_open_config_file(const char *file_name,
						  const char *type);

struct kshark_config_doc *kshark_json_to_conf(struct json_object *jobj);

#ifdef __cplusplus
}
#endif

#endif // _LIB_KSHARK_H
