// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 VMware Inc, Slavomir Kaslev <kaslevs@vmware.com>
 *
 * based on prior implementation by Yoshihiro Yunomae
 * Copyright (C) 2013 Hitachi, Ltd.
 * Yoshihiro YUNOMAE <yoshihiro.yunomae.ez@hitachi.com>
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/vm_sockets.h>
#include <pthread.h>

#include "trace-local.h"
#include "trace-msg.h"

#define GET_LOCAL_CID	0x7b9

static int get_local_cid(unsigned int *cid)
{
	int fd, ret = 0;

	fd = open("/dev/vsock", O_RDONLY);
	if (fd < 0)
		return -errno;

	if (ioctl(fd, GET_LOCAL_CID, cid))
		ret = -errno;

	close(fd);
	return ret;
}

int trace_make_vsock(unsigned int port)
{
	struct sockaddr_vm addr = {
		.svm_family = AF_VSOCK,
		.svm_cid = VMADDR_CID_ANY,
		.svm_port = port,
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

int trace_get_vsock_port(int sd, unsigned int *port)
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

static void make_vsocks(int nr, int *fds, unsigned int *ports)
{
	unsigned int port;
	int i, fd, ret;

	for (i = 0; i < nr; i++) {
		fd = trace_make_vsock(VMADDR_PORT_ANY);
		if (fd < 0)
			die("Failed to open vsocket");

		ret = trace_get_vsock_port(fd, &port);
		if (ret < 0)
			die("Failed to get vsocket address");

		fds[i] = fd;
		ports[i] = port;
	}
}

static int open_agent_fifos(int nr_cpus, int *fds)
{
	char path[PATH_MAX];
	int i, fd, ret;

	for (i = 0; i < nr_cpus; i++) {
		snprintf(path, sizeof(path), VIRTIO_FIFO_FMT, i);
		fd = open(path, O_WRONLY);
		if (fd < 0) {
			ret = -errno;
			goto cleanup;
		}

		fds[i] = fd;
	}

	return 0;

cleanup:
	while (--i >= 0)
		close(fds[i]);

	return ret;
}

static char *get_clock(int argc, char **argv)
{
	int i;

	if (!argc || !argv)
		return NULL;

	for (i = 0; i < argc - 1; i++) {
		if (!strcmp("-C", argv[i]))
			return argv[i+1];
	}
	return NULL;
}

static void agent_handle(int sd, int nr_cpus, int page_size)
{
	struct tracecmd_tsync_protos *tsync_protos = NULL;
	struct tracecmd_time_sync *tsync = NULL;
	struct tracecmd_msg_handle *msg_handle;
	char *tsync_proto = NULL;
	unsigned long long trace_id;
	unsigned int tsync_port = 0;
	unsigned int *ports;
	char **argv = NULL;
	int argc = 0;
	bool use_fifos;
	int *fds;
	int ret;

	fds = calloc(nr_cpus, sizeof(*fds));
	ports = calloc(nr_cpus, sizeof(*ports));
	if (!fds || !ports)
		die("Failed to allocate memory");

	msg_handle = tracecmd_msg_handle_alloc(sd, 0);
	if (!msg_handle)
		die("Failed to allocate message handle");

	ret = tracecmd_msg_recv_trace_req(msg_handle, &argc, &argv,
					  &use_fifos, &trace_id,
					  &tsync_protos);
	if (ret < 0)
		die("Failed to receive trace request");

	if (use_fifos && open_agent_fifos(nr_cpus, fds))
		use_fifos = false;

	if (!use_fifos)
		make_vsocks(nr_cpus, fds, ports);
	if (tsync_protos && tsync_protos->names) {
		tsync = tracecmd_tsync_with_host(tsync_protos,
						 get_clock(argc, argv));
		if (tsync)
			tracecmd_tsync_get_session_params(tsync, &tsync_proto, &tsync_port);
		else
			warning("Failed to negotiate timestamps synchronization with the host");
	}
	trace_id = tracecmd_generate_traceid();
	ret = tracecmd_msg_send_trace_resp(msg_handle, nr_cpus, page_size,
					   ports, use_fifos, trace_id,
					   tsync_proto, tsync_port);
	if (ret < 0)
		die("Failed to send trace response");

	trace_record_agent(msg_handle, nr_cpus, fds, argc, argv,
			   use_fifos, trace_id);

	if (tsync) {
		tracecmd_tsync_with_host_stop(tsync);
		tracecmd_tsync_free(tsync);
	}

	if (tsync_protos) {
		free(tsync_protos->names);
		free(tsync_protos);
	}
	free(argv[0]);
	free(argv);
	free(ports);
	free(fds);
	tracecmd_msg_handle_close(msg_handle);
	exit(0);
}

static volatile pid_t handler_pid;

static void handle_sigchld(int sig)
{
	int wstatus;
	pid_t pid;

	for (;;) {
		pid = waitpid(-1, &wstatus, WNOHANG);
		if (pid <= 0)
			break;

		if (pid == handler_pid)
			handler_pid = 0;
	}
}

static pid_t do_fork()
{
	/* in debug mode, we do not fork off children */
	if (tracecmd_get_debug())
		return 0;

	return fork();
}

static void agent_serve(unsigned int port)
{
	int sd, cd, nr_cpus;
	unsigned int cid;
	pid_t pid;

	signal(SIGCHLD, handle_sigchld);

	nr_cpus = tracecmd_count_cpus();
	page_size = getpagesize();

	sd = trace_make_vsock(port);
	if (sd < 0)
		die("Failed to open vsocket");

	if (!get_local_cid(&cid))
		printf("listening on @%u:%u\n", cid, port);

	for (;;) {
		cd = accept(sd, NULL, NULL);
		if (cd < 0) {
			if (errno == EINTR)
				continue;
			die("accept");
		}

		if (handler_pid)
			goto busy;

		pid = do_fork();
		if (pid == 0) {
			close(sd);
			signal(SIGCHLD, SIG_DFL);
			agent_handle(cd, nr_cpus, page_size);
		}
		if (pid > 0)
			handler_pid = pid;

busy:
		close(cd);
	}
}

enum {
	DO_DEBUG	= 255
};

void trace_agent(int argc, char **argv)
{
	bool do_daemon = false;
	unsigned int port = TRACE_AGENT_DEFAULT_PORT;

	if (argc < 2)
		usage(argv);

	if (strcmp(argv[1], "agent") != 0)
		usage(argv);

	for (;;) {
		int c, option_index = 0;
		static struct option long_options[] = {
			{"port", required_argument, NULL, 'p'},
			{"help", no_argument, NULL, '?'},
			{"debug", no_argument, NULL, DO_DEBUG},
			{NULL, 0, NULL, 0}
		};

		c = getopt_long(argc-1, argv+1, "+hp:D",
				long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 'h':
			usage(argv);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'D':
			do_daemon = true;
			break;
		case DO_DEBUG:
			tracecmd_set_debug(true);
			break;
		default:
			usage(argv);
		}
	}

	if (optind < argc-1)
		usage(argv);

	if (do_daemon && daemon(1, 0))
		die("daemon");

	agent_serve(port);
}
