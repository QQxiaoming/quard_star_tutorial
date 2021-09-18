// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsUtils.cpp
 *  @brief   KernelShark Utils.
 */

// KernelShark
#include "KsUtils.hpp"
#include "KsWidgetsLib.hpp"

namespace KsUtils {

/** @brief Get a sorted vector of CPU Ids. */
QVector<int> getCPUList()
{
	kshark_context *kshark_ctx(nullptr);
	int nCPUs;

	if (!kshark_instance(&kshark_ctx))
		return {};

	nCPUs = tep_get_cpus(kshark_ctx->pevent);
	QVector<int> allCPUs = QVector<int>(nCPUs);
	std::iota(allCPUs.begin(), allCPUs.end(), 0);

	return allCPUs;
}

/** @brief Get a sorted vector of Task's Pids. */
QVector<int> getPidList()
{
	kshark_context *kshark_ctx(nullptr);
	int nTasks, *tempPids;
	QVector<int> pids;

	if (!kshark_instance(&kshark_ctx))
		return pids;

	nTasks = kshark_get_task_pids(kshark_ctx, &tempPids);
	for (int r = 0; r < nTasks; ++r) {
		pids.append(tempPids[r]);
	}

	free(tempPids);

	std::sort(pids.begin(), pids.end());

	return pids;
}

/**
 * @brief Get a sorted vector of Event Ids.
 */
QVector<int> getEventIdList(tep_event_sort_type sortType)
{
	kshark_context *kshark_ctx(nullptr);
	tep_event **events;
	int nEvts;

	if (!kshark_instance(&kshark_ctx))
		return {};

	nEvts = tep_get_events_count(kshark_ctx->pevent);
	events = tep_list_events(kshark_ctx->pevent, sortType);

	QVector<int> allEvts(nEvts);
	for (int i = 0; i < nEvts; ++i)
		allEvts[i] = events[i]->id;

	return allEvts;
}

/** @brief Get a sorted vector of Id values of a filter. */
QVector<int> getFilterIds(tracecmd_filter_id *filter)
{
	kshark_context *kshark_ctx(nullptr);
	int *cpuFilter, n;
	QVector<int> v;

	if (!kshark_instance(&kshark_ctx))
		return v;

	cpuFilter = tracecmd_filter_ids(filter);
	n = filter->count;
	for (int i = 0; i < n; ++i)
		v.append(cpuFilter[i]);

	std::sort(v.begin(), v.end());

	free(cpuFilter);
	return v;
}

/**
 * Set the bit of the filter mask of the kshark session context responsible
 * for the visibility of the events in the Table View.
 */
void listFilterSync(bool state)
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx))
		return;

	if (state) {
		kshark_ctx->filter_mask |= KS_TEXT_VIEW_FILTER_MASK;
	} else {
		kshark_ctx->filter_mask &= ~KS_TEXT_VIEW_FILTER_MASK;
	}
}

/**
 * Set the bit of the filter mask of the kshark session context responsible
 * for the visibility of the events in the Graph View.
 */
void graphFilterSync(bool state)
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx))
		return;

	if (state) {
		kshark_ctx->filter_mask |= KS_GRAPH_VIEW_FILTER_MASK;
		kshark_ctx->filter_mask |= KS_EVENT_VIEW_FILTER_MASK;
	} else {
		kshark_ctx->filter_mask &= ~KS_GRAPH_VIEW_FILTER_MASK;
		kshark_ctx->filter_mask &= ~KS_EVENT_VIEW_FILTER_MASK;
	}
}


/**
 * @brief Add a checkbox to a menu.
 *
 * @param menu: Input location for the menu object, to which the checkbox will be added.
 * @param name: The name of the checkbox.
 *
 * @returns The checkbox object;
 */
QCheckBox *addCheckBoxToMenu(QMenu *menu, QString name)
{
	QWidget  *containerWidget = new QWidget(menu);
	containerWidget->setLayout(new QHBoxLayout());
	containerWidget->layout()->setContentsMargins(FONT_WIDTH, FONT_HEIGHT/5,
						      FONT_WIDTH, FONT_HEIGHT/5);
	QCheckBox *checkBox = new QCheckBox(name, menu);
	containerWidget->layout()->addWidget(checkBox);

	QWidgetAction *action = new QWidgetAction(menu);
	action->setDefaultWidget(containerWidget);
	menu->addAction(action);

	return checkBox;
}

