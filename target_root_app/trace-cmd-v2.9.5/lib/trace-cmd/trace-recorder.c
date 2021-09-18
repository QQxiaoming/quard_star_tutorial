// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2009, 2010 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 */
#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>

#include "tracefs.h"
#include "trace-cmd-private.h"
#include "event-utils.h"

/* F_GETPIPE_SZ was introduced in 2.6.35, older systems don't have it */
#ifndef F_GETPIPE_SZ
# define F_GETPIPE_SZ	1032 /* The Linux number for the option */
#endif

#ifndef SPLICE_F_MOVE
# define SPLICE_F_MOVE		1
# define SPLICE_F_NONBLOCK	2
# define SPLICE_F_MORE		4
# define SPLICE_F_GIFT		8
#endif

#define POLL_TIMEOUT_MS		1000

struct tracecmd_recorder {
	int		fd;
	int		fd1;
	int		fd2;
	int		trace_fd;
	int		brass[2];
	int		pipe_size;
	int		page_size;
	int		cpu;
	int		stop;
	int		max;
	int		pages;
	int		count;
	unsigned	fd_flags;
	unsigned	trace_fd_flags;
	unsigned	flags;
};

static int append_file(int size, int dst, int src)
{
	char buf[size];
	int r;

	lseek64(src, 0, SEEK_SET);

	/* If there's an error, then we are pretty much screwed :-p */
	do {
		r = read(src, buf, size);
		if (r < 0)
			return r;
		r = write(dst, buf, r);
		if (r < 0)
			return r;
	} while (r);
	return 0;
}

void tracecmd_free_recorder(struct tracecmd_recorder *recorder)
{
	if (!recorder)
		return;

	if (recorder->max) {
		/* Need to put everything into fd1 */
		if (recorder->fd == recorder->fd1) {
			int ret;
			/*
			 * Crap, the older data is in fd2, and we need
			 * to append fd1 onto it, and then copy over to fd1
			 */
			ret = append_file(recorder->page_size,
					  recorder->fd2, recorder->fd1);
			/* Error on copying, then just keep fd1 */
			if (ret) {
				lseek64(recorder->fd1, 0, SEEK_END);
				goto close;
			}
			lseek64(recorder->fd1, 0, SEEK_SET);
			ftruncate(recorder->fd1, 0);
		}
		append_file(recorder->page_size, recorder->fd1, recorder->fd2);
	}
 close:
	if (recorder->brass[0] >= 0)
		close(recorder->brass[0]);

	if (recorder->brass[1] >= 0)
		close(recorder->brass[1]);

	if (recorder->trace_fd >= 0)
		close(recorder->trace_fd);

	if (recorder->fd1 >= 0)
		close(recorder->fd1);

	if (recorder->fd2 >= 0)
		close(recorder->fd2);

	free(recorder);
}

