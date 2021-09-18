// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

 /**
  *  @file    libkshark-configio.c
  *  @brief   Json Configuration I/O.
  */

// C
#ifndef _GNU_SOURCE
/** Use GNU C Library. */
#define _GNU_SOURCE

#endif

#include <stdio.h>
#include <sys/stat.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-model.h"

static struct json_object *kshark_json_config_alloc(const char *type)
{
	json_object *jobj, *jtype;

	jobj = json_object_new_object();
	jtype = json_object_new_string(type);

	if (!jobj || !jtype)
		goto fail;

	/* Set the type of this Json document. */
	json_object_object_add(jobj, "type", jtype);

	return jobj;

 fail:
	fprintf(stderr, "Failed to allocate memory for json_object.\n");
	json_object_put(jobj);
	json_object_put(jtype);

	return NULL;
}

/**
 * @brief Allocate kshark_config_doc and set its format.
 *
 * @param format: Input location for the Configuration format identifier.
 *		  Currently only Json and String formats are supported.
 *
 * @returns kshark_config_doc instance on success, otherwise NULL. Use
 *	    free() to free the object.
 */
struct kshark_config_doc *
kshark_config_alloc(enum kshark_config_formats format)
{
	struct kshark_config_doc *doc;

	switch (format) {
	case KS_CONFIG_AUTO:
	case KS_CONFIG_JSON:
	case KS_CONFIG_STRING:
		doc = malloc(sizeof(*doc));
		if (!doc)
			goto fail;

		doc->format = format;
		doc->conf_doc = NULL;
		return doc;
	default:
		fprintf(stderr, "Document format %d not supported\n",
		format);
	}

	return NULL;

 fail:
	fprintf(stderr, "Failed to allocate memory for kshark_config_doc.\n");
	return NULL;
}

/**
 * @brief Create an empty Configuration document and set its format and type.
 *
 * @param type: String describing the type of the document,
 *		e.g. "kshark.config.record" or "kshark.config.filter".
 * @param format: Input location for the Configuration format identifier.
 *		  Currently only Json format is supported.
 *
 * @returns kshark_config_doc instance on success, otherwise NULL. Use
 *	    kshark_free_config_doc() to free the object.
 */
struct kshark_config_doc *
kshark_config_new(const char *type, enum kshark_config_formats format)
{
	struct kshark_config_doc *doc = NULL;

	if (format == KS_CONFIG_AUTO)
		format = KS_CONFIG_JSON;

	switch (format) {
	case KS_CONFIG_JSON:
		doc = kshark_config_alloc(format);
		if (doc) {
			doc->conf_doc = kshark_json_config_alloc(type);
			if (!doc->conf_doc) {
				free(doc);
				doc = NULL;
			}
		}

		break;
	case KS_CONFIG_STRING:
		doc = kshark_config_alloc(format);
		break;
	default:
		fprintf(stderr, "Document format %d not supported\n",
			format);
		return NULL;
	}

	return doc;
}

/**
 * @brief Free the Configuration document.
 *
 * @param conf: Input location for the kshark_config_doc instance. It is safe
 *	        to pass a NULL value.
 */
void kshark_free_config_doc(struct kshark_config_doc *conf)
{
	if (!conf)
		return;

	switch (conf->format) {
	case KS_CONFIG_JSON:
		json_object_put(conf->conf_doc);
		break;
	case KS_CONFIG_STRING:
		free(conf->conf_doc);
		break;
	}

	free(conf);
}

/**
 * @brief Use an existing Json document to create a new KernelShark
 *	  Configuration document.
 *
 * @param jobj: Input location for the json_object instance.
 *
 * @returns shark_config_doc instance on success, otherwise NULL. Use
 *	    kshark_free_config_doc() to free the object.
 */
struct kshark_config_doc *kshark_json_to_conf(struct json_object *jobj)
{
	struct kshark_config_doc *conf = kshark_config_alloc(KS_CONFIG_JSON);

	if (conf)
		conf->conf_doc = jobj;

	return conf;
}

/**
 * @brief Use an existing string to create a new KernelShark Configuration
 * document.
 *
 * @param val: Input location for the string.
 *
 * @returns shark_config_doc instance on success, otherwise NULL. Use
 *	    kshark_free_config_doc() to free the object.
 */
struct kshark_config_doc *kshark_string_to_conf(const char* val)
{
	struct kshark_config_doc *conf;
	char *str;

	conf = kshark_config_alloc(KS_CONFIG_STRING);
	if (conf) {
		if (asprintf(&str, "%s", val) > 0) {
			conf->conf_doc = str;
		} else {
			fprintf(stderr,
				"Failed to allocate string conf. doc.\n");
			free(conf);
			conf = NULL;
		}
	}

	return conf;
}

/**
 * @brief Add a field to a KernelShark Configuration document.
 *
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 * @param key: The name of the field.
 * @param val: Input location for the kshark_config_doc to be added. Currently
 *	       only Json and String formats are supported. Pass KS_CONFIG_AUTO
 *	       if you want "val" to have the same fornat as "conf". Upon
 *	       calling this function, the ownership of "val" transfers to
 *	       "conf".
 *
 * @returns True on success, otherwise False.
 */
bool kshark_config_doc_add(struct kshark_config_doc *conf,
			   const char *key,
			   struct kshark_config_doc *val)
{
	struct json_object *jobj = NULL;

	if (!conf || !val)
		return false;

	if (val->format == KS_CONFIG_AUTO)
		val->format = conf->format;

