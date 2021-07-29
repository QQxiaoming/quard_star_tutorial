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

#ifndef QWEBGLWINDOW_H
#define QWEBGLWINDOW_H

#include "qwebglintegration.h"
#include "qwebglscreen.h"

#include <QtCore/qscopedpointer.h>
#include <QtGui/qpa/qplatformwindow.h>

QT_BEGIN_NAMESPACE

class QOpenGLCompositorBackingStore;

class QWebGLWindowPrivate;

class QWebGLWindow : public QPlatformWindow
{
    friend class QWebGLIntegrationPrivate;
    friend class QWebGLContext;
    friend class QWebGLWebSocketServer;

public:
    QWebGLWindow(QWindow *w);
    ~QWebGLWindow() override;

    void create();
    void destroy();

    void raise() override;

    WId winId() const override;

    QSurfaceFormat format() const override;

    QWebGLScreen *screen() const override;

    void setGeometry(const QRect &rect) override;
    void setDefaults(const QMap<GLenum, QVariant> &values);

private:
    Q_DISABLE_COPY(QWebGLWindow)
    Q_DECLARE_PRIVATE(QWebGLWindow)
    QScopedPointer<QWebGLWindowPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBGLWINDOW_H
