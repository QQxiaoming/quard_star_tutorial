/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsTraceViewer.hpp
 *  @brief   KernelShark Trace Viewer widget.
 */

#ifndef _KS_TRACEVIEW_H
#define _KS_TRACEVIEW_H

// Qt
#include <QTableView>

// KernelShark
#include "KsUtils.hpp"
#include "KsModels.hpp"
#include "KsSearchFSM.hpp"
#include "KsDualMarker.hpp"

/**
 * Table View class, needed in order to reimplemented the handler for mouse
 * press events.
 */
class KsTableView : public QTableView
{
	Q_OBJECT
public:
	/** Create KsTableView. */
	explicit KsTableView(QWidget *parent = nullptr)
	: QTableView(parent) {};

	void mousePressEvent(QMouseEvent *event) override;

	void scrollTo(const QModelIndex &index, ScrollHint hint) override;
};

/**
 * The KsTraceViewer class provides a widget for browsing in the trace data
 * shown in a text form.
 */
class KsTraceViewer : public QWidget
{
	Q_OBJECT
public:
	explicit KsTraceViewer(QWidget *parent = nullptr);

	void loadData(KsDataStore *data);

	void setMarkerSM(KsDualMarkerSM *m);

	void reset();

	size_t getTopRow() const;

	void setTopRow(size_t r);

	void resizeEvent(QResizeEvent* event) override;

	void keyReleaseEvent(QKeyEvent *event);

	void markSwitch();

	void showRow(size_t r, bool mark);

	void clearSelection();

	void passiveMarkerSelectRow(int row);

	int selectedRow();

	void update(KsDataStore *data);

signals:
	/** Signal emitted when new row is selected. */
	void select(size_t);

	/**
	 * This signal is used to re-emitted the addTaskPlot signal of the
	 * KsQuickContextMenu.
	 */
	void addTaskPlot(int pid);

	/**
	 * This signal is used to re-emitted the deselect signal of the
	 * KsQuickMarkerMenu.
	 */
	void deselect();

private:
	QVBoxLayout	_layout;

	KsTableView	_view;

	KsViewModel		_model;

	KsFilterProxyModel	_proxyModel;

	QStringList	_tableHeader;

	QToolBar	_toolbar;

	QLabel		_labelSearch, _labelGrFollows;

	KsSearchFSM	_searchFSM;

	QCheckBox	_graphFollowsCheckBox;

	bool		_graphFollows;

	QList<int>		_matchList;

	QList<int>::iterator	_it;

	KsDualMarkerSM		*_mState;

	KsDataStore		*_data;

	enum Condition
	{
		Containes = 0,
		Match = 1,
		NotHave = 2
	};

	void _searchReset();

	void _resizeToContents();

	size_t _searchItems();

	void _searchItemsST() {_proxyModel.search(&_searchFSM, &_matchList);}

	void _searchItemsMT();

	void _searchEditText(const QString &);

	void _graphFollowsChanged(int);

	void _lockSearchPanel(bool lock);

	void _search();

	void _next();

	void _prev();

	void _updateSearchCount();

	void _searchStop();

	void _searchContinue();

	void _clicked(const QModelIndex& i);

	void _onCustomContextMenu(const QPoint &);

	void _setSearchIterator(int row);

	bool _searchDone()
	{
		return _searchFSM.getState() == search_state_t::Done_s ||
		       _searchFSM.getState() == search_state_t::Paused_s;
	}

private slots:
	void _searchEdit(int);
};

#endif // _KS_TRACEVIEW_H
