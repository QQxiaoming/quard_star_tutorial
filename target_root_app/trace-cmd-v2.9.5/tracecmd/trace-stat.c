// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2014 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include "tracefs.h"
#include "trace-local.h"

#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

static inline int is_top_instance(struct buffer_instance *instance)
{
	return instance == &top_instance;
}

static int get_instance_file_fd(struct buffer_instance *instance,
				const char *file)
{
	char *path;
	int fd;

	path = tracefs_instance_get_file(instance->tracefs, file);
	fd = open(path, O_RDONLY);
	tracefs_put_tracing_file(path);

	return fd;
}

char *strstrip(char *str)
{
	char *s;

	if (!str)
		return NULL;

	s = str + strlen(str) - 1;
	while (s >= str && isspace(*s))
		s--;
	s++;
	*s = '\0';

	for (s = str; *s && isspace(*s); s++)
		;

	return s;
}

/* FIXME: append_file() is duplicated and could be consolidated */
char *append_file(const char *dir, const char *name)
{
	char *file;
	int ret;

	ret = asprintf(&file, "%s/%s", dir, name);
	if (ret < 0)
		die("Failed to allocate %s/%s", dir, name);

	return file;
}

static char *get_fd_content(int fd, const char *file)
{
	char *str = NULL;
	int cnt = 0;
	int ret;

	for (;;) {
		str = realloc(str, BUFSIZ * ++cnt);
		if (!str)
			die("malloc");
		ret = read(fd, str + BUFSIZ * (cnt - 1), BUFSIZ);
		if (ret < 0)
			die("reading %s\n", file);
		if (ret < BUFSIZ)
			break;
	}
	str[BUFSIZ * (cnt-1) + ret] = 0;

	return str;
}

char *get_file_content(const char *file)
{
	char *str;
	int fd;

	fd = open(file, O_RDONLY);
	if (fd < 0)
		return NULL;

	str = get_fd_content(fd, file);
	close(fd);

	return str;
}

static char *get_instance_file_content(struct buffer_instance *instance,
				       const char *file)
{
	char *str = NULL;
	int fd;

	fd = get_instance_file_fd(instance, file);
	if (fd < 0)
		return NULL;

	str = get_fd_content(fd, file);

	close(fd);
	return str;
}

static void report_file(struct buffer_instance *instance,
			char *name, char *def_value, char *description)
{
	char *str;
	char *cont;

	if (!tracefs_file_exists(instance->tracefs, name))
		return;
	str = get_instance_file_content(instance, name);
	if (!str)
		return;
	cont = strstrip(str);
	if (cont[0] && strcmp(cont, def_value) != 0)
		printf("\n%s%s\n", description, cont);

	free(str);
}

static int report_instance(const char *name, void *data)
{
	bool *first = (bool *)data;

	if (*first) {
		*first = false;
		printf("\nInstances:\n");
	}
	printf(" %s\n", name);
	return 0;
}

static void report_instances(void)
{
	bool first = true;

	tracefs_instances_walk(report_instance, &first);
}

struct event_iter *trace_event_iter_alloc(const char *path)
{
	struct event_iter *iter;

	iter = malloc(sizeof(*iter));
	if (!iter)
		die("Failed to allocate event_iter for path %s", path);
	memset(iter, 0, sizeof(*iter));

	iter->system_dir = opendir(path);
	if (!iter->system_dir)
		die("opendir");

	return iter;
}

enum event_iter_type
trace_event_iter_next(struct event_iter *iter, const char *path, const char *system)
{
	struct dirent *dent;

	if (system && !iter->event_dir) {
		char *event;
		struct stat st;

		event = append_file(path, system);

		stat(event, &st);
		if (!S_ISDIR(st.st_mode)) {
			free(event);
			goto do_system;
		}

		iter->event_dir = opendir(event);
		if (!iter->event_dir)
			die("opendir %s", event);
		free(event);
	}

	if (iter->event_dir) {
		while ((dent = readdir(iter->event_dir))) {
			const char *name = dent->d_name;

			if (strcmp(name, ".") == 0 ||
			    strcmp(name, "..") == 0)
				continue;

			iter->event_dent = dent;
			return EVENT_ITER_EVENT;
		}
		closedir(iter->event_dir);
		iter->event_dir = NULL;
	}

 do_system:
	while ((dent = readdir(iter->system_dir))) {
		const char *name = dent->d_name;

		if (strcmp(name, ".") == 0 ||
		    strcmp(name, "..") == 0)
			continue;

		iter->system_dent = dent;

		return EVENT_ITER_SYSTEM;
	}

	return EVENT_ITER_NONE;
}

