// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2020, VMware, Tzvetomir Stoyanov <tz.stoyanov@gmail.com>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "tracefs.h"

#define TRACEFS_SUITE		"trasefs library"
#define TEST_INSTANCE_NAME	"cunit_test_iter"
#define TEST_ARRAY_SIZE		500

static struct tracefs_instance *test_instance;
static struct tep_handle *test_tep;
struct test_sample {
	int cpu;
	int value;
};
static struct test_sample test_array[TEST_ARRAY_SIZE];
static int test_found;

static int test_callback(struct tep_event *event, struct tep_record *record,
			  int cpu, void *context)
{
	struct tep_format_field *field;
	struct test_sample *sample;
	int *cpu_test = (int *)context;
	int i;

	if (cpu_test && *cpu_test >= 0 && *cpu_test != cpu)
		return 0;
	field = tep_find_field(event, "buf");
	if (field) {
		sample = ((struct test_sample *)(record->data + field->offset));
		for (i = 0; i < TEST_ARRAY_SIZE; i++) {
			if (test_array[i].value == sample->value &&
			    test_array[i].cpu == cpu) {
				test_array[i].value = 0;
				test_found++;
				break;
			}
		}
	}

	return 0;
}

static void test_iter_write(void)
{
	int cpus = sysconf(_SC_NPROCESSORS_CONF);
	cpu_set_t *cpuset, *cpusave;
	int cpu_size;
	char *path;
	int i, fd;
	int ret;
	cpuset = CPU_ALLOC(cpus);
	cpusave = CPU_ALLOC(cpus);
	cpu_size = CPU_ALLOC_SIZE(cpus);
	CPU_ZERO_S(cpu_size, cpuset);

	sched_getaffinity(0, cpu_size, cpusave);

	path = tracefs_instance_get_file(test_instance, "trace_marker");
	CU_TEST(path != NULL);
	fd = open(path, O_WRONLY);
	tracefs_put_tracing_file(path);
	CU_TEST(fd >= 0);

	for (i = 0; i < TEST_ARRAY_SIZE; i++) {
		test_array[i].cpu = rand() % cpus;
		test_array[i].value = random();
		if (!test_array[i].value)
			test_array[i].value++;
		CU_TEST(test_array[i].cpu < cpus);
		CPU_ZERO_S(cpu_size, cpuset);
		CPU_SET(test_array[i].cpu, cpuset);
		sched_setaffinity(0, cpu_size, cpuset);
		ret = write(fd, test_array + i, sizeof(struct test_sample));
		CU_TEST(ret == sizeof(struct test_sample));
	}

	sched_setaffinity(0, cpu_size, cpusave);
	close(fd);
}


static void iter_raw_events_on_cpu(int cpu)
{
	int check = 0;
	int ret;
	int i;

	test_found = 0;
	test_iter_write();
	ret = tracefs_iterate_raw_events(test_tep, test_instance, NULL, 0,
					 test_callback, &cpu);
	CU_TEST(ret == 0);
	if (cpu < 0) {
		CU_TEST(test_found == TEST_ARRAY_SIZE);
	} else {
		for (i = 0; i < TEST_ARRAY_SIZE; i++) {
			if (test_array[i].cpu == cpu) {
				check++;
				CU_TEST(test_array[i].value == 0)
			} else {
				CU_TEST(test_array[i].value != 0)
			}
		}
		CU_TEST(test_found == check);
	}
}

static void test_iter_raw_events(void)
{
	int cpus = sysconf(_SC_NPROCESSORS_CONF);
	int ret;
	int i;

	ret = tracefs_iterate_raw_events(NULL, test_instance, NULL, 0, test_callback, NULL);
	CU_TEST(ret < 0);
	ret = tracefs_iterate_raw_events(test_tep, NULL, NULL, 0, test_callback, NULL);
	CU_TEST(ret == 0);
	ret = tracefs_iterate_raw_events(test_tep, test_instance, NULL, 0, NULL, NULL);
	CU_TEST(ret < 0);

	iter_raw_events_on_cpu(-1);
	for (i = 0; i < cpus; i++)
		iter_raw_events_on_cpu(i);
}

