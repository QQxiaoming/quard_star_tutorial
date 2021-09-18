// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2019, VMware, Tzvetomir Stoyanov <tz.stoyanov@gmail.com>
 *
 */

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#ifdef VSOCK
#include <linux/vm_sockets.h>
#endif
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>

#include "trace-cmd-private.h"
#include "tracefs.h"
#include "event-utils.h"
#include "trace-tsync-local.h"

struct tsync_proto {
	struct tsync_proto *next;
	char proto_name[TRACECMD_TSYNC_PNAME_LENGTH];
	enum tracecmd_time_sync_role roles;
	int accuracy;
	int supported_clocks;
	unsigned int flags;

	int (*clock_sync_init)(struct tracecmd_time_sync *clock_context);
	int (*clock_sync_free)(struct tracecmd_time_sync *clock_context);
	int (*clock_sync_calc)(struct tracecmd_time_sync *clock_context,
			       long long *offset, long long *scaling,
			       long long *timestamp);
};

static struct tsync_proto *tsync_proto_list;

static struct tsync_proto *tsync_proto_find(const char *proto_name)
{
	struct tsync_proto *proto;

	if (!proto_name)
		return NULL;
	for (proto = tsync_proto_list; proto; proto = proto->next) {
		if (strlen(proto->proto_name) == strlen(proto_name) &&
		     !strncmp(proto->proto_name, proto_name, TRACECMD_TSYNC_PNAME_LENGTH))
			return proto;
	}
	return NULL;
}

int tracecmd_tsync_proto_register(const char *proto_name, int accuracy, int roles,
				  int supported_clocks, unsigned int flags,
				  int (*init)(struct tracecmd_time_sync *),
				  int (*free)(struct tracecmd_time_sync *),
				  int (*calc)(struct tracecmd_time_sync *,
					      long long *, long long *, long long *))
{
	struct tsync_proto *proto = NULL;

	if (tsync_proto_find(proto_name))
		return -1;
	proto = calloc(1, sizeof(struct tsync_proto));
	if (!proto)
		return -1;
	strncpy(proto->proto_name, proto_name, TRACECMD_TSYNC_PNAME_LENGTH);
	proto->accuracy = accuracy;
	proto->roles = roles;
	proto->supported_clocks = supported_clocks;
	proto->clock_sync_init = init;
	proto->clock_sync_free = free;
	proto->clock_sync_calc = calc;

	proto->next = tsync_proto_list;
	tsync_proto_list = proto;
	return 0;
}

int tracecmd_tsync_proto_unregister(char *proto_name)
{
	struct tsync_proto **last = &tsync_proto_list;

	if (!proto_name)
		return -1;

	for (; *last; last = &(*last)->next) {
		if (strlen((*last)->proto_name) == strlen(proto_name) &&
		    !strncmp((*last)->proto_name, proto_name, TRACECMD_TSYNC_PNAME_LENGTH)) {
			struct tsync_proto *proto = *last;

			*last = proto->next;
			free(proto);
			return 0;
		}
	}

	return -1;
}

bool tsync_proto_is_supported(const char *proto_name)
{
	if (tsync_proto_find(proto_name))
		return true;
	return false;
}

/**
 * tracecmd_tsync_get_offsets - Return the calculated time offsets
 *
 * @tsync: Pointer to time sync context
 * @count: Returns the number of calculated time offsets
 * @ts: Array of size @count containing timestamps of callculated offsets
 * @offsets: array of size @count, containing offsets for each timestamp
 * @scalings: array of size @count, containing scaling ratios for each timestamp
 *
 * Retuns -1 in case of an error, or 0 otherwise
 */
int tracecmd_tsync_get_offsets(struct tracecmd_time_sync *tsync,
				int *count, long long **ts,
				long long **offsets, long long **scalings)
{
	struct clock_sync_context *tsync_context;

	if (!tsync || !tsync->context)
		return -1;
	tsync_context = (struct clock_sync_context *)tsync->context;
	if (count)
		*count = tsync_context->sync_count;
	if (ts)
		*ts = tsync_context->sync_ts;
	if (offsets)
		*offsets = tsync_context->sync_offsets;
	if (scalings)
		*scalings = tsync_context->sync_scalings;

	return 0;
}

/**
 * tsync_get_proto_flags - Get protocol flags
 *
 * @tsync: Pointer to time sync context
 * @flags: Returns the protocol flags, a combination of TRACECMD_TSYNC_FLAG_...
 *
 * Retuns -1 in case of an error, or 0 otherwise
 */
static int tsync_get_proto_flags(struct tracecmd_time_sync *tsync,
				 unsigned int *flags)
{
	struct tsync_proto *protocol;

	if (!tsync)
		return -1;
	protocol = tsync_proto_find(tsync->proto_name);
	if (!protocol)
		return -1;

	if (flags)
		*flags = protocol->flags;

	return 0;
}


#define PROTO_MASK_SIZE (sizeof(char))
#define PROTO_MASK_BITS (PROTO_MASK_SIZE * 8)
/**
 * tsync_proto_select - Select time sync protocol, to be used for
 *		timestamp synchronization with a peer
 *
 * @protos: list of tsync protocol names
 * @clock : trace clock
 * @role : local time sync role
 *
 * Retuns pointer to a protocol name, that can be used with the peer, or NULL
 *	  in case there is no match with supported protocols.
 *	  The returned string MUST NOT be freed by the caller
 */
static const char *
tsync_proto_select(const struct tracecmd_tsync_protos *protos,
		   const char *clock, enum tracecmd_time_sync_role role)
{
	struct tsync_proto *selected = NULL;
	struct tsync_proto *proto;
	char **pname;
	int clock_id = 0;

	if (!protos)
		return NULL;

	clock_id = tracecmd_clock_str2id(clock);
	pname = protos->names;
	while (*pname) {
		for (proto = tsync_proto_list; proto; proto = proto->next) {
			if (!(proto->roles & role))
				continue;
			if (proto->supported_clocks && clock_id &&
			    !(proto->supported_clocks & clock_id))
				continue;
			if (strncmp(proto->proto_name, *pname, TRACECMD_TSYNC_PNAME_LENGTH))
				continue;
			if (selected) {
				if (selected->accuracy > proto->accuracy)
					selected = proto;
			} else
				selected = proto;
		}
		pname++;
	}

	if (selected)
		return selected->proto_name;

	return NULL;
}

/**
 * tracecmd_tsync_proto_getall - Returns list of all supported
 *				 time sync protocols
 * @protos: return, allocated list of time sync protocol names,
 *	       supported by the peer. Must be freed by free()
 * @clock: selected trace clock
 * @role: supported protocol role
 *
 * If completed successfully 0 is returned and allocated list of strings in @protos.
 * The last list entry is NULL. In case of an error, -1 is returned.
 * @protos must be freed with free()
 */
int tracecmd_tsync_proto_getall(struct tracecmd_tsync_protos **protos, const char *clock, int role)
{
	struct tracecmd_tsync_protos *plist = NULL;
	struct tsync_proto *proto;
	int clock_id = 0;
	int count = 1;
	int i;

	if (clock)
		clock_id =  tracecmd_clock_str2id(clock);
	for (proto = tsync_proto_list; proto; proto = proto->next) {
		if (!(proto->roles & role))
			continue;
		if (proto->supported_clocks && clock_id &&
		    !(proto->supported_clocks & clock_id))
			continue;
		count++;
	}
	plist = calloc(1, sizeof(struct tracecmd_tsync_protos));
	if (!plist)
		goto error;
	plist->names = calloc(count, sizeof(char *));
	if (!plist->names)
		return -1;

	for (i = 0, proto = tsync_proto_list; proto && i < (count - 1); proto = proto->next) {
		if (!(proto->roles & role))
			continue;
		if (proto->supported_clocks && clock_id &&
		    !(proto->supported_clocks & clock_id))
			continue;
		plist->names[i++] = proto->proto_name;
	}

	*protos = plist;
	return 0;

error:
	if (plist) {
		free(plist->names);
		free(plist);
	}
	return -1;
}

