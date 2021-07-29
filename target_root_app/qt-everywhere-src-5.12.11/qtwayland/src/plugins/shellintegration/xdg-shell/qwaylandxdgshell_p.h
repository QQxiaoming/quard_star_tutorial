/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2017 Eurogiciel, author: <philippe.coval@eurogiciel.fr>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDXDGSHELL_H
#define QWAYLANDXDGSHELL_H

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

#include "qwayland-xdg-shell.h"

#include "qwaylandxdgdecorationv1_p.h"

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <QtWaylandClient/private/qwaylandshellsurface_p.h>

#include <QtCore/QSize>
#include <QtGui/QRegion>

#include <wayland-client.h>

QT_BEGIN_NAMESPACE

class QWindow;

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandWindow;
class QWaylandInputDevice;
class QWaylandXdgShell;

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgSurface : public QWaylandShellSurface, public QtWayland::xdg_surface
{
    Q_OBJECT
public:
    QWaylandXdgSurface(QWaylandXdgShell *shell, ::xdg_surface *surface, QWaylandWindow *window);
    ~QWaylandXdgSurface() override;

    void resize(QWaylandInputDevice *inputDevice, enum xdg_toplevel_resize_edge edges);
    void resize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize edges) override;
    bool move(QWaylandInputDevice *inputDevice) override;
    void setTitle(const QString &title) override;
    void setAppId(const QString &appId) override;
    void setWindowFlags(Qt::WindowFlags flags) override;

    bool isExposed() const override;
    bool handleExpose(const QRegion &) override;
    bool handlesActiveState() const { return m_toplevel; }
    void applyConfigure() override;
    bool wantsDecorations() const override;

protected:
    void requestWindowStates(Qt::WindowStates states) override;
    void xdg_surface_configure(uint32_t serial) override;

private:
    class Toplevel: public QtWayland::xdg_toplevel
    {
    public:
        Toplevel(QWaylandXdgSurface *xdgSurface);
        ~Toplevel() override;

        void applyConfigure();
        bool wantsDecorations();

        void xdg_toplevel_configure(int32_t width, int32_t height, wl_array *states) override;
        void xdg_toplevel_close() override;

        void requestWindowFlags(Qt::WindowFlags flags);
        void requestWindowStates(Qt::WindowStates states);
        struct {
            QSize size = {0, 0};
            Qt::WindowStates states = Qt::WindowNoState;
        }  m_pending, m_applied;
        QSize m_normalSize;

        QWaylandXdgSurface *m_xdgSurface = nullptr;
        QWaylandXdgToplevelDecorationV1 *m_decoration = nullptr;
    };

    class Popup : public QtWayland::xdg_popup {
    public:
        Popup(QWaylandXdgSurface *xdgSurface, QWaylandXdgSurface *parent, QtWayland::xdg_positioner *positioner);
        ~Popup() override;

        void grab(QWaylandInputDevice *seat, uint serial);
        void xdg_popup_popup_done() override;

        QWaylandXdgSurface *m_xdgSurface = nullptr;
        QWaylandXdgSurface *m_parent = nullptr;
        bool m_grabbing = false;
    };

    void setToplevel();
    void setPopup(QWaylandWindow *parent);
    void setGrabPopup(QWaylandWindow *parent, QWaylandInputDevice *device, int serial);

    QWaylandXdgShell *m_shell = nullptr;
    QWaylandWindow *m_window = nullptr;
    Toplevel *m_toplevel = nullptr;
    Popup *m_popup = nullptr;
    bool m_configured = false;
    QRegion m_exposeRegion;
    uint m_pendingConfigureSerial = 0;

    friend class QWaylandXdgShell;
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgShell : public QtWayland::xdg_wm_base
{
public:
    QWaylandXdgShell(QWaylandDisplay *display, uint32_t id, uint32_t availableVersion);
    ~QWaylandXdgShell() override;

    QWaylandXdgDecorationManagerV1 *decorationManager() { return m_xdgDecorationManager.data(); }
    QWaylandXdgSurface *getXdgSurface(QWaylandWindow *window);

protected:
    void xdg_wm_base_ping(uint32_t serial) override;

private:
    static void handleRegistryGlobal(void *data, ::wl_registry *registry, uint id,
                                     const QString &interface, uint version);

    QWaylandDisplay *m_display = nullptr;
    QScopedPointer<QWaylandXdgDecorationManagerV1> m_xdgDecorationManager;
    QWaylandXdgSurface::Popup *m_topmostGrabbingPopup = nullptr;

    friend class QWaylandXdgSurface;
};

QT_END_NAMESPACE

}

#endif // QWAYLANDXDGSHELL_H
