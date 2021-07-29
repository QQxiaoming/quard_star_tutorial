/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Virtual Keyboard module of the Qt Toolkit.
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

#include "extensionloader.h"
#include <QtVirtualKeyboard/QVirtualKeyboardExtensionPlugin>
#include <QtCore/private/qfactoryloader_p.h>

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

QMutex ExtensionLoader::m_mutex;
QHash<QString, QJsonObject> ExtensionLoader::m_plugins;
bool ExtensionLoader::m_alreadyDiscovered = false;

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
        (QVirtualKeyboardExtensionPluginFactoryInterface_iid,
         QLatin1String("/virtualkeyboard")))

QHash<QString, QJsonObject> ExtensionLoader::plugins(bool reload)
{
    QMutexLocker lock(&m_mutex);

    if (reload == true)
        m_alreadyDiscovered = false;

    if (!m_alreadyDiscovered) {
        loadPluginMetadata();
        m_alreadyDiscovered = true;
    }
    return m_plugins;
}

QJsonObject ExtensionLoader::loadMeta(const QString &extensionName)
{
    QJsonObject metaData;
    metaData = QJsonObject();
    metaData.insert(QLatin1String("index"), -1);

    QList<QJsonObject> candidates = ExtensionLoader::plugins().values(extensionName);

    int versionFound = -1;
    int idx = -1;

    // figure out which version of the plugin we want
    for (int i = 0; i < candidates.size(); ++i) {
        QJsonObject meta = candidates[i];
        if (meta.contains(QLatin1String("Version"))
                && meta.value(QLatin1String("Version")).isDouble()) {
            int ver = int(meta.value(QLatin1String("Version")).toDouble());
            if (ver > versionFound) {
                versionFound = ver;
                idx = i;
            }
        }
    }

    if (idx != -1) {
        metaData = candidates[idx];
        return metaData;
    }
    return QJsonObject();
}

QVirtualKeyboardExtensionPlugin *ExtensionLoader::loadPlugin(QJsonObject metaData)
{
    if (int(metaData.value(QLatin1String("index")).toDouble()) < 0) {
        return NULL;
    }
    int idx = int(metaData.value(QLatin1String("index")).toDouble());
    return qobject_cast<QVirtualKeyboardExtensionPlugin *>(loader()->instance(idx));
}

void ExtensionLoader::loadPluginMetadata()
{
    QFactoryLoader *l = loader();
    QList<QJsonObject> meta = l->metaData();
    for (int i = 0; i < meta.size(); ++i) {
        QJsonObject obj = meta.at(i).value(QLatin1String("MetaData")).toObject();
        QString name = obj.value(QLatin1String("Name")).toString();
        if (!name.isEmpty()) {
            obj.insert(QLatin1String("index"), i);
            m_plugins.insertMulti(name, obj);
        }
    }
}

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE
