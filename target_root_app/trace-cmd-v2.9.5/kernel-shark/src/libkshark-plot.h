/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

 /**
  *  @file    libkshark-plot.h
  *  @brief   Basic tools for OpenGL plotting.
  */

#ifndef _LIB_KSHARK_PLOT_H
#define _LIB_KSHARK_PLOT_H

#ifdef __cplusplus
extern "C" {
#endif

/** Structure defining a RGB color. */
struct ksplot_color {
	/** The Red component of the color. */
	uint8_t red;

	/** The Green component of the color. */
	uint8_t green;

	/** The Blue component of the color. */
	uint8_t blue;
};

/** Structure defining a 2D point. */
struct ksplot_point {
	/** The horizontal coordinate of the point in pixels. */
	int x;

	/** The vertical coordinate of the pointin in pixels. */
	int y;
};

void ksplot_make_scene(int width, int height);

void ksplot_init_opengl(int dpr);

void ksplot_resize_opengl(int width, int height);

void ksplot_draw_point(const struct ksplot_point *p,
		       const struct ksplot_color *col,
		       float size);

void ksplot_draw_line(const struct ksplot_point *a,
		      const struct ksplot_point *b,
		      const struct ksplot_color *col,
		      float size);

void ksplot_draw_polygon(const struct ksplot_point *points,
			 size_t n_points,
			 const struct ksplot_color *col,
			 float size);

void ksplot_draw_polygon_contour(const struct ksplot_point *points,
				 size_t n_points,
				 const struct ksplot_color *col,
				 float size);

#ifdef __cplusplus
}
#endif

#endif
