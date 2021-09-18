/* SPDX-License-Identifier: LGPL-2.1 */
/*
 * Copyright (C) 2019, VMware, Tzvetomir Stoyanov <tz.stoyanov@gmail.com>
 *
 */
#ifndef _TRACE_TSYNC_LOCAL_H
#define _TRACE_TSYNC_LOCAL_H

#include <stdbool.h>

struct tracecmd_time_sync {
	pthread_t			thread;
	bool				thread_running;
	unsigned long long		trace_id;
	char				*proto_name;
	int				loop_interval;
	pthread_mutex_t			lock;
	pthread_cond_t			cond;
	pthread_barrier_t		first_sync;
	char				*clock_str;
	struct tracecmd_msg_handle	*msg_handle;
	void				*context;
	int				guest_pid;
	int				vcpu_count;
};

struct clock_sync_context {
	void				*proto_data;	/* time sync protocol specific data */
	bool				is_server;	/* server side time sync role */
	bool				is_guest;	/* guest or host time sync role */
	struct tracefs_instance		*instance;	/* ftrace buffer, used for time sync events */

	/* Arrays with calculated time offsets at given time */
	int				sync_size;	/* Allocated size of sync_ts,
							 * sync_offsets and sync_scalings
							 */
	int				sync_count;	/* Number of elements in sync_ts,
							 * sync_offsets and sync_scalings
							 */
	long long			*sync_ts;
	long long			*sync_offsets;
	long long			*sync_scalings;

	/* Identifiers of local and remote time sync peers: cid and port */
	unsigned int			local_cid;
	unsigned int			local_port;
	unsigned int			remote_cid;
	unsigned int			remote_port;
};

int tracecmd_tsync_proto_register(const char *proto_name, int accuracy, int roles,
				  int supported_clocks, unsigned int flags,
				  int (*init)(struct tracecmd_time_sync *),
				  int (*free)(struct tracecmd_time_sync *),
				  int (*calc)(struct tracecmd_time_sync *,
					      long long *, long long *, long long *));
int tracecmd_tsync_proto_unregister(char *proto_name);

int ptp_clock_sync_register(void);

#endif /* _TRACE_TSYNC_LOCAL_H */
