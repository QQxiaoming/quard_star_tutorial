// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsTraceViewer.cpp
 *  @brief   KernelShark Trace Viewer widget.
 */

// C++11
#include <thread>
#include <future>
#include <queue>

// KernelShark
#include "KsQuickContextMenu.hpp"
#include "KsTraceViewer.hpp"
#include "KsWidgetsLib.hpp"

/**
 * Reimplemented handler for mouse press events. Right mouse click events will
 * be ignored. This is done because we want the Right click is being used to
 * open a Context menu.
 */
void KsTableView::mousePressEvent(QMouseEvent *e) {
	if(e->button() == Qt::RightButton)
		return;

	QTableView::mousePressEvent(e);
}

/**
 * Reimplemented the handler for Auto-scrolling. With this we disable
 * the Horizontal Auto-scrolling.
 */
void KsTableView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
	int bottomMargin(2);

	if (hint == QAbstractItemView::EnsureVisible &&
	    index.row() > indexAt(rect().topLeft()).row() &&
	    index.row() < indexAt(rect().bottomLeft()).row() - bottomMargin)
		return;

	QTableView::scrollTo(index, hint);
}


/** Create a default (empty) Trace viewer widget. */
KsTraceViewer::KsTraceViewer(QWidget *parent)
: QWidget(parent),
  _view(this),
  _model(this),
  _proxyModel(this),
  _tableHeader(_model.header()),
  _toolbar(this),
  _labelSearch("Search: Column", this),
  _labelGrFollows("Graph follows  ", this),
  _searchFSM(this),
  _graphFollowsCheckBox(this),
  _graphFollows(true),
  _mState(nullptr),
  _data(nullptr)
{
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

	/* Make a search toolbar. */
	_toolbar.setOrientation(Qt::Horizontal);
	_toolbar.setMaximumHeight(FONT_HEIGHT * 1.75);

	/* On the toolbar make two Combo boxes for the search settings. */
	_toolbar.addWidget(&_labelSearch);
	_searchFSM._columnComboBox.addItems(_tableHeader);

	/*
	 * Using the old Signal-Slot syntax because
	 * QComboBox::currentIndexChanged has overloads.
	 */
	connect(&_searchFSM._columnComboBox,	SIGNAL(currentIndexChanged(int)),
		this,				SLOT(_searchEdit(int)));

	/*
	 * Using the old Signal-Slot syntax because
	 * QComboBox::currentIndexChanged has overloads.
	 */
	connect(&_searchFSM._selectComboBox,	SIGNAL(currentIndexChanged(int)),
		this,				SLOT(_searchEdit(int)));

	/* On the toolbar, make a Line edit field for search. */
	_searchFSM._searchLineEdit.setMaximumWidth(FONT_WIDTH * 20);

	connect(&_searchFSM._searchLineEdit,	&QLineEdit::returnPressed,
		this,				&KsTraceViewer::_search);

	connect(&_searchFSM._searchLineEdit,	&QLineEdit::textEdited,
		this,				&KsTraceViewer::_searchEditText);

	/* On the toolbar, add Prev & Next buttons. */
	connect(&_searchFSM._nextButton,	&QPushButton::pressed,
		this,				&KsTraceViewer::_next);

	connect(&_searchFSM._prevButton,	&QPushButton::pressed,
		this,				&KsTraceViewer::_prev);


	connect(&_searchFSM._searchStopButton,	&QPushButton::pressed,
		this,				&KsTraceViewer::_searchStop);

	connect(&_searchFSM._searchRestartButton,	&QPushButton::pressed,
		this,				&KsTraceViewer::_searchContinue);

	_searchFSM.placeInToolBar(&_toolbar);

	/*
	 * On the toolbar, make a Check box for connecting the search pannel
	 * to the Graph widget.
	 */
	_toolbar.addWidget(&_graphFollowsCheckBox);
	_toolbar.addWidget(&_labelGrFollows);
	_graphFollowsCheckBox.setCheckState(Qt::Checked);
	connect(&_graphFollowsCheckBox,	&QCheckBox::stateChanged,
		this,			&KsTraceViewer::_graphFollowsChanged);

	/* Initialize the trace viewer. */
	_view.horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	_view.verticalHeader()->setVisible(false);
	_view.setEditTriggers(QAbstractItemView::NoEditTriggers);
	_view.setSelectionBehavior(QAbstractItemView::SelectRows);
	_view.setSelectionMode(QAbstractItemView::SingleSelection);
	_view.verticalHeader()->setDefaultSectionSize(FONT_HEIGHT * 1.25);

	 _proxyModel.setSource(&_model);
	_view.setModel(&_proxyModel);
	connect(&_proxyModel, &QAbstractItemModel::modelReset,
		this, &KsTraceViewer::_searchReset);

	_view.setContextMenuPolicy(Qt::CustomContextMenu);
	connect(&_view,	&QWidget::customContextMenuRequested,
		this,	&KsTraceViewer::_onCustomContextMenu);

	connect(&_view,	&QTableView::clicked,
		this,	&KsTraceViewer::_clicked);

	/* Set the layout. */
	_layout.addWidget(&_toolbar);
	_layout.addWidget(&_view);
	this->setLayout(&_layout);
}