/**
 * @brief Simple CPU matching function to be user for data collections.
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param e: kshark_entry to be checked.
 * @param cpu: Matching condition value.
 *
 * @returns True if the CPU of the entry matches the value of "cpu" and
 * 	    the entry is visibility in Graph. Otherwise false.
 */
bool matchCPUVisible(struct kshark_context *kshark_ctx,
		     struct kshark_entry *e, int cpu)
{
	return (e->cpu == cpu && (e->visible & KS_GRAPH_VIEW_FILTER_MASK));
}

/**
 * @brief Check if the application runs from its installation location.
 */
bool isInstalled()
{
	QString appPath = QCoreApplication::applicationDirPath();
	QString installPath(_INSTALL_PREFIX);

	installPath += "/bin";
	installPath = QDir::cleanPath(installPath);

	return appPath == installPath;
}

static QString getFileDialog(QWidget *parent,
			     const QString &windowName,
			     const QString &filter,
			     QString &lastFilePath,
			     bool forSave)
{
	QString fileName;

	if (lastFilePath.isEmpty()) {
		lastFilePath = isInstalled() ? QDir::homePath() :
					       QDir::currentPath();
	}

	if (forSave) {
		fileName = QFileDialog::getSaveFileName(parent,
							windowName,
							lastFilePath,
							filter);
	} else {
		fileName = QFileDialog::getOpenFileName(parent,
							windowName,
							lastFilePath,
							filter);
	}

	if (!fileName.isEmpty())
		lastFilePath = QFileInfo(fileName).path();

	return fileName;
}

static QStringList getFilesDialog(QWidget *parent,
				  const QString &windowName,
				  const QString &filter,
				  QString &lastFilePath)
{
	QStringList fileNames;

	if (lastFilePath.isEmpty()) {
		lastFilePath = isInstalled() ? QDir::homePath() :
					       QDir::currentPath();
	}

	fileNames = QFileDialog::getOpenFileNames(parent,
						  windowName,
						  lastFilePath,
						  filter);

	if (!fileNames.isEmpty())
		lastFilePath = QFileInfo(fileNames[0]).path();

	return fileNames;
}

/**
 * @brief Open a standard Qt getFileName dialog and return the name of the
 *	  selected file. Only one file can be selected.
 */
QString getFile(QWidget *parent,
		const QString &windowName,
		const QString &filter,
		QString &lastFilePath)
{
	return getFileDialog(parent, windowName, filter, lastFilePath, false);
}

/**
 * @brief Open a standard Qt getFileName dialog and return the names of the
 *	  selected files. Multiple files can be selected.
 */
QStringList getFiles(QWidget *parent,
		     const QString &windowName,
		     const QString &filter,
		     QString &lastFilePath)
{
	return getFilesDialog(parent, windowName, filter, lastFilePath);
}

/**
 * @brief Open a standard Qt getFileName dialog and return the name of the
 *	  selected file. Only one file can be selected.
 */
QString getSaveFile(QWidget *parent,
		    const QString &windowName,
		    const QString &filter,
		    const QString &extension,
		    QString &lastFilePath)
{
	QString fileName = getFileDialog(parent,
					 windowName,
					 filter,
					 lastFilePath,
					 true);

	if (!fileName.isEmpty() && !fileName.endsWith(extension)) {
		fileName += extension;

		if (QFileInfo(fileName).exists()) {
			if (!KsWidgetsLib::fileExistsDialog(fileName))
				fileName.clear();
		}
	}

	return fileName;
}

/**
 * Separate the command line arguments inside the string taking into account
 * possible shell quoting and new lines.
 */
QStringList splitArguments(QString cmd)
{
	QString::SplitBehavior opt = QString::SkipEmptyParts;
	int i, progress = 0, size;
	QStringList argv;
	QChar quote = 0;

	/* Remove all new lines. */
	cmd.replace("\\\n", " ");

	size = cmd.count();
	auto lamMid = [&] () {return cmd.mid(progress, i - progress);};
	for (i = 0; i < size; ++i) {
		if (cmd[i] == '\\') {
			cmd.remove(i, 1);
			size --;
			continue;
		}

		if (cmd[i] == '\'' || cmd[i] == '"') {
			if (quote.isNull()) {
				argv << lamMid().split(" ", opt);
				quote = cmd[i++];
				progress = i;
			} else if (quote == cmd[i]) {
				argv << lamMid();
				quote = 0;
				progress = ++i;
			}
		}
	}

	argv << cmd.right(size - progress).split(" ", opt);

	return argv;
}

