// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsMainWindow.cpp
 *  @brief   KernelShark GUI main window.
 */

// C
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

// C++11
#include <thread>

// Qt
#include <QMenu>
#include <QFileDialog>
#include <QMenuBar>
#include <QLabel>
#include <QLocalSocket>

// KernelShark
#include "libkshark.h"
#include "KsCmakeDef.hpp"
#include "KsMainWindow.hpp"
#include "KsCaptureDialog.hpp"
#include "KsAdvFilteringDialog.hpp"

/** Create KernelShark Main window. */
KsMainWindow::KsMainWindow(QWidget *parent)
: QMainWindow(parent),
  _splitter(Qt::Vertical, this),
  _data(this),
  _view(this),
  _graph(this),
  _mState(this),
  _plugins(this),
  _capture(this),
  _captureLocalServer(this),
  _openAction("Open", this),
  _restoreSessionAction("Restore Last Session", this),
  _importSessionAction("Import Session", this),
  _exportSessionAction("Export Session", this),
  _quitAction("Quit", this),
  _importFilterAction("Import Filter", this),
  _exportFilterAction("Export Filter", this),
  _graphFilterSyncCBox(nullptr),
  _listFilterSyncCBox(nullptr),
  _showEventsAction("Show events", this),
  _showTasksAction("Show tasks", this),
  _showCPUsAction("Show CPUs", this),
  _advanceFilterAction("Advance Filtering", this),
  _clearAllFilters("Clear all filters", this),
  _cpuSelectAction("CPUs", this),
  _taskSelectAction("Tasks", this),
  _managePluginsAction("Manage plugins", this),
  _addPluginsAction("Add plugins", this),
  _captureAction("Record", this),
  _colorAction(this),
  _colSlider(this),
  _colorPhaseSlider(Qt::Horizontal, this),
  _fullScreenModeAction("Full Screen Mode", this),
  _aboutAction("About", this),
  _contentsAction("Contents", this),
  _bugReportAction("Report a bug", this),
  _deselectShortcut(this),
  _settings(_getCacheDir() + "/setting.ini", QSettings::IniFormat)
{
	setWindowTitle("Kernel Shark");
	_createActions();
	_createMenus();
	_initCapture();

	if (geteuid() == 0)
		_rootWarning();

	_splitter.addWidget(&_graph);
	_splitter.addWidget(&_view);
	setCentralWidget(&_splitter);
	connect(&_splitter,	&QSplitter::splitterMoved,
		this,		&KsMainWindow::_splitterMoved);

	_view.setMarkerSM(&_mState);
	connect(&_mState,	&KsDualMarkerSM::markSwitchForView,
		&_view,		&KsTraceViewer::markSwitch);

	_graph.setMarkerSM(&_mState);

	connect(&_mState,	&KsDualMarkerSM::updateGraph,
		&_graph,	&KsTraceGraph::markEntry);

	connect(&_mState,	&KsDualMarkerSM::updateView,
		&_view,		&KsTraceViewer::showRow);

	connect(&_view,		&KsTraceViewer::select,
		&_graph,	&KsTraceGraph::markEntry);

	connect(&_view,		&KsTraceViewer::addTaskPlot,
		&_graph,	&KsTraceGraph::addTaskPlot);

	connect(_graph.glPtr(), &KsGLWidget::updateView,
		&_view,		&KsTraceViewer::showRow);

	connect(&_graph,	&KsTraceGraph::deselect,
		this,		&KsMainWindow::_deselectActive);

	connect(&_view,		&KsTraceViewer::deselect,
		this,		&KsMainWindow::_deselectActive);

	connect(&_data,		&KsDataStore::updateWidgets,
		&_view,		&KsTraceViewer::update);

	connect(&_data,		&KsDataStore::updateWidgets,
		&_graph,	&KsTraceGraph::update);

	connect(&_plugins,	&KsPluginManager::dataReload,
		&_data,		&KsDataStore::reload);

	_deselectShortcut.setKey(Qt::CTRL + Qt::Key_D);
	connect(&_deselectShortcut,	&QShortcut::activated,
		this,			&KsMainWindow::_deselectActive);

	connect(&_mState,	&KsDualMarkerSM::deselectA,
		this,		&KsMainWindow::_deselectA);

	connect(&_mState,	&KsDualMarkerSM::deselectB,
		this,		&KsMainWindow::_deselectB);

	_lastDataFilePath = _settings.value("dataPath").toString();
	_lastConfFilePath = _settings.value("confPath").toString();
	_lastPluginFilePath = _settings.value("pluginPath").toString();

	_resizeEmpty();
}

/** Destroy KernelShark Main window. */
KsMainWindow::~KsMainWindow()
{
	kshark_context *kshark_ctx(nullptr);
	QString file = lastSessionFile();

	if (!file.isEmpty()) {
		QByteArray fileBA = file.toLocal8Bit();

		_updateSession();
		kshark_save_config_file(fileBA.data(),
					_session.getConfDocPtr());
	}

	_settings.setValue("dataPath", _lastDataFilePath);
	_settings.setValue("confPath", _lastConfFilePath);
	_settings.setValue("pluginPath", _lastPluginFilePath);

	_data.clear();

	/*
	 * Do not show error messages if the "capture" process is still
	 * running (Capture dialog is not closed).
	 */
	if (_capture.state() != QProcess::NotRunning) {
		disconnect(_captureErrorConnection);
		_capture.close();
		_capture.waitForFinished();
	}

	if (kshark_instance(&kshark_ctx))
		kshark_free(kshark_ctx);
}

