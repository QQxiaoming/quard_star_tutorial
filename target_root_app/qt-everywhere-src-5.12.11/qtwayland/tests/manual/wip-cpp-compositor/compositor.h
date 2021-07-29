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

#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <QtCore/QPointer>

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandView>

QT_BEGIN_NAMESPACE

class Window;
class QOpenGLTexture;
class QWaylandXdgShell;
class QWaylandXdgSurface;
class QWaylandXdgToplevel;

class View : public QWaylandView
{
    Q_OBJECT
public:
    explicit View() = default;
    QOpenGLTexture *getTexture();
    QSize size() const { return surface() ? surface()->size() : QSize(); }
    QRect globalGeometry() const { return {globalPosition(), surface()->size()}; }
    QPoint globalPosition() const { return m_globalPosition; }
    void setGlobalPosition(const QPoint &position);
    QPoint mapToLocal(const QPoint &globalPosition) const;
    void setAnchorEdges(Qt::Edges edges) { m_anchorEdges = edges; }
    void updateAnchoredPosition();

    virtual void handleResizeMove(const QPoint &delta);
    virtual void handleResizeRelease() {}

signals:
    void globalPositionChanged();
    void startResize();
    void startMove();

private:
    QOpenGLTexture *m_texture = nullptr;
    QPoint m_globalPosition;
    Qt::Edges m_anchorEdges;
    QSize m_lastSize;
};

class ToplevelView : public View
{
    Q_OBJECT
public:
    explicit ToplevelView(QWaylandXdgToplevel *toplevel);
    void handleResizeMove(const QPoint &delta) override;
    void handleResizeRelease() override;

private:
    QWaylandXdgToplevel *m_toplevel = nullptr;
    struct Resize  {
        QSize initialSize;
        Qt::Edges edges;
    } m_resize;
};

class Compositor : public QWaylandCompositor
{
    Q_OBJECT
public:
    explicit Compositor() = default;
    ~Compositor() override;
    void create() override;
    void setWindow(Window *window) { m_window = window; }

    QList<View *> views() const { return m_views; }
    View *viewAt(const QPoint &position);

    void raise(View *view);

    void handleGlInitialized() { create(); }
    void handleMousePress(const QPoint &position, Qt::MouseButton button);
    void handleMouseRelease(const QPoint &position, Qt::MouseButton button, Qt::MouseButtons buttons);
    void handleMouseMove(const QPoint &position);
    void handleMouseWheel(Qt::Orientation orientation, int delta);

    void handleKeyPress(quint32 nativeScanCode);
    void handleKeyRelease(quint32 nativeScanCode);

signals:
    void startMove();

private slots:
    void handleXdgToplevelCreated(QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface);
    void addView(View *view);
    void handleViewSurfaceDestroyed();
    void triggerRender();

private:
    Window *m_window = nullptr;
    QWaylandXdgShell *m_xdgShell = nullptr;
    QList<View *> m_views; // Sorted by painters algorithm (back to front)
    struct Grab {
        QPointer<View> view;
        enum State { None, Input, Move, Resize };
        State state = None;
        QPoint startLocalPosition; // in View's coordinate system
        QPoint startGlobalPosition;
    } m_grab;
};

QT_END_NAMESPACE

#endif // COMPOSITOR_H
