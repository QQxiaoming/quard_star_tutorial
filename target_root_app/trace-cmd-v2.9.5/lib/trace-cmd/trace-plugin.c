// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2009, 2010 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 */
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <libgen.h>
#include "trace-cmd.h"
#include "trace-local.h"

#define LOCAL_PLUGIN_DIR ".local/lib/trace-cmd/plugins/"

struct trace_plugin_list {
	struct trace_plugin_list	*next;
	char				*name;
	void				*handle;
};

struct trace_plugin_context {
	enum tracecmd_context context;
	enum tracecmd_plugin_flag flags;
	union {
		void				*data;
		struct tracecmd_input		*trace_input;
		struct tracecmd_output		*trace_output;
	};
};

/**
 * tracecmd_plugin_context_create - Create and initialize tracecmd plugins context.
 * @context: Context of the trace-cmd command.
 * @data: Pointer to the context specific data, which will be passed to plugins.
 *
 * Returns a pointer to created tracecmd plugins context, or NULL in case memory
 * allocation fails. The returned pointer should be freed by free ().
 */
struct trace_plugin_context *
tracecmd_plugin_context_create(enum tracecmd_context context, void *data)
{
	struct trace_plugin_context *trace;

	trace = calloc(1, sizeof(struct trace_plugin_context));
	if (!trace)
		return NULL;
	trace->context = context;
	trace->data = data;
	return trace;
}

/**
 * tracecmd_plugin_set_flag - Set a flag to tracecmd plugins context.
 * @context: Context of the trace-cmd command.
 * @flag: Flag, whil will be set.
 *
 */
void tracecmd_plugin_set_flag(struct trace_plugin_context *context,
			      enum tracecmd_plugin_flag flag)
{
	if (context)
		context->flags |= flag;
}

/**
 * tracecmd_plugin_context_input - Get a tracecmd_input plugin context.
 * @context: Context of the trace-cmd command.
 *
 * Returns pointer to tracecmd_input, if such context is available or
 * NULL otherwise.
 */
struct tracecmd_input *
tracecmd_plugin_context_input(struct trace_plugin_context *context)
{
	if (!context || context->context != TRACECMD_INPUT)
		return NULL;
	return context->trace_input;
}

/**
 * tracecmd_plugin_context_output - Get a tracecmd_output plugin context
 * @context: Context of the trace-cmd command.
 *
 * Returns pointer to tracecmd_output, if such context is available or
 * NULL otherwise.
 */
struct tracecmd_output *
tracecmd_plugin_context_output(struct trace_plugin_context *context)
{
	if (!context || context->context != TRACECMD_OUTPUT)
		return NULL;
	return context->trace_output;
}

static void
load_plugin(struct trace_plugin_context *trace, const char *path,
	    const char *file, void *data)
{
	struct trace_plugin_list **plugin_list = data;
	tracecmd_plugin_load_func func;
	struct trace_plugin_list *list;
	const char *alias;
	char *plugin;
	void *handle;
	int ret;

	ret = asprintf(&plugin, "%s/%s", path, file);
	if (ret < 0) {
		warning("could not allocate plugin memory\n");
		return;
	}

	handle = dlopen(plugin, RTLD_NOW | RTLD_GLOBAL);
	if (!handle) {
		warning("could not load plugin '%s'\n%s\n",
			plugin, dlerror());
		goto out_free;
	}

	alias = dlsym(handle, TRACECMD_PLUGIN_ALIAS_NAME);
	if (!alias)
		alias = file;

	func = dlsym(handle, TRACECMD_PLUGIN_LOADER_NAME);
	if (!func) {
		warning("could not find func '%s' in plugin '%s'\n%s\n",
			TRACECMD_PLUGIN_LOADER_NAME, plugin, dlerror());
		goto out_free;
	}

	list = malloc(sizeof(*list));
	if (!list) {
		warning("could not allocate plugin memory\n");
		goto out_free;
	}

	list->next = *plugin_list;
	list->handle = handle;
	list->name = plugin;
	*plugin_list = list;

	pr_info("registering plugin: %s", plugin);
	func(trace);
	return;

 out_free:
	free(plugin);
}

