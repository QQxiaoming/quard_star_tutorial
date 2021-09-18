// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsDualMarker.cpp
 *  @brief   KernelShark Dual Marker.
 */

#include "KsDualMarker.hpp"
#include "KsGLWidget.hpp"

/**
 * Reimplemented handler for mouse press events. Right mouse click events will
 * deselect the corresponding marker.
 */
void KsMarkerButton::mousePressEvent(QMouseEvent *e)
{
	if(e->button() == Qt::RightButton) {
		emit deselect();

		return;
	}

	QPushButton::mousePressEvent(e);
}

/**
 * @brief Create KsGraphMark object.
 *
 * @param s: The Identifier of the marker (state A or state B).
 */
KsGraphMark::KsGraphMark(DualMarkerState s)
: _color(Qt::darkGreen),
  _state(s)
{
	reset();
	_mark._color << _color;
}

/**
 * @brief Create KsGraphMark object.
 *
 * @param s: The Identifier of the marker (state A or state B).
 * @param col: The color of the marker.
 */
KsGraphMark::KsGraphMark(DualMarkerState s, QColor col)
: _color(col),
  _state(s)
{
	reset();
	_mark._color << _color;
}

/** Reset the marker. */
void KsGraphMark::reset()
{
	_isSet = false;
	_bin = -1;
	_cpu = -1;
	_task = -1;
	_pos = 0;

	_mark._visible = false;
}

/**
 * @brief Set the marker.
 *
 * @param data: Input location for the Data Store object.
 * @param histo: Input location for the model descriptor.
 * @param pos: The index inside the data array this marker will points to.
 * @param cpuGraph: The index of the CPU Graph this marker points to.
 * @param taskGraph: The index of the Task Graph this marker points to.
 */
bool KsGraphMark::set(const KsDataStore &data,
		      kshark_trace_histo *histo,
		      size_t pos, int cpuGraph, int taskGraph)
{
	uint8_t visFlags;

	_isSet = true;
	_pos = pos;
	_ts = data.rows()[_pos]->ts;
	visFlags = data.rows()[_pos]->visible;

	if ((visFlags & KS_TEXT_VIEW_FILTER_MASK) &&
	    (visFlags & KS_GRAPH_VIEW_FILTER_MASK))
		_mark.setDashed(false);
	else
		_mark.setDashed(true);

	_cpu = cpuGraph;
	_task = taskGraph;

	if (_ts > histo->max || _ts < histo->min) {
		_bin = -1;
		_mark._visible = false;
		return false;
	}

	_bin = (_ts - histo->min)/histo->bin_size;
	setVisible(true);

	return true;
}

/**
 * @brief Use this function to update the marker when the state of the model
 *	  has changed.
 *
 * @param data: Input location for the Data Store object.
 * @param histo: Input location for the model descriptor.
 */
bool KsGraphMark::update(const KsDataStore &data, kshark_trace_histo *histo)
{
	if (!_isSet)
		return false;

	return set(data, histo, this->_pos, this->_cpu, this->_task);
}

/** Unset the Marker and make it invisible. */
void KsGraphMark::remove()
{
	_isSet = false;
	setVisible(false);
}

/** An operator for getting the opposite state of the marker identifier. */
DualMarkerState operator!(const DualMarkerState &state)
{
	if (state == DualMarkerState::B)
		return DualMarkerState::A;

	return DualMarkerState::B;
}

/** @brief Create a Dual Marker State Machine. */
KsDualMarkerSM::KsDualMarkerSM(QWidget *parent)
: QWidget(parent),
  _buttonA("Marker A", this),
  _buttonB("Marker B", this),
  _labelDeltaDescr("    A,B Delta: ", this),
  _markA(DualMarkerState::A, Qt::darkGreen),
  _markB(DualMarkerState::B, Qt::darkCyan),
  _scCtrlA(this),
  _scCtrlB(this)
{
	QString styleSheetA, styleSheetB;

	_buttonA.setFixedWidth(STRING_WIDTH(" Marker A "));
	_buttonB.setFixedWidth(STRING_WIDTH(" Marker B "));

	for (auto const &l: {&_labelMA, &_labelMB, &_labelDelta}) {
		l->setFrameStyle(QFrame::Panel | QFrame::Sunken);
		l->setStyleSheet("QLabel {background-color : white; color : black}");
		l->setTextInteractionFlags(Qt::TextSelectableByMouse);
		l->setFixedWidth(FONT_WIDTH * 16);
	}

	styleSheetA = "background : " +
		      _markA._color.name() +
		      "; color : white";

	_stateA = new QState;
	_stateA->setObjectName("A");
	_stateA->assignProperty(&_buttonA,
				"styleSheet",
				styleSheetA);

	styleSheetB = "background : " +
		      _markB._color.name() +
		      "; color : white";

	_stateB = new QState;
	_stateB->setObjectName("B");
	_stateB->assignProperty(&_buttonB,
				"styleSheet",
				styleSheetB);

	/* Define transitions from State A to State B. */
	_stateA->addTransition(this,	&KsDualMarkerSM::machineToB, _stateB);

	_scCtrlA.setKey(Qt::CTRL + Qt::Key_A);
	_stateA->addTransition(&_scCtrlB, &QShortcut::activated, _stateB);

	connect(&_scCtrlA,	&QShortcut::activated,
		this,		&KsDualMarkerSM::_doStateA);

	_stateA->addTransition(&_buttonB, &QPushButton::clicked, _stateB);

	connect(&_buttonB,	&QPushButton::clicked,
		this,		&KsDualMarkerSM::_doStateB);

	connect(&_buttonB,	&KsMarkerButton::deselect,
		this,		&KsDualMarkerSM::deselectB);

	/* Define transitions from State B to State A. */
	_stateB->addTransition(this,	&KsDualMarkerSM::machineToA, _stateA);

	_scCtrlB.setKey(Qt::CTRL + Qt::Key_B);
	_stateB->addTransition(&_scCtrlA, &QShortcut::activated, _stateA);

	connect(&_scCtrlB,	&QShortcut::activated,
		this,		&KsDualMarkerSM::_doStateB);

	_stateB->addTransition(&_buttonA, &QPushButton::clicked, _stateA);

	connect(&_buttonA,	&QPushButton::clicked,
		this,		&KsDualMarkerSM::_doStateA);

	connect(&_buttonA,	&KsMarkerButton::deselect,
		this,		&KsDualMarkerSM::deselectA);

	_machine.setGlobalRestorePolicy(QState::RestoreProperties);
	_machine.addState(_stateA);
	_machine.addState(_stateB);
	_machine.setInitialState(_stateA);
	_markState = DualMarkerState::A;
	_machine.start();
}

