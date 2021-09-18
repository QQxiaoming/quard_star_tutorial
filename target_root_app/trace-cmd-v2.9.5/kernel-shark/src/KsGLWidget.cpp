// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

 /**
 *  @file    KsGLWidget.cpp
 *  @brief   OpenGL widget for plotting trace graphs.
 */

// OpenGL
#include <GL/glut.h>
#include <GL/gl.h>

// KernelShark
#include "KsGLWidget.hpp"
#include "KsUtils.hpp"
#include "KsPlugins.hpp"
#include "KsDualMarker.hpp"

/** Create a default (empty) OpenGL widget. */
KsGLWidget::KsGLWidget(QWidget *parent)
: QOpenGLWidget(parent),
  _hMargin(20),
  _vMargin(30),
  _vSpacing(20),
  _mState(nullptr),
  _data(nullptr),
  _rubberBand(QRubberBand::Rectangle, this),
  _rubberBandOrigin(0, 0),
  _dpr(1)
{
	setMouseTracking(true);

	/*
	 * Using the old Signal-Slot syntax because QWidget::update has
	 * overloads.
	 */
	connect(&_model, SIGNAL(modelReset()), this, SLOT(update()));
}

KsGLWidget::~KsGLWidget()
{
	for (auto &g: _graphs)
		delete g;
}

/** Reimplemented function used to set up all required OpenGL resources. */
void KsGLWidget::initializeGL()
{
	_dpr = QApplication::desktop()->devicePixelRatio();
	ksplot_init_opengl(_dpr);
}

/**
 * Reimplemented function used to reprocess all graphs whene the widget has
 * been resized.
 */
void KsGLWidget::resizeGL(int w, int h)
{
	ksplot_resize_opengl(w, h);
	if(!_data)
		return;

	/*
	 * From the size of the widget, calculate the number of bins.
	 * One bin will correspond to one pixel.
	 */
	int nBins = width() - _hMargin * 2;

	/*
	 * Reload the data. The range of the histogram is the same
	 * but the number of bins changes.
	 */
	ksmodel_set_bining(_model.histo(),
			   nBins,
			   _model.histo()->min,
			   _model.histo()->max);

	_model.fill(_data->rows(), _data->size());
}

/** Reimplemented function used to plot trace graphs. */
void KsGLWidget::paintGL()
{
	float size = 1.5 * _dpr;

	glClear(GL_COLOR_BUFFER_BIT);

	if (isEmpty())
		return;

	/* Draw the time axis. */
	_drawAxisX(size);

	/* Process and draw all graphs by using the built-in logic. */
	_makeGraphs(_cpuList, _taskList);
	for (auto const &g: _graphs)
		g->draw(size);

	/* Process and draw all plugin-specific shapes. */
	_makePluginShapes(_cpuList, _taskList);
	while (!_shapes.empty()) {
		auto s = _shapes.front();
		_shapes.pop_front();

		s->_size = size;
		s->draw();

		delete s;
	}

	/*
	 * Update and draw the markers. Make sure that the active marker
	 * is drawn on top.
	 */
	_mState->updateMarkers(*_data, this);
	_mState->passiveMarker().draw();
	_mState->activeMarker().draw();
}

/** Reset (empty) the widget. */
void KsGLWidget::reset()
{
	_cpuList = {};
	_taskList = {};
	_data = nullptr;
	_model.reset();
}

/** Check if the widget is empty (not showing anything). */
bool KsGLWidget::isEmpty() const {
	return !_data ||
	       !_data->size() ||
	       (!_cpuList.size() && !_taskList.size());
}

/** Reimplemented event handler used to receive mouse press events. */
void KsGLWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		_posMousePress = _posInRange(event->pos().x());
		_rangeBoundInit(_posMousePress);
	}
}

