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

#ifndef QWEBGLINTEGRATION_H
#define QWEBGLINTEGRATION_H

#include <QtCore/qloggingcategory.h>
#include <QtCore/qscopedpointer.h>
#include <QtGui/qpa/qplatformintegration.h>
#include <QtGui/qpa/qplatformnativeinterface.h>

QT_BEGIN_NAMESPACE

class QPlatformSurface;
class QWebGLIntegrationPrivate;

Q_DECLARE_LOGGING_CATEGORY(lcWebGL)

class QWebGLIntegration : public QPlatformIntegration, public QPlatformNativeInterface
{
public:
    QWebGLIntegration(quint16 port);
    ~QWebGLIntegration();

    static QWebGLIntegration *instance();

    void initialize() override;
    void destroy() override;

    QAbstractEventDispatcher *createEventDispatcher() const override;
    QPlatformFontDatabase *fontDatabase() const override;
    QPlatformServices *services() const override;
    QPlatformInputContext *inputContext() const override;
    QPlatformTheme *createPlatformTheme(const QString &name) const override;
    QPlatformWindow *createPlatformWindow(QWindow *window) const override;
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const override;
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const override;

    bool hasCapability(QPlatformIntegration::Capability cap) const override;

    QPlatformNativeInterface *nativeInterface() const override;

    void openUrl(const QUrl &url);

private:
    Q_DISABLE_COPY(QWebGLIntegration)
    Q_DECLARE_PRIVATE(QWebGLIntegration)
    QScopedPointer<QWebGLIntegrationPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBGLINTEGRATION_H
