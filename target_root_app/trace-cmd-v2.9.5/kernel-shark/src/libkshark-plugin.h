/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2016 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 */

 /**
  *  @file    libkshark-plugin.h
  *  @brief   KernelShark plugins.
  */

#ifndef _KSHARK_PLUGIN_H
#define _KSHARK_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// trace-cmd
#include "traceevent/event-parse.h"

/* Quiet warnings over documenting simple structures */
//! @cond Doxygen_Suppress

#define KSHARK_PLUGIN_INITIALIZER kshark_plugin_initializer

#define KSHARK_PLUGIN_DEINITIALIZER kshark_plugin_deinitializer

#define _MAKE_STR(x)	#x
#define MAKE_STR(x)	_MAKE_STR(x)

#define KSHARK_PLUGIN_INITIALIZER_NAME MAKE_STR(KSHARK_PLUGIN_INITIALIZER)

#define KSHARK_PLUGIN_DEINITIALIZER_NAME MAKE_STR(KSHARK_PLUGIN_DEINITIALIZER)

struct kshark_context;

struct kshark_entry;

//! @endcond

/**
 * A function type to be used when defining load/reload/unload plugin
 * functions.
 */
typedef int (*kshark_plugin_load_func)(struct kshark_context *);

struct kshark_trace_histo;

/**
 * Structure representing the C arguments of the drawing function of
 * a plugin.
 */
struct kshark_cpp_argv {
	/** Pointer to the model descriptor object. */
	struct kshark_trace_histo	*histo;
};

/** A function type to be used when defining plugin functions for drawing. */
typedef void
(*kshark_plugin_draw_handler_func)(struct kshark_cpp_argv *argv,
				   int val, int draw_action);

/**
 * A function type to be used when defining plugin functions for data
 * manipulation.
 */
typedef void
(*kshark_plugin_event_handler_func)(struct kshark_context *kshark_ctx,
				    struct tep_record *rec,
				    struct kshark_entry *e);

/** Plugin action identifier. */
enum kshark_plugin_actions {
	/**
	 * Load plugins action. This action identifier is used when handling
	 * plugins.
	 */
	KSHARK_PLUGIN_INIT,

	/**
	 * Reload plugins action. This action identifier is used when handling
	 * plugins.
	 */
	KSHARK_PLUGIN_UPDATE,

	/**
	 * Unload plugins action. This action identifier is used when handling
	 * plugins.
	 */
	KSHARK_PLUGIN_CLOSE,

	/**
	 * Task draw action. This action identifier is used by the plugin draw
	 * function.
	 */
	KSHARK_PLUGIN_TASK_DRAW,

	/**
	 * CPU draw action. This action identifier is used by the plugin draw
	 * function.
	 */
	KSHARK_PLUGIN_CPU_DRAW,
};

/**
 * Plugin Event handler structure, defining the properties of the required
 * kshark_entry.
 */
struct kshark_event_handler {
	/** Pointer to the next Plugin Event handler. */
	struct kshark_event_handler		*next;

	/** Unique Id ot the trace event type. */
	int					id;

	/**
	 * Event action function. This action can be used to modify the content
	 * of all kshark_entries having Event Ids equal to "id".
	 */
	kshark_plugin_event_handler_func	event_func;

	/**
	 * Draw action function. This action can be used to draw additional
	 * graphical elements (shapes) for all kshark_entries having Event Ids
	 * equal to "id".
	 */
	kshark_plugin_draw_handler_func		draw_func;
};

struct kshark_event_handler *
kshark_find_event_handler(struct kshark_event_handler *handlers, int event_id);

int kshark_register_event_handler(struct kshark_event_handler **handlers,
				  int event_id,
				  kshark_plugin_event_handler_func evt_func,
				  kshark_plugin_draw_handler_func dw_func);

void kshark_unregister_event_handler(struct kshark_event_handler **handlers,
				     int event_id,
				     kshark_plugin_event_handler_func evt_func,
				     kshark_plugin_draw_handler_func dw_func);

void kshark_free_event_handler_list(struct kshark_event_handler *handlers);

/** Linked list of plugins. */
struct kshark_plugin_list {
	/** Pointer to the next Plugin. */
	struct kshark_plugin_list	*next;

	/** The plugin object file to load. */
	char				*file;

	/** Plugin Event handler. */
	void				*handle;

	/** Callback function for initialization of the plugin. */
	kshark_plugin_load_func		init;

	/** Callback function for deinitialization of the plugin. */
	kshark_plugin_load_func		close;
};

int kshark_register_plugin(struct kshark_context *kshark_ctx,
			   const char *file);

void kshark_unregister_plugin(struct kshark_context *kshark_ctx,
			      const char *file);

void kshark_free_plugin_list(struct kshark_plugin_list *plugins);

int kshark_handle_plugins(struct kshark_context *kshark_ctx,
			  enum kshark_plugin_actions  task_id);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _KSHARK_PLUGIN_H