	switch (conf->format) {
	case KS_CONFIG_JSON:
		switch (val->format) {
		case KS_CONFIG_JSON:
			json_object_object_add(conf->conf_doc, key,
					       val->conf_doc);
			break;

		case KS_CONFIG_STRING:
			jobj = json_object_new_string(val->conf_doc);
			if (!jobj)
				goto fail;

			json_object_object_add(conf->conf_doc, key, jobj);
			break;

		default:
			fprintf(stderr, "Value format %d not supported\n",
				val->format);
			return false;
		}

		free(val);
		break;
	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return false;
	}

	return true;

 fail:
	fprintf(stderr, "Failed to allocate memory for json_object.\n");
	json_object_put(jobj);

	return false;
}

static bool get_jval(struct kshark_config_doc *conf,
		     const char *key, void **val)
{
	return json_object_object_get_ex(conf->conf_doc, key,
					 (json_object **) val);
}

/**
 * @brief Get the KernelShark Configuration document associate with a given
 *	  field name.
 *
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 * @param key: The name of the field.
 * @param val: Output location for the kshark_config_doc instance containing
 *	       the field. Currently only Json and String formats are supported.
 *
 * @returns True, if the key exists, otherwise False.
 */
bool kshark_config_doc_get(struct kshark_config_doc *conf,
			   const char *key,
			   struct kshark_config_doc *val)
{
	struct kshark_config_doc *tmp;

	if (!conf || !val)
		return false;

	if (val->format == KS_CONFIG_AUTO)
		val->format = conf->format;

	switch (conf->format) {
	case KS_CONFIG_JSON:
		switch (val->format) {
		case KS_CONFIG_JSON:
			json_object_put(val->conf_doc);
			if (!get_jval(conf, key, &val->conf_doc))
				goto fail;

			return true;
		case KS_CONFIG_STRING:
			tmp = kshark_config_alloc(KS_CONFIG_AUTO);
			if (!tmp)
				goto fail;

			if (!get_jval(conf, key, &tmp->conf_doc))
				goto fail;

			val->conf_doc =
				(char *) json_object_get_string(tmp->conf_doc);
			free(tmp);

			return true;
		default:
			fprintf(stderr, "Value format %d not supported\n",
				val->format);
			return false;
		}

		break;
	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return false;
	}

	return true;

 fail:
	fprintf(stderr, "Failed to get config. document.\n");
	return false;
}

/**
 * @brief Create an empty Record Configuration document. The type description
 *	  is set to "kshark.config.record".
 *
 * @returns kshark_config_doc instance on success, otherwise NULL. Use
 *	    kshark_free_config_doc() to free the object.
 */
struct kshark_config_doc *
kshark_record_config_new(enum kshark_config_formats format)
{
	return kshark_config_new("kshark.config.record", format);
}

/**
 * @brief Create an empty Filter Configuration document. The type description
 *	  is set to "kshark.config.filter".
 *
 * @returns kshark_config_doc instance on success, otherwise NULL. Use
 *	    kshark_free_config_doc() to free the object.
 */
struct kshark_config_doc *
kshark_filter_config_new(enum kshark_config_formats format)
{
	return kshark_config_new("kshark.config.filter", format);
}

/**
 * @brief Create an empty Text Configuration document. The Text Configuration
 *	  documents do not use type descriptions.
 *
 * @returns kshark_config_doc instance on success, otherwise NULL. Use free()
 *	    to free the object.
 */
struct kshark_config_doc *kshark_string_config_alloc(void)
{
	return kshark_config_alloc(KS_CONFIG_STRING);
}

static void json_del_if_exist(struct json_object *jobj, const char *key)
{
	struct json_object *temp;
	if (json_object_object_get_ex(jobj, key, &temp))
	    json_object_object_del(jobj, key);
}

static bool kshark_json_type_check(struct json_object *jobj, const char *type)
{
	struct json_object *jtype;
	const char *type_str;

	if (!json_object_object_get_ex(jobj, "type", &jtype))
		return false;

	type_str = json_object_get_string(jtype);
	if (strcmp(type_str, type) != 0)
		return false;

	return true;
}

/**
 * @brief Check the type of a Configuration document and compare with an
 *	  expected value.
 *
 * @param conf: Input location for the kshark_config_doc instance.
 * @param type: Input location for the expected value of the Configuration
 *		document type, e.g. "kshark.config.record" or
 *		"kshark.config.filter".
 *
 * @returns True, if the document has the expected type, otherwise False.
 */
bool kshark_type_check(struct kshark_config_doc *conf, const char *type)
{
	switch (conf->format) {
	case KS_CONFIG_JSON:
		return kshark_json_type_check(conf->conf_doc, type);

	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return false;
	}
}

static bool kshark_trace_file_to_json(const char *file,
				      struct json_object *jobj)
{
	struct json_object *jfile_name, *jtime;
	struct stat st;

	if (!file || !jobj)
		return false;

	if (stat(file, &st) != 0) {
		fprintf(stderr, "Unable to find file %s\n", file);
		return false;
	}

	jfile_name = json_object_new_string(file);
	jtime = json_object_new_int64(st.st_mtime);

	if (!jfile_name || !jtime)
		goto fail;

	json_object_object_add(jobj, "file", jfile_name);
	json_object_object_add(jobj, "time", jtime);

	return true;

 fail:
	fprintf(stderr, "Failed to allocate memory for json_object.\n");
	json_object_put(jobj);
	json_object_put(jfile_name);
	json_object_put(jtime);

	return false;
}

/**
 * @brief Record the name of a trace data file and its timestamp into a
 *	  Configuration document.
 *
 * @param file: The name of the file.
 * @param format: Input location for the Configuration format identifier.
 *		  Currently only Json format is supported.
 *
 * @returns True on success, otherwise False.
 */
struct kshark_config_doc *
kshark_export_trace_file(const char *file,
			 enum kshark_config_formats format)
{
	/*  Create a new Configuration document. */
	struct kshark_config_doc *conf =
		kshark_config_new("kshark.config.data", format);

	if (!conf)
		return NULL;

	switch (format) {
	case KS_CONFIG_JSON:
		kshark_trace_file_to_json(file, conf->conf_doc);
		return conf;

	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return NULL;
	}
}

static bool kshark_trace_file_from_json(const char **file,
					struct json_object *jobj)
{
	struct json_object *jfile_name, *jtime;
	const char *file_str;
	struct stat st;
	int64_t time;

	if (!jobj)
		return false;

	if (!kshark_json_type_check(jobj, "kshark.config.data") ||
	    !json_object_object_get_ex(jobj, "file", &jfile_name) ||
	    !json_object_object_get_ex(jobj, "time", &jtime)) {
		fprintf(stderr,
			"Failed to retrieve data file from json_object.\n");
		return false;
	}

	file_str = json_object_get_string(jfile_name);
	time = json_object_get_int64(jtime);

	if (stat(file_str, &st) != 0) {
		fprintf(stderr, "Unable to find file %s\n", file_str);
		return false;
	}

	if (st.st_mtime != time) {
		fprintf(stderr,"Timestamp mismatch!\nFile %s", file_str);
		return false;
	}

	*file = file_str;

	return true;
}

/**
 * @brief Read the name of a trace data file and its timestamp from a
 *	  Configuration document and check if such a file exists.
 *	  If the file exists, open it.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns The name of the file on success, otherwise NULL. "conf" has
 *	    the ownership over the returned string.
 */
const char* kshark_import_trace_file(struct kshark_context *kshark_ctx,
				     struct kshark_config_doc *conf)
{
	const char *file = NULL;
	switch (conf->format) {
	case KS_CONFIG_JSON:
		if (kshark_trace_file_from_json(&file, conf->conf_doc))
			kshark_open(kshark_ctx, file);

		break;

	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return NULL;
	}

	return file;
}

static bool kshark_model_to_json(struct kshark_trace_histo *histo,
				 struct json_object *jobj)
{
	struct json_object *jrange, *jmin, *jmax, *jn_bins;
	if (!histo || !jobj)
		return false;

	jrange = json_object_new_array();

	jmin = json_object_new_int64(histo->min);
	jmax = json_object_new_int64(histo->max);
	jn_bins = json_object_new_int(histo->n_bins);

	if (!jrange || !jmin || !jmax || !jn_bins)
		goto fail;

	json_object_array_put_idx(jrange, 0, jmin);
	json_object_array_put_idx(jrange, 1, jmax);

	json_object_object_add(jobj, "range", jrange);
	json_object_object_add(jobj, "bins", jn_bins);

	return true;

 fail:
	fprintf(stderr, "Failed to allocate memory for json_object.\n");
	json_object_put(jobj);
	json_object_put(jrange);
	json_object_put(jmin);
	json_object_put(jmax);
	json_object_put(jn_bins);

	return false;
}

/**
 * @brief Record the current configuration of the Vis. model into a
 *	  Configuration document.
 * Load the configuration of the Vis. model from a Configuration
 *	  document.
 *
 * @param histo: Input location for the Vis. model descriptor.
 * @param format: Input location for the kshark_config_doc instance. Currently
 *		  only Json format is supported.
 *
 * @returns True on success, otherwise False.
 */
struct kshark_config_doc *
kshark_export_model(struct kshark_trace_histo *histo,
		    enum kshark_config_formats format)
{
	/*  Create a new Configuration document. */
	struct kshark_config_doc *conf =
		kshark_config_new("kshark.config.model", format);

	if (!conf)
		return NULL;

	switch (format) {
	case KS_CONFIG_JSON:
		kshark_model_to_json(histo, conf->conf_doc);
		return conf;

	default:
		fprintf(stderr, "Document format %d not supported\n",
			format);
		return NULL;
	}
}

static bool kshark_model_from_json(struct kshark_trace_histo *histo,
				   struct json_object *jobj)
{
	struct json_object *jrange, *jmin, *jmax, *jn_bins;
	uint64_t min, max;
	int n_bins;

	if (!histo || !jobj)
		return false;

	if (!kshark_json_type_check(jobj, "kshark.config.model") ||
	    !json_object_object_get_ex(jobj, "range", &jrange) ||
	    !json_object_object_get_ex(jobj, "bins", &jn_bins) ||
	    json_object_get_type(jrange) != json_type_array ||
	    json_object_array_length(jrange) != 2)
		goto fail;

	jmin = json_object_array_get_idx(jrange, 0);
	jmax = json_object_array_get_idx(jrange, 1);
	if (!jmin || !jmax)
		goto fail;

	min = json_object_get_int64(jmin);
	max = json_object_get_int64(jmax);
	n_bins = json_object_get_int(jn_bins);
	ksmodel_set_bining(histo, n_bins, min, max);

	if (histo->data && histo->data_size)
		ksmodel_fill(histo, histo->data, histo->data_size);