int KsGLWidget::_getLastTask(struct kshark_trace_histo *histo,
			     int bin, int cpu)
{
	kshark_context *kshark_ctx(nullptr);
	kshark_entry_collection *col;
	int pid;

	if (!kshark_instance(&kshark_ctx))
		return KS_EMPTY_BIN;

	col = kshark_find_data_collection(kshark_ctx->collections,
					  KsUtils::matchCPUVisible,
					  cpu);

	for (int b = bin; b >= 0; --b) {
		pid = ksmodel_get_pid_back(histo, b, cpu, false, col, nullptr);
		if (pid >= 0)
			return pid;
	}

	return ksmodel_get_pid_back(histo, LOWER_OVERFLOW_BIN,
					   cpu,
					   false,
					   col,
					   nullptr);
}

int KsGLWidget::_getLastCPU(struct kshark_trace_histo *histo,
			     int bin, int pid)
{
	kshark_context *kshark_ctx(nullptr);
	kshark_entry_collection *col;
	int cpu;

	if (!kshark_instance(&kshark_ctx))
		return KS_EMPTY_BIN;

	col = kshark_find_data_collection(kshark_ctx->collections,
					  kshark_match_pid,
					  pid);

	for (int b = bin; b >= 0; --b) {
		cpu = ksmodel_get_cpu_back(histo, b, pid, false, col, nullptr);
		if (cpu >= 0)
			return cpu;
	}

	return ksmodel_get_cpu_back(histo, LOWER_OVERFLOW_BIN,
					   pid,
					   false,
					   col,
					   nullptr);

}

/** Reimplemented event handler used to receive mouse move events. */
void KsGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	int bin, cpu, pid;
	size_t row;
	bool ret;

	if (isEmpty())
		return;

	if (_rubberBand.isVisible())
		_rangeBoundStretched(_posInRange(event->pos().x()));

	bin = event->pos().x() - _hMargin;
	cpu = getPlotCPU(event->pos());
	pid = getPlotPid(event->pos());

	ret = _find(bin, cpu, pid, 5, false, &row);
	if (ret) {
		emit found(row);
	} else {
		if (cpu >= 0) {
			pid = _getLastTask(_model.histo(), bin, cpu);
		}

		if (pid > 0) {
			cpu = _getLastCPU(_model.histo(), bin, pid);
		}

		emit notFound(ksmodel_bin_ts(_model.histo(), bin), cpu, pid);
	}
}

/** Reimplemented event handler used to receive mouse release events. */
void KsGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (isEmpty())
		return;

	if (event->button() == Qt::LeftButton) {
		size_t posMouseRel = _posInRange(event->pos().x());
		int min, max;
		if (_posMousePress < posMouseRel) {
			min = _posMousePress - _hMargin;
			max = posMouseRel - _hMargin;
		} else {
			max = _posMousePress - _hMargin;
			min = posMouseRel - _hMargin;
		}

		_rangeChanged(min, max);
	}
}

/** Reimplemented event handler used to receive mouse double click events. */
void KsGLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		_findAndSelect(event);
}

/** Reimplemented event handler used to receive mouse wheel events. */
void KsGLWidget::wheelEvent(QWheelEvent * event)
{
	int zoomFocus;

	if (QApplication::keyboardModifiers() != Qt::ControlModifier ||
	    isEmpty())
		return;

	if (_mState->activeMarker()._isSet &&
	    _mState->activeMarker().isVisible()) {
		/*
		 * Use the position of the marker as a focus point for the
		 * zoom.
		 */
		zoomFocus = _mState->activeMarker()._bin;
	} else {
		/*
		 * Use the position of the mouse as a focus point for the
		 * zoom.
		 */
		zoomFocus = event->pos().x() - _hMargin;
	}

	if (event->delta() > 0) {
		_model.zoomIn(.05, zoomFocus);
	} else {
		_model.zoomOut(.05, zoomFocus);
	}

	_mState->updateMarkers(*_data, this);
}

