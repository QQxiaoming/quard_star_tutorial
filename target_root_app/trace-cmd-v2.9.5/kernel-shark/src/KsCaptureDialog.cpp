// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsCaptureDialog.cpp
 *  @brief   Dialog for trace data recording.
 */

// Qt
#include <QLocalSocket>

// KernelShark
#include "libkshark.h"
#include "KsUtils.hpp"
#include "KsCmakeDef.hpp"
#include "KsCaptureDialog.hpp"

extern "C" {
  // To get access to geteuid()
  #include <unistd.h>
  #include <sys/types.h>
}

static inline tep_handle *local_events()
{
	return tracefs_local_events(tracefs_tracing_dir());
}

/**
 * @brief Create KsCommandLineEdit.
 *
 * @param text: Defaulst text to be shown.
 * @param parent: The parent of this widget.
 */
KsCommandLineEdit::KsCommandLineEdit(QString text, QWidget *parent)
: QPlainTextEdit(text, parent) {}

QSize KsCommandLineEdit::minimumSizeHint() const
{
	return {FONT_WIDTH * 30, FONT_HEIGHT * 2};
}

QSize KsCommandLineEdit::sizeHint() const
{
	return {FONT_WIDTH * 30, FONT_HEIGHT * 3};
}

/** @brief Create KsCaptureControl widget. */
KsCaptureControl::KsCaptureControl(QWidget *parent)
: QWidget(parent),
  _localTEP(local_events()),
  _eventsWidget(_localTEP, this),
  _pluginsLabel("Plugin: ", this),
  _outputLabel("Output file: ", this),
  _commandLabel("Command: ", this),
  _outputLineEdit("trace.dat", this),
  _commandLineEdit("sleep 0.1", this),
  _settingsToolBar(this),
  _controlToolBar(this),
  _pluginsComboBox(this),
  _importSettingsButton("Import Settings", this),
  _exportSettingsButton("Export Settings", this),
  _outputBrowseButton("Browse", this),
  _commandCheckBox("Display output", this),
  _captureButton("Capture", &_controlToolBar),
  _applyButton("Apply", &_controlToolBar),
  _closeButton("Close", &_controlToolBar)
{
	QStringList pluginList = _getPlugins();
	int row(0);

	auto lamAddLine = [&] {
		QFrame* line = new QFrame();

		line->setFrameShape(QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);
		_topLayout.addWidget(line);
	};

	if (pluginList.count() == 0 || !_localTEP) {
		/*
		 * No plugins or events have been found. Most likely this is
		 * because the process has no Root privileges or because
		 * tracefs cannot be mounted.
		 */
		QString message("Error: No events or plugins found.\n");

		if (!_localTEP)
			message += "Cannot find or mount tracing directory.\n";

		// geteuid() returns 0 if running as effective id of root
		if (!pluginList.count() && geteuid())
			message += "Root privileges are required.\n";

		QLabel *errorLabel = new QLabel(message);

		errorLabel->setStyleSheet("QLabel {color : red;}");
		_topLayout.addWidget(errorLabel);

		lamAddLine();

		/* Disable "Capture" buttons. */
		_captureButton.setDisabled(true);
	}

	pluginList.prepend("nop");

	_settingsToolBar.addWidget(&_importSettingsButton);
	_settingsToolBar.addSeparator();
	_settingsToolBar.addWidget(&_exportSettingsButton);
	_topLayout.addWidget(&_settingsToolBar);

	lamAddLine();

	_eventsWidget.setDefault(false);
	_eventsWidget.setMinimumHeight(25 * FONT_HEIGHT);
	_eventsWidget.removeSystem("ftrace");
	_topLayout.addWidget(&_eventsWidget);

	_pluginsLabel.adjustSize();
	_execLayout.addWidget(&_pluginsLabel, row, 0);

	_pluginsComboBox.addItems(pluginList);
	_execLayout.addWidget(&_pluginsComboBox, row++, 1);

	_outputLabel.adjustSize();
	_execLayout.addWidget(&_outputLabel, row, 0);
	_outputLineEdit.setFixedWidth(FONT_WIDTH * 30);
	_execLayout.addWidget(&_outputLineEdit, row, 1);
	_outputBrowseButton.adjustSize();
	_execLayout.addWidget(&_outputBrowseButton, row++, 2);

	_commandLabel.adjustSize();
	_commandLabel.setFixedWidth(_outputLabel.width());
	_execLayout.addWidget(&_commandLabel, row, 0);

	_commandLineEdit.setFixedWidth(FONT_WIDTH * 30);
	_commandLineEdit.setMinimumHeight(FONT_HEIGHT * 2);
	_commandLineEdit.setMaximumHeight(FONT_HEIGHT * 9);

	_execLayout.addWidget(&_commandLineEdit, row, 1);
	_commandCheckBox.setCheckState(Qt::Unchecked);
	_commandCheckBox.adjustSize();
	_execLayout.addWidget(&_commandCheckBox, row++, 2);

	_topLayout.addLayout(&_execLayout);

	lamAddLine();

	_captureButton.setFixedWidth(STRING_WIDTH("_Capture_") + FONT_WIDTH * 2);
	_applyButton.setFixedWidth(_captureButton.width());
	_closeButton.setFixedWidth(_captureButton.width());

	_controlToolBar.addWidget(&_captureButton);
	_controlToolBar.addWidget(&_applyButton);
	_controlToolBar.addWidget(&_closeButton);
	_topLayout.addWidget(&_controlToolBar);

	setLayout(&_topLayout);

	connect(&_importSettingsButton,	&QPushButton::pressed,
		this,			&KsCaptureControl::_importSettings);

	connect(&_exportSettingsButton,	&QPushButton::pressed,
		this,			&KsCaptureControl::_exportSettings);

	connect(&_outputBrowseButton,	&QPushButton::pressed,
		this,			&KsCaptureControl::_browse);

	connect(&_applyButton,		&QPushButton::pressed,
		this,			&KsCaptureControl::_apply);
}

