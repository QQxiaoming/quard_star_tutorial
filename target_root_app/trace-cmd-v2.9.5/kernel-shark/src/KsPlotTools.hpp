/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

 /**
 *  @file    KsPlotTools.hpp
 *  @brief   KernelShark Plot tools.
 */

#ifndef _KS_PLOT_TOOLS_H
#define _KS_PLOT_TOOLS_H

// C++
#include <forward_list>
#include <unordered_map>

// KernelShark
#include "libkshark.h"
#include "libkshark-plot.h"
#include "libkshark-model.h"

namespace KsPlot {

/** This class represents a RGB color. */
class Color {
public:
	Color();

	Color(uint8_t r, uint8_t g, uint8_t b);

	Color(int rgb);

	/** @brief Get the Red coordinate of the color. */
	uint8_t r() const {return _col_c.red;}

	/** @brief Get the Green coordinate of the color. */
	uint8_t g() const {return _col_c.green;}

	/** @brief Get the Blue coordinate of the color. */
	uint8_t b() const {return _col_c.blue;}

	void set(uint8_t r, uint8_t g, uint8_t b);

	void set(int rgb);

	void setRainbowColor(int n);

	/**
	 * @brief Get the C struct defining the RGB color.
	 */
	const ksplot_color *color_c_ptr() const {return &_col_c;}

	/**
	 * @brief Set the frequency value used to generate the Rainbow
	 * palette.
	 */
	static void setRainbowFrequency(float f) {_frequency = f;}

	/**
	 * @brief Get the frequency value used to generate the Rainbow
	 * palette.
	 */
	static float getRainbowFrequency() {return _frequency;}

private:
	ksplot_color _col_c;

	/** The frequency value used to generate the Rainbow palette. */
	static float _frequency;
};

/** Hash table of colors. */
typedef std::unordered_map<int, KsPlot::Color> ColorTable;

ColorTable getTaskColorTable();

ColorTable getCPUColorTable();

Color getColor(ColorTable *colors, int pid);

/** Represents an abstract graphical element. */
class PlotObject {
public:
	/**
	 * @brief Create a default object.
	 */
	PlotObject() : _visible(true), _size(2.) {}

	/**
	 * @brief Destroy the object. Keep this destructor virtual.
	 */
	virtual ~PlotObject() {}

	/** Generic function used to draw different objects. */
	void draw() const {
		if (_visible)
			_draw(_color, _size);
	}

	/** Is this object visible. */
	bool	_visible;

	/** The color of the object. */
	Color	_color;

	/** The size of the object. */
	float	_size;

private:
	virtual void _draw(const Color &col, float s) const = 0;
};

/** List of graphical element. */
typedef std::forward_list<PlotObject*> PlotObjList;

class Point;

/** Represents an abstract shape. */
class Shape : public PlotObject {
public:
	Shape();

	Shape(int n);

	Shape(const Shape &);

	Shape(Shape &&);

	/* Keep this destructor virtual. */
	virtual ~Shape();

	void operator=(const Shape &s);

	void setPoint(size_t i, int x, int y);

	void setPoint(size_t i, const ksplot_point &p);

	void setPoint(size_t i, const Point &p);

	const ksplot_point *getPoint(size_t i) const;

	void setPointX(size_t i, int x);

	void setPointY(size_t i, int y);

	int getPointX(size_t i) const;

	int getPointY(size_t i) const;

	/**
	 * @brief Get the number of point used to define the polygon.
	 */
	size_t pointCount() const {return _nPoints;}

protected:
	/** The number of point used to define the polygon. */
	size_t		_nPoints;

	/** The array of point used to define the polygon. */
	ksplot_point	*_points;
};

/** This class represents a 2D poin. */
class Point : public Shape {
public:
	Point();

	Point(int x, int y);

	/**
	 * @brief Destroy the Point object. Keep this destructor virtual.
	 */
	virtual ~Point() {}

	/** @brief Get the horizontal coordinate of the point. */
	int x() const {return getPointX(0);}

	/** @brief Get the vertical coordinate of the point. */
	int y() const {return getPointY(0);}

	/** @brief Set the horizontal coordinate of the point. */
	void setX(int x) {setPointX(0, x);}

