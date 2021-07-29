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

#include "qwebglintegration.h"

#include <QtGui/qpa/qplatformintegrationplugin.h>

#include <cstring>

QT_BEGIN_NAMESPACE

class QWebGLIntegrationPlugin : public QPlatformIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformIntegrationFactoryInterface_iid FILE "webgl.json")
public:
    QPlatformIntegration *create(const QString&, const QStringList&) override;
};

QPlatformIntegration* QWebGLIntegrationPlugin::create(const QString& system,
                                                      const QStringList& paramList)
{
    quint16 port = 8080;
    if (!paramList.isEmpty()) {
        for (const QString &parameter : qAsConst(paramList)) {
            const QStringList parts = parameter.split('=');
            if (parts.first() == QStringLiteral("port")) {
                if (parts.size() != 2) {
                    qCCritical(lcWebGL, "Port parameter specified with no value");
                    return nullptr;
                }
                bool ok;
                port = parts.last().toUShort(&ok);
                if (!ok) {
                    qCCritical(lcWebGL, "Invalid port number");
                    return nullptr;
                }
            }
            if (parts.first() == QStringLiteral("noloadingscreen"))
                qputenv("QT_WEBGL_LOADINGSCREEN", "0");
        }
    }
    if (!system.compare(QLatin1String("webgl"), Qt::CaseInsensitive))
        return new QWebGLIntegration(port);

    return nullptr;
}

QT_END_NAMESPACE

#include "qwebglmain.moc"