/**
 * Use the settings of the Control panel to generate a list of command line
 * arguments for trace-cmd.
 */
QStringList KsCaptureControl::getArgs()
{
	QStringList argv;

	argv << "record";

	if (_pluginsComboBox.currentText() != "nop")
		argv << "-p" << _pluginsComboBox.currentText();

	if (_eventsWidget.all())
		argv << "-e" << "all";
	else
		argv << _eventsWidget.getCheckedEvents(true);

	argv << "-o" << outputFileName();

	argv << KsUtils::splitArguments(_commandLineEdit.toPlainText());

	return argv;
}

QStringList KsCaptureControl::_getPlugins()
{
	QStringList pluginList;
	char **all_plugins;

	all_plugins = tracefs_tracers(tracefs_tracing_dir());

	if (!all_plugins)
		return pluginList;

	for (int i = 0; all_plugins[i]; ++i) {
		/*
		 * TODO plugin selection here.
		 * printf("plugin %i %s\n", i, all_plugins[i]);
		 */
		pluginList << all_plugins[i];
		free(all_plugins[i]);
	}

	free (all_plugins);
	std::sort(pluginList.begin(), pluginList.end());

	return pluginList;
}

void KsCaptureControl::_importSettings()
{
	int nEvts = tep_get_events_count(_localTEP), nIds;
	kshark_config_doc *conf, *jevents, *temp;
	QVector<bool> v(nEvts, false);
	tracecmd_filter_id *eventHash;
	QVector<int> eventIds;
	QString fileName;

	auto lamImportError = [this] () {
		emit print("ERROR: Unable to load the configuration file.\n");
	};

	/** Get all available events. */
	eventIds = KsUtils::getEventIdList(TEP_EVENT_SORT_SYSTEM);

	/* Get the configuration document. */
	fileName = KsUtils::getFile(this, "Import from Filter",
				    "Kernel Shark Config files (*.json);;",
				    _lastFilePath);

	if (fileName.isEmpty()) {
		lamImportError();
		return;
	}

	conf = kshark_open_config_file(fileName.toStdString().c_str(),
				       "kshark.config.record");
	if (!conf) {
		lamImportError();
		return;
	}

	/*
	 * Load the hash table of selected events from the configuration
	 * document.
	 */
	jevents = kshark_config_alloc(KS_CONFIG_JSON);
	if (!kshark_config_doc_get(conf, "Events", jevents)) {
		lamImportError();
		return;
	}

	eventHash = tracecmd_filter_id_hash_alloc();
	nIds = kshark_import_event_filter(_localTEP, eventHash, "Events", jevents);
	if (nIds < 0) {
		QString err("WARNING: ");
		err += "Some of the imported events are not available on this system.\n";
		err += "All missing events are ignored.\n";
		emit print(err);
	}

	for (int i = 0; i < nEvts; ++i) {
		if (tracecmd_filter_id_find(eventHash, eventIds[i]))
			v[i] = true;
	}

	_eventsWidget.set(v);
	tracecmd_filter_id_hash_free(eventHash);

	/** Get all available plugins. */
	temp = kshark_string_config_alloc();

	if (kshark_config_doc_get(conf, "Plugin", temp)) {
		const char *plugin = KS_C_STR_CAST(temp->conf_doc);
		int pluginIndex = _pluginsComboBox.findText(plugin);

		if (pluginIndex >= 0) {
			_pluginsComboBox.setCurrentText(KS_C_STR_CAST(temp->conf_doc));
		} else {
			QString err("WARNING: The traceer plugin \"");
			err += plugin;
			err += "\" is not available on this machine\n";
			emit print(err);
		}
	}

	if (kshark_config_doc_get(conf, "Output", temp))
		_outputLineEdit.setText(KS_C_STR_CAST(temp->conf_doc));

	if (kshark_config_doc_get(conf, "Command", temp))
		_commandLineEdit.setPlainText(KS_C_STR_CAST(temp->conf_doc));
}

