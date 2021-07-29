/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt WebGL module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebglwindow.h"
#include "qwebglwindow_p.h"

#include "qwebglintegration_p.h"
#include "qwebglwebsocketserver.h"

#include <QtCore/qtextstream.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/private/qopenglcontext_p.h>
#include <QtGui/private/qwindow_p.h>
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QtGui/qpa/qplatformintegration.h>
#include <QtGui/qopenglcontext.h>

#include "qwebglwindow.h"

QT_BEGIN_NAMESPACE

static Q_LOGGING_CATEGORY(lc, "qt.qpa.webgl.window")

QAtomicInt QWebGLWindowPrivate::nextId(1);

QWebGLWindowPrivate::QWebGLWindowPrivate(QWebGLWindow *p) :
    q_ptr(p)
{}

QWebGLWindow::QWebGLWindow(QWindow *w) :
    QPlatformWindow(w),
    d_ptr(new QWebGLWindowPrivate(this))
{
    Q_D(QWebGLWindow);
    d->raster = false;
    d->flags = 0;
}

QWebGLWindow::~QWebGLWindow()
{
    destroy();
}

void QWebGLWindow::create()
{
    Q_D(QWebGLWindow);
    if (d->flags.testFlag(QWebGLWindowPrivate::Created))
        return;

    d->id = QWebGLWindowPrivate::nextId.fetchAndAddAcquire(1);
    qCDebug(lc, "Window %d created", d->id);

    // Save the original surface type before changing to OpenGLSurface.
    d->raster = (window()->surfaceType() == QSurface::RasterSurface);
    if (d->raster) // change to OpenGL, but not for RasterGLSurface
        window()->setSurfaceType(QSurface::OpenGLSurface);

    if (window()->windowState() == Qt::WindowFullScreen) {
        QRect fullscreenRect(QPoint(), screen()->availableGeometry().size());
        QPlatformWindow::setGeometry(fullscreenRect);
        QWindowSystemInterface::handleGeometryChange(window(), fullscreenRect);
        return;
    }

    d->flags = QWebGLWindowPrivate::Created;

    if (window()->type() == Qt::Desktop)
        return;

    d->flags |= QWebGLWindowPrivate::HasNativeWindow;
    setGeometry(window()->geometry()); // will become fullscreen
    QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(0, 0), geometry().size()));

    if (d->raster) {
        QOpenGLContext *context = new QOpenGLContext(QGuiApplication::instance());
        context->setShareContext(qt_gl_global_share_context());
        context->setFormat(d->format);
        context->setScreen(window()->screen());
        if (Q_UNLIKELY(!context->create()))
            qFatal("QWebGL: Failed to create compositing context");
    }
}

void QWebGLWindow::destroy()
{
    Q_D(QWebGLWindow);
    qCDebug(lc, "Destroying %d", d->id);
    if (d->flags.testFlag(QWebGLWindowPrivate::HasNativeWindow)) {
        invalidateSurface();
    }

    qt_window_private(window())->updateRequestPending = false;

    d->flags = 0;

    auto integrationPrivate = QWebGLIntegrationPrivate::instance();
    auto clientData = integrationPrivate->findClientData(surface()->surfaceHandle());
    if (clientData) {
        const QVariantMap values {
            { "winId", winId() }
        };
        if (clientData->socket)
            integrationPrivate->sendMessage(clientData->socket,
                                            QWebGLWebSocketServer::MessageType::DestroyCanvas,
                                            values);
        clientData->platformWindows.removeAll(this);
    }
}

void QWebGLWindow::raise()
{
    QWindow *wnd = window();
    if (wnd->type() != Qt::Desktop) {
        QWindowSystemInterface::handleExposeEvent(wnd, QRect(QPoint(0, 0), wnd->geometry().size()));
    }
}

QSurfaceFormat QWebGLWindow::format() const
{
    Q_D(const QWebGLWindow);
    return d->format;
}

QWebGLScreen *QWebGLWindow::screen() const
{
    return static_cast<QWebGLScreen *>(QPlatformWindow::screen());
}

void QWebGLWindow::setGeometry(const QRect &rect)
{
    QWindowSystemInterface::handleGeometryChange(window(), rect);
    QPlatformWindow::setGeometry(rect);
}

void QWebGLWindow::setDefaults(const QMap<GLenum, QVariant> &values)
{
    Q_D(QWebGLWindow);
    d->defaults.set_value(values);
}

WId QWebGLWindow::winId() const
{
    Q_D(const QWebGLWindow);
    return d->id;
}

QT_END_NAMESPACE
