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

#include "qwaylandxdgshellintegration_p.h"

#include <QtWaylandCompositor/QWaylandXdgSurface>
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>

QT_BEGIN_NAMESPACE

namespace QtWayland {

static void handlePopupCreated(QWaylandQuickShellSurfaceItem *parentItem, QWaylandXdgPopup *popup)
{
    if (parentItem->shellSurface() == popup->parentXdgSurface())
        QWaylandQuickShellSurfaceItemPrivate::get(parentItem)->maybeCreateAutoPopup(popup->xdgSurface());
}

XdgToplevelIntegration::XdgToplevelIntegration(QWaylandQuickShellSurfaceItem *item)
    : QWaylandQuickShellIntegration(item)
    , m_item(item)
    , m_xdgSurface(qobject_cast<QWaylandXdgSurface *>(item->shellSurface()))
    , m_toplevel(m_xdgSurface->toplevel())
    , grabberState(GrabberState::Default)
{
    Q_ASSERT(m_toplevel);

    m_item->setSurface(m_xdgSurface->surface());

    connect(m_toplevel, &QWaylandXdgToplevel::startMove, this, &XdgToplevelIntegration::handleStartMove);
    connect(m_toplevel, &QWaylandXdgToplevel::startResize, this, &XdgToplevelIntegration::handleStartResize);
    connect(m_toplevel, &QWaylandXdgToplevel::setMaximized, this, &XdgToplevelIntegration::handleSetMaximized);
    connect(m_toplevel, &QWaylandXdgToplevel::unsetMaximized, this, &XdgToplevelIntegration::handleUnsetMaximized);
    connect(m_toplevel, &QWaylandXdgToplevel::maximizedChanged, this, &XdgToplevelIntegration::handleMaximizedChanged);
    connect(m_toplevel, &QWaylandXdgToplevel::setFullscreen, this, &XdgToplevelIntegration::handleSetFullscreen);
    connect(m_toplevel, &QWaylandXdgToplevel::unsetFullscreen, this, &XdgToplevelIntegration::handleUnsetFullscreen);
    connect(m_toplevel, &QWaylandXdgToplevel::fullscreenChanged, this, &XdgToplevelIntegration::handleFullscreenChanged);
    connect(m_toplevel, &QWaylandXdgToplevel::activatedChanged, this, &XdgToplevelIntegration::handleActivatedChanged);
    connect(m_xdgSurface->shell(), &QWaylandXdgShell::popupCreated, this, [item](QWaylandXdgPopup *popup, QWaylandXdgSurface *){
        handlePopupCreated(item, popup);
    });
    connect(m_xdgSurface->surface(), &QWaylandSurface::sizeChanged, this, &XdgToplevelIntegration::handleSurfaceSizeChanged);
    connect(m_toplevel, &QObject::destroyed, this, &XdgToplevelIntegration::handleToplevelDestroyed);
}

bool XdgToplevelIntegration::mouseMoveEvent(QMouseEvent *event)
{
    if (grabberState == GrabberState::Resize) {
        Q_ASSERT(resizeState.seat == m_item->compositor()->seatFor(event));
        if (!resizeState.initialized) {
            resizeState.initialMousePos = event->windowPos();
            resizeState.initialized = true;
            return true;
        }
        QPointF delta = m_item->mapToSurface(event->windowPos() - resizeState.initialMousePos);
        QSize newSize = m_toplevel->sizeForResize(resizeState.initialWindowSize, delta, resizeState.resizeEdges);
        m_toplevel->sendResizing(newSize);
    } else if (grabberState == GrabberState::Move) {
        Q_ASSERT(moveState.seat == m_item->compositor()->seatFor(event));
        QQuickItem *moveItem = m_item->moveItem();
        if (!moveState.initialized) {
            moveState.initialOffset = moveItem->mapFromItem(nullptr, event->windowPos());
            moveState.initialized = true;
            return true;
        }
        if (!moveItem->parentItem())
            return true;
        QPointF parentPos = moveItem->parentItem()->mapFromItem(nullptr, event->windowPos());
        moveItem->setPosition(parentPos - moveState.initialOffset);
    }
    return false;
}

bool XdgToplevelIntegration::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    if (grabberState != GrabberState::Default) {
        grabberState = GrabberState::Default;
        return true;
    }
    return false;
}

