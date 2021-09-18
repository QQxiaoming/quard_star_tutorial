// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsSession.cpp
 *  @brief   KernelShark Session.
 */

// KernelShark
#include "libkshark.h"
#include "KsSession.hpp"
#include "KsMainWindow.hpp"

/** Create a KsSession object. */
KsSession::KsSession()
{
	_config = kshark_config_new("kshark.config.session",
				      KS_CONFIG_JSON);
}

/** Destroy a KsSession object. */
KsSession::~KsSession()
{
	kshark_free_config_doc(_config);
}

/** Import a user session from a Json file. */
bool KsSession::importFromFile(QString jfileName)
{
	kshark_config_doc *configTmp =
		kshark_open_config_file(jfileName.toStdString().c_str(),
					"kshark.config.session");

	if (configTmp) {
		kshark_free_config_doc(_config);
		_config = configTmp;
		return true;
	}

	return false;
}

/** Export the current user session from a Json file. */
void KsSession::exportToFile(QString jfileName)
{
	kshark_save_config_file(jfileName.toStdString().c_str(), _config);
}

/**
 * @brief Save the state of the visualization model.
 *
 * @param histo: Input location for the model descriptor.
 */
void KsSession::saveVisModel(kshark_trace_histo *histo)
{
	kshark_config_doc *model =
		kshark_export_model(histo, KS_CONFIG_JSON);

	kshark_config_doc_add(_config, "Model", model);
}

/**
 * @brief Load the state of the visualization model.
 *
 * @param model: Input location for the KsGraphModel object.
 */
void KsSession::loadVisModel(KsGraphModel *model)
{
	kshark_config_doc *modelConf = kshark_config_alloc(KS_CONFIG_JSON);

	if (!kshark_config_doc_get(_config, "Model", modelConf))
		return;

	kshark_import_model(model->histo(), modelConf);
	model->update();
}

/** Save the trace data file. */
void KsSession::saveDataFile(QString fileName)
{
	kshark_config_doc *file =
		kshark_export_trace_file(fileName.toStdString().c_str(),
					 KS_CONFIG_JSON);

	kshark_config_doc_add(_config, "Data", file);
}

/** Get the trace data file. */
QString KsSession::getDataFile(kshark_context *kshark_ctx)
{
	kshark_config_doc *file = kshark_config_alloc(KS_CONFIG_JSON);
	const char *file_str;

	if (!kshark_config_doc_get(_config, "Data", file))
		return QString();

	file_str = kshark_import_trace_file(kshark_ctx, file);
	if (file_str)
		return QString(file_str);

	return QString();
}

/**
 * @brief Save the configuration of the filters.
 *
 * @param kshark_ctx: Input location for context pointer.
 */
void KsSession::saveFilters(kshark_context *kshark_ctx)
{
	kshark_config_doc *filters =
		kshark_export_all_filters(kshark_ctx, KS_CONFIG_JSON);

	kshark_config_doc_add(_config, "Filters", filters);
}

/**
 * @brief Load the configuration of the filters and filter the data.
 *
 * @param kshark_ctx: Input location for context pointer.
 * @param data: Input location for KsDataStore object;
 */
void KsSession::loadFilters(kshark_context *kshark_ctx, KsDataStore *data)
{
	kshark_config_doc *filters = kshark_config_alloc(KS_CONFIG_JSON);

	if (!kshark_config_doc_get(_config, "Filters", filters))
		return;

	kshark_import_all_filters(kshark_ctx, filters);

	if (kshark_ctx->advanced_event_filter->filters)
		data->reload();
	else
		kshark_filter_entries(kshark_ctx, data->rows(), data->size());

	data->registerCPUCollections();

	emit data->updateWidgets(data);
}

/**
 * @brief Save the state of the table.
 *
 * @param view: Input location for the KsTraceViewer widget.
 */
void KsSession::saveTable(const KsTraceViewer &view) {
	kshark_config_doc *topRow = kshark_config_alloc(KS_CONFIG_JSON);
	int64_t r = view.getTopRow();

	topRow->conf_doc = json_object_new_int64(r);
	kshark_config_doc_add(_config, "ViewTop",topRow);
}

/**
 * @brief Load the state of the table.
 *
 * @param view: Input location for the KsTraceViewer widget.
 */
void KsSession::loadTable(KsTraceViewer *view) {
	kshark_config_doc *topRow = kshark_config_alloc(KS_CONFIG_JSON);
	size_t r = 0;

	if (!kshark_config_doc_get(_config, "ViewTop", topRow))
		return;

	if (_config->format == KS_CONFIG_JSON)
		r = json_object_get_int64(KS_JSON_CAST(topRow->conf_doc));

	view->setTopRow(r);
}