/** Reimplemented event handler used to receive key press events. */
void KsGLWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->isAutoRepeat())
		return;

	switch (event->key()) {
	case Qt::Key_Plus:
		emit zoomIn();
		return;

	case Qt::Key_Minus:
		emit zoomOut();
		return;

	case Qt::Key_Left:
		emit scrollLeft();
		return;

	case Qt::Key_Right:
		emit scrollRight();
		return;

	default:
		QOpenGLWidget::keyPressEvent(event);
		return;
	}
}

/** Reimplemented event handler used to receive key release events. */
void KsGLWidget::keyReleaseEvent(QKeyEvent *event)
{
	if (event->isAutoRepeat())
		return;

	if(event->key() == Qt::Key_Plus ||
	   event->key() == Qt::Key_Minus ||
	   event->key() == Qt::Key_Left ||
	   event->key() == Qt::Key_Right) {
		emit stopUpdating();
		return;
	}

	QOpenGLWidget::keyPressEvent(event);
	return;
}

/**
 * The maximum number of CPU plots to be shown by default when the GUI starts.
 */
#define KS_MAX_START_PLOTS 16

/**
 * @brief Load and show trace data.
 *
 * @param data: Input location for the KsDataStore object.
 *	  KsDataStore::loadDataFile() must be called first.
 */
void KsGLWidget::loadData(KsDataStore *data)
{
	uint64_t tMin, tMax;
	int nCPUs, nBins;

	_data = data;

	/*
	 * From the size of the widget, calculate the number of bins.
	 * One bin will correspond to one pixel.
	 */
	nBins = width() - _hMargin * 2;

	_model.reset();

	/* Now load the entire set of trace data. */
	tMin = _data->rows()[0]->ts;
	tMax = _data->rows()[_data->size() - 1]->ts;
	ksmodel_set_bining(_model.histo(), nBins, tMin, tMax);
	_model.fill(_data->rows(), _data->size());

	/* Make a default CPU list. All CPUs (or the first N_max) will be plotted. */
	_cpuList = {};

	nCPUs = tep_get_cpus(_data->tep());
	if (nCPUs > KS_MAX_START_PLOTS)
		nCPUs = KS_MAX_START_PLOTS;

	for (int i = 0; i < nCPUs; ++i)
		_cpuList.append(i);

	/* Make a default task list. No tasks will be plotted. */
	_taskList = {};

	loadColors();
	_makeGraphs(_cpuList, _taskList);
}

/**
 * Create a Hash table of Rainbow colors. The sorted Pid values are mapped to
 * the palette of Rainbow colors.
 */
void KsGLWidget::loadColors()
{
	_pidColors.clear();
	_pidColors = KsPlot::getTaskColorTable();
	_cpuColors.clear();
	_cpuColors = KsPlot::getCPUColorTable();
}

/**
 * Position the graphical elements of the marker according to the current
 * position of the graphs inside the GL widget.
 */
void KsGLWidget::setMark(KsGraphMark *mark)
{
	mark->_mark.setDPR(_dpr);
	mark->_mark.setX(mark->_bin + _hMargin);
	mark->_mark.setY(_vMargin / 2 + 2, height() - _vMargin);

	if (mark->_cpu >= 0) {
		mark->_mark.setCPUY(_graphs[mark->_cpu]->getBase());
		mark->_mark.setCPUVisible(true);
	} else {
		mark->_mark.setCPUVisible(false);
	}

	if (mark->_task >= 0) {
		mark->_mark.setTaskY(_graphs[mark->_task]->getBase());
		mark->_mark.setTaskVisible(true);
	} else {
		mark->_mark.setTaskVisible(false);
	}
}

/**
 * @brief Check if a given KernelShark entry is ploted.
 *
 * @param e: Input location for the KernelShark entry.
 * @param graphCPU: Output location for index of the CPU graph to which this
 *		    entry belongs. If such a graph does not exist the outputted
 *		    value is "-1".
 * @param graphTask: Output location for index of the Task graph to which this
 *		     entry belongs. If such a graph does not exist the
 *		     outputted value is "-1".
 */
void KsGLWidget::findGraphIds(const kshark_entry &e,
			      int *graphCPU,
			      int *graphTask)
{
	int graph(0);
	bool cpuFound(false), taskFound(false);

	/*
	 * Loop over all CPU graphs and try to find the one that
	 * contains the entry.
	 */
	for (auto const &c: _cpuList) {
		if (c == e.cpu) {
			cpuFound = true;
			break;
		}
		++graph;
	}

	if (cpuFound)
		*graphCPU = graph;
	else
		*graphCPU = -1;

	/*
	 * Loop over all Task graphs and try to find the one that
	 * contains the entry.
	 */
	graph = _cpuList.count();
	for (auto const &p: _taskList) {
		if (p == e.pid) {
			taskFound = true;
			break;
		}
		++graph;
	}

	if (taskFound)
		*graphTask = graph;
	else
		*graphTask = -1;
}

void KsGLWidget::_drawAxisX(float size)
{
	KsPlot::Point a0(_hMargin, _vMargin / 4), a1(_hMargin, _vMargin / 2);
	KsPlot::Point b0(width() / 2, _vMargin / 4), b1(width() / 2, _vMargin / 2);
	KsPlot::Point c0(width() - _hMargin, _vMargin / 4),
			 c1(width() - _hMargin, _vMargin / 2);

	a0._size = c0._size = _dpr;

	a0.draw();
	c0.draw();
	KsPlot::drawLine(a0, a1, {}, size);
	KsPlot::drawLine(b0, b1, {}, size);
	KsPlot::drawLine(c0, c1, {}, size);
	KsPlot::drawLine(a0, c0, {}, size);
}

void KsGLWidget::_makeGraphs(QVector<int> cpuList, QVector<int> taskList)
{
	/* The very first thing to do is to clean up. */
	for (auto &g: _graphs)
		delete g;
	_graphs.resize(0);

	if (!_data || !_data->size())
		return;

	auto lamAddGraph = [&](KsPlot::Graph *graph) {
		/*
		* Calculate the base level of the CPU graph inside the widget.
		* Remember that the "Y" coordinate is inverted.
		*/
		if (!graph)
			return;

		int base = _vMargin +
			   _vSpacing * _graphs.count() +
			   KS_GRAPH_HEIGHT * (_graphs.count() + 1);

		graph->setBase(base);
		_graphs.append(graph);
	};

	/* Create CPU graphs according to the cpuList. */
	for (auto const &cpu: cpuList)
		lamAddGraph(_newCPUGraph(cpu));

	/* Create Task graphs taskList to the taskList. */
	for (auto const &pid: taskList)
		lamAddGraph(_newTaskGraph(pid));
}

void KsGLWidget::_makePluginShapes(QVector<int> cpuList, QVector<int> taskList)
{
	kshark_context *kshark_ctx(nullptr);
	kshark_event_handler *evt_handlers;
	KsCppArgV cppArgv;

	if (!kshark_instance(&kshark_ctx))
		return;

	cppArgv._histo = _model.histo();
	cppArgv._shapes = &_shapes;

	for (int g = 0; g < cpuList.count(); ++g) {
		cppArgv._graph = _graphs[g];
		evt_handlers = kshark_ctx->event_handlers;
		while (evt_handlers) {
			evt_handlers->draw_func(cppArgv.toC(),
						cpuList[g],
						KSHARK_PLUGIN_CPU_DRAW);

			evt_handlers = evt_handlers->next;
		}
	}

	for (int g = 0; g < taskList.count(); ++g) {
		cppArgv._graph = _graphs[cpuList.count() + g];
		evt_handlers = kshark_ctx->event_handlers;
		while (evt_handlers) {
			evt_handlers->draw_func(cppArgv.toC(),
						taskList[g],
						KSHARK_PLUGIN_TASK_DRAW);

			evt_handlers = evt_handlers->next;
		}
	}
}