/** Parse a string containing Ids. The string can be of the form "1 4-7 9". */
QVector<int> parseIdList(QString v_str)
{
	QStringList list = v_str.split(",", QString::SkipEmptyParts);
	QVector<int> v;

	for (auto item: list) {
		int i = item.indexOf('-');
		if (i > 0) {
			/* This item is an interval. */
			int to = item.right(item.size() - i - 1).toInt();
			int from = item.left(i).toInt();
			int s = v.size();

			v.resize(s + to - from + 1);
			std::iota(v.begin() + s, v.end(), from);
		} else {
			v.append(item.toInt());
		}
	}

	return v;
}

}; // KsUtils

/** A stream operator for converting QColor into KsPlot::Color. */
KsPlot::Color& operator <<(KsPlot::Color &thisColor, const QColor &c)
{
	thisColor.set(c.red(), c.green(), c.blue());

	return thisColor;
}

/** Create a default (empty) KsDataStore. */
KsDataStore::KsDataStore(QWidget *parent)
: QObject(parent),
  _tep(nullptr),
  _rows(nullptr),
  _dataSize(0)
{}

/** Destroy the KsDataStore object. */
KsDataStore::~KsDataStore()
{}

/** Load trace data for file. */
void KsDataStore::loadDataFile(const QString &file)
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx))
		return;

	clear();

	if (!kshark_open(kshark_ctx, file.toStdString().c_str())) {
		qCritical() << "ERROR Loading file " << file;
		return;
	}

	_tep = kshark_ctx->pevent;

	if (kshark_ctx->event_handlers == nullptr)
		kshark_handle_plugins(kshark_ctx, KSHARK_PLUGIN_INIT);
	else
		kshark_handle_plugins(kshark_ctx, KSHARK_PLUGIN_UPDATE);

	_dataSize = kshark_load_data_entries(kshark_ctx, &_rows);
}

void KsDataStore::_freeData()
{
	if (_dataSize > 0) {
		for (ssize_t r = 0; r < _dataSize; ++r)
			free(_rows[r]);

		free(_rows);
	}

	_rows = nullptr;
	_dataSize = 0;
}

/** Reload the trace data. */
void KsDataStore::reload()
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx))
		return;

	_freeData();

	_dataSize = kshark_load_data_entries(kshark_ctx, &_rows);
	_tep = kshark_ctx->pevent;

	emit updateWidgets(this);
}

/** Free the loaded trace data and close the file. */
void KsDataStore::clear()
{
	kshark_context *kshark_ctx(nullptr);

	_freeData();
	_tep = nullptr;

	if (kshark_instance(&kshark_ctx) && kshark_ctx->handle)
		kshark_close(kshark_ctx);
}

/** Update the visibility of the entries (filter). */
void KsDataStore::update()
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx))
		return;

	_unregisterCPUCollections();

	if (kshark_filter_is_set(kshark_ctx)) {
		kshark_filter_entries(kshark_ctx, _rows, _dataSize);
		emit updateWidgets(this);
	}

	registerCPUCollections();
}

/** Register a collection of visible entries for each CPU. */
void KsDataStore::registerCPUCollections()
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx) ||
	    !kshark_filter_is_set(kshark_ctx))
		return;

	int nCPUs = tep_get_cpus(_tep);
	for (int cpu = 0; cpu < nCPUs; ++cpu) {
		kshark_register_data_collection(kshark_ctx,
						_rows, _dataSize,
						KsUtils::matchCPUVisible,
						cpu,
						0);
	}
}

void KsDataStore::_unregisterCPUCollections()
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx))
		return;

	int nCPUs = tep_get_cpus(_tep);
	for (int cpu = 0; cpu < nCPUs; ++cpu) {
		kshark_unregister_data_collection(&kshark_ctx->collections,
						  KsUtils::matchCPUVisible,
						  cpu);
	}
}

void KsDataStore::_applyIdFilter(int filterId, QVector<int> vec)
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx))
		return;

	switch (filterId) {
		case KS_SHOW_EVENT_FILTER:
		case KS_HIDE_EVENT_FILTER:
			kshark_filter_clear(kshark_ctx, KS_SHOW_EVENT_FILTER);
			kshark_filter_clear(kshark_ctx, KS_HIDE_EVENT_FILTER);
			break;
		case KS_SHOW_TASK_FILTER:
		case KS_HIDE_TASK_FILTER:
			kshark_filter_clear(kshark_ctx, KS_SHOW_TASK_FILTER);
			kshark_filter_clear(kshark_ctx, KS_HIDE_TASK_FILTER);
			break;
		case KS_SHOW_CPU_FILTER:
		case KS_HIDE_CPU_FILTER:
			kshark_filter_clear(kshark_ctx, KS_SHOW_CPU_FILTER);
			kshark_filter_clear(kshark_ctx, KS_HIDE_CPU_FILTER);
			break;
		default:
			return;
	}

	for (auto &&val: vec)
		kshark_filter_add_id(kshark_ctx, filterId, val);

	if (!_tep)
		return;

	_unregisterCPUCollections();

	/*
	 * If the advanced event filter is set, the data has to be reloaded,
	 * because the advanced filter uses tep_records.
	 */
	if (kshark_ctx->advanced_event_filter->filters)
		reload();
	else
		kshark_filter_entries(kshark_ctx, _rows, _dataSize);

	registerCPUCollections();

	emit updateWidgets(this);
}

/** Apply Show Task filter. */
void KsDataStore::applyPosTaskFilter(QVector<int> vec)
{
	_applyIdFilter(KS_SHOW_TASK_FILTER, vec);
}

/** Apply Hide Task filter. */
void KsDataStore::applyNegTaskFilter(QVector<int> vec)
{
	_applyIdFilter(KS_HIDE_TASK_FILTER, vec);
}

/** Apply Show Event filter. */
void KsDataStore::applyPosEventFilter(QVector<int> vec)
{
	_applyIdFilter(KS_SHOW_EVENT_FILTER, vec);
}

/** Apply Hide Event filter. */
void KsDataStore::applyNegEventFilter(QVector<int> vec)
{
	_applyIdFilter(KS_HIDE_EVENT_FILTER, vec);
}

/** Apply Show CPU filter. */
void KsDataStore::applyPosCPUFilter(QVector<int> vec)
{
	_applyIdFilter(KS_SHOW_CPU_FILTER, vec);
}

/** Apply Hide CPU filter. */
void KsDataStore::applyNegCPUFilter(QVector<int> vec)
{
	_applyIdFilter(KS_HIDE_CPU_FILTER, vec);
}

/** Disable all filters. */
void KsDataStore::clearAllFilters()
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx) || !_tep)
		return;

	_unregisterCPUCollections();

	kshark_filter_clear(kshark_ctx, KS_SHOW_TASK_FILTER);
	kshark_filter_clear(kshark_ctx, KS_HIDE_TASK_FILTER);
	kshark_filter_clear(kshark_ctx, KS_SHOW_EVENT_FILTER);
	kshark_filter_clear(kshark_ctx, KS_HIDE_EVENT_FILTER);
	kshark_filter_clear(kshark_ctx, KS_SHOW_CPU_FILTER);
	kshark_filter_clear(kshark_ctx, KS_HIDE_CPU_FILTER);

	tep_filter_reset(kshark_ctx->advanced_event_filter);
	kshark_clear_all_filters(kshark_ctx, _rows, _dataSize);

	emit updateWidgets(this);
}

/**
 * @brief Create Plugin Manager. Use list of plugins declared in the
 *	  CMake-generated header file.
 */
KsPluginManager::KsPluginManager(QWidget *parent)
: QObject(parent)
{
	kshark_context *kshark_ctx(nullptr);
	_parsePluginList();

	if (!kshark_instance(&kshark_ctx))
		return;

	registerFromList(kshark_ctx);
}