/**
 * @brief Save the KernelShark Main window size.
 *
 * @param window: Input location for the KsMainWindow widget.
 */
void KsSession::saveMainWindowSize(const QMainWindow &window)
{
	kshark_config_doc *windowConf = kshark_config_alloc(KS_CONFIG_JSON);
	int width = window.width(), height = window.height();
	json_object *jwindow;

	if (window.isFullScreen()) {
		jwindow = json_object_new_string("FullScreen");
	} else {
		jwindow = json_object_new_array();
		json_object_array_put_idx(jwindow, 0, json_object_new_int(width));
		json_object_array_put_idx(jwindow, 1, json_object_new_int(height));
	}

	windowConf->conf_doc = jwindow;
	kshark_config_doc_add(_config, "MainWindow", windowConf);
}

/**
 * @brief Load the KernelShark Main window size.
 *
 * @param window: Input location for the KsMainWindow widget.
 */
void KsSession::loadMainWindowSize(KsMainWindow *window)
{
	kshark_config_doc *windowConf = kshark_config_alloc(KS_CONFIG_JSON);
	json_object *jwindow, *jwidth, *jheight;
	int width, height;

	if (!kshark_config_doc_get(_config, "MainWindow", windowConf))
		return;

	if (_config->format == KS_CONFIG_JSON) {
		jwindow = KS_JSON_CAST(windowConf->conf_doc);
		if (json_object_get_type(jwindow) == json_type_string &&
		    QString(json_object_get_string(jwindow)) == "FullScreen") {
			window->setFullScreenMode(true);
			return;
		}

		jwidth = json_object_array_get_idx(jwindow, 0);
		jheight = json_object_array_get_idx(jwindow, 1);

		width = json_object_get_int(jwidth);
		height = json_object_get_int(jheight);

		window->setFullScreenMode(false);
		window->resize(width, height);
	}
}

/**
 * @brief Save the state of the Main window spliter.
 *
 * @param splitter: Input location for the splitter widget.
 */
void KsSession::saveSplitterSize(const QSplitter &splitter)
{
	kshark_config_doc *spl = kshark_config_alloc(KS_CONFIG_JSON);
	json_object *jspl = json_object_new_array();
	QList<int> sizes = splitter.sizes();

	json_object_array_put_idx(jspl, 0, json_object_new_int(sizes[0]));
	json_object_array_put_idx(jspl, 1, json_object_new_int(sizes[1]));

	spl->conf_doc = jspl;
	kshark_config_doc_add(_config, "Splitter", spl);
}

/**
 * @brief Load the state of the Main window spliter.
 *
 * @param splitter: Input location for the splitter widget.
 */
void KsSession::loadSplitterSize(QSplitter *splitter)
{
	kshark_config_doc *spl = kshark_config_alloc(KS_CONFIG_JSON);
	json_object *jspl, *jgraphsize, *jviewsize;
	int graphSize(1), viewSize(1);
	QList<int> sizes;

	if (!kshark_config_doc_get(_config, "Splitter", spl))
		return;

	if (_config->format == KS_CONFIG_JSON) {
		jspl = KS_JSON_CAST(spl->conf_doc);
		jgraphsize = json_object_array_get_idx(jspl, 0);
		jviewsize = json_object_array_get_idx(jspl, 1);

		graphSize = json_object_get_int(jgraphsize);
		viewSize = json_object_get_int(jviewsize);
		if (graphSize == 0 && viewSize == 0) {
			/* 0/0 spliter ratio is undefined. Make it 1/1. */
			viewSize = graphSize = 1;
		}
	}

	sizes << graphSize << viewSize;
	splitter->setSizes(sizes);
}

/** @brief Save the Color scheme used. */
void KsSession::saveColorScheme() {
	kshark_config_doc *colSch = kshark_config_alloc(KS_CONFIG_JSON);
	double s = KsPlot::Color::getRainbowFrequency();

	colSch->conf_doc = json_object_new_double(s);
	kshark_config_doc_add(_config, "ColorScheme", colSch);
}

/** @brief Get the Color scheme used. */
float KsSession::getColorScheme() {
	kshark_config_doc *colSch = kshark_config_alloc(KS_CONFIG_JSON);

	/* Default color scheme. */
	float s = 0.75;

	if (!kshark_config_doc_get(_config, "ColorScheme", colSch))
		return s;

	if (_config->format == KS_CONFIG_JSON)
		s = json_object_get_double(KS_JSON_CAST(colSch->conf_doc));

	return s;
}

/**
 * @brief Save the list of the graphs plotted.
 *
 * @param glw: Input location for the KsGLWidget widget.
 */