/** Set the list ot CPU cores to be plotted. */
void KsMainWindow::setCPUPlots(QVector<int> cpus)
{
	int nCPUs = tep_get_cpus(_data.tep());
	auto lamCPUCheck = [=] (int cpu) {
		if (cpu >= nCPUs) {
			qWarning() << "Warning: No CPU" << cpu << "found in the data.";
			return true;
		}

		return false;
	};

	cpus.erase(std::remove_if(cpus.begin(), cpus.end(), lamCPUCheck),
		   cpus.end());

	_graph.cpuReDraw(cpus);
}

/** Set the list ot tasks (pids) to be plotted. */
void KsMainWindow::setTaskPlots(QVector<int> pids)
{
	QVector<int> allPids = KsUtils::getPidList();
	auto lamPidCheck = [=] (int pid) {
		int i = allPids.indexOf(pid);
		if (i < 0) {
			qWarning() << "Warning: No Pid" << pid << "found in the data.";
			return true;
		}

		return false;
	};

	pids.erase(std::remove_if(pids.begin(), pids.end(), lamPidCheck),
		   pids.end());

	_graph.taskReDraw(pids);
}

/**
 * Reimplemented event handler used to update the geometry of the window on
 * resize events.
 */
void KsMainWindow::resizeEvent(QResizeEvent* event)
{
	QMainWindow::resizeEvent(event);

	_session.saveMainWindowSize(*this);
	_session.saveSplitterSize(_splitter);
}

void KsMainWindow::_createActions()
{
	/* File menu */
	_openAction.setIcon(QIcon::fromTheme("document-open"));
	_openAction.setShortcut(tr("Ctrl+O"));
	_openAction.setStatusTip("Open an existing data file");

	connect(&_openAction,	&QAction::triggered,
		this,		&KsMainWindow::_open);

	_restoreSessionAction.setIcon(QIcon::fromTheme("document-open-recent"));
	connect(&_restoreSessionAction,	&QAction::triggered,
		this,			&KsMainWindow::_restoreSession);

	_importSessionAction.setIcon(QIcon::fromTheme("document-send"));
	_importSessionAction.setStatusTip("Load a session");

	connect(&_importSessionAction,	&QAction::triggered,
		this,			&KsMainWindow::_importSession);

	_exportSessionAction.setIcon(QIcon::fromTheme("document-revert"));
	_exportSessionAction.setStatusTip("Export this session");

	connect(&_exportSessionAction,	&QAction::triggered,
		this,			&KsMainWindow::_exportSession);

	_quitAction.setIcon(QIcon::fromTheme("window-close"));
	_quitAction.setShortcut(tr("Ctrl+Q"));
	_quitAction.setStatusTip("Exit KernelShark");

	connect(&_quitAction,	&QAction::triggered,
		this,		&KsMainWindow::close);

	/* Filter menu */
	_importFilterAction.setIcon(QIcon::fromTheme("document-send"));
	_importFilterAction.setStatusTip("Load a filter");

	connect(&_importFilterAction,	&QAction::triggered,
		this,			&KsMainWindow::_importFilter);

	_exportFilterAction.setIcon(QIcon::fromTheme("document-revert"));
	_exportFilterAction.setStatusTip("Export a filter");

	connect(&_exportFilterAction,	&QAction::triggered,
		this,			&KsMainWindow::_exportFilter);

	connect(&_showEventsAction,	&QAction::triggered,
		this,			&KsMainWindow::_showEvents);

	connect(&_showTasksAction,	&QAction::triggered,
		this,			&KsMainWindow::_showTasks);

	connect(&_showCPUsAction,	&QAction::triggered,
		this,			&KsMainWindow::_showCPUs);

	connect(&_advanceFilterAction,	&QAction::triggered,
		this,			&KsMainWindow::_advancedFiltering);

	connect(&_clearAllFilters,	&QAction::triggered,
		this,			&KsMainWindow::_clearFilters);

	/* Plot menu */
	connect(&_cpuSelectAction,	&QAction::triggered,
		this,			&KsMainWindow::_cpuSelect);

	connect(&_taskSelectAction,	&QAction::triggered,
		this,			&KsMainWindow::_taskSelect);

	/* Tools menu */
	_managePluginsAction.setShortcut(tr("Ctrl+P"));
	_managePluginsAction.setIcon(QIcon::fromTheme("preferences-system"));
	_managePluginsAction.setStatusTip("Manage plugins");

	connect(&_managePluginsAction,	&QAction::triggered,
		this,			&KsMainWindow::_pluginSelect);

	_addPluginsAction.setIcon(QIcon::fromTheme("applications-engineering"));
	_addPluginsAction.setStatusTip("Add plugins");

	connect(&_addPluginsAction,	&QAction::triggered,
		this,			&KsMainWindow::_pluginAdd);

	_captureAction.setIcon(QIcon::fromTheme("media-record"));
	_captureAction.setShortcut(tr("Ctrl+R"));
	_captureAction.setStatusTip("Capture trace data");

	connect(&_captureAction,	&QAction::triggered,
		this,			&KsMainWindow::_record);

	_colorPhaseSlider.setMinimum(20);
	_colorPhaseSlider.setMaximum(180);
	_colorPhaseSlider.setValue(KsPlot::Color::getRainbowFrequency() * 100);
	_colorPhaseSlider.setFixedWidth(FONT_WIDTH * 15);

	connect(&_colorPhaseSlider,	&QSlider::valueChanged,
		this,			&KsMainWindow::_setColorPhase);

	_colSlider.setLayout(new QHBoxLayout);
	_colSlider.layout()->addWidget(new QLabel("Color scheme", this));
	_colSlider.layout()->addWidget(&_colorPhaseSlider);
	_colorAction.setDefaultWidget(&_colSlider);

	_fullScreenModeAction.setIcon(QIcon::fromTheme("view-fullscreen"));
	_fullScreenModeAction.setShortcut(tr("Ctrl+Shift+F"));
	_fullScreenModeAction.setStatusTip("Full Screen Mode");

	connect(&_fullScreenModeAction,	&QAction::triggered,
		this,			&KsMainWindow::_changeScreenMode);

	/* Help menu */
	_aboutAction.setIcon(QIcon::fromTheme("help-about"));

	connect(&_aboutAction,		&QAction::triggered,
		this,			&KsMainWindow::_aboutInfo);

	_contentsAction.setIcon(QIcon::fromTheme("help-contents"));
	connect(&_contentsAction,	&QAction::triggered,
		this,			&KsMainWindow::_contents);

	connect(&_bugReportAction,	&QAction::triggered,
		this,			&KsMainWindow::_bugReport);
}

