// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

/**
 *  @file    KsPlotTools.cpp
 *  @brief   KernelShark Plot tools.
 */

// C
#include <math.h>

// C++
#include <algorithm>
#include <vector>

// OpenGL
#include <GL/freeglut.h>
#include <GL/gl.h>

// KernelShark
#include "KsPlotTools.hpp"

namespace KsPlot
{

float Color::_frequency = .75;

/**
 * @brief Create a default color (black).
 */
Color::Color()
{
	_col_c.red = _col_c.green = _col_c.blue = 0;
}

/**
 * @brief Constructs a RGB color object.
 *
 * @param r: The red component of the color.
 * @param g: The green component of the color.
 * @param b: The blue component of the color
 */
Color::Color(uint8_t r, uint8_t g, uint8_t b)
{
	set(r, g, b);
}

/**
 * @brief Constructs a RGB color object.
 *
 * @param rgb: RGB value.
 */
Color::Color(int rgb)
{
	set(rgb);
}

/**
 * @brief Sets the color.
 *
 * @param r: The red component of the color.
 * @param g: The green component of the color.
 * @param b: The blue component of the color
 */
void Color::set(uint8_t r, uint8_t g, uint8_t b)
{
	_col_c.red = r;
	_col_c.green = g;
	_col_c.blue = b;
}

/**
 * @brief Sets the color.
 *
 * @param rgb: RGB value.
 */
void Color::set(int rgb)
{
	int r = rgb & 0xFF;
	int g = (rgb >> 8) & 0xFF;
	int b = (rgb >> 16) & 0xFF;

	set(r, g, b);
}

/**
 * @brief The color is selected from the Rainbow palette.
 *
 * @param n: index of the color inside the Rainbow palette.
 */
void Color::setRainbowColor(int n)
{
	int r = sin(_frequency * n + 0) * 127 + 128;
	int g = sin(_frequency * n + 2) * 127 + 128;
	int b = sin(_frequency * n + 4) * 127 + 128;

	set(r, g, b);
}

/**
 * @brief Create a Hash table of Rainbow colors. The sorted Pid values are
 *	  mapped to the palette of Rainbow colors.
 *
 * @returns ColorTable instance.
 */
ColorTable getTaskColorTable()
{
	struct kshark_context *kshark_ctx(nullptr);
	ColorTable colors;
	int nTasks, pid, *pids, i(0);

	if (!kshark_instance(&kshark_ctx))
		return colors;

	nTasks = kshark_get_task_pids(kshark_ctx, &pids);
	if (!nTasks)
		return colors;

	std::vector<int> temp_pids(pids, pids + nTasks);
	std::sort(temp_pids.begin(), temp_pids.end());

	free(pids);

	if (temp_pids[i] == 0) {
		/* The "Idle" process (pid = 0) will be plotted in black. */
		colors[i++] = {};
	}

	for (; i < nTasks; ++i) {
		pid = temp_pids[i];
		colors[pid].setRainbowColor(i - 1);
	}

	return colors;
}

/**
 * @brief Create a Hash table of Rainbow colors. The CPU Ids are
 *	  mapped to the palette of Rainbow colors.
 *
 * @returns ColorTable instance.
 */
ColorTable getCPUColorTable()
{
	struct kshark_context *kshark_ctx(nullptr);
	ColorTable colors;
	int nCPUs;

	if (!kshark_instance(&kshark_ctx))
		return colors;

	nCPUs =  tep_get_cpus(kshark_ctx->pevent);
	for (int i = 0; i < nCPUs; ++i)
		colors[i].setRainbowColor(i);

	return colors;
}

/**
 * @brief Search the Hash table of Rainbow colors for a particular key (pid).
 *
 * @param colors: Input location for the ColorTable instance.
 * @param pid: the Process Id to search for.
 *
 * @returns The Rainbow color of the key "pid". If "pid" does not exist, the
 *	    returned color is Black.
 */
Color getColor(ColorTable *colors, int pid)
{
	auto item = colors->find(pid);

	if (item != colors->end())
		return item->second;

	return {};
}

/**
 * @brief Create a default Shape.
 */
Shape::Shape()
: _nPoints(0),
  _points(nullptr)
{}

/**
 * @brief Create a Shape defined by "n" points. All points are initialized
 * at (0, 0).
 */
Shape::Shape(int n)
: _nPoints(n),
  _points(new(std::nothrow) ksplot_point[n]())
{
	if (!_points) {
		_size = 0;
		fprintf(stderr,
			"Failed to allocate memory for ksplot_points.\n");
	}
}

/** Copy constructor. */
Shape::Shape(const Shape &s)
: _nPoints(0),
  _points(nullptr)
{
	*this = s;
}

/** Move constructor. */
Shape::Shape(Shape &&s)
: _nPoints(s._nPoints),
  _points(s._points)
{
	s._nPoints = 0;
	s._points = nullptr;
}

/**
* @brief Destroy the Shape object.
*/
Shape::~Shape() {
	delete[] _points;
}

/** Assignment operator. */
void Shape::operator=(const Shape &s)
{
	PlotObject::operator=(s);

	if (s._nPoints != _nPoints) {
		delete[] _points;
		_points = new(std::nothrow) ksplot_point[s._nPoints];
	}

	if (_points) {
		_nPoints = s._nPoints;
		memcpy(_points, s._points,
		       sizeof(_points) * _nPoints);
	} else {
		_nPoints = 0;
		fprintf(stderr,
			"Failed to allocate memory for ksplot_points.\n");
	}
}

/**
 * @brief Set the point of the polygon indexed by "i".
 *
 * @param i: the index of the point to be set.
 * @param x: X coordinate of the point in pixels.
 * @param y: Y coordinate of the point in pixels.
 */
void Shape::setPoint(size_t i, int x, int y)
{
	if (i < _nPoints) {
		_points[i].x = x;
		_points[i].y = y;
	}
}

/**
 * @brief Set the point of the polygon indexed by "i".
 *
 * @param i: the index of the point to be set.
 * @param p: A ksplot_point object used to provide coordinate values.
 */
void Shape::setPoint(size_t i, const ksplot_point &p)
{
	setPoint(i, p.x, p.y);
}

/**
 * @brief Set the point of the polygon indexed by "i".
 *
 * @param i: the index of the point to be set.
 * @param p: A Point object used to provide coordinate values.
 */
void Shape::setPoint(size_t i, const Point &p)
{
	setPoint(i, p.x(), p.y());
}

/**
 * @brief Get the point "i". If the point does not exist, the function returns
 *	  nullptr.
 */
const ksplot_point *Shape::getPoint(size_t i) const
{
	if (i < _nPoints)
		return &_points[i];

	return nullptr;
}

/**
 * @brief Set the horizontal coordinate of the point "i".
 *
 * @param i: the index of the point to be set.
 * @param x: X coordinate of the point in pixels.
 */
void Shape::setPointX(size_t i, int x) {
	if (i < _nPoints)
		_points[i].x = x;
}

/**
 * @brief Set the vertical coordinate of the point "i".
 *
 * @param i: the index of the point to be set.
 * @param y: Y coordinate of the point in pixels.
 */
void Shape::setPointY(size_t i, int y) {
	if (i < _nPoints)
		_points[i].y = y;
}

/**
 * @brief Get the horizontal coordinate of the point "i". If the point does
 * 	  not exist, the function returns 0.
 */
int Shape::getPointX(size_t i) const {
	if (i < _nPoints)
		return _points[i].x;

	return 0;
}

/**
 * @brief Get the vertical coordinate of the point "i". If the point does
 * 	  not exist, the function returns 0.
 */
int Shape::getPointY(size_t i) const {
	if (i < _nPoints)
		return _points[i].y;

	return 0;
}

/** @brief Create a default Point. The point is initialized at (0, 0). */
Point::Point()
: Shape(1)
{}

/**
 * @brief Create a point.
 *
 * @param x: X coordinate of the point in pixels.
 * @param y: Y coordinate of the point in pixels.
 */
Point::Point(int x, int y)
: Shape(1)
{
	setPoint(0, x, y);
}

void Point::_draw(const Color &col, float size) const
{
	if (_nPoints == 1)
		ksplot_draw_point(_points, col.color_c_ptr(), size);
}

/**
 * @brief Draw a line between point "a" and point "b".
 *
 * @param a: The first finishing point of the line.
 * @param b: The second finishing point of the line.
 * @param col: The color of the line.
 * @param size: The size of the line.
 */
void drawLine(const Point &a, const Point &b,
	      const Color &col, float size)
{
	ksplot_draw_line(a.point_c_ptr(),
			 b.point_c_ptr(),
			 col.color_c_ptr(),
			 size);
}

/**
 * @brief Draw a dashed line between point "a" and point "b".
 *
 * @param a: The first finishing point of the line.
 * @param b: The second finishing point of the line.
 * @param col: The color of the line.
 * @param size: The size of the line.
 * @param period: The period of the dashed line.
 */
void drawDashedLine(const Point &a, const Point &b,
		    const Color &col, float size, float period)
{
	int dx = b.x() - a.x(), dy = b.y() - a.y();
	float mod = sqrt(dx * dx + dy * dy);
	int n = mod / period;
	Point p1, p2;

	for (int i = 0; i < n; ++i) {
		p1.setX(a.x() + (i + .25) * dx / n);
		p1.setY(a.y() + (i + .25) * dy / n);
		p2.setX(a.x() + (i + .75) * dx / n);
		p2.setY(a.y() + (i + .75) * dy / n);
		drawLine(p1, p2, col, size);
	}
}

/** @brief Create a default line. The two points are initialized at (0, 0). */
Line::Line()
: Shape(2)
{}

/**
 * @brief Create a line between the point "a" and point "b".
 *
 * @param a: first finishing point of the line.
 * @param b: second finishing point of the line.
 */
Line::Line(const Point &a, const Point &b)
: Shape(2)
{
	setPoint(0, a.x(), a.y());
	setPoint(1, b.x(), b.y());
}

void Line::_draw(const Color &col, float size) const
{
	if (_nPoints == 2)
		ksplot_draw_line(&_points[0], &_points[1],
				 col.color_c_ptr(), size);
}

/**
 * @brief Create a default polygon. All points are initialized at (0, 0).
 *
 * @param n: The number of edges of the polygon.
 */
Polygon::Polygon(size_t n)
: Shape(n),
  _fill(true)
{}

void Polygon::_draw(const Color &col, float size) const
{
	if (_fill)
		ksplot_draw_polygon(_points, _nPoints,
				    col.color_c_ptr(),
				    size);
	else
		ksplot_draw_polygon_contour(_points, _nPoints,
					    col.color_c_ptr(),
					    size);
}

/**
 * @brief Create a default Mark.
 */
Mark::Mark()
: _dashed(false)
{
	_visible = false;
	_cpu._color = Color(225, 255, 100);
	_cpu._size = 5.5f;
	_task._color = Color(0, 255, 0);
	_task._size = 5.5f;
}

void Mark::_draw(const Color &col, float size) const
{
	if (_dashed)
		drawDashedLine(_a, _b, col, size, 3 * _cpu._size / size);
	else
		drawLine(_a, _b, col, size);

	_cpu.draw();
	_task.draw();
}

/**
 * @brief Set the device pixel ratio.
 *
 * @param dpr: device pixel ratio value.
 */
void Mark::setDPR(int dpr)
{
	_size = 1.5 * dpr;
	_task._size = _cpu._size = 1.5 + 4.0 * dpr;
}

/**
 * @brief Set the X coordinate (horizontal) of the Mark.
 *
 * @param x: X coordinate of the Makr in pixels.
 */
void Mark::setX(int x)
{
	_a.setX(x);
	_b.setX(x);
	_cpu.setX(x);
	_task.setX(x);
}

/**
 * @brief Set the Y coordinates (vertical) of the Mark's finishing points.
 *
 * @param yA: Y coordinate of the first finishing point of the Mark's line.
 * @param yB: Y coordinate of the second finishing point of the Mark's line.
 */
void Mark::setY(int yA, int yB)
{
	_a.setY(yA);
	_b.setY(yB);
}

/**
 * @brief Set the Y coordinates (vertical) of the Mark's CPU points.
 *
 * @param yCPU: Y coordinate of the Mark's CPU point.
 */
void Mark::setCPUY(int yCPU)
{
	_cpu.setY(yCPU);
}

/**
 * @brief Set the visiblity of the Mark's CPU points.
 *
 * @param v: If True, the CPU point will be visible.
 */
void Mark::setCPUVisible(bool v)
{
	_cpu._visible = v;
}

/**
 * @brief Set the Y coordinates (vertical) of the Mark's Task points.
 *
 * @param yTask: Y coordinate of the Mark's Task point.
 */
void Mark::setTaskY(int yTask)
{
	_task.setY(yTask);
}

/**
 * @brief Set the visiblity of the Mark's Task points.
 *
 * @param v: If True, the Task point will be visible.
 */
void Mark::setTaskVisible(bool v)
{
	_task._visible = v;
}

/**
 * @brief Create a default Bin.
 */
Bin::Bin()
: _idFront(KS_EMPTY_BIN),
  _idBack(KS_EMPTY_BIN)
{}

void Bin::_draw(const Color &col, float size) const
{
	drawLine(_base, _val, col, size);
}

/**
 * @brief Draw only the "val" Point og the Bin.
 *
 * @param size: The size of the point.
 */
void Bin::drawVal(float size)
{
	_val._size = size;
	_val.draw();
}

/**
 * @brief Create a default (empty) Graph.
 */
Graph::Graph()
: _histoPtr(nullptr),
  _bins(nullptr),
  _size(0),
  _hMargin(30),
  _collectionPtr(nullptr),
  _binColors(nullptr),
  _ensembleColors(nullptr),
  _zeroSuppress(false)
{}

/**
 * @brief Create a Graph to represent the state of the Vis. model.
 *
 * @param histo: Input location for the model descriptor.
 * @param bct: Input location for the Hash table of bin's colors.
 * @param ect: Input location for the Hash table of ensemble's colors.
 */
Graph::Graph(kshark_trace_histo *histo, KsPlot::ColorTable *bct, KsPlot::ColorTable *ect)
: _histoPtr(histo),
  _bins(new(std::nothrow) Bin[histo->n_bins]),
  _size(histo->n_bins),
  _hMargin(30),
  _collectionPtr(nullptr),
  _binColors(bct),
  _ensembleColors(ect),
  _zeroSuppress(false)
{
	if (!_bins) {
		_size = 0;
		fprintf(stderr, "Failed to allocate memory graph's bins.\n");
	}

	_initBins();
}

/**
 * @brief Destroy the Graph object.
 */
Graph::~Graph()
{
	delete[] _bins;
}

void Graph::_initBins()
{
	for (int i = 0; i < _size; ++i) {
		_bins[i]._base.setX(i + _hMargin);
		_bins[i]._base.setY(0);
		_bins[i]._val.setX(_bins[i]._base.x());
		_bins[i]._val.setY(_bins[i]._base.y());
	}
}

/**
 *  Get the number of bins.
 */
int Graph::size()
{
	return _size;
}

/**
 * @brief Reinitialize the Graph according to the Vis. model.
 *
 * @param histo: Input location for the model descriptor.
 */
void Graph::setModelPtr(kshark_trace_histo *histo)
{
	if (_size != histo->n_bins) {
		delete[] _bins;
		_size = histo->n_bins;
		_bins = new(std::nothrow) Bin[_size];
		if (!_bins) {
			_size = 0;
			fprintf(stderr,
				"Failed to allocate memory graph's bins.\n");
		}
	}

	_histoPtr = histo;
	_initBins();
}

/**
 * @brief This function will set the Y (vertical) coordinate of the Graph's
 *	  base. It is safe to use this function even if the Graph contains
 *	  data.
 *
 * @param b: Y coordinate of the Graph's base in pixels.
 */
void Graph::setBase(int b)
{
	int mod;

	if (!_size)
		return;

	if (b == _bins[0]._base.y()) // Nothing to do.
		return;

	for (int i = 0; i < _size; ++i) {
		mod = _bins[i].mod();
		_bins[i]._base.setY(b);
		_bins[i]._val.setY(b + mod);
	}
}

/**
 * @brief Set the vertical size (height) of the Graph.
 *
 * @param h: the height of the Graph in pixels.
 */
void Graph::setHeight(int h)
{
	_height = h;
}

/**
 * @brief Set the size of the white space added on both sides of the Graph.
 *
 * @param hMargin: the size of the white space in pixels.
 */
void Graph::setHMargin(int hMargin)
{
	if (!_size)
		return;

	if (hMargin == _bins[0]._base.x()) // Nothing to do.
		return;

	for (int i = 0; i < _size; ++i) {
		_bins[i]._base.setX(i + hMargin);
		_bins[i]._val.setX(_bins[i]._base.x());
	}

	_hMargin = hMargin;
}

/**
 * @brief Set the value of a given bin.
 *
 * @param bin: Bin Id.
 * @param val: Bin height in pixels.
 */
void Graph::setBinValue(int bin, int val)
{
	_bins[bin].setVal(val);
}

/**
 * @brief Set the Process Id (Front and Back) a given bin.
 *
 * @param bin: Bin Id.
 * @param pidF: The Process Id detected at the from (first in time) edge of
 *		the bin.
 * @param pidB: The Process Id detected at the back (last in time) edge of
 *		the bin.
 */
void Graph::setBinPid(int bin, int pidF, int pidB)
{
	_bins[bin]._idFront = pidF;
	_bins[bin]._idBack = pidB;
}

/**
 * @brief Set the color of a given bin.
 *
 * @param bin: Bin Id.
 * @param col: the color of the bin.
 */
void Graph::setBinColor(int bin, const Color &col)
{
	_bins[bin]._color = col;
}

/**
 * @brief Set the visiblity mask of a given bin.
 *
 * @param bin: Bin Id.
 * @param m: the visiblity mask.
 */
void Graph::setBinVisMask(int bin, uint8_t m)
{
	_bins[bin]._visMask = m;
}

/**
 * @brief Set all fields of a given bin.
 *
 * @param bin: Bin Id.
 * @param pidF: The Process Id detected at the from (first in time) edge of
 *		the bin.
 * @param pidB: The Process Id detected at the back (last in time) edge of
 *		the bin.
 * @param col: the color of the bin.
 * @param m: the visiblity mask.
 */
void Graph::setBin(int bin, int pidF, int pidB, const Color &col, uint8_t m)
{
	setBinPid(bin, pidF, pidB);
	setBinValue(bin, _height * .7);
	setBinColor(bin, col);
	setBinVisMask(bin, m);
}

/**
 * @brief Process a CPU Graph.
 *
 * @param cpu: The CPU core.
 */
void Graph::fillCPUGraph(int cpu)
{
	struct kshark_entry *eFront;
	int pidFront(0), pidBack(0);
	int pidBackNoFilter;
	uint8_t visMask;
	ssize_t index;
	int bin;

	auto lamGetPid = [&] (int bin)
	{
		eFront = nullptr;

		pidFront = ksmodel_get_pid_front(_histoPtr, bin,
							    cpu,
							    true,
							    _collectionPtr,
							    &index);

		if (index >= 0)
			eFront = _histoPtr->data[index];

		pidBack = ksmodel_get_pid_back(_histoPtr, bin,
							  cpu,
							  true,
							  _collectionPtr,
							  nullptr);

		pidBackNoFilter =
			ksmodel_get_pid_back(_histoPtr, bin,
						       cpu,
						       false,
						       _collectionPtr,
						       nullptr);

		if (pidBack != pidBackNoFilter)
			pidBack = KS_FILTERED_BIN;

		visMask = 0x0;
		if (eFront) {
			if (!(eFront->visible & KS_EVENT_VIEW_FILTER_MASK) &&
			    ksmodel_cpu_visible_event_exist(_histoPtr, bin,
								       cpu,
								       _collectionPtr,
								       &index)) {
				visMask = _histoPtr->data[index]->visible;
			} else {
				visMask = eFront->visible;
			}
		}
	};

	auto lamSetBin = [&] (int bin)
	{
		if (pidFront != KS_EMPTY_BIN || pidBack != KS_EMPTY_BIN) {
			/* This is a regular process. */
			setBin(bin, pidFront, pidBack,
			       getColor(_binColors, pidFront), visMask);
		} else {
			/* The bin contens no data from this CPU. */
			setBinPid(bin, KS_EMPTY_BIN, KS_EMPTY_BIN);
		}
	};

	/*
	 * Check the content of the very first bin and see if the CPU is
	 * active.
	 */
	bin = 0;
	lamGetPid(bin);
	if (pidFront >= 0) {
		/*
		 * The CPU is active and this is a regular process.
		 * Set this bin.
		 */
		lamSetBin(bin);
	} else {
		/*
		 * No data from this CPU in the very first bin. Use the Lower
		 * Overflow Bin to retrieve the Process Id (if any). First
		 * get the Pid back, ignoring the filters.
		 */
		pidBackNoFilter = ksmodel_get_pid_back(_histoPtr,
						       LOWER_OVERFLOW_BIN,
						       cpu,
						       false,
						       _collectionPtr,
						       nullptr);

		/* Now get the Pid back, applying filters. */
		pidBack = ksmodel_get_pid_back(_histoPtr,
					       LOWER_OVERFLOW_BIN,
					       cpu,
					       true,
					       _collectionPtr,
					       nullptr);

		if (pidBack != pidBackNoFilter) {
			/* The Lower Overflow Bin ends with filtered data. */
			setBinPid(bin, KS_FILTERED_BIN, KS_FILTERED_BIN);
		} else {
			/*
			 * The Lower Overflow Bin ends with data which has
			 * to be plotted.
			 */
			setBinPid(bin, pidBack, pidBack);
		}
	}

	/*
	 * The first bin is already processed. The loop starts from the second
	 * bin.
	 */
	for (bin = 1; bin < _histoPtr->n_bins; ++bin) {
		/*
		 * Check the content of this bin and see if the CPU is active.
		 * If yes, retrieve the Process Id.
		 */
		lamGetPid(bin);
		lamSetBin(bin);
	}
}

/**
 * @brief Process a Task Graph.
 *
 * @param pid: The Process Id of the Task.
 */
void Graph::fillTaskGraph(int pid)
{
	int cpuFront, cpuBack(0), pidFront(0), pidBack(0), lastCpu(-1), bin(0);
	struct kshark_entry *eFront;
	uint8_t visMask;
	ssize_t index;

	auto lamSetBin = [&] (int bin)
	{
		if (cpuFront >= 0) {
			KsPlot::Color col = getColor(_binColors, pid);

			/* Data from the Task has been found in this bin. */
			if (pid == pidFront && pid == pidBack) {
				/* No data from other tasks in this bin. */
				setBin(bin, cpuFront, cpuBack, col, visMask);
			} else if (pid != pidFront && pid != pidBack) {
				/*
				 * There is some data from other tasks at both
				 * front and back sides of this bin. But we
				 * still want to see this bin drawn.
				 */
				setBin(bin, cpuFront, KS_FILTERED_BIN, col,
				       visMask);
			} else {
				if (pidFront != pid) {
					/*
					 * There is some data from another
					 * task at the front side of this bin.
					 */
					cpuFront = KS_FILTERED_BIN;
				}

				if (pidBack != pid) {
					/*
					 * There is some data from another
					 * task at the back side of this bin.
					 */
					cpuBack = KS_FILTERED_BIN;
				}

				setBin(bin, cpuFront, cpuBack, col, visMask);
			}

			lastCpu = cpuBack;
		} else {
			/*
			 * No data from the Task in this bin. Check the CPU,
			 * previously used by the task. We are looking for
			 * data from another task running on the same CPU,
			 * hence we cannot use the collection of this task.
			 */
			int cpuPid = ksmodel_get_pid_back(_histoPtr,
							  bin,
							  lastCpu,
							  false,
							  nullptr, // No collection
							  nullptr);

			if (cpuPid != KS_EMPTY_BIN) {
				/*
				 * If the CPU is active and works on another
				 * task break the graph here.
				 */
				setBinPid(bin, KS_FILTERED_BIN, KS_EMPTY_BIN);
			} else {
				/*
				 * No data from this CPU in the bin.
				 * Continue the graph.
				 */
				setBinPid(bin, KS_EMPTY_BIN, KS_EMPTY_BIN);
			}
		}
	};

	auto lamGetPidCPU = [&] (int bin)
	{
		eFront = nullptr;
		/* Get the CPU used by this task. */
		cpuFront = ksmodel_get_cpu_front(_histoPtr, bin,
						 pid,
						 false,
						 _collectionPtr,
						 &index);
		if (index >= 0)
			eFront = _histoPtr->data[index];

		cpuBack = ksmodel_get_cpu_back(_histoPtr, bin,
					       pid,
					       false,
					       _collectionPtr,
					       nullptr);

		if (cpuFront < 0) {
			pidFront = pidBack = cpuFront;
		} else {
			/*
			 * Get the process Id at the begining and at the end
			 * of the bin.
			 */
			pidFront = ksmodel_get_pid_front(_histoPtr,
							 bin,
							 cpuFront,
							 false,
							 _collectionPtr,
							 nullptr);

			pidBack = ksmodel_get_pid_back(_histoPtr,
						       bin,
						       cpuBack,
						       false,
						       _collectionPtr,
						       nullptr);

			visMask = 0x0;
			if (eFront) {
				if (!(eFront->visible & KS_EVENT_VIEW_FILTER_MASK) &&
				    ksmodel_task_visible_event_exist(_histoPtr,
								     bin,
								     pid,
								     _collectionPtr,
								     &index)) {
					visMask = _histoPtr->data[index]->visible;
				} else {
					visMask = eFront->visible;
				}
			}
		}
	};

	/*
	 * Check the content of the very first bin and see if the Task is
	 * active.
	 */
	lamGetPidCPU(bin);

	if (cpuFront >= 0) {
		/* The Task is active. Set this bin. */
		lamSetBin(bin);
	} else {
		/*
		 * No data from this Task in the very first bin. Use the Lower
		 * Overflow Bin to retrieve the CPU used by the task (if any).
		 */
		cpuFront = ksmodel_get_cpu_back(_histoPtr, LOWER_OVERFLOW_BIN, pid,
					   false, _collectionPtr, nullptr);
		if (cpuFront >= 0) {
			/*
			 * The Lower Overflow Bin contains data from this Task.
			 * Now look again in the Lower Overflow Bin and Bim 0
			 * and find the Pid of the last active task on the same
			 * CPU.
			 */
			int pidCpu0, pidCpuLOB;

			pidCpu0 = ksmodel_get_pid_back(_histoPtr,
						       0,
						       cpuFront,
						       false,
						       _collectionPtr,
						       nullptr);

			pidCpuLOB = ksmodel_get_pid_back(_histoPtr,
							 LOWER_OVERFLOW_BIN,
							 cpuFront,
							 false,
							 _collectionPtr,
							 nullptr);
			if (pidCpu0 < 0 && pidCpuLOB == pid) {
				/*
				 * The Task is the last one running on this
				 * CPU. Set the Pid of the bin. In this case
				 * the very first bin is empty but we derive
				 * the Process Id from the Lower Overflow Bin.
				 */
				setBinPid(bin, cpuFront, cpuFront);
				lastCpu = cpuFront;
			} else {
				setBinPid(bin, KS_EMPTY_BIN, KS_EMPTY_BIN);
			}
		}
	}

	/*
	 * The first bin is already processed. The loop starts from the second
	 * bin.
	 */
	for (bin = 1; bin < _histoPtr->n_bins; ++bin) {
		lamGetPidCPU(bin);

		/* Set the bin accordingly. */
		lamSetBin(bin);
	}
}

/**
 * @brief Draw the Graph
 *
 * @param size: The size of the lines of the individual Bins.
 */
void Graph::draw(float size)
{
	int lastPid(-1), b(0), boxH(_height * .3);
	Rectangle taskBox;

	/*
	 * Start by drawing a line between the base points of the first and
	 * the last bin.
	 */
	drawLine(_bins[0]._base, _bins[_size - 1]._base, {}, size);

	/* Draw as vartical lines all bins containing data. */
	for (int i = 0; i < _size; ++i)
		if (_bins[i]._idFront >= 0 || _bins[i]._idBack >= 0)
			if (_bins[i]._visMask & KS_EVENT_VIEW_FILTER_MASK) {
				_bins[i]._size = size;
				_bins[i].draw();
			}

	auto lamCheckEnsblVal = [this] (int v) {
		return v > 0 || (v == 0 && !this->_zeroSuppress);
	};

	/*
	 * Draw colored boxes for processes. First find the first bin, which
	 * contains data and determine its PID.
	 */
	for (; b < _size; ++b) {
		if (lamCheckEnsblVal(_bins[b]._idBack)) {
			lastPid = _bins[b]._idFront;
			/*
			 * Initialize a box starting from this bin.
			 * The color of the taskBox corresponds to the Pid
			 * of the process.
			 */
			taskBox._color = getColor(_ensembleColors, lastPid);
			taskBox.setPoint(0, _bins[b]._base.x(),
					_bins[b]._base.y() - boxH);
			taskBox.setPoint(1, _bins[b]._base.x(),
					_bins[b]._base.y());
			break;
		}
	}

	for (; b < _size; ++b) {
		if (_bins[b]._idFront == KS_EMPTY_BIN &&
		    _bins[b]._idBack == KS_EMPTY_BIN) {
			/*
			 * This bin is empty. If a colored taskBox is already
			 * initialized, it will be extended.
			 */
			continue;
		}

		if (_bins[b]._idFront != _bins[b]._idBack ||
		    _bins[b]._idFront != lastPid ||
		    _bins[b]._idBack  != lastPid) {
			/* A new process starts here. */
			if (b > 0 && lamCheckEnsblVal(lastPid)) {
				/*
				 * There is another process running up to this
				 * point. Close its colored box here and draw.
				 */
				taskBox.setPoint(3, _bins[b]._base.x() - 1,
						_bins[b]._base.y() - boxH);
				taskBox.setPoint(2, _bins[b]._base.x() - 1,
						_bins[b]._base.y());
				taskBox.draw();
			}

			if (lamCheckEnsblVal(_bins[b]._idBack)) {
				/*
				 * This is a regular process. Initialize
				 * colored box starting from this bin.
				 */
				taskBox._color = getColor(_ensembleColors,
							 _bins[b]._idBack);

				taskBox.setPoint(0, _bins[b]._base.x() - 1,
						_bins[b]._base.y() - boxH);
				taskBox.setPoint(1, _bins[b]._base.x() - 1,
						_bins[b]._base.y());
			}

			lastPid = _bins[b]._idBack;
		}
	}

	if (lamCheckEnsblVal(lastPid) > 0) {
		/*
		 * This is the end of the Graph and we have a process running.
		 * Close its colored box and draw.
		 */
		taskBox.setPoint(3, _bins[_size - 1]._base.x(),
				_bins[_size - 1]._base.y() - boxH);
		taskBox.setPoint(2, _bins[_size - 1]._base.x(),
				_bins[_size - 1]._base.y());
		taskBox.draw();
	}
}

}; // KsPlot