/**
 * @brief Load and show trace data.
 *
 * @param data: Input location for the KsDataStore object.
 *	  KsDataStore::loadDataFile() must be called first.
 */
void KsTraceViewer::loadData(KsDataStore *data)
{
	_data = data;
	_model.reset();
	_proxyModel.fill(data);
	_model.fill(data);
	this->_resizeToContents();

	this->setMinimumHeight(SCREEN_HEIGHT / 5);
}

/** Connect the QTableView widget and the State machine of the Dual marker. */
void KsTraceViewer::setMarkerSM(KsDualMarkerSM *m)
{
	QString styleSheetA, styleSheetB;

	_mState = m;
	_model.setColors(_mState->markerA()._color,
			 _mState->markerB()._color);

	/*
	 * Assign a property to State A of the Dual marker state machine. When
	 * the marker is in State A the background color of the selected row
	 * will be the same as the color of Marker A.
	 */
	styleSheetA = "selection-background-color : " +
		      _mState->markerA()._color.name() + ";";

	_mState->stateAPtr()->assignProperty(&_view, "styleSheet",
						     styleSheetA);

	/*
	 * Assign a property to State B. When the marker is in State B the
	 * background color of the selected row will be the same as the color
	 * of Marker B.
	 */
	styleSheetB = "selection-background-color : " +
		      _mState->markerB()._color.name() + ";";

	_mState->stateBPtr()->assignProperty(&_view, "styleSheet",
						     styleSheetB);
}

/** Reset (empty) the table. */
void KsTraceViewer::reset()
{
	this->setMinimumHeight(FONT_HEIGHT * 10);
	_model.reset();
	_resizeToContents();
}

void KsTraceViewer::_searchReset()
{
	_searchFSM.handleInput(sm_input_t::Change);
	_proxyModel.searchReset();
}

/** Get the index of the first (top) visible row. */
size_t  KsTraceViewer::getTopRow() const
{
	return _view.indexAt(_view.rect().topLeft()).row();
}

/** Position given row at the top of the table. */
void  KsTraceViewer::setTopRow(size_t r)
{
	_view.scrollTo(_proxyModel.index(r, 0),
		       QAbstractItemView::PositionAtTop);
}

/** Update the content of the table. */
void KsTraceViewer::update(KsDataStore *data)
{
	/* The Proxy model has to be updated first! */
	_proxyModel.fill(data);
	_model.update(data);
	_data = data;
	if (_mState->activeMarker()._isSet)
		showRow(_mState->activeMarker()._pos, true);
}

