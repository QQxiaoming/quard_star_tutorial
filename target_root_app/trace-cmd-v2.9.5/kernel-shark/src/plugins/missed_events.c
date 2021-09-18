// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    missed_events.c
 *  @brief   Plugin for visualization of missed events due to overflow of the
 *	     ring buffer.
 */

// KernelShark
#include "plugins/missed_events.h"

static void nop_action(struct kshark_context *kshark_ctx,
				struct tep_record *record,
				struct kshark_entry *entry)
{}

/** Load this plugin. */
int KSHARK_PLUGIN_INITIALIZER(struct kshark_context *kshark_ctx)
{
	kshark_register_event_handler(&kshark_ctx->event_handlers,
				      KS_EVENT_OVERFLOW,
				      nop_action,
				      draw_missed_events);

	return 1;
}

/** Unload this plugin. */
int KSHARK_PLUGIN_DEINITIALIZER(struct kshark_context *kshark_ctx)
{
	kshark_unregister_event_handler(&kshark_ctx->event_handlers,
					KS_EVENT_OVERFLOW,
					nop_action,
					draw_missed_events);

	return 1;
}