void KsMainWindow::_createMenus()
{
	QMenu *file, *sessions, *filter, *plots, *tools, *help;
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx))
		return;

	/* File menu */
	file = menuBar()->addMenu("File");
	file->addAction(&_openAction);

	sessions = file->addMenu("Sessions");
	sessions->setIcon(QIcon::fromTheme("document-properties"));
	sessions->addAction(&_restoreSessionAction);
	sessions->addAction(&_importSessionAction);
	sessions->addAction(&_exportSessionAction);
	file->addAction(&_quitAction);

	/* Filter menu */
	filter = menuBar()->addMenu("Filter");

	connect(filter, 		&QMenu::aboutToShow,
		this,			&KsMainWindow::_updateFilterMenu);

	filter->addAction(&_importFilterAction);
	filter->addAction(&_exportFilterAction);

	/*
	 * Set the default filter mask. Filter will apply to both View and
	 * Graph.
	 */
	kshark_ctx->filter_mask =
		KS_TEXT_VIEW_FILTER_MASK | KS_GRAPH_VIEW_FILTER_MASK;

	kshark_ctx->filter_mask |= KS_EVENT_VIEW_FILTER_MASK;

	_graphFilterSyncCBox =
		KsUtils::addCheckBoxToMenu(filter, "Apply filters to Graph");
	_graphFilterSyncCBox->setChecked(true);

	connect(_graphFilterSyncCBox,	&QCheckBox::stateChanged,
		this,			&KsMainWindow::_graphFilterSync);

	_listFilterSyncCBox =
		KsUtils::addCheckBoxToMenu(filter, "Apply filters to List");
	_listFilterSyncCBox->setChecked(true);

	connect(_listFilterSyncCBox,	&QCheckBox::stateChanged,
		this,			&KsMainWindow::_listFilterSync);

	filter->addAction(&_showEventsAction);
	filter->addAction(&_showTasksAction);
	filter->addAction(&_showCPUsAction);
	filter->addAction(&_advanceFilterAction);
	filter->addAction(&_clearAllFilters);

	/* Plot menu */
	plots = menuBar()->addMenu("Plots");
	plots->addAction(&_cpuSelectAction);
	plots->addAction(&_taskSelectAction);

	/* Tools menu */
	tools = menuBar()->addMenu("Tools");
	tools->addAction(&_managePluginsAction);
	tools->addAction(&_addPluginsAction);
	tools->addAction(&_captureAction);
	tools->addSeparator();
	tools->addAction(&_colorAction);
	tools->addAction(&_fullScreenModeAction);

	/* Help menu */
	help = menuBar()->addMenu("Help");
	help->addAction(&_aboutAction);
	help->addAction(&_contentsAction);
	help->addAction(&_bugReportAction);
}

void KsMainWindow::_open()
{
	QString fileName;

	fileName = KsUtils::getFile(this, "Open File",
				    "trace-cmd files (*.dat);;All files (*)",
				    _lastDataFilePath);

	if (!fileName.isEmpty())
		loadDataFile(fileName);
}

