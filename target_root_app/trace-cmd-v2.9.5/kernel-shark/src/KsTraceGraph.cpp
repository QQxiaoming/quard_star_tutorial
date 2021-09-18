// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsTraceGraph.cpp
 *  @brief   KernelShark Trace Graph widget.
 */

// KernelShark
#include "KsUtils.hpp"
#include "KsDualMarker.hpp"
#include "KsTraceGraph.hpp"
#include "KsQuickContextMenu.hpp"

/** Create a default (empty) Trace graph widget. */
KsTraceGraph::KsTraceGraph(QWidget *parent)
: QWidget(parent),
  _pointerBar(this),
  _navigationBar(this),
  _zoomInButton("+", this),
  _quickZoomInButton("++", this),
  _zoomOutButton("-", this),
  _quickZoomOutButton("- -", this),
  _scrollLeftButton("<", this),
  _scrollRightButton(">", this),
  _labelP1("Pointer: ", this),
  _labelP2("", this),
  _labelI1("", this),
  _labelI2("", this),
  _labelI3("", this),
  _labelI4("", this),
  _labelI5("", this),
  _scrollArea(this),
  _drawWindow(&_scrollArea),
  _legendWindow(&_drawWindow),
  _legendAxisX(&_drawWindow),
  _labelXMin("", &_legendAxisX),
  _labelXMid("", &_legendAxisX),
  _labelXMax("", &_legendAxisX),
  _glWindow(&_drawWindow),
  _mState(nullptr),
  _data(nullptr),
  _keyPressed(false)
{
	auto lamMakeNavButton = [&](QPushButton *b) {
		b->setMaximumWidth(FONT_WIDTH * 5);

		connect(b,	&QPushButton::released,
			this,	&KsTraceGraph::_stopUpdating);
		_navigationBar.addWidget(b);
	};

	_pointerBar.setMaximumHeight(FONT_HEIGHT * 1.75);
	_pointerBar.setOrientation(Qt::Horizontal);

	_navigationBar.setMaximumHeight(FONT_HEIGHT * 1.75);
	_navigationBar.setMinimumWidth(FONT_WIDTH * 90);
	_navigationBar.setOrientation(Qt::Horizontal);

	_pointerBar.addWidget(&_labelP1);
	_labelP2.setFrameStyle(QFrame::Panel | QFrame::Sunken);
	_labelP2.setStyleSheet("QLabel { background-color : white; color: black}");
	_labelP2.setTextInteractionFlags(Qt::TextSelectableByMouse);
	_labelP2.setFixedWidth(FONT_WIDTH * 16);
	_pointerBar.addWidget(&_labelP2);
	_pointerBar.addSeparator();

	_labelI1.setStyleSheet("QLabel {color : blue;}");
	_labelI2.setStyleSheet("QLabel {color : green;}");
	_labelI3.setStyleSheet("QLabel {color : red;}");
	_labelI4.setStyleSheet("QLabel {color : blue;}");
	_labelI5.setStyleSheet("QLabel {color : green;}");

	_pointerBar.addWidget(&_labelI1);
	_pointerBar.addSeparator();
	_pointerBar.addWidget(&_labelI2);
	_pointerBar.addSeparator();
	_pointerBar.addWidget(&_labelI3);
	_pointerBar.addSeparator();
	_pointerBar.addWidget(&_labelI4);
	_pointerBar.addSeparator();
	_pointerBar.addWidget(&_labelI5);

	_legendAxisX.setFixedHeight(FONT_HEIGHT * 1.5);
	_legendAxisX.setLayout(new QHBoxLayout);
	_legendAxisX.layout()->setSpacing(0);
	_legendAxisX.layout()->setContentsMargins(0, 0, FONT_WIDTH, 0);

	_labelXMin.setAlignment(Qt::AlignLeft);
	_labelXMid.setAlignment(Qt::AlignHCenter);
	_labelXMax.setAlignment(Qt::AlignRight);

	_legendAxisX.layout()->addWidget(&_labelXMin);
	_legendAxisX.layout()->addWidget(&_labelXMid);
	_legendAxisX.layout()->addWidget(&_labelXMax);
	_legendAxisX.setStyleSheet("QLabel { background-color : white; color: black}");

	_drawWindow.setMinimumSize(100, 100);
	_drawWindow.setStyleSheet("QWidget {background-color : white;}");

	_drawLayout.setContentsMargins(0, 0, 0, 0);
	_drawLayout.setSpacing(0);
	_drawLayout.addWidget(&_legendAxisX, 0, 1);
	_drawLayout.addWidget(&_legendWindow, 1, 0);
	_drawLayout.addWidget(&_glWindow, 1, 1);
	_drawWindow.setLayout(&_drawLayout);

	_drawWindow.installEventFilter(this);

	connect(&_glWindow,	&KsGLWidget::select,
		this,		&KsTraceGraph::markEntry);

	connect(&_glWindow,	&KsGLWidget::found,
		this,		&KsTraceGraph::_setPointerInfo);

	connect(&_glWindow,	&KsGLWidget::notFound,
		this,		&KsTraceGraph::_resetPointer);

	connect(&_glWindow,	&KsGLWidget::zoomIn,
		this,		&KsTraceGraph::_zoomIn);

	connect(&_glWindow,	&KsGLWidget::zoomOut,
		this,		&KsTraceGraph::_zoomOut);

	connect(&_glWindow,	&KsGLWidget::scrollLeft,
		this,		&KsTraceGraph::_scrollLeft);

	connect(&_glWindow,	&KsGLWidget::scrollRight,
		this,		&KsTraceGraph::_scrollRight);

	connect(&_glWindow,	&KsGLWidget::stopUpdating,
		this,		&KsTraceGraph::_stopUpdating);

	connect(_glWindow.model(),	&KsGraphModel::modelReset,
		this,			&KsTraceGraph::_updateTimeLegends);

	_glWindow.setContextMenuPolicy(Qt::CustomContextMenu);
	connect(&_glWindow,	&QWidget::customContextMenuRequested,
		this,		&KsTraceGraph::_onCustomContextMenu);

	_scrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_scrollArea.setWidget(&_drawWindow);

	lamMakeNavButton(&_scrollLeftButton);
	connect(&_scrollLeftButton,	&QPushButton::pressed,
		this,			&KsTraceGraph::_scrollLeft);

	lamMakeNavButton(&_zoomInButton);
	connect(&_zoomInButton,		&QPushButton::pressed,
		this,			&KsTraceGraph::_zoomIn);

	lamMakeNavButton(&_zoomOutButton);
	connect(&_zoomOutButton,	&QPushButton::pressed,
		this,			&KsTraceGraph::_zoomOut);

	lamMakeNavButton(&_scrollRightButton);
	connect(&_scrollRightButton,	&QPushButton::pressed,
		this,			&KsTraceGraph::_scrollRight);

	_navigationBar.addSeparator();

	lamMakeNavButton(&_quickZoomInButton);
	connect(&_quickZoomInButton,	&QPushButton::pressed,
		this,			&KsTraceGraph::_quickZoomIn);

	lamMakeNavButton(&_quickZoomOutButton);
	connect(&_quickZoomOutButton,	&QPushButton::pressed,
		this,			&KsTraceGraph::_quickZoomOut);

	_layout.addWidget(&_pointerBar);
	_layout.addWidget(&_navigationBar);
	_layout.addWidget(&_scrollArea);
	this->setLayout(&_layout);
	updateGeom();
}