/** Parse the plugin list declared in the CMake-generated header file. */
void KsPluginManager::_parsePluginList()
{
	_ksPluginList = KsUtils::getPluginList();
	int nPlugins = _ksPluginList.count();

	_registeredKsPlugins.resize(nPlugins);
	for (int i = 0; i < nPlugins; ++i) {
		if (_ksPluginList[i].contains(" default", Qt::CaseInsensitive)) {
			_ksPluginList[i].remove(" default", Qt::CaseInsensitive);
			_registeredKsPlugins[i] = true;
		} else {
			_registeredKsPlugins[i] = false;
		}
	}
}

/**
 * Register the plugins by using the information in "_ksPluginList" and
 * "_registeredKsPlugins".
 */
void KsPluginManager::registerFromList(kshark_context *kshark_ctx)
{
	auto lamRegBuiltIn = [&kshark_ctx, this](const QString &plugin)
	{
		char *lib;
		int n;

		lib = _pluginLibFromName(plugin, n);
		if (n <= 0)
			return;

		kshark_register_plugin(kshark_ctx, lib);
		free(lib);
	};

	auto lamRegUser = [&kshark_ctx](const QString &plugin)
	{
		std::string lib = plugin.toStdString();
		kshark_register_plugin(kshark_ctx, lib.c_str());
	};

	_forEachInList(_ksPluginList,
		       _registeredKsPlugins,
		       lamRegBuiltIn);

	_forEachInList(_userPluginList,
		       _registeredUserPlugins,
		       lamRegUser);
}

/**
 * Unegister the plugins by using the information in "_ksPluginList" and
 * "_registeredKsPlugins".
 */
void KsPluginManager::unregisterFromList(kshark_context *kshark_ctx)
{
	auto lamUregBuiltIn = [&kshark_ctx, this](const QString &plugin)
	{
		char *lib;
		int n;

		lib = _pluginLibFromName(plugin, n);
		if (n <= 0)
			return;

		kshark_unregister_plugin(kshark_ctx, lib);
		free(lib);
	};

	auto lamUregUser = [&kshark_ctx](const QString &plugin)
	{
		std::string lib = plugin.toStdString();
		kshark_unregister_plugin(kshark_ctx, lib.c_str());
	};

	_forEachInList(_ksPluginList,
		       _registeredKsPlugins,
			lamUregBuiltIn);

	_forEachInList(_userPluginList,
		       _registeredUserPlugins,
			lamUregUser);
}

char *KsPluginManager::_pluginLibFromName(const QString &plugin, int &n)
{
	QString appPath = QCoreApplication::applicationDirPath();
	QString libPath = appPath + "/../../kernel-shark/lib";
	std::string pluginStr = plugin.toStdString();
	char *lib;

	libPath = QDir::cleanPath(libPath);
	if (!KsUtils::isInstalled() && QDir(libPath).exists()) {
		std::string pathStr = libPath.toStdString();
		n = asprintf(&lib, "%s/plugin-%s.so",
			     pathStr.c_str(), pluginStr.c_str());
	} else {
		n = asprintf(&lib, "%s/plugin-%s.so",
			     KS_PLUGIN_INSTALL_PREFIX, pluginStr.c_str());
	}

	return lib;
}

/**
 * @brief Register a Plugin.
 *
 * @param plugin: provide here the name of the plugin (as in the CMake-generated
 *		  header file) of a name of the plugin's library file (.so).
 */
void KsPluginManager::registerPlugin(const QString &plugin)
{
	kshark_context *kshark_ctx(nullptr);
	char *lib;
	int n;

	if (!kshark_instance(&kshark_ctx))
		return;

	for (int i = 0; i < _ksPluginList.count(); ++i) {
		if (_ksPluginList[i] == plugin) {
			/*
			 * The argument is the name of the plugin. From the
			 * name get the library .so file.
			 */
			lib = _pluginLibFromName(plugin, n);
			if (n > 0) {
				kshark_register_plugin(kshark_ctx, lib);
				_registeredKsPlugins[i] = true;
				free(lib);
			}

			return;

		} else if (plugin.contains("/lib/plugin-" + _ksPluginList[i],
					   Qt::CaseInsensitive)) {
			/*
			 * The argument is the name of the library .so file.
			 */
			n = asprintf(&lib, "%s", plugin.toStdString().c_str());
			if (n > 0) {
				kshark_register_plugin(kshark_ctx, lib);
				_registeredKsPlugins[i] = true;
				free(lib);
			}

			return;
		}
	}

	/* No plugin with this name in the list. Try to add it anyway. */
	if (plugin.endsWith(".so") && QFileInfo::exists(plugin)) {
		kshark_register_plugin(kshark_ctx,
				       plugin.toStdString().c_str());

		_userPluginList.append(plugin);
		_registeredUserPlugins.append(true);
	} else {
		qCritical() << "ERROR: " << plugin << "cannot be registered!";
	}
}