	return true;

 fail:
	fprintf(stderr, "Failed to load event filter from json_object.\n");
	return false;
}

/**
 * @brief Load the configuration of the Vis. model from a Configuration
 *	  document.
 *
 * @param histo: Input location for the Vis. model descriptor.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns True, if the model has been loaded. If the model configuration
 *	    document contains no data or in a case of an error, the function
 *	    returns False.
 */
bool kshark_import_model(struct kshark_trace_histo *histo,
			 struct kshark_config_doc *conf)
{
	switch (conf->format) {
	case KS_CONFIG_JSON:
		return kshark_model_from_json(histo, conf->conf_doc);

	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return false;
	}
}

static bool kshark_event_filter_to_json(struct tep_handle *pevent,
					struct tracecmd_filter_id *filter,
					const char *filter_name,
					struct json_object *jobj)
{
	json_object *jfilter_data, *jevent, *jsystem, *jname;
	struct tep_event *event;
	int i, evt, *ids, nr_events;
	char *temp;

	jevent = jsystem = jname = NULL;

	/*
	 * If this Json document already contains a description of the filter,
	 * delete this description.
	 */
	json_del_if_exist(jobj, filter_name);

	/* Get the array of Ids to be fitered. */
	ids = tracecmd_filter_ids(filter);
	if (!ids)
		return true;

	/* Create a Json array and fill the Id values into it. */
	jfilter_data = json_object_new_array();
	if (!jfilter_data)
		goto fail;

	nr_events = tep_get_events_count(pevent);
	for (i = 0; i < filter->count; ++i) {
		for (evt = 0; evt < nr_events; ++evt) {
			event = tep_get_event(pevent, evt);
			if (event->id == ids[i]) {
				jevent = json_object_new_object();

				temp = event->system;
				jsystem = json_object_new_string(temp);

				temp = event->name;
				jname = json_object_new_string(temp);

				if (!jevent || !jsystem || !jname)
					goto fail;

				json_object_object_add(jevent, "system",
							       jsystem);

				json_object_object_add(jevent, "name",
							       jname);

				json_object_array_add(jfilter_data, jevent);

				break;
			}
		}
	}

	free(ids);

	/* Add the array of Ids to the filter config document. */
	json_object_object_add(jobj, filter_name, jfilter_data);

	return true;

 fail:
	fprintf(stderr, "Failed to allocate memory for json_object.\n");
	json_object_put(jfilter_data);
	json_object_put(jevent);
	json_object_put(jsystem);
	json_object_put(jname);
	free(ids);

	return false;
}

/**
 * @brief Record the current configuration of an Event Id filter into a
 *	  Configuration document.
 *
 * @param pevent: Input location for the Page event.
 * @param filter: Input location for an Id filter.
 * @param filter_name: The name of the filter to show up in the Json document.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns True on success, otherwise False.
 */
bool kshark_export_event_filter(struct tep_handle *pevent,
				struct tracecmd_filter_id *filter,
				const char *filter_name,
				struct kshark_config_doc *conf)
{
	switch (conf->format) {
	case KS_CONFIG_JSON:
		return kshark_event_filter_to_json(pevent, filter,
						   filter_name,
						   conf->conf_doc);

	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return false;
	}
}

static int kshark_event_filter_from_json(struct tep_handle *pevent,
					  struct tracecmd_filter_id *filter,
					  const char *filter_name,
					  struct json_object *jobj)
{
	json_object *jfilter, *jevent, *jsystem, *jname;
	const char *system_str, *name_str;
	struct tep_event *event;
	int i, length, count = 0;

	/*
	 * Use the name of the filter to find the array of events associated
	 * with this filter. Notice that the filter config document may
	 * contain no data for this particular filter.
	 */
	if (!json_object_object_get_ex(jobj, filter_name, &jfilter))
		return 0;

	if (!kshark_json_type_check(jobj, "kshark.config.filter") ||
	    json_object_get_type(jfilter) != json_type_array)
		goto fail;

	/* Set the filter. */
	length = json_object_array_length(jfilter);
	for (i = 0; i < length; ++i) {
		jevent = json_object_array_get_idx(jfilter, i);

		if (!json_object_object_get_ex(jevent, "system", &jsystem) ||
		    !json_object_object_get_ex(jevent, "name", &jname))
			goto fail;

		system_str = json_object_get_string(jsystem);
		name_str = json_object_get_string(jname);

		event = tep_find_event_by_name(pevent, system_str, name_str);
		if (!event)
			continue;

		tracecmd_filter_id_add(filter, event->id);
		++count;
	}

	if (count != length)
		count = -count;

	return count;

 fail:
	fprintf(stderr, "Failed to load event filter from json_object.\n");
	tracecmd_filter_id_clear(filter);
	return 0;
}

/**
 * @brief Load from Configuration document the configuration of an Event Id filter.
 *
 * @param pevent: Input location for the Page event.
 * @param filter: Input location for an Id filter.
 * @param filter_name: The name of the filter as showing up in the Config.
 *	               document.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns The total number of events added to the filter. If not all events
 *	    listed in the input configuration have been added successfully,
 *	    the returned number is negative.
 */
int kshark_import_event_filter(struct tep_handle *pevent,
			       struct tracecmd_filter_id *filter,
			       const char *filter_name,
			       struct kshark_config_doc *conf)
{
	switch (conf->format) {
	case KS_CONFIG_JSON:
		return kshark_event_filter_from_json(pevent, filter,
						     filter_name,
						     conf->conf_doc);

	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return 0;
	}
}