/**
 * @brief Load and show trace data.
 *
 * @param data: Input location for the KsDataStore object.
 *	  KsDataStore::loadDataFile() must be called first.
 */
void KsTraceGraph::loadData(KsDataStore *data)
{
	_data = data;
	_glWindow.loadData(data);
	_updateGraphLegends();
	updateGeom();
}

/** Connect the KsGLWidget widget and the State machine of the Dual marker. */
void KsTraceGraph::setMarkerSM(KsDualMarkerSM *m)
{
	_mState = m;
	_navigationBar.addSeparator();
	_mState->placeInToolBar(&_navigationBar);
	_glWindow.setMarkerSM(m);
}

/** Reset (empty) the widget. */
void KsTraceGraph::reset()
{
	/* Reset (empty) the OpenGL widget. */
	_glWindow.reset();

	_labelP2.setText("");
	for (auto l1: {&_labelI1, &_labelI2, &_labelI3, &_labelI4, &_labelI5})
		l1->setText("");

	_selfUpdate();
	for (auto l2: {&_labelXMin, &_labelXMid, &_labelXMax})
		l2->setText("");
}

void KsTraceGraph::_selfUpdate()
{
	_updateGraphLegends();
	_updateTimeLegends();
	_markerReDraw();
	_glWindow.model()->update();
	updateGeom();
}

void KsTraceGraph::_zoomIn()
{
	_updateGraphs(GraphActions::ZoomIn);
}

void KsTraceGraph::_zoomOut()
{
	_updateGraphs(GraphActions::ZoomOut);
}

void KsTraceGraph::_quickZoomIn()
{
	if (_glWindow.isEmpty())
		return;

	/* Bin size will be 100 ns. */
	_glWindow.model()->quickZoomIn(100);
	if (_mState->activeMarker()._isSet &&
	    _mState->activeMarker().isVisible()) {
		/*
		 * Use the position of the active marker as
		 * a focus point of the zoom.
		 */
		uint64_t ts = _mState->activeMarker()._ts;
		_glWindow.model()->jumpTo(ts);
	}
}

void KsTraceGraph::_quickZoomOut()
{
	if (_glWindow.isEmpty())
		return;

	_glWindow.model()->quickZoomOut();
}

void KsTraceGraph::_scrollLeft()
{
	_updateGraphs(GraphActions::ScrollLeft);
}

void KsTraceGraph::_scrollRight()
{
	_updateGraphs(GraphActions::ScrollRight);
}

void KsTraceGraph::_stopUpdating()
{
	/*
	 * The user is no longer pressing the action button. Reset the
	 * "Key Pressed" flag. This will stop the ongoing user action.
	 */
	_keyPressed = false;
}

QString KsTraceGraph::_t2str(uint64_t sec, uint64_t usec) {
	QString usecStr;
	QTextStream ts(&usecStr);

	ts.setFieldAlignment(QTextStream::AlignRight);
	ts.setFieldWidth(6);
	ts.setPadChar('0');

	ts << usec;

	return QString::number(sec) + "." + usecStr;
}

void KsTraceGraph::_resetPointer(uint64_t ts, int cpu, int pid)
{
	uint64_t sec, usec;

	kshark_convert_nano(ts, &sec, &usec);
	_labelP2.setText(_t2str(sec, usec));

	if (pid > 0 && cpu >= 0) {
		struct kshark_context *kshark_ctx(NULL);

		if (!kshark_instance(&kshark_ctx))
			return;

		QString comm(tep_data_comm_from_pid(kshark_ctx->pevent, pid));
		comm.append("-");
		comm.append(QString("%1").arg(pid));
		_labelI1.setText(comm);
		_labelI2.setText(QString("CPU %1").arg(cpu));
	} else {
		_labelI1.setText("");
		_labelI2.setText("");
	}

	for (auto const &l: {&_labelI3, &_labelI4, &_labelI5}) {
		l->setText("");
	}
}

void KsTraceGraph::_setPointerInfo(size_t i)
{
	kshark_entry *e = _data->rows()[i];
	QString event(kshark_get_event_name_easy(e));
	QString lat(kshark_get_latency_easy(e));
	QString info(kshark_get_info_easy(e));
	QString comm(kshark_get_task_easy(e));
	int labelWidth, width;
	QString elidedText;
	uint64_t sec, usec;

	kshark_convert_nano(e->ts, &sec, &usec);
	_labelP2.setText(_t2str(sec, usec));

	comm.append("-");
	comm.append(QString("%1").arg(kshark_get_pid_easy(e)));

	_labelI1.setText(comm);
	_labelI2.setText(QString("CPU %1").arg(e->cpu));
	_labelI3.setText(lat);
	_labelI4.setText(event);
	_labelI5.setText(info);
	QCoreApplication::processEvents();

	labelWidth =
		_pointerBar.geometry().right() - _labelI4.geometry().right();
	if (labelWidth > STRING_WIDTH(info) + FONT_WIDTH * 5)
		return;

	/*
	 * The Info string is too long and cannot be displayed on the toolbar.
	 * Try to fit the text in the available space.
	 */
	QFontMetrics metrix(_labelI5.font());
	width = labelWidth - FONT_WIDTH * 3;
	elidedText = metrix.elidedText(info, Qt::ElideRight, width);

	while(labelWidth < STRING_WIDTH(elidedText) + FONT_WIDTH * 5) {
		width -= FONT_WIDTH * 3;
		elidedText = metrix.elidedText(info, Qt::ElideRight, width);
	}

	_labelI5.setText(elidedText);
	_labelI5.setVisible(true);
	QCoreApplication::processEvents();
}

/**
 * @brief Use the active marker to select particular entry.
 *
 * @param row: The index of the entry to be selected by the marker.
 */