struct tracecmd_recorder *
tracecmd_create_buffer_recorder_fd2(int fd, int fd2, int cpu, unsigned flags,
				    const char *buffer, int maxkb)
{
	struct tracecmd_recorder *recorder;
	char *path = NULL;
	int pipe_size = 0;
	int ret;

	recorder = malloc(sizeof(*recorder));
	if (!recorder)
		return NULL;

	recorder->cpu = cpu;
	recorder->flags = flags;

	recorder->fd_flags = SPLICE_F_MOVE;

	if (!(recorder->flags & TRACECMD_RECORD_BLOCK))
		recorder->fd_flags |= SPLICE_F_NONBLOCK;

	recorder->trace_fd_flags = SPLICE_F_MOVE;

	/* Init to know what to free and release */
	recorder->trace_fd = -1;
	recorder->brass[0] = -1;
	recorder->brass[1] = -1;

	recorder->page_size = getpagesize();
	if (maxkb) {
		int kb_per_page = recorder->page_size >> 10;

		if (!kb_per_page)
			kb_per_page = 1;
		recorder->max = maxkb / kb_per_page;
		/* keep max half */
		recorder->max >>= 1;
		if (!recorder->max)
			recorder->max = 1;
	} else
		recorder->max = 0;

	recorder->count = 0;
	recorder->pages = 0;

	/* fd always points to what to write to */
	recorder->fd = fd;
	recorder->fd1 = fd;
	recorder->fd2 = fd2;

	if (buffer) {
		if (flags & TRACECMD_RECORD_SNAPSHOT)
			ret = asprintf(&path, "%s/per_cpu/cpu%d/snapshot_raw",
				       buffer, cpu);
		else
			ret = asprintf(&path, "%s/per_cpu/cpu%d/trace_pipe_raw",
				       buffer, cpu);
		if (ret < 0)
			goto out_free;

		recorder->trace_fd = open(path, O_RDONLY);
		free(path);

		if (recorder->trace_fd < 0)
			goto out_free;
	}

	if (!(recorder->flags & (TRACECMD_RECORD_NOSPLICE |
				 TRACECMD_RECORD_NOBRASS))) {
		ret = pipe(recorder->brass);
		if (ret < 0)
			goto out_free;

		ret = fcntl(recorder->brass[0], F_GETPIPE_SZ, &pipe_size);
		/*
		 * F_GETPIPE_SZ was introduced in 2.6.35, ftrace was introduced
		 * in 2.6.31. If we are running on an older kernel, just fall
		 * back to using page_size for splice(). It could also return
		 * success, but not modify pipe_size.
		 */
		if (ret < 0 || !pipe_size)
			pipe_size = recorder->page_size;

		recorder->pipe_size = pipe_size;
	}

	return recorder;

 out_free:
	tracecmd_free_recorder(recorder);
	return NULL;
}

struct tracecmd_recorder *
tracecmd_create_buffer_recorder_fd(int fd, int cpu, unsigned flags, const char *buffer)
{
	return tracecmd_create_buffer_recorder_fd2(fd, -1, cpu, flags, buffer, 0);
}

static struct tracecmd_recorder *
__tracecmd_create_buffer_recorder(const char *file, int cpu, unsigned flags,
				  const char *buffer)
{
	struct tracecmd_recorder *recorder;
	int fd;

	fd = open(file, O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE, 0644);
	if (fd < 0)
		return NULL;

	recorder = tracecmd_create_buffer_recorder_fd(fd, cpu, flags, buffer);
	if (!recorder) {
		close(fd);
		unlink(file);
	}

	return recorder;
}

struct tracecmd_recorder *
tracecmd_create_buffer_recorder_maxkb(const char *file, int cpu, unsigned flags,
				      const char *buffer, int maxkb)
{
	struct tracecmd_recorder *recorder = NULL;
	char *file2;
	int len;
	int fd;
	int fd2;

	if (!maxkb)
		return tracecmd_create_buffer_recorder(file, cpu, flags, buffer);

	len = strlen(file);
	file2 = malloc(len + 3);
	if (!file2)
		return NULL;

	sprintf(file2, "%s.1", file);

	fd = open(file, O_RDWR | O_CREAT | O_TRUNC | O_LARGEFILE, 0644);
	if (fd < 0)
		goto out;

	fd2 = open(file2, O_RDWR | O_CREAT | O_TRUNC | O_LARGEFILE, 0644);
	if (fd2 < 0)
		goto err;

	recorder = tracecmd_create_buffer_recorder_fd2(fd, fd2, cpu, flags, buffer, maxkb);
	if (!recorder)
		goto err2;
 out:
	/* Unlink file2, we need to add everything to file at the end */
	unlink(file2);
	free(file2);

	return recorder;
 err2:
	close(fd2);
 err:
	close(fd);
	unlink(file);
	goto out;
}

struct tracecmd_recorder *
tracecmd_create_buffer_recorder(const char *file, int cpu, unsigned flags,
				const char *buffer)
{
	return __tracecmd_create_buffer_recorder(file, cpu, flags, buffer);
}