static int get_first_cpu(cpu_set_t **pin_mask, size_t *m_size)
{
	int cpus = tracecmd_count_cpus();
	cpu_set_t *cpu_mask;
	int mask_size;
	int i;

	cpu_mask = CPU_ALLOC(cpus);
	*pin_mask = CPU_ALLOC(cpus);
	if (!cpu_mask || !*pin_mask || 1)
		goto error;

	mask_size = CPU_ALLOC_SIZE(cpus);
	CPU_ZERO_S(mask_size, cpu_mask);
	CPU_ZERO_S(mask_size, *pin_mask);

	if (sched_getaffinity(0, mask_size, cpu_mask) == -1)
		goto error;

	for (i = 0; i < cpus; i++) {
		if (CPU_ISSET_S(i, mask_size, cpu_mask)) {
			CPU_SET_S(i, mask_size, *pin_mask);
			break;
		}
	}

	if (CPU_COUNT_S(mask_size, *pin_mask) < 1)
		goto error;

	CPU_FREE(cpu_mask);
	*m_size = mask_size;
	return 0;

error:
	if (cpu_mask)
		CPU_FREE(cpu_mask);
	if (*pin_mask)
		CPU_FREE(*pin_mask);
	*pin_mask = NULL;
	*m_size = 0;
	return -1;
}

#ifdef VSOCK
static int vsock_open(unsigned int cid, unsigned int port)
{
	struct sockaddr_vm addr = {
		.svm_family = AF_VSOCK,
		.svm_cid = cid,
		.svm_port = port,
	};
	int sd;

	sd = socket(AF_VSOCK, SOCK_STREAM, 0);
	if (sd < 0)
		return -errno;

	if (connect(sd, (struct sockaddr *)&addr, sizeof(addr)))
		return -errno;

	return sd;
}

static int vsock_make(void)
{
	struct sockaddr_vm addr = {
		.svm_family = AF_VSOCK,
		.svm_cid = VMADDR_CID_ANY,
		.svm_port = VMADDR_PORT_ANY,
	};
	int sd;

	sd = socket(AF_VSOCK, SOCK_STREAM, 0);
	if (sd < 0)
		return -errno;

	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)))
		return -errno;

	if (listen(sd, SOMAXCONN))
		return -errno;

	return sd;
}

int vsock_get_port(int sd, unsigned int *port)
{
	struct sockaddr_vm addr;
	socklen_t addr_len = sizeof(addr);

	if (getsockname(sd, (struct sockaddr *)&addr, &addr_len))
		return -errno;

	if (addr.svm_family != AF_VSOCK)
		return -EINVAL;

	if (port)
		*port = addr.svm_port;

	return 0;
}

static int get_vsocket_params(int fd, unsigned int *lcid, unsigned int *lport,
			      unsigned int *rcid, unsigned int *rport)
{
	struct sockaddr_vm addr;
	socklen_t addr_len = sizeof(addr);

	memset(&addr, 0, sizeof(addr));
	if (getsockname(fd, (struct sockaddr *)&addr, &addr_len))
		return -1;
	if (addr.svm_family != AF_VSOCK)
		return -1;
	*lport = addr.svm_port;
	*lcid = addr.svm_cid;

	memset(&addr, 0, sizeof(addr));
	addr_len = sizeof(addr);
	if (getpeername(fd, (struct sockaddr *)&addr, &addr_len))
		return -1;
	if (addr.svm_family != AF_VSOCK)
		return -1;
	*rport = addr.svm_port;
	*rcid = addr.svm_cid;

	return 0;
}

#else
static int vsock_open(unsigned int cid, unsigned int port)
{
	return -ENOTSUP;
}

static int vsock_make(void)
{
	return -ENOTSUP;

}

static int vsock_get_port(int sd, unsigned int *port)
{
	return -ENOTSUP;
}

static int get_vsocket_params(int fd, unsigned int *lcid, unsigned int *lport,
			      unsigned int *rcid, unsigned int *rport)
{
	return -ENOTSUP;
}

#endif /* VSOCK */

static struct tracefs_instance *
clock_synch_create_instance(const char *clock, unsigned int cid)
{
	struct tracefs_instance *instance;
	char inst_name[256];

	snprintf(inst_name, 256, "clock_synch-%d", cid);

	instance = tracefs_instance_create(inst_name);
	if (!instance)
		return NULL;

	tracefs_instance_file_write(instance, "trace", "\0");
	if (clock)
		tracefs_instance_file_write(instance, "trace_clock", clock);
	return instance;
}

static void
clock_synch_delete_instance(struct tracefs_instance *inst)
{
	if (!inst)
		return;
	tracefs_instance_destroy(inst);
	tracefs_instance_free(inst);
}

static int clock_context_init(struct tracecmd_time_sync *tsync,
			      struct tsync_proto **proto, bool guest)
{
	struct clock_sync_context *clock = NULL;
	struct tsync_proto *protocol;

	if (tsync->context)
		return 0;

	protocol = tsync_proto_find(tsync->proto_name);
	if (!protocol || !protocol->clock_sync_calc)
		return -1;

	clock = calloc(1, sizeof(struct clock_sync_context));
	if (!clock)
		return -1;
	clock->is_guest = guest;
	clock->is_server = clock->is_guest;

	if (get_vsocket_params(tsync->msg_handle->fd, &clock->local_cid,
			       &clock->local_port, &clock->remote_cid,
			       &clock->remote_port))
		goto error;

	clock->instance = clock_synch_create_instance(tsync->clock_str,
						      clock->remote_cid);
	if (!clock->instance)
		goto error;

	tsync->context = clock;
	if (protocol->clock_sync_init && protocol->clock_sync_init(tsync) < 0)
		goto error;

	*proto = protocol;

	return 0;
error:
	tsync->context = NULL;
	free(clock);
	return -1;
}

/**
 * tracecmd_tsync_free - Free time sync context, allocated by
 *		tracecmd_tsync_with_host() or tracecmd_tsync_with_guest() APIs
 *
 * @tsync: Pointer to time sync context
 *
 */
void tracecmd_tsync_free(struct tracecmd_time_sync *tsync)
{
	struct clock_sync_context *tsync_context;
	struct tsync_proto *proto;

	if (!tsync || !tsync->context)
		return;
	tsync_context = (struct clock_sync_context *)tsync->context;

	proto = tsync_proto_find(tsync->proto_name);
	if (proto && proto->clock_sync_free)
		proto->clock_sync_free(tsync);

	clock_synch_delete_instance(tsync_context->instance);
	tsync_context->instance = NULL;

	free(tsync_context->sync_ts);
	free(tsync_context->sync_offsets);
	free(tsync_context->sync_scalings);
	tsync_context->sync_ts = NULL;
	tsync_context->sync_offsets = NULL;
	tsync_context->sync_scalings = NULL;
	tsync_context->sync_count = 0;
	tsync_context->sync_size = 0;
	pthread_mutex_destroy(&tsync->lock);
	pthread_cond_destroy(&tsync->cond);
	pthread_barrier_destroy(&tsync->first_sync);
	free(tsync->clock_str);
	free(tsync->proto_name);
	free(tsync);
}

static int tsync_send(struct tracecmd_time_sync *tsync,
		      struct tsync_proto *proto)
{
	long long timestamp = 0;
	long long scaling = 0;
	long long offset = 0;
	int ret;

