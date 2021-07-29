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
#ifndef QT_WAYLAND_XDG_SHELL_UNSTABLE_V5
#define QT_WAYLAND_XDG_SHELL_UNSTABLE_V5

#include "wayland-xdg-shell-unstable-v5-client-protocol_p.h"
#include <QByteArray>
#include <QString>

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")

namespace QtWayland {
    class  xdg_shell_v5
    {
    public:
        xdg_shell_v5(struct ::wl_registry *registry, int id, int version);
        xdg_shell_v5(struct ::xdg_shell_v5 *object);
        xdg_shell_v5();

        virtual ~xdg_shell_v5();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::xdg_shell_v5 *object);

        struct ::xdg_shell_v5 *object() { return m_xdg_shell; }
        const struct ::xdg_shell_v5 *object() const { return m_xdg_shell; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        enum version {
            version_current = 5 // Always the latest version
        };

        enum error {
            error_role = 0, // given wl_surface has another role
            error_defunct_surfaces = 1, // xdg_shell was destroyed before children
            error_not_the_topmost_popup = 2, // the client tried to map or destroy a non-topmost popup
            error_invalid_popup_parent = 3 // the client specified an invalid popup parent surface
        };

        void destroy();
        void use_unstable_version(int32_t version);
        struct ::xdg_surface_v5 *get_xdg_surface(struct ::wl_surface *surface);
        struct ::xdg_popup_v5 *get_xdg_popup(struct ::wl_surface *surface, struct ::wl_surface *parent, struct ::wl_seat *seat, uint32_t serial, int32_t x, int32_t y);
        void pong(uint32_t serial);

    protected:
        virtual void xdg_shell_ping(uint32_t serial);

    private:
        void init_listener();
        static const struct xdg_shell_listener m_xdg_shell_listener;
        static void handle_ping(
            void *data,
            struct ::xdg_shell_v5 *object,
            uint32_t serial);
        struct ::xdg_shell_v5 *m_xdg_shell;
    };

    class  xdg_surface_v5
    {
    public:
        xdg_surface_v5(struct ::wl_registry *registry, int id, int version);
        xdg_surface_v5(struct ::xdg_surface_v5 *object);
        xdg_surface_v5();

        virtual ~xdg_surface_v5();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::xdg_surface_v5 *object);

        struct ::xdg_surface_v5 *object() { return m_xdg_surface; }
        const struct ::xdg_surface_v5 *object() const { return m_xdg_surface; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

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

        void destroy();
        void set_parent(struct ::xdg_surface_v5 *parent);
        void set_title(const QString &title);
        void set_app_id(const QString &app_id);
        void show_window_menu(struct ::wl_seat *seat, uint32_t serial, int32_t x, int32_t y);
        void move(struct ::wl_seat *seat, uint32_t serial);
        void resize(struct ::wl_seat *seat, uint32_t serial, uint32_t edges);
        void ack_configure(uint32_t serial);
        void set_window_geometry(int32_t x, int32_t y, int32_t width, int32_t height);
        void set_maximized();
        void unset_maximized();
        void set_fullscreen(struct ::wl_output *output);
        void unset_fullscreen();
        void set_minimized();

    protected:
        virtual void xdg_surface_configure(int32_t width, int32_t height, wl_array *states, uint32_t serial);
        virtual void xdg_surface_close();

    private:
        void init_listener();
        static const struct xdg_surface_listener m_xdg_surface_listener;
        static void handle_configure(
            void *data,
            struct ::xdg_surface_v5 *object,
            int32_t width,
            int32_t height,
            wl_array *states,
            uint32_t serial);
        static void handle_close(
            void *data,
            struct ::xdg_surface_v5 *object);
        struct ::xdg_surface_v5 *m_xdg_surface;
    };

    class  xdg_popup_v5
    {
    public:
        xdg_popup_v5(struct ::wl_registry *registry, int id, int version);
        xdg_popup_v5(struct ::xdg_popup_v5 *object);
        xdg_popup_v5();

        virtual ~xdg_popup_v5();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::xdg_popup_v5 *object);

        struct ::xdg_popup_v5 *object() { return m_xdg_popup; }
        const struct ::xdg_popup_v5 *object() const { return m_xdg_popup; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        void destroy();

    protected:
        virtual void xdg_popup_popup_done();

    private:
        void init_listener();
        static const struct xdg_popup_listener m_xdg_popup_listener;
        static void handle_popup_done(
            void *data,
            struct ::xdg_popup_v5 *object);
        struct ::xdg_popup_v5 *m_xdg_popup;
    };
}

QT_WARNING_POP
QT_END_NAMESPACE

#endif
