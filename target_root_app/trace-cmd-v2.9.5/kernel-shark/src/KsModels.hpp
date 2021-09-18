/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsModels.hpp
 *  @brief   Models for data representation.
 */

#ifndef _KS_MODELS_H
#define _KS_MODELS_H

// C++11
#include <mutex>
#include <condition_variable>

// Qt
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QProgressBar>
#include <QLabel>
#include <QColor>

// KernelShark
#include "libkshark.h"
#include "libkshark-model.h"
#include "KsSearchFSM.hpp"

/** A negative row index, to be used for deselecting the Passive Marker. */
#define KS_NO_ROW_SELECTED -1

enum class DualMarkerState;

class KsDataStore;

/**
 * Class KsViewModel provides models for trace data representation in a
 * table view.
 */
class KsViewModel : public QAbstractTableModel
{
public:
	explicit KsViewModel(QObject *parent = nullptr);

	/** Set the colors of the two markers. */
	void setColors(const QColor &colA, const QColor &colB) {
		_colorMarkA = colA;
		_colorMarkB = colB;
	};

	/**
	 * Get the number of rows. This is an implementation of the pure
	 * virtual method of the abstract model class.
	 */
	int rowCount(const QModelIndex &) const override {return _nRows;}

	/**
	 * Get the number of columns. This is an implementation of the pure
	 * virtual method of the abstract model class.
	 */
	int columnCount(const QModelIndex &) const override
	{
		return _header.count();
	}

	QVariant headerData(int section,
			    Qt::Orientation orientation,
			    int role) const override;

	QVariant data(const QModelIndex &index, int role) const override;

	void fill(KsDataStore *data);

	void selectRow(DualMarkerState state, int row);

	void reset();

	void update(KsDataStore *data);

	/** Get the list of column's headers. */
	QStringList header() const {return _header;}

	QString getValueStr(int column, int row) const;

	QVariant getValue(int column, int row) const;

	size_t search(int column,
		      const QString &searchText,
		      search_condition_func cond,
		      QList<size_t> *matchList);

	/** Table columns Identifiers. */
	enum {
		/** Identifier of the Index column. */
		TRACE_VIEW_COL_INDEX,

		/** Identifier of the CPU column. */
		TRACE_VIEW_COL_CPU,

		/** Identifier of the Timestamp column. */
		TRACE_VIEW_COL_TS,

		/** Identifier of the Task name (command) column. */
		TRACE_VIEW_COL_COMM,

		/** Identifier of the Process Id column. */
		TRACE_VIEW_COL_PID,

		/** Identifier of the Latency Id column. */
		TRACE_VIEW_COL_LAT,

		/** Identifier of the Event name Id column. */
		TRACE_VIEW_COL_EVENT,

		/** Identifier of the Event name Id column. */
		TRACE_VIEW_COL_INFO,

		/** Number of column. */
		TRACE_VIEW_N_COLUMNS,
	};

private:
	/** Trace data array. */
	kshark_entry		**_data;

	/** The size of the data array. */
	size_t			_nRows;

	/** The headers of the individual columns. */
	QStringList		_header;

	/** The index of marker A inside the data array. */
	int	_markA;

	/** The index of marker A inside the data array. */
	int	_markB;

	/** The color of the row selected by marker A. */
	QColor	_colorMarkA;

	/** The color of the row selected by marker B. */
	QColor	_colorMarkB;
};

/**
 * Class KsFilterProxyModel provides support for filtering trace data in
 * table view.
 */
class KsFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	explicit KsFilterProxyModel(QObject *parent = nullptr);

	bool filterAcceptsRow(int sourceRow,
			      const QModelIndex &sourceParent) const override;

	void fill(KsDataStore *data);

	void setSource(KsViewModel *s);

	size_t search(int column,
		      const QString &searchText,
		      search_condition_func cond,
		      QList<int> *matchList,
		      QProgressBar *pb = nullptr,
		      QLabel *l = nullptr);

	size_t search(KsSearchFSM *sm, QList<int> *matchList);

	QList<int> searchThread(int column,
				const QString &searchText,
				search_condition_func cond,
				int step,
				int first,
				int last,
				int *lastRowSearched,
				bool notify);

	/** Get the progress of the search. */
	int searchProgress() const {return _searchProgress;}

	/** Reset the progress value of the search. */
	void searchReset() {
		_searchProgress = 0;
		_searchStop = false;
	}

	/**
	 * Use the "row" index in the Proxy model to retrieve the "row" index
	 * in the source model.
	 */
	int mapRowFromSource(int r) const
	{
		/*This works because the row number is shown in column "0". */
		return this->data(this->index(r, 0)).toInt();
	}

	/** Get the source model. */
	KsViewModel *source() {return _source;}

	/**
	 * A condition variable used to notify the main thread to update the
	 * search progressbar.
	 */
	std::condition_variable	_pbCond;

	/** A mutex used by the condition variable. */
	std::mutex		_mutex;

	/** A flag used to stop the serch for all threads. */
	bool			_searchStop;

private:
	int			_searchProgress;

	/** Trace data array. */
	kshark_entry		**_data;

	KsViewModel	 	*_source;

	size_t _search(int column,
		       const QString &searchText,
		       search_condition_func cond,
		       QList<int> *matchList,
		       int step,
		       int first, int last,
		       QProgressBar *pb,
		       QLabel *l,
		       int *lastRowSearched,
		       bool notify);
};

/**
 * Class KsGraphModel provides a model for visualization of trace data. This
 * class is a wrapper of kshark_trace_histo and is needed only because we want
 * to use the signals defined in QAbstractTableModel.
 */
class KsGraphModel : public QAbstractTableModel
{
public:
	explicit KsGraphModel(QObject *parent = nullptr);

	virtual ~KsGraphModel();

	/**
	 * This dummy function is an implementation of the pure
	 * virtual method of the abstract model class.
	 */
	int rowCount(const QModelIndex &) const override
	{
		return _histo.n_bins;
	}

	/**
	 * This dummy function is an implementation of the pure
	 * virtual method of the abstract model class.
	 */
	int columnCount(const QModelIndex &) const override {return 0;}

	/**
	 * This dummy function is an implementation of the pure
	 * virtual method of the abstract model class.
	 */
	QVariant data(const QModelIndex &index, int role) const override
	{
		return {};
	}

	/** Get the kshark_trace_histo object. */
	kshark_trace_histo *histo() {return &_histo;}

	void fill(kshark_entry **entries, size_t n);

	void shiftForward(size_t n);

	void shiftBackward(size_t n);

	void jumpTo(size_t ts);

	void zoomOut(double r, int mark = -1);

	void zoomIn(double r, int mark = -1);

	void quickZoomOut();

	void quickZoomIn(uint64_t binSize);

	void reset();

	void update(KsDataStore *data = nullptr);

private:
	kshark_trace_histo	_histo;
};

/** Defines a default number of bins to be used by the visualization model. */
#define KS_DEFAULT_NBUNS	1024

#endif // _KS_MODELS_H