	ret = proto->clock_sync_calc(tsync, &offset, &scaling, &timestamp);

	return ret;
}

static void tsync_with_host(struct tracecmd_time_sync *tsync)
{
	char protocol[TRACECMD_TSYNC_PNAME_LENGTH];
	struct tsync_proto *proto;
	unsigned int command;
	int ret;

	clock_context_init(tsync, &proto, true);
	if (!tsync->context)
		return;

	while (true) {
		ret = tracecmd_msg_recv_time_sync(tsync->msg_handle,
						  protocol, &command,
						  NULL, NULL);

		if (ret || strncmp(protocol, TRACECMD_TSYNC_PROTO_NONE, TRACECMD_TSYNC_PNAME_LENGTH) ||
		    command != TRACECMD_TIME_SYNC_CMD_PROBE)
			break;
		ret = tsync_send(tsync, proto);
		if (ret)
			break;
	}
}

static int tsync_get_sample(struct tracecmd_time_sync *tsync,
			    struct tsync_proto *proto, int array_step)
{
	struct clock_sync_context *clock;
	long long *sync_scalings = NULL;
	long long *sync_offsets = NULL;
	long long *sync_ts = NULL;
	long long timestamp = 0;
	long long scaling = 0;
	long long offset = 0;
	int ret;

	ret = proto->clock_sync_calc(tsync, &offset, &scaling, &timestamp);
	if (ret) {
		warning("Failed to synchronize timestamps with guest");
		return -1;
	}
	if (!offset || !timestamp || !scaling)
		return 0;
	clock = tsync->context;
	if (clock->sync_count >= clock->sync_size) {
		sync_ts = realloc(clock->sync_ts,
				  (clock->sync_size + array_step) * sizeof(long long));
		sync_offsets = realloc(clock->sync_offsets,
				       (clock->sync_size + array_step) * sizeof(long long));
		sync_scalings = realloc(clock->sync_scalings,
				       (clock->sync_size + array_step) * sizeof(long long));

		if (!sync_ts || !sync_offsets || !sync_scalings) {
			free(sync_ts);
			free(sync_offsets);
			free(sync_scalings);
			return -1;
		}
		clock->sync_size += array_step;
		clock->sync_ts = sync_ts;
		clock->sync_offsets = sync_offsets;
		clock->sync_scalings = sync_scalings;
	}

	clock->sync_ts[clock->sync_count] = timestamp;
	clock->sync_offsets[clock->sync_count] = offset;
	clock->sync_scalings[clock->sync_count] = scaling;
	clock->sync_count++;

	return 0;
}

#define TIMER_SEC_NANO 1000000000LL
static inline void get_ts_loop_delay(struct timespec *timeout, int delay_ms)
{
	memset(timeout, 0, sizeof(struct timespec));
	clock_gettime(CLOCK_REALTIME, timeout);

	timeout->tv_nsec += ((unsigned long long)delay_ms * 1000000LL);

	if (timeout->tv_nsec >= TIMER_SEC_NANO) {
		timeout->tv_sec += timeout->tv_nsec / TIMER_SEC_NANO;
		timeout->tv_nsec %= TIMER_SEC_NANO;
	}
}

