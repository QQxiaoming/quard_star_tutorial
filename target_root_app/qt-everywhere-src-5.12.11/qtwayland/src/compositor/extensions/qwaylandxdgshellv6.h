/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef QWAYLANDXDGSHELLV6_H
#define QWAYLANDXDGSHELLV6_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandResource>
#include <QtWaylandCompositor/QWaylandShell>
#include <QtWaylandCompositor/QWaylandShellSurface>
#include <QtWaylandCompositor/qwaylandquickchildren.h>

#include <QtCore/QRect>

struct wl_resource;

QT_BEGIN_NAMESPACE

class QWaylandClient;
class QWaylandOutput;
class QWaylandSeat;
class QWaylandSurface;
class QWaylandSurfaceRole;
class QWaylandXdgShellV6Private;
class QWaylandXdgSurfaceV6;
class QWaylandXdgSurfaceV6Private;
class QWaylandXdgToplevelV6;
class QWaylandXdgToplevelV6Private;
class QWaylandXdgPopupV6;
class QWaylandXdgPopupV6Private;
class QWaylandXdgPositionerV6;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgShellV6 : public QWaylandShellTemplate<QWaylandXdgShellV6>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgShellV6)
public:
    QWaylandXdgShellV6();
    QWaylandXdgShellV6(QWaylandCompositor *compositor);

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

public Q_SLOTS:
    uint ping(QWaylandClient *client);

Q_SIGNALS:
    void xdgSurfaceCreated(QWaylandXdgSurfaceV6 *xdgSurface);
    void toplevelCreated(QWaylandXdgToplevelV6 *toplevel, QWaylandXdgSurfaceV6 *xdgSurface);
    void popupCreated(QWaylandXdgPopupV6 *popup, QWaylandXdgSurfaceV6 *xdgSurface);
    void pong(uint serial);

private Q_SLOTS:
    void handleSeatChanged(QWaylandSeat *newSeat, QWaylandSeat *oldSeat);
    void handleFocusChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface);
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgSurfaceV6 : public QWaylandShellSurfaceTemplate<QWaylandXdgSurfaceV6>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgSurfaceV6)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(QWaylandXdgSurfaceV6)
    Q_PROPERTY(QWaylandXdgShellV6 *shell READ shell NOTIFY shellChanged)
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(QWaylandXdgToplevelV6 *toplevel READ toplevel NOTIFY toplevelCreated)
    Q_PROPERTY(QWaylandXdgPopupV6 *popup READ popup NOTIFY popupCreated)
    Q_PROPERTY(QRect windowGeometry READ windowGeometry NOTIFY windowGeometryChanged)

public:
    QWaylandXdgSurfaceV6();
    QWaylandXdgSurfaceV6(QWaylandXdgShellV6* xdgShell, QWaylandSurface *surface, const QWaylandResource &resource);

    Q_INVOKABLE void initialize(QWaylandXdgShellV6* xdgShell, QWaylandSurface *surface, const QWaylandResource &resource);

    Qt::WindowType windowType() const override;

    QWaylandXdgShellV6 *shell() const;
    QWaylandSurface *surface() const;
    QWaylandXdgToplevelV6 *toplevel() const;
    QWaylandXdgPopupV6 *popup() const;
    QRect windowGeometry() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
    static QWaylandXdgSurfaceV6 *fromResource(::wl_resource *resource);

#if QT_CONFIG(wayland_compositor_quick)
    QWaylandQuickShellIntegration *createIntegration(QWaylandQuickShellSurfaceItem *item) override;
#endif

Q_SIGNALS:
    void shellChanged();
    void surfaceChanged();
    void toplevelCreated();
    void popupCreated();
    void windowGeometryChanged();

private:
    void initialize() override;

