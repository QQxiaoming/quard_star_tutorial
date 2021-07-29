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

#ifndef QWAYLANDXDGSHELLV6_P_H
#define QWAYLANDXDGSHELLV6_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwaylandshell_p.h>
#include <QtWaylandCompositor/private/qwayland-server-xdg-shell-unstable-v6.h>

#include <QtWaylandCompositor/QWaylandXdgShellV6>

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

struct Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgPositionerV6Data {
    QSize size;
    QRect anchorRect;
    Qt::Edges anchorEdges = 0;
    Qt::Edges gravityEdges = 0;
    uint constraintAdjustments = ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_NONE;
    QPoint offset;
    QWaylandXdgPositionerV6Data();
    bool isComplete() const;
    QPoint anchorPoint() const;
    QPoint unconstrainedPosition() const;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgShellV6Private
        : public QWaylandShellPrivate
        , public QtWaylandServer::zxdg_shell_v6
{
    Q_DECLARE_PUBLIC(QWaylandXdgShellV6)
public:
    QWaylandXdgShellV6Private();
    void ping(Resource *resource, uint32_t serial);
    void registerXdgSurface(QWaylandXdgSurfaceV6 *xdgSurface);
    void unregisterXdgSurface(QWaylandXdgSurfaceV6 *xdgSurface);
    static QWaylandXdgShellV6Private *get(QWaylandXdgShellV6 *xdgShell) { return xdgShell->d_func(); }
    static Qt::Edges convertToEdges(uint xdgEdges);

    QSet<uint32_t> m_pings;
    QMultiMap<struct wl_client *, QWaylandXdgSurfaceV6 *> m_xdgSurfaces;

    QWaylandXdgSurfaceV6 *xdgSurfaceFromSurface(QWaylandSurface *surface);

protected:
    void zxdg_shell_v6_destroy(Resource *resource) override;
    void zxdg_shell_v6_create_positioner(Resource *resource, uint32_t id) override;
    void zxdg_shell_v6_get_xdg_surface(Resource *resource, uint32_t id,
                                       struct ::wl_resource *surface) override;
    void zxdg_shell_v6_pong(Resource *resource, uint32_t serial) override;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgSurfaceV6Private
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::zxdg_surface_v6
{
    Q_DECLARE_PUBLIC(QWaylandXdgSurfaceV6)
public:
    QWaylandXdgSurfaceV6Private();
    void setWindowType(Qt::WindowType windowType);
    void handleFocusLost();
    void handleFocusReceived();
    static QWaylandXdgSurfaceV6Private *get(QWaylandXdgSurfaceV6 *xdgSurface) { return xdgSurface->d_func(); }

    QRect calculateFallbackWindowGeometry() const;
    void updateFallbackWindowGeometry();

private:
    QWaylandXdgShellV6 *m_xdgShell = nullptr;
    QWaylandSurface *m_surface = nullptr;

    QWaylandXdgToplevelV6 *m_toplevel = nullptr;
    QWaylandXdgPopupV6 *m_popup = nullptr;
    QRect m_windowGeometry;
    bool m_unsetWindowGeometry = true;
    QMargins m_windowMargins;
    Qt::WindowType m_windowType = Qt::WindowType::Window;

    void zxdg_surface_v6_destroy_resource(Resource *resource) override;
    void zxdg_surface_v6_destroy(Resource *resource) override;
    void zxdg_surface_v6_get_toplevel(Resource *resource, uint32_t id) override;
    void zxdg_surface_v6_get_popup(Resource *resource, uint32_t id, struct ::wl_resource *parent, struct ::wl_resource *positioner) override;
    void zxdg_surface_v6_ack_configure(Resource *resource, uint32_t serial) override;
    void zxdg_surface_v6_set_window_geometry(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgToplevelV6Private : public QObjectPrivate, public QtWaylandServer::zxdg_toplevel_v6
{
    Q_DECLARE_PUBLIC(QWaylandXdgToplevelV6)
public:
    struct ConfigureEvent {
        ConfigureEvent() = default;
        ConfigureEvent(const QVector<QWaylandXdgToplevelV6::State>
                       &incomingStates,
                       const QSize &incomingSize, uint incomingSerial)
        : states(incomingStates), size(incomingSize), serial(incomingSerial)
        { }
        QVector<QWaylandXdgToplevelV6::State> states;
        QSize size = {0, 0};
        uint serial = 0;
    };

    QWaylandXdgToplevelV6Private(QWaylandXdgSurfaceV6 *xdgSurface, const QWaylandResource& resource);
    ConfigureEvent lastSentConfigure() const { return m_pendingConfigures.empty() ? m_lastAckedConfigure : m_pendingConfigures.last(); }
    void handleAckConfigure(uint serial); //TODO: move?
    void handleFocusLost();
    void handleFocusReceived();

    static QWaylandXdgToplevelV6Private *get(QWaylandXdgToplevelV6 *toplevel) { return toplevel->d_func(); }

protected:

    void zxdg_toplevel_v6_destroy_resource(Resource *resource) override;

    void zxdg_toplevel_v6_destroy(Resource *resource) override;
    void zxdg_toplevel_v6_set_parent(Resource *resource, struct ::wl_resource *parent) override;
    void zxdg_toplevel_v6_set_title(Resource *resource, const QString &title) override;
    void zxdg_toplevel_v6_set_app_id(Resource *resource, const QString &app_id) override;
    void zxdg_toplevel_v6_show_window_menu(Resource *resource, struct ::wl_resource *seat, uint32_t serial, int32_t x, int32_t y) override;
    void zxdg_toplevel_v6_move(Resource *resource, struct ::wl_resource *seatResource, uint32_t serial) override;
    void zxdg_toplevel_v6_resize(Resource *resource, struct ::wl_resource *seat, uint32_t serial, uint32_t edges) override;
    void zxdg_toplevel_v6_set_max_size(Resource *resource, int32_t width, int32_t height) override;
    void zxdg_toplevel_v6_set_min_size(Resource *resource, int32_t width, int32_t height) override;
    void zxdg_toplevel_v6_set_maximized(Resource *resource) override;
    void zxdg_toplevel_v6_unset_maximized(Resource *resource) override;
    void zxdg_toplevel_v6_set_fullscreen(Resource *resource, struct ::wl_resource *output) override;
    void zxdg_toplevel_v6_unset_fullscreen(Resource *resource) override;
    void zxdg_toplevel_v6_set_minimized(Resource *resource) override;

public:
    QWaylandXdgSurfaceV6 *m_xdgSurface = nullptr;
    QWaylandXdgToplevelV6 *m_parentToplevel = nullptr;
    QList<ConfigureEvent> m_pendingConfigures;
    ConfigureEvent m_lastAckedConfigure;
    QString m_title;
    QString m_appId;
    QSize m_maxSize;
    QSize m_minSize;

    static QWaylandSurfaceRole s_role;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgPopupV6Private : public QObjectPrivate, public QtWaylandServer::zxdg_popup_v6
{
    Q_DECLARE_PUBLIC(QWaylandXdgPopupV6)
public:
    struct ConfigureEvent {
        QRect geometry;
        uint serial;
    };

    QWaylandXdgPopupV6Private(QWaylandXdgSurfaceV6 *xdgSurface, QWaylandXdgSurfaceV6 *parentXdgSurface,
                              QWaylandXdgPositionerV6 *positioner, const QWaylandResource& resource);

    void handleAckConfigure(uint serial);

    static QWaylandXdgPopupV6Private *get(QWaylandXdgPopupV6 *popup) { return popup->d_func(); }

    static QWaylandSurfaceRole s_role;

private:
    uint sendConfigure(const QRect &geometry);

protected:
    void zxdg_popup_v6_destroy(Resource *resource) override;
    void zxdg_popup_v6_grab(Resource *resource, struct ::wl_resource *seat, uint32_t serial) override;

private:
    QWaylandXdgSurfaceV6 *m_xdgSurface = nullptr;
    QWaylandXdgSurfaceV6 *m_parentXdgSurface = nullptr;
    QWaylandXdgPositionerV6Data m_positionerData;
    QRect m_geometry;
    QList<ConfigureEvent> m_pendingConfigures;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgPositionerV6 : public QtWaylandServer::zxdg_positioner_v6
{
public:
    QWaylandXdgPositionerV6(const QWaylandResource& resource);
    static QWaylandXdgPositionerV6 *fromResource(wl_resource *resource);

protected:
    void zxdg_positioner_v6_destroy_resource(Resource *resource) override; //TODO: do something special here?

    void zxdg_positioner_v6_destroy(Resource *resource) override;
    void zxdg_positioner_v6_set_size(Resource *resource, int32_t width, int32_t height) override;
    void zxdg_positioner_v6_set_anchor_rect(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void zxdg_positioner_v6_set_anchor(Resource *resource, uint32_t anchor) override;
    void zxdg_positioner_v6_set_gravity(Resource *resource, uint32_t gravity) override;
    void zxdg_positioner_v6_set_constraint_adjustment(Resource *resource, uint32_t constraint_adjustment) override;
    void zxdg_positioner_v6_set_offset(Resource *resource, int32_t x, int32_t y) override;

public:
    QWaylandXdgPositionerV6Data m_data;
};

QT_END_NAMESPACE

#endif // QWAYLANDXDGSHELLV6_P_H
