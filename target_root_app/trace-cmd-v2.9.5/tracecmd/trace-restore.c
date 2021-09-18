// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2009, 2010 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 */
#define _LARGEFILE64_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "trace-local.h"

void trace_restore (int argc, char **argv)
{
	struct tracecmd_output *handle;
	const char *output_file = DEFAULT_INPUT_FILE;
	const char *output = NULL;
	const char *input = NULL;
	const char *tracing_dir = NULL;
	const char *kallsyms = NULL;
	struct stat st1;
	struct stat st2;
	int first_arg;
	int create_only = 0;
	int args;
	int c;

	if (argc < 2)
		usage(argv);

	if (strcmp(argv[1], "restore") != 0)
		usage(argv);

	while ((c = getopt(argc-1, argv+1, "+hco:i:t:k:")) >= 0) {
		switch (c) {
		case 'h':
			usage(argv);
			break;
		case 'c':
			if (input)
				die("-c and -i are incompatible");
			create_only = 1;
			/* make output default to partial */
			output_file = "trace-partial.dat";
			break;

		case 't':
			tracing_dir = optarg;
			break;
		case 'k':
			kallsyms = optarg;
			break;
		case 'o':
			if (output)
				die("only one output file allowed");
			output = optarg;
			break;

		case 'i':
			if (input)
				die("only one input file allowed");
			if (create_only)
				die("-c and -i are incompatible");
			input = optarg;
			break;

		default:
			usage(argv);
		}
	}

	if (!output)
		output = output_file;

	if ((argc - optind) <= 1) {
		if (!create_only) {
			warning("No data files found");
			usage(argv);
		}

		handle = tracecmd_create_init_file_override(output, tracing_dir,
							    kallsyms);
		if (!handle)
			die("Unabled to create output file %s", output);
		if (tracecmd_write_cmdlines(handle) < 0)
			die("Failed to write command lines");
		tracecmd_output_close(handle);
		exit(0);
	}
	first_arg = optind + 1;
	args = argc - first_arg;
	printf("first = %d %s args=%d\n", first_arg, argv[first_arg], args);

	/* Make sure input and output are not the same file */
	if (input && output) {
		if (stat(input, &st1) < 0)
			die("%s:", input);
		/* output exists? otherwise we don't care */
		if (stat(output, &st2) == 0) {
			if (st1.st_ino == st2.st_ino &&
			    st1.st_dev == st2.st_dev)
				die("input and output file are the same");
		}
	}

	if (input) {
		struct tracecmd_input *ihandle;

		ihandle = tracecmd_alloc(input, 0);
		if (!ihandle)
			die("error reading file %s", input);
		/* make sure headers are ok */
		if (tracecmd_read_headers(ihandle, TRACECMD_FILE_CMD_LINES) < 0)
			die("error reading file %s headers", input);

		handle = tracecmd_copy(ihandle, output);
		tracecmd_close(ihandle);
	} else
		handle = tracecmd_create_init_file(output);

	if (!handle)
		die("error writing to %s", output);

	if (tracecmd_append_cpu_data(handle, args, &argv[first_arg]) < 0)
		die("failed to append data");

	return;
}