/**
 * tracecmd_create_recorder_virt - Create a recorder reading tracing data
 * from the trace_fd file descriptor instead of from the local tracefs
 * @file: output filename where tracing data will be written
 * @cpu: which CPU is being traced
 * @flags: flags configuring the recorder (see TRACECMD_RECORDER_* enums)
 * @trace_fd: file descriptor from where tracing data will be read
 */
struct tracecmd_recorder *
tracecmd_create_recorder_virt(const char *file, int cpu, unsigned flags,
			      int trace_fd)
{
	struct tracecmd_recorder *recorder;

	recorder = __tracecmd_create_buffer_recorder(file, cpu, flags, NULL);
	if (recorder)
		recorder->trace_fd = trace_fd;

	return recorder;
}

struct tracecmd_recorder *tracecmd_create_recorder_fd(int fd, int cpu, unsigned flags)
{
	const char *tracing;

	tracing = tracefs_tracing_dir();
	if (!tracing) {
		errno = ENODEV;
		return NULL;
	}

	return tracecmd_create_buffer_recorder_fd(fd, cpu, flags, tracing);
}

struct tracecmd_recorder *tracecmd_create_recorder(const char *file, int cpu, unsigned flags)
{
	const char *tracing;

	tracing = tracefs_tracing_dir();
	if (!tracing) {
		errno = ENODEV;
		return NULL;
	}

	return tracecmd_create_buffer_recorder(file, cpu, flags, tracing);
}

struct tracecmd_recorder *
tracecmd_create_recorder_maxkb(const char *file, int cpu, unsigned flags, int maxkb)
{
	const char *tracing;

	tracing = tracefs_tracing_dir();
	if (!tracing) {
		errno = ENODEV;
		return NULL;
	}

	return tracecmd_create_buffer_recorder_maxkb(file, cpu, flags, tracing, maxkb);
}

static inline void update_fd(struct tracecmd_recorder *recorder, int size)
{
	int fd;

	if (!recorder->max)
		return;

	recorder->count += size;

	if (recorder->count >= recorder->page_size) {
		recorder->count = 0;
		recorder->pages++;
	}

	if (recorder->pages < recorder->max)
		return;

	recorder->pages = 0;

	fd = recorder->fd;

	/* Swap fd to next file. */
	if (fd == recorder->fd1)
		fd = recorder->fd2;
	else
		fd = recorder->fd1;

	/* Zero out the new file we are writing to */
	lseek64(fd, 0, SEEK_SET);
	ftruncate(fd, 0);

	recorder->fd = fd;
}

/*
 * Returns -1 on error.
 *          or bytes of data read.
 */
static long splice_data(struct tracecmd_recorder *recorder)
{
	long total_read = 0;
	long read;
	long ret;

	read = splice(recorder->trace_fd, NULL, recorder->brass[1], NULL,
		      recorder->pipe_size, recorder->trace_fd_flags);
	if (read < 0) {
		if (errno == EAGAIN || errno == EINTR || errno == ENOTCONN)
			return 0;

		warning("recorder error in splice input");
		return -1;
	} else if (read == 0)
		return 0;

 again:
	ret = splice(recorder->brass[0], NULL, recorder->fd, NULL,
		     read, recorder->fd_flags);
	if (ret < 0) {
		if (errno != EAGAIN && errno != EINTR) {
			warning("recorder error in splice output");
			return -1;
		}
		return total_read;
	} else
		update_fd(recorder, ret);
	total_read = ret;
	read -= ret;
	if (read)
		goto again;

	return total_read;
}

/*
 * Returns -1 on error.
 *          or bytes of data read.
 */