void trace_event_iter_free(struct event_iter *iter)
{
	if (!iter)
		return;

	if (iter->event_dir)
		closedir(iter->event_dir);

	closedir(iter->system_dir);
	free(iter);
}

static void reset_event_iter(struct event_iter *iter)
{
	if (iter->event_dir) {
		closedir(iter->event_dir);
		iter->event_dir = NULL;
	}

	rewinddir(iter->system_dir);
}

static int process_individual_events(const char *path, struct event_iter *iter)
{
	struct stat st;
	const char *system = iter->system_dent->d_name;
	char *file;
	char *enable = NULL;
	char *str;
	int ret = 0;

	file = append_file(path, system);

	stat(file, &st);
	if (!S_ISDIR(st.st_mode))
		goto out;

	enable = append_file(file, "enable");
	str = get_file_content(enable);
	if (!str)
		goto out;

	if (*str != '1' && *str != '0')
		ret = 1;
	free(str);

 out:
	free(enable);
	free(file);

	return ret;
}

static void
process_event_enable(char *path, const char *system, const char *name,
		     enum event_process *processed)
{
	struct stat st;
	char *enable = NULL;
	char *file;
	char *str;

	if (system)
		path = append_file(path, system);

	file = append_file(path, name);

	if (system)
		free(path);

	stat(file, &st);
	if (!S_ISDIR(st.st_mode))
		goto out;

	enable = append_file(file, "enable");
	str = get_file_content(enable);
	if (!str)
		goto out;

	if (*str == '1') {
		if (!system) {
			if (!*processed)
				printf(" Individual systems:\n");
			printf( "   %s\n", name);
			*processed = PROCESSED_SYSTEM;
		} else {
			if (!*processed) {
				printf(" Individual events:\n");
				*processed = PROCESSED_SYSTEM;
			}
			if (*processed == PROCESSED_SYSTEM) {
				printf("    %s\n", system);
				*processed = PROCESSED_EVENT;
			}
			printf( "        %s\n", name);
		}
	}
	free(str);

 out:
	free(enable);
	free(file);
}

static void report_events(struct buffer_instance *instance)
{
	struct event_iter *iter;
	char *str;
	char *cont;
	char *path;
	char *system;
	enum event_iter_type type;
	enum event_process processed = PROCESSED_NONE;
	enum event_process processed_part = PROCESSED_NONE;

	str = get_instance_file_content(instance, "events/enable");
	if (!str)
		return;

	cont = strstrip(str);

	printf("\nEvents:\n");

	switch(*cont) {
	case '1':
		printf(" All enabled\n");
		free(str);
		return;
	case '0':
		printf(" All disabled\n");
		free(str);
		return;
	}

	free(str);

	path = tracefs_instance_get_file(instance->tracefs, "events");
	if (!path)
		die("malloc");

	iter = trace_event_iter_alloc(path);

	while (trace_event_iter_next(iter, path, NULL)) {
		process_event_enable(path, NULL, iter->system_dent->d_name, &processed);
	}

	reset_event_iter(iter);

	system = NULL;
	while ((type = trace_event_iter_next(iter, path, system))) {

		if (type == EVENT_ITER_SYSTEM) {

			/* Only process systems that are not fully enabled */
			if (!process_individual_events(path, iter))
				continue;

			system = iter->system_dent->d_name;
			if (processed_part)
				processed_part = PROCESSED_SYSTEM;
			continue;
		}

		process_event_enable(path, iter->system_dent->d_name,
				     iter->event_dent->d_name, &processed_part);
	}

	trace_event_iter_free(iter);

	if (!processed && !processed_part)
		printf("  (none enabled)\n");

	tracefs_put_tracing_file(path);
}

