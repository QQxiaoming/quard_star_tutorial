/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtQuickControls2/private/qquickstyleplugin_p.h>
#include <QtCore/qloggingcategory.h>
#include <QtQml/qqml.h>

#include "qquickimageselector_p.h"
#include "qquickimaginestyle_p.h"
#include "qquickimaginetheme_p.h"
#include "qquickninepatchimage_p.h"

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQtQuickControlsImagine, "qt.quick.controls.imagine")

class QtQuickControls2ImagineStylePlugin: public QQuickStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QtQuickControls2ImagineStylePlugin(QObject *parent = nullptr);

    void registerTypes(const char *uri) override;

    QString name() const override;
    void initializeTheme(QQuickTheme *theme) override;
};

QtQuickControls2ImagineStylePlugin::QtQuickControls2ImagineStylePlugin(QObject *parent) : QQuickStylePlugin(parent)
{
}

void QtQuickControls2ImagineStylePlugin::registerTypes(const char *uri)
{
    qmlRegisterModule(uri, 2, QT_VERSION_MINOR); // Qt 5.12->2.12, 5.13->2.13...
    qmlRegisterUncreatableType<QQuickImagineStyle>(uri, 2, 3, "Imagine", tr("Imagine is an attached property"));

    QByteArray import = QByteArray(uri) + ".impl";
    qmlRegisterModule(import, 2, QT_VERSION_MINOR); // Qt 5.12->2.12, 5.13->2.13...

    qmlRegisterType<QQuickAnimatedImageSelector>(import, 2, 3, "AnimatedImageSelector");
    qmlRegisterType<QQuickImageSelector>(import, 2, 3, "ImageSelector");
    qmlRegisterType<QQuickNinePatchImage>(import, 2, 3, "NinePatchImage");
    qmlRegisterType<QQuickNinePatchImageSelector>(import, 2, 3, "NinePatchImageSelector");
}

QString QtQuickControls2ImagineStylePlugin::name() const
{
    return QStringLiteral("Imagine");
}

void QtQuickControls2ImagineStylePlugin::initializeTheme(QQuickTheme *theme)
{
    QQuickImagineTheme::initialize(theme);
}

QT_END_NAMESPACE

#include "qtquickcontrols2imaginestyleplugin.moc"
