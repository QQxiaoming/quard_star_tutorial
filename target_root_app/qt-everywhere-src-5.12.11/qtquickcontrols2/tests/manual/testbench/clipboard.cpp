/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "clipboard.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QSettings>

const QStringList keys = {
    "Palette/window",
    "Palette/windowText",
    "Palette/base",
    "Palette/text",
    "Palette/button",
    "Palette/buttonText",
    "Palette/brightText",
    "Palette/toolTipBase",
    "Palette/toolTipText",
    "Palette/light",
    "Palette/midlight",
    "Palette/dark",
    "Palette/mid",
    "Palette/shadow",
    "Palette/highlight",
    "Palette/highlightedText",
    "Palette/link"
};

Clipboard::Clipboard(QObject *parent) :
    QObject(parent)
{
}

// Converts the JS map into a big string and copies it to the clipboard.
void Clipboard::copy(const QJSValue &keyValueMap)
{
    QString paletteSettingsString;
    QVariantMap map = keyValueMap.toVariant().value<QVariantMap>();
    const QList<QString> mapKeys = map.keys();
    for (const QString &key : mapKeys) {
        paletteSettingsString += "Palette/" + key + "=" + map.value(key).toString() + ",";
    }

    // Remove the trailing comma.
    if (!paletteSettingsString.isEmpty())
        paletteSettingsString.chop(1);

    QGuiApplication::clipboard()->setText(paletteSettingsString);
}

// Converts the big string into a JS map and returns it.
QVariant Clipboard::paste() const
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard->text().isEmpty())
        return QVariant();

    QVariantMap keyValueMap;

    const QStringList settingsList = clipboard->text().split(QLatin1Char(','));
    for (const QString &setting : settingsList) {
        const QStringList keyValuePair = setting.split(QLatin1Char('='));
        if (keyValuePair.size() < 2)
            continue;

        QString key = keyValuePair.first();
        if (keys.contains(key)) {
            key.remove(QLatin1String("Palette/"));
            const QString value = keyValuePair.last();

            keyValueMap.insert(key, value);
        }
    }

    return QVariant(keyValueMap);
}
