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

#ifndef QWAYLANDXDGSHELL_P_H
#define QWAYLANDXDGSHELL_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwaylandshell_p.h>
#include <QtWaylandCompositor/private/qwayland-server-xdg-shell.h>

#include <QtWaylandCompositor/QWaylandXdgShell>

#include <QtWaylandCompositor/private/qwaylandxdgdecorationv1_p.h>

#include <QtCore/QSet>

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

QT_BEGIN_NAMESPACE

struct Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgPositionerData {
    QSize size;
    QRect anchorRect;
    Qt::Edges anchorEdges = 0;
    Qt::Edges gravityEdges = 0;
    uint constraintAdjustments = XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_NONE;
    QPoint offset;
    QWaylandXdgPositionerData();
    bool isComplete() const;
    QPoint anchorPoint() const;
    QPoint unconstrainedPosition() const;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgShellPrivate
        : public QWaylandShellPrivate
        , public QtWaylandServer::xdg_wm_base
{
    Q_DECLARE_PUBLIC(QWaylandXdgShell)
public:
    QWaylandXdgShellPrivate();
    void ping(Resource *resource, uint32_t serial);
    void registerXdgSurface(QWaylandXdgSurface *xdgSurface);
    void unregisterXdgSurface(QWaylandXdgSurface *xdgSurface);
    static QWaylandXdgShellPrivate *get(QWaylandXdgShell *xdgShell) { return xdgShell->d_func(); }

    QSet<uint32_t> m_pings;
    QMultiMap<struct wl_client *, QWaylandXdgSurface *> m_xdgSurfaces;

    QWaylandXdgSurface *xdgSurfaceFromSurface(QWaylandSurface *surface);

protected:
    void xdg_wm_base_destroy(Resource *resource) override;
    void xdg_wm_base_create_positioner(Resource *resource, uint32_t id) override;
    void xdg_wm_base_get_xdg_surface(Resource *resource, uint32_t id,
                                     struct ::wl_resource *surface) override;
    void xdg_wm_base_pong(Resource *resource, uint32_t serial) override;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgSurfacePrivate
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::xdg_surface
{
    Q_DECLARE_PUBLIC(QWaylandXdgSurface)
public:
    QWaylandXdgSurfacePrivate();
    void setWindowType(Qt::WindowType windowType);
    void handleFocusLost();
    void handleFocusReceived();
    static QWaylandXdgSurfacePrivate *get(QWaylandXdgSurface *xdgSurface) { return xdgSurface->d_func(); }

    QRect calculateFallbackWindowGeometry() const;
    void updateFallbackWindowGeometry();

private:
    QWaylandXdgShell *m_xdgShell = nullptr;
    QWaylandSurface *m_surface = nullptr;

    QWaylandXdgToplevel *m_toplevel = nullptr;
    QWaylandXdgPopup *m_popup = nullptr;
    QRect m_windowGeometry;
    bool m_unsetWindowGeometry = true;
    QMargins m_windowMargins;
    Qt::WindowType m_windowType = Qt::WindowType::Window;

    void xdg_surface_destroy_resource(Resource *resource) override;
    void xdg_surface_destroy(Resource *resource) override;
    void xdg_surface_get_toplevel(Resource *resource, uint32_t id) override;
    void xdg_surface_get_popup(Resource *resource, uint32_t id, struct ::wl_resource *parent, struct ::wl_resource *positioner) override;
    void xdg_surface_ack_configure(Resource *resource, uint32_t serial) override;
    void xdg_surface_set_window_geometry(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgToplevelPrivate : public QObjectPrivate, public QtWaylandServer::xdg_toplevel
{
    Q_DECLARE_PUBLIC(QWaylandXdgToplevel)
public:
    struct ConfigureEvent {
        ConfigureEvent() = default;
        ConfigureEvent(const QVector<QWaylandXdgToplevel::State>
                       &incomingStates,
                       const QSize &incomingSize, uint incomingSerial)
        : states(incomingStates), size(incomingSize), serial(incomingSerial)
        { }
        QVector<QWaylandXdgToplevel::State> states;
        QSize size = {0, 0};
        uint serial = 0;
    };

    QWaylandXdgToplevelPrivate(QWaylandXdgSurface *xdgSurface, const QWaylandResource& resource);
    ConfigureEvent lastSentConfigure() const { return m_pendingConfigures.empty() ? m_lastAckedConfigure : m_pendingConfigures.last(); }
    void handleAckConfigure(uint serial); //TODO: move?
    void handleFocusLost();
    void handleFocusReceived();

    static QWaylandXdgToplevelPrivate *get(QWaylandXdgToplevel *toplevel) { return toplevel->d_func(); }
    static Qt::Edges convertToEdges(resize_edge edge);

protected:

    void xdg_toplevel_destroy_resource(Resource *resource) override;

    void xdg_toplevel_destroy(Resource *resource) override;
    void xdg_toplevel_set_parent(Resource *resource, struct ::wl_resource *parent) override;
    void xdg_toplevel_set_title(Resource *resource, const QString &title) override;
    void xdg_toplevel_set_app_id(Resource *resource, const QString &app_id) override;
    void xdg_toplevel_show_window_menu(Resource *resource, struct ::wl_resource *seat, uint32_t serial, int32_t x, int32_t y) override;
    void xdg_toplevel_move(Resource *resource, struct ::wl_resource *seatResource, uint32_t serial) override;
    void xdg_toplevel_resize(Resource *resource, struct ::wl_resource *seat, uint32_t serial, uint32_t edges) override;
    void xdg_toplevel_set_max_size(Resource *resource, int32_t width, int32_t height) override;
    void xdg_toplevel_set_min_size(Resource *resource, int32_t width, int32_t height) override;
    void xdg_toplevel_set_maximized(Resource *resource) override;
    void xdg_toplevel_unset_maximized(Resource *resource) override;
    void xdg_toplevel_set_fullscreen(Resource *resource, struct ::wl_resource *output) override;
    void xdg_toplevel_unset_fullscreen(Resource *resource) override;
    void xdg_toplevel_set_minimized(Resource *resource) override;

public:
    QWaylandXdgSurface *m_xdgSurface = nullptr;
    QWaylandXdgToplevel *m_parentToplevel = nullptr;
    QList<ConfigureEvent> m_pendingConfigures;
    ConfigureEvent m_lastAckedConfigure;
    QString m_title;
    QString m_appId;
    QSize m_maxSize;
    QSize m_minSize = {0, 0};
    QWaylandXdgToplevelDecorationV1 *m_decoration = nullptr;

    static QWaylandSurfaceRole s_role;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgPopupPrivate : public QObjectPrivate, public QtWaylandServer::xdg_popup
{
    Q_DECLARE_PUBLIC(QWaylandXdgPopup)
public:
    struct ConfigureEvent {
        QRect geometry;
        uint serial;
    };

    QWaylandXdgPopupPrivate(QWaylandXdgSurface *xdgSurface, QWaylandXdgSurface *parentXdgSurface,
                            QWaylandXdgPositioner *positioner, const QWaylandResource& resource);

    void handleAckConfigure(uint serial);

    static QWaylandXdgPopupPrivate *get(QWaylandXdgPopup *popup) { return popup->d_func(); }

    static QWaylandSurfaceRole s_role;

private:
    uint sendConfigure(const QRect &geometry);

protected:
    void xdg_popup_destroy(Resource *resource) override;
    void xdg_popup_grab(Resource *resource, struct ::wl_resource *seat, uint32_t serial) override;

private:
    QWaylandXdgSurface *m_xdgSurface = nullptr;
    QWaylandXdgSurface *m_parentXdgSurface = nullptr;
    QWaylandXdgPositionerData m_positionerData;
    QRect m_geometry;
    QList<ConfigureEvent> m_pendingConfigures;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgPositioner : public QtWaylandServer::xdg_positioner
{
public:
    QWaylandXdgPositioner(const QWaylandResource& resource);
    static QWaylandXdgPositioner *fromResource(wl_resource *resource);
    static Qt::Edges convertToEdges(anchor anchor);
    static Qt::Edges convertToEdges(gravity gravity);

protected:
    void xdg_positioner_destroy_resource(Resource *resource) override; //TODO: do something special here?

    void xdg_positioner_destroy(Resource *resource) override;
    void xdg_positioner_set_size(Resource *resource, int32_t width, int32_t height) override;
    void xdg_positioner_set_anchor_rect(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void xdg_positioner_set_anchor(Resource *resource, uint32_t anchor) override;
    void xdg_positioner_set_gravity(Resource *resource, uint32_t gravity) override;
    void xdg_positioner_set_constraint_adjustment(Resource *resource, uint32_t constraint_adjustment) override;
    void xdg_positioner_set_offset(Resource *resource, int32_t x, int32_t y) override;

public:
    QWaylandXdgPositionerData m_data;
};

QT_END_NAMESPACE

#endif // QWAYLANDXDGSHELL_P_H
