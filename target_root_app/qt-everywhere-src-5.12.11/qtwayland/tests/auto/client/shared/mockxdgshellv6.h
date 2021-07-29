/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qwayland-server-xdg-shell-unstable-v6.h>

#include <QSharedPointer>
#include <QVector>

#ifndef MOCKXDGSHELLV6_H
#define MOCKXDGSHELLV6_H

class MockXdgToplevelV6;

namespace Impl {

class XdgToplevelV6;
class XdgShellV6;
class Surface;

class XdgSurfaceV6 : public QtWaylandServer::zxdg_surface_v6
{
public:
    XdgSurfaceV6(XdgShellV6 *shell, Surface *surface, wl_client *client, uint32_t id);
    ~XdgSurfaceV6() override;
    XdgShellV6 *shell() const { return m_shell; }
    Surface *surface() const { return m_surface; }
    XdgToplevelV6 *toplevel() const { return m_toplevel; }

    void sendConfigure(uint32_t serial);
    bool configureSent() const { return m_configureSent; }

protected:
    void zxdg_surface_v6_destroy_resource(Resource *) override { delete this; }
    void zxdg_surface_v6_get_toplevel(Resource *resource, uint32_t id) override;
    void zxdg_surface_v6_set_window_geometry(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void zxdg_surface_v6_destroy(Resource *resource) override;

private:
    Surface *m_surface = nullptr;
    XdgToplevelV6 *m_toplevel = nullptr;
    XdgShellV6 *m_shell = nullptr;
    bool m_configureSent = false;

    friend class XdgToplevelV6;
};

class XdgToplevelV6 : public QtWaylandServer::zxdg_toplevel_v6
{
public:
    XdgToplevelV6(XdgSurfaceV6 *xdgSurface, wl_client *client, uint32_t id, int version);
    ~XdgToplevelV6() override;
    XdgSurfaceV6 *xdgSurface() const { return m_xdgSurface; }

    QSharedPointer<MockXdgToplevelV6> mockToplevel() const { return m_mockToplevel; }

protected:
    void zxdg_toplevel_v6_destroy_resource(Resource *) override { delete this; }
    void zxdg_toplevel_v6_destroy(Resource *resource) override;
    void zxdg_toplevel_v6_set_minimized(Resource *resource) override;
    void zxdg_toplevel_v6_set_maximized(Resource *resource) override;
    void zxdg_toplevel_v6_unset_maximized(Resource *resource) override;
    void zxdg_toplevel_v6_set_fullscreen(Resource *resource, struct ::wl_resource *output) override;
    void zxdg_toplevel_v6_unset_fullscreen(Resource *resource) override;

private:
    XdgSurfaceV6 *m_xdgSurface = nullptr;
    QSharedPointer<MockXdgToplevelV6> m_mockToplevel;
};

class XdgShellV6 : public QtWaylandServer::zxdg_shell_v6
{
public:
    explicit XdgShellV6(::wl_display *display) : zxdg_shell_v6(display, 1) {}
    QVector<XdgToplevelV6 *> toplevels() const { return m_toplevels; }

protected:
    void zxdg_shell_v6_get_xdg_surface(Resource *resource, uint32_t id, ::wl_resource *surface) override;

private:
    void addToplevel(XdgToplevelV6 *toplevel) { m_toplevels.append(toplevel); }
    void removeToplevel(XdgToplevelV6 *toplevel) { m_toplevels.removeOne(toplevel); }
    QVector<XdgToplevelV6 *> m_toplevels;

    friend class XdgToplevelV6;
};

} // namespace Impl

#endif // MOCKXDGSHELLV6_H