static bool kshark_filter_array_to_json(struct tracecmd_filter_id *filter,
					const char *filter_name,
					struct json_object *jobj)
{
	json_object *jfilter_data, *jpid = NULL;
	int i, *ids;

	/*
	 * If this Json document already contains a description of the filter,
	 * delete this description.
	 */
	json_del_if_exist(jobj, filter_name);

	/* Get the array of Ids to be filtered. */
	ids = tracecmd_filter_ids(filter);
	if (!ids)
		return true;

	/* Create a Json array and fill the Id values into it. */
	jfilter_data = json_object_new_array();
	if (!jfilter_data)
		goto fail;

	for (i = 0; i < filter->count; ++i) {
		jpid = json_object_new_int(ids[i]);
		if (!jpid)
			goto fail;

		json_object_array_add(jfilter_data, jpid);
	}

	free(ids);

	/* Add the array of Ids to the filter config document. */
	json_object_object_add(jobj, filter_name, jfilter_data);

	return true;

 fail:
	fprintf(stderr, "Failed to allocate memory for json_object.\n");
	json_object_put(jfilter_data);
	json_object_put(jpid);
	free(ids);

	return false;
}

/**
 * @brief Record the current configuration of a simple Id filter into a
 *	  Configuration document.
 *
 * @param filter: Input location for an Id filter.
 * @param filter_name: The name of the filter to show up in the Json document.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns True on success, otherwise False.
 */
bool kshark_export_filter_array(struct tracecmd_filter_id *filter,
				const char *filter_name,
				struct kshark_config_doc *conf)
{
	switch (conf->format) {
	case KS_CONFIG_JSON:
		return kshark_filter_array_to_json(filter, filter_name,
						   conf->conf_doc);

	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return false;
	}
}

static bool kshark_filter_array_from_json(struct tracecmd_filter_id *filter,
					  const char *filter_name,
					  struct json_object *jobj)
{
	json_object *jfilter, *jpid;
	int i, length;

	/*
	 * Use the name of the filter to find the array of events associated
	 * with this filter. Notice that the filter config document may
	 * contain no data for this particular filter.
	 */
	if (!json_object_object_get_ex(jobj, filter_name, &jfilter))
		return false;

	if (!kshark_json_type_check(jobj, "kshark.config.filter") ||
	    json_object_get_type(jfilter) != json_type_array)
		goto fail;

	/* Set the filter. */
	length = json_object_array_length(jfilter);
	for (i = 0; i < length; ++i) {
		jpid = json_object_array_get_idx(jfilter, i);
		if (!jpid)
			goto fail;

		tracecmd_filter_id_add(filter, json_object_get_int(jpid));
	}

	return true;

 fail:
	fprintf(stderr, "Failed to load task filter from json_object.\n");
	return false;
}

/**
 * @brief Load from Configuration document the configuration of a simple
 *	  Id filter.
 *
 * @param filter: Input location for an Id filter.
 * @param filter_name: The name of the filter as showing up in the Config.
 *	               document.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns True, if a filter has been loaded. If the filter configuration
 *	    document contains no data for this particular filter or in a case
 *	    of an error, the function returns False.
 */
bool kshark_import_filter_array(struct tracecmd_filter_id *filter,
				const char *filter_name,
				struct kshark_config_doc *conf)
{
	switch (conf->format) {
	case KS_CONFIG_JSON:
		return kshark_filter_array_from_json(filter, filter_name,
						     conf->conf_doc);

	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return false;
	}
}

static bool kshark_adv_filters_to_json(struct kshark_context *kshark_ctx,
				       struct json_object *jobj)
{
	struct tep_event_filter *adv_filter = kshark_ctx->advanced_event_filter;
	json_object *jfilter_data, *jevent, *jsystem, *jname, *jfilter;
	struct tep_event **events;
	char *str;
	int i;

	jevent = jsystem = jname = jfilter = NULL;

	/*
	 * If this Json document already contains a description of the model,
	 * delete this description.
	 */
	json_del_if_exist(jobj, KS_ADV_EVENT_FILTER_NAME);

	if (!kshark_ctx->advanced_event_filter ||
	    !kshark_ctx->advanced_event_filter->filters)
		return true;

	/* Create a Json array and fill the Id values into it. */
	jfilter_data = json_object_new_array();
	if (!jfilter_data)
		goto fail;

	events = tep_list_events(kshark_ctx->pevent, TEP_EVENT_SORT_SYSTEM);
	if (!events)
		return false;

	for (i = 0; events[i]; i++) {
		str = tep_filter_make_string(adv_filter,
					     events[i]->id);
		if (!str)
			continue;

		jevent = json_object_new_object();
		jsystem = json_object_new_string(events[i]->system);
		jname = json_object_new_string(events[i]->name);
		jfilter = json_object_new_string(str);
		if (!jevent || !jsystem || !jname || !jfilter)
			goto fail;

		json_object_object_add(jevent, "system", jsystem);
		json_object_object_add(jevent, "name", jname);
		json_object_object_add(jevent, "condition", jfilter);

		json_object_array_add(jfilter_data, jevent);
	}

	/* Add the array of advanced filters to the filter config document. */
	json_object_object_add(jobj, KS_ADV_EVENT_FILTER_NAME, jfilter_data);

	return true;