QString KsMainWindow::_getCacheDir()
{
	QString dir;

	auto lamMakePath = [&] (bool ask) {
		if (ask) {
			QMessageBox::StandardButton reply;
			QString err("KernelShark cache directory not found!\n");
			QString question =
				QString("Do you want to create %1").arg(dir);

			reply = QMessageBox::question(this, "KernelShark",
						      err + question,
						      QMessageBox::Yes |
						      QMessageBox::No);

			if (reply == QMessageBox::No) {
				dir.clear();
				return;
			}
		}

		QDir().mkpath(dir);
	};

	auto lamRootHome = [] () {
		struct passwd *pwd = getpwuid(0);

		return pwd ? QString(pwd->pw_dir) : QString("/root");
	};

	dir = getenv("KS_USER_CACHE_DIR");
	if (!dir.isEmpty()) {
		if (!QDir(dir).exists())
			lamMakePath(true);
	} else {
		auto appCachePath = QStandardPaths::GenericCacheLocation;
		dir = QStandardPaths::writableLocation(appCachePath);
		dir += "/kernelshark";

		if (geteuid() == 0)
			dir.replace(QDir::homePath(), lamRootHome());

		if (!QDir(dir).exists())
			lamMakePath(false);
	}

	return dir;
}

/** Get the description file of the last session. */
QString KsMainWindow::lastSessionFile()
{
	QString file;

	file = _getCacheDir();
	if (!file.isEmpty())
		file += "/lastsession.json";

	return file;
}

void KsMainWindow::_restoreSession()
{
	loadSession(lastSessionFile());
}

void KsMainWindow::_importSession()
{
	QString fileName;

	fileName = KsUtils::getFile(this, "Import Session",
				    "Kernel Shark Config files (*.json);;",
				    _lastConfFilePath);

	if (fileName.isEmpty())
		return;

	loadSession(fileName);
}

void KsMainWindow::_updateSession()
{
	kshark_context *kshark_ctx(nullptr);

	if (!kshark_instance(&kshark_ctx))
		return;

	_session.saveGraphs(*_graph.glPtr());
	_session.saveVisModel(_graph.glPtr()->model()->histo());
	_session.saveFilters(kshark_ctx);
	_session.saveDualMarker(&_mState);
	_session.saveTable(_view);
	_session.saveColorScheme();
	_session.savePlugins(_plugins);
}

void KsMainWindow::_exportSession()
{
	QString fileName;

	fileName = KsUtils::getSaveFile(this, "Export Filter",
					"Kernel Shark Config files (*.json);;",
					".json",
					_lastConfFilePath);
	if (fileName.isEmpty())
		return;

	_updateSession();
	_session.exportToFile(fileName);
}

void KsMainWindow::_filterSyncCBoxUpdate(kshark_context *kshark_ctx)
{
	if (kshark_ctx->filter_mask & KS_TEXT_VIEW_FILTER_MASK)
		_listFilterSyncCBox->setChecked(true);
	else
		_listFilterSyncCBox->setChecked(false);

	if (kshark_ctx->filter_mask &
	    (KS_GRAPH_VIEW_FILTER_MASK | KS_EVENT_VIEW_FILTER_MASK))
		_graphFilterSyncCBox->setChecked(true);
	else
		_graphFilterSyncCBox->setChecked(false);
}

void KsMainWindow::_updateFilterMenu()
{
	kshark_context *kshark_ctx(nullptr);

	if (kshark_instance(&kshark_ctx))
		_filterSyncCBoxUpdate(kshark_ctx);
}

void KsMainWindow::_importFilter()
{
	kshark_context *kshark_ctx(nullptr);
	kshark_config_doc *conf;
	QString fileName;

	if (!kshark_instance(&kshark_ctx) || _data.size() < 1)
		return;

	fileName = KsUtils::getFile(this, "Import Filter",
				    "Kernel Shark Config files (*.json);;",
				    _lastConfFilePath);

	if (fileName.isEmpty())
		return;

	conf = kshark_open_config_file(fileName.toStdString().c_str(),
				       "kshark.config.filter");
	if (!conf)
		return;

	kshark_import_all_event_filters(kshark_ctx, conf);
	kshark_free_config_doc(conf);

	kshark_filter_entries(kshark_ctx, _data.rows(), _data.size());
	_filterSyncCBoxUpdate(kshark_ctx);
	emit _data.updateWidgets(&_data);
}

void KsMainWindow::_exportFilter()
{
	kshark_context *kshark_ctx(nullptr);
	kshark_config_doc *conf(nullptr);
	QString fileName;

	if (!kshark_instance(&kshark_ctx))
		return;

	fileName = KsUtils::getSaveFile(this, "Export Filter",
					"Kernel Shark Config files (*.json);;",
					".json",
					_lastConfFilePath);

	if (fileName.isEmpty())
		return;

	kshark_export_all_event_filters(kshark_ctx, &conf);
	kshark_save_config_file(fileName.toStdString().c_str(), conf);
	kshark_free_config_doc(conf);
}

void KsMainWindow::_listFilterSync(int state)
{
	KsUtils::listFilterSync(state);
	_data.update();
}

void KsMainWindow::_graphFilterSync(int state)
{
	KsUtils::graphFilterSync(state);
	_data.update();
}

