/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

 /**
  *  @file    libkshark-model.h
  *  @brief   Visualization model for FTRACE (trace-cmd) data.
  */

#ifndef _LIB_KSHARK_MODEL_H
#define _LIB_KSHARK_MODEL_H

// KernelShark
#include "libkshark.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Overflow Bin identifiers. The two overflow bins are used to hold the data
 * outside the visualized range.
 */
enum OverflowBin {
	/**
	 * Identifier of the Upper Overflow Bin. This bin is used to hold the
	 * data after (in time) the end of the visualized range.
	 */
	UPPER_OVERFLOW_BIN = -1,

	/**
	 * Identifier of the Lower Overflow Bin. This bin is used to hold the
	 * data before (in time) the beginning of the visualized range.
	 */
	LOWER_OVERFLOW_BIN = -2,
};

/** Structure describing the current state of the visualization model. */
struct kshark_trace_histo {
	/** Trace data array. */
	struct kshark_entry	**data;

	/** The size of the data array. */
	size_t			data_size;

	/** The first entry (index of data array) in each bin. */
	ssize_t			*map;

	/** Number of entries in each bin. */
	size_t			*bin_count;

	/** Total number of entries in all bin except the overflow bins. */
	int			tot_count;

	/**
	 * Lower edge of the time-window to be visualized. Only entries having
	 * timestamp >= min will be visualized.
	 */
	uint64_t		min;

	/**
	 * Upper edge of the time-window to be visualized. Only entries having
	 * timestamp <= max will be visualized.
	 */
	uint64_t		max;

	/** The size in time for each bin. */
	uint64_t		bin_size;

	/** Number of bins. */
	int			n_bins;
};

void ksmodel_init(struct kshark_trace_histo *histo);

void ksmodel_clear(struct kshark_trace_histo *histo);

void ksmodel_set_bining(struct kshark_trace_histo *histo,
			size_t n, uint64_t min, uint64_t max);

void ksmodel_fill(struct kshark_trace_histo *histo,
		  struct kshark_entry **data, size_t n);

size_t ksmodel_bin_count(struct kshark_trace_histo *histo, int bin);

void ksmodel_shift_forward(struct kshark_trace_histo *histo, size_t n);

void ksmodel_shift_backward(struct kshark_trace_histo *histo, size_t n);

void ksmodel_jump_to(struct kshark_trace_histo *histo, uint64_t ts);

void ksmodel_zoom_out(struct kshark_trace_histo *histo,
		      double r, int mark);

void ksmodel_zoom_in(struct kshark_trace_histo *histo,
		     double r, int mark);

ssize_t ksmodel_first_index_at_bin(struct kshark_trace_histo *histo, int bin);

ssize_t ksmodel_last_index_at_bin(struct kshark_trace_histo *histo, int bin);

ssize_t ksmodel_first_index_at_cpu(struct kshark_trace_histo *histo,
				   int bin, int cpu);

ssize_t ksmodel_first_index_at_pid(struct kshark_trace_histo *histo,
				   int bin, int pid);

const struct kshark_entry *
ksmodel_get_entry_front(struct kshark_trace_histo *histo,
			int bin, bool vis_only,
			matching_condition_func func, int val,
			struct kshark_entry_collection *col,
			ssize_t *index);

const struct kshark_entry *
ksmodel_get_entry_back(struct kshark_trace_histo *histo,
		       int bin, bool vis_only,
		       matching_condition_func func, int val,
		       struct kshark_entry_collection *col,
		       ssize_t *index);

int ksmodel_get_pid_front(struct kshark_trace_histo *histo,
			  int bin, int cpu, bool vis_only,
			  struct kshark_entry_collection *col,
			  ssize_t *index);

int ksmodel_get_pid_back(struct kshark_trace_histo *histo,
			 int bin, int cpu, bool vis_only,
			 struct kshark_entry_collection *col,
			 ssize_t *index);

int ksmodel_get_cpu_front(struct kshark_trace_histo *histo,
			  int bin, int pid, bool vis_only,
			  struct kshark_entry_collection *col,
			  ssize_t *index);

int ksmodel_get_cpu_back(struct kshark_trace_histo *histo,
			 int bin, int pid, bool vis_only,
			 struct kshark_entry_collection *col,
			 ssize_t *index);

bool ksmodel_cpu_visible_event_exist(struct kshark_trace_histo *histo,
				     int bin, int cpu,
				     struct kshark_entry_collection *col,
				     ssize_t *index);

bool ksmodel_task_visible_event_exist(struct kshark_trace_histo *histo,
				      int bin, int pid,
				      struct kshark_entry_collection *col,
				      ssize_t *index);

const struct kshark_entry *
ksmodel_get_cpu_missed_events(struct kshark_trace_histo *histo,
			      int bin, int cpu,
			      struct kshark_entry_collection *col,
			      ssize_t *index);

const struct kshark_entry *
ksmodel_get_task_missed_events(struct kshark_trace_histo *histo,
			       int bin, int pid,
			       struct kshark_entry_collection *col,
			       ssize_t *index);

static inline double ksmodel_bin_time(struct kshark_trace_histo *histo,
				      int bin)
{
	return (histo->min + bin*histo->bin_size) * 1e-9;
}

static inline uint64_t ksmodel_bin_ts(struct kshark_trace_histo *histo,
				      int bin)
{
	return (histo->min + bin*histo->bin_size);
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
