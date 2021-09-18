// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2010 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 */
#define _LARGEFILE64_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "trace-local.h"

static unsigned int page_size;
static const char *default_input_file = DEFAULT_INPUT_FILE;
static const char *input_file;

enum split_types {
	SPLIT_NONE,
	/* The order of these must be reverse of the case statement in the options */
	SPLIT_SECONDS,
	SPLIT_MSECS,
	SPLIT_USECS,
	SPLIT_EVENTS,
	SPLIT_PAGES,
	SPLIT_NR_TYPES,
};

struct cpu_data {
	unsigned long long		ts;
	unsigned long long		offset;
	struct tep_record		*record;
	int				cpu;
	int				fd;
	int				index;
	void				*commit;
	void				*page;
	char				*file;
};

static int create_type_len(struct tep_handle *pevent, int time, int len)
{
	static int bigendian = -1;
	char *ptr;
	int test;

	if (bigendian < 0) {
		test = 0x4321;
		ptr = (char *)&test;
		if (*ptr == 0x21)
			bigendian = 0;
		else
			bigendian = 1;
	}

	if (tep_is_file_bigendian(pevent))
		time |= (len << 27);
	else
		time = (time << 5) | len;

	return tep_read_number(pevent, &time, 4);
}

static int write_record(struct tracecmd_input *handle,
			struct tep_record *record,
			struct cpu_data *cpu_data,
			enum split_types type)
{
	unsigned long long diff;
	struct tep_handle *pevent;
	void *page;
	int len = 0;
	char *ptr;
	int index = 0;
	int time;

	page = cpu_data->page;

	pevent = tracecmd_get_tep(handle);

	ptr = page + cpu_data->index;

	diff = record->ts - cpu_data->ts;
	if (diff > (1 << 27)) {
		/* Add a time stamp */
		len = RINGBUF_TYPE_TIME_EXTEND;
		time = (unsigned int)(diff & ((1ULL << 27) - 1));
		time = create_type_len(pevent, time, len);
		*(unsigned *)ptr = time;
		ptr += 4;
		time = (unsigned int)(diff >> 27);
		*(unsigned *)ptr = tep_read_number(pevent, &time, 4);
		cpu_data->ts = record->ts;
		cpu_data->index += 8;
		return 0;
	}

	if (record->size && (record->size < 28 * 4))
		len = record->size / 4;

	time = (unsigned)diff;
	time = create_type_len(pevent, time, len);

	memcpy(ptr, &time, 4);
	ptr += 4;
	index = 4;

	if (!len) {
		len = record->size + 4;
		*(unsigned *)ptr = tep_read_number(pevent, &len, 4);
		ptr += 4;
		index += 4;
	}

	len = (record->size + 3) & ~3;
	index += len;

	memcpy(ptr, record->data, len);

	cpu_data->index += index;
	cpu_data->ts = record->ts;

	return 1;
}

static void write_page(struct tep_handle *pevent,
		       struct cpu_data *cpu_data, int long_size)
{
	if (long_size == 8) {
		unsigned long long index = cpu_data->index - 16;
		*(unsigned long long *)cpu_data->commit =
				tep_read_number(pevent, &index, 8);
	} else {
		unsigned int index = cpu_data->index - 12;
		*(unsigned int *)cpu_data->commit =
			tep_read_number(pevent, &index, 4);
	}
	write(cpu_data->fd, cpu_data->page, page_size);
}

static struct tep_record *read_record(struct tracecmd_input *handle,
				      int percpu, int *cpu)
{
	if (percpu)
		return tracecmd_read_data(handle, *cpu);

	return tracecmd_read_next_data(handle, cpu);
}

static void set_cpu_time(struct tracecmd_input *handle,
			 int percpu, unsigned long long start, int cpu, int cpus)
{
	if (percpu) {
		tracecmd_set_cpu_to_timestamp(handle, cpu, start);
		return;
	}

	for (cpu = 0; cpu < cpus; cpu++)
		tracecmd_set_cpu_to_timestamp(handle, cpu, start);
	return;
}

static int parse_cpu(struct tracecmd_input *handle,
		     struct cpu_data *cpu_data,
		     unsigned long long start,
		     unsigned long long end,
		     int count_limit, int percpu, int cpu,
		     enum split_types type)
{
	struct tep_record *record;
	struct tep_handle *pevent;
	void *ptr;
	int page_size;
	int long_size = 0;
	int cpus;
	int count = 0;
	int pages = 0;

	cpus = tracecmd_cpus(handle);

	long_size = tracecmd_long_size(handle);
	page_size = tracecmd_page_size(handle);
	pevent = tracecmd_get_tep(handle);

