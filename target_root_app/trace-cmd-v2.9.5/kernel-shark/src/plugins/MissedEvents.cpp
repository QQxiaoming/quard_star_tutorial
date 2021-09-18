// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    MissedEvents.cpp
 *  @brief   Plugin for visualization of events, missed due to overflow of the ring buffer.
 */

// C++
#include<iostream>

// KernelShark
#include "libkshark.h"
#include "plugins/missed_events.h"
#include "KsPlotTools.hpp"
#include "KsPlugins.hpp"

using namespace KsPlot;

/**
 * This class represents the graphical element of the KernelShark marker for
 * Missed events.
 */
class MissedEventsMark : public PlotObject {
public:
	/** Create a default Missed events marker. */
	MissedEventsMark() : _base(0, 0), _height(0)
	{
		_color = {0, 0, 255};
		_size = 2;
	}

	/**
	 * @brief Create and position a Missed events marker.
	 *
	 * @param p: Base point of the marker.
	 * @param h: vertical size (height) of the marker.
	 */
	MissedEventsMark(const Point &p, int h)
	: _base(p.x(), p.y()), _height(h)
	{
		_color = {0, 0, 255};
		_size = 2;
	}

	/** Set the Base point of the marker. */
	void setBase(const Point &p) {_base.set(p.x(), p.y());}

	/** Set the vertical size (height) point of the marker. */
	void setHeight(int h) {_height = h;}

private:
	/** Base point of the Mark's line. */
	Point	_base;

	/** The vertical size (height) of the Mark. */
	int	_height;

	void _draw(const Color &col, float size = 1.) const override;
};

void MissedEventsMark::_draw(const Color &col, float size) const
{
	Point p(_base.x(), _base.y() - _height);
	drawLine(_base, p, col, size);

	Rectangle rec;
	rec.setPoint(0, p.x(), p.y());
	rec.setPoint(1, p.x() - _height / 4, p.y());
	rec.setPoint(2, p.x() - _height / 4, p.y() + _height / 4);
	rec.setPoint(3, p.x(), p.y() + _height / 4);
	rec._color = col;
	rec.draw();
}

//! @cond Doxygen_Suppress

#define PLUGIN_MAX_ENTRIES		10000

#define KS_TASK_COLLECTION_MARGIN	25

//! @endcond

static void pluginDraw(kshark_context *kshark_ctx,
		       KsCppArgV *argvCpp,
		       int val, int draw_action)
{
	int height = argvCpp->_graph->getHeight();
	const kshark_entry *entry(nullptr);
	MissedEventsMark *mark;
	ssize_t index;

	int nBins = argvCpp->_graph->size();
	for (int bin = 0; bin < nBins; ++bin) {
		if (draw_action == KSHARK_PLUGIN_TASK_DRAW)
			entry = ksmodel_get_task_missed_events(argvCpp->_histo,
							       bin, val,
							       nullptr,
							       &index);
		if (draw_action == KSHARK_PLUGIN_CPU_DRAW)
			entry = ksmodel_get_cpu_missed_events(argvCpp->_histo,
							      bin, val,
							      nullptr,
							      &index);

		if (entry) {
			mark = new MissedEventsMark(argvCpp->_graph->getBin(bin)._base,
						    height);

			argvCpp->_shapes->push_front(mark);
		}
	}
}

/**
 * @brief Plugin's draw function.
 *
 * @param argv_c: A C pointer to be converted to KsCppArgV (C++ struct).
 * @param val: Process or CPU Id value.
 * @param draw_action: Draw action identifier.
 */
void draw_missed_events(kshark_cpp_argv *argv_c,
			int val, int draw_action)
{
	kshark_context *kshark_ctx(NULL);

	if (!kshark_instance(&kshark_ctx))
		return;

	KsCppArgV *argvCpp = KS_ARGV_TO_CPP(argv_c);

	/*
	 * Plotting the "Missed events" makes sense only in the case of a deep
	 * zoom. Here we set a threshold based on the total number of entries
	 * being visualized by the model.
	 * Don't be afraid to play with different values for this threshold.
	 */
	if (argvCpp->_histo->tot_count > PLUGIN_MAX_ENTRIES)
		return;

	try {
		pluginDraw(kshark_ctx, argvCpp, val, draw_action);
	} catch (const std::exception &exc) {
		std::cerr << "Exception in MissedEvents\n" << exc.what();
	}
}