	/** @brief Set the vertical coordinate of the point. */
	void setY(int y) {setPointY(0, y);}

	/**
	 * @brief Set the coordinats of the point.
	 *
	 * @param x: horizontal coordinate of the point in pixels.
	 * @param y: vertical coordinate of the point in pixels.
	 */
	void set(int x, int y) {setPoint(0, x, y);}

	/**
	 * @brief Get the C struct defining the point.
	 */
	const ksplot_point *point_c_ptr() const {return getPoint(0);}

private:
	void _draw(const Color &col, float size = 1.) const override;
};

void drawLine(const Point &a, const Point &b,
	      const Color &col, float size);

void drawDashedLine(const Point &a, const Point &b,
		    const Color &col, float size, float period);

/** This class represents a straight line. */
class Line : public Shape {
public:
	Line();

	Line(const Point &a, const Point &b);

	/**
	 * @brief Destroy the Line object. Keep this destructor virtual.
	 */
	virtual ~Line() {}

	/**
	 * @brief Set the coordinats of the first finishing point of the
	 *	  line.
	 *
	 * @param x: horizontal coordinate of the point in pixels.
	 * @param y: vertical coordinate of the point in pixels.
	 */
	void setA(int x, int y) { setPoint(0, x, y);}

	/** @brief Get the first finishing point of the line. */
	const ksplot_point *getA() const {return getPoint(0);}

	/**
	 * @brief Set the coordinats of the second finishing point of the
	 *	  line.
	 *
	 * @param x: horizontal coordinate of the point in pixels.
	 * @param y: vertical coordinate of the point in pixels.
	 */
	void setB(int x, int y) {setPoint(1, x, y);}

	/** @brief Get the second finishing point of the line. */
	const ksplot_point *getB() const {return getPoint(1);}

private:
	void _draw(const Color &col, float size = 1.) const override;
};

/** This class represents a polygon. */
class Polygon : public Shape {
public:
	Polygon(size_t n);

	/**
	 * @brief Destroy the polygon object. Keep this destructor virtual.
	 */
	virtual ~Polygon() {}

	/**
	 * @brief Specify the way the polygon will be drawn.
	 *
	 * @param f: If True, the area of the polygon will be colored.
	 *	  Otherwise only the contour of the polygon will be plotted.
	 */
	void setFill(bool f) {_fill = f;}

private:
	Polygon() = delete;

	void _draw(const Color &, float size = 1.) const override;

	/**
	 * If True, the area of the polygon will be colored. Otherwise only
	 * the contour of the polygon will be plotted.
	 */
	bool		_fill;
};

/** This class represents a triangle. */
class Triangle : public Polygon {
public:
	/**
	 * Create a default triangle. All points are initialized at (0, 0).
	 */
	Triangle() : Polygon(3) {}

	/** Destroy the Triangle object. Keep this destructor virtual. */
	virtual ~Triangle() {}
};

/** This class represents a rectangle. */
class Rectangle : public Polygon {
public:
	/**
	 * Create a default Rectangle. All points are initialized at (0, 0).
	 */
	Rectangle() : Polygon(4) {}

	/** Destroy the Rectangle object. Keep this destructor virtual. */
	virtual ~Rectangle() {}
};

/**
 * This class represents the graphical element of the KernelShark GUI marker.
 */
class Mark : public PlotObject {
public:
	Mark();

	/**
	 * @brief Destroy the Mark object. Keep this destructor virtual.
	 */
	virtual ~Mark() {}

	void setDPR(int dpr);

	void setX(int x);

	void setY(int yA, int yB);

	void setCPUY(int yCPU);

	void setCPUVisible(bool v);

	void setTaskY(int yTask);

	void setTaskVisible(bool v);

	/** If True, the Mark will be plotted as a dashed line. */
	void setDashed(bool d) {_dashed = d;}

private:
	void _draw(const Color &col, float size = 1.) const override;

	/** First finishing point of the Mark's line. */
	Point _a;

	/** Second finishing point of the Mark's line. */
	Point _b;

	/** A point indicating the position of the Mark in a CPU graph. */
	Point _cpu;

	/** A point indicating the position of the Mark in a Task graph. */
	Point _task;

	/* If True, plot the Mark as a dashed line. */
	bool _dashed;
};

/** This class represents a KernelShark graph's bin. */
class Bin : public PlotObject {
public:
	Bin();