void KsMainWindow::_presetCBWidget(tracecmd_filter_id *showFilter,
				   tracecmd_filter_id *hideFilter,
				   KsCheckBoxWidget *cbw)
{
	if (!kshark_this_filter_is_set(showFilter) &&
	    !kshark_this_filter_is_set(hideFilter)) {
		/*
		 * No filter is set currently. All CheckBoxes of the Widget
		 * will be checked.
		 */
		cbw->setDefault(true);
	} else {
		QVector<int> ids = cbw->getIds();
		QVector<bool>  status;
		int n = ids.count();
		bool show, hide;

		if (kshark_this_filter_is_set(showFilter)) {
			/*
			 * The "show only" filter is set. The default status
			 * of all CheckBoxes will be "unchecked".
			 */
			status = QVector<bool>(n, false);
			for (int i = 0; i < n; ++i) {
				show = !!tracecmd_filter_id_find(showFilter,
							         ids[i]);

				hide = !!tracecmd_filter_id_find(hideFilter,
							         ids[i]);

				if (show && !hide) {
					/*
					 * Both "show" and "hide" define this
					 * Id as visible. Set the status of
					 * its CheckBoxes to "checked".
					 */
					status[i] = true;
				}
			}
		} else {
			/*
			 * Only the "do not show" filter is set. The default
			 * status of all CheckBoxes will be "checked".
			 */
			status = QVector<bool>(n, true);
			for (int i = 0; i < n; ++i) {
				hide = !!tracecmd_filter_id_find(hideFilter,
							         ids[i]);

				if (hide)
					status[i] = false;
			}
		}

		cbw->set(status);
	}
}

void KsMainWindow::_applyFilter(QVector<int> all, QVector<int> show,
				std::function<void(QVector<int>)> posFilter,
				std::function<void(QVector<int>)> negFilter)
{
	if (show.count() < all.count() / 2) {
		posFilter(show);
	} else {
		/*
		 * It is more efficiant to apply negative (do not show) filter.
		 */
		QVector<int> diff;

		/*
		 * The Ids may not be sorted, because in the widgets the items
		 * are shown sorted by name. Get those Ids sorted first.
		 */
		std::sort(all.begin(), all.end());
		std::sort(show.begin(), show.end());

		/*
		 * The IDs of the "do not show" filter are given by the
		 * difference between "all" Ids and the Ids of the "show only"
		 * filter.
		 */
		std::set_difference(all.begin(), all.end(),
				    show.begin(), show.end(),
				    std::inserter(diff, diff.begin()));

		negFilter(diff);
	}
}

/* Quiet warnings over documenting simple structures */
//! @cond Doxygen_Suppress

#define LAMDA_FILTER(method) [=] (QVector<int> vec) {method(vec);}

//! @endcond

void KsMainWindow::_showEvents()
{
	kshark_context *kshark_ctx(nullptr);
	KsCheckBoxWidget *events_cb;
	KsCheckBoxDialog *dialog;
	QVector<bool> v;

	if (!kshark_instance(&kshark_ctx))
		return;

	events_cb = new KsEventsCheckBoxWidget(_data.tep(), this);
	dialog = new KsCheckBoxDialog(events_cb, this);
	_presetCBWidget(kshark_ctx->show_event_filter,
		        kshark_ctx->hide_event_filter,
		        events_cb);

	auto lamFilter = [=] (QVector<int> show) {
		QVector<int> all = KsUtils::getEventIdList();
		_applyFilter(all, show,
			     LAMDA_FILTER(_data.applyPosEventFilter),
			     LAMDA_FILTER(_data.applyNegEventFilter));
	};

	connect(dialog,		&KsCheckBoxDialog::apply, lamFilter);

	dialog->show();
}

void KsMainWindow::_showTasks()
{
	kshark_context *kshark_ctx(nullptr);
	KsCheckBoxWidget *tasks_cbd;
	KsCheckBoxDialog *dialog;
	QVector<bool> v;

	if (!kshark_instance(&kshark_ctx))
		return;

	tasks_cbd = new KsTasksCheckBoxWidget(_data.tep(), true, this);
	dialog = new KsCheckBoxDialog(tasks_cbd, this);
	_presetCBWidget(kshark_ctx->show_task_filter,
			kshark_ctx->hide_task_filter,
			tasks_cbd);

	auto lamFilter = [=] (QVector<int> show) {
		QVector<int> all = KsUtils::getEventIdList();
		_applyFilter(all, show,
			     LAMDA_FILTER(_data.applyPosTaskFilter),
			     LAMDA_FILTER(_data.applyNegTaskFilter));
	};

	connect(dialog,		&KsCheckBoxDialog::apply, lamFilter);

	dialog->show();
}

void KsMainWindow::_showCPUs()
{
	kshark_context *kshark_ctx(nullptr);
	KsCheckBoxWidget *cpu_cbd;
	KsCheckBoxDialog *dialog;
	QVector<bool> v;

	if (!kshark_instance(&kshark_ctx))
		return;

	cpu_cbd = new KsCPUCheckBoxWidget(_data.tep(), this);
	dialog = new KsCheckBoxDialog(cpu_cbd, this);
	_presetCBWidget(kshark_ctx->show_cpu_filter,
			kshark_ctx->hide_cpu_filter,
			cpu_cbd);

	auto lamFilter = [=] (QVector<int> show) {
		QVector<int> all = KsUtils::getEventIdList();
		_applyFilter(all, show,
			     LAMDA_FILTER(_data.applyPosCPUFilter),
			     LAMDA_FILTER(_data.applyNegCPUFilter));
	};

	connect(dialog,		&KsCheckBoxDialog::apply, lamFilter);

	dialog->show();
}