static void
process_event_filter(char *path, struct event_iter *iter, enum event_process *processed)
{
	const char *system = iter->system_dent->d_name;
	const char *event = iter->event_dent->d_name;
	struct stat st;
	char *filter = NULL;
	char *file;
	char *str;
	char *cont;

	path = append_file(path, system);
	file = append_file(path, event);
	free(path);

	stat(file, &st);
	if (!S_ISDIR(st.st_mode))
		goto out;

	filter = append_file(file, "filter");
	str = get_file_content(filter);
	if (!str)
		goto out;

	cont = strstrip(str);

	if (strcmp(cont, "none") == 0) {
		free(str);
		goto out;
	}

	if (!*processed)
		printf("\nFilters:\n");
	printf( "  %s:%s \"%s\"\n", system, event, cont);
	*processed = PROCESSED_SYSTEM;
	free(str);

 out:
	free(filter);
	free(file);
}

static void report_event_filters(struct buffer_instance *instance)
{
	struct event_iter *iter;
	char *path;
	char *system;
	enum event_iter_type type;
	enum event_process processed = PROCESSED_NONE;

	path = tracefs_instance_get_file(instance->tracefs, "events");
	if (!path)
		die("malloc");

	iter = trace_event_iter_alloc(path);

	processed = PROCESSED_NONE;
	system = NULL;
	while ((type = trace_event_iter_next(iter, path, system))) {

		if (type == EVENT_ITER_SYSTEM) {
			system = iter->system_dent->d_name;
			continue;
		}

		process_event_filter(path, iter, &processed);
	}

	trace_event_iter_free(iter);

	tracefs_put_tracing_file(path);
}

static void
process_event_trigger(char *path, struct event_iter *iter, enum event_process *processed)
{
	const char *system = iter->system_dent->d_name;
	const char *event = iter->event_dent->d_name;
	struct stat st;
	char *trigger = NULL;
	char *file;
	char *str;
	char *cont;

	path = append_file(path, system);
	file = append_file(path, event);
	free(path);

	stat(file, &st);
	if (!S_ISDIR(st.st_mode))
		goto out;

	trigger = append_file(file, "trigger");
	str = get_file_content(trigger);
	if (!str)
		goto out;

	cont = strstrip(str);

	if (cont[0] == '#') {
		free(str);
		goto out;
	}

	if (!*processed)
		printf("\nTriggers:\n");
	printf( "  %s:%s \"%s\"\n", system, event, cont);
	*processed = PROCESSED_SYSTEM;
	free(str);

 out:
	free(trigger);
	free(file);
}

static void report_event_triggers(struct buffer_instance *instance)
{
	struct event_iter *iter;
	char *path;
	char *system;
	enum event_iter_type type;
	enum event_process processed = PROCESSED_NONE;

	path = tracefs_instance_get_file(instance->tracefs, "events");
	if (!path)
		die("malloc");

	iter = trace_event_iter_alloc(path);

	processed = PROCESSED_NONE;
	system = NULL;
	while ((type = trace_event_iter_next(iter, path, system))) {

		if (type == EVENT_ITER_SYSTEM) {
			system = iter->system_dent->d_name;
			continue;
		}

		process_event_trigger(path, iter, &processed);
	}

	trace_event_iter_free(iter);

	tracefs_put_tracing_file(path);
}

enum func_states {
	FUNC_STATE_START,
	FUNC_STATE_SKIP,
	FUNC_STATE_PRINT,
};

static void list_functions(const char *path, char *string)
{
	enum func_states state;
	struct stat st;
	char *str;
	int ret = 0;
	int len;
	int i;
	int first = 0;

	/* Ignore if it does not exist. */
	ret = stat(path, &st);
	if (ret < 0)
		return;

	str = get_file_content(path);
	if (!str)
		return;

	len = strlen(str);

	state = FUNC_STATE_START;

	/* Skip all lines that start with '#' */
	for (i = 0; i < len; i++) {

		if (state == FUNC_STATE_PRINT)
			putchar(str[i]);

		if (str[i] == '\n') {
			state = FUNC_STATE_START;
			continue;
		}

		if (state == FUNC_STATE_SKIP)
			continue;

		if (state == FUNC_STATE_START && str[i] == '#') {
			state = FUNC_STATE_SKIP;
			continue;
		}

		if (!first) {
			printf("\n%s:\n", string);
			first = 1;
		}

		if (state != FUNC_STATE_PRINT) {
			state = FUNC_STATE_PRINT;
			printf("   ");
			putchar(str[i]);
		}
	}
	free(str);
}

static void report_graph_funcs(struct buffer_instance *instance)
{
	char *path;

	path = tracefs_instance_get_file(instance->tracefs, "set_graph_function");
	if (!path)
		die("malloc");

	list_functions(path, "Function Graph Filter");
	
	tracefs_put_tracing_file(path);

	path = tracefs_instance_get_file(instance->tracefs, "set_graph_notrace");
	if (!path)
		die("malloc");

	list_functions(path, "Function Graph No Trace");
	
	tracefs_put_tracing_file(path);
}

static void report_ftrace_filters(struct buffer_instance *instance)
{
	char *path;

	path = tracefs_instance_get_file(instance->tracefs, "set_ftrace_filter");
	if (!path)
		die("malloc");

	list_functions(path, "Function Filter");
	
	tracefs_put_tracing_file(path);

	path = tracefs_instance_get_file(instance->tracefs, "set_ftrace_notrace");
	if (!path)
		die("malloc");

	list_functions(path, "Function No Trace");
	
	tracefs_put_tracing_file(path);
}

static void report_buffers(struct buffer_instance *instance)
{
#define FILE_SIZE 100
	char *str;
	char *cont;
	char file[FILE_SIZE];
	int cpu;

	str = get_instance_file_content(instance, "buffer_size_kb");
	if (!str)
		return;

	cont = strstrip(str);

	/* If it's not expanded yet, just skip */
	if (strstr(cont, "expanded") != NULL)
		goto out;

	if (strcmp(cont, "X") != 0) {
		printf("\nBuffer size in kilobytes (per cpu):\n");
		printf("   %s\n", str);
		goto total;
	}

	/* Read the sizes of each CPU buffer */
	for (cpu = 0; ; cpu++) {

		snprintf(file, FILE_SIZE, "per_cpu/cpu%d/buffer_size_kb", cpu);
		str = get_instance_file_content(instance, file);
		if (!str)
			break;

		cont = strstrip(str);
		if (!cpu)
			putchar('\n');

		printf("CPU %d buffer size (kb): %s\n", cpu, cont);
		free(str);
	}

 total:
	free(str);

	str = get_instance_file_content(instance, "buffer_total_size_kb");
	if (!str)
		return;

	cont = strstrip(str);
	printf("\nBuffer total size in kilobytes:\n");
	printf("   %s\n", str);

 out:
	free(str);
}

static void report_clock(struct buffer_instance *instance)
{
	struct tracefs_instance *tracefs = instance ? instance->tracefs : NULL;
	char *clock;

	clock = tracefs_get_clock(tracefs);

	/* Default clock is "local", only show others */
	if (clock && !strcmp(clock, "local") == 0)
		printf("\nClock: %s\n", clock);

	free(clock);
}