#define RAND_STR_SIZE 20
#define RAND_ASCII "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static const char *get_rand_str()
{
	static char str[RAND_STR_SIZE];
	static char sym[] = RAND_ASCII;
	struct timespec clk;
	int i;

	clock_gettime(CLOCK_REALTIME, &clk);
	srand(clk.tv_nsec);
	for (i = 0; i < RAND_STR_SIZE; i++)
		str[i] = sym[rand() % (sizeof(sym) - 1)];

	str[RAND_STR_SIZE - 1] = 0;
	return str;
}

static void test_trace_file(void)
{
	const char *tmp = get_rand_str();
	const char *tdir;
	struct stat st;
	char *file;

	tdir  = tracefs_tracing_dir();
	CU_TEST(tdir != NULL);
	CU_TEST(stat(tdir, &st) == 0);
	CU_TEST(S_ISDIR(st.st_mode));

	file = tracefs_get_tracing_file(NULL);
	CU_TEST(file == NULL);
	file = tracefs_get_tracing_file(tmp);
	CU_TEST(file != NULL);
	CU_TEST(stat(file, &st) != 0);
	tracefs_put_tracing_file(file);

	file = tracefs_get_tracing_file("trace");
	CU_TEST(file != NULL);
	CU_TEST(stat(file, &st) == 0);
	tracefs_put_tracing_file(file);
}

static void test_instance_file_read(struct tracefs_instance *inst, char *fname)
{
	const char *tdir  = tracefs_tracing_dir();
	char buf[BUFSIZ];
	char *fpath;
	char *file;
	size_t fsize = 0;
	int size = 0;
	int fd;

	if (inst) {
		CU_TEST(asprintf(&fpath, "%s/instances/%s/%s",
			tdir, tracefs_instance_get_name(inst), fname) > 0);
	} else {
		CU_TEST(asprintf(&fpath, "%s/%s", tdir, fname) > 0);
	}

	memset(buf, 0, BUFSIZ);
	fd = open(fpath, O_RDONLY);
	CU_TEST(fd >= 0);
	fsize = read(fd, buf, BUFSIZ);
	CU_TEST(fsize >= 0);
	close(fd);
	buf[BUFSIZ - 1] = 0;

	file = tracefs_instance_file_read(inst, fname, &size);
	CU_TEST(file != NULL);
	CU_TEST(size == fsize);
	CU_TEST(strcmp(file, buf) == 0);

	free(fpath);
	free(file);
}

