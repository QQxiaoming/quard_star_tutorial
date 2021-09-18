/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

/**
 *  @file    sched_events.h
 *  @brief   Plugin for Sched events.
 */

#ifndef _KS_PLUGIN_SHED_H
#define _KS_PLUGIN_SHED_H

// KernelShark
#include "libkshark.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Structure representing a plugin-specific context. */
struct plugin_sched_context {
	/** Input handle for the trace data file. */
	struct tracecmd_input	*handle;

	/** Page event used to parse the page. */
	struct tep_handle	*pevent;

	/** Pointer to the sched_switch_event object. */
	struct tep_event	*sched_switch_event;

	/** Pointer to the sched_switch_next_field format descriptor. */
	struct tep_format_field	*sched_switch_next_field;

	/** Pointer to the sched_switch_comm_field format descriptor. */
	struct tep_format_field	*sched_switch_comm_field;

	/** Pointer to the sched_switch_prev_state_field format descriptor. */
	struct tep_format_field	*sched_switch_prev_state_field;

	/** Pointer to the sched_wakeup_event object. */
	struct tep_event	*sched_wakeup_event;

	/** Pointer to the sched_wakeup_pid_field format descriptor. */
	struct tep_format_field	*sched_wakeup_pid_field;

	/** Pointer to the sched_wakeup_new_event object. */
	struct tep_event	*sched_wakeup_new_event;

	/** Pointer to the sched_wakeup_new_pid_field format descriptor. */
	struct tep_format_field	*sched_wakeup_new_pid_field;

	/** Pointer to the sched_waking_event object. */
	struct tep_event        *sched_waking_event;

	/** Pointer to the sched_waking_pid_field format descriptor. */
	struct tep_format_field *sched_waking_pid_field;

	/** List of Data collections used by this plugin. */
	struct kshark_entry_collection	*collections;

	/** Hash of the tasks for which the second pass is already done. */
	struct tracecmd_filter_id	*second_pass_hash;
};

int plugin_get_next_pid(struct tep_record *record);

bool plugin_wakeup_match_rec_pid(struct kshark_context *kshark_ctx,
				 struct kshark_entry *e, int pid);

bool plugin_switch_match_rec_pid(struct kshark_context *kshark_ctx,
				 struct kshark_entry *e, int pid);

bool plugin_switch_match_entry_pid(struct kshark_context *kshark_ctx,
				   struct kshark_entry *e,
				   int pid);

bool plugin_match_pid(struct kshark_context *kshark_ctx,
		      struct kshark_entry *e, int pid);

void plugin_draw(struct kshark_cpp_argv *argv, int pid, int draw_action);

#ifdef __cplusplus
}
#endif

#endif
