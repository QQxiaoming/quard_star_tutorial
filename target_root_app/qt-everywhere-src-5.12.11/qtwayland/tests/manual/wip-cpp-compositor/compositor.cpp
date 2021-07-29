/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Wayland module
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "compositor.h"
#include "window.h"

#include <QtWaylandCompositor/QWaylandOutput>
#include <QtWaylandCompositor/QWaylandXdgShell>
#include <QtWaylandCompositor/QWaylandSeat>

#include <QOpenGLFunctions>

QOpenGLTexture *View::getTexture() {
    if (advance())
        m_texture = currentBuffer().toOpenGLTexture();
    return m_texture;
}

void View::setGlobalPosition(const QPoint &position)
{
    if (m_globalPosition == position)
        return;

    m_globalPosition = position;
    emit globalPositionChanged();
}

QPoint View::mapToLocal(const QPoint &globalPosition) const
{
    return globalPosition - this->globalPosition();
}

void View::updateAnchoredPosition()
{
    QPoint offset;
    QSize size = surface()->size();
    QSize delta = size - m_lastSize;
    if (m_anchorEdges & Qt::RightEdge)
        offset.setX(-delta.width());
    if (m_anchorEdges & Qt::BottomEdge)
        offset.setY(-delta.height());
    setGlobalPosition(globalPosition() + offset);
    m_lastSize = size;
}

void View::handleResizeMove(const QPoint &delta)
{
    Q_UNUSED(delta);
    qWarning() << "Resize not implemented for this view";
}

ToplevelView::ToplevelView(QWaylandXdgToplevel *toplevel)
    : m_toplevel(toplevel)
{
    QWaylandXdgSurface *xdgSurface = toplevel->xdgSurface();
    setSurface(xdgSurface->surface());
    connect(toplevel, &QWaylandXdgToplevel::startMove, this, &View::startMove);
    connect(toplevel, &QWaylandXdgToplevel::startResize, this, [this](QWaylandSeat *, Qt::Edges edges) {
        m_resize.edges = edges;
        m_resize.initialSize = m_toplevel->xdgSurface()->windowGeometry().size();
        Qt::Edges opposite = edges ^ Qt::Edges(0b1111);
        setAnchorEdges(opposite);
        emit startResize();
    });
    QVector<QWaylandXdgToplevel::State> states{QWaylandXdgToplevel::ActivatedState};
    toplevel->sendConfigure(QSize(0, 0), states);
}

void ToplevelView::handleResizeMove(const QPoint &delta)
{
    QSize newSize = m_toplevel->sizeForResize(m_resize.initialSize, delta, m_resize.edges);
    m_toplevel->sendResizing(newSize);
}

void ToplevelView::handleResizeRelease()
{
    setAnchorEdges({});
    m_resize.edges = {};
    m_resize.initialSize = {};
}

Compositor::~Compositor()
{
    delete m_xdgShell;
}

void Compositor::create()
{
    QWaylandCompositor::create();

    m_xdgShell = new QWaylandXdgShell(this);
    connect(m_xdgShell, &QWaylandXdgShell::toplevelCreated, this, &Compositor::handleXdgToplevelCreated);
}

View *Compositor::viewAt(const QPoint &position)
{
    // Since views are stored in painting order (back to front), we have to iterate backwards
    // to find the topmost view at the given point
    for (auto it = m_views.crbegin(); it != m_views.crend(); ++it) {
        View *view = *it;
        if (view->globalGeometry().contains(position))
            return view;
    }
    return nullptr;
}

void Compositor::raise(View *view)
{
    m_views.removeAll(view);
    m_views << view;
    defaultSeat()->setKeyboardFocus(view->surface());
    triggerRender();
}