#define ALL_TRACERS	"available_tracers"
#define CUR_TRACER	"current_tracer"
#define PER_CPU		"per_cpu"
static void test_instance_file(void)
{
	struct tracefs_instance *instance = NULL;
	struct tracefs_instance *second = NULL;
	const char *name = get_rand_str();
	const char *inst_name = NULL;
	const char *tdir;
	char *inst_file;
	char *inst_dir;
	struct stat st;
	char *fname;
	char *file1;
	char *file2;
	char *tracer;
	int size;
	int ret;

	tdir  = tracefs_tracing_dir();
	CU_TEST(tdir != NULL);
	CU_TEST(asprintf(&inst_dir, "%s/instances/%s", tdir, name) > 0);
	CU_TEST(stat(inst_dir, &st) != 0);

	CU_TEST(tracefs_instance_exists(name) == false);
	instance = tracefs_instance_create(name);
	CU_TEST(instance != NULL);
	CU_TEST(tracefs_instance_is_new(instance));
	second = tracefs_instance_create(name);
	CU_TEST(second != NULL);
	CU_TEST(!tracefs_instance_is_new(second));
	tracefs_instance_free(second);
	CU_TEST(tracefs_instance_exists(name) == true);
	CU_TEST(stat(inst_dir, &st) == 0);
	CU_TEST(S_ISDIR(st.st_mode));
	inst_name = tracefs_instance_get_name(instance);
	CU_TEST(inst_name != NULL);
	CU_TEST(strcmp(inst_name, name) == 0);

	fname = tracefs_instance_get_dir(NULL);
	CU_TEST(fname != NULL);
	CU_TEST(strcmp(fname, tdir) == 0);
	free(fname);

	fname = tracefs_instance_get_dir(instance);
	CU_TEST(fname != NULL);
	CU_TEST(strcmp(fname, inst_dir) == 0);
	free(fname);

	CU_TEST(asprintf(&fname, "%s/"ALL_TRACERS, tdir) > 0);
	CU_TEST(fname != NULL);
	inst_file = tracefs_instance_get_file(NULL, ALL_TRACERS);
	CU_TEST(inst_file != NULL);
	CU_TEST(strcmp(fname, inst_file) == 0);
	tracefs_put_tracing_file(inst_file);
	free(fname);

	CU_TEST(asprintf(&fname, "%s/instances/%s/"ALL_TRACERS, tdir, name) > 0);
	CU_TEST(fname != NULL);
	CU_TEST(stat(fname, &st) == 0);
	inst_file = tracefs_instance_get_file(instance, ALL_TRACERS);
	CU_TEST(inst_file != NULL);
	CU_TEST(strcmp(fname, inst_file) == 0);

	test_instance_file_read(NULL, ALL_TRACERS);
	test_instance_file_read(instance, ALL_TRACERS);

	file1 = tracefs_instance_file_read(instance, ALL_TRACERS, NULL);
	CU_TEST(file1 != NULL);
	tracer = strtok(file1, " ");
	CU_TEST(tracer != NULL);
	ret = tracefs_instance_file_write(instance, CUR_TRACER, tracer);
	CU_TEST(ret == strlen(tracer));
	file2 = tracefs_instance_file_read(instance, CUR_TRACER, &size);
	CU_TEST(file2 != NULL);
	CU_TEST(size >= strlen(tracer));
	CU_TEST(strncmp(file2, tracer, strlen(tracer)) == 0);
	free(file1);
	free(file2);

	tracefs_put_tracing_file(inst_file);
	free(fname);

	CU_TEST(tracefs_file_exists(NULL, (char *)name) == false);
	CU_TEST(tracefs_dir_exists(NULL, (char *)name) == false);
	CU_TEST(tracefs_file_exists(instance, (char *)name) == false);
	CU_TEST(tracefs_dir_exists(instance, (char *)name) == false);

	CU_TEST(tracefs_file_exists(NULL, CUR_TRACER) == true);
	CU_TEST(tracefs_dir_exists(NULL, CUR_TRACER) == false);
	CU_TEST(tracefs_file_exists(instance, CUR_TRACER) == true);
	CU_TEST(tracefs_dir_exists(instance, CUR_TRACER) == false);

	CU_TEST(tracefs_file_exists(NULL, PER_CPU) == false);
	CU_TEST(tracefs_dir_exists(NULL, PER_CPU) == true);
	CU_TEST(tracefs_file_exists(instance, PER_CPU) == false);
	CU_TEST(tracefs_dir_exists(instance, PER_CPU) == true);

	CU_TEST(tracefs_instance_destroy(NULL) != 0);
	CU_TEST(tracefs_instance_destroy(instance) == 0);
	CU_TEST(tracefs_instance_destroy(instance) != 0);
	tracefs_instance_free(instance);
	CU_TEST(stat(inst_dir, &st) != 0);
	free(inst_dir);
}

static void exclude_string(char **strings, char *name)
{
	int i;

	for (i = 0; strings[i]; i++) {
		if (strcmp(strings[i], name) == 0) {
			free(strings[i]);
			strings[i] = strdup("/");
			return;
		}
	}
}

static void test_check_files(const char *fdir, char **files)
{
	struct dirent *dent;
	DIR *dir;
	int i;

	dir = opendir(fdir);
	CU_TEST(dir != NULL);

	while ((dent = readdir(dir)))
		exclude_string(files, dent->d_name);

	closedir(dir);

	for (i = 0; files[i]; i++)
		CU_TEST(files[i][0] == '/');
}

