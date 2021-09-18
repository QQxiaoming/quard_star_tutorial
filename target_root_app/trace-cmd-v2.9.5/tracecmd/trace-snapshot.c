// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2013 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "tracefs.h"
#include "trace-local.h"

static void write_file(const char *name, char *val)
{
	char *path;
	int fd;
	ssize_t n;

	path = tracefs_get_tracing_file(name);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		die("writing %s", path);

	n = write(fd, val, strlen(val));
	if (n < 0)
		die("failed to write to %s\n", path);

	tracefs_put_tracing_file(path);
	close(fd);
}

void trace_snapshot (int argc, char **argv)
{
	const char *buffer = NULL;
	const char *file = "snapshot";
	struct stat st;
	char *name;
	char cpu_path[128];
	int take_snap = 0;
	int reset_snap = 0;
	int free_snap = 0;
	int cpu = -1;
	int ret;
	int c;

	if (argc < 2)
		usage(argv);

	if (strcmp(argv[1], "snapshot") != 0)
		usage(argv);

	while ((c = getopt(argc-1, argv+1, "srfB:c:")) >= 0) {
		switch (c) {
		case 'h':
			usage(argv);
			break;
		case 's':
			take_snap = 1;
			if (free_snap)
				die("can't take snapshot and free it at the same time");
			break;
		case 'f':
			free_snap = 1;
			if (take_snap)
				die("can't take snapshot and free it at the same time");
			break;
		case 'r':
			reset_snap = 1;
			break;
		case 'B':
			if (buffer)
				die("Can only do one buffer at a time");
			buffer = optarg;
			break;
		case 'c':
			if (cpu >= 0)
				die("Can only do one CPU (or all) at a time");
			cpu = atoi(optarg);
			break;
		default:
			usage(argv);
		}
	}

	if (cpu >= 0) {
		snprintf(cpu_path, 128, "per_cpu/cpu%d/%s", cpu, file);
		file = cpu_path;
	}

	name = tracefs_get_tracing_file(file);
	ret = stat(name, &st);
	if (ret < 0)
		die("Snapshot feature is not supported by this kernel");
	tracefs_put_tracing_file(name);

	if (!reset_snap && !take_snap && !free_snap) {
		show_file(file);
		exit(0);
	}

	if (reset_snap)
		write_file(file, "2");

	if (free_snap)
		write_file(file, "0");

	if (take_snap)
		write_file(file, "1");
}