/** @brief Unregister a Built in KernelShark plugin.
 *<br>
 * WARNING: Do not use this function to unregister User plugins.
 * Instead use directly kshark_unregister_plugin().
 *
 * @param plugin: provide here the name of the plugin (as in the CMake-generated
 *		  header file) or a name of the plugin's library file (.so).
 *
 */
void KsPluginManager::unregisterPlugin(const QString &plugin)
{
	kshark_context *kshark_ctx(nullptr);
	char *lib;
	int n;

	if (!kshark_instance(&kshark_ctx))
		return;

	for (int i = 0; i < _ksPluginList.count(); ++i) {
		if (_ksPluginList[i] == plugin) {
			/*
			 * The argument is the name of the plugin. From the
			 * name get the library .so file.
			 */
			lib = _pluginLibFromName(plugin, n);
			if (n > 0) {
				kshark_unregister_plugin(kshark_ctx, lib);
				_registeredKsPlugins[i] = false;
				free(lib);
			}

			return;
		} else if (plugin.contains("/lib/plugin-" +
			                   _ksPluginList[i], Qt::CaseInsensitive)) {
			/*
			 * The argument is the name of the library .so file.
			 */
			n = asprintf(&lib, "%s", plugin.toStdString().c_str());
			if (n > 0) {
				kshark_unregister_plugin(kshark_ctx, lib);
				_registeredKsPlugins[i] = false;
				free(lib);
			}

			return;
		}
	}
}

/** @brief Add to the list and initialize user-provided plugins. All other
 *	   previously loaded plugins will be reinitialized and the data will be
 *	   reloaded.
 *
 * @param fileNames: the library files (.so) of the plugins.
*/
void KsPluginManager::addPlugins(const QStringList &fileNames)
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx))
		return;

	kshark_handle_plugins(kshark_ctx, KSHARK_PLUGIN_CLOSE);

	for (auto const &p: fileNames)
		registerPlugin(p);

	kshark_handle_plugins(kshark_ctx, KSHARK_PLUGIN_INIT);

	emit dataReload();
}

/** Unload all plugins. */
void KsPluginManager::unloadAll()
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx))
		return;

	kshark_handle_plugins(kshark_ctx, KSHARK_PLUGIN_CLOSE);
	kshark_free_plugin_list(kshark_ctx->plugins);
	kshark_ctx->plugins = nullptr;
	kshark_free_event_handler_list(kshark_ctx->event_handlers);

	unregisterFromList(kshark_ctx);
}

/** @brief Update (change) the Plugins.
 *
 * @param pluginIds: The indexes of the plugins to be loaded.
 */
void KsPluginManager::updatePlugins(QVector<int> pluginIds)
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx))
		return;

	auto register_plugins = [&] (QVector<int> ids)
	{
		int nKsPlugins = _registeredKsPlugins.count();

		/* First clear all registered plugins. */
		for (auto &p: _registeredKsPlugins)
			p = false;
		for (auto &p: _registeredUserPlugins)
			p = false;

		/* The vector contains the indexes of those to register. */
		for (auto const &p: ids) {
			if (p < nKsPlugins)
				_registeredKsPlugins[p] = true;
			else
				_registeredUserPlugins[p - nKsPlugins] = true;
		}
		registerFromList(kshark_ctx);
	};

	if (!kshark_ctx->pevent) {
		kshark_free_plugin_list(kshark_ctx->plugins);
		kshark_ctx->plugins = nullptr;

		/*
		 * No data is loaded. For the moment, just register the
		 * plugins. Handling of the plugins will be done after
		 * we load a data file.
		 */
		register_plugins(pluginIds);
		return;
	}

	/* Clean up all old plugins first. */
	unloadAll();

	/* Now load. */
	register_plugins(pluginIds);
	kshark_handle_plugins(kshark_ctx, KSHARK_PLUGIN_INIT);

	emit dataReload();
}