void XdgToplevelIntegration::handleStartMove(QWaylandSeat *seat)
{
    grabberState = GrabberState::Move;
    moveState.seat = seat;
    moveState.initialized = false;
}

void XdgToplevelIntegration::handleStartResize(QWaylandSeat *seat, Qt::Edges edges)
{
    grabberState = GrabberState::Resize;
    resizeState.seat = seat;
    resizeState.resizeEdges = edges;
    resizeState.initialWindowSize = m_xdgSurface->windowGeometry().size();
    resizeState.initialPosition = m_item->moveItem()->position();
    resizeState.initialSurfaceSize = m_item->surface()->size();
    resizeState.initialized = false;
}

void XdgToplevelIntegration::handleSetMaximized()
{
    if (!m_item->view()->isPrimary())
        return;

    QVector<QWaylandXdgToplevel::State> states = m_toplevel->states();

    if (!states.contains(QWaylandXdgToplevel::State::FullscreenState) && !states.contains(QWaylandXdgToplevel::State::MaximizedState)) {
        windowedGeometry.initialWindowSize = m_xdgSurface->windowGeometry().size();
        windowedGeometry.initialPosition = m_item->moveItem()->position();
    }

    // Any prior output-resize handlers are irrelevant at this point.
    disconnect(nonwindowedState.sizeChangedConnection);
    nonwindowedState.output = m_item->view()->output();
    nonwindowedState.sizeChangedConnection = connect(nonwindowedState.output, &QWaylandOutput::availableGeometryChanged, this, &XdgToplevelIntegration::handleMaximizedSizeChanged);
    handleMaximizedSizeChanged();
}

void XdgToplevelIntegration::handleMaximizedSizeChanged()
{
    // Insurance against handleToplevelDestroyed() not managing to disconnect this
    // handler in time.
    if (m_toplevel == nullptr)
        return;

    m_toplevel->sendMaximized(nonwindowedState.output->availableGeometry().size() / nonwindowedState.output->scaleFactor());
}

void XdgToplevelIntegration::handleUnsetMaximized()
{
    if (!m_item->view()->isPrimary())
        return;

    // If no prior windowed size was recorded, send a 0x0 configure event
    // to allow the client to choose its preferred size.
    if (windowedGeometry.initialWindowSize.isValid())
        m_toplevel->sendUnmaximized(windowedGeometry.initialWindowSize);
    else
        m_toplevel->sendUnmaximized();
}

void XdgToplevelIntegration::handleMaximizedChanged()
{
    if (m_toplevel->maximized()) {
        if (auto *output = m_item->view()->output()) {
            m_item->moveItem()->setPosition(output->position() + output->availableGeometry().topLeft());
        } else {
            qCWarning(qLcWaylandCompositor) << "The view does not have a corresponding output,"
                                            << "ignoring maximized state";
        }
    } else {
        m_item->moveItem()->setPosition(windowedGeometry.initialPosition);
    }
}

void XdgToplevelIntegration::handleSetFullscreen()
{
    if (!m_item->view()->isPrimary())
        return;

    QVector<QWaylandXdgToplevel::State> states = m_toplevel->states();

    if (!states.contains(QWaylandXdgToplevel::State::FullscreenState) && !states.contains(QWaylandXdgToplevel::State::MaximizedState)) {
        windowedGeometry.initialWindowSize = m_xdgSurface->windowGeometry().size();
        windowedGeometry.initialPosition = m_item->moveItem()->position();
    }

    // Any prior output-resize handlers are irrelevant at this point.
    disconnect(nonwindowedState.sizeChangedConnection);
    nonwindowedState.output = m_item->view()->output();
    nonwindowedState.sizeChangedConnection = connect(nonwindowedState.output, &QWaylandOutput::geometryChanged, this, &XdgToplevelIntegration::handleFullscreenSizeChanged);
    handleFullscreenSizeChanged();
}

