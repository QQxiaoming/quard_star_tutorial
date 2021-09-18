/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

 /**
 *  @file    KsGLWidget.hpp
 *  @brief   OpenGL widget for plotting trace graphs.
 */

#ifndef _KS_GLWIDGET_H
#define _KS_GLWIDGET_H

// Qt
#include <QRubberBand>

// KernelShark
#include "KsUtils.hpp"
#include "KsPlotTools.hpp"
#include "KsModels.hpp"
#include "KsDualMarker.hpp"

/**
 * The KsGLWidget class provides a widget for rendering OpenGL graphics used
 * to plot trace graphs.
 */
class KsGLWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
	explicit KsGLWidget(QWidget *parent = NULL);

	~KsGLWidget();

	void initializeGL() override;

	void resizeGL(int w, int h) override;

	void paintGL() override;

	void reset();

	bool isEmpty() const;

	/** Reprocess all graphs. */
	void update() {resizeGL(width(), height());}

	void mousePressEvent(QMouseEvent *event);

	void mouseMoveEvent(QMouseEvent *event);

	void mouseReleaseEvent(QMouseEvent *event);

	void mouseDoubleClickEvent(QMouseEvent *event);

	void wheelEvent(QWheelEvent * event);

	void keyPressEvent(QKeyEvent *event);

	void keyReleaseEvent(QKeyEvent *event);

	void loadData(KsDataStore *data);

	void loadColors();

	/**
	 * Reimplementing the event handler of the focus event, in order to
	 * avoid the update (redrawing) of the graphs every time when the
	 * widget grabs the focus of the keyboard. This is done because we do
	 * not need to redraw, while on the other hand on large data-sets,
	 * redrawing can take a lot of time.
	 */
	void focusInEvent(QFocusEvent* e) override {}

	/**
	 * Reimplementing the event handler of the focus event, in order to
	 * avoid the update (redrawing) of the graphs every time when the
	 * widget releases the focus of the keyboard. This is done because we
	 * do not need to redraw, while on the other hand on large data-sets,
	 * redrawing can take a lot of time.
	 */
	void focusOutEvent(QFocusEvent* e) override {}

	/**
	 * Provide the widget with a pointer to the Dual Marker state machine
	 * object.
	 */
	void setMarkerSM(KsDualMarkerSM *m) {_mState = m;}

	/** Get the KsGraphModel object. */
	KsGraphModel *model() {return &_model;}

	/** Get the number of CPU graphs. */
	int cpuGraphCount() const {return _cpuList.count();}

	/** Get the number of Task graphs. */
	int taskGraphCount() const {return _taskList.count();}

	/** Get the total number of graphs. */
	int graphCount() const {return _cpuList.count() + _taskList.count();}

	/** Get the height of the widget. */
	int height() const
	{
		return graphCount() * (KS_GRAPH_HEIGHT + _vSpacing) +
		       _vMargin * 2;
	}

	/** Get the device pixel ratio. */
	int dpr() const {return _dpr;}

	/** Get the size of the horizontal margin space. */
	int hMargin()	const {return _hMargin;}

	/** Get the size of the vertical margin space. */
	int vMargin()	const {return _vMargin;}

	/** Get the size of the vertical spaceing between the graphs. */
	int vSpacing()	const {return _vSpacing;}

	void setMark(KsGraphMark *mark);

	void findGraphIds(const kshark_entry &e,
			  int *graphCPU,
			  int *graphTask);

	bool find(const QPoint &point, int variance, bool joined,
		  size_t *index);

	int getPlotCPU(const QPoint &point);

	int getPlotPid(const QPoint &point);

	/** CPUs to be plotted. */
	QVector<int>	_cpuList;

	/** Tasks to be plotted. */
	QVector<int>	_taskList;

signals:
	/**
	 * This signal is emitted when the mouse moves over a visible
	 * KernelShark entry.
	 */
	void found(size_t pos);

	/**
	 * This signal is emitted when the mouse moves but there is no visible
	 * KernelShark entry under the cursor.
	 */
	void notFound(uint64_t ts, int cpu, int pid);

	/** This signal is emitted when the Plus key is pressed. */
	void zoomIn();

	/** This signal is emitted when the Minus key is pressed. */
	void zoomOut();

	/** This signal is emitted when the Left Arrow key is pressed. */
	void scrollLeft();

	/** This signal is emitted when the Right Arrow key is pressed. */
	void scrollRight();

	/**
	 * This signal is emitted when one of the 4 Action keys is release
	 * (after being pressed).
	 */
	void stopUpdating();

	/**
	 * This signal is emitted in the case of a double click over a visible
	 * KernelShark entry.
	 */
	void select(size_t pos);

	/**
	 * This signal is emitted when the KsTraceViewer widget needs to be
	 * updated.
	 */
	void updateView(size_t pos, bool mark);

private:
	QVector<KsPlot::Graph*>	_graphs;

	KsPlot::PlotObjList	_shapes;

	KsPlot::ColorTable	_pidColors;

	KsPlot::ColorTable	_cpuColors;

	int		_hMargin, _vMargin;

	unsigned int	_vSpacing;

	KsGraphModel	 _model;

	KsDualMarkerSM	*_mState;

	KsDataStore	*_data;

	QRubberBand	_rubberBand;

	QPoint		_rubberBandOrigin;

	size_t		_posMousePress;

	bool		_keyPressed;

	int 		_dpr;

	void _drawAxisX(float size);

	void _makeGraphs(QVector<int> cpuMask, QVector<int> taskMask);

	KsPlot::Graph *_newCPUGraph(int cpu);

	KsPlot::Graph *_newTaskGraph(int pid);

	void _makePluginShapes(QVector<int> cpuMask, QVector<int> taskMask);

	int _posInRange(int x);

	void _rangeBoundInit(int x);

	void _rangeBoundStretched(int x);

	void _rangeChanged(int binMin, int binMax);

	bool _findAndSelect(QMouseEvent *event);

	bool _find(int bin, int cpu, int pid,
		   int variance, bool joined, size_t *row);

	int _getNextCPU(int pid, int bin);

	int _getLastTask(struct kshark_trace_histo *histo, int bin, int cpu);

	int _getLastCPU(struct kshark_trace_histo *histo, int bin, int pid);

	void _deselect();
};

#endif
