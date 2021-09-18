// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2008, 2009, 2010 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 * Copyright (C) 2020, VMware, Tzvetomir Stoyanov <tz.stoyanov@gmail.com>
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>

#include "trace-local.h"
#include "trace-msg.h"

static struct trace_guest *guests;
static size_t guests_len;

static int set_vcpu_pid_mapping(struct trace_guest *guest, int cpu, int pid)
{
	int *cpu_pid;
	int i;

	if (cpu >= guest->cpu_max) {
		cpu_pid = realloc(guest->cpu_pid, (cpu + 1) * sizeof(int));
		if (!cpu_pid)
			return -1;
		/* Handle sparse CPU numbers */
		for (i = guest->cpu_max; i < cpu; i++)
			cpu_pid[i] = -1;
		guest->cpu_max = cpu + 1;
		guest->cpu_pid = cpu_pid;
	}
	guest->cpu_pid[cpu] = pid;
	return 0;
}

struct trace_guest *get_guest_by_cid(unsigned int guest_cid)
{
	int i;

	if (!guests)
		return NULL;

	for (i = 0; i < guests_len; i++)
		if (guest_cid == guests[i].cid)
			return guests + i;
	return NULL;
}

struct trace_guest *get_guest_by_name(char *name)
{
	int i;

	if (!guests)
		return NULL;

	for (i = 0; i < guests_len; i++)
		if (strcmp(name, guests[i].name) == 0)
			return guests + i;
	return NULL;
}

static char *get_qemu_guest_name(char *arg)
{
	char *tok, *end = arg;

	while ((tok = strsep(&end, ","))) {
		if (strncmp(tok, "guest=", 6) == 0)
			return tok + 6;
	}

	return arg;
}

static int read_qemu_guests_pids(char *guest_task, struct trace_guest *guest)
{
	struct dirent *entry;
	char path[PATH_MAX];
	char *buf = NULL;
	size_t n = 0;
	int ret = 0;
	long vcpu;
	long pid;
	DIR *dir;
	FILE *f;

	snprintf(path, sizeof(path), "/proc/%s/task", guest_task);
	dir = opendir(path);
	if (!dir)
		return -1;

	while (!ret && (entry = readdir(dir))) {
		if (!(entry->d_type == DT_DIR && is_digits(entry->d_name)))
			continue;

		snprintf(path, sizeof(path), "/proc/%s/task/%s/comm",
			 guest_task, entry->d_name);
		f = fopen(path, "r");
		if (!f)
			continue;

		if (getline(&buf, &n, f) >= 0 &&
		    strncmp(buf, "CPU ", 4) == 0) {
			vcpu = strtol(buf + 4, NULL, 10);
			pid = strtol(entry->d_name, NULL, 10);
			if (vcpu < INT_MAX && pid < INT_MAX &&
			    vcpu >= 0 && pid >= 0) {
				if (set_vcpu_pid_mapping(guest, vcpu, pid))
					ret = -1;
			}
		}

		fclose(f);
	}
	free(buf);
	return ret;
}

void read_qemu_guests(void)
{
	static bool initialized;
	struct dirent *entry;
	char path[PATH_MAX];
	DIR *dir;

	if (initialized)
		return;

	initialized = true;
	dir = opendir("/proc");
	if (!dir)
		die("Can not open /proc");

	while ((entry = readdir(dir))) {
		bool is_qemu = false, last_was_name = false;
		struct trace_guest guest = {};
		char *p, *arg = NULL;
		size_t arg_size = 0;
		FILE *f;

		if (!(entry->d_type == DT_DIR && is_digits(entry->d_name)))
			continue;

		guest.pid = atoi(entry->d_name);
		snprintf(path, sizeof(path), "/proc/%s/cmdline", entry->d_name);
		f = fopen(path, "r");
		if (!f)
			continue;

		while (getdelim(&arg, &arg_size, 0, f) != -1) {
			if (!is_qemu && strstr(arg, "qemu-system-")) {
				is_qemu = true;
				continue;
			}

			if (!is_qemu)
				continue;

			if (strcmp(arg, "-name") == 0) {
				last_was_name = true;
				continue;
			}

			if (last_was_name) {
				guest.name = strdup(get_qemu_guest_name(arg));
				if (!guest.name)
					die("allocating guest name");
				last_was_name = false;
				continue;
			}

			p = strstr(arg, "guest-cid=");
			if (p) {
				guest.cid = atoi(p + 10);
				continue;
			}
		}

		if (!is_qemu)
			goto next;

		if (read_qemu_guests_pids(entry->d_name, &guest))
			warning("Failed to retrieve VPCU - PID mapping for guest %s",
					guest.name ? guest.name : "Unknown");

		guests = realloc(guests, (guests_len + 1) * sizeof(*guests));
		if (!guests)
			die("Can not allocate guest buffer");
		guests[guests_len++] = guest;

next:
		free(arg);
		fclose(f);
	}

	closedir(dir);
}

int get_guest_vcpu_pid(unsigned int guest_cid, unsigned int guest_vcpu)
{
	int i;

	if (!guests)
		return -1;

	for (i = 0; i < guests_len; i++) {
		if (guests[i].cpu_pid < 0 || guest_vcpu >= guests[i].cpu_max)
			continue;
		if (guest_cid == guests[i].cid)
			return guests[i].cpu_pid[guest_vcpu];
	}
	return -1;
}