void KsMainWindow::_advancedFiltering()
{
	KsAdvFilteringDialog *dialog;

	if (!_data.tep()) {
		QErrorMessage *em = new QErrorMessage(this);
		QString text("Unable to open Advanced filtering dialog.");

		text += " Tracing data has to be loaded first.";

		em->showMessage(text, "advancedFiltering");
		qCritical() << "ERROR: " << text;

		return;
	}

	dialog = new KsAdvFilteringDialog(this);
	connect(dialog,		&KsAdvFilteringDialog::dataReload,
		&_data,		&KsDataStore::reload);

	dialog->show();
}

void KsMainWindow::_clearFilters()
{
	_data.clearAllFilters();
}

void KsMainWindow::_cpuSelect()
{
	KsCheckBoxWidget *cpus_cbd = new KsCPUCheckBoxWidget(_data.tep(), this);
	KsCheckBoxDialog *dialog = new KsCheckBoxDialog(cpus_cbd, this);

	if(_data.tep()) {
		int nCPUs = tep_get_cpus(_data.tep());
		if (nCPUs == _graph.glPtr()->cpuGraphCount()) {
			cpus_cbd->setDefault(true);
		} else {
			QVector<bool> v(nCPUs, false);

			for (auto const &cpu: _graph.glPtr()->_cpuList)
				v[cpu] = true;

			cpus_cbd->set(v);
		}
	}

	connect(dialog,		&KsCheckBoxDialog::apply,
		&_graph,	&KsTraceGraph::cpuReDraw);

	dialog->show();
}

void KsMainWindow::_taskSelect()
{
	KsCheckBoxWidget *tasks_cbd = new KsTasksCheckBoxWidget(_data.tep(),
								true,
								this);
	KsCheckBoxDialog *dialog = new KsCheckBoxDialog(tasks_cbd, this);
	QVector<int> pids = KsUtils::getPidList();
	int nPids = pids.count();

	if (nPids == _graph.glPtr()->taskGraphCount()) {
		tasks_cbd->setDefault(true);
	} else {
		QVector<bool> v(nPids, false);
		for (int i = 0; i < nPids; ++i) {
			for (auto const &pid: _graph.glPtr()->_taskList) {
				if (pids[i] == pid) {
					v[i] = true;
					break;
				}
			}
		}

		tasks_cbd->set(v);
	}

	connect(dialog,		&KsCheckBoxDialog::apply,
		&_graph,	&KsTraceGraph::taskReDraw);

	dialog->show();
}

void KsMainWindow::_pluginSelect()
{
	KsCheckBoxWidget *plugin_cbd;
	KsCheckBoxDialog *dialog;
	QVector<bool> registeredPlugins;
	QStringList plugins;

	plugins << _plugins._ksPluginList << _plugins._userPluginList;

	registeredPlugins << _plugins._registeredKsPlugins
			  << _plugins._registeredUserPlugins;

	plugin_cbd = new KsPluginCheckBoxWidget(plugins, this);
	plugin_cbd->set(registeredPlugins);

	dialog = new KsCheckBoxDialog(plugin_cbd, this);

	connect(dialog,		&KsCheckBoxDialog::apply,
		&_plugins,	&KsPluginManager::updatePlugins);

	dialog->show();
}

void KsMainWindow::_pluginAdd()
{
	QStringList fileNames;

	fileNames = KsUtils::getFiles(this, "Add KernelShark plugins",
				     "KernelShark Plugins (*.so);;",
				     _lastPluginFilePath);

	if (!fileNames.isEmpty())
		_plugins.addPlugins(fileNames);
}

void KsMainWindow::_record()
{
	bool canDoAsRoot(false);

#ifdef DO_AS_ROOT
	canDoAsRoot = true;
#endif

	if (geteuid() && !canDoAsRoot) {
		QString message;

		message = "Record is currently not supported.";
		message += " Install \"pkexec\" and then do:<br>";
		message += " cd build <br> sudo ./cmake_uninstall.sh <br>";
		message += " ./cmake_clean.sh <br> cmake .. <br> make <br>";
		message += " sudo make install";

		_error(message, "recordCantStart", false, false);
		return;
	}

	_capture.start();
}

void KsMainWindow::_setColorPhase(int f)
{
	KsPlot::Color::setRainbowFrequency(f / 100.);
	_graph.glPtr()->loadColors();
	_graph.glPtr()->model()->update();
}

void KsMainWindow::_changeScreenMode()
{
	if (isFullScreen()) {
		_fullScreenModeAction.setText("Full Screen Mode");
		_fullScreenModeAction.setIcon(QIcon::fromTheme("view-fullscreen"));
		showNormal();
	} else {
		_fullScreenModeAction.setText("Exit Full Screen Mode");
		_fullScreenModeAction.setIcon(QIcon::fromTheme("view-restore"));
		showFullScreen();
	}
}