void KsTraceViewer::_onCustomContextMenu(const QPoint &point)
{
	QModelIndex i = _view.indexAt(point);

	if (i.isValid()) {
		/*
		 * Use the index of the proxy model to retrieve the value
		 * of the row number in the source model.
		 */
		size_t row = _proxyModel.mapRowFromSource(i.row());
		KsQuickContextMenu menu(_data, row, _mState, this);

		/*
		 * Note that this slot was connected to the
		 * customContextMenuRequested signal of the Table widget.
		 * Because of this the coordinates of the point are given with
		 * respect to the frame of this widget.
		 */
		QPoint global = _view.mapToGlobal(point);
		global.ry() -= menu.sizeHint().height() / 2;

		/*
		 * Shift the menu so that it is not positioned under the mouse.
		 * This will prevent from an accidental selection of the menu
		 * item under the mouse.
		 */
		global.rx() += FONT_WIDTH;

		connect(&menu,	&KsQuickContextMenu::addTaskPlot,
			this,	&KsTraceViewer::addTaskPlot);

		connect(&menu,	&KsQuickMarkerMenu::deselect,
			this,	&KsTraceViewer::deselect);

		menu.exec(global);
	}
}

void KsTraceViewer::_searchEdit(int index)
{
	_searchReset(); // The search has been modified.
}

void KsTraceViewer::_searchEditText(const QString &text)
{
	_searchReset(); // The search has been modified.
}

void KsTraceViewer::_graphFollowsChanged(int state)
{
	int row = selectedRow();

	_graphFollows = (bool) state;
	if (_graphFollows && row != KS_NO_ROW_SELECTED)
		emit select(row); // Send a signal to the Graph widget.
}

void KsTraceViewer::_search()
{
	if (!_searchDone()) {
		/*
		 * The search is not done. This means that the search settings
		 * have been modified since the last time we searched.
		 */
		_matchList.clear();
		_searchItems();

		if (!_matchList.empty()) {
			showRow(*_it, true);

			if (_graphFollows)
				emit select(*_it); // Send a signal to the Graph widget.
		}
	} else {
		/*
		 * If the search is done, pressing "Enter" is equivalent
		 * to pressing "Next" button.
		 */
		this->_next();
	}
}

void KsTraceViewer::_next()
{
	if (!_searchDone()) {
		_search();
		return;
	}

	if (!_matchList.empty()) { // Items have been found.
		int row = selectedRow();
		/*
		 * The iterator can only be at the selected row or if the
		 * selected row is not a match at the first matching row after
		 * the selected one.
		 */
		if (*_it == row) {
			++_it; // Move the iterator.
			if (_it == _matchList.end()) {
				/*
				 * This is the last item of the list.
				 * Go back to the beginning.
				 */
				_it = _matchList.begin();
			}
		}

		// Select the row of the item.
		showRow(*_it, true);

		if (_graphFollows)
			emit select(*_it); // Send a signal to the Graph widget.
	}

	_updateSearchCount();
}

void KsTraceViewer::_prev()
{
	if (!_searchDone()) {
		_search();
		return;
	}

	if (!_matchList.empty()) { // Items have been found.
		if (_it == _matchList.begin()) {
			// This is the first item of the list. Go to the last item.
			_it = _matchList.end() - 1;
		} else {
			--_it; // Move the iterator.
		}

		// Select the row of the item.
		showRow(*_it, true);

		if (_graphFollows)
			emit select(*_it); // Send a signal to the Graph widget.
	}

	_updateSearchCount();
}

void KsTraceViewer::_updateSearchCount()
{
	int index, total;
	QString countText;

	if (_matchList.isEmpty())
		return;

	index = _it - _matchList.begin();
	total =_matchList.count();

	countText = QString(" %1 / %2").arg(index + 1).arg(total);
	_searchFSM._searchCountLabel.setText(countText);
}

void KsTraceViewer::_searchStop()
{
	_proxyModel._searchStop = true;
	_searchFSM.handleInput(sm_input_t::Stop);
}

void KsTraceViewer::_searchContinue()
{
	_proxyModel._searchStop = false;
	_searchItems();
}

void KsTraceViewer::_clicked(const QModelIndex& i)
{
	/*
	 * Use the index of the proxy model to retrieve the value
	 * of the row number in the base model.
	 */
	size_t row = _proxyModel.mapRowFromSource(i.row());

	if (_searchDone() && _matchList.count()) {
		_setSearchIterator(row);
		_updateSearchCount();
	}

	if (_graphFollows)
		emit select(row); // Send a signal to the Graph widget.
}