KsPlot::Graph *KsGLWidget::_newCPUGraph(int cpu)
{
	/* The CPU graph needs to know only the colors of the tasks. */
	KsPlot::Graph *graph = new KsPlot::Graph(_model.histo(),
						 &_pidColors,
						 &_pidColors);
	graph->setZeroSuppressed(true);

	kshark_context *kshark_ctx(nullptr);
	kshark_entry_collection *col;

	if (!kshark_instance(&kshark_ctx))
		return nullptr;

	graph->setHMargin(_hMargin);
	graph->setHeight(KS_GRAPH_HEIGHT);

	col = kshark_find_data_collection(kshark_ctx->collections,
					  KsUtils::matchCPUVisible,
					  cpu);

	graph->setDataCollectionPtr(col);
	graph->fillCPUGraph(cpu);

	return graph;
}

KsPlot::Graph *KsGLWidget::_newTaskGraph(int pid)
{
	/*
	 * The Task graph needs to know the colors of the tasks and the colors
	 * of the CPUs.
	 */
	KsPlot::Graph *graph = new KsPlot::Graph(_model.histo(),
						 &_pidColors,
						 &_cpuColors);
	kshark_context *kshark_ctx(nullptr);
	kshark_entry_collection *col;

	if (!kshark_instance(&kshark_ctx))
		return nullptr;

	graph->setHMargin(_hMargin);
	graph->setHeight(KS_GRAPH_HEIGHT);

	col = kshark_find_data_collection(kshark_ctx->collections,
					  kshark_match_pid, pid);
	if (!col) {
		/*
		 * If a data collection for this task does not exist,
		 * register a new one.
		 */
		col = kshark_register_data_collection(kshark_ctx,
						      _data->rows(),
						      _data->size(),
						      kshark_match_pid, pid,
						      25);
	}

	/*
	 * Data collections are efficient only when used on graphs, having a
	 * lot of empty bins.
	 * TODO: Determine the optimal criteria to decide whether to use or
	 * not use data collection for this graph.
	 */
	if (_data->size() < 1e6 &&
	    col && col->size &&
	    _data->size() / col->size < 100) {
		/*
		 * No need to use collection in this case. Free the collection
		 * data, but keep the collection registered. This will prevent
		 * from recalculating the same collection next time when this
		 * task is ploted.
		 */
		kshark_reset_data_collection(col);
	}

	graph->setDataCollectionPtr(col);
	graph->fillTaskGraph(pid);

	return graph;
}

/**
 * @brief Find the KernelShark entry under the the cursor.
 *
 * @param point: The position of the cursor.
 * @param variance: The variance of the position (range) in which an entry will
 *		    be searched.
 * @param joined: It True, search also in the associated CPU/Task graph.
 * @param index: Output location for the index of the entry under the cursor.
 * 		 If no entry has been found, the outputted value is zero.
 *
 * @returns True, if an entry has been found, otherwise False.
 */
bool KsGLWidget::find(const QPoint &point, int variance, bool joined,
		      size_t *index)
{
	/*
	 * Get the bin, pid and cpu numbers.
	 * Remember that one bin corresponds to one pixel.
	 */
	int bin = point.x() - _hMargin;
	int cpu = getPlotCPU(point);
	int pid = getPlotPid(point);

	return _find(bin, cpu, pid, variance, joined, index);
}

int KsGLWidget::_getNextCPU(int pid, int bin)
{
	kshark_context *kshark_ctx(nullptr);
	kshark_entry_collection *col;
	int cpu;

	if (!kshark_instance(&kshark_ctx))
		return KS_EMPTY_BIN;

	col = kshark_find_data_collection(kshark_ctx->collections,
					  kshark_match_pid,
					  pid);
	if (!col)
		return KS_EMPTY_BIN;

	for (int i = bin; i < _model.histo()->n_bins; ++i) {
		cpu = ksmodel_get_cpu_front(_model.histo(), i, pid,
					    false, col, nullptr);
		if (cpu >= 0)
			return cpu;
	}

	return KS_EMPTY_BIN;
}

