/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsAdvFilteringDialog.hpp
 *  @brief   GUI Dialog for Advanced filtering settings.
 */

#ifndef _KS_ADV_FILTERING_DIALOG_H
#define _KS_ADV_FILTERING_DIALOG_H

// Qt
#include <QtWidgets>

// KernelShark
#include "KsWidgetsLib.hpp"

/**
 * The KsAdvFilteringDialog class provides a dialog for Advanced filtering.
 */
class KsAdvFilteringDialog : public QDialog
{
	Q_OBJECT
public:
	explicit KsAdvFilteringDialog(QWidget *parent = nullptr);

signals:
	/** Signal emitted when the _apply button of the dialog is pressed. */
	void dataReload();

private:
	int 			_noHelpHeight;

	QMap<int, QString>	_filters;

	KsCheckBoxTable		*_table;

	QVBoxLayout	_topLayout;

	QHBoxLayout	_buttonLayout;

	QToolBar	_condToolBar1, _condToolBar2, _condToolBar3;

	QLabel		_descrLabel, _sysEvLabel, _opsLabel, _fieldLabel;

	QComboBox	_systemComboBox, _eventComboBox;

	QComboBox	_opsComboBox, _fieldComboBox;

	QLineEdit	_filterEdit;

	QPushButton	_helpButton;

	QPushButton	_insertEvtButton, _insertOpButton, _insertFieldButton;

	QPushButton	_applyButton, _cancelButton;

	QMetaObject::Connection		_applyButtonConnection;

	void _help();

	void _applyPress();

	void _insertEvt();

	void _insertOperator();

	void _insertField();

	QString _description();

	QStringList _operators();

	void _getFilters(struct kshark_context *kshark_ctx);

	void _makeFilterTable(struct kshark_context *kshark_ctx);

	QStringList _getEventFormatFields(struct tep_event *event);

	void _setSystemCombo(struct kshark_context *kshark_ctx);

private slots:
	void _systemChanged(const QString&);

	void _eventChanged(const QString&);
};

#endif // _KS_ADV_FILTERING_DIALOG_H
