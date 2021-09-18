/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <ykaradzov@vmware.com>
 */

/**
 *  @file    missed_events.h
 *  @brief   Plugin for visualization of missed events due to overflow of the
 *	     ring buffer.
 */

#ifndef _KS_PLUGIN_M_EVTS_H
#define _KS_PLUGIN_M_EVTS_H

// KernelShark
#include "libkshark.h"

#ifdef __cplusplus
extern "C" {
#endif

void draw_missed_events(struct kshark_cpp_argv *argv,
			int pid, int draw_action);

#ifdef __cplusplus
}
#endif

#endif