bool KsGLWidget::_find(int bin, int cpu, int pid,
		       int variance, bool joined, size_t *row)
{
	int hSize = _model.histo()->n_bins;
	ssize_t found;

	if (bin < 0 || bin > hSize || (cpu < 0 && pid < 0)) {
		/*
		 * The click is outside of the range of the histogram.
		 * Do nothing.
		 */
		*row = 0;
		return false;
	}

	auto lamGetEntryByCPU = [&](int b) {
		/* Get the first data entry in this bin. */
		found = ksmodel_first_index_at_cpu(_model.histo(),
							   b, cpu);
		if (found < 0) {
			/*
			 * The bin is empty or the entire connect of the bin
			 * has been filtered.
			 */
			return false;
		}

		*row = found;
		return true;
	};

	auto lamGetEntryByPid = [&](int b) {
		/* Get the first data entry in this bin. */
		found = ksmodel_first_index_at_pid(_model.histo(),
							   b, pid);
		if (found < 0) {
			/*
			 * The bin is empty or the entire connect of the bin
			 * has been filtered.
			 */
			return false;
		}

		*row = found;
		return true;
	};

	auto lamFindEntryByCPU = [&](int b) {
		/*
		 * The click is over the CPU graphs. First try the exact
		 * match.
		 */
		if (lamGetEntryByCPU(bin))
			return true;

		/* Now look for a match, nearby the position of the click. */
		for (int i = 1; i < variance; ++i) {
			if (bin + i <= hSize && lamGetEntryByCPU(bin + i))
				return true;

			if (bin - i >= 0 && lamGetEntryByCPU(bin - i))
				return true;
		}

		*row = 0;
		return false;
	};

	auto lamFindEntryByPid = [&](int b) {
		/*
		 * The click is over the Task graphs. First try the exact
		 * match.
		 */
		if (lamGetEntryByPid(bin))
			return true;

		/* Now look for a match, nearby the position of the click. */
		for (int i = 1; i < variance; ++i) {
			if ((bin + i <= hSize) && lamGetEntryByPid(bin + i))
				return true;

			if ((bin - i >= 0) && lamGetEntryByPid(bin - i))
				return true;
		}

		*row = 0;
		return false;
	};

	if (cpu >= 0)
		return lamFindEntryByCPU(bin);

	if (pid >= 0) {
		bool ret = lamFindEntryByPid(bin);

		/*
		 * If no entry has been found and we have a joined search, look
		 * for an entry on the next CPU used by this task.
		 */
		if (!ret && joined) {
			cpu = _getNextCPU(pid, bin);
			ret = lamFindEntryByCPU(bin);
		}

		return ret;
	}

	*row = 0;
	return false;
}

bool KsGLWidget::_findAndSelect(QMouseEvent *event)
{
	size_t row;
	bool found = find(event->pos(), 10, true, &row);

	if (found) {
		emit select(row);
		emit updateView(row, true);
	}

	return found;
}

void KsGLWidget::_rangeBoundInit(int x)
{
	/*
	 * Set the origin of the rubber band that shows the new range. Only
	 * the X coordinate of the origin matters. The Y coordinate will be
	 * set to zero.
	 */
	_rubberBandOrigin.rx() = x;
	_rubberBandOrigin.ry() = 0;

	_rubberBand.setGeometry(_rubberBandOrigin.x(),
				_rubberBandOrigin.y(),
				0, 0);

	/* Make the rubber band visible, although its size is zero. */
	_rubberBand.show();
}