static void test_system_event(void)
{
	const char *tdir;
	char **systems;
	char **events;
	char *sdir = NULL;

	tdir  = tracefs_tracing_dir();
	CU_TEST(tdir != NULL);

	systems = tracefs_event_systems(tdir);
	CU_TEST(systems != NULL);

	events = tracefs_system_events(tdir, systems[0]);
	CU_TEST(events != NULL);

	asprintf(&sdir, "%s/events/%s", tdir, systems[0]);
	CU_TEST(sdir != NULL);
	test_check_files(sdir, events);
	free(sdir);
	sdir = NULL;

	asprintf(&sdir, "%s/events", tdir);
	CU_TEST(sdir != NULL);
	test_check_files(sdir, systems);

	tracefs_list_free(systems);
	tracefs_list_free(events);

	free(sdir);
}

static void test_tracers(void)
{
	const char *tdir;
	char **tracers;
	char *tfile;
	char *tracer;
	int i;

	tdir  = tracefs_tracing_dir();
	CU_TEST(tdir != NULL);

	tracers = tracefs_tracers(tdir);
	CU_TEST(tracers != NULL);

	tfile = tracefs_instance_file_read(NULL, ALL_TRACERS, NULL);

	tracer = strtok(tfile, " ");
	while (tracer) {
		exclude_string(tracers, tracer);
		tracer = strtok(NULL, " ");
	}

	for (i = 0; tracers[i]; i++)
		CU_TEST(tracers[i][0] == '/');

	tracefs_list_free(tracers);
	free(tfile);
}

static void test_check_events(struct tep_handle *tep, char *system, bool exist)
{
	struct dirent *dent;
	char file[PATH_MAX];
	char buf[1024];
	char *edir = NULL;
	const char *tdir;
	DIR *dir;
	int fd;

	tdir  = tracefs_tracing_dir();
	CU_TEST(tdir != NULL);

	asprintf(&edir, "%s/events/%s", tdir, system);
	dir = opendir(edir);
	CU_TEST(dir != NULL);

	while ((dent = readdir(dir))) {
		if (dent->d_name[0] == '.')
			continue;
		sprintf(file, "%s/%s/id", edir, dent->d_name);
		fd = open(file, O_RDONLY);
		if (fd < 0)
			continue;
		CU_TEST(read(fd, buf, 1024) > 0);
		if (exist) {
			CU_TEST(tep_find_event(tep, atoi(buf)) != NULL);
		} else {
			CU_TEST(tep_find_event(tep, atoi(buf)) == NULL);
		}

		close(fd);
	}

	closedir(dir);
	free(edir);

}

static void test_local_events(void)
{
	struct tep_handle *tep;
	const char *tdir;
	char **systems;
	char *lsystems[3];
	int i;

	tdir  = tracefs_tracing_dir();
	CU_TEST(tdir != NULL);

	tep = tracefs_local_events(tdir);
	CU_TEST(tep != NULL);

	systems = tracefs_event_systems(tdir);
	CU_TEST(systems != NULL);

	for (i = 0; systems[i]; i++)
		test_check_events(tep, systems[i], true);
	tep_free(tep);

	memset(lsystems, 0, sizeof(lsystems));
	for (i = 0; systems[i]; i++) {
		if (!lsystems[0])
			lsystems[0] = systems[i];
		else if (!lsystems[2])
			lsystems[2] = systems[i];
		else
			break;
	}

	if (lsystems[0] && lsystems[2]) {
		tep = tracefs_local_events_system(tdir,
						  (const char * const *)lsystems);
		CU_TEST(tep != NULL);
		test_check_events(tep, lsystems[0], true);
		test_check_events(tep, lsystems[2], false);
	}
	tep_free(tep);

	tep = tep_alloc();
	CU_TEST(tep != NULL);
	CU_TEST(tracefs_fill_local_events(tdir, tep, NULL) == 0);
	for (i = 0; systems[i]; i++)
		test_check_events(tep, systems[i], true);

	tep_free(tep);

	tracefs_list_free(systems);
}

