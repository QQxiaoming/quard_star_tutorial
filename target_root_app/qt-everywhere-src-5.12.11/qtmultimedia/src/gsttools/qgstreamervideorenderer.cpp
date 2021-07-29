/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qgstreamervideorenderer_p.h"
#include <private/qvideosurfacegstsink_p.h>
#include <private/qgstutils_p.h>
#include <qabstractvideosurface.h>
#include <QtCore/qdebug.h>

#include <gst/gst.h>

static inline void resetSink(GstElement *&element, GstElement *v = nullptr)
{
    if (element)
        gst_object_unref(GST_OBJECT(element));

    if (v)
        qt_gst_object_ref_sink(GST_OBJECT(v));

    element = v;
}

QGstreamerVideoRenderer::QGstreamerVideoRenderer(QObject *parent)
    : QVideoRendererControl(parent)
{
}

QGstreamerVideoRenderer::~QGstreamerVideoRenderer()
{
    resetSink(m_videoSink);
}

void QGstreamerVideoRenderer::setVideoSink(GstElement *sink)
{
    if (!sink)
        return;

    resetSink(m_videoSink, sink);
    emit sinkChanged();
}

GstElement *QGstreamerVideoRenderer::videoSink()
{
    if (!m_videoSink && m_surface) {
        auto sink = reinterpret_cast<GstElement *>(QVideoSurfaceGstSink::createSink(m_surface));
        resetSink(m_videoSink, sink);
    }

    return m_videoSink;
}

void QGstreamerVideoRenderer::stopRenderer()
{
    if (m_surface)
        m_surface->stop();
}

QAbstractVideoSurface *QGstreamerVideoRenderer::surface() const
{
    return m_surface;
}

void QGstreamerVideoRenderer::setSurface(QAbstractVideoSurface *surface)
{
    if (m_surface != surface) {
        resetSink(m_videoSink);

        if (m_surface) {
            disconnect(m_surface.data(), SIGNAL(supportedFormatsChanged()),
                       this, SLOT(handleFormatChange()));
        }

        bool wasReady = isReady();

        m_surface = surface;

        if (m_surface) {
            connect(m_surface.data(), SIGNAL(supportedFormatsChanged()),
                    this, SLOT(handleFormatChange()));
        }

        if (wasReady != isReady())
            emit readyChanged(isReady());

        emit sinkChanged();
    }
}

void QGstreamerVideoRenderer::handleFormatChange()
{
    setVideoSink(nullptr);
}