/**
 * Reset the Mark A and Mark B and clear the information shown by the Marker's
 * toolbar.
 */
void KsDualMarkerSM::reset()
{
	_markA.reset();
	_markB.reset();
	_labelMA.setText("");
	_labelMB.setText("");
	_labelDelta.setText("");
}

/** Restart the Dual Marker State Machine. */
void KsDualMarkerSM::restart()
{
	_machine.stop();
	reset();
	_markState = DualMarkerState::A;
	_machine.start();
}

void KsDualMarkerSM::_doStateA()
{
	if (_markState !=  DualMarkerState::A) {
		_markState = DualMarkerState::A;
		emit markSwitchForView();
	} else if (activeMarker()._isSet) {
		emit updateView(activeMarker()._pos, true);
		emit updateGraph(activeMarker()._pos);
	}
}

void KsDualMarkerSM::_doStateB()
{
	if (_markState !=  DualMarkerState::B) {
		_markState = DualMarkerState::B;
		emit markSwitchForView();
	} else if (activeMarker()._isSet) {
		emit updateView(activeMarker()._pos, true);
		emit updateGraph(activeMarker()._pos);
	}
}

/** Get the Graph marker associated with a given state. */
KsGraphMark &KsDualMarkerSM::getMarker(DualMarkerState s)
{
	if (s == DualMarkerState::A)
		return _markA;

	return _markB;
}

/** Position all buttons and labels of the Dual Marker in a toolbar. */
void KsDualMarkerSM::placeInToolBar(QToolBar *tb)
{
	tb->addWidget(new QLabel("   "));
	tb->addWidget(&_buttonA);
	tb->addWidget(&_labelMA);
	tb->addSeparator();
	tb->addWidget(new QLabel("   "));
	tb->addWidget(&_buttonB);
	tb->addWidget(&_labelMB);
	tb->addSeparator();
	tb->addWidget(&_labelDeltaDescr);
	tb->addWidget(&_labelDelta);
}

/** Set the state of the Dual Marker State Machine. */
void KsDualMarkerSM::setState(DualMarkerState st) {
	if (st == _markState)
		return;

	if (st == DualMarkerState::A) {
		emit machineToA();
		_doStateA();
	}

	if (st == DualMarkerState::B) {
		emit machineToB();
		_doStateB();
	}
}

/**
 * @brief Use this function to update the two marker when the state of the
 *	  model has changed.
 *
 * @param data: Input location for the Data Store object.
 * @param glw: Input location for the OpenGL widget object.
 */
void KsDualMarkerSM::updateMarkers(const KsDataStore &data,
				   KsGLWidget *glw)
{
	if(_markA.update(data, glw->model()->histo()))
		glw->setMark(&_markA);

	if(_markB.update(data, glw->model()->histo()))
		glw->setMark(&_markB);

	updateLabels();
}

/**
 * @brief Use this function to update the labels when the state of the model
 *	  has changed.
 */
void KsDualMarkerSM::updateLabels()
{
	char separator(' ');
	int precision(6); // 1 microsecond precision.

	auto lamSetTimeLabel = [&precision, &separator] (QLabel &l, int64_t t) {
		QString time = KsUtils::Ts2String(t, precision);
		int i = time.indexOf('.') + 4;

		/* Insert separators for milliseconds amd microseconds. */
		while (i < time.size()) {
			time.insert(i, separator);
			i = i + 4;
		}

		l.setText(time);
	};

	// Marker A
	if (_markA._isSet)
		lamSetTimeLabel(_labelMA, _markA._ts);
	else
		_labelMA.clear();

	// Marker B
	if (_markB._isSet)
		lamSetTimeLabel(_labelMB, _markB._ts);
	else
		_labelMB.clear();

	// Delta
	if (_markA._isSet && _markB._isSet) {
		precision = 9; // 1 nanoseconds precision.
		lamSetTimeLabel(_labelDelta, _markB._ts - _markA._ts);
	} else {
		_labelDelta.clear();
	}
}
