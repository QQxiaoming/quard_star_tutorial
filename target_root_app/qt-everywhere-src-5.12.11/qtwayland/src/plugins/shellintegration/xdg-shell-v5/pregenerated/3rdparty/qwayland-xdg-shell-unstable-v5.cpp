// Hand-edited to resolve conflicts with xdg-shell stable
/*
 * Copyright © 2008-2013 Kristian Høgsberg
 * Copyright © 2013      Rafael Antognolli
 * Copyright © 2013      Jasper St. Pierre
 * Copyright © 2010-2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "qwayland-xdg-shell-unstable-v5_p.h"

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")

namespace QtWayland {
    xdg_shell_v5::xdg_shell_v5(struct ::wl_registry *registry, int id, int version)
    {
        init(registry, id, version);
    }

    xdg_shell_v5::xdg_shell_v5(struct ::xdg_shell_v5 *obj)
        : m_xdg_shell(obj)
    {
        init_listener();
    }

    xdg_shell_v5::xdg_shell_v5()
        : m_xdg_shell(nullptr)
    {
    }

    xdg_shell_v5::~xdg_shell_v5()
    {
    }

    void xdg_shell_v5::init(struct ::wl_registry *registry, int id, int version)
    {
        m_xdg_shell = static_cast<struct ::xdg_shell_v5 *>(wl_registry_bind(registry, id, &xdg_shell_v5_interface, version));
        init_listener();
    }

    void xdg_shell_v5::init(struct ::xdg_shell_v5 *obj)
    {
        m_xdg_shell = obj;
        init_listener();
    }

    bool xdg_shell_v5::isInitialized() const
    {
        return m_xdg_shell != nullptr;
    }

    const struct wl_interface *xdg_shell_v5::interface()
    {
        return &::xdg_shell_v5_interface;
    }

    void xdg_shell_v5::destroy()
    {
        xdg_shell_destroy(
            m_xdg_shell);
        m_xdg_shell = nullptr;
    }

    void xdg_shell_v5::use_unstable_version(int32_t version)
    {
        xdg_shell_use_unstable_version(
            m_xdg_shell,
            version);
    }

    struct ::xdg_surface_v5 *xdg_shell_v5::get_xdg_surface(struct ::wl_surface *surface)
    {
        return xdg_shell_get_xdg_surface(
            m_xdg_shell,
            surface);
    }

    struct ::xdg_popup_v5 *xdg_shell_v5::get_xdg_popup(struct ::wl_surface *surface, struct ::wl_surface *parent, struct ::wl_seat *seat, uint32_t serial, int32_t x, int32_t y)
    {
        return xdg_shell_get_xdg_popup(
            m_xdg_shell,
            surface,
            parent,
            seat,
            serial,
            x,
            y);
    }

    void xdg_shell_v5::pong(uint32_t serial)
    {
        xdg_shell_pong(
            m_xdg_shell,
            serial);
    }

    void xdg_shell_v5::xdg_shell_ping(uint32_t )
    {
    }

    void xdg_shell_v5::handle_ping(
        void *data,
        struct ::xdg_shell_v5 *object,
        uint32_t serial)
    {
        Q_UNUSED(object);
        static_cast<xdg_shell_v5 *>(data)->xdg_shell_ping(
            serial);
    }

    const struct xdg_shell_listener xdg_shell_v5::m_xdg_shell_listener = {
        xdg_shell_v5::handle_ping
    };

    void xdg_shell_v5::init_listener()
    {
        xdg_shell_add_listener(m_xdg_shell, &m_xdg_shell_listener, this);
    }

    xdg_surface_v5::xdg_surface_v5(struct ::wl_registry *registry, int id, int version)
    {
        init(registry, id, version);
    }

    xdg_surface_v5::xdg_surface_v5(struct ::xdg_surface_v5 *obj)
        : m_xdg_surface(obj)
    {
        init_listener();
    }

    xdg_surface_v5::xdg_surface_v5()
        : m_xdg_surface(nullptr)
    {
    }

    xdg_surface_v5::~xdg_surface_v5()
    {
    }

    void xdg_surface_v5::init(struct ::wl_registry *registry, int id, int version)
    {
        m_xdg_surface = static_cast<struct ::xdg_surface_v5 *>(wl_registry_bind(registry, id, &xdg_surface_v5_interface, version));
        init_listener();
    }

    void xdg_surface_v5::init(struct ::xdg_surface_v5 *obj)
    {
        m_xdg_surface = obj;
        init_listener();
    }

    bool xdg_surface_v5::isInitialized() const
    {
        return m_xdg_surface != nullptr;
    }

    const struct wl_interface *xdg_surface_v5::interface()
    {
        return &::xdg_surface_v5_interface;
    }

    void xdg_surface_v5::destroy()
    {
        xdg_surface_destroy(
            m_xdg_surface);
        m_xdg_surface = nullptr;
    }

    void xdg_surface_v5::set_parent(struct ::xdg_surface_v5 *parent)
    {
        xdg_surface_set_parent(
            m_xdg_surface,
            parent);
    }

    void xdg_surface_v5::set_title(const QString &title)
    {
        xdg_surface_set_title(
            m_xdg_surface,
            title.toUtf8().constData());
    }

    void xdg_surface_v5::set_app_id(const QString &app_id)
    {
        xdg_surface_set_app_id(
            m_xdg_surface,
            app_id.toUtf8().constData());
    }

    void xdg_surface_v5::show_window_menu(struct ::wl_seat *seat, uint32_t serial, int32_t x, int32_t y)
    {
        xdg_surface_show_window_menu(
            m_xdg_surface,
            seat,
            serial,
            x,
            y);
    }

    void xdg_surface_v5::move(struct ::wl_seat *seat, uint32_t serial)
    {
        xdg_surface_move(
            m_xdg_surface,
            seat,
            serial);
    }

    void xdg_surface_v5::resize(struct ::wl_seat *seat, uint32_t serial, uint32_t edges)
    {
        xdg_surface_resize(
            m_xdg_surface,
            seat,
            serial,
            edges);
    }

    void xdg_surface_v5::ack_configure(uint32_t serial)
    {
        xdg_surface_ack_configure(
            m_xdg_surface,
            serial);
    }

    void xdg_surface_v5::set_window_geometry(int32_t x, int32_t y, int32_t width, int32_t height)
    {
        xdg_surface_set_window_geometry(
            m_xdg_surface,
            x,
            y,
            width,
            height);
    }

    void xdg_surface_v5::set_maximized()
    {
        xdg_surface_set_maximized(
            m_xdg_surface);
    }

    void xdg_surface_v5::unset_maximized()
    {
        xdg_surface_unset_maximized(
            m_xdg_surface);
    }

    void xdg_surface_v5::set_fullscreen(struct ::wl_output *output)
    {
        xdg_surface_set_fullscreen(
            m_xdg_surface,
            output);
    }

    void xdg_surface_v5::unset_fullscreen()
    {
        xdg_surface_unset_fullscreen(
            m_xdg_surface);
    }

    void xdg_surface_v5::set_minimized()
    {
        xdg_surface_set_minimized(
            m_xdg_surface);
    }

    void xdg_surface_v5::xdg_surface_configure(int32_t , int32_t , wl_array *, uint32_t )
    {
    }

    void xdg_surface_v5::handle_configure(
        void *data,
        struct ::xdg_surface_v5 *object,
        int32_t width,
        int32_t height,
        wl_array *states,
        uint32_t serial)
    {
        Q_UNUSED(object);
        static_cast<xdg_surface_v5 *>(data)->xdg_surface_configure(
            width,
            height,
            states,
            serial);
    }

    void xdg_surface_v5::xdg_surface_close()
    {
    }

    void xdg_surface_v5::handle_close(
        void *data,
        struct ::xdg_surface_v5 *object)
    {
        Q_UNUSED(object);
        static_cast<xdg_surface_v5 *>(data)->xdg_surface_close();
    }

    const struct xdg_surface_listener xdg_surface_v5::m_xdg_surface_listener = {
        xdg_surface_v5::handle_configure,
        xdg_surface_v5::handle_close
    };

    void xdg_surface_v5::init_listener()
    {
        xdg_surface_add_listener(m_xdg_surface, &m_xdg_surface_listener, this);
    }

    xdg_popup_v5::xdg_popup_v5(struct ::wl_registry *registry, int id, int version)
    {
        init(registry, id, version);
    }

    xdg_popup_v5::xdg_popup_v5(struct ::xdg_popup_v5 *obj)
        : m_xdg_popup(obj)
    {
        init_listener();
    }

    xdg_popup_v5::xdg_popup_v5()
        : m_xdg_popup(nullptr)
    {
    }

    xdg_popup_v5::~xdg_popup_v5()
    {
    }

    void xdg_popup_v5::init(struct ::wl_registry *registry, int id, int version)
    {
        m_xdg_popup = static_cast<struct ::xdg_popup_v5 *>(wl_registry_bind(registry, id, &xdg_popup_v5_interface, version));
        init_listener();
    }

    void xdg_popup_v5::init(struct ::xdg_popup_v5 *obj)
    {
        m_xdg_popup = obj;
        init_listener();
    }

    bool xdg_popup_v5::isInitialized() const
    {
        return m_xdg_popup != nullptr;
    }

    const struct wl_interface *xdg_popup_v5::interface()
    {
        return &::xdg_popup_v5_interface;
    }

    void xdg_popup_v5::destroy()
    {
        xdg_popup_destroy(
            m_xdg_popup);
        m_xdg_popup = nullptr;
    }

    void xdg_popup_v5::xdg_popup_popup_done()
    {
    }

    void xdg_popup_v5::handle_popup_done(
        void *data,
        struct ::xdg_popup_v5 *object)
    {
        Q_UNUSED(object);
        static_cast<xdg_popup_v5 *>(data)->xdg_popup_popup_done();
    }

    const struct xdg_popup_listener xdg_popup_v5::m_xdg_popup_listener = {
        xdg_popup_v5::handle_popup_done
    };

    void xdg_popup_v5::init_listener()
    {
        xdg_popup_add_listener(m_xdg_popup, &m_xdg_popup_listener, this);
    }
}

QT_WARNING_POP
QT_END_NAMESPACE