private Q_SLOTS:
    void handleSurfaceSizeChanged();
    void handleBufferScaleChanged();
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgToplevelV6 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgToplevelV6)
    Q_PROPERTY(QWaylandXdgToplevelV6 *parentToplevel READ parentToplevel NOTIFY parentToplevelChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString appId READ appId NOTIFY appIdChanged)
    Q_PROPERTY(QSize maxSize READ maxSize NOTIFY maxSizeChanged)
    Q_PROPERTY(QSize minSize READ minSize NOTIFY minSizeChanged)
    Q_PROPERTY(QList<int> states READ statesAsInts NOTIFY statesChanged)
    Q_PROPERTY(bool maximized READ maximized NOTIFY maximizedChanged)
    Q_PROPERTY(bool fullscreen READ fullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(bool resizing READ resizing NOTIFY resizingChanged)
    Q_PROPERTY(bool activated READ activated NOTIFY activatedChanged)
public:
    enum State : uint {
        MaximizedState  = 1,
        FullscreenState = 2,
        ResizingState   = 3,
        ActivatedState  = 4
    };
    Q_ENUM(State)

    QWaylandXdgToplevelV6(QWaylandXdgSurfaceV6 *xdgSurface, QWaylandResource &resource);

    QWaylandXdgToplevelV6 *parentToplevel() const;

    QString title() const;
    QString appId() const;
    QSize maxSize() const;
    QSize minSize() const;
    QVector<QWaylandXdgToplevelV6::State> states() const;
    bool maximized() const;
    bool fullscreen() const;
    bool resizing() const;
    bool activated() const;

    Q_INVOKABLE QSize sizeForResize(const QSizeF &size, const QPointF &delta, Qt::Edges edges) const;
    uint sendConfigure(const QSize &size, const QVector<State> &states);
    Q_INVOKABLE uint sendConfigure(const QSize &size, const QVector<int> &states);
    Q_INVOKABLE void sendClose();
    Q_INVOKABLE uint sendMaximized(const QSize &size);
    Q_INVOKABLE uint sendUnmaximized(const QSize &size = QSize(0, 0));
    Q_INVOKABLE uint sendFullscreen(const QSize &size);
    Q_INVOKABLE uint sendResizing(const QSize &maxSize);

    static QWaylandSurfaceRole *role();

Q_SIGNALS:
    void parentToplevelChanged();
    void titleChanged();
    void appIdChanged();
    void maxSizeChanged();
    void minSizeChanged();
    void startMove(QWaylandSeat *seat);
    void startResize(QWaylandSeat *seat, Qt::Edges edges);
    void statesChanged();
    void maximizedChanged();
    void fullscreenChanged();
    void resizingChanged();
    void activatedChanged();

    void showWindowMenu(QWaylandSeat *seat, const QPoint &localSurfacePosition);
    void setMaximized();
    void unsetMaximized();
    void setFullscreen(QWaylandOutput *output);
    void unsetFullscreen();
    void setMinimized();

private:
    QList<int> statesAsInts() const;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgPopupV6 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgPopupV6)
    Q_PROPERTY(QWaylandXdgSurfaceV6 *xdgSurface READ xdgSurface)
    Q_PROPERTY(QWaylandXdgSurfaceV6 *parentXdgSurface READ parentXdgSurface)
    Q_PROPERTY(QRect configuredGeometry READ configuredGeometry NOTIFY configuredGeometryChanged)

    // Positioner properties
    Q_PROPERTY(QRect anchorRect READ anchorRect CONSTANT)
    Q_PROPERTY(Qt::Edges anchorEdges READ anchorEdges CONSTANT)
    Q_PROPERTY(Qt::Edges gravityEdges READ gravityEdges CONSTANT)
    Q_PROPERTY(Qt::Orientations slideConstraints READ slideConstraints CONSTANT)
    Q_PROPERTY(Qt::Orientations flipConstraints READ flipConstraints CONSTANT)
    Q_PROPERTY(Qt::Orientations resizeConstraints READ resizeConstraints CONSTANT)
    Q_PROPERTY(QPoint offset READ offset CONSTANT)
    Q_PROPERTY(QSize positionerSize READ positionerSize CONSTANT)
    Q_PROPERTY(QPoint unconstrainedPosition READ unconstrainedPosition CONSTANT)
public:
    QWaylandXdgSurfaceV6 *xdgSurface() const;
    QWaylandXdgSurfaceV6 *parentXdgSurface() const;
    QRect configuredGeometry() const;

    // Positioner properties
    QRect anchorRect() const;
    Qt::Edges anchorEdges() const ;
    Qt::Edges gravityEdges() const ;
    Qt::Orientations slideConstraints() const;
    Qt::Orientations flipConstraints() const;
    Qt::Orientations resizeConstraints() const;
    QPoint offset() const;
    QSize positionerSize() const;
    QPoint unconstrainedPosition() const;

    Q_INVOKABLE uint sendConfigure(const QRect &geometry);

    static QWaylandSurfaceRole *role();

Q_SIGNALS:
    void configuredGeometryChanged();

private:
    QWaylandXdgPopupV6(QWaylandXdgSurfaceV6 *xdgSurface, QWaylandXdgSurfaceV6 *parentXdgSurface,
                       QWaylandXdgPositionerV6 *positioner, QWaylandResource &resource);
    friend class QWaylandXdgSurfaceV6Private;
};

QT_END_NAMESPACE

#endif // QWAYLANDXDGSHELLV6_H