void KsTraceGraph::markEntry(size_t row)
{
	int graph, cpuGrId, taskGrId;

	_glWindow.findGraphIds(*_data->rows()[row], &cpuGrId, &taskGrId);

	/*
	 * If a Task graph has been found, this Task graph will be
	 * visible. If no Task graph has been found, make visible
	 * the corresponding CPU graph.
	 */
	if (taskGrId >= 0)
		graph = taskGrId;
	else
		graph = cpuGrId;

	_scrollArea.ensureVisible(0,
				  _legendAxisX.height() +
				  _glWindow.vMargin() +
				  KS_GRAPH_HEIGHT / 2 +
				  graph*(KS_GRAPH_HEIGHT + _glWindow.vSpacing()),
				  50,
				  KS_GRAPH_HEIGHT / 2 + _glWindow.vSpacing() / 2);

	_glWindow.model()->jumpTo(_data->rows()[row]->ts);
	_mState->activeMarker().set(*_data,
				    _glWindow.model()->histo(),
				    row, cpuGrId, taskGrId);

	_mState->updateMarkers(*_data, &_glWindow);
}

void KsTraceGraph::_markerReDraw()
{
	int cpuGrId, taskGrId;
	size_t row;

	if (_mState->markerA()._isSet) {
		row = _mState->markerA()._pos;
		_glWindow.findGraphIds(*_data->rows()[row], &cpuGrId, &taskGrId);
		_mState->markerA().set(*_data,
				       _glWindow.model()->histo(),
				       row, cpuGrId, taskGrId);
	}

	if (_mState->markerB()._isSet) {
		row = _mState->markerB()._pos;
		_glWindow.findGraphIds(*_data->rows()[row], &cpuGrId, &taskGrId);
		_mState->markerB().set(*_data,
				       _glWindow.model()->histo(),
				       row, cpuGrId, taskGrId);
	}
}

/**
 * @brief Redreaw all CPU graphs.
 *
 * @param v: CPU ids to be plotted.
 */
void KsTraceGraph::cpuReDraw(QVector<int> v)
{
	_glWindow._cpuList = v;
	_selfUpdate();
}

/**
 * @brief Redreaw all Task graphs.
 *
 * @param v: Process ids of the tasks to be plotted.
 */
void KsTraceGraph::taskReDraw(QVector<int> v)
{
	_glWindow._taskList = v;
	_selfUpdate();
}

/** Add (and plot) a CPU graph to the existing list of CPU graphs. */
void KsTraceGraph::addCPUPlot(int cpu)
{
	if (_glWindow._cpuList.contains(cpu))
		return;

	_glWindow._cpuList.append(cpu);
	std::sort(_glWindow._cpuList.begin(), _glWindow._cpuList.end());
	_selfUpdate();
}

/** Add (and plot) a Task graph to the existing list of Task graphs. */
void KsTraceGraph::addTaskPlot(int pid)
{
	if (_glWindow._taskList.contains(pid))
		return;

	_glWindow._taskList.append(pid);
	std::sort(_glWindow._taskList.begin(), _glWindow._taskList.end());
	_selfUpdate();
}

/** Remove a CPU graph from the existing list of CPU graphs. */
void KsTraceGraph::removeCPUPlot(int cpu)
{
	if (!_glWindow._cpuList.contains(cpu))
		return;

	_glWindow._cpuList.removeAll(cpu);
	_selfUpdate();
}

/** Remove a Task graph from the existing list of Task graphs. */
void KsTraceGraph::removeTaskPlot(int pid)
{
	if (!_glWindow._taskList.contains(pid))
		return;

	_glWindow._taskList.removeAll(pid);
	_selfUpdate();
}

/** Update the content of all graphs. */
void KsTraceGraph::update(KsDataStore *data)
{
	_glWindow.model()->update(data);
	_selfUpdate();
}