void KsMainWindow::_aboutInfo()
{
	KsMessageDialog *message;
	QString text;

	text.append(" KernelShark\n\n version: ");
	text.append(KS_VERSION_STRING);
	text.append("\n");

	message = new KsMessageDialog(text);
	message->setWindowTitle("About");
	message->show();
}

void KsMainWindow::_contents()
{
	QDesktopServices::openUrl(QUrl("http://kernelshark.org/",
				  QUrl::TolerantMode));
}

void KsMainWindow::_bugReport()
{
	QUrl bugs("https://bugzilla.kernel.org/buglist.cgi?component=Trace-cmd%2FKernelshark&product=Tools&resolution=---",
		  QUrl::TolerantMode);

	QDesktopServices::openUrl(bugs);
}

/** Load trace data for file. */
void KsMainWindow::loadDataFile(const QString& fileName)
{
	char buff[FILENAME_MAX];
	QString pbLabel("Loading    ");
	bool loadDone = false;
	struct stat st;
	int ret;

	ret = stat(fileName.toStdString().c_str(), &st);
	if (ret != 0) {
		QString text("Unable to find file ");

		text.append(fileName);
		text.append(".");
		_error(text, "loadDataErr1", true, true);

		return;
	}

	qInfo() << "Loading " << fileName;

	_mState.reset();
	_view.reset();
	_graph.reset();

	if (fileName.size() < 40) {
		pbLabel += fileName;
	} else {
		pbLabel += "...";
		pbLabel += fileName.mid(fileName.size() - 37, 37);
	}

	setWindowTitle("Kernel Shark");
	KsProgressBar pb(pbLabel);
	QApplication::processEvents();

	auto lamLoadJob = [&](KsDataStore *d) {
		d->loadDataFile(fileName);
		loadDone = true;
	};
	std::thread tload(lamLoadJob, &_data);

	for (int i = 0; i < 160; ++i) {
		/*
		 * TODO: The way this progress bar gets updated here is a pure
		 * cheat. See if this can be implemented better.
		*/
		if (loadDone)
			break;

		pb.setValue(i);
		usleep(150000);
	}

	tload.join();

	if (_data.size() < 1) {
		QString text("No data was loaded from file ");

		text.append(fileName + ".");
		_error(text, "loadDataErr2", true, true);

		return;
	}

	pb.setValue(165);
	_view.loadData(&_data);

	pb.setValue(180);
	_graph.loadData(&_data);
	pb.setValue(195);
	setWindowTitle("Kernel Shark (" + fileName + ")");

	if (realpath(fileName.toStdString().c_str(), buff)) {
		QString path(buff);
		_session.saveDataFile(path);
	}
}

void KsMainWindow::_error(const QString &mesg, const QString &errCode,
			  bool resize, bool unloadPlugins)
{
	QErrorMessage *em = new QErrorMessage(this);
	QString text = mesg;
	QString html = mesg;

	if (resize)
		_resizeEmpty();

	if (unloadPlugins)
		_plugins.unloadAll();

	text.replace("<br>", "\n", Qt::CaseInsensitive);
	html.replace("\n", "<br>", Qt::CaseInsensitive);

	qCritical().noquote() << "ERROR: " << text;
	em->showMessage(html, errCode);
	em->exec();
}

/**
 * @brief Load user session.
 *
 * @param fileName: Json file containing the description of the session.
 */
void KsMainWindow::loadSession(const QString &fileName)
{
	kshark_context *kshark_ctx(nullptr);
	struct stat st;
	int ret;

	if (!kshark_instance(&kshark_ctx))
		return;

	ret = stat(fileName.toStdString().c_str(), &st);
	if (ret != 0) {
		QString text("Unable to find session file ");

		text.append(fileName);
		text.append("\n");
		_error(text, "loadSessErr0", true, true);

		return;
	}

	if (!_session.importFromFile(fileName)) {
		QString text("Unable to open session description file ");

		text.append(fileName);
		text.append(".\n");
		_error(text, "loadSessErr1", true, true);

		return;
	}

	_session.loadPlugins(kshark_ctx, &_plugins);

	QString dataFile(_session.getDataFile(kshark_ctx));
	if (dataFile.isEmpty()) {
		QString text("Unable to open trace data file for session ");

		text.append(fileName);
		text.append("\n");
		_error(text, "loadSessErr1", true, true);

		return;
	}

	loadDataFile(dataFile);
	if (!_data.tep()) {
		_plugins.unloadAll();
		return;
	}

	KsProgressBar pb("Loading session settings ...");
	pb.setValue(10);

	_session.loadGraphs(&_graph);
	pb.setValue(20);

	_session.loadFilters(kshark_ctx, &_data);
	_filterSyncCBoxUpdate(kshark_ctx);
	pb.setValue(130);

	_session.loadSplitterSize(&_splitter);
	_session.loadMainWindowSize(this);
	this->show();
	pb.setValue(140);

	_session.loadDualMarker(&_mState, &_graph);
	_session.loadVisModel(_graph.glPtr()->model());
	_mState.updateMarkers(_data, _graph.glPtr());
	pb.setValue(170);

	_session.loadTable(&_view);
	_colorPhaseSlider.setValue(_session.getColorScheme() * 100);
	_graph.updateGeom();
}

