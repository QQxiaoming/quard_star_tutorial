// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsQuickContextMenu.cpp
 *  @brief   Quick Context Menus for KernelShark.
 */

#include "KsQuickContextMenu.hpp"
#include "KsTraceGraph.hpp"

/**
 * @brief Create KsQuickMarkerMenu.
 *
 * @param dm: The State machine of the Dual marker.
 * @param parent: The parent of this widget.
 */
KsQuickMarkerMenu::KsQuickMarkerMenu(KsDualMarkerSM *dm, QWidget *parent)
: QMenu("Context Menu", parent),
  _dm(dm),
  _deselectAction(this)
{
	if (dm->activeMarker()._isSet) {
		addSection("Marker menu");
		_deselectAction.setText("Deselect");
		_deselectAction.setShortcut(tr("Ctrl+D"));
		_deselectAction.setStatusTip(tr("Deselect marker"));

		connect(&_deselectAction,	&QAction::triggered,
			this,			&KsQuickMarkerMenu::deselect);

		addAction(&_deselectAction);
	}
}

/**
 * @brief Create KsQuickContextMenu.
 *
 * @param data: Input location for the KsDataStore object.
 * @param row: The index of the entry used to initialize the menu.
 * @param dm: The State machine of the Dual marker.
 * @param parent: The parent of this widget.
 */
KsQuickContextMenu::KsQuickContextMenu(KsDataStore *data, size_t row,
				       KsDualMarkerSM *dm,
				       QWidget *parent)
: KsQuickMarkerMenu(dm, parent),
  _data(data),
  _row(row),
  _graphSyncCBox(nullptr),
  _listSyncCBox(nullptr),
  _hideTaskAction(this),
  _showTaskAction(this),
  _hideEventAction(this),
  _showEventAction(this),
  _hideCPUAction(this),
  _showCPUAction(this),
  _addCPUPlotAction(this),
  _addTaskPlotAction(this),
  _removeCPUPlotAction(this),
  _removeTaskPlotAction(this),
  _clearAllFilters(this)
{
	typedef void (KsQuickContextMenu::*mfp)();
	QString taskName, parentName, descr;
	KsTraceGraph *graphs;
	int pid, cpu;

	if (!parent || !_data)
		return;

	taskName = kshark_get_task_easy(_data->rows()[_row]);
	pid = kshark_get_pid_easy(_data->rows()[_row]);
	cpu = _data->rows()[_row]->cpu;

	auto lamAddAction = [this, &descr] (QAction *action, mfp mf) {
		action->setText(descr);

		connect(action,	&QAction::triggered,
			this,	mf);

		addAction(action);
	};

	parentName = parent->metaObject()->className();

	addSection("Pointer filter menu");

	descr = "Show task [";
	descr += taskName;
	descr += "-";
	descr += QString("%1").arg(pid);
	descr += "] only";
	lamAddAction(&_showTaskAction, &KsQuickContextMenu::_showTask);

	descr = "Hide task [";
	descr += taskName;
	descr += "-";
	descr += QString("%1").arg(pid);
	descr += "]";
	lamAddAction(&_hideTaskAction, &KsQuickContextMenu::_hideTask);

	descr = "Show event [";
	descr += kshark_get_event_name_easy(_data->rows()[_row]);
	descr += "] only";
	lamAddAction(&_showEventAction, &KsQuickContextMenu::_showEvent);

	descr = "Hide event [";
	descr += kshark_get_event_name_easy(_data->rows()[_row]);
	descr += "]";
	lamAddAction(&_hideEventAction, &KsQuickContextMenu::_hideEvent);

	if (parentName == "KsTraceViewer") {
		descr = QString("Show CPU [%1] only").arg(cpu);
		lamAddAction(&_showCPUAction, &KsQuickContextMenu::_showCPU);
	}

	descr = QString("Hide CPU [%1]").arg(_data->rows()[_row]->cpu);
	lamAddAction(&_hideCPUAction, &KsQuickContextMenu::_hideCPU);

	descr = "Clear all filters";
	lamAddAction(&_clearAllFilters, &KsQuickContextMenu::_clearFilters);

	addSection("Pointer plot menu");

	if (parentName == "KsTraceViewer") {
		descr = "Add [";
		descr += taskName;
		descr += "-";
		descr += QString("%1").arg(pid);
		descr += "] plot";
		lamAddAction(&_addTaskPlotAction,
			     &KsQuickContextMenu::_addTaskPlot);
	}

	if (parentName == "KsTraceGraph" &&
	    (graphs = dynamic_cast<KsTraceGraph *>(parent))) {
		if (graphs->glPtr()->_taskList.contains(pid)) {
			descr = "Remove [";
			descr += taskName;
			descr += "-";
			descr += QString("%1").arg(pid);
			descr += "] plot";
			lamAddAction(&_removeTaskPlotAction,
				     &KsQuickContextMenu::_removeTaskPlot);
		} else {
			descr = "Add [";
			descr += taskName;
			descr += "-";
			descr += QString("%1").arg(pid);
			descr += "] plot";
			lamAddAction(&_addTaskPlotAction,
				     &KsQuickContextMenu::_addTaskPlot);
		}

		if (graphs->glPtr()->_cpuList.contains(cpu)) {
			descr = "Remove [CPU ";
			descr += QString("%1").arg(cpu);
			descr += "] plot";
			lamAddAction(&_removeCPUPlotAction,
				     &KsQuickContextMenu::_removeCPUPlot);
		} else {
			descr = "Add [CPU ";
			descr += QString("%1").arg(cpu);
			descr += "] plot";
			lamAddAction(&_addCPUPlotAction,
				     &KsQuickContextMenu::_addCPUPlot);
		}
	}
}

