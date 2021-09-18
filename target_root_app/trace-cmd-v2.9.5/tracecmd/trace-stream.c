// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2014 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/types.h>

#include "trace-local.h"

/*
 * Stream runs for a single machine. We are going to cheat
 * and use the trace-output and trace-input code to create
 * our pevent. First just create a trace.dat file and then read
 * it to create the pevent and handle.
 */
struct tracecmd_input *
trace_stream_init(struct buffer_instance *instance, int cpu, int fd, int cpus,
		  struct hook_list *hooks,
		  tracecmd_handle_init_func handle_init, int global)
{
	struct tracecmd_input *trace_input;
	struct tracecmd_output *trace_output;
	static FILE *fp = NULL;
	static int tfd;
	static int ofd;
	long flags;

	if (instance->handle) {
		trace_input = instance->handle;
		goto make_pipe;
	}

	if (!fp) {
		fp = tmpfile();
		if (!fp)
			return NULL;
		tfd = fileno(fp);

		ofd = dup(tfd);
		trace_output = tracecmd_create_init_fd(ofd);
		if (!trace_output) {
			fclose(fp);
			return NULL;
		}
		tracecmd_output_free(trace_output);
	}

	lseek(ofd, 0, SEEK_SET);

	trace_input = tracecmd_alloc_fd(ofd, 0);
	if (!trace_input) {
		close(ofd);
		goto fail;
	}

	if (tracecmd_read_headers(trace_input, TRACECMD_FILE_PRINTK) < 0)
		goto fail_free_input;

	if (handle_init)
		handle_init(trace_input, hooks, global);

 make_pipe:
	/* Do not block on this pipe */
	flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	if (tracecmd_make_pipe(trace_input, cpu, fd, cpus) < 0)
		goto fail_free_input;

	instance->handle = trace_input;

	return trace_input;

 fail_free_input:
	tracecmd_close(trace_input);
 fail:
	fclose(fp);

	return NULL;
}

int trace_stream_read(struct pid_record_data *pids, int nr_pids, struct timeval *tv)
{
	struct tep_record *record;
	struct pid_record_data *pid;
	struct pid_record_data *last_pid;
	fd_set rfds;
	int top_rfd = 0;
	int nr_fd;
	int ret;
	int i;

	last_pid = NULL;

 again:
	for (i = 0; i < nr_pids; i++) {
		pid = &pids[i];

		if (!pid->record)
			pid->record = tracecmd_read_data(pid->instance->handle, pid->cpu);
		record = pid->record;
		if (!record && errno == EINVAL)
			/* pipe has closed */
			pid->closed = 1;

		if (record &&
		    (!last_pid || record->ts < last_pid->record->ts))
			last_pid = pid;
	}
	if (last_pid) {
		trace_show_data(last_pid->instance->handle, last_pid->record);
		tracecmd_free_record(last_pid->record);
		last_pid->record = NULL;
		return 1;
	}

	nr_fd = 0;
	FD_ZERO(&rfds);

	for (i = 0; i < nr_pids; i++) {
		/* Do not process closed pipes */
		if (pids[i].closed)
			continue;
		nr_fd++;
		if (pids[i].brass[0] > top_rfd)
			top_rfd = pids[i].brass[0];

		FD_SET(pids[i].brass[0], &rfds);
	}

	if (!nr_fd)
		return 0;

	ret = select(top_rfd + 1, &rfds, NULL, NULL, tv);

	if (ret > 0)
		goto again;

	return ret;
}
