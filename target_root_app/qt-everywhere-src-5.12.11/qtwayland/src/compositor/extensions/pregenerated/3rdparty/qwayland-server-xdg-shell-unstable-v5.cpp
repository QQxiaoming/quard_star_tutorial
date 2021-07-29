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

#include <QtWaylandCompositor/private/qwayland-server-xdg-shell-unstable-v5_p.h>

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")

namespace QtWaylandServer {
    xdg_shell_v5::xdg_shell_v5(struct ::wl_client *client, int id, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(client, id, version);
    }

    xdg_shell_v5::xdg_shell_v5(struct ::wl_display *display, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(display, version);
    }

    xdg_shell_v5::xdg_shell_v5(struct ::wl_resource *resource)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(resource);
    }

    xdg_shell_v5::xdg_shell_v5()
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
    }

    xdg_shell_v5::~xdg_shell_v5()
    {
        for (auto resource : qAsConst(m_resource_map))
            wl_resource_set_implementation(resource->handle, nullptr, nullptr, nullptr);

        if (m_global) {
            wl_global_destroy(m_global);
            wl_list_remove(&m_displayDestroyedListener.link);
        }
    }

    void xdg_shell_v5::init(struct ::wl_client *client, int id, int version)
    {
        m_resource = bind(client, id, version);
    }

    void xdg_shell_v5::init(struct ::wl_resource *resource)
    {
        m_resource = bind(resource);
    }

    xdg_shell_v5::Resource *xdg_shell_v5::add(struct ::wl_client *client, int version)
    {
        Resource *resource = bind(client, 0, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    xdg_shell_v5::Resource *xdg_shell_v5::add(struct ::wl_client *client, int id, int version)
    {
        Resource *resource = bind(client, id, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    void xdg_shell_v5::init(struct ::wl_display *display, int version)
    {
        m_global = wl_global_create(display, &::xdg_shell_v5_interface, version, this, bind_func);
        m_globalVersion = version;
        m_displayDestroyedListener.notify = xdg_shell_v5::display_destroy_func;
        m_displayDestroyedListener.parent = this;
        wl_display_add_destroy_listener(display, &m_displayDestroyedListener);
    }

    const struct wl_interface *xdg_shell_v5::interface()
    {
        return &::xdg_shell_v5_interface;
    }

    xdg_shell_v5::Resource *xdg_shell_v5::xdg_shell_allocate()
    {
        return new Resource;
    }

    void xdg_shell_v5::xdg_shell_bind_resource(Resource *)
    {
    }

    void xdg_shell_v5::xdg_shell_destroy_resource(Resource *)
    {
    }

    void xdg_shell_v5::bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id)
    {
        xdg_shell_v5 *that = static_cast<xdg_shell_v5 *>(data);
        that->add(client, id, qMin(that->m_globalVersion, version));
    }

    void xdg_shell_v5::display_destroy_func(struct ::wl_listener *listener, void *data)
    {
        Q_UNUSED(data);
        xdg_shell_v5 *that = static_cast<xdg_shell_v5::DisplayDestroyedListener *>(listener)->parent;
        that->m_global = nullptr;
    }

    void xdg_shell_v5::destroy_func(struct ::wl_resource *client_resource)
    {
        Resource *resource = Resource::fromResource(client_resource);
        xdg_shell_v5 *that = resource->xdg_shell_object;
        that->m_resource_map.remove(resource->client(), resource);
        that->xdg_shell_destroy_resource(resource);
        delete resource;
    }

    xdg_shell_v5::Resource *xdg_shell_v5::bind(struct ::wl_client *client, uint32_t id, int version)
    {
        Q_ASSERT_X(!wl_client_get_object(client, id), "QWaylandObject bind", QStringLiteral("binding to object %1 more than once").arg(id).toLocal8Bit().constData());
        struct ::wl_resource *handle = wl_resource_create(client, &::xdg_shell_v5_interface, version, id);
        return bind(handle);
    }

    xdg_shell_v5::Resource *xdg_shell_v5::bind(struct ::wl_resource *handle)
    {
        Resource *resource = xdg_shell_allocate();
        resource->xdg_shell_object = this;

        wl_resource_set_implementation(handle, &m_xdg_shell_interface, resource, destroy_func);
        resource->handle = handle;
        xdg_shell_bind_resource(resource);
        return resource;
    }
    xdg_shell_v5::Resource *xdg_shell_v5::Resource::fromResource(struct ::wl_resource *resource)
    {
        if (wl_resource_instance_of(resource, &::xdg_shell_v5_interface, &m_xdg_shell_interface))
            return static_cast<Resource *>(resource->data);
        return nullptr;
    }

    const struct ::xdg_shell_v5_interface xdg_shell_v5::m_xdg_shell_interface = {
        xdg_shell_v5::handle_destroy,
        xdg_shell_v5::handle_use_unstable_version,
        xdg_shell_v5::handle_get_xdg_surface,
        xdg_shell_v5::handle_get_xdg_popup,
        xdg_shell_v5::handle_pong
    };

    void xdg_shell_v5::xdg_shell_destroy(Resource *)
    {
    }

    void xdg_shell_v5::xdg_shell_use_unstable_version(Resource *, int32_t )
    {
    }

    void xdg_shell_v5::xdg_shell_get_xdg_surface(Resource *, uint32_t, struct ::wl_resource *)
    {
    }

    void xdg_shell_v5::xdg_shell_get_xdg_popup(Resource *, uint32_t, struct ::wl_resource *, struct ::wl_resource *, struct ::wl_resource *, uint32_t , int32_t , int32_t )
    {
    }

    void xdg_shell_v5::xdg_shell_pong(Resource *, uint32_t )
    {
    }


    void xdg_shell_v5::handle_destroy(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_shell_v5 *>(r->xdg_shell_object)->xdg_shell_destroy(
            r);
    }

    void xdg_shell_v5::handle_use_unstable_version(
        ::wl_client *client,
        struct wl_resource *resource,
        int32_t version)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_shell_v5 *>(r->xdg_shell_object)->xdg_shell_use_unstable_version(
            r,
            version);
    }

    void xdg_shell_v5::handle_get_xdg_surface(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t id,
        struct ::wl_resource *surface)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_shell_v5 *>(r->xdg_shell_object)->xdg_shell_get_xdg_surface(
            r,
            id,
            surface);
    }

    void xdg_shell_v5::handle_get_xdg_popup(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t id,
        struct ::wl_resource *surface,
        struct ::wl_resource *parent,
        struct ::wl_resource *seat,
        uint32_t serial,
        int32_t x,
        int32_t y)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_shell_v5 *>(r->xdg_shell_object)->xdg_shell_get_xdg_popup(
            r,
            id,
            surface,
            parent,
            seat,
            serial,
            x,
            y);
    }

    void xdg_shell_v5::handle_pong(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t serial)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_shell_v5 *>(r->xdg_shell_object)->xdg_shell_pong(
            r,
            serial);
    }

    void xdg_shell_v5::send_ping(uint32_t serial)
    {
        send_ping(
            m_resource->handle,
            serial);
    }

    void xdg_shell_v5::send_ping(struct ::wl_resource *resource, uint32_t serial)
    {
        xdg_shell_send_ping(
            resource,
            serial);
    }


    xdg_surface_v5::xdg_surface_v5(struct ::wl_client *client, int id, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(client, id, version);
    }

    xdg_surface_v5::xdg_surface_v5(struct ::wl_display *display, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(display, version);
    }

    xdg_surface_v5::xdg_surface_v5(struct ::wl_resource *resource)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(resource);
    }

    xdg_surface_v5::xdg_surface_v5()
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
    }

    xdg_surface_v5::~xdg_surface_v5()
    {
        for (auto resource : qAsConst(m_resource_map))
            wl_resource_set_implementation(resource->handle, nullptr, nullptr, nullptr);

        if (m_global) {
            wl_global_destroy(m_global);
            wl_list_remove(&m_displayDestroyedListener.link);
        }
    }

    void xdg_surface_v5::init(struct ::wl_client *client, int id, int version)
    {
        m_resource = bind(client, id, version);
    }

    void xdg_surface_v5::init(struct ::wl_resource *resource)
    {
        m_resource = bind(resource);
    }

    xdg_surface_v5::Resource *xdg_surface_v5::add(struct ::wl_client *client, int version)
    {
        Resource *resource = bind(client, 0, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    xdg_surface_v5::Resource *xdg_surface_v5::add(struct ::wl_client *client, int id, int version)
    {
        Resource *resource = bind(client, id, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    void xdg_surface_v5::init(struct ::wl_display *display, int version)
    {
        m_global = wl_global_create(display, &::xdg_surface_v5_interface, version, this, bind_func);
        m_globalVersion = version;
        m_displayDestroyedListener.notify = xdg_surface_v5::display_destroy_func;
        m_displayDestroyedListener.parent = this;
        wl_display_add_destroy_listener(display, &m_displayDestroyedListener);
    }

    const struct wl_interface *xdg_surface_v5::interface()
    {
        return &::xdg_surface_v5_interface;
    }

    xdg_surface_v5::Resource *xdg_surface_v5::xdg_surface_allocate()
    {
        return new Resource;
    }

    void xdg_surface_v5::xdg_surface_bind_resource(Resource *)
    {
    }

    void xdg_surface_v5::xdg_surface_destroy_resource(Resource *)
    {
    }

    void xdg_surface_v5::bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id)
    {
        xdg_surface_v5 *that = static_cast<xdg_surface_v5 *>(data);
        that->add(client, id, qMin(that->m_globalVersion, version));
    }

    void xdg_surface_v5::display_destroy_func(struct ::wl_listener *listener, void *data)
    {
        Q_UNUSED(data);
        xdg_surface_v5 *that = static_cast<xdg_surface_v5::DisplayDestroyedListener *>(listener)->parent;
        that->m_global = nullptr;
    }

    void xdg_surface_v5::destroy_func(struct ::wl_resource *client_resource)
    {
        Resource *resource = Resource::fromResource(client_resource);
        xdg_surface_v5 *that = resource->xdg_surface_object;
        that->m_resource_map.remove(resource->client(), resource);
        that->xdg_surface_destroy_resource(resource);
        delete resource;
    }

    xdg_surface_v5::Resource *xdg_surface_v5::bind(struct ::wl_client *client, uint32_t id, int version)
    {
        Q_ASSERT_X(!wl_client_get_object(client, id), "QWaylandObject bind", QStringLiteral("binding to object %1 more than once").arg(id).toLocal8Bit().constData());
        struct ::wl_resource *handle = wl_resource_create(client, &::xdg_surface_v5_interface, version, id);
        return bind(handle);
    }

    xdg_surface_v5::Resource *xdg_surface_v5::bind(struct ::wl_resource *handle)
    {
        Resource *resource = xdg_surface_allocate();
        resource->xdg_surface_object = this;

        wl_resource_set_implementation(handle, &m_xdg_surface_interface, resource, destroy_func);
        resource->handle = handle;
        xdg_surface_bind_resource(resource);
        return resource;
    }
    xdg_surface_v5::Resource *xdg_surface_v5::Resource::fromResource(struct ::wl_resource *resource)
    {
        if (wl_resource_instance_of(resource, &::xdg_surface_v5_interface, &m_xdg_surface_interface))
            return static_cast<Resource *>(resource->data);
        return nullptr;
    }

    const struct ::xdg_surface_v5_interface xdg_surface_v5::m_xdg_surface_interface = {
        xdg_surface_v5::handle_destroy,
        xdg_surface_v5::handle_set_parent,
        xdg_surface_v5::handle_set_title,
        xdg_surface_v5::handle_set_app_id,
        xdg_surface_v5::handle_show_window_menu,
        xdg_surface_v5::handle_move,
        xdg_surface_v5::handle_resize,
        xdg_surface_v5::handle_ack_configure,
        xdg_surface_v5::handle_set_window_geometry,
        xdg_surface_v5::handle_set_maximized,
        xdg_surface_v5::handle_unset_maximized,
        xdg_surface_v5::handle_set_fullscreen,
        xdg_surface_v5::handle_unset_fullscreen,
        xdg_surface_v5::handle_set_minimized
    };

    void xdg_surface_v5::xdg_surface_destroy(Resource *)
    {
    }

    void xdg_surface_v5::xdg_surface_set_parent(Resource *, struct ::wl_resource *)
    {
    }

    void xdg_surface_v5::xdg_surface_set_title(Resource *, const QString &)
    {
    }

    void xdg_surface_v5::xdg_surface_set_app_id(Resource *, const QString &)
    {
    }

    void xdg_surface_v5::xdg_surface_show_window_menu(Resource *, struct ::wl_resource *, uint32_t , int32_t , int32_t )
    {
    }

    void xdg_surface_v5::xdg_surface_move(Resource *, struct ::wl_resource *, uint32_t )
    {
    }

    void xdg_surface_v5::xdg_surface_resize(Resource *, struct ::wl_resource *, uint32_t , uint32_t )
    {
    }

    void xdg_surface_v5::xdg_surface_ack_configure(Resource *, uint32_t )
    {
    }

    void xdg_surface_v5::xdg_surface_set_window_geometry(Resource *, int32_t , int32_t , int32_t , int32_t )
    {
    }

    void xdg_surface_v5::xdg_surface_set_maximized(Resource *)
    {
    }

    void xdg_surface_v5::xdg_surface_unset_maximized(Resource *)
    {
    }

    void xdg_surface_v5::xdg_surface_set_fullscreen(Resource *, struct ::wl_resource *)
    {
    }

    void xdg_surface_v5::xdg_surface_unset_fullscreen(Resource *)
    {
    }

    void xdg_surface_v5::xdg_surface_set_minimized(Resource *)
    {
    }


    void xdg_surface_v5::handle_destroy(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_destroy(
            r);
    }

    void xdg_surface_v5::handle_set_parent(
        ::wl_client *client,
        struct wl_resource *resource,
        struct ::wl_resource *parent)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_set_parent(
            r,
            parent);
    }

    void xdg_surface_v5::handle_set_title(
        ::wl_client *client,
        struct wl_resource *resource,
        const char *title)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_set_title(
            r,
            QString::fromUtf8(title));
    }

    void xdg_surface_v5::handle_set_app_id(
        ::wl_client *client,
        struct wl_resource *resource,
        const char *app_id)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_set_app_id(
            r,
            QString::fromUtf8(app_id));
    }

    void xdg_surface_v5::handle_show_window_menu(
        ::wl_client *client,
        struct wl_resource *resource,
        struct ::wl_resource *seat,
        uint32_t serial,
        int32_t x,
        int32_t y)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_show_window_menu(
            r,
            seat,
            serial,
            x,
            y);
    }

    void xdg_surface_v5::handle_move(
        ::wl_client *client,
        struct wl_resource *resource,
        struct ::wl_resource *seat,
        uint32_t serial)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_move(
            r,
            seat,
            serial);
    }

    void xdg_surface_v5::handle_resize(
        ::wl_client *client,
        struct wl_resource *resource,
        struct ::wl_resource *seat,
        uint32_t serial,
        uint32_t edges)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_resize(
            r,
            seat,
            serial,
            edges);
    }

    void xdg_surface_v5::handle_ack_configure(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t serial)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_ack_configure(
            r,
            serial);
    }

    void xdg_surface_v5::handle_set_window_geometry(
        ::wl_client *client,
        struct wl_resource *resource,
        int32_t x,
        int32_t y,
        int32_t width,
        int32_t height)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_set_window_geometry(
            r,
            x,
            y,
            width,
            height);
    }

    void xdg_surface_v5::handle_set_maximized(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_set_maximized(
            r);
    }

    void xdg_surface_v5::handle_unset_maximized(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_unset_maximized(
            r);
    }

    void xdg_surface_v5::handle_set_fullscreen(
        ::wl_client *client,
        struct wl_resource *resource,
        struct ::wl_resource *output)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_set_fullscreen(
            r,
            output);
    }

    void xdg_surface_v5::handle_unset_fullscreen(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_unset_fullscreen(
            r);
    }

    void xdg_surface_v5::handle_set_minimized(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface_v5 *>(r->xdg_surface_object)->xdg_surface_set_minimized(
            r);
    }

    void xdg_surface_v5::send_configure(int32_t width, int32_t height, const QByteArray &states, uint32_t serial)
    {
        send_configure(
            m_resource->handle,
            width,
            height,
            states,
            serial);
    }

    void xdg_surface_v5::send_configure(struct ::wl_resource *resource, int32_t width, int32_t height, const QByteArray &states, uint32_t serial)
    {
        struct wl_array states_data;
        states_data.size = states.size();
        states_data.data = static_cast<void *>(const_cast<char *>(states.constData()));
        states_data.alloc = 0;

        xdg_surface_send_configure(
            resource,
            width,
            height,
            &states_data,
            serial);
    }


    void xdg_surface_v5::send_close()
    {
        send_close(
            m_resource->handle);
    }

    void xdg_surface_v5::send_close(struct ::wl_resource *resource)
    {
        xdg_surface_send_close(
            resource);
    }


    xdg_popup_v5::xdg_popup_v5(struct ::wl_client *client, int id, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(client, id, version);
    }

    xdg_popup_v5::xdg_popup_v5(struct ::wl_display *display, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(display, version);
    }

    xdg_popup_v5::xdg_popup_v5(struct ::wl_resource *resource)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(resource);
    }

    xdg_popup_v5::xdg_popup_v5()
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
    }

    xdg_popup_v5::~xdg_popup_v5()
    {
        for (auto resource : qAsConst(m_resource_map))
            wl_resource_set_implementation(resource->handle, nullptr, nullptr, nullptr);

        if (m_global) {
            wl_global_destroy(m_global);
            wl_list_remove(&m_displayDestroyedListener.link);
        }
    }

    void xdg_popup_v5::init(struct ::wl_client *client, int id, int version)
    {
        m_resource = bind(client, id, version);
    }

    void xdg_popup_v5::init(struct ::wl_resource *resource)
    {
        m_resource = bind(resource);
    }

    xdg_popup_v5::Resource *xdg_popup_v5::add(struct ::wl_client *client, int version)
    {
        Resource *resource = bind(client, 0, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    xdg_popup_v5::Resource *xdg_popup_v5::add(struct ::wl_client *client, int id, int version)
    {
        Resource *resource = bind(client, id, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    void xdg_popup_v5::init(struct ::wl_display *display, int version)
    {
        m_global = wl_global_create(display, &::xdg_popup_v5_interface, version, this, bind_func);
        m_globalVersion = version;
        m_displayDestroyedListener.notify = xdg_popup_v5::display_destroy_func;
        m_displayDestroyedListener.parent = this;
        wl_display_add_destroy_listener(display, &m_displayDestroyedListener);
    }

    const struct wl_interface *xdg_popup_v5::interface()
    {
        return &::xdg_popup_v5_interface;
    }

    xdg_popup_v5::Resource *xdg_popup_v5::xdg_popup_allocate()
    {
        return new Resource;
    }

    void xdg_popup_v5::xdg_popup_bind_resource(Resource *)
    {
    }

    void xdg_popup_v5::xdg_popup_destroy_resource(Resource *)
    {
    }

    void xdg_popup_v5::bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id)
    {
        xdg_popup_v5 *that = static_cast<xdg_popup_v5 *>(data);
        that->add(client, id, qMin(that->m_globalVersion, version));
    }

    void xdg_popup_v5::display_destroy_func(struct ::wl_listener *listener, void *data)
    {
        Q_UNUSED(data);
        xdg_popup_v5 *that = static_cast<xdg_popup_v5::DisplayDestroyedListener *>(listener)->parent;
        that->m_global = nullptr;
    }

    void xdg_popup_v5::destroy_func(struct ::wl_resource *client_resource)
    {
        Resource *resource = Resource::fromResource(client_resource);
        xdg_popup_v5 *that = resource->xdg_popup_object;
        that->m_resource_map.remove(resource->client(), resource);
        that->xdg_popup_destroy_resource(resource);
        delete resource;
    }

    xdg_popup_v5::Resource *xdg_popup_v5::bind(struct ::wl_client *client, uint32_t id, int version)
    {
        Q_ASSERT_X(!wl_client_get_object(client, id), "QWaylandObject bind", QStringLiteral("binding to object %1 more than once").arg(id).toLocal8Bit().constData());
        struct ::wl_resource *handle = wl_resource_create(client, &::xdg_popup_v5_interface, version, id);
        return bind(handle);
    }

    xdg_popup_v5::Resource *xdg_popup_v5::bind(struct ::wl_resource *handle)
    {
        Resource *resource = xdg_popup_allocate();
        resource->xdg_popup_object = this;

        wl_resource_set_implementation(handle, &m_xdg_popup_interface, resource, destroy_func);
        resource->handle = handle;
        xdg_popup_bind_resource(resource);
        return resource;
    }
    xdg_popup_v5::Resource *xdg_popup_v5::Resource::fromResource(struct ::wl_resource *resource)
    {
        if (wl_resource_instance_of(resource, &::xdg_popup_v5_interface, &m_xdg_popup_interface))
            return static_cast<Resource *>(resource->data);
        return nullptr;
    }

    const struct ::xdg_popup_v5_interface xdg_popup_v5::m_xdg_popup_interface = {
        xdg_popup_v5::handle_destroy
    };

    void xdg_popup_v5::xdg_popup_destroy(Resource *)
    {
    }


    void xdg_popup_v5::handle_destroy(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_popup_v5 *>(r->xdg_popup_object)->xdg_popup_destroy(
            r);
    }

    void xdg_popup_v5::send_popup_done()
    {
        send_popup_done(
            m_resource->handle);
    }

    void xdg_popup_v5::send_popup_done(struct ::wl_resource *resource)
    {
        xdg_popup_send_popup_done(
            resource);
    }

}

QT_WARNING_POP
QT_END_NAMESPACE