#define CLOCK_TS_ARRAY 5
static int tsync_with_guest(struct tracecmd_time_sync *tsync)
{
	int ts_array_size = CLOCK_TS_ARRAY;
	struct tsync_proto *proto;
	struct timespec timeout;
	bool first = true;
	bool end = false;
	int ret;

	clock_context_init(tsync, &proto, false);
	if (!tsync->context) {
		pthread_barrier_wait(&tsync->first_sync);
		return -1;
	}

	if (tsync->loop_interval > 0 &&
	    tsync->loop_interval < (CLOCK_TS_ARRAY * 1000))
		ts_array_size = (CLOCK_TS_ARRAY * 1000) / tsync->loop_interval;

	while (true) {
		pthread_mutex_lock(&tsync->lock);
		ret = tracecmd_msg_send_time_sync(tsync->msg_handle,
						  TRACECMD_TSYNC_PROTO_NONE,
						  TRACECMD_TIME_SYNC_CMD_PROBE,
						  0, NULL);
		ret = tsync_get_sample(tsync, proto, ts_array_size);
		if (first) {
			first = false;
			pthread_barrier_wait(&tsync->first_sync);
		}
		if (ret || end)
			break;
		if (tsync->loop_interval > 0) {
			get_ts_loop_delay(&timeout, tsync->loop_interval);
			ret = pthread_cond_timedwait(&tsync->cond, &tsync->lock, &timeout);
			pthread_mutex_unlock(&tsync->lock);
			if (ret && ret != ETIMEDOUT)
				break;
			else if (!ret)
				end = true;
		} else {
			pthread_cond_wait(&tsync->cond, &tsync->lock);
			end = true;
			pthread_mutex_unlock(&tsync->lock);
		}
	};

	tracecmd_msg_send_time_sync(tsync->msg_handle,
				    TRACECMD_TSYNC_PROTO_NONE,
				    TRACECMD_TIME_SYNC_CMD_STOP,
				    0, NULL);
	return 0;
}

static void *tsync_host_thread(void *data)
{
	struct tracecmd_time_sync *tsync = NULL;

	tsync = (struct tracecmd_time_sync *)data;
	tsync_with_guest(tsync);
	tracecmd_msg_handle_close(tsync->msg_handle);
	tsync->msg_handle = NULL;

	pthread_exit(0);
}

/**
 * tracecmd_tsync_with_guest - Synchronize timestamps with guest
 *
 * @trace_id: Local ID for the current trace session
 * @cid: CID of the guest
 * @port: VSOCKET port, on which the guest listens for tsync requests
 * @guest_pid: PID of the host OS process, running the guest
 * @guest_cpus: Number of the guest VCPUs
 * @proto_name: Name of the negotiated time synchronization protocol
 * @clock: Trace clock, used for that session
 *
 * On success, a pointer to time sync context is returned, or NULL in
 * case of an error. The context must be freed with tracecmd_tsync_free()
 *
 * This API spawns a pthread, which performs time stamps synchronization
 * until tracecmd_tsync_with_guest_stop() is called.
 */
struct tracecmd_time_sync *
tracecmd_tsync_with_guest(unsigned long long trace_id, int loop_interval,
			  unsigned int cid, unsigned int port, int guest_pid,
			  int guest_cpus, const char *proto_name, const char *clock)
{
	struct tracecmd_time_sync *tsync;
	cpu_set_t *pin_mask = NULL;
	pthread_attr_t attrib;
	size_t mask_size = 0;
	int fd = -1;
	int ret;

	if (!proto_name)
		return NULL;

	tsync = calloc(1, sizeof(*tsync));
	if (!tsync)
		return NULL;

	tsync->trace_id = trace_id;
	tsync->loop_interval = loop_interval;
	tsync->proto_name = strdup(proto_name);
	fd = vsock_open(cid, port);
	if (fd < 0)
		goto error;

	tsync->msg_handle = tracecmd_msg_handle_alloc(fd, 0);
	if (!tsync->msg_handle) {
		ret = -1;
		goto error;
	}
	tsync->guest_pid = guest_pid;
	tsync->vcpu_count = guest_cpus;

	if (clock)
		tsync->clock_str = strdup(clock);
	pthread_mutex_init(&tsync->lock, NULL);
	pthread_cond_init(&tsync->cond, NULL);
	pthread_barrier_init(&tsync->first_sync, NULL, 2);
	pthread_attr_init(&attrib);
	pthread_attr_setdetachstate(&attrib, PTHREAD_CREATE_JOINABLE);

	ret = pthread_create(&tsync->thread, &attrib, tsync_host_thread, tsync);
	if (ret)
		goto error;
	tsync->thread_running = true;

	if (!get_first_cpu(&pin_mask, &mask_size))
		pthread_setaffinity_np(tsync->thread, mask_size, pin_mask);
	pthread_barrier_wait(&tsync->first_sync);

	if (pin_mask)
		CPU_FREE(pin_mask);
	pthread_attr_destroy(&attrib);

	return tsync;

error:
	if (tsync->msg_handle)
		tracecmd_msg_handle_close(tsync->msg_handle);
	else if (fd >= 0)
		close(fd);
	free(tsync);

	return NULL;
}

