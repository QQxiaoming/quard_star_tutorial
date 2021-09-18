/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsQuickContextMenu.hpp
 *  @brief   Quick Context Menus for KernelShark.
 */

#ifndef _KS_QUICK_CTX_MENU_H
#define _KS_QUICK_CTX_MENU_H

#include "KsDualMarker.hpp"
#include "KsUtils.hpp"
#include "KsGLWidget.hpp"

/**
 * The KsQuickMarkerMenu class provides menu for quick Dual Marker related
 * actions.
 */
class KsQuickMarkerMenu : public QMenu {
	Q_OBJECT
public:
	KsQuickMarkerMenu(KsDualMarkerSM *dm, QWidget *parent = nullptr);

signals:
	/** Signal to deselect the active marker. */
	void deselect();

private:
	KsDualMarkerSM	*_dm;

	QAction _deselectAction;
};

/**
 * The KsQuickFilterMenu class provides a menu for easy filtering and plotting.
 * The menu is initialized from a single kshark_entry and uses the content of
 * this entry to provides quick actions for filtering and plottin.
 */
class KsQuickContextMenu : public KsQuickMarkerMenu {
	Q_OBJECT
public:
	KsQuickContextMenu() = delete;

	KsQuickContextMenu(KsDataStore *data, size_t row,
			   KsDualMarkerSM *dm,
			   QWidget *parent = nullptr);

signals:
	/** Signal to add a task plot. */
	void addTaskPlot(int);

	/** Signal to add a CPU plot. */
	void addCPUPlot(int);

	/** Signal to remove a task plot. */
	void removeTaskPlot(int);

	/** Signal to remove a CPU plot. */
	void removeCPUPlot(int);

private:
	void _hideTask();

	void _showTask();

	void _hideEvent();

	void _showEvent();

	void _showCPU();

	void _hideCPU();

	void _addCPUPlot();

	void _addTaskPlot();

	void _removeCPUPlot();

	void _removeTaskPlot();

	QVector<int> _getFilterVector(tracecmd_filter_id *filter, int newId);

	void _clearFilters() {_data->clearAllFilters();}

	KsDataStore	*_data;

	size_t		_row;

	QCheckBox	*_graphSyncCBox, *_listSyncCBox;

	QAction _hideTaskAction, _showTaskAction;

	QAction _hideEventAction, _showEventAction;

	QAction _hideCPUAction, _showCPUAction;

	QAction _addCPUPlotAction;

	QAction _addTaskPlotAction;

	QAction _removeCPUPlotAction;

	QAction _removeTaskPlotAction;

	QAction _clearAllFilters;
};

/**
 * The KsQuickMarkerMenu is a baser class for Remove Plot menus.
 */
class KsRmPlotContextMenu : public KsQuickMarkerMenu {
	Q_OBJECT
public:
	KsRmPlotContextMenu() = delete;

	KsRmPlotContextMenu(KsDualMarkerSM *dm, QWidget *parent = nullptr);

signals:
	/** Signal to remove a plot. */
	void removePlot(int);

protected:
	/** Menu action. */
	QAction _removePlotAction;
};

/**
 * The KsQuickMarkerMenu class provides CPU Plot remove menus.
 */
struct KsRmCPUPlotMenu : public KsRmPlotContextMenu {
	KsRmCPUPlotMenu(KsDualMarkerSM *dm, int cpu,
			QWidget *parent = nullptr);
};

/**
 * The KsQuickMarkerMenu class provides Task Plot remove menus.
 */
struct KsRmTaskPlotMenu : public KsRmPlotContextMenu {
	KsRmTaskPlotMenu(KsDualMarkerSM *dm, int pid,
			 QWidget *parent = nullptr);
};

#endif