void Compositor::handleMousePress(const QPoint &position, Qt::MouseButton button)
{
    if (m_grab.state == Grab::None) {
        m_grab.view = viewAt(position);
        if (m_grab.view) {
            m_grab.state = Grab::Input;
            m_grab.startGlobalPosition = position;
            m_grab.startLocalPosition = m_grab.view->mapToLocal(position);
            raise(m_grab.view);
        }
    }

    switch (m_grab.state) {
    case Grab::Input: {
        auto *seat = defaultSeat();
        seat->sendMouseMoveEvent(m_grab.view, m_grab.view->mapToLocal(position));
        seat->sendMousePressEvent(button);
        break;
    }
    case Grab::Move:
    case Grab::Resize:
    case Grab::None:
        break;
    }
}

void Compositor::handleMouseRelease(const QPoint &position, Qt::MouseButton button, Qt::MouseButtons buttons)
{
    auto *seat = defaultSeat();

    switch (m_grab.state) {
    case Grab::Input:
        seat->sendMouseMoveEvent(m_grab.view, m_grab.view->mapToLocal(position));
        seat->sendMouseReleaseEvent(button);
        if (buttons == Qt::NoButton) {
            View *newView = viewAt(position);
            if (newView != m_grab.view) {
                seat->setMouseFocus(newView);
                if (newView)
                    seat->sendMouseMoveEvent(newView, newView->mapToLocal(position));
            }
            m_grab.view = nullptr;
            m_grab.state = Grab::None;
        }
        break;
    case Grab::Move:
    case Grab::Resize:
        m_grab.state = Grab::None;
        m_grab.view = nullptr;
        if (View *view = viewAt(position))
            seat->sendMouseMoveEvent(view, view->mapToLocal(position));
        break;
    case Grab::None:
        if (View *view = viewAt(position))
            seat->sendMouseMoveEvent(view, view->mapToLocal(position));
        break;
    }
}

void Compositor::handleMouseMove(const QPoint &position)
{
    switch (m_grab.state) {
    case Grab::Input:
        defaultSeat()->sendMouseMoveEvent(m_grab.view, m_grab.view->mapToLocal(position));
        break;
    case Grab::None:
        if (View *view = viewAt(position))
            defaultSeat()->sendMouseMoveEvent(view, view->mapToLocal(position));
        break;
    case Grab::Resize:
        m_grab.view->handleResizeMove(position - m_grab.startGlobalPosition);
        break;
    case Grab::Move:
        m_grab.view->setGlobalPosition(position - m_grab.startLocalPosition);
        break;
    }
}

void Compositor::handleMouseWheel(Qt::Orientation orientation, int delta)
{
    defaultSeat()->sendMouseWheelEvent(orientation, delta);
}

void Compositor::handleKeyPress(quint32 nativeScanCode)
{
    defaultSeat()->sendKeyPressEvent(nativeScanCode);
}

void Compositor::handleKeyRelease(quint32 nativeScanCode)
{
    defaultSeat()->sendKeyReleaseEvent(nativeScanCode);
}

void Compositor::handleXdgToplevelCreated(QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface)
{
    Q_UNUSED(xdgSurface);
    auto *view = new ToplevelView(toplevel);
    addView(view);
}

void Compositor::addView(View *view)
{
    view->setOutput(outputFor(m_window));
    m_views << view;
    connect(view, &QWaylandView::surfaceDestroyed, this, &Compositor::handleViewSurfaceDestroyed);
    connect(view, &View::globalPositionChanged, this, &Compositor::triggerRender);
    connect(view->surface(), &QWaylandSurface::redraw, this, &Compositor::triggerRender);
    connect(view, &View::startMove, this, [this, view](){
        m_grab.view = view;
        m_grab.state = Grab::Move;
    });
    connect(view, &View::startResize, this, [this, view]() {
        m_grab.view = view;
        m_grab.state = Grab::Resize;
    });
}

void Compositor::handleViewSurfaceDestroyed()
{
    auto *view = qobject_cast<ToplevelView*>(sender());
    m_views.removeAll(view);
    delete view;
    triggerRender();
}

void Compositor::triggerRender()
{
    m_window->requestUpdate();
}