/**
 * tracecmd_write_guest_time_shift - Write collected timestamp corrections in a file
 *
 * @handle: Handle to a trace file, where timestamp corrections will be saved
 * @tsync: Time sync context with collected timestamp corrections
 *
 * Returns 0 on success, or -1 in case of an error.
 *
 * This API writes collected timestamp corrections in the metadata of the
 * trace file, as TRACECMD_OPTION_TIME_SHIFT option.
 */
int tracecmd_write_guest_time_shift(struct tracecmd_output *handle,
				    struct tracecmd_time_sync *tsync)
{
	struct iovec vector[6];
	unsigned int flags;
	long long *scalings = NULL;
	long long *offsets = NULL;
	long long *ts = NULL;
	int count;
	int ret;

	ret = tracecmd_tsync_get_offsets(tsync, &count,
					 &ts, &offsets, &scalings);
	if (ret < 0 || !count || !ts || !offsets || !scalings)
		return -1;
	ret = tsync_get_proto_flags(tsync, &flags);
	if (ret < 0)
		return -1;

	vector[0].iov_len = 8;
	vector[0].iov_base =  &(tsync->trace_id);
	vector[1].iov_len = 4;
	vector[1].iov_base = &flags;
	vector[2].iov_len = 4;
	vector[2].iov_base = &count;
	vector[3].iov_len = 8 * count;
	vector[3].iov_base = ts;
	vector[4].iov_len = 8 * count;
	vector[4].iov_base = offsets;
	vector[5].iov_len = 8 * count;
	vector[5].iov_base = scalings;
	tracecmd_add_option_v(handle, TRACECMD_OPTION_TIME_SHIFT, vector, 6);
	tracecmd_append_options(handle);
#ifdef TSYNC_DEBUG
	if (count > 1)
		printf("Got %d timestamp synch samples in %lld ns trace\n\r",
			count, ts[count - 1] - ts[0]);
#endif
	return 0;
}

/**
 * tracecmd_tsync_with_guest_stop - Stop the time sync session with a guest
 *
 * @tsync: Time sync context, representing a running time sync session
 *
 * Returns 0 on success, or -1 in case of an error.
 *
 */
int tracecmd_tsync_with_guest_stop(struct tracecmd_time_sync *tsync)
{
	if (!tsync || !tsync->thread_running)
		return -1;

	/* Signal the time synchronization thread to complete and wait for it */
	pthread_mutex_lock(&tsync->lock);
	pthread_cond_signal(&tsync->cond);
	pthread_mutex_unlock(&tsync->lock);
	pthread_join(tsync->thread, NULL);
	return 0;
}

static void *tsync_agent_thread(void *data)
{
	struct tracecmd_time_sync *tsync = NULL;
	int sd;

	tsync = (struct tracecmd_time_sync *)data;

	while (true) {
		sd = accept(tsync->msg_handle->fd, NULL, NULL);
		if (sd < 0) {
			if (errno == EINTR)
				continue;
			goto out;
		}
		break;
	}
	close(tsync->msg_handle->fd);
	tsync->msg_handle->fd = sd;

	tsync_with_host(tsync);

out:
	tracecmd_msg_handle_close(tsync->msg_handle);
	tracecmd_tsync_free(tsync);
	free(tsync);
	close(sd);

	pthread_exit(0);
}

