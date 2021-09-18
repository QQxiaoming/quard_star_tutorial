// SPDX-License-Identifier: GPL-2.0

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

// C
#include <stdio.h>
#include <stdlib.h>

// KernelShark
#include "libkshark.h"

const char *default_file = "trace.dat";

int main(int argc, char **argv)
{
	struct kshark_context *kshark_ctx;
	struct kshark_entry **data = NULL;
	ssize_t r, n_rows, n_tasks;
	char *entry_str;
	bool status;
	int *pids;

	/* Create a new kshark session. */
	kshark_ctx = NULL;
	if (!kshark_instance(&kshark_ctx))
		return 1;

	/* Open a trace data file produced by trace-cmd. */
	if (argc > 1)
		status = kshark_open(kshark_ctx, argv[1]);
	else
		status = kshark_open(kshark_ctx, default_file);

	if (!status) {
		kshark_free(kshark_ctx);
		return 1;
	}

	/* Load the content of the file into an array of entries. */
	n_rows = kshark_load_data_entries(kshark_ctx, &data);
	if (n_rows < 1) {
		kshark_free(kshark_ctx);
		return 1;
	}

	/* Print to the screen the list of all tasks. */
	n_tasks = kshark_get_task_pids(kshark_ctx, &pids);
	for (r = 0; r < n_tasks; ++r) {
		const char *task_str =
			tep_data_comm_from_pid(kshark_ctx->pevent,
					       pids[r]);

		printf("task: %s-%i\n", task_str, pids[r]);
	}

	free(pids);

	puts("\n\n");

	/* Print to the screen the first 10 entries. */
	for (r = 0; r < 10; ++r) {
		entry_str = kshark_dump_entry(data[r]);
		puts(entry_str);
		free(entry_str);
	}

	puts("\n...\n");

	/* Print the last 10 entries. */
	for (r = n_rows - 10; r < n_rows; ++r) {
		entry_str = kshark_dump_entry(data[r]);
		puts(entry_str);
		free(entry_str);
	}

	/* Free the memory. */
	for (r = 0; r < n_rows; ++r)
		free(data[r]);

	free(data);

	/* Close the file. */
	kshark_close(kshark_ctx);

	/* Close the session. */
	kshark_free(kshark_ctx);

	return 0;
}
