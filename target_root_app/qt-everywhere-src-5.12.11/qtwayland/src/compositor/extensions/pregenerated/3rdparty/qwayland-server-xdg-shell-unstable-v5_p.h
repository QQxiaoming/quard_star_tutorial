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

#ifndef QT_WAYLAND_SERVER_XDG_SHELL_UNSTABLE_V5
#define QT_WAYLAND_SERVER_XDG_SHELL_UNSTABLE_V5

#include "wayland-server.h"
#include <QtWaylandCompositor/private/wayland-xdg-shell-unstable-v5-server-protocol_p.h>
#include <QByteArray>
#include <QMultiMap>
#include <QString>

#ifndef WAYLAND_VERSION_CHECK
#define WAYLAND_VERSION_CHECK(major, minor, micro) \
    ((WAYLAND_VERSION_MAJOR > (major)) || \
    (WAYLAND_VERSION_MAJOR == (major) && WAYLAND_VERSION_MINOR > (minor)) || \
    (WAYLAND_VERSION_MAJOR == (major) && WAYLAND_VERSION_MINOR == (minor) && WAYLAND_VERSION_MICRO >= (micro)))
#endif

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")
QT_WARNING_DISABLE_CLANG("-Wmissing-field-initializers")

#if !defined(Q_WAYLAND_SERVER_XDG_SHELL_UNSTABLE_V5_EXPORT)
#  if defined(QT_SHARED)
#    define Q_WAYLAND_SERVER_XDG_SHELL_UNSTABLE_V5_EXPORT Q_DECL_EXPORT
#  else
#    define Q_WAYLAND_SERVER_XDG_SHELL_UNSTABLE_V5_EXPORT
#  endif
#endif

namespace QtWaylandServer {
    class Q_WAYLAND_SERVER_XDG_SHELL_UNSTABLE_V5_EXPORT xdg_shell_v5
    {
    public:
        xdg_shell_v5(struct ::wl_client *client, int id, int version);
        xdg_shell_v5(struct ::wl_display *display, int version);
        xdg_shell_v5(struct ::wl_resource *resource);
        xdg_shell_v5();

        virtual ~xdg_shell_v5();

        class Resource
        {
        public:
            Resource() : xdg_shell_object(nullptr), handle(nullptr) {}
            virtual ~Resource() {}

            xdg_shell_v5 *xdg_shell_object;
            struct ::wl_resource *handle;

            struct ::wl_client *client() const { return wl_resource_get_client(handle); }
            int version() const { return wl_resource_get_version(handle); }

            static Resource *fromResource(struct ::wl_resource *resource);
        };

        void init(struct ::wl_client *client, int id, int version);
        void init(struct ::wl_display *display, int version);
        void init(struct ::wl_resource *resource);

        Resource *add(struct ::wl_client *client, int version);
        Resource *add(struct ::wl_client *client, int id, int version);
        Resource *add(struct wl_list *resource_list, struct ::wl_client *client, int id, int version);

        Resource *resource() { return m_resource; }
        const Resource *resource() const { return m_resource; }

        QMultiMap<struct ::wl_client*, Resource*> resourceMap() { return m_resource_map; }
        const QMultiMap<struct ::wl_client*, Resource*> resourceMap() const { return m_resource_map; }

        bool isGlobal() const { return m_global != nullptr; }
        bool isResource() const { return m_resource != nullptr; }

        static const struct ::wl_interface *interface();
        static QByteArray interfaceName() { return interface()->name; }
        static int interfaceVersion() { return interface()->version; }


        enum version {
            version_current = 5 // Always the latest version
        };

        enum error {
            error_role = 0, // given wl_surface has another role
            error_defunct_surfaces = 1, // xdg_shell was destroyed before children
            error_not_the_topmost_popup = 2, // the client tried to map or destroy a non-topmost popup
            error_invalid_popup_parent = 3 // the client specified an invalid popup parent surface
        };

        void send_ping(uint32_t serial);
        void send_ping(struct ::wl_resource *resource, uint32_t serial);

    protected:
        virtual Resource *xdg_shell_allocate();

        virtual void xdg_shell_bind_resource(Resource *resource);
        virtual void xdg_shell_destroy_resource(Resource *resource);