static void report_cpumask(struct buffer_instance *instance)
{
	char *str;
	char *cont;
	int cpus;
	int n;
	int i;

	str = get_instance_file_content(instance, "tracing_cpumask");
	if (!str)
		return;

	cont = strstrip(str);

	/* check to make sure all CPUs on this machine are set */
	cpus = tracecmd_count_cpus();

	for (i = strlen(cont) - 1; i >= 0 && cpus > 0; i--) {
		if (cont[i] == ',')
			continue;

		if (cont[i] == 'f') {
			cpus -= 4;
			continue;
		}

		if (cpus >= 4)
			break;

		if (cont[i] >= '0' && cont[i] <= '9')
			n = cont[i] - '0';
		else
			n = 10 + (cont[i] - 'a');

		while (cpus > 0) {
			if (!(n & 1))
				break;
			n >>= 1;
			cpus--;
		}
		break;
	}

	/* If cpus is greater than zero, one isn't set */
	if (cpus > 0)
		printf("\nCPU mask: %s\n", cont);

	free(str);
}

static void report_probes(struct buffer_instance *instance,
			  const char *file, const char *string)
{
	char *str;
	char *cont;
	int newline;
	int i;

	str = get_instance_file_content(instance, file);
	if (!str)
		return;

	cont = strstrip(str);
	if (strlen(cont) == 0)
		goto out;

	printf("\n%s:\n", string);

	newline = 1;
	for (i = 0; cont[i]; i++) {
		if (newline)
			printf("   ");
		putchar(cont[i]);
		if (cont[i] == '\n')
			newline = 1;
		else
			newline = 0;
	}
	putchar('\n');
 out:
	free(str);
}

static void report_kprobes(struct buffer_instance *instance)
{
	report_probes(instance, "kprobe_events", "Kprobe events");
}

static void report_uprobes(struct buffer_instance *instance)
{
	report_probes(instance, "uprobe_events", "Uprobe events");
}

static void report_traceon(struct buffer_instance *instance)
{
	char *str;
	char *cont;

	str = get_instance_file_content(instance, "tracing_on");
	if (!str)
		return;

	cont = strstrip(str);

	/* double newline as this is the last thing printed */
	if (strcmp(cont, "0") == 0)
		printf("\nTracing is disabled\n\n");
	else
		printf("\nTracing is enabled\n\n");

	free(str);
}

static void stat_instance(struct buffer_instance *instance)
{
	if (instance != &top_instance) {
		if (instance != first_instance)
			printf("---------------\n");
		printf("Instance: %s\n",
			tracefs_instance_get_name(instance->tracefs));
	}

	report_file(instance, "current_tracer", "nop", "Tracer: ");
	report_events(instance);
	report_event_filters(instance);
	report_event_triggers(instance);
	report_ftrace_filters(instance);
	report_graph_funcs(instance);
	report_buffers(instance);
	report_clock(instance);
	report_cpumask(instance);
	report_file(instance, "tracing_max_latency", "0", "Max Latency: ");
	report_kprobes(instance);
	report_uprobes(instance);
	report_file(instance, "set_event_pid", "", "Filtered event PIDs:\n");
	report_file(instance, "set_ftrace_pid", "no pid",
		    "Filtered function tracer PIDs:\n");
	report_traceon(instance);
	report_file(instance, "error_log", "", "Error log:\n");
	if (instance == &top_instance)
		report_instances();
}

void trace_stat (int argc, char **argv)
{
	struct buffer_instance *instance = &top_instance;
	int topt = 0;
	int status;
	int c;

	init_top_instance();

	for (;;) {
		c = getopt(argc-1, argv+1, "tB:");
		if (c == -1)
			break;
		switch (c) {
		case 'h':
			usage(argv);
			break;
		case 'B':
			instance = allocate_instance(optarg);
			if (!instance)
				die("Failed to create instance");
			add_instance(instance, tracecmd_count_cpus());
			/* top instance requires direct access */
			if (!topt && is_top_instance(first_instance))
				first_instance = instance;
			break;
		case 't':
			/* Force to use top instance */
			topt = 1;
			instance = &top_instance;
			break;
		default:
			usage(argv);
		}
	}

	update_first_instance(instance, topt);

	for_all_instances(instance) {
		stat_instance(instance);
	}

	if (tracecmd_stack_tracer_status(&status) >= 0) {
		if (status > 0)
			printf("Stack tracing is enabled\n\n");
	} else {
		printf("Error reading stack tracer status\n\n");
	}

	exit(0);
}