void KsCaptureControl::_exportSettings()
{
	kshark_config_doc *conf, *events;
	json_object *jplugin;
	QString plugin, out, comm;
	QVector<int> ids;
	QString fileName;

	fileName = KsUtils::getSaveFile(this, "Export to File",
					"Kernel Shark Config files (*.json);;",
					".json",
					_lastFilePath);

	if (fileName.isEmpty())
		return;

	/* Create a configuration document. */
	conf = kshark_record_config_new(KS_CONFIG_JSON);
	events = kshark_filter_config_new(KS_CONFIG_JSON);

	/*
	 * Use the tracecmd_filter_id to save all selected events in the
	 * configuration file.
	 */
	ids = _eventsWidget.getCheckedIds();
	tracecmd_filter_id *eventHash = tracecmd_filter_id_hash_alloc();
	for (auto const &id: ids)
		tracecmd_filter_id_add(eventHash, id);

	kshark_export_event_filter(_localTEP, eventHash, "Events", events);
	kshark_config_doc_add(conf, "Events", events);

	tracecmd_filter_id_hash_free(eventHash);

	/* Save the plugin. */
	plugin = _pluginsComboBox.currentText();
	jplugin = json_object_new_string(plugin.toStdString().c_str());
	kshark_config_doc_add(conf, "Plugin", kshark_json_to_conf(jplugin));

	/* Save the output file. */
	out = outputFileName();
	json_object *jout = json_object_new_string(out.toStdString().c_str());
	kshark_config_doc_add(conf, "Output", kshark_json_to_conf(jout));

	/* Save the command. */
	comm = _commandLineEdit.toPlainText();
	json_object *jcomm = json_object_new_string(comm.toStdString().c_str());
	kshark_config_doc_add(conf, "Command", kshark_json_to_conf(jcomm));

	kshark_save_config_file(fileName.toStdString().c_str(), conf);
}

void KsCaptureControl::_browse()
{
	QString fileName =
		KsUtils::getSaveFile(this, "Save File",
				     "trace-cmd files (*.dat);;All files (*)",
				     ".dat",
				     _lastFilePath);

	if (!fileName.isEmpty())
		_outputLineEdit.setText(fileName);
}