 fail:
	fprintf(stderr, "Failed to allocate memory for json_object.\n");
	json_object_put(jfilter_data);
	json_object_put(jevent);
	json_object_put(jsystem);
	json_object_put(jname);
	json_object_put(jfilter);

	return false;
}

/**
 * @brief Record the current configuration of the advanced filter into a
 *	  Configuration document.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported. If NULL, a new Adv. Filter
 *		Configuration document will be created.
 *
 * @returns True on success, otherwise False.
 */
bool kshark_export_adv_filters(struct kshark_context *kshark_ctx,
			       struct kshark_config_doc **conf)
{
	if (!*conf)
		*conf = kshark_filter_config_new(KS_CONFIG_JSON);

	if (!*conf)
		return false;

	switch ((*conf)->format) {
	case KS_CONFIG_JSON:
		return kshark_adv_filters_to_json(kshark_ctx,
						  (*conf)->conf_doc);

	default:
		fprintf(stderr, "Document format %d not supported\n",
			(*conf)->format);
		return false;
	}
}

static bool kshark_adv_filters_from_json(struct kshark_context *kshark_ctx,
					 struct json_object *jobj)
{
	struct tep_event_filter *adv_filter = kshark_ctx->advanced_event_filter;
	json_object *jfilter, *jsystem, *jname, *jcond;
	int i, length, n, ret = 0;
	char *filter_str = NULL;

	/*
	 * Use the name of the filter to find the array of events associated
	 * with this filter. Notice that the filter config document may
	 * contain no data for this particular filter.
	 */
	if (!json_object_object_get_ex(jobj, KS_ADV_EVENT_FILTER_NAME,
				       &jfilter))
		return false;

	if (!kshark_json_type_check(jobj, "kshark.config.filter") ||
	    json_object_get_type(jfilter) != json_type_array)
		goto fail;

	/* Set the filter. */
	length = json_object_array_length(jfilter);
	for (i = 0; i < length; ++i) {
		jfilter = json_object_array_get_idx(jfilter, i);

		if (!json_object_object_get_ex(jfilter, "system", &jsystem) ||
		    !json_object_object_get_ex(jfilter, "name", &jname) ||
		    !json_object_object_get_ex(jfilter, "condition", &jcond))
			goto fail;

		n = asprintf(&filter_str, "%s/%s:%s",
			     json_object_get_string(jsystem),
			     json_object_get_string(jname),
			     json_object_get_string(jcond));

		if (n <= 0) {
			filter_str = NULL;
			goto fail;
		}

		ret = tep_filter_add_filter_str(adv_filter,
						filter_str);
		if (ret < 0)
			goto fail;
	}

	return true;

 fail:
	fprintf(stderr, "Failed to laod Advanced filters.\n");
	if (ret < 0) {
		char error_str[200];
		int error_status =
			tep_strerror(kshark_ctx->pevent, ret, error_str,
				     sizeof(error_str));

		if (error_status == 0)
			fprintf(stderr, "filter failed due to: %s\n",
					error_str);
	}

	free(filter_str);
	return false;
}

/**
 * @brief Load from Configuration document the configuration of the advanced
 *	  filter.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns True, if a filter has been loaded. If the filter configuration
 *	    document contains no data for the Adv. filter or in a case of
 *	    an error, the function returns False.
 */
bool kshark_import_adv_filters(struct kshark_context *kshark_ctx,
			       struct kshark_config_doc *conf)
{
	switch (conf->format) {
	case KS_CONFIG_JSON:
		return kshark_adv_filters_from_json(kshark_ctx,
						    conf->conf_doc);

	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return false;
	}
}

static bool kshark_user_mask_to_json(struct kshark_context *kshark_ctx,
				     struct json_object *jobj)
{
	uint8_t mask = kshark_ctx->filter_mask;
	json_object *jmask;

	jmask = json_object_new_int((int) mask);
	if (!jmask)
		return false;

	/* Add the mask to the filter config document. */
	json_object_object_add(jobj, KS_USER_FILTER_MASK_NAME, jmask);
	return true;
}

/**
 * @brief Record the current value of the the user-specified filter mask into
 *	  a Configuration document.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported. If NULL, a new Adv. Filter
 *		Configuration document will be created.
 *
 * @returns True on success, otherwise False.
 */
bool kshark_export_user_mask(struct kshark_context *kshark_ctx,
			     struct kshark_config_doc **conf)
{
	if (!*conf)
		*conf = kshark_filter_config_new(KS_CONFIG_JSON);

	if (!*conf)
		return false;

	switch ((*conf)->format) {
	case KS_CONFIG_JSON:
		return kshark_user_mask_to_json(kshark_ctx,
						(*conf)->conf_doc);

	default:
		fprintf(stderr, "Document format %d not supported\n",
			(*conf)->format);
		return false;
	}
}

static bool kshark_user_mask_from_json(struct kshark_context *kshark_ctx,
				       struct json_object *jobj)
{
	json_object *jmask;
	uint8_t mask;

	if (!kshark_json_type_check(jobj, "kshark.config.filter"))
		return false;
	/*
	 * Use the name of the filter to find the value of the filter maks.
	 * Notice that the filter config document may contain no data for
	 * the mask.
	 */
	if (!json_object_object_get_ex(jobj, KS_USER_FILTER_MASK_NAME,
				       &jmask))
		return false;

	mask = json_object_get_int(jmask);
	kshark_ctx->filter_mask = mask;

	return true;
}