/** Make a given row of the table visible. */
void KsTraceViewer::showRow(size_t r, bool mark)
{
	/*
	 * Use the index in the source model to retrieve the value of the row number
	 * in the proxy model.
	 */
	QModelIndex index = _proxyModel.mapFromSource(_model.index(r, 0));

	if (mark) { // The row will be selected (colored).
		/* Get the first and the last visible rows of the table. */
		int visiTot = _view.indexAt(_view.rect().topLeft()).row();
		int visiBottom = _view.indexAt(_view.rect().bottomLeft()).row() - 2;

		/* Scroll only if the row to be shown in not vizible. */
		if (index.row() < visiTot || index.row() > visiBottom)
			_view.scrollTo(index, QAbstractItemView::PositionAtCenter);

		_view.selectRow(index.row());
	} else {
		/*
		 * Just make sure that the row is visible. It will show up at
		 * the top of the visible part of the table.
		 */
		_view.scrollTo(index, QAbstractItemView::PositionAtTop);
	}
}

/** Deselects the selected items (row) if any. */
void KsTraceViewer::clearSelection()
{
	_view.clearSelection();
}

/** Switch the Dual marker. */
void KsTraceViewer::markSwitch()
{
	int row;

	/* The state of the Dual marker has changed. Get the new active marker. */
	DualMarkerState state = _mState->getState();

	/* First deal with the passive marker. */
	if (_mState->getMarker(!state)._isSet) {
		/*
		 * The passive marker is set. Use the model to color the row of
		 * the passive marker.
		 */
		_model.selectRow(!state, _mState->getMarker(!state)._pos);
	}
	else {
		/*
		 * The passive marker is not set.
		 * Make sure that the model colors nothing.
		 */
		_model.selectRow(!state, KS_NO_ROW_SELECTED);
	}

	/*
	 * Now deal with the active marker. This has to be done after dealing
	 *  with the model, because changing the model clears the selection.
	 */
	if (_mState->getMarker(state)._isSet) {
		/*
		 * The active marker is set. Use QTableView to select its row.
		 * The index in the source model is used to retrieve the value
		 * of the row number in the proxy model.
		 */
		size_t row =_mState->getMarker(state)._pos;

		QModelIndex index =
			_proxyModel.mapFromSource(_model.index(row, 0));

		if (index.isValid()) {
			/*
			 * The row of the active marker will be colored according to
			 * the assigned property of the current state of the Dual
			 * marker. Auto-scrolling is temporarily disabled because we
			 * do not want to scroll to the position of the marker yet.
			 */
			_view.setAutoScroll(false);
			_view.selectRow(index.row());
			_view.setAutoScroll(true);
		} else {
			_view.clearSelection();
		}
	} else {
		_view.clearSelection();
	}

	row = selectedRow();
	if (row >= 0) {
		_setSearchIterator(row);
		_updateSearchCount();
	}
}

/**
 * Reimplemented event handler used to update the geometry of the widget on
 * resize events.
 */
void KsTraceViewer::resizeEvent(QResizeEvent* event)
{
	int nColumns = _tableHeader.count();
	int tableSize(0), viewSize, freeSpace;

	_resizeToContents();
	for (int c = 0; c < nColumns; ++c) {
		tableSize += _view.columnWidth(c);
	}

	viewSize = _view.width() -
		   qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

	if ((freeSpace = viewSize - tableSize) > 0) {
		_view.setColumnWidth(nColumns - 1, _view.columnWidth(nColumns - 1) +
						   freeSpace -
						   2); /* Just a little bit less space.
							* This will allow the scroll bar
							* to disappear when the widget
							* is extended to maximum. */
	}
}

/**
 * Reimplemented event handler used to move the active marker.
 */
void KsTraceViewer::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
		int row = selectedRow();
		if (row >= 0)
			emit select(row); // Send a signal to the Graph widget.

		return;
	}

	QWidget::keyReleaseEvent(event);
}