	/**
	 * @brief Destroy the Bin object. Keep this destructor virtual.
	 */
	virtual ~Bin() {}

	void drawVal(float size = 2.);

	/** Get the height (module) of the line, representing the Bin. */
	int mod() {return _val.y() - _base.y();}

	/** @brief Set the vertical coordinate of the "val" Point. */
	void setVal(int v) {_val.setY(_base.y() - v); }

	/**
	 * The Id value (pid or cpu) detected at the front (first in time) edge
	 * of the bin.
	 */
	int	_idFront;

	/**
	 * The Id value (pid or cpu) detected at the back (last in time) edge
	 * of the bin.
	 */
	int	_idBack;

	/** Lower finishing point of the line, representing the Bin. */
	Point	_base;

	/** Upper finishing point of the line, representing the Bin. */
	Point	_val;

	/** A bit mask controlling the visibility of the Bin. */
	uint8_t	_visMask;

private:
	void _draw(const Color &col, float size = 1.) const override;
};

/** This class represents a KernelShark graph. */
class Graph {
public:
	Graph();

	/*
	 * Disable copying. We can enable the Copy Constructor in the future,
	 * but only if we really need it for some reason.
	 */
	Graph(const Graph &) = delete;

	/* Disable moving. Same as copying.*/
	Graph(Graph &&) = delete;

	Graph(kshark_trace_histo *histo, KsPlot::ColorTable *bct,
					 KsPlot::ColorTable *ect);

	/* Keep this destructor virtual. */
	virtual ~Graph();

	int size();

	void setModelPtr(kshark_trace_histo *histo);

	/**
	 * @brief Provide the Graph with a Data Collection. The collection
	 *	  will be used to optimise the processing of the content of
	 *	  the bins.
	 *
	 * @param col: Input location for the data collection descriptor.
	 */
	void setDataCollectionPtr(kshark_entry_collection *col) {
		_collectionPtr = col;
	}

	/** @brief Set the Hash table of Task's colors. */
	void setBinColorTablePtr(KsPlot::ColorTable *ct) {_binColors = ct;}

	void fillCPUGraph(int cpu);

	void fillTaskGraph(int pid);

	void draw(float s = 1);

	void setBase(int b);

	/** @brief Get the vertical coordinate of the Graph's base. */
	int getBase() const {return _bins[0]._base.y();}

	void setHeight(int h);

	/** @brief Get the vertical size (height) of the Graph. */
	int getHeight() const {return _height;}

	void setBinValue(int bin, int val);

	void setBinPid(int bin, int pidF, int pidB);

	void setBinColor(int bin, const Color &col);

	void setBinVisMask(int bin, uint8_t m);

	void setBin(int bin, int pidF, int pidB,
		    const Color &col, uint8_t m);

	/** @brief Get a particular bin. */
	const Bin &getBin(int bin) const {return _bins[bin];}

	void setHMargin(int hMargin);

	/**
	 * Check if this graph is Zero Suppressed. Zero Suppressed means that
	 * bins having Id value = 0 (Idle task records) are not grouped
	 * together.
	 */
	bool zeroSuppressed(bool zs) {return _zeroSuppress;}

	/**
	 * Set Zero Suppression. If True, the bins having Id value = 0 (Idle
	 * task records) are not grouped together.
	 */
	void setZeroSuppressed(bool zs) {_zeroSuppress = zs;}

private:
	/** Pointer to the model descriptor object. */
	kshark_trace_histo	*_histoPtr;

	/** An array of Bins. */
	Bin			*_bins;

	/** The number of Bins. */
	int			_size;

	/**
	 * The size (in pixels) of the white space added on both sides of
	 * the Graph.
	 */
	int			_hMargin;

	/** The vertical size (height) of the Graph. */
	int			_height;

	/** Pointer to the data collection object. */
	kshark_entry_collection	*_collectionPtr;

	/** Hash table of bin's colors. */
	ColorTable		*_binColors;

	/** Hash table of ensemble's colors. */
	ColorTable		*_ensembleColors;

	bool			_zeroSuppress;

	void _initBins();
};

}; // KsPlot

#endif  /* _KS_PLOT_TOOLS_H */