void KsCaptureControl::_apply()
{
	QStringList argv = getArgs();

	if (argv.count())
		emit argsReady(argv.join(" "));
}

/** @brief Create KsCaptureMonitor widget. */
KsCaptureMonitor::KsCaptureMonitor(QWidget *parent)
: QWidget(parent),
  _mergedChannels(false),
  _argsModified(false),
  _panel(this),
  _name("Output display", this),
  _space("max size ", this),
  _readOnlyCB("read only", this),
  _maxLinNumEdit(QString("%1").arg(KS_CAP_MON_MAX_LINE_NUM), this),
  _consolOutput("", this)
{
	_panel.setMaximumHeight(FONT_HEIGHT * 1.75);
	_panel.addWidget(&_name);

	_space.setAlignment(Qt::AlignRight);
	_panel.addWidget(&_space);

	_maxLinNumEdit.setFixedWidth(FONT_WIDTH * 7);
	_panel.addWidget(&_maxLinNumEdit);
	_panel.addSeparator();
	_readOnlyCB.setCheckState(Qt::Checked);
	_panel.addWidget(&_readOnlyCB);
	_layout.addWidget(&_panel);

	_consolOutput.setStyleSheet("QLabel {background-color : white;}");
	_consolOutput.setMinimumWidth(FONT_WIDTH * 60);
	_consolOutput.setMinimumHeight(FONT_HEIGHT * 10);
	_consolOutput.setMaximumBlockCount(KS_CAP_MON_MAX_LINE_NUM);

	_space.setMinimumWidth(FONT_WIDTH * 50 - _name.width() - _readOnlyCB.width());
	_consolOutput.setReadOnly(true);
	_layout.addWidget(&_consolOutput);

	this->setLayout(&_layout);

	connect(&_maxLinNumEdit,	&QLineEdit::textChanged,
		this,			&KsCaptureMonitor::_maxLineNumber);

	connect(&_readOnlyCB,		&QCheckBox::stateChanged,
		this,			&KsCaptureMonitor::_readOnly);

	connect(&_consolOutput,		&QPlainTextEdit::textChanged,
		this,			&KsCaptureMonitor::_argVModified);

	this->show();
}

void KsCaptureMonitor::_maxLineNumber(const QString &test)
{
	bool ok;
	int max = test.toInt(&ok);

	if (ok)
		_consolOutput.setMaximumBlockCount(max);
}

void KsCaptureMonitor::_readOnly(int state)
{
	if (state == Qt::Checked)
		_consolOutput.setReadOnly(true);
	else
		_consolOutput.setReadOnly(false);
}

void KsCaptureMonitor::_argsReady(const QString &args)
{
	_name.setText("Capture options:");
	_consolOutput.setPlainText(args);
	_argsModified = false;
}

void KsCaptureMonitor::_argVModified()
{
	_argsModified = true;
}

void KsCaptureMonitor::_printAllStandardError()
{
	QProcess *_capture = (QProcess*) sender();

	_consolOutput.moveCursor(QTextCursor::End);
	_consolOutput.insertPlainText(_capture->readAllStandardError());
	_consolOutput.moveCursor(QTextCursor::End);
	QCoreApplication::processEvents();
}

void KsCaptureMonitor::_printAllStandardOutput()
{
	QProcess *_capture = (QProcess*) sender();

	if (!_mergedChannels)
		return;

	_consolOutput.appendPlainText(_capture->readAllStandardOutput());
	QCoreApplication::processEvents();
}

/**
 * Connect the Capture monitor widget to the signals of the recording process.
 */
void KsCaptureMonitor::connectMe(QProcess *proc, KsCaptureControl *ctrl)
{
	connect(proc,	&QProcess::started,
		this,	&KsCaptureMonitor::_captureStarted);

	/* Using the old Signal-Slot syntax because QProcess::finished has overloads. */
	connect(proc,	SIGNAL(finished(int, QProcess::ExitStatus)),
		this,	SLOT(_captureFinished(int, QProcess::ExitStatus)));

	connect(proc,	&QProcess::readyReadStandardError,
		this,	&KsCaptureMonitor::_printAllStandardError);

	connect(proc,	&QProcess::readyReadStandardOutput,
		this,	&KsCaptureMonitor::_printAllStandardOutput);

	connect(ctrl,	&KsCaptureControl::argsReady,
		this,	&KsCaptureMonitor::_argsReady);

	connect(ctrl,	&KsCaptureControl::print,
		this,	&KsCaptureMonitor::print);
}