void KsGLWidget::_rangeBoundStretched(int x)
{
	QPoint pos;

	pos.rx() = x;
	pos.ry() = this->height();

	/*
	 * Stretch the rubber band between the origin position and the current
	 * position of the mouse. Only the X coordinate matters. The Y
	 * coordinate will be the height of the widget.
	 */
	if (_rubberBandOrigin.x() < pos.x()) {
		_rubberBand.setGeometry(QRect(_rubberBandOrigin.x(),
					      _rubberBandOrigin.y(),
					      pos.x() - _rubberBandOrigin.x(),
					      pos.y() - _rubberBandOrigin.y()));
	} else {
		_rubberBand.setGeometry(QRect(pos.x(),
					      _rubberBandOrigin.y(),
					      _rubberBandOrigin.x() - pos.x(),
					      pos.y() - _rubberBandOrigin.y()));
	}
}

void KsGLWidget::_rangeChanged(int binMin, int binMax)
{
	size_t nBins = _model.histo()->n_bins;
	int binMark = _mState->activeMarker()._bin;
	uint64_t min, max;

	/* The rubber band is no longer needed. Make it invisible. */
	_rubberBand.hide();

	if ( (binMax - binMin) < 4) {
		/* Most likely this is an accidental click. Do nothing. */
		return;
	}

	/*
	 * Calculate the new range of the histogram. The number of bins will
	 * stay the same.
	 */

	min = ksmodel_bin_ts(_model.histo(), binMin);
	max = ksmodel_bin_ts(_model.histo(), binMax);
	if (max - min < nBins) {
		/*
		 * The range cannot be smaller than the number of bins.
		 * Do nothing.
		 */
		return;
	}

	/* Recalculate the model and update the markers. */
	ksmodel_set_bining(_model.histo(), nBins, min, max);
	_model.fill(_data->rows(), _data->size());
	_mState->updateMarkers(*_data, this);

	/*
	 * If the Marker is inside the new range, make sure that it will
	 * be visible in the table. Note that for this check we use the
	 * bin number of the marker, retrieved before its update.
	 */
	if (_mState->activeMarker()._isSet &&
	    binMark < binMax && binMark > binMin) {
		emit updateView(_mState->activeMarker()._pos, true);
		return;
	}

	/*
	 * Find the first bin which contains unfiltered data and send a signal
	 * to the View widget to make this data visible.
	 */
	for (int bin = 0; bin < _model.histo()->n_bins; ++bin) {
		int64_t row = ksmodel_first_index_at_bin(_model.histo(), bin);
		if (row != KS_EMPTY_BIN &&
		    (_data->rows()[row]->visible & KS_TEXT_VIEW_FILTER_MASK)) {
			emit updateView(row, false);
			return;
		}
	}
}

int KsGLWidget::_posInRange(int x)
{
	int posX;
	if (x < _hMargin)
		posX = _hMargin;
	else if (x > (width() - _hMargin))
		posX = width() - _hMargin;
	else
		posX = x;

	return posX;
}

/** Get the CPU Id of the Graph plotted at given position. */
int KsGLWidget::getPlotCPU(const QPoint &point)
{
	int cpuId, y = point.y();

	if (_cpuList.count() == 0)
		return -1;

	cpuId = (y - _vMargin + _vSpacing / 2) / (_vSpacing + KS_GRAPH_HEIGHT);
	if (cpuId < 0 || cpuId >= _cpuList.count())
		return -1;

	return _cpuList[cpuId];
}

/** Get the CPU Id of the Graph plotted at given position. */
int KsGLWidget::getPlotPid(const QPoint &point)
{
	int pidId, y = point.y();

	if (_taskList.count() == 0)
		return -1;

	pidId = (y - _vMargin -
		     _cpuList.count()*(KS_GRAPH_HEIGHT + _vSpacing) +
		     _vSpacing / 2) / (_vSpacing + KS_GRAPH_HEIGHT);

	if (pidId < 0 || pidId >= _taskList.count())
		return -1;

	return _taskList[pidId];
}
