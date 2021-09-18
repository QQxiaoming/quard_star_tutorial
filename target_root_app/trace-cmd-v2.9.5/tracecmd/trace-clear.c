// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2008, 2009, 2010 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 * Updates:
 * Copyright (C) 2020, VMware, Tzvetomir Stoyanov <tz.stoyanov@gmail.com>
 *
 */
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "tracefs.h"
#include "trace-local.h"

struct instances_list {
	struct instances_list *next;
	struct tracefs_instance *instance;
};

static int add_new_instance(struct instances_list **list, char *name)
{
	struct instances_list *new;

	if (!tracefs_instance_exists(name))
		return -1;
	new = calloc(1, sizeof(*new));
	if (!new)
		return -1;
	new->instance = tracefs_instance_create(name);
	if (!new->instance) {
		free(new);
		return -1;
	}

	new->next = *list;
	*list = new;
	return 0;
}

static int add_instance_walk(const char *name, void *data)
{
	return add_new_instance((struct instances_list **)data, (char *)name);
}

static void clear_list(struct instances_list *list)
{
	struct instances_list *del;

	while (list) {
		del = list;
		list = list->next;
		tracefs_instance_free(del->instance);
		free(del);
	}
}

static void clear_instance_trace(struct tracefs_instance *instance)
{
	FILE *fp;
	char *path;

	/* reset the trace */
	path = tracefs_instance_get_file(instance, "trace");
	fp = fopen(path, "w");
	if (!fp)
		die("writing to '%s'", path);
	tracefs_put_tracing_file(path);
	fwrite("0", 1, 1, fp);
	fclose(fp);
}

static void clear_trace(struct instances_list *instances)
{
	if (instances) {
		while (instances) {
			clear_instance_trace(instances->instance);
			instances = instances->next;
		}
	} else
		clear_instance_trace(NULL);
}

void trace_clear(int argc, char **argv)
{
	struct instances_list *instances = NULL;
	bool all = false;
	int c;

	for (;;) {
		int option_index = 0;
		static struct option long_options[] = {
			{"all", no_argument, NULL, 'a'},
			{"help", no_argument, NULL, '?'},
			{NULL, 0, NULL, 0}
		};

		c = getopt_long (argc-1, argv+1, "+haB:",
				 long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 'B':
			if (add_new_instance(&instances, optarg))
				die("Failed to allocate instance %s", optarg);
			break;
		case 'a':
			all = true;
			if (tracefs_instances_walk(add_instance_walk, &instances))
				die("Failed to add all instances");
			break;
		case 'h':
		case '?':
		default:
			usage(argv);
			break;
		}
	}

	clear_trace(instances);
	if (all)
		clear_trace(NULL);
	clear_list(instances);
	exit(0);
}