/**
 * tracecmd_tsync_with_host - Synchronize timestamps with host
 *
 * @tsync_protos: List of tsync protocols, supported by the host
 * @clock: Trace clock, used for that session
 * @port: returned, VSOCKET port, on which the guest listens for tsync requests
 *
 * On success, a pointer to time sync context is returned, or NULL in
 * case of an error. The context must be freed with tracecmd_tsync_free()
 *
 * This API spawns a pthread, which performs time stamps synchronization
 * until tracecmd_tsync_with_host_stop() is called.
 */
struct tracecmd_time_sync *
tracecmd_tsync_with_host(const struct tracecmd_tsync_protos *tsync_protos,
			 const char *clock)
{
	struct tracecmd_time_sync *tsync;
	cpu_set_t *pin_mask = NULL;
	pthread_attr_t attrib;
	size_t mask_size = 0;
	unsigned int port;
	const char *proto;
	int ret;
	int fd;

	tsync = calloc(1, sizeof(struct tracecmd_time_sync));
	if (!tsync)
		return NULL;

	proto = tsync_proto_select(tsync_protos, clock,
				   TRACECMD_TIME_SYNC_ROLE_GUEST);
	if (!proto)
		goto error;
	tsync->proto_name = strdup(proto);
	fd = vsock_make();
	if (fd < 0)
		goto error;

	if (vsock_get_port(fd, &port) < 0)
		goto error;
	tsync->msg_handle = tracecmd_msg_handle_alloc(fd, 0);
	if (clock)
		tsync->clock_str = strdup(clock);

	pthread_attr_init(&attrib);
	tsync->vcpu_count = tracecmd_count_cpus();
	pthread_attr_setdetachstate(&attrib, PTHREAD_CREATE_JOINABLE);

	ret = pthread_create(&tsync->thread, &attrib, tsync_agent_thread, tsync);
	if (ret) {
		pthread_attr_destroy(&attrib);
		goto error;
	}
	tsync->thread_running = true;
	if (!get_first_cpu(&pin_mask, &mask_size))
		pthread_setaffinity_np(tsync->thread, mask_size, pin_mask);

	if (pin_mask)
		CPU_FREE(pin_mask);
	pthread_attr_destroy(&attrib);
	return tsync;

error:
	if (tsync) {
		if (tsync->msg_handle)
			tracecmd_msg_handle_close(tsync->msg_handle);
		else if (fd >= 0)
			close(fd);
		free(tsync->clock_str);
		free(tsync);
	}

	return NULL;

}

/**
 * tracecmd_tsync_with_host_stop - Stop the time sync session with a host
 *
 * @tsync: Time sync context, representing a running time sync session
 *
 * Returns 0 on success, or error number in case of an error.
 *
 */
int tracecmd_tsync_with_host_stop(struct tracecmd_time_sync *tsync)
{
	return pthread_join(tsync->thread, NULL);
}

/**
 * tracecmd_tsync_get_session_params - Get parameters of established time sync session
 *
 * @tsync: Time sync context, representing a running time sync session
 * @selected_proto: return, name of the selected time sync protocol for this session
 * @tsync_port: return, a VSOCK port on which new time sync requests are accepted.
 *
 * Returns 0 on success, or -1 in case of an error.
 *
 */
int tracecmd_tsync_get_session_params(struct tracecmd_time_sync *tsync,
				      char **selected_proto,
				      unsigned int *tsync_port)
{
	int ret;

	if (!tsync)
		return -1;

	if (tsync_port) {
		if (!tsync->msg_handle)
			return -1;
		ret = vsock_get_port(tsync->msg_handle->fd, tsync_port);
		if (ret < 0)
			return ret;
	}
	if (selected_proto) {
		if (!tsync->proto_name)
			return -1;
		(*selected_proto) = strdup(tsync->proto_name);

	}

	return 0;
}