void KsQuickContextMenu::_hideTask()
{
	int pid = kshark_get_pid_easy(_data->rows()[_row]);
	kshark_context *kshark_ctx(nullptr);
	QVector<int> vec;

	if (!kshark_instance(&kshark_ctx))
		return;

	vec =_getFilterVector(kshark_ctx->hide_task_filter, pid);
	_data->applyNegTaskFilter(vec);
}

void KsQuickContextMenu::_showTask()
{
	int pid = kshark_get_pid_easy(_data->rows()[_row]);

	_data->applyPosTaskFilter(QVector<int>(1, pid));
}

void KsQuickContextMenu::_hideEvent()
{
	int eventId = kshark_get_event_id_easy(_data->rows()[_row]);
	kshark_context *kshark_ctx(nullptr);
	QVector<int> vec;

	if (!kshark_instance(&kshark_ctx))
		return;

	vec =_getFilterVector(kshark_ctx->hide_event_filter, eventId);
	_data->applyNegEventFilter(vec);
}

void KsQuickContextMenu::_showEvent()
{
	int eventId = kshark_get_event_id_easy(_data->rows()[_row]);

	_data->applyPosEventFilter(QVector<int>(1, eventId));
}

void KsQuickContextMenu::_showCPU()
{
	int cpu = _data->rows()[_row]->cpu;

	_data->applyPosCPUFilter(QVector<int>(1, cpu));
}

void KsQuickContextMenu::_hideCPU()
{
	kshark_context *kshark_ctx(nullptr);
	QVector<int> vec;

	if (!kshark_instance(&kshark_ctx))
		return;

	vec =_getFilterVector(kshark_ctx->hide_cpu_filter,
			      _data->rows()[_row]->cpu);

	_data->applyNegCPUFilter(vec);
}

QVector<int> KsQuickContextMenu::_getFilterVector(tracecmd_filter_id *filter, int newId)
{
	QVector<int> vec = KsUtils::getFilterIds(filter);
	if (!vec.contains(newId))
		vec.append(newId);

	return vec;
}

void KsQuickContextMenu::_addTaskPlot()
{
	int pid = kshark_get_pid_easy(_data->rows()[_row]);

	emit addTaskPlot(pid);
}

void KsQuickContextMenu::_addCPUPlot()
{
	emit addCPUPlot(_data->rows()[_row]->cpu);
}

void KsQuickContextMenu::_removeTaskPlot()
{
	int pid = kshark_get_pid_easy(_data->rows()[_row]);

	emit removeTaskPlot(pid);
}

void KsQuickContextMenu::_removeCPUPlot()
{
	emit removeCPUPlot(_data->rows()[_row]->cpu);
}

/**
 * @brief Create KsRmPlotContextMenu.
 *
 * @param dm: The State machine of the Dual marker.
 * @param parent: The parent of this widget.
 */
KsRmPlotContextMenu::KsRmPlotContextMenu(KsDualMarkerSM *dm,
					 QWidget *parent)
: KsQuickMarkerMenu(dm, parent),
  _removePlotAction(this)
{
	addSection("Plots");

	connect(&_removePlotAction,	&QAction::triggered,
		this,			&KsRmPlotContextMenu::removePlot);

	addAction(&_removePlotAction);
}

/**
 * @brief Create KsRmCPUPlotMenu.
 *
 * @param dm: The State machine of the Dual marker.
 * @param cpu : CPU Id.
 * @param parent: The parent of this widget.
 */
KsRmCPUPlotMenu::KsRmCPUPlotMenu(KsDualMarkerSM *dm, int cpu,
				 QWidget *parent)
: KsRmPlotContextMenu(dm, parent)
{
	_removePlotAction.setText(QString("Remove [CPU %1]").arg(cpu));
}

/**
 * @brief Create KsRmTaskPlotMenu.
 *
 * @param dm: The State machine of the Dual marker.
 * @param pid: Process Id.
 * @param parent: The parent of this widget.
 */
KsRmTaskPlotMenu::KsRmTaskPlotMenu(KsDualMarkerSM *dm, int pid,
				   QWidget *parent)
: KsRmPlotContextMenu(dm, parent)
{
	kshark_context *kshark_ctx(nullptr);
	QString descr("Remove [ ");

	if (!kshark_instance(&kshark_ctx))
		return;

	descr += tep_data_comm_from_pid(kshark_ctx->pevent, pid);
	descr += "-";
	descr += QString("%1").arg(pid);
	descr += "] plot";
	_removePlotAction.setText(descr);
}
