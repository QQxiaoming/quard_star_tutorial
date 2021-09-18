/* SPDX-License-Identifier: LGPL-2.1 */
/*
 * Copyright (C) 2019, VMware, Tzvetomir Stoyanov <tz.stoyanov@gmail.com>
 *
 */
#ifndef _TRACE_FS_H
#define _TRACE_FS_H

#include <sched.h>
#include "event-parse.h"

char *tracefs_get_tracing_file(const char *name);
void tracefs_put_tracing_file(char *name);

/* The returned string must *not* be freed */
const char *tracefs_tracing_dir(void);

/* ftarce instances */
struct tracefs_instance;

void tracefs_instance_free(struct tracefs_instance *instance);
struct tracefs_instance *tracefs_instance_create(const char *name);
int tracefs_instance_destroy(struct tracefs_instance *instance);
bool tracefs_instance_is_new(struct tracefs_instance *instance);
const char *tracefs_instance_get_name(struct tracefs_instance *instance);
char *
tracefs_instance_get_file(struct tracefs_instance *instance, const char *file);
char *tracefs_instance_get_dir(struct tracefs_instance *instance);
int tracefs_instance_file_write(struct tracefs_instance *instance,
				const char *file, const char *str);
char *tracefs_instance_file_read(struct tracefs_instance *instance,
				 char *file, int *psize);
int tracefs_instances_walk(int (*callback)(const char *, void *), void *context);

bool tracefs_instance_exists(const char *name);
bool tracefs_file_exists(struct tracefs_instance *instance, char *name);
bool tracefs_dir_exists(struct tracefs_instance *instance, char *name);

/* events */
void tracefs_list_free(char **list);
char **tracefs_event_systems(const char *tracing_dir);
char **tracefs_system_events(const char *tracing_dir, const char *system);
int tracefs_iterate_raw_events(struct tep_handle *tep,
				struct tracefs_instance *instance,
				cpu_set_t *cpus, int cpu_size,
				int (*callback)(struct tep_event *,
						struct tep_record *,
						int, void *),
				void *callback_context);

char **tracefs_tracers(const char *tracing_dir);

struct tep_handle *tracefs_local_events(const char *tracing_dir);
struct tep_handle *tracefs_local_events_system(const char *tracing_dir,
					       const char * const *sys_names);
int tracefs_fill_local_events(const char *tracing_dir,
			       struct tep_handle *tep, int *parsing_failures);

char *tracefs_get_clock(struct tracefs_instance *instance);

#endif /* _TRACE_FS_H */
