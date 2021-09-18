/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsCaptureDialog.hpp
 *  @brief   Dialog for trace data recording.
 */

#ifndef _KS_CAPTURE_H
#define _KS_CAPTURE_H

// Qt
#include <QtWidgets>

// KernelShark
#include "KsWidgetsLib.hpp"

/**
 * The KsCommandLineEdit class is used to override the default size hints of
 * the QPlainTextEdit class.
 */
struct KsCommandLineEdit : public QPlainTextEdit
{
	KsCommandLineEdit(QString text, QWidget *parent = 0);

private:
	QSize sizeHint() const override;

	QSize minimumSizeHint() const override;
};

/**
 * The KsCaptureControl class provides a control panel for the KernelShark
 * Capture dialog.
 */
class KsCaptureControl : public QWidget
{
	Q_OBJECT
public:
	explicit KsCaptureControl(QWidget *parent = 0);

	QStringList getArgs();

	/** Get the name of the tracing data output file. */
	QString outputFileName() const {return _outputLineEdit.text();}

	/** Set the name of the tracing data output file. */
	void setOutputFileName(const QString &f) {_outputLineEdit.setText(f);}

signals:
	/** This signal is emitted when the "Apply" button is pressed. */
	void argsReady(const QString &args);

	/**
	 * This signal is emitted when text has to be printed on the
	 * KsCaptureMonitor widget.
	 */
	void print(const QString &message);

private:
	tep_handle		*_localTEP;

	KsEventsCheckBoxWidget	_eventsWidget;

	QVBoxLayout	_topLayout;

	QGridLayout	_execLayout;

	QLabel		_pluginsLabel, _outputLabel, _commandLabel;

	QLineEdit	_outputLineEdit;

	KsCommandLineEdit	_commandLineEdit;

	QToolBar	_settingsToolBar, _controlToolBar;

	QComboBox	_pluginsComboBox;

	QPushButton	_importSettingsButton, _exportSettingsButton;

	QPushButton	_outputBrowseButton;

	QString		_lastFilePath;

	QStringList _getPlugins();

	void _importSettings();

	void _exportSettings();

	void _browse();

	void _apply();

public:
	/**
	 * A Check box used to indicate if the output of the command needs to
	 * be shown by the KsCaptureMonitor widget.
	 */
	QCheckBox	_commandCheckBox;

	/** Capture button for the control panel. */
	QPushButton	_captureButton;

	/** Apply button for the control panel. */
	QPushButton	_applyButton;

	/** Close button for the control panel. */
	QPushButton	_closeButton;
};

/**
 * The KsCaptureMonitor class provides a terminal-like widget for monitoring
 * the tracing data recording process.
 */
class KsCaptureMonitor : public QWidget
{
	Q_OBJECT
public:
	explicit KsCaptureMonitor(QWidget *parent = 0);

	/** Get the text shown by the widget. */
	QString text() const {return _consolOutput.toPlainText();}

	/** Clear the text shown by the widget. */
	void clear() {_consolOutput.clear();}

	void print(const QString &message);

	void connectMe(QProcess *proc, KsCaptureControl *ctrl);

	/** A flag indicating if the stdout and stderr channels are _merged. */
	bool		_mergedChannels;

	/**
	 * A flag indicating, if the list of the command line arguments for trace-cmd
	 * has been edited by the user.
	 */
	bool		_argsModified;

private:
	QVBoxLayout	_layout;

	QToolBar	_panel;

	QLabel		_name, _space;

	QCheckBox	_readOnlyCB;

	QLineEdit	_maxLinNumEdit;

	QPlainTextEdit	_consolOutput;

	void _argsReady(const QString &test);

	void _maxLineNumber(const QString &test);

	void _readOnly(int);

	void _argVModified();

	void _captureStarted();

	void _printAllStandardError();

	void _printAllStandardOutput();

private slots:
	void _captureFinished(int, QProcess::ExitStatus);
};

/** Default number of lines shown by the KsCaptureMonitor widget. */
#define KS_CAP_MON_MAX_LINE_NUM 200

/**
 * The KsCaptureDialog class provides a dialog for recording of tracing data.
 */
class KsCaptureDialog : public QWidget
{
	Q_OBJECT
public:
	explicit KsCaptureDialog(QWidget *parent = 0);

	/** Set the name of the tracing data output file. */
	void setOutputFileName(const QString &f)
	{
		_captureCtrl.setOutputFileName(f);
	}

private:
	QHBoxLayout		_layout;

	KsCaptureControl	_captureCtrl;

	KsCaptureMonitor	_captureMon;

	QProcess		_captureProc;

	void _capture();

	void _setChannelMode(int state);

	void _sendOpenReq(const QString &fileName);
};

#endif // _KS_CAPTURE_H
