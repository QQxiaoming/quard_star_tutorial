/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsTraceGraph.hpp
 *  @brief   KernelShark Trace Graph widget.
 */
#ifndef _KS_TRACEGRAPH_H
#define _KS_TRACEGRAPH_H

// KernelShark
#include "KsGLWidget.hpp"

/**
 * Scroll Area class, needed in order to reimplemented the handler for mouse
 * wheel events.
 */
class KsGraphScrollArea : public QScrollArea {
public:
	/** Create a default Scroll Area. */
	explicit KsGraphScrollArea(QWidget *parent = nullptr)
	: QScrollArea(parent) {}

	/**
	 * Reimplemented handler for mouse wheel events. All mouse wheel
	 * events will be ignored.
	 */
	void wheelEvent(QWheelEvent *evt) {
		if (QApplication::keyboardModifiers() != Qt::ControlModifier)
			QScrollArea::wheelEvent(evt);
	}
};

/**
 * The KsTraceViewer class provides a widget for interactive visualization of
 * trace data shown as time-series.
 */
class KsTraceGraph : public QWidget
{
	Q_OBJECT
public:
	explicit KsTraceGraph(QWidget *parent = nullptr);

	void loadData(KsDataStore *data);

	void setMarkerSM(KsDualMarkerSM *m);

	void reset();

	/** Get the KsGLWidget object. */
	KsGLWidget *glPtr() {return &_glWindow;}

	void markEntry(size_t);

	void cpuReDraw(QVector<int>);

	void taskReDraw(QVector<int>);

	void addCPUPlot(int);

	void addTaskPlot(int);

	void removeCPUPlot(int);

	void removeTaskPlot(int);

	void update(KsDataStore *data);

	void updateGeom();

	void resizeEvent(QResizeEvent* event) override;

	bool eventFilter(QObject* obj, QEvent* evt) override;

signals:
	/**
	 * This signal is emitted in the case of a right mouse button click or
	 * in the case of a double click over an empty area (no visible
	 * KernelShark entries).
	 */
	void deselect();

private:

	void _zoomIn();

	void _zoomOut();

	void _quickZoomIn();

	void _quickZoomOut();

	void _scrollLeft();

	void _scrollRight();

	void _stopUpdating();

	void _resetPointer(uint64_t ts, int cpu, int pid);

	void _setPointerInfo(size_t);

	void _updateTimeLegends();

	void _updateGraphLegends();

	void _selfUpdate();

	void _markerReDraw();

	QString _t2str(uint64_t sec, uint64_t usec);

	enum class GraphActions {
		ZoomIn,
		ZoomOut,
		ScrollLeft,
		ScrollRight
	};

	void _updateGraphs(GraphActions action);

	void _onCustomContextMenu(const QPoint &point);

	QToolBar	_pointerBar, _navigationBar;

	QPushButton	_zoomInButton, _quickZoomInButton;
	QPushButton	_zoomOutButton, _quickZoomOutButton;

	QPushButton	_scrollLeftButton, _scrollRightButton;

	QLabel	_labelP1, _labelP2,				  // Pointer
		_labelI1, _labelI2, _labelI3, _labelI4, _labelI5; // Proc. info

	KsGraphScrollArea	_scrollArea;

	QWidget		_drawWindow, _legendWindow, _legendAxisX;

	QLabel		_labelXMin, _labelXMid, _labelXMax;

	KsGLWidget	_glWindow;

	QGridLayout	_drawLayout;

	QVBoxLayout	_layout;

	KsDualMarkerSM	*_mState;

	KsDataStore 	*_data;

	bool		 _keyPressed;
};

#endif // _KS_TRACEGRAPH_H
