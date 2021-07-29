/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDXDGSHELLINTEGRATION_H
#define QWAYLANDXDGSHELLINTEGRATION_H

#include <QtWaylandCompositor/private/qwaylandquickshellsurfaceitem_p.h>
#include <QtWaylandCompositor/QWaylandQuickShellSurfaceItem>
#include <QtWaylandCompositor/QWaylandXdgToplevel>

QT_BEGIN_NAMESPACE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class QWaylandXdgSurface;

namespace QtWayland {

class XdgToplevelIntegration : public QWaylandQuickShellIntegration
{
    Q_OBJECT
public:
    XdgToplevelIntegration(QWaylandQuickShellSurfaceItem *item);
    bool mouseMoveEvent(QMouseEvent *event) override;
    bool mouseReleaseEvent(QMouseEvent *event) override;

private Q_SLOTS:
    void handleStartMove(QWaylandSeat *seat);
    void handleStartResize(QWaylandSeat *seat, Qt::Edges edges);
    void handleSetMaximized();
    void handleUnsetMaximized();
    void handleMaximizedChanged();
    void handleSetFullscreen();
    void handleUnsetFullscreen();
    void handleFullscreenChanged();
    void handleActivatedChanged();
    void handleSurfaceSizeChanged();
    void handleToplevelDestroyed();
    void handleMaximizedSizeChanged();
    void handleFullscreenSizeChanged();

private:
    QWaylandQuickShellSurfaceItem *m_item = nullptr;
    QWaylandXdgSurface *m_xdgSurface = nullptr;
    QWaylandXdgToplevel *m_toplevel = nullptr;

    enum class GrabberState {
        Default,
        Resize,
        Move
    };
    GrabberState grabberState;

    struct {
        QWaylandSeat *seat = nullptr;
        QPointF initialOffset;
        bool initialized;
    } moveState;

    struct {
        QWaylandSeat *seat = nullptr;
        Qt::Edges resizeEdges;
        QSizeF initialWindowSize;
        QPointF initialMousePos;
        QPointF initialPosition;
        QSize initialSurfaceSize;
        bool initialized;
    } resizeState;

    struct {
        QSize initialWindowSize;
        QPointF initialPosition;
    } windowedGeometry;

    struct {
        QWaylandOutput *output = nullptr;
        QMetaObject::Connection sizeChangedConnection; // Depending on whether maximized or fullscreen,
                                                       // will be hooked to geometry-changed or available-
                                                       // geometry-changed.
    } nonwindowedState;
};

class XdgPopupIntegration : public QWaylandQuickShellIntegration
{
    Q_OBJECT
public:
    XdgPopupIntegration(QWaylandQuickShellSurfaceItem *item);

private Q_SLOTS:
    void handleGeometryChanged();

private:
    QWaylandQuickShellSurfaceItem *m_item = nullptr;
    QWaylandXdgSurface *m_xdgSurface = nullptr;
    QWaylandXdgPopup *m_popup = nullptr;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDXDGSHELLINTEGRATION_H
