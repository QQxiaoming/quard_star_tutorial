// SPDX-License-Identifier: GPL-2.0

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

// C
#include <stdio.h>
#include <stdlib.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-model.h"

#define N_BINS 5

const char *default_file = "trace.dat";

void dump_bin(struct kshark_trace_histo *histo, int bin,
	      const char *type, int val)
{
	const struct kshark_entry *e_front, *e_back;
	char *entry_str;
	ssize_t i_front, i_back;

	printf("bin %i {\n", bin);
	if (strcmp(type, "cpu") == 0) {
		e_front = ksmodel_get_entry_front(histo, bin, true,
						  kshark_match_cpu, val,
						  NULL,
						  &i_front);

		e_back = ksmodel_get_entry_back(histo, bin, true,
						kshark_match_cpu, val,
						NULL,
						&i_back);
	} else if (strcmp(type, "task") == 0) {
		e_front = ksmodel_get_entry_front(histo, bin, true,
						  kshark_match_pid, val,
						  NULL,
						  &i_front);

		e_back = ksmodel_get_entry_back(histo, bin, true,
						kshark_match_pid, val,
						NULL,
						&i_back);
	} else {
		i_front = ksmodel_first_index_at_bin(histo, bin);
		e_front = histo->data[i_front];

		i_back = ksmodel_last_index_at_bin(histo, bin);
		e_back = histo->data[i_back];
	}

	if (i_front == KS_EMPTY_BIN) {
		puts ("EMPTY BIN");
	} else {
		entry_str = kshark_dump_entry(e_front);
		printf("%zd -> %s\n", i_front, entry_str);
		free(entry_str);

		entry_str = kshark_dump_entry(e_back);
		printf("%zd -> %s\n", i_back, entry_str);
		free(entry_str);
	}

	puts("}\n");
}

void dump_histo(struct kshark_trace_histo *histo, const char *type, int val)
{
	size_t bin;

	for (bin = 0; bin < histo->n_bins; ++bin)
		dump_bin(histo, bin, type, val);
}

int main(int argc, char **argv)
{
	struct kshark_context *kshark_ctx;
	struct kshark_entry **data = NULL;
	struct kshark_trace_histo histo;
	ssize_t i, n_rows, n_tasks;
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

	/* Get a list of all tasks. */
	n_tasks = kshark_get_task_pids(kshark_ctx, &pids);

	/* Initialize the Visualization Model. */
	ksmodel_init(&histo);
	ksmodel_set_bining(&histo, N_BINS, data[0]->ts,
					   data[n_rows - 1]->ts);

	/* Fill the model with data and calculate its state. */
	ksmodel_fill(&histo, data, n_rows);

	/* Dump the raw bins. */
	dump_histo(&histo, "", 0);

	puts("\n...\n\n");

	/*
	 * Change the state of the model. Do 50% Zoom-In and dump only CPU 0.
	 */
	ksmodel_zoom_in(&histo, .50, -1);
	dump_histo(&histo, "cpu", 0);

	puts("\n...\n\n");

	/* Shift forward by two bins and this time dump only CPU 1. */
	ksmodel_shift_forward(&histo, 2);
	dump_histo(&histo, "cpu", 1);

	puts("\n...\n\n");

	/*
	 * Do 10% Zoom-Out, using the last bin as a focal point. Dump the last
	 * Task.
	 */
	ksmodel_zoom_out(&histo, .10, N_BINS - 1);
	dump_histo(&histo, "task", pids[n_tasks - 1]);

	/* Reset (clear) the model. */
	ksmodel_clear(&histo);

	/* Free the memory. */
	for (i = 0; i < n_rows; ++i)
		free(data[i]);

	free(data);

	/* Close the file. */
	kshark_close(kshark_ctx);

	/* Close the session. */
	kshark_free(kshark_ctx);

	return 0;
}