void KsCaptureMonitor::_captureStarted()
{
	_name.setText("Terminal output:");
	_readOnlyCB.setCheckState(Qt::Checked);

	QCoreApplication::processEvents();
}

void KsCaptureMonitor::_captureFinished(int exit, QProcess::ExitStatus status)
{
	QProcess *_capture = (QProcess *)sender();

	if (exit != 0 || status != QProcess::NormalExit) {
		QString errMessage("Capture process failed: ");

		errMessage += _capture->errorString();
		_consolOutput.appendPlainText(errMessage);

		QCoreApplication::processEvents();
	}
}

/** Print a message. */
void KsCaptureMonitor::print(const QString &message)
{
	_consolOutput.appendPlainText(message);
}

/** @brief Create KsCaptureDialog widget. */
KsCaptureDialog::KsCaptureDialog(QWidget *parent)
: QWidget(parent),
  _captureCtrl(this),
  _captureMon(this),
  _captureProc(this)
{
	QString captureExe;

	this->setWindowTitle("Capture");
	_layout.addWidget(&_captureCtrl);
	_layout.addWidget(&_captureMon);
	this->setLayout(&_layout);

	connect(&_captureCtrl._commandCheckBox,	&QCheckBox::stateChanged,
		this,				&KsCaptureDialog::_setChannelMode);

	connect(&_captureCtrl._captureButton,	&QPushButton::pressed,
		this,				&KsCaptureDialog::_capture);

	connect(&_captureCtrl._closeButton,	&QPushButton::pressed,
		this,				&KsCaptureDialog::close);

	if (!KsUtils::isInstalled())
		captureExe = QString(_INSTALL_PREFIX) + QString("/bin/trace-cmd");
	else
		captureExe = TRACECMD_EXECUTABLE;

	_captureProc.setProgram(captureExe);

	_captureMon.connectMe(&_captureProc, &_captureCtrl);
}

void KsCaptureDialog::_capture()
{
	QStringList argv;
	int argc;

	if(_captureMon._argsModified) {
		argv = KsUtils::splitArguments(_captureMon.text());
	} else {
		argv = _captureCtrl.getArgs();
	}

	_captureMon.print("\n");
	_captureMon.print(QString("trace-cmd " + argv.join(" ") + "\n"));
	_captureProc.setArguments(argv);
	_captureProc.start();
	_captureProc.waitForFinished();

	/* Reset the _argsModified flag. */
	_captureMon._argsModified = false;

	if (_captureProc.exitCode() != 0 ||
	    _captureProc.exitStatus() != QProcess::NormalExit)
		return;

	/*
	 * Capture finished successfully. Open the produced tracing data file
	 * in KernelShark.
	 */
	argc = argv.count();
	for (int i = 0; i < argc; ++i) {
		if (argv[i] == "-o") {
			_sendOpenReq(argv[i + 1]);
			break;
		}
	}
}

void KsCaptureDialog::_setChannelMode(int state)
{
	if (state > 0) {
		_captureMon._mergedChannels = true;
	} else {
		_captureMon._mergedChannels = false;
	}
}

void KsCaptureDialog::_sendOpenReq(const QString &fileName)
{
	QLocalSocket *socket;

	socket = new QLocalSocket(this);
	socket->connectToServer("KSCapture", QIODevice::WriteOnly);
	if (socket->waitForConnected()) {
		QByteArray block;
		QDataStream out(&block, QIODevice::WriteOnly);
		const QString message = fileName;

		out << quint32(message.size());
		out << message;

		socket->write(block);
		socket->flush();
		socket->disconnectFromServer();
	} else {
		QString error(socket->errorString());

		error += "\n(maybe KernelShark GUI is not open)";
		_captureMon.print(error);
	}
}
