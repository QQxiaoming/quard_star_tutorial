/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
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
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtScript/qscriptextensionplugin.h>
#include <QtScript/qscriptengine.h>
#include <qdebug.h>

class SimplePlugin : public QScriptExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.org.qt-project.Qt.QScriptExtensionInterface" FILE "simpleplugin.json")
    Q_INTERFACES(QScriptExtensionInterface)

public:
    SimplePlugin(QObject *parent = 0);
    ~SimplePlugin();

    virtual QStringList keys() const;
    virtual void initialize(const QString &key, QScriptEngine *engine);
};

SimplePlugin::SimplePlugin(QObject *parent)
    : QScriptExtensionPlugin(parent)
{
}

SimplePlugin::~SimplePlugin()
{
}

QStringList SimplePlugin::keys() const
{
    return QStringList() << "simple"
                         << "simple.foo"
                         << "simple.foo.bar";
}

void SimplePlugin::initialize(const QString &key, QScriptEngine *engine)
{
    engine->globalObject().setProperty("pluginKey", key);
    QScriptValue package = setupPackage(key, engine);
    engine->globalObject().setProperty("package", package);
}

#include "simpleplugin.moc"