static void
load_plugins_dir(struct trace_plugin_context *trace, const char *suffix,
		 const char *path,
		 void (*load_plugin)(struct trace_plugin_context *trace,
				     const char *path,
				     const char *name,
				     void *data),
		 void *data)
{
	struct dirent *dent;
	struct stat st;
	DIR *dir;
	int ret;

	ret = stat(path, &st);
	if (ret < 0)
		return;

	if (!S_ISDIR(st.st_mode))
		return;

	dir = opendir(path);
	if (!dir)
		return;

	while ((dent = readdir(dir))) {
		const char *name = dent->d_name;

		if (strcmp(name, ".") == 0 ||
		    strcmp(name, "..") == 0)
			continue;

		/* Only load plugins that end in suffix */
		if (strcmp(name + (strlen(name) - strlen(suffix)), suffix) != 0)
			continue;

		load_plugin(trace, path, name, data);
	}

	closedir(dir);
}

static char *get_source_plugins_dir(void)
{
	char *p, path[PATH_MAX+1];
	int ret;

	ret = readlink("/proc/self/exe", path, PATH_MAX);
	if (ret > PATH_MAX || ret < 0)
		return NULL;

	path[ret] = 0;
	dirname(path);
	p = strrchr(path, '/');
	if (!p)
		return NULL;
	/* Check if we are in the the source tree */
	if (strcmp(p, "/tracecmd") != 0)
		return NULL;

	strcpy(p, "/lib/trace-cmd/plugins");
	return strdup(path);
}

static void
load_plugins_hook(struct trace_plugin_context *trace, const char *suffix,
		  void (*load_plugin)(struct trace_plugin_context *trace,
				      const char *path,
				      const char *name,
				      void *data),
		  void *data)
{
	char *home;
	char *path;
	char *envdir;
	int ret;

	if (trace && trace->flags & TRACECMD_DISABLE_PLUGINS)
		return;

	/*
	 * If a system plugin directory was defined,
	 * check that first.
	 */
#ifdef PLUGIN_TRACECMD_DIR
	if (!trace || !(trace->flags & TRACECMD_DISABLE_SYS_PLUGINS))
		load_plugins_dir(trace, suffix, PLUGIN_TRACECMD_DIR,
				 load_plugin, data);
#endif

	/*
	 * Next let the environment-set plugin directory
	 * override the system defaults.
	 */
	envdir = getenv("TRACECMD_PLUGIN_DIR");
	if (envdir)
		load_plugins_dir(trace, suffix, envdir, load_plugin, data);

	/*
	 * Now let the home directory override the environment
	 * or system defaults.
	 */
	home = getenv("HOME");
	if (!home)
		return;

	ret = asprintf(&path, "%s/%s", home, LOCAL_PLUGIN_DIR);
	if (ret < 0) {
		warning("could not allocate plugin memory\n");
		return;
	}

	load_plugins_dir(trace, suffix, path, load_plugin, data);

	free(path);

	path = get_source_plugins_dir();
	if (path) {
		load_plugins_dir(trace, suffix, path, load_plugin, data);
		free(path);
	}
}

/**
 * tracecmd_load_plugins - Load trace-cmd specific plugins.
 * @context: Context of the trace-cmd command, will be passed to the plugins
 *	     at load time.
 *
 * Returns a list of loaded plugins
 */
struct trace_plugin_list*
tracecmd_load_plugins(struct trace_plugin_context *trace)
{
	struct trace_plugin_list *list = NULL;

	load_plugins_hook(trace, ".so", load_plugin, &list);
	return list;
}

/**
 * tracecmd_unload_plugins - Unload trace-cmd specific plugins.
 * @plugin_list - List of plugins, previously loaded with tracecmd_load_plugins.
 * @context: Context of the trace-cmd command, will be passed to the plugins
 *	     at unload time.
 *
 */
void
tracecmd_unload_plugins(struct trace_plugin_list *plugin_list,
			struct trace_plugin_context *trace)
{
	tracecmd_plugin_unload_func func;
	struct trace_plugin_list *list;

	while (plugin_list) {
		list = plugin_list;
		plugin_list = list->next;
		func = dlsym(list->handle, TRACECMD_PLUGIN_UNLOADER_NAME);
		if (func)
			func(trace);
		dlclose(list->handle);
		free(list->name);
		free(list);
	}
}