/** Update the geometry of the widget. */
void KsTraceGraph::updateGeom()
{
	int saWidth, saHeight, dwWidth, hMin;

	/* Set the size of the Scroll Area. */
	saWidth = width() - _layout.contentsMargins().left() -
			    _layout.contentsMargins().right();

	saHeight = height() - _pointerBar.height() -
			      _navigationBar.height() -
			      _layout.spacing() * 2 -
			      _layout.contentsMargins().top() -
			      _layout.contentsMargins().bottom();

	_scrollArea.resize(saWidth, saHeight);

	/*
	 * Calculate the width of the Draw Window, taking into account the size
	 * of the scroll bar.
	 */
	dwWidth = _scrollArea.width();
	if (_glWindow.height() + _legendAxisX.height() > _scrollArea.height())
		dwWidth -=
			qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

	/*
	 * Set the height of the Draw window according to the number of
	 * plotted graphs.
	 */
	_drawWindow.resize(dwWidth,
			   _glWindow.height() + _legendAxisX.height());

	/* Set the minimum height of the Graph widget. */
	hMin = _drawWindow.height() +
	       _pointerBar.height() +
	       _navigationBar.height() +
	       _layout.contentsMargins().top() +
	       _layout.contentsMargins().bottom();

	if (hMin > KS_GRAPH_HEIGHT * 8)
		hMin = KS_GRAPH_HEIGHT * 8;

	setMinimumHeight(hMin);

	/*
	 * Now use the height of the Draw Window to fix the maximum height
	 * of the Graph widget.
	 */
	setMaximumHeight(_drawWindow.height() +
			 _pointerBar.height() +
			 _navigationBar.height() +
			 _layout.spacing() * 2 +
			 _layout.contentsMargins().top() +
			 _layout.contentsMargins().bottom() +
			 2);  /* Just a little bit of extra space. This will
			       * allow the scroll bar to disappear when the
			       * widget is extended to maximum.
			       */

	_glWindow.update();
}

void KsTraceGraph::_updateGraphLegends()
{
	QString graphLegends, graphName;
	QVBoxLayout *layout;
	int width = 0;

	if (_legendWindow.layout()) {
		/*
		 * Remove and delete the existing layout of the legend window.
		 */
		QLayoutItem *child;
		while ((child = _legendWindow.layout()->takeAt(0)) != 0) {
			delete child->widget();
			delete child;
		}

		delete _legendWindow.layout();
	}

	layout = new QVBoxLayout;
	layout->setContentsMargins(FONT_WIDTH, 0, 0, 0);
	layout->setSpacing(_glWindow.vSpacing());
	layout->setAlignment(Qt::AlignTop);
	layout->addSpacing(_glWindow.vMargin());

	auto lamMakeName = [&]() {
		QLabel *name = new QLabel(graphName);

		if (width < STRING_WIDTH(graphName))
			width = STRING_WIDTH(graphName);

		name->setAlignment(Qt::AlignBottom);
		name->setStyleSheet("QLabel {background-color : white; color : black}");
		name->setFixedHeight(KS_GRAPH_HEIGHT);
		layout->addWidget(name);
	};

	for (auto const &cpu: _glWindow._cpuList) {
		graphName = QString("CPU %1").arg(cpu);
		lamMakeName();
	}

	for (auto const &pid: _glWindow._taskList) {
		graphName = QString(tep_data_comm_from_pid(_data->tep(),
							   pid));
		graphName.append(QString("-%1").arg(pid));
		lamMakeName();
	}

	_legendWindow.setLayout(layout);
	_legendWindow.setMaximumWidth(width + FONT_WIDTH);
}

void KsTraceGraph::_updateTimeLegends()
{
	uint64_t sec, usec, tsMid;

	kshark_convert_nano(_glWindow.model()->histo()->min, &sec, &usec);
	_labelXMin.setText(_t2str(sec, usec));

	tsMid = (_glWindow.model()->histo()->min +
		 _glWindow.model()->histo()->max) / 2;
	kshark_convert_nano(tsMid, &sec, &usec);
	_labelXMid.setText(_t2str(sec, usec));

	kshark_convert_nano(_glWindow.model()->histo()->max, &sec, &usec);
	_labelXMax.setText(_t2str(sec, usec));
}

/**
 * Reimplemented event handler used to update the geometry of the widget on
 * resize events.
 */
void KsTraceGraph::resizeEvent(QResizeEvent* event)
{
	updateGeom();
}

/**
 * Reimplemented event handler (overriding a virtual function from QObject)
 * used to detect the position of the mouse with respect to the Draw window and
 * according to this position to grab / release the focus of the keyboard. The
 * function has nothing to do with the filtering of the trace events.
 */
bool KsTraceGraph::eventFilter(QObject* obj, QEvent* evt)
{
	if (obj == &_drawWindow && evt->type() == QEvent::Enter)
		_glWindow.setFocus();

	if (obj == &_drawWindow && evt->type() == QEvent::Leave)
		_glWindow.clearFocus();

	return QWidget::eventFilter(obj, evt);
}