void XdgToplevelIntegration::handleFullscreenSizeChanged()
{
    // Insurance against handleToplevelDestroyed() not managing to disconnect this
    // handler in time.
    if (m_toplevel == nullptr)
        return;

    m_toplevel->sendFullscreen(nonwindowedState.output->geometry().size() / nonwindowedState.output->scaleFactor());
}

void XdgToplevelIntegration::handleUnsetFullscreen()
{
    if (!m_item->view()->isPrimary())
        return;

    // If no prior windowed size was recorded, send a 0x0 configure event
    // to allow the client to choose its preferred size.
    if (windowedGeometry.initialWindowSize.isValid())
        m_toplevel->sendUnmaximized(windowedGeometry.initialWindowSize);
    else
        m_toplevel->sendUnmaximized();
}

void XdgToplevelIntegration::handleFullscreenChanged()
{
    if (m_toplevel->fullscreen()) {
        if (auto *output = m_item->view()->output()) {
            m_item->moveItem()->setPosition(output->position() + output->geometry().topLeft());
        } else {
            qCWarning(qLcWaylandCompositor) << "The view does not have a corresponding output,"
                                            << "ignoring fullscreen state";
        }
    } else {
        m_item->moveItem()->setPosition(windowedGeometry.initialPosition);
    }
}

void XdgToplevelIntegration::handleActivatedChanged()
{
    if (m_toplevel->activated())
        m_item->raise();
}

void XdgToplevelIntegration::handleSurfaceSizeChanged()
{
    if (grabberState == GrabberState::Resize) {
        qreal x = resizeState.initialPosition.x();
        qreal y = resizeState.initialPosition.y();
        if (resizeState.resizeEdges & Qt::TopEdge)
            y += resizeState.initialSurfaceSize.height() - m_item->surface()->size().height();

        if (resizeState.resizeEdges & Qt::LeftEdge)
            x += resizeState.initialSurfaceSize.width() - m_item->surface()->size().width();
        m_item->moveItem()->setPosition(QPointF(x, y));
    }
}

void XdgToplevelIntegration::handleToplevelDestroyed()
{
    // Disarm any handlers that might fire on the now-stale toplevel pointer
    nonwindowedState.output = nullptr;
    disconnect(nonwindowedState.sizeChangedConnection);
}

XdgPopupIntegration::XdgPopupIntegration(QWaylandQuickShellSurfaceItem *item)
    : m_item(item)
    , m_xdgSurface(qobject_cast<QWaylandXdgSurface *>(item->shellSurface()))
    , m_popup(m_xdgSurface->popup())
{
    Q_ASSERT(m_popup);

    m_item->setSurface(m_xdgSurface->surface());
    handleGeometryChanged();

    connect(m_popup, &QWaylandXdgPopup::configuredGeometryChanged, this, &XdgPopupIntegration::handleGeometryChanged);
    connect(m_xdgSurface->shell(), &QWaylandXdgShell::popupCreated, this, [item](QWaylandXdgPopup *popup, QWaylandXdgSurface *){
        handlePopupCreated(item, popup);
    });
}

void XdgPopupIntegration::handleGeometryChanged()
{
    if (m_item->view()->output()) {
        const QPoint windowOffset = m_popup->parentXdgSurface()->windowGeometry().topLeft();
        const QPoint position = m_popup->unconstrainedPosition() + windowOffset;
        //TODO: positioner size or other size...?
        const float scaleFactor = m_item->view()->output()->scaleFactor();
        //TODO check positioner constraints etc... sliding, flipping
        m_item->moveItem()->setPosition(position * scaleFactor);
    } else {
        qWarning() << "XdgPopupIntegration popup item without output" << m_item;
    }
}

}

QT_END_NAMESPACE