void KsSession::saveGraphs(const KsGLWidget &glw)
{
	_saveCPUPlots(glw._cpuList);
	_saveTaskPlots(glw._taskList);
}

/**
 * @brief Load the list of the graphs and plot.
 *
 * @param graphs: Input location for the KsTraceGraph widget.
 */
void KsSession::loadGraphs(KsTraceGraph *graphs)
{
	graphs->cpuReDraw(_getCPUPlots());
	graphs->taskReDraw(_getTaskPlots());
}

void KsSession::_saveCPUPlots(const QVector<int> &cpus)
{
	kshark_config_doc *cpuPlts = kshark_config_alloc(KS_CONFIG_JSON);
	json_object *jcpus = json_object_new_array();

	for (int i = 0; i < cpus.count(); ++i) {
		json_object *jcpu = json_object_new_int(cpus[i]);
		json_object_array_put_idx(jcpus, i, jcpu);
	}

	cpuPlts->conf_doc = jcpus;
	kshark_config_doc_add(_config, "CPUPlots", cpuPlts);
}

QVector<int> KsSession::_getCPUPlots()
{
	kshark_config_doc *cpuPlts = kshark_config_alloc(KS_CONFIG_JSON);
	json_object *jcpus;
	QVector<int> cpus;
	size_t length;

	if (!kshark_config_doc_get(_config, "CPUPlots", cpuPlts))
		return cpus;

	if (_config->format == KS_CONFIG_JSON) {
		jcpus = KS_JSON_CAST(cpuPlts->conf_doc);
		length = json_object_array_length(jcpus);
		for (size_t i = 0; i < length; ++i) {
			int cpu = json_object_get_int(json_object_array_get_idx(jcpus,
										i));
			cpus.append(cpu);
		}
	}

	return cpus;
}

void KsSession::_saveTaskPlots(const QVector<int> &tasks)
{
	kshark_config_doc *taskPlts = kshark_config_alloc(KS_CONFIG_JSON);
	json_object *jtasks = json_object_new_array();

	for (int i = 0; i < tasks.count(); ++i) {
		json_object *jtask = json_object_new_int(tasks[i]);
		json_object_array_put_idx(jtasks, i, jtask);
	}

	taskPlts->conf_doc = jtasks;
	kshark_config_doc_add(_config, "TaskPlots", taskPlts);
}

QVector<int> KsSession::_getTaskPlots()
{
	kshark_config_doc *taskPlts = kshark_config_alloc(KS_CONFIG_JSON);
	json_object *jtasks;
	QVector<int> tasks;
	size_t length;

	if (!kshark_config_doc_get(_config, "TaskPlots", taskPlts))
		return tasks;

	if (_config->format == KS_CONFIG_JSON) {
		jtasks = KS_JSON_CAST(taskPlts->conf_doc);
		length = json_object_array_length(jtasks);
		for (size_t i = 0; i < length; ++i) {
			int pid = json_object_get_int(json_object_array_get_idx(jtasks,
										i));
			tasks.append(pid);
		}
	}

	return tasks;
}

/**
 * @brief Save the state of the Dual marker.
 *
 * @param dm: Input location for the KsDualMarkerSM object.
 */
void KsSession::saveDualMarker(KsDualMarkerSM *dm)
{
	struct kshark_config_doc *markers =
		kshark_config_new("kshark.config.markers", KS_CONFIG_JSON);
	json_object *jd_mark = KS_JSON_CAST(markers->conf_doc);

	auto save_mark = [&jd_mark] (KsGraphMark *m, const char *name)
	{
		json_object *jmark = json_object_new_object();

		if (m->_isSet) {
			json_object_object_add(jmark, "isSet",
					       json_object_new_boolean(true));

			json_object_object_add(jmark, "row",
					       json_object_new_int(m->_pos));
		} else {
			json_object_object_add(jmark, "isSet",
					       json_object_new_boolean(false));
		}

		json_object_object_add(jd_mark, name, jmark);
	};

	save_mark(&dm->markerA(), "markA");
	save_mark(&dm->markerB(), "markB");

	if (dm->getState() == DualMarkerState::A)
		json_object_object_add(jd_mark, "Active",
				       json_object_new_string("A"));
	else
		json_object_object_add(jd_mark, "Active",
				       json_object_new_string("B"));

	kshark_config_doc_add(_config, "Markers", markers);
}

/**
 * @brief Load the state of the Dual marker.
 *
 * @param dm: Input location for the KsDualMarkerSM object.
 * @param graphs: Input location for the KsTraceGraph widget.
 */
void KsSession::loadDualMarker(KsDualMarkerSM *dm, KsTraceGraph *graphs)
{
	size_t pos;

	dm->reset();
	dm->setState(DualMarkerState::A);
	if (_getMarker("markA", &pos)) {
		graphs->markEntry(pos);
	} else {
		dm->markerA().remove();
	}

	dm->setState(DualMarkerState::B);
	if (_getMarker("markB", &pos)) {
		graphs->markEntry(pos);
	} else {
		dm->markerB().remove();
	}

	dm->setState(_getMarkerState());
	if (dm->activeMarker()._isSet) {
		pos = dm->activeMarker()._pos;
		emit graphs->glPtr()->updateView(pos, true);
	}
}

json_object *KsSession::_getMarkerJson()
{
	struct kshark_config_doc *markers =
		kshark_config_alloc(KS_CONFIG_JSON);

	if (!kshark_config_doc_get(_config, "Markers", markers) ||
	    !kshark_type_check(markers, "kshark.config.markers"))
		return nullptr;

	return KS_JSON_CAST(markers->conf_doc);
}

bool KsSession::_getMarker(const char* name, size_t *pos)
{
	json_object *jd_mark, *jmark;

	*pos = 0;
	jd_mark = _getMarkerJson();
	if (!jd_mark)
		return false;

	if (json_object_object_get_ex(jd_mark, name, &jmark)) {
		json_object *jis_set;
		json_object_object_get_ex(jmark, "isSet", &jis_set);
		if (!json_object_get_boolean(jis_set))
			return false;

		json_object *jpos;
		json_object_object_get_ex(jmark, "row", &jpos);
		*pos = json_object_get_int64(jpos);
	}

	return true;
}

DualMarkerState KsSession::_getMarkerState()
{
	json_object *jd_mark, *jstate;
	const char* state;

	jd_mark = _getMarkerJson();
	json_object_object_get_ex(jd_mark, "Active", &jstate);
	state = json_object_get_string(jstate);

	if (strcmp(state, "A") == 0)
		return DualMarkerState::A;

	return DualMarkerState::B;
}

/**
 * @brief Save the configuration of the plugins.
 *
 * @param pm: Input location for the KsPluginManager object.
 */
void KsSession::savePlugins(const KsPluginManager &pm)
{
	struct kshark_config_doc *plugins =
		kshark_config_new("kshark.config.plugins", KS_CONFIG_JSON);
	json_object *jplugins = KS_JSON_CAST(plugins->conf_doc);
	const QVector<bool> &registeredPlugins = pm._registeredKsPlugins;
	const QStringList &pluginList = pm._ksPluginList;
	int nPlugins = pluginList.length();
	json_object *jlist, *jpl;
	QByteArray array;
	char* buffer;
	bool active;

	jlist = json_object_new_array();
	for (int i = 0; i < nPlugins; ++i) {
		array = pluginList[i].toLocal8Bit();
		buffer = array.data();
		jpl = json_object_new_array();
		json_object_array_put_idx(jpl, 0, json_object_new_string(buffer));

		active = registeredPlugins[i];
		json_object_array_put_idx(jpl, 1, json_object_new_boolean(active));
		json_object_array_put_idx(jlist, i, jpl);
	}

	json_object_object_add(jplugins, "Plugin List", jlist);
	kshark_config_doc_add(_config, "Plugins", plugins);
}

/**
 * @brief Load the configuration of the plugins.
 *
 * @param kshark_ctx: Input location for context pointer.
 * @param pm: Input location for the KsPluginManager object.
 */
void KsSession::loadPlugins(kshark_context *kshark_ctx, KsPluginManager *pm)
{
	kshark_config_doc *plugins = kshark_config_alloc(KS_CONFIG_JSON);
	json_object *jplugins, *jlist, *jpl;
	const char *pluginName;
	QVector<int> pluginIds;
	int length, index;
	bool loaded;

	if (!kshark_config_doc_get(_config, "Plugins", plugins) ||
	    !kshark_type_check(plugins, "kshark.config.plugins"))
		return;

	if (plugins->format == KS_CONFIG_JSON) {
		jplugins = KS_JSON_CAST(plugins->conf_doc);
		json_object_object_get_ex(jplugins, "Plugin List", &jlist);
		if (!jlist ||
	            json_object_get_type(jlist) != json_type_array ||
		    !json_object_array_length(jlist))
			return;

		length = json_object_array_length(jlist);
		for (int i = 0; i < length; ++i) {
			jpl = json_object_array_get_idx(jlist, i);
			pluginName = json_object_get_string(json_object_array_get_idx(jpl, 0));
			index = pm->_ksPluginList.indexOf(pluginName);
			loaded = json_object_get_boolean(json_object_array_get_idx(jpl, 1));
			if (index >= 0 && loaded)
				pluginIds.append(index);
		}
	}

	pm->updatePlugins(pluginIds);
}
