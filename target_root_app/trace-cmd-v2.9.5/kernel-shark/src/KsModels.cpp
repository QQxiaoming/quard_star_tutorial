// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsModels.cpp
 *  @brief   Models for data representation.
 */

// KernelShark
#include "KsModels.hpp"
#include "KsWidgetsLib.hpp"
#include "KsUtils.hpp"

/** Create a default (empty) KsFilterProxyModel object. */
KsFilterProxyModel::KsFilterProxyModel(QObject *parent)
: QSortFilterProxyModel(parent),
  _searchStop(false),
  _source(nullptr)
{}

/**
 * Returns False if the item in the row indicated by the sourceRow and
 * sourceParentshould be filtered out. Otherwise returns True.
 */
bool
KsFilterProxyModel::filterAcceptsRow(int sourceRow,
				     const QModelIndex &sourceParent) const
{
	if (_data[sourceRow]->visible & KS_TEXT_VIEW_FILTER_MASK)
		return true;

	return false;
}

/** Provide the Proxy model with data. */
void KsFilterProxyModel::fill(KsDataStore *data)
{
	_data = data->rows();
}

/** Set the source model for this Proxy model. */
void KsFilterProxyModel::setSource(KsViewModel *s)
{
	QSortFilterProxyModel::setSourceModel(s);
	_source = s;
}

size_t KsFilterProxyModel::_search(int column,
				   const QString &searchText,
				   search_condition_func cond,
				   QList<int> *matchList,
				   int step,
				   int first, int last,
				   QProgressBar *pb,
				   QLabel *l,
				   int *lastRowSearched,
				   bool notify)
{
	int index, row, nRows(last - first + 1);
	int milestone(1), pbCount(1);
	QString item;

	if (nRows > KS_PROGRESS_BAR_MAX)
		milestone = pbCount = nRows / (KS_PROGRESS_BAR_MAX - step -
					       _searchProgress);
	else
		_searchProgress = KS_PROGRESS_BAR_MAX - nRows;

	/* Loop over the items of the proxy model. */
	for (index = first; index <= last; index += step) {
		/*
		 * Use the index of the proxy model to retrieve the value
		 * of the row number in the base model.
		 */
		row = mapRowFromSource(index);
		item = _source->getValueStr(column, row);
		if (cond(searchText, item))
			matchList->append(row);

		if (_searchStop) {
			if (lastRowSearched)
				*lastRowSearched = index;

			if (notify)
				_pbCond.notify_one();

			break;
		}

		/* Deal with the Progress bar of the seatch. */
		if ((index - first) >= milestone) {
			milestone += pbCount;
			if (notify) {
				/*
				 * This is a multi-threaded search. Notify
				 * the main thread to update the progress bar.
				 */
				std::lock_guard<std::mutex> lk(_mutex);
				++_searchProgress;
				_pbCond.notify_one();
			} else {
				if (pb) {
					pb->setValue(pb->value() + 1);
					++_searchProgress;
				}

				if (l)
					l->setText(QString(" %1").arg(matchList->count()));

				QApplication::processEvents();
			}
		}
	}

	return index;
}

/** @brief Search the content of the table for a data satisfying an abstract
 *	   condition.
 *
 * @param column: The number of the column to search in.
 * @param searchText: The text to search for.
 * @param cond: Matching condition function.
 * @param matchList: Output location for a list containing the row indexes of
 *		     the cells satisfying matching condition.
 * @param pb: Input location for a Progressbar used to visualize the progress
 *	      of the search.
 * @param l: Input location for a label used to show the number of cells found.
 *
 * @returns The number of cells satisfying the matching condition.
 */
size_t KsFilterProxyModel::search(int column,
				  const QString &searchText,
				  search_condition_func cond,
				  QList<int> *matchList,
				  QProgressBar *pb,
				  QLabel *l)
{
	int nRows = rowCount({});
	_search(column,
		searchText,
		cond,
		matchList,
		1,		// step
		0,		// first
		nRows - 1,	// last
		pb,
		l,
		nullptr,
		false);

	return matchList->count();
}

/** @brief Search the content of the table for a data satisfying an abstract
 *	   condition.
 *
 * @param sm: Input location for the Search State machine object.
 * @param matchList: Output location for a list containing the row indexes of
 *
 * @returns The number of cells satisfying the matching condition.
 */
size_t KsFilterProxyModel::search(KsSearchFSM *sm, QList<int> *matchList)
{
	int nRows = rowCount({});

	_search(sm->column(),
		sm->searchText(),
		sm->condition(),
		matchList,
		1,				// step
		sm->_lastRowSearched + 1,	// first
		nRows - 1,			// last
		&sm->_searchProgBar,
		&sm->_searchCountLabel,
		&sm->_lastRowSearched,
		false);

	return matchList->count();
}

/** @brief Search the content of the table for a data satisfying an abstract
 *	   condition.
 *
 * @param column: The number of the column to search in.
 * @param searchText: The text to search for.
 * @param cond: Matching condition function.
 * @param step: The step used by the thread of the search when looping over
 *		the data.
 * @param first: Row index specifying the position inside the table from
 *		 where the search starts.
 * @param last:  Row index specifying the position inside the table from
 *		 where the search ends.
 * @param lastRowSearched: Output location for parameter showing the index of
 *			   the last searched item (data row).
 * @param notify: Input location for flag specifying if the search has to
 *		  notify the main thread when to update the progress bar.
 *
 * @returns A list containing the row indexes of the cells satisfying matching
 *	    condition.
 */
QList<int> KsFilterProxyModel::searchThread(int column,
					    const QString &searchText,
					    search_condition_func cond,
					    int step,
					    int first,
					    int last,
					    int *lastRowSearched,
					    bool notify)
{
	QList<int> matchList;
	_search(column,
		searchText,
		cond,
		&matchList,
		step,
		first,
		last,
		nullptr,
		nullptr,
		lastRowSearched,
		notify);

	return matchList;
}

/** Create default (empty) KsViewModel object. */
KsViewModel::KsViewModel(QObject *parent)
: QAbstractTableModel(parent),
  _data(nullptr),
  _nRows(0),
  _header({"#", "CPU", "Time Stamp", "Task", "PID",
	   "Latency", "Event", "Info"}),
  _markA(KS_NO_ROW_SELECTED),
  _markB(KS_NO_ROW_SELECTED)
{}

/**
 * Get the data stored under the given role for the item referred to by
 * the index. This is an implementation of the pure virtual method of the
 * abstract model class.
 */
QVariant KsViewModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::ForegroundRole) {
		if (index.row() == _markA)
			return QVariant::fromValue(QColor(Qt::white));

		if (index.row() == _markB)
			return QVariant::fromValue(QColor(Qt::white));
	}

	if (role == Qt::BackgroundRole) {
		if (index.row() == _markA)
			return QVariant::fromValue(QColor(_colorMarkA));

		if (index.row() == _markB)
			return QVariant::fromValue(QColor(_colorMarkB));
	}

	if (role == Qt::DisplayRole)
		return this->getValue(index.column(), index.row());

	return {};
}

/** Get the string data stored in a given cell of the table. */
QString KsViewModel::getValueStr(int column, int row) const
{
	int pid;

	switch (column) {
		case TRACE_VIEW_COL_INDEX :
			return QString("%1").arg(row);

		case TRACE_VIEW_COL_CPU:
			return QString("%1").arg(_data[row]->cpu);

		case TRACE_VIEW_COL_TS:
			return KsUtils::Ts2String(_data[row]->ts, 6);

		case TRACE_VIEW_COL_COMM:
			return kshark_get_task_easy(_data[row]);

		case TRACE_VIEW_COL_PID:
			pid = kshark_get_pid_easy(_data[row]);
			return QString("%1").arg(pid);

		case TRACE_VIEW_COL_LAT:
			return kshark_get_latency_easy(_data[row]);

		case TRACE_VIEW_COL_EVENT:
			return kshark_get_event_name_easy(_data[row]);

		case TRACE_VIEW_COL_INFO :
			return kshark_get_info_easy(_data[row]);

		default:
			return {};
	}
}

/** Get the data stored in a given cell of the table. */
QVariant KsViewModel::getValue(int column, int row) const
{
	return getValueStr(column, row);
}

/**
 * Get the header of a column. This is an implementation of the pure virtual
 * method of the abstract model class.
 */
QVariant KsViewModel::headerData(int column,
				 Qt::Orientation orientation,
				 int role) const
{
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return {};

	if (column < _header.count())
		return _header.at(column);

	return {};
}

/** Provide the model with data. */
void KsViewModel::fill(KsDataStore *data)
{
	beginInsertRows(QModelIndex(), 0, data->size() - 1);

	_data = data->rows();
	_nRows = data->size();

	endInsertRows();
}

/** @brief Select a row in the table.
 *
 * @param state: Identifier of the marker used to select the row.
 * @param row: Row index.
 */
void KsViewModel::selectRow(DualMarkerState state, int row)
{
	if (state == DualMarkerState::A) {
		_markA = row;
		_markB = KS_NO_ROW_SELECTED;
	} else {
		_markB = row;
		_markA = KS_NO_ROW_SELECTED;
	}
}

/** Reset the model. */
void KsViewModel::reset()
{
	beginResetModel();

	_data = nullptr;
	_nRows = 0;

	endResetModel();
}

/** Update the model. Use this function if the data has changed. */
void KsViewModel::update(KsDataStore *data)
{
	/*
	 * Do not try to skip the reset(). The row count has to be set to
	 * zero before you full.
	 */
	reset();
	fill(data);
}

/** @brief Search the content of the table for a data satisfying an abstract
 *	   condition.
 *
 * @param column: The number of the column to search in.
 * @param searchText: The text to search for.
 * @param cond: Matching condition function.
 * @param matchList: Output location for a list containing the row indexes of
 *		     the cells satisfying the matching condition.
 *
 * @returns The number of cells satisfying the matching condition.
 */
size_t KsViewModel::search(int column,
			   const QString &searchText,
			   search_condition_func cond,
			   QList<size_t> *matchList)
{
	int nRows = rowCount({});
	QVariant item;

	for (int r = 0; r < nRows; ++r) {
		item = getValue(r, column);
		if (cond(searchText, item.toString())) {
			matchList->append(r);
		}
	}

	return matchList->count();
}

/** Create a default (empty) KsFilterProxyModel object. */
KsGraphModel::KsGraphModel(QObject *parent)
: QAbstractTableModel(parent)
{
	ksmodel_init(&_histo);
}

/** Destroy KsFilterProxyModel object. */
KsGraphModel::~KsGraphModel()
{
	ksmodel_clear(&_histo);
}

/**
 * @brief Provide the Visualization model with data. Calculate the current
 *	  state of the model.
 *
 * @param entries: Input location for the trace data.
 * @param n: Number of bins.
 */
void KsGraphModel::fill(kshark_entry **entries, size_t n)
{
	if (n == 0)
		return;

	beginResetModel();

	if (_histo.n_bins == 0)
		ksmodel_set_bining(&_histo,
				   KS_DEFAULT_NBUNS,
				   entries[0]->ts,
				   entries[n-1]->ts);

	ksmodel_fill(&_histo, entries, n);

	endResetModel();
}

/**
 * @brief Shift the time-window of the model forward. Recalculate the current
 *	  state of the model.
 *
 * @param n: Number of bins to shift.
 */
void KsGraphModel::shiftForward(size_t n)
{
	beginResetModel();
	ksmodel_shift_forward(&_histo, n);
	endResetModel();
}

/**
 * @brief Shift the time-window of the model backward. Recalculate the current
 *	  state of the model.
 *
 * @param n: Number of bins to shift.
 */
void KsGraphModel::shiftBackward(size_t n)
{
	beginResetModel();
	ksmodel_shift_backward(&_histo, n);
	endResetModel();
}

/**
 * @brief Move the time-window of the model to a given location. Recalculate
 *	  the current state of the model.
 *
 * @param ts: position in time to be visualized.
 */
void KsGraphModel::jumpTo(size_t ts)
{
	beginResetModel();
	ksmodel_jump_to(&_histo, ts);
	endResetModel();
}

/**
 * @brief Extend the time-window of the model. Recalculate the current state
 *	  of the model.
 *
 * @param r: Scale factor of the zoom-out.
 * @param mark: Focus point of the zoom-out.
 */
void KsGraphModel::zoomOut(double r, int mark)
{
	beginResetModel();
	ksmodel_zoom_out(&_histo, r, mark);
	endResetModel();
}

/**
 * @brief Shrink the time-window of the model. Recalculate the current state
 *	  of the model.
 *
 * @param r: Scale factor of the zoom-in.
 * @param mark: Focus point of the zoom-in.
 */
void KsGraphModel::zoomIn(double r, int mark)
{
	beginResetModel();
	ksmodel_zoom_in(&_histo, r, mark);
	endResetModel();
}

/** Quick zoom out. The entire data-set will be visualized. */
void KsGraphModel::quickZoomOut()
{
	beginResetModel();

	ksmodel_set_bining(&_histo,
			   _histo.n_bins,
			   _histo.data[0]->ts,
			   _histo.data[_histo.data_size - 1]->ts);

	ksmodel_fill(&_histo, _histo.data, _histo.data_size);

	endResetModel();
}

/**
 * @brief Quick zoom in to a state of the Visualization model which has the
 * given bin size. The actual value of the bin size may en up being slightly
 * different because of the fine tuning performed by the model.
 *
 * @param binSize: an approximate value for the new size of the bins.
 */
void KsGraphModel::quickZoomIn(uint64_t binSize)
{
	double range, r;

	range =  _histo.max - _histo.min;
	r = 1 - (binSize * _histo.n_bins) / range;
	zoomIn(r);
}

/** Reset the model. */
void KsGraphModel::reset()
{
	beginResetModel();
	ksmodel_clear(&_histo);
	endResetModel();
}

/** Update the model. Use this function if the data has changed. */
void KsGraphModel::update(KsDataStore *data)
{
	beginResetModel();
	if (data)
		ksmodel_fill(&_histo, data->rows(), data->size());
	endResetModel();
}
