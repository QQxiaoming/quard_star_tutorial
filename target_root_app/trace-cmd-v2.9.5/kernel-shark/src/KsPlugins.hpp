/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

/**
  *  @file    KsPlugins.hpp
  *  @brief   KernelShark C++ plugin declarations.
  */

#ifndef _KS_PLUGINS_H
#define _KS_PLUGINS_H

// KernelShark
#include "libkshark-model.h"
#include "KsPlotTools.hpp"

/**
 * Structure representing the vector of C++ arguments of the drawing function
 * of a plugin.
 */
struct KsCppArgV {
	/** Pointer to the model descriptor object. */
	kshark_trace_histo	*_histo;

	/** Pointer to the graph object. */
	KsPlot::Graph		*_graph;

	/**
	 * Pointer to the list of shapes. All shapes created by the plugin
	 * will be added to this list.
	 */
	KsPlot::PlotObjList	*_shapes;

	/**
	 * Convert the "this" pointer of the C++ argument vector into a
	 * C pointer.
	 */
	kshark_cpp_argv *toC()
	{
		return reinterpret_cast<kshark_cpp_argv *>(this);
	}
};

/**
 * Macro used to convert a C pointer into a pointer to KsCppArgV (C++ struct).
 */
#define KS_ARGV_TO_CPP(a) (reinterpret_cast<KsCppArgV *>(a))

#endif
