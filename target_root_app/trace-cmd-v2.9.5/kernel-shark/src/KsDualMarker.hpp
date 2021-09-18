/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsDualMarker.hpp
 *  @brief   KernelShark Dual Marker.
 */

#ifndef _KS_DUAL_MARKER_H
#define _KS_DUAL_MARKER_H

// Qt
#include <QtWidgets>

// KernelShark
#include "KsUtils.hpp"
#include "KsPlotTools.hpp"

/**
 * The Marker Button class provides a button that deselect the corresponding
 * marker in the case of a Right  mouse click.
 */
class KsMarkerButton : public QPushButton
{
	Q_OBJECT
public:
	/**
	 * @brief Create a default button.
	 */
	explicit KsMarkerButton(QWidget *parent = nullptr)
	: QPushButton(parent) {}

	/**
	 * @brief Create a button with text.
	 */
	explicit KsMarkerButton(const QString &text, QWidget *parent = nullptr)
	: QPushButton(text, parent) {}

	void mousePressEvent(QMouseEvent *e);

signals:
	/**
	 * This signal is emitted when the button is click by the Right mouse
	 * button.
	 */
	void deselect();
};

class KsGLWidget;

/** The KsGraphMark represents a marker for KernelShark GUI. */
class KsGraphMark : public QObject
{
	Q_OBJECT
public:
	KsGraphMark() = delete;

	KsGraphMark(DualMarkerState s);

	KsGraphMark(DualMarkerState s, QColor col);

	void reset();

	bool set(const KsDataStore &data,
		 kshark_trace_histo *histo,
		 size_t pos,
		 int cpuGraph,
		 int taskGraph);

	bool update(const KsDataStore &data, kshark_trace_histo *histo);

	/** Is this marker visible. */
	bool isVisible() const {return _mark._visible;}

	/** Draw this marker. */
	void draw() const {_mark.draw();}

	/** Set the visiblity of the marker. */
	void setVisible(bool v) {_mark._visible = v;}

	void remove();

public:
	/** Is this marker set. */
	bool		_isSet;

	/** The number of the bin this marker points to. */
	int		_bin;

	/** The index of the CPU Graph this marker points to. */
	int		_cpu;

	/** The  index of the Task Graph this marker points to. */
	int		_task;

	/** The index inside the data array this marker points to. */
	size_t		_pos;

	/** The timestamp of the marker. */
	uint64_t	_ts;

	/** The RGB color of the marker. */
	QColor		_color;

	/** The Identifier of the marker (A or B). */
	const DualMarkerState	_state;

	/** The graphical element of this marker. */
	KsPlot::Mark		_mark;
};

DualMarkerState operator !(const DualMarkerState &state);

/**
 * The DualMarkerState represents the State Machine of the KernelShark GUI
 * Dual Marker.
 */
class KsDualMarkerSM : public QWidget
{
	Q_OBJECT
public:
	explicit KsDualMarkerSM(QWidget *parent = nullptr);

	void reset();

	void restart();

	void placeInToolBar(QToolBar *tb);

	/** Get the Identifier of the current state of the State Machine. */
	DualMarkerState getState() const {return _markState;}

	void setState(DualMarkerState st);

	KsGraphMark &getMarker(DualMarkerState s);

	/** Get the active marker. */
	KsGraphMark &activeMarker() {return getMarker(_markState);}

	/** Get the passive marker. */
	KsGraphMark &passiveMarker() {return getMarker(!_markState);}

	/** Get the marker A. */
	KsGraphMark &markerA() {return _markA;}

	/** Get the marker B. */
	KsGraphMark &markerB() {return _markB;}

	/** Get a pointer to the State A object. */
	QState *stateAPtr() {return _stateA;}

	/** Get a pointer to the State B object. */
	QState *stateBPtr() {return _stateB;}

	void updateMarkers(const KsDataStore &data,
			   KsGLWidget *glw);

	void updateLabels();

signals:
	/**
	 * This signal is emitted when the Table View has to switch the color
	 * of the selected row.
	 */
	void markSwitchForView();

	/**
	 * This signal is emitted when the Table View has to show a different
	 * entry (row).
	 */
	void updateView(size_t pos, bool mark);

	/**
	 * This signal is emitted when the Graph mark has to show a different
	 * entry.
	 */
	void updateGraph(size_t pos);

	/**
	 * This signal is emitted when the State Machine has to switch to
	 * state A.
	 */
	void machineToA();

	/**
	 * This signal is emitted when the State Machine has to switch to
	 * state B.
	 */
	void machineToB();

	/**
	 * This signal is used to re-emitted the deselect signal of the
	 * Marker A button.
	 */
	void deselectA();

	/**
	 * This signal is used to re-emitted the deselect signal of the
	 * Marker B button.
	 */
	void deselectB();

private:
	KsMarkerButton	 _buttonA;

	KsMarkerButton	 _buttonB;

	QLabel		 _labelMA, _labelMB, _labelDelta;

	QLabel		 _labelDeltaDescr;

	QState		*_stateA;

	QState		*_stateB;

	QStateMachine	 _machine;

	DualMarkerState	 _markState;

	KsGraphMark	 _markA, _markB;

	QShortcut        _scCtrlA, _scCtrlB;

	void _doStateA();

	void _doStateB();
};

#endif