/**
 * @brief Load from Configuration document the value of the user-specified
 *	  filter mask.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns True, if a mask has been loaded. If the filter configuration
 *	    document contains no data for the mask or in a case of an error,
 *	    the function returns False.
 */
bool kshark_import_user_mask(struct kshark_context *kshark_ctx,
			     struct kshark_config_doc *conf)
{
	switch (conf->format) {
	case KS_CONFIG_JSON:
		return kshark_user_mask_from_json(kshark_ctx,
						  conf->conf_doc);

	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return false;
	}
}

/**
 * @brief Record the current configuration of "show task" and "hide task"
 *	  filters into a Json document.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported. If NULL, a new Filter
 *		Configuration document will be created.
 *
 * @returns True, if a filter has been recorded. If both filters contain
 *	    no Id values or in a case of an error, the function returns False.
 */
bool kshark_export_all_event_filters(struct kshark_context *kshark_ctx,
				     struct kshark_config_doc **conf)
{
	bool ret = true;

	if (!*conf)
		*conf = kshark_filter_config_new(KS_CONFIG_JSON);

	if (!*conf)
		return false;

	/* Save a filter only if it contains Id values. */
	if (kshark_this_filter_is_set(kshark_ctx->show_event_filter))
		ret &= kshark_export_event_filter(kshark_ctx->pevent,
						  kshark_ctx->show_event_filter,
						  KS_SHOW_EVENT_FILTER_NAME,
						  *conf);

	if (kshark_this_filter_is_set(kshark_ctx->hide_event_filter))
		ret &= kshark_export_event_filter(kshark_ctx->pevent,
						  kshark_ctx->hide_event_filter,
						  KS_HIDE_EVENT_FILTER_NAME,
						  *conf);

	return ret;
}

/**
 * @brief Record the current configuration of "show task" and "hide task"
 *	  filters into a Configuration document.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported. If NULL, a new Filter
 *		Configuration document will be created.
 *
 * @returns True, if a filter has been recorded. If both filters contain
 *	    no Id values or in a case of an error, the function returns False.
 */
bool kshark_export_all_task_filters(struct kshark_context *kshark_ctx,
				    struct kshark_config_doc **conf)
{
	bool ret = true;

	if (!*conf)
		*conf = kshark_filter_config_new(KS_CONFIG_JSON);

	if (!*conf)
		return false;

	/* Save a filter only if it contains Id values. */
	if (kshark_this_filter_is_set(kshark_ctx->show_task_filter))
		ret &= kshark_export_filter_array(kshark_ctx->show_task_filter,
						  KS_SHOW_TASK_FILTER_NAME,
						  *conf);

	if (kshark_this_filter_is_set(kshark_ctx->hide_task_filter))
		ret &= kshark_export_filter_array(kshark_ctx->hide_task_filter,
						  KS_HIDE_TASK_FILTER_NAME,
						  *conf);

	return ret;
}


/**
 * @brief Record the current configuration of "show cpu" and "hide cpu"
 *	  filters into a Configuration document.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported. If NULL, a new Filter
 *		Configuration document will be created.
 *
 * @returns True, if a filter has been recorded. If both filters contain
 *	    no Id values or in a case of an error, the function returns False.
 */
bool kshark_export_all_cpu_filters(struct kshark_context *kshark_ctx,
				   struct kshark_config_doc **conf)
{
	bool ret = true;

	if (!*conf)
		*conf = kshark_filter_config_new(KS_CONFIG_JSON);

	if (!*conf)
		return false;

	/* Save a filter only if it contains Id values. */
	if (kshark_this_filter_is_set(kshark_ctx->show_cpu_filter))
		ret &= kshark_export_filter_array(kshark_ctx->show_cpu_filter,
						  KS_SHOW_CPU_FILTER_NAME,
						  *conf);

	if (kshark_this_filter_is_set(kshark_ctx->hide_cpu_filter))
		ret &= kshark_export_filter_array(kshark_ctx->hide_cpu_filter,
						  KS_HIDE_CPU_FILTER_NAME,
						  *conf);

	return ret;
}

/**
 * @brief Load from a Configuration document the configuration of "show event"
 *	  and "hide event" filters.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns True, if a filter has been loaded. If the filter configuration
 *	    document contains no data for any event filter or in a case
 *	    of an error, the function returns False.
 */
bool kshark_import_all_event_filters(struct kshark_context *kshark_ctx,
				     struct kshark_config_doc *conf)
{
	bool ret = false;

	ret |= kshark_import_event_filter(kshark_ctx->pevent,
					  kshark_ctx->hide_event_filter,
					  KS_HIDE_EVENT_FILTER_NAME,
					  conf);

	ret |= kshark_import_event_filter(kshark_ctx->pevent,
					  kshark_ctx->show_event_filter,
					  KS_SHOW_EVENT_FILTER_NAME,
					  conf);

	return ret;
}

/**
 * @brief Load from Configuration document the configuration of "show task"
 *	  and "hide task" filters.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns True, if a filter has been loaded. If the filter configuration
 *	    document contains no data for any task filter or in a case of an
 *	    error, the function returns False.
 */
bool kshark_import_all_task_filters(struct kshark_context *kshark_ctx,
				    struct kshark_config_doc *conf)
{
	bool ret = false;

	ret |= kshark_import_filter_array(kshark_ctx->hide_task_filter,
					  KS_HIDE_TASK_FILTER_NAME,
					  conf);