void KsMainWindow::_initCapture()
{
	bool canDoAsRoot(false);

#ifdef DO_AS_ROOT
	canDoAsRoot = true;
#endif

	if (geteuid() && !canDoAsRoot)
		return;

	if (geteuid()) {
		_capture.setProgram("kshark-su-record");
	} else {
		QStringList argv;

		_capture.setProgram("kshark-record");
		argv << QString("-o") << QDir::homePath() + "/trace.dat";
		_capture.setArguments(argv);
	}

	connect(&_capture,	&QProcess::started,
		this,		&KsMainWindow::_captureStarted);

	/*
	 * Using the old Signal-Slot syntax because QProcess::finished has
	 * overloads.
	 */
	connect(&_capture,	SIGNAL(finished(int, QProcess::ExitStatus)),
		this,		SLOT(_captureFinished(int, QProcess::ExitStatus)));

	_captureErrorConnection =
		connect(&_capture,	&QProcess::errorOccurred,
			this,		&KsMainWindow::_captureError);

	connect(&_captureLocalServer,	&QLocalServer::newConnection,
		this,			&KsMainWindow::_readSocket);

}

void KsMainWindow::_captureStarted()
{
	_captureLocalServer.listen("KSCapture");
}

/**
 * If the authorization could not be obtained because the user dismissed
 * the authentication dialog (clicked Cancel), pkexec exits with a return
 * value of 126.
 */
#define PKEXEC_DISMISS_RET	126

void KsMainWindow::_captureFinished(int ret, QProcess::ExitStatus st)
{
	QProcess *capture = (QProcess *)sender();

	_captureLocalServer.close();

	if (ret == PKEXEC_DISMISS_RET) {
		/*
		 * Authorization could not be obtained because the user
		 * dismissed the authentication dialog.
		 */
		return;
	}

	if (ret != 0 && st == QProcess::NormalExit)
		_captureErrorMessage(capture);
}

void KsMainWindow::_captureError(QProcess::ProcessError error)
{
	QProcess *capture = (QProcess *)sender();
	_captureErrorMessage(capture);
}

void KsMainWindow::_captureErrorMessage(QProcess *capture)
{
	QString message = "Capture process failed: ";

	message += capture->errorString();
	message += "<br>Standard Error: ";
	message += capture->readAllStandardError();
	_error(message, "captureFinishedErr", false, false);
}

void KsMainWindow::_readSocket()
{
	QLocalSocket *socket;
	quint32 blockSize;
	QString fileName;

	auto lamSocketError = [&](QString message)
	{
		message = "ERROR from Local Server: " + message;
		_error(message, "readSocketErr", false, false);
	};

	socket = _captureLocalServer.nextPendingConnection();
	if (!socket) {
		lamSocketError("Pending connectio not found!");
		return;
	}

	QDataStream in(socket);
	socket->waitForReadyRead();
	if (socket->bytesAvailable() < (int)sizeof(quint32)) {
		lamSocketError("Message size is corrupted!");
		return;
	};

	in >> blockSize;
	if (socket->bytesAvailable() < blockSize || in.atEnd()) {
		lamSocketError("Message is corrupted!");
		return;
	}

	in >> fileName;
	loadDataFile(fileName);
}

void KsMainWindow::_splitterMoved(int pos, int index)
{
	_session.saveSplitterSize(_splitter);
}

void KsMainWindow::_deselectActive()
{
	_view.clearSelection();
	_mState.activeMarker().remove();
	_mState.updateLabels();
	_graph.glPtr()->model()->update();
}

void KsMainWindow::_deselectA()
{
	if (_mState.getState() == DualMarkerState::A)
		_view.clearSelection();
	else
		_view.passiveMarkerSelectRow(KS_NO_ROW_SELECTED);

	_mState.markerA().remove();
	_mState.updateLabels();
	_graph.glPtr()->model()->update();
}

void KsMainWindow::_deselectB()
{
	if (_mState.getState() == DualMarkerState::B)
		_view.clearSelection();
	else
		_view.passiveMarkerSelectRow(KS_NO_ROW_SELECTED);

	_mState.markerB().remove();
	_mState.updateLabels();
	_graph.glPtr()->model()->update();
}

void KsMainWindow::_rootWarning()
{
	QString cbFlag("noRootWarn");

	if (_settings.value(cbFlag).toBool())
		return;

	QMessageBox warn;
	warn.setText("KernelShark is running with Root privileges.");
	warn.setInformativeText("Continue at your own risk.");
	warn.setIcon(QMessageBox::Warning);
	warn.setStandardButtons(QMessageBox::Close);

	QCheckBox cb("Don't show this message again.");

	auto lamCbChec = [&] (int state) {
		if (state)
			_settings.setValue(cbFlag, true);
	};

	connect(&cb, &QCheckBox::stateChanged, lamCbChec);
	warn.setCheckBox(&cb);
	warn.exec();
}