void KsTraceGraph::_updateGraphs(GraphActions action)
{
	double k;
	int bin;

	if (_glWindow.isEmpty())
		return;

	/*
	 * Set the "Key Pressed" flag. The flag will stay set as long as the user
	 * keeps the corresponding action button pressed.
	 */
	_keyPressed = true;

	/* Initialize the zooming factor with a small value. */
	k = .01;
	while (_keyPressed) {
		switch (action) {
		case GraphActions::ZoomIn:
			if (_mState->activeMarker()._isSet &&
			    _mState->activeMarker().isVisible()) {
				/*
				 * Use the position of the active marker as
				 * a focus point of the zoom.
				 */
				bin = _mState->activeMarker()._bin;
				_glWindow.model()->zoomIn(k, bin);
			} else {
				/*
				 * The default focus point is the center of the
				 * range interval of the model.
				 */
				_glWindow.model()->zoomIn(k);
			}

			break;

		case GraphActions::ZoomOut:
			if (_mState->activeMarker()._isSet &&
			    _mState->activeMarker().isVisible()) {
				/*
				 * Use the position of the active marker as
				 * a focus point of the zoom.
				 */
				bin = _mState->activeMarker()._bin;
				_glWindow.model()->zoomOut(k, bin);
			} else {
				/*
				 * The default focus point is the center of the
				 * range interval of the model.
				 */
				_glWindow.model()->zoomOut(k);
			}

			break;

		case GraphActions::ScrollLeft:
			_glWindow.model()->shiftBackward(10);
			break;

		case GraphActions::ScrollRight:
			_glWindow.model()->shiftForward(10);
			break;
		}

		/*
		 * As long as the action button is pressed, the zooming factor
		 * will grow smoothly until it reaches a maximum value. This
		 * will have a visible effect of an accelerating zoom.
		 */
		if (k < .25)
			k  *= 1.02;

		_mState->updateMarkers(*_data, &_glWindow);
		_updateTimeLegends();
		QCoreApplication::processEvents();
	}
}

void KsTraceGraph::_onCustomContextMenu(const QPoint &point)
{
	KsQuickMarkerMenu *menu(nullptr);
	int cpu, pid;
	size_t row;
	bool found;

	found = _glWindow.find(point, 20, true, &row);
	if (found) {
		/* KernelShark entry has been found under the cursor. */
		KsQuickContextMenu *entryMenu;
		menu = entryMenu = new KsQuickContextMenu(_data, row,
							  _mState, this);

		connect(entryMenu,	&KsQuickContextMenu::addTaskPlot,
			this,		&KsTraceGraph::addTaskPlot);

		connect(entryMenu,	&KsQuickContextMenu::addCPUPlot,
			this,		&KsTraceGraph::addCPUPlot);

		connect(entryMenu,	&KsQuickContextMenu::removeTaskPlot,
			this,		&KsTraceGraph::removeTaskPlot);

		connect(entryMenu,	&KsQuickContextMenu::removeCPUPlot,
			this,		&KsTraceGraph::removeCPUPlot);
	} else {
		cpu = _glWindow.getPlotCPU(point);
		if (cpu >= 0) {
			/*
			 * This is a CPU plot, but we do not have an entry
			 * under the cursor.
			 */
			KsRmCPUPlotMenu *rmMenu;
			menu = rmMenu = new KsRmCPUPlotMenu(_mState, cpu, this);

			auto lamRmPlot = [&cpu, this] () {
				removeCPUPlot(cpu);
			};

			connect(rmMenu, &KsRmPlotContextMenu::removePlot,
				lamRmPlot);
		}

		pid = _glWindow.getPlotPid(point);
		if (pid >= 0) {
			/*
			 * This is a Task plot, but we do not have an entry
			 * under the cursor.
			 */
			KsRmTaskPlotMenu *rmMenu;
			menu = rmMenu = new KsRmTaskPlotMenu(_mState, pid, this);

			auto lamRmPlot = [&pid, this] () {
				removeTaskPlot(pid);
			};

			connect(rmMenu, &KsRmPlotContextMenu::removePlot,
				lamRmPlot);
		}
	}

	if (menu) {
		connect(menu,	&KsQuickMarkerMenu::deselect,
			this,	&KsTraceGraph::deselect);

		/*
		 * Note that this slot was connected to the
		 * customContextMenuRequested signal of the OpenGL widget.
		 * Because of this the coordinates of the point are given with
		 * respect to the frame of this widget.
		 */
		QPoint global = _glWindow.mapToGlobal(point);
		global.ry() -= menu->sizeHint().height() / 2;

		/*
		 * Shift the menu so that it is not positioned under the mouse.
		 * This will prevent from an accidental selection of the menu
		 * item under the mouse.
		 */
		global.rx() += FONT_WIDTH;

		menu->exec(global);
	}
}