	ret |= kshark_import_filter_array(kshark_ctx->show_task_filter,
					  KS_SHOW_TASK_FILTER_NAME,
					  conf);

	return ret;
}

/**
 * @brief Load from Configuration document the configuration of "show cpu"
 *	  and "hide cpu" filters.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns True, if a filter has been loaded. If the filter configuration
 *	    document contains no data for any cpu filter or in a case of an
 *	    error, the function returns False.
 */
bool kshark_import_all_cpu_filters(struct kshark_context *kshark_ctx,
				    struct kshark_config_doc *conf)
{
	bool ret = false;

	ret |= kshark_import_filter_array(kshark_ctx->hide_cpu_filter,
					  KS_HIDE_CPU_FILTER_NAME,
					  conf);

	ret |= kshark_import_filter_array(kshark_ctx->show_cpu_filter,
					  KS_SHOW_CPU_FILTER_NAME,
					  conf);

	return ret;
}

/**
 * @brief Create a Filter Configuration document containing the current
 *	  configuration of all filters.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param format: Input location for the kshark_config_doc instance. Currently
 *		  only Json format is supported.
 *
 * @returns kshark_config_doc instance on success, otherwise NULL. Use
 *	    kshark_free_config_doc() to free the object.
 */
struct kshark_config_doc *
kshark_export_all_filters(struct kshark_context *kshark_ctx,
			  enum kshark_config_formats format)
{
	/*  Create a new Configuration document. */
	struct kshark_config_doc *conf =
		kshark_filter_config_new(format);

	/* Save a filter only if it contains Id values. */
	if (!conf ||
	    !kshark_export_all_event_filters(kshark_ctx, &conf) ||
	    !kshark_export_all_task_filters(kshark_ctx, &conf) ||
	    !kshark_export_all_cpu_filters(kshark_ctx, &conf) ||
	    !kshark_export_user_mask(kshark_ctx, &conf) ||
	    !kshark_export_adv_filters(kshark_ctx, &conf)) {
		kshark_free_config_doc(conf);
		return NULL;
	}

	return conf;
}

/**
 * @brief Load from a Configuration document the configuration of all filters.
 *
 * @param kshark_ctx: Input location for session context pointer.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns True, if a filter has been loaded. If the filter configuration
 *	    document contains no data for any filter or in a case of an error,
 *	    the function returns False.
 */
bool kshark_import_all_filters(struct kshark_context *kshark_ctx,
			       struct kshark_config_doc *conf)
{
	bool ret;
	ret = kshark_import_all_task_filters(kshark_ctx, conf);
	ret |= kshark_import_all_cpu_filters(kshark_ctx, conf);
	ret |= kshark_import_all_event_filters(kshark_ctx, conf);
	ret |= kshark_import_user_mask(kshark_ctx, conf);
	ret |= kshark_import_adv_filters(kshark_ctx, conf);

	return ret;
}

static bool kshark_save_json_file(const char *file_name,
				  struct json_object *jobj)
{
	int flags;

	/* Save the file in a human-readable form. */
	flags = JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY;
	if (json_object_to_file_ext(file_name, jobj, flags) == 0)
		return true;

	return false;
}

/**
 * @brief Save a Configuration document into a file.
 *
 * @param file_name: The name of the file.
 * @param conf: Input location for the kshark_config_doc instance. Currently
 *		only Json format is supported.
 *
 * @returns True on success, otherwise False.
 */
bool kshark_save_config_file(const char *file_name,
			     struct kshark_config_doc *conf)
{
	switch (conf->format) {
	case KS_CONFIG_JSON:
		return kshark_save_json_file(file_name, conf->conf_doc);

	default:
		fprintf(stderr, "Document format %d not supported\n",
			conf->format);
		return false;
	}
}

static struct json_object *kshark_open_json_file(const char *file_name,
						 const char *type)
{
	struct json_object *jobj, *var;
	const char *type_var;

	jobj = json_object_from_file(file_name);

	if (!jobj)
		return NULL;

	/* Get the type of the document. */
	if (!json_object_object_get_ex(jobj, "type", &var))
		goto fail;

	type_var = json_object_get_string(var);

	if (strcmp(type, type_var) != 0)
		goto fail;

	return jobj;

 fail:
	/* The document has a wrong type. */
	fprintf(stderr, "Failed to open Json file %s.\n", file_name);
	fprintf(stderr, "The document has a wrong type.\n");

	json_object_put(jobj);
	return NULL;
}

static const char *get_ext(const char *filename)
{
	const char *dot = strrchr(filename, '.');

	if(!dot)
		return "unknown";

	return dot + 1;
}

/**
 * @brief Open for read a Configuration file and check if it has the
 *	  expected type.
 *
 * @param file_name: The name of the file. Currently only Json files are
 *		     supported.
 * @param type: String describing the expected type of the document,
 *		e.g. "kshark.config.record" or "kshark.config.filter".
 *
 * @returns kshark_config_doc instance on success, or NULL on failure. Use
 *	    kshark_free_config_doc() to free the object.
 */
struct kshark_config_doc *kshark_open_config_file(const char *file_name,
						  const char *type)
{
	struct kshark_config_doc *conf = NULL;

	if (strcmp(get_ext(file_name), "json") == 0) {
		struct json_object *jobj =
			kshark_open_json_file(file_name, type);

		if (jobj) {
			conf = malloc(sizeof(*conf));
			conf->conf_doc = jobj;
			conf->format = KS_CONFIG_JSON;
		}
	}

	return conf;
}