        virtual void xdg_shell_destroy(Resource *resource);
        virtual void xdg_shell_use_unstable_version(Resource *resource, int32_t version);
        virtual void xdg_shell_get_xdg_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface);
        virtual void xdg_shell_get_xdg_popup(Resource *resource, uint32_t id, struct ::wl_resource *surface, struct ::wl_resource *parent, struct ::wl_resource *seat, uint32_t serial, int32_t x, int32_t y);
        virtual void xdg_shell_pong(Resource *resource, uint32_t serial);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);
        static void display_destroy_func(struct ::wl_listener *listener, void *data);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::xdg_shell_v5_interface m_xdg_shell_interface;

        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_use_unstable_version(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t version);
        static void handle_get_xdg_surface(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id,
            struct ::wl_resource *surface);
        static void handle_get_xdg_popup(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id,
            struct ::wl_resource *surface,
            struct ::wl_resource *parent,
            struct ::wl_resource *seat,
            uint32_t serial,
            int32_t x,
            int32_t y);
        static void handle_pong(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t serial);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
        struct DisplayDestroyedListener : ::wl_listener {
            xdg_shell_v5 *parent;
        };
        DisplayDestroyedListener m_displayDestroyedListener;
    };

    class Q_WAYLAND_SERVER_XDG_SHELL_UNSTABLE_V5_EXPORT xdg_surface_v5
    {
    public:
        xdg_surface_v5(struct ::wl_client *client, int id, int version);
        xdg_surface_v5(struct ::wl_display *display, int version);
        xdg_surface_v5(struct ::wl_resource *resource);
        xdg_surface_v5();

        virtual ~xdg_surface_v5();

        class Resource
        {
        public:
            Resource() : xdg_surface_object(nullptr), handle(nullptr) {}
            virtual ~Resource() {}

            xdg_surface_v5 *xdg_surface_object;
            struct ::wl_resource *handle;

            struct ::wl_client *client() const { return wl_resource_get_client(handle); }
            int version() const { return wl_resource_get_version(handle); }

            static Resource *fromResource(struct ::wl_resource *resource);
        };

        void init(struct ::wl_client *client, int id, int version);
        void init(struct ::wl_display *display, int version);
        void init(struct ::wl_resource *resource);

        Resource *add(struct ::wl_client *client, int version);
        Resource *add(struct ::wl_client *client, int id, int version);
        Resource *add(struct wl_list *resource_list, struct ::wl_client *client, int id, int version);

        Resource *resource() { return m_resource; }
        const Resource *resource() const { return m_resource; }

        QMultiMap<struct ::wl_client*, Resource*> resourceMap() { return m_resource_map; }
        const QMultiMap<struct ::wl_client*, Resource*> resourceMap() const { return m_resource_map; }

        bool isGlobal() const { return m_global != nullptr; }
        bool isResource() const { return m_resource != nullptr; }

        static const struct ::wl_interface *interface();
        static QByteArray interfaceName() { return interface()->name; }
        static int interfaceVersion() { return interface()->version; }


        enum resize_edge {
            resize_edge_none = 0,
            resize_edge_top = 1,
            resize_edge_bottom = 2,
            resize_edge_left = 4,
            resize_edge_top_left = 5,
            resize_edge_bottom_left = 6,
            resize_edge_right = 8,
            resize_edge_top_right = 9,
            resize_edge_bottom_right = 10
        };

        enum state {
            state_maximized = 1, // the surface is maximized
            state_fullscreen = 2, // the surface is fullscreen
            state_resizing = 3,
            state_activated = 4
        };

        void send_configure(int32_t width, int32_t height, const QByteArray &states, uint32_t serial);
        void send_configure(struct ::wl_resource *resource, int32_t width, int32_t height, const QByteArray &states, uint32_t serial);
        void send_close();
        void send_close(struct ::wl_resource *resource);

    protected:
        virtual Resource *xdg_surface_allocate();

        virtual void xdg_surface_bind_resource(Resource *resource);
        virtual void xdg_surface_destroy_resource(Resource *resource);

        virtual void xdg_surface_destroy(Resource *resource);
        virtual void xdg_surface_set_parent(Resource *resource, struct ::wl_resource *parent);
        virtual void xdg_surface_set_title(Resource *resource, const QString &title);
        virtual void xdg_surface_set_app_id(Resource *resource, const QString &app_id);
        virtual void xdg_surface_show_window_menu(Resource *resource, struct ::wl_resource *seat, uint32_t serial, int32_t x, int32_t y);
        virtual void xdg_surface_move(Resource *resource, struct ::wl_resource *seat, uint32_t serial);
        virtual void xdg_surface_resize(Resource *resource, struct ::wl_resource *seat, uint32_t serial, uint32_t edges);
        virtual void xdg_surface_ack_configure(Resource *resource, uint32_t serial);
        virtual void xdg_surface_set_window_geometry(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height);
        virtual void xdg_surface_set_maximized(Resource *resource);
        virtual void xdg_surface_unset_maximized(Resource *resource);
        virtual void xdg_surface_set_fullscreen(Resource *resource, struct ::wl_resource *output);
        virtual void xdg_surface_unset_fullscreen(Resource *resource);
        virtual void xdg_surface_set_minimized(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);
        static void display_destroy_func(struct ::wl_listener *listener, void *data);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::xdg_surface_v5_interface m_xdg_surface_interface;

        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_set_parent(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *parent);
        static void handle_set_title(
            ::wl_client *client,
            struct wl_resource *resource,
            const char *title);
        static void handle_set_app_id(
            ::wl_client *client,
            struct wl_resource *resource,
            const char *app_id);
        static void handle_show_window_menu(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *seat,
            uint32_t serial,
            int32_t x,
            int32_t y);
        static void handle_move(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *seat,
            uint32_t serial);
        static void handle_resize(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *seat,
            uint32_t serial,
            uint32_t edges);
        static void handle_ack_configure(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t serial);
        static void handle_set_window_geometry(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t x,
            int32_t y,
            int32_t width,
            int32_t height);
        static void handle_set_maximized(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_unset_maximized(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_set_fullscreen(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *output);
        static void handle_unset_fullscreen(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_set_minimized(
            ::wl_client *client,
            struct wl_resource *resource);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
        struct DisplayDestroyedListener : ::wl_listener {
            xdg_surface_v5 *parent;
        };
        DisplayDestroyedListener m_displayDestroyedListener;
    };

    class Q_WAYLAND_SERVER_XDG_SHELL_UNSTABLE_V5_EXPORT xdg_popup_v5
    {
    public:
        xdg_popup_v5(struct ::wl_client *client, int id, int version);
        xdg_popup_v5(struct ::wl_display *display, int version);
        xdg_popup_v5(struct ::wl_resource *resource);
        xdg_popup_v5();

        virtual ~xdg_popup_v5();

        class Resource
        {
        public:
            Resource() : xdg_popup_object(nullptr), handle(nullptr) {}
            virtual ~Resource() {}

            xdg_popup_v5 *xdg_popup_object;
            struct ::wl_resource *handle;

            struct ::wl_client *client() const { return wl_resource_get_client(handle); }
            int version() const { return wl_resource_get_version(handle); }

            static Resource *fromResource(struct ::wl_resource *resource);
        };

        void init(struct ::wl_client *client, int id, int version);
        void init(struct ::wl_display *display, int version);
        void init(struct ::wl_resource *resource);

        Resource *add(struct ::wl_client *client, int version);
        Resource *add(struct ::wl_client *client, int id, int version);
        Resource *add(struct wl_list *resource_list, struct ::wl_client *client, int id, int version);

        Resource *resource() { return m_resource; }
        const Resource *resource() const { return m_resource; }

        QMultiMap<struct ::wl_client*, Resource*> resourceMap() { return m_resource_map; }
        const QMultiMap<struct ::wl_client*, Resource*> resourceMap() const { return m_resource_map; }

        bool isGlobal() const { return m_global != nullptr; }
        bool isResource() const { return m_resource != nullptr; }

        static const struct ::wl_interface *interface();
        static QByteArray interfaceName() { return interface()->name; }
        static int interfaceVersion() { return interface()->version; }


        void send_popup_done();
        void send_popup_done(struct ::wl_resource *resource);

    protected:
        virtual Resource *xdg_popup_allocate();

        virtual void xdg_popup_bind_resource(Resource *resource);
        virtual void xdg_popup_destroy_resource(Resource *resource);

        virtual void xdg_popup_destroy(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);
        static void display_destroy_func(struct ::wl_listener *listener, void *data);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::xdg_popup_v5_interface m_xdg_popup_interface;

        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
        struct DisplayDestroyedListener : ::wl_listener {
            xdg_popup_v5 *parent;
        };
        DisplayDestroyedListener m_displayDestroyedListener;
    };
}

QT_WARNING_POP
QT_END_NAMESPACE

#endif
