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

#ifndef QWAYLANDXDGSHELL_H
#define QWAYLANDXDGSHELL_H

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
class QWaylandXdgShellPrivate;
class QWaylandXdgSurface;
class QWaylandXdgSurfacePrivate;
class QWaylandXdgToplevel;
class QWaylandXdgToplevelPrivate;
class QWaylandXdgPopup;
class QWaylandXdgPopupPrivate;
class QWaylandXdgPositioner;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgShell : public QWaylandShellTemplate<QWaylandXdgShell>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgShell)
public:
    explicit QWaylandXdgShell();
    explicit QWaylandXdgShell(QWaylandCompositor *compositor);

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

public Q_SLOTS:
    uint ping(QWaylandClient *client);

Q_SIGNALS:
    void xdgSurfaceCreated(QWaylandXdgSurface *xdgSurface);
    void toplevelCreated(QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface);
    void popupCreated(QWaylandXdgPopup *popup, QWaylandXdgSurface *xdgSurface);
    void pong(uint serial);

private Q_SLOTS:
    void handleSeatChanged(QWaylandSeat *newSeat, QWaylandSeat *oldSeat);
    void handleFocusChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface);
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgSurface : public QWaylandShellSurfaceTemplate<QWaylandXdgSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgSurface)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(QWaylandXdgSurface)
    Q_PROPERTY(QWaylandXdgShell *shell READ shell NOTIFY shellChanged)
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(QWaylandXdgToplevel *toplevel READ toplevel NOTIFY toplevelCreated)
    Q_PROPERTY(QWaylandXdgPopup *popup READ popup NOTIFY popupCreated)
    Q_PROPERTY(QRect windowGeometry READ windowGeometry NOTIFY windowGeometryChanged)

public:
    explicit QWaylandXdgSurface();
    explicit QWaylandXdgSurface(QWaylandXdgShell* xdgShell, QWaylandSurface *surface, const QWaylandResource &resource);

    Q_INVOKABLE void initialize(QWaylandXdgShell* xdgShell, QWaylandSurface *surface, const QWaylandResource &resource);

    Qt::WindowType windowType() const override;

    QWaylandXdgShell *shell() const;
    QWaylandSurface *surface() const;
    QWaylandXdgToplevel *toplevel() const;
    QWaylandXdgPopup *popup() const;
    QRect windowGeometry() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
    static QWaylandXdgSurface *fromResource(::wl_resource *resource);

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

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgToplevel : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgToplevel)
    Q_PROPERTY(QWaylandXdgSurface *xdgSurface READ xdgSurface CONSTANT)
    Q_PROPERTY(QWaylandXdgToplevel *parentToplevel READ parentToplevel NOTIFY parentToplevelChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString appId READ appId NOTIFY appIdChanged)
    Q_PROPERTY(QSize maxSize READ maxSize NOTIFY maxSizeChanged)
    Q_PROPERTY(QSize minSize READ minSize NOTIFY minSizeChanged)
    Q_PROPERTY(QList<int> states READ statesAsInts NOTIFY statesChanged)
    Q_PROPERTY(bool maximized READ maximized NOTIFY maximizedChanged)
    Q_PROPERTY(bool fullscreen READ fullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(bool resizing READ resizing NOTIFY resizingChanged)
    Q_PROPERTY(bool activated READ activated NOTIFY activatedChanged)
    Q_PROPERTY(enum DecorationMode decorationMode READ decorationMode NOTIFY decorationModeChanged)
public:
    enum State : uint {
        MaximizedState  = 1,
        FullscreenState = 2,
        ResizingState   = 3,
        ActivatedState  = 4
    };
    Q_ENUM(State)

    enum DecorationMode {
        ClientSideDecoration = 1,
        ServerSideDecoration = 2,
    };
    Q_ENUM(DecorationMode)

    explicit QWaylandXdgToplevel(QWaylandXdgSurface *xdgSurface, QWaylandResource &resource);
    ~QWaylandXdgToplevel() override;

    QWaylandXdgSurface *xdgSurface() const;
    QWaylandXdgToplevel *parentToplevel() const;

    QString title() const;
    QString appId() const;
    QSize maxSize() const;
    QSize minSize() const;
    QVector<QWaylandXdgToplevel::State> states() const;
    bool maximized() const;
    bool fullscreen() const;
    bool resizing() const;
    bool activated() const;
    DecorationMode decorationMode() const;

    Q_INVOKABLE QSize sizeForResize(const QSizeF &size, const QPointF &delta, Qt::Edges edges) const;
    uint sendConfigure(const QSize &size, const QVector<State> &states);
    Q_INVOKABLE uint sendConfigure(const QSize &size, const QVector<int> &states);
    Q_INVOKABLE void sendClose();
    Q_INVOKABLE uint sendMaximized(const QSize &size);
    Q_INVOKABLE uint sendUnmaximized(const QSize &size = QSize(0, 0));
    Q_INVOKABLE uint sendFullscreen(const QSize &size);
    Q_INVOKABLE uint sendResizing(const QSize &maxSize);

    static QWaylandSurfaceRole *role();
    static QWaylandXdgToplevel *fromResource(::wl_resource *resource);

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

    void decorationModeChanged();

private:
    QList<int> statesAsInts() const;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgPopup : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgPopup)
    Q_PROPERTY(QWaylandXdgSurface *xdgSurface READ xdgSurface)
    Q_PROPERTY(QWaylandXdgSurface *parentXdgSurface READ parentXdgSurface)
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
    QWaylandXdgSurface *xdgSurface() const;
    QWaylandXdgSurface *parentXdgSurface() const;
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
    explicit QWaylandXdgPopup(QWaylandXdgSurface *xdgSurface, QWaylandXdgSurface *parentXdgSurface,
                              QWaylandXdgPositioner *positioner, QWaylandResource &resource);
    friend class QWaylandXdgSurfacePrivate;
};

QT_END_NAMESPACE

#endif // QWAYLANDXDGSHELL_H
