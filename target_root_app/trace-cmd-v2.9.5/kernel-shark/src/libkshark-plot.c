/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

 /**
  *  @file    libkshark-plot.c
  *  @brief   Basic tools for OpenGL plotting.
  */

// OpenGL
#include <GL/freeglut.h>
#include <GL/gl.h>

// KernelShark
#include "libkshark-plot.h"

/**
 * @brief Create an empty scene for drawing.
 *
 * @param width: Width of the screen window in pixels.
 * @param height: Height of the screen window in pixels.
 */
void ksplot_make_scene(int width, int height)
{
	/* Set Display mode. */
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

	/* Prevent the program from exiting when a window is closed. */
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,
		      GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	/* Set window size. */
	glutInitWindowSize(width, height);

	/* Set window position on screen. */
	glutInitWindowPosition(50, 50);

	/* Open the screen window. */
	glutCreateWindow("KernelShark Plot");

	/*
	 * Set the origin of the coordinate system to be the top left corner.
	 * The "Y" coordinate is inverted.
	 */
	gluOrtho2D(0, width, height, 0);
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

/**
 * @brief Initialize OpenGL.
 *
 * @param dpr: Device Pixel Ratio.
 */
void ksplot_init_opengl(int dpr)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POLYGON_SMOOTH);
	glLineWidth(1.5 * dpr);
	glPointSize(2.5 * dpr);
	glClearColor(1, 1, 1, 1);
}

/**
 * @brief To be called whenever the OpenGL window has been resized.
 *
 * @param width: Width of the screen window in pixels.
 * @param height: Height of the screen window in pixels.
 */
void ksplot_resize_opengl(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	/*
	 * Set the origin of the coordinate system to be the top left corner.
	 * The "Y" coordinate is inverted.
	 */
	gluOrtho2D(0, width, height, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/**
 * @brief Draw a point.
 *
 * @param p: Input location for the point object.
 * @param col: The color of the point.
 * @param size: The size of the point.
 */
void ksplot_draw_point(const struct ksplot_point *p,
		       const struct ksplot_color *col,
		       float size)
{
	if (!p || !col || size < .5f)
		return;

	glPointSize(size);
	glBegin(GL_POINTS);
	glColor3ub(col->red, col->green, col->blue);
	glVertex2i(p->x, p->y);
	glEnd();
}

/**
 * @brief Draw a line.
 *
 * @param a: Input location for the first finishing point of the line.
 * @param b: Input location for the second finishing point of the line.
 * @param col: The color of the line.
 * @param size: The size of the line.
 */
void ksplot_draw_line(const struct ksplot_point *a,
		      const struct ksplot_point *b,
		      const struct ksplot_color *col,
		      float size)
{
	if (!a || !b || !col || size < .5f)
		return;

	glLineWidth(size);
	glBegin(GL_LINES);
	glColor3ub(col->red, col->green, col->blue);
	glVertex2i(a->x, a->y);
	glVertex2i(b->x, b->y);
	glEnd();
}

/**
 * @brief Draw a polygon.
 *
 * @param points: Input location for the array of points defining the polygon.
 * @param n_points: The size of the array of points.
 * @param col: The color of the polygon.
 * @param size: The size of the polygon.
 */
void ksplot_draw_polygon(const struct ksplot_point *points,
			 size_t n_points,
			 const struct ksplot_color *col,
			 float size)
{
	if (!points || !n_points || !col || size < .5f)
		return;

	if (n_points == 1) {
		ksplot_draw_point(points, col, size);
		return;
	}

	if (n_points == 2) {
		ksplot_draw_line(points, points + 1, col, size);
		return;
	}

	/* Obtain a point inside the surface of the polygon. */
	struct ksplot_point in_point;
	in_point.x = (points[0].x + points[2].x) / 2;
	in_point.y = (points[0].y + points[2].y) / 2;

	/*
	 * Draw a Triangle Fan using the internal point as a central
	 * vertex.
	 */
	glBegin(GL_TRIANGLE_FAN);
	glColor3ub(col->red, col->green, col->blue);
	glVertex2i(in_point.x, in_point.y);
	for (size_t i = 0; i < n_points; ++i)
		glVertex2i(points[i].x, points[i].y);

	glVertex2i(points[0].x, points[0].y);
	glEnd();
}

/**
 * @brief Draw the contour of a polygon.
 *
 * @param points: Input location for the array of points defining the polygon.
 * @param n_points: The size of the array of points.
 * @param col: The color of the polygon.
 * @param size: The size of the polygon.
 */
void ksplot_draw_polygon_contour(const struct ksplot_point *points,
				 size_t n_points,
				 const struct ksplot_color *col,
				 float size)
{
	if (!points || !n_points || !col || size < .5f)
		return;

	/* Loop over the points of the polygon and draw connecting lines. */
	for(size_t i = 1; i < n_points; ++i)
		ksplot_draw_line(&points[i - 1],
				 &points[i],
				 col,
				 size);

	/* Close the contour. */
	ksplot_draw_line(&points[0],
			 &points[n_points - 1],
			 col,
			 size);
}