	/* Force new creation of first page */
	if (percpu) {
		cpu_data[cpu].index = page_size + 1;
		cpu_data[cpu].page = NULL;
	} else {
		for (cpu = 0; cpu < cpus; cpu++) {
			cpu_data[cpu].index = page_size + 1;
			cpu_data[cpu].page = NULL;
		}
	}

	/*
	 * Get the cpu pointers up to the start of the
	 * start time stamp.
	 */

	record = read_record(handle, percpu, &cpu);

	if (start) {
		set_cpu_time(handle, percpu, start, cpu, cpus);
		while (record && record->ts < start) {
			tracecmd_free_record(record);
			record = read_record(handle, percpu, &cpu);
		}
	} else if (record)
		start = record->ts;

	while (record && (!end || record->ts <= end)) {
		if (cpu_data[cpu].index + record->record_size > page_size) {

			if (type == SPLIT_PAGES && ++pages > count_limit)
				break;

			if (cpu_data[cpu].page)
				write_page(pevent, &cpu_data[cpu], long_size);
			else {
				cpu_data[cpu].page = malloc(page_size);
				if (!cpu_data[cpu].page)
					die("Failed to allocate page");
			}

			memset(cpu_data[cpu].page, 0, page_size);
			ptr = cpu_data[cpu].page;

			*(unsigned long long*)ptr =
				tep_read_number(pevent, &(record->ts), 8);
			cpu_data[cpu].ts = record->ts;
			ptr += 8;
			cpu_data[cpu].commit = ptr;
			ptr += long_size;
			cpu_data[cpu].index = 8 + long_size;
		}

		cpu_data[cpu].offset = record->offset;

		if (write_record(handle, record, &cpu_data[cpu], type)) {
			tracecmd_free_record(record);
			record = read_record(handle, percpu, &cpu);

			/* if we hit the end of the cpu, clear the offset */
			if (!record) {
				if (percpu)
					cpu_data[cpu].offset = 0;
				else
					for (cpu = 0; cpu < cpus; cpu++)
						cpu_data[cpu].offset = 0;
			}

			switch (type) {
			case SPLIT_NONE:
				break;
			case SPLIT_SECONDS:
				if (record &&
				    record->ts >
				    (start + (unsigned long long)count_limit * 1000000000ULL)) {
					tracecmd_free_record(record);
					record = NULL;
				}
				break;
			case SPLIT_MSECS:
				if (record &&
				    record->ts >
				    (start + (unsigned long long)count_limit * 1000000ULL)) {
					tracecmd_free_record(record);
					record = NULL;
				}
				break;
			case SPLIT_USECS:
				if (record &&
				    record->ts >
				    (start + (unsigned long long)count_limit * 1000ULL)) {
					tracecmd_free_record(record);
					record = NULL;
				}
				break;
			case SPLIT_EVENTS:
				if (++count >= count_limit) {
					tracecmd_free_record(record);
					record = NULL;
				}
				break;
			default:
				break;
			}
		}
	}

	if (record)
		tracecmd_free_record(record);

	if (percpu) {
		if (cpu_data[cpu].page) {
			write_page(pevent, &cpu_data[cpu], long_size);
			free(cpu_data[cpu].page);
			cpu_data[cpu].page = NULL;
		}
	} else {
		for (cpu = 0; cpu < cpus; cpu++) {
			if (cpu_data[cpu].page) {
				write_page(pevent, &cpu_data[cpu], long_size);
				free(cpu_data[cpu].page);
				cpu_data[cpu].page = NULL;
			}
		}
	}

	return 0;
}

static double parse_file(struct tracecmd_input *handle,
			 const char *output_file,
			 unsigned long long start,
			 unsigned long long end, int percpu, int only_cpu,
			 int count, enum split_types type)
{
	unsigned long long current;
	struct tracecmd_output *ohandle;
	struct cpu_data *cpu_data;
	struct tep_record *record;
	char **cpu_list;
	char *output;
	char *base;
	char *file;
	char *dir;
	int cpus;
	int cpu;
	int fd;

	output = strdup(output_file);
	dir = dirname(output);
	base = basename(output);

	ohandle = tracecmd_copy(handle, output_file);

	cpus = tracecmd_cpus(handle);
	cpu_data = malloc(sizeof(*cpu_data) * cpus);
	if (!cpu_data)
		die("Failed to allocate cpu_data for %d cpus", cpus);

	for (cpu = 0; cpu < cpus; cpu++) {
		int ret;

		ret = asprintf(&file, "%s/.tmp.%s.%d", dir, base, cpu);
		if (ret < 0)
			die("Failed to allocate file for %s %s %d", dir, base, cpu);
		fd = open(file, O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE, 0644);
		cpu_data[cpu].cpu = cpu;
		cpu_data[cpu].fd = fd;
		cpu_data[cpu].file = file;
		cpu_data[cpu].offset = 0;
		if (start)
			tracecmd_set_cpu_to_timestamp(handle, cpu, start);
	}

	if (only_cpu >= 0) {
		parse_cpu(handle, cpu_data, start, end, count,
			  1, only_cpu, type);
	} else if (percpu) {
		for (cpu = 0; cpu < cpus; cpu++)
			parse_cpu(handle, cpu_data, start,
				  end, count, percpu, cpu, type);
	} else
		parse_cpu(handle, cpu_data, start,
			  end, count, percpu, -1, type);

	cpu_list = malloc(sizeof(*cpu_list) * cpus);
	if (!cpu_list)
		die("Failed to allocate cpu_list for %d cpus", cpus);
	for (cpu = 0; cpu < cpus; cpu ++)
		cpu_list[cpu] = cpu_data[cpu].file;

	tracecmd_append_cpu_data(ohandle, cpus, cpu_list);

	current = end;
	for (cpu = 0; cpu < cpus; cpu++) {
		/* Set the tracecmd cursor to the next set of records */
		if (cpu_data[cpu].offset) {
			record = tracecmd_read_at(handle, cpu_data[cpu].offset, NULL);
			if (record && (!current || record->ts > current))
				current = record->ts + 1;
			tracecmd_free_record(record);
		}
		unlink(cpu_data[cpu].file);
		free(cpu_data[cpu].file);
	}
	free(cpu_data);
	free(cpu_list);
	free(output);
	tracecmd_output_close(ohandle);

	return current;
}