void KsTraceViewer::_resizeToContents()
{
	int rows, columnSize, markRow = selectedRow();

	_view.setVisible(false);
	_view.resizeColumnsToContents();
	_view.setVisible(true);

	/*
	 * It looks like a Qt bug, but sometimes when no row is selected in
	 * the table, the automatic resize of the widget (the lines above) has
	 * the parasitic effect to select the first row of the table.
	 */
	if (markRow == KS_NO_ROW_SELECTED)
		_view.clearSelection();

	/*
	 * Because of some unknown reason the first column doesn't get
	 * resized properly by the code above. We will resize this
	 * column by hand.
	 */
	rows = _model.rowCount({});
	columnSize = STRING_WIDTH(QString("%1").arg(rows)) + FONT_WIDTH;
	_view.setColumnWidth(0, columnSize);
}

//! @cond Doxygen_Suppress

#define KS_SEARCH_SHOW_PROGRESS_MIN 100000

//! @endcond

size_t KsTraceViewer::_searchItems()
{
	int column = _searchFSM._columnComboBox.currentIndex();
	QString searchText = _searchFSM._searchLineEdit.text();
	int count, dataRow;

	if (searchText.isEmpty()) {
		/*
		 * No text is provided by the user. Most probably this
		 * is an accidental key press. */
		return 0;
	}

	if (_proxyModel.rowCount({}) < KS_SEARCH_SHOW_PROGRESS_MIN) {
		/*
		 * This is a small data-set. Do a single-threaded search
		 * without showing the progress. We will bypass the state
		 * switching, hence the search condition has to be updated
		 * by hand.
		 */
		_searchFSM.updateCondition();
		_proxyModel.search(column, searchText, _searchFSM.condition(), &_matchList,
				   nullptr, nullptr);
	} else {
		_searchFSM.handleInput(sm_input_t::Start);

		if (column == KsViewModel::TRACE_VIEW_COL_INFO ||
		    column == KsViewModel::TRACE_VIEW_COL_LAT)
			_searchItemsST();
		else
			_searchItemsMT();
	}

	count = _matchList.count();
	_searchFSM.handleInput(sm_input_t::Finish);

	if (count == 0) // No items have been found. Do nothing.
		return 0;

	dataRow = selectedRow();
	if (dataRow >= 0) {
		_view.clearSelection();
		_setSearchIterator(dataRow);
		showRow(*_it, true);

		if (_graphFollows)
			emit select(*_it); // Send a signal to the Graph widget.
	} else {
		/* Move the iterator to the beginning of the match list. */
		_it = _matchList.begin();
	}

	_updateSearchCount();

	return count;
}

void KsTraceViewer::_setSearchIterator(int row)
{
	_it = _matchList.begin();
	if (_matchList.isEmpty())
		return;

	/*
	 * Move the iterator to the first element of the match list
	 * after the selected one.
	 */
	while (*_it < row) {
		++_it;  // Move the iterator.
		if (_it == _matchList.end()) {
			/*
			 * This is the last item of the list. Go back
			 * to the beginning.
			 */
			_it = _matchList.begin();
			break;
		}
	}
}