struct test_walk_instance {
	struct tracefs_instance *instance;
	bool found;
};
#define WALK_COUNT 10
int test_instances_walk_cb(const char *name, void *data)
{
	struct test_walk_instance *instances  = (struct test_walk_instance *)data;
	int i;

	CU_TEST(instances != NULL);
	CU_TEST(name != NULL);

	for (i = 0; i < WALK_COUNT; i++) {
		if (!strcmp(name,
			    tracefs_instance_get_name(instances[i].instance))) {
			instances[i].found = true;
			break;
		}
	}

	return 0;
}

static void test_instances_walk(void)
{
	struct test_walk_instance instances[WALK_COUNT];
	int i;

	memset(instances, 0, WALK_COUNT * sizeof(struct test_walk_instance));
	for (i = 0; i < WALK_COUNT; i++) {
		instances[i].instance = tracefs_instance_create(get_rand_str());
		CU_TEST(instances[i].instance != NULL);
	}

	CU_TEST(tracefs_instances_walk(test_instances_walk_cb, instances) == 0);
	for (i = 0; i < WALK_COUNT; i++) {
		CU_TEST(instances[i].found);
		tracefs_instance_destroy(instances[i].instance);
		instances[i].found = false;
	}

	CU_TEST(tracefs_instances_walk(test_instances_walk_cb, instances) == 0);
	for (i = 0; i < WALK_COUNT; i++) {
		CU_TEST(!instances[i].found);
		tracefs_instance_free(instances[i].instance);
	}
}

static void current_clock_check(const char *clock)
{
	int size = 0;
	char *clocks;
	char *str;

	clocks = tracefs_instance_file_read(test_instance, "trace_clock", &size);
	CU_TEST(clocks != NULL);
	CU_TEST(size > strlen(clock));
	str = strstr(clocks, clock);
	CU_TEST(str != NULL);
	CU_TEST(str != clocks);
	CU_TEST(*(str - 1) == '[');
	CU_TEST(*(str + strlen(clock)) == ']');
	free(clocks);
}

static void test_get_clock(void)
{
	const char *clock;

	clock = tracefs_get_clock(test_instance);
	CU_TEST(clock != NULL);
	current_clock_check(clock);
	free((char *)clock);
}

static int test_suite_destroy(void)
{
	tracefs_instance_destroy(test_instance);
	tracefs_instance_free(test_instance);
	tep_free(test_tep);
	return 0;
}

static int test_suite_init(void)
{
	const char *systems[] = {"ftrace", NULL};

	test_tep = tracefs_local_events_system(NULL, systems);
	if (test_tep == NULL)
		return 1;
	test_instance = tracefs_instance_create(TEST_INSTANCE_NAME);
	if (!test_instance)
		return 1;

	return 0;
}

void test_tracefs_lib(void)
{
	CU_pSuite suite = NULL;

	suite = CU_add_suite(TRACEFS_SUITE, test_suite_init, test_suite_destroy);
	if (suite == NULL) {
		fprintf(stderr, "Suite \"%s\" cannot be ceated\n", TRACEFS_SUITE);
		return;
	}
	CU_add_test(suite, "tracing file / directory APIs",
		    test_trace_file);
	CU_add_test(suite, "instance file / directory APIs",
		    test_instance_file);
	CU_add_test(suite, "systems and events APIs",
		    test_system_event);
	CU_add_test(suite, "tracefs_iterate_raw_events API",
		    test_iter_raw_events);
	CU_add_test(suite, "tracefs_tracers API",
		    test_tracers);
	CU_add_test(suite, "tracefs_local events API",
		    test_local_events);
	CU_add_test(suite, "tracefs_instances_walk API",
		    test_instances_walk);
	CU_add_test(suite, "tracefs_get_clock API",
		    test_get_clock);
}