void trace_split (int argc, char **argv)
{
	struct tracecmd_input *handle;
	unsigned long long start_ns = 0, end_ns = 0;
	unsigned long long current;
	double start, end;
	char *endptr;
	char *output = NULL;
	char *output_file;
	enum split_types split_type = SPLIT_NONE;
	enum split_types type = SPLIT_NONE;
	int count;
	int repeat = 0;
	int percpu = 0;
	int cpu = -1;
	int ac;
	int c;

	if (strcmp(argv[1], "split") != 0)
		usage(argv);

	while ((c = getopt(argc-1, argv+1, "+ho:i:s:m:u:e:p:rcC:")) >= 0) {
		switch (c) {
		case 'h':
			usage(argv);
			break;
		case 'p':
			type++;
		case 'e':
			type++;
		case 'u':
			type++;
		case 'm':
			type++;
		case 's':
			type++;
			if (split_type != SPLIT_NONE)
				die("Only one type of split is allowed");
			count = atoi(optarg);
			if (count <= 0)
				die("Units must be greater than 0");
			split_type = type;

			/* Spliting by pages only makes sense per cpu */
			if (type == SPLIT_PAGES)
				percpu = 1;
			break;
		case 'r':
			repeat = 1;
			break;
		case 'c':
			percpu = 1;
			break;
		case 'C':
			cpu = atoi(optarg);
			break;
		case 'o':
			if (output)
				die("only one output file allowed");
			output = strdup(optarg);
			break;
		case 'i':
			input_file = optarg;
			break;
		default:
			usage(argv);
		}
	}

	ac = (argc - optind);

	if (ac >= 2) {
		optind++;
		start = strtod(argv[optind], &endptr);
		if (ac > 3)
			usage(argv);

		/* Make sure a true start value was entered */
		if (*endptr != 0)
			die("Start value not floating point: %s", argv[optind]);

		start_ns = (unsigned long long)(start * 1000000000.0);
		optind++;
		if (ac == 3) {
			end = strtod(argv[optind], &endptr);

			/* Make sure a true end value was entered */
			if (*endptr != 0)
				die("End value not floating point: %s",
				    argv[optind]);

			end_ns = (unsigned long long)(end * 1000000000.0);
			if (end_ns < start_ns)
				die("Error: end is less than start");
		}
	}

	if (!input_file)
		input_file = default_input_file;

	handle = tracecmd_open(input_file, 0);
	if (!handle)
		die("error reading %s", input_file);

	if (tracecmd_get_file_state(handle) == TRACECMD_FILE_CPU_LATENCY)
		die("trace-cmd split does not work with latency traces\n");

	page_size = tracecmd_page_size(handle);

	if (!output)
		output = strdup(input_file);

	if (!repeat) {
		output = realloc(output, strlen(output) + 3);
		strcat(output, ".1");
	}

	current = start_ns;
	output_file = malloc(strlen(output) + 50);
	if (!output_file)
		die("Failed to allocate for %s", output);
	c = 1;

	do {
		if (repeat)
			sprintf(output_file, "%s.%04d", output, c++);
		else
			strcpy(output_file, output);
			
		current = parse_file(handle, output_file, start_ns, end_ns,
				     percpu, cpu, count, type);
		if (!repeat)
			break;
		start_ns = 0;
	} while (current && (!end_ns || current < end_ns));

	free(output);
	free(output_file);

	tracecmd_close(handle);

	return;
}