static long direct_splice_data(struct tracecmd_recorder *recorder)
{
	struct pollfd pfd = {
		.fd = recorder->trace_fd,
		.events = POLLIN,
	};
	long read;
	int ret;

	/*
	 * splice(2) in Linux used to not check O_NONBLOCK flag of pipe file
	 * descriptors before [1]. To avoid getting blocked in the splice(2)
	 * call below after the user had requested to stop tracing, we poll(2)
	 * here. This poll() is not necessary on newer kernels.
	 *
	 * [1] https://github.com/torvalds/linux/commit/ee5e001196d1345b8fee25925ff5f1d67936081e
	 */
	ret = poll(&pfd, 1, POLL_TIMEOUT_MS);
	if (ret < 0)
		return -1;

	if (!(pfd.revents | POLLIN))
		return 0;

	read = splice(recorder->trace_fd, NULL, recorder->fd, NULL,
		      recorder->pipe_size, recorder->fd_flags);
	if (read < 0) {
		if (errno == EAGAIN || errno == EINTR || errno == ENOTCONN)
			return 0;

		warning("recorder error in splice input");
		return -1;
	}

	return read;
}

/*
 * Returns -1 on error.
 *          or bytes of data read.
 */
static long read_data(struct tracecmd_recorder *recorder)
{
	char buf[recorder->page_size];
	long left;
	long r, w;

	r = read(recorder->trace_fd, buf, recorder->page_size);
	if (r < 0) {
		if (errno == EAGAIN || errno == EINTR || errno == ENOTCONN)
			return 0;

		warning("recorder error in read input");
		return -1;
	}

	left = r;
	do {
		w = write(recorder->fd, buf + (r - left), left);
		if (w > 0) {
			left -= w;
			update_fd(recorder, w);
		}
	} while (w >= 0 && left);

	if (w < 0)
		r = w;

	return r;
}

static long move_data(struct tracecmd_recorder *recorder)
{
	if (recorder->flags & TRACECMD_RECORD_NOSPLICE)
		return read_data(recorder);

	if (recorder->flags & TRACECMD_RECORD_NOBRASS)
		return direct_splice_data(recorder);

	return splice_data(recorder);
}

static void set_nonblock(struct tracecmd_recorder *recorder)
{
	long flags;

	/* Do not block on reads for flushing */
	flags = fcntl(recorder->trace_fd, F_GETFL);
	fcntl(recorder->trace_fd, F_SETFL, flags | O_NONBLOCK);

	/* Do not block on streams for write */
	recorder->fd_flags |= SPLICE_F_NONBLOCK;
}

long tracecmd_flush_recording(struct tracecmd_recorder *recorder)
{
	char buf[recorder->page_size];
	long total = 0;
	long wrote = 0;
	long ret;

	set_nonblock(recorder);

	do {
		ret = move_data(recorder);
		if (ret < 0)
			return ret;
		total += ret;
	} while (ret);

	/* splice only reads full pages */
	do {
		ret = read(recorder->trace_fd, buf, recorder->page_size);
		if (ret > 0) {
			write(recorder->fd, buf, ret);
			wrote += ret;
		}

	} while (ret > 0);

	/* Make sure we finish off with a page size boundary */
	wrote &= recorder->page_size - 1;
	if (wrote) {
		memset(buf, 0, recorder->page_size);
		write(recorder->fd, buf, recorder->page_size - wrote);
		total += recorder->page_size;
	}

	return total;
}

int tracecmd_start_recording(struct tracecmd_recorder *recorder, unsigned long sleep)
{
	struct timespec req = {
		.tv_sec = sleep / 1000000,
		.tv_nsec = (sleep % 1000000) * 1000,
	};
	long read = 1;
	long ret;

	recorder->stop = 0;

	do {
		/* Only sleep if we did not read anything last time */
		if (!read && sleep)
			nanosleep(&req, NULL);

		read = 0;
		do {
			ret = move_data(recorder);
			if (ret < 0)
				return ret;
			read += ret;
		} while (ret);
	} while (!recorder->stop);

	/* Flush out the rest */
	ret = tracecmd_flush_recording(recorder);

	if (ret < 0)
		return ret;

	return 0;
}

void tracecmd_stop_recording(struct tracecmd_recorder *recorder)
{
	if (!recorder)
		return;

	set_nonblock(recorder);

	recorder->stop = 1;
}