void KsTraceViewer::_searchItemsMT()
{
	int nThreads = std::thread::hardware_concurrency();
	int startFrom, nRows(_proxyModel.rowCount({}));
	std::vector<QPair<int, int>> ranges(nThreads);
	std::vector<std::future<QList<int>>> maps;
	std::mutex lrs_mtx;

	auto lamLRSUpdate = [&] (int lastRowSearched) {
		std::lock_guard<std::mutex> lock(lrs_mtx);

		if (_searchFSM._lastRowSearched > lastRowSearched ||
		    _searchFSM._lastRowSearched < 0) {
			/*
			 * This thread has been slower and processed
			 * less data. Take the place where it stopped
			 * as a starting point of the next search.
			 */
			_searchFSM._lastRowSearched = lastRowSearched;
		}
	};

	auto lamSearchMap = [&] (const int first, bool notify) {
		int lastRowSearched;
		QList<int> list;

		list = _proxyModel.searchThread(_searchFSM._columnComboBox.currentIndex(),
						_searchFSM._searchLineEdit.text(),
						_searchFSM.condition(),
						nThreads,
						first, nRows - 1,
						&lastRowSearched,
						notify);

		lamLRSUpdate(lastRowSearched);

		return list;
	};

	using merge_pair_t = std::pair<int, int>;
	using merge_container_t = std::vector<merge_pair_t>;

	auto lamComp = [] (const merge_pair_t& itemA, const merge_pair_t& itemB) {
		return itemA.second > itemB.second;
	};

	using merge_queue_t = std::priority_queue<merge_pair_t,
						  merge_container_t,
						  decltype(lamComp)>;

	auto lamSearchMerge = [&] (QList<int> &resultList,
				   QVector< QList<int> >&mapList) {
		merge_queue_t queue(lamComp);
		int id, stop(-1);

		auto pop = [&] () {
			if (queue.size() == 0)
				return stop;

			auto item = queue.top();
			queue.pop();

			if (!mapList[item.first].empty()) {
				/*
				 * Replace the popped item with the next
				 * matching item fron the same search thread.
				 */
				queue.push(std::make_pair(item.first,
							  mapList[item.first].front()));
				mapList[item.first].pop_front();
			}

			if (_searchFSM.getState() == search_state_t::Paused_s &&
			    item.second > _searchFSM._lastRowSearched) {
				/*
				 * The search has been paused and we already
				 * passed the last row searched by the slowest
				 * search thread. Stop here and ignore all
				 * following matches found by faster threads.
				 */
				return stop;
			}

			return item.second;
		};

		for (int i = 0; i < mapList.size(); ++i)
			if ( mapList[i].count()) {
				queue.push(std::make_pair(i, mapList[i].front()));
				mapList[i].pop_front();
			}

		id = pop();
		while (id >= 0) {
			resultList.append(id);
			id = pop();
		}
	};

	startFrom = _searchFSM._lastRowSearched + 1;
	_searchFSM._lastRowSearched = -1;

	/* Start the thread that will update the progress bar. */
	maps.push_back(std::async(lamSearchMap,
				  startFrom,
				  true)); // notify = true

	/* Start all other threads. */
	for (int r = 1; r < nThreads; ++r)
		maps.push_back(std::async(lamSearchMap,
					  startFrom + r,
					  false)); // notify = false

	while (_searchFSM.getState() == search_state_t::InProgress_s &&
	       _proxyModel.searchProgress() < KS_PROGRESS_BAR_MAX - nThreads - 1) {
		std::unique_lock<std::mutex> lk(_proxyModel._mutex);
		_proxyModel._pbCond.wait(lk);
		_searchFSM.setProgress(_proxyModel.searchProgress());
		QApplication::processEvents();
	}

	QVector<QList<int>> res;
	for (auto &m: maps)
		res.append(std::move(m.get()));

	lamSearchMerge(_matchList, res);
}

/**
 * @brief Color (select) the given row in the table, by using the color of the
 * 	  Passive marker.
 *
 * @param row: The row index. If the Passive marker is selected and the input
 *	        value is negative, the Passive marker will be deselected.
 */
void KsTraceViewer::passiveMarkerSelectRow(int row)
{
	DualMarkerState state = _mState->getState();

	_view.setVisible(false);
	_model.selectRow(!state, row);
	_view.setVisible(true);
}

/**
 * Get the currently selected row. If no row is selected the function
 * returns KS_NO_ROW_SELECTED (-1).
 */
int KsTraceViewer::selectedRow()
{
	QItemSelectionModel *sm = _view.selectionModel();
	int dataRow = KS_NO_ROW_SELECTED;

	if (sm->hasSelection()) {
		/* Only one row at the time can be selected. */
		QModelIndex  i = sm->selectedRows()[0];
		dataRow = _proxyModel.mapRowFromSource(i.row());
	}

	return dataRow;
}
