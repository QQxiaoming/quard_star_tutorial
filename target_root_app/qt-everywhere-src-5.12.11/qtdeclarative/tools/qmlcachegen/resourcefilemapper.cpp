/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#include "resourcefilemapper.h"

#include <QFileInfo>
#include <QDir>
#include <QXmlStreamReader>

ResourceFileMapper::ResourceFileMapper(const QStringList &resourceFiles)
{
    for (const QString &fileName: resourceFiles) {
        QFile f(fileName);
        if (!f.open(QIODevice::ReadOnly))
            continue;
        populateFromQrcFile(f);
    }
}

bool ResourceFileMapper::isEmpty() const
{
    return qrcPathToFileSystemPath.isEmpty();
}

QStringList ResourceFileMapper::resourcePaths(const QString &fileName)
{
    const QString absPath = QDir::cleanPath(QDir::current().absoluteFilePath(fileName));
    QHashIterator<QString, QString> it(qrcPathToFileSystemPath);
    QStringList resourcePaths;
    while (it.hasNext()) {
        it.next();
        if (QFileInfo(it.value()) == QFileInfo(absPath))
            resourcePaths.append(it.key());
    }
    return resourcePaths;
}

QStringList ResourceFileMapper::qmlCompilerFiles() const
{
    QStringList files;
    for (auto it = qrcPathToFileSystemPath.constBegin(), end = qrcPathToFileSystemPath.constEnd();
         it != end; ++it) {
        const QString &qrcPath = it.key();
        const QString suffix = QFileInfo(qrcPath).suffix();
        if (suffix != QStringLiteral("qml") && suffix != QStringLiteral("js") && suffix != QStringLiteral("mjs"))
            continue;
        files << qrcPath;
    }
    return files;
}

void ResourceFileMapper::populateFromQrcFile(QFile &file)
{
    enum State {
        InitialState,
        InRCC,
        InResource,
        InFile
    };
    State state = InitialState;

    QDir qrcDir = QFileInfo(file).absoluteDir();

    QString prefix;
    QString currentFileName;
    QXmlStreamAttributes currentFileAttributes;

    QXmlStreamReader reader(&file);
    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::StartElement:
            if (reader.name() == QStringLiteral("RCC")) {
                if (state != InitialState)
                    return;
                state = InRCC;
                continue;
            } else if (reader.name() == QStringLiteral("qresource")) {
                if (state != InRCC)
                    return;
                state = InResource;
                QXmlStreamAttributes attributes = reader.attributes();
                if (attributes.hasAttribute(QStringLiteral("prefix")))
                    prefix = attributes.value(QStringLiteral("prefix")).toString();
                if (!prefix.startsWith(QLatin1Char('/')))
                    prefix.prepend(QLatin1Char('/'));
                if (!prefix.endsWith(QLatin1Char('/')))
                    prefix.append(QLatin1Char('/'));
                continue;
            } else if (reader.name() == QStringLiteral("file")) {
                if (state != InResource)
                    return;
                state = InFile;
                currentFileAttributes = reader.attributes();
                continue;
            }
            return;

        case QXmlStreamReader::EndElement:
            if (reader.name() == QStringLiteral("file")) {
                if (state != InFile)
                    return;
                state = InResource;
                continue;
            } else if (reader.name() == QStringLiteral("qresource")) {
                if (state != InResource)
                    return;
                state = InRCC;
                continue;
            } else if (reader.name() == QStringLiteral("RCC")) {
                if (state != InRCC)
                    return;
                state = InitialState;
                continue;
            }
            return;

        case QXmlStreamReader::Characters: {
            if (reader.isWhitespace())
                break;
            if (state != InFile)
                return;
            currentFileName = reader.text().toString();
            if (currentFileName.isEmpty())
                continue;

            const QString fsPath = QDir::cleanPath(qrcDir.absoluteFilePath(currentFileName));

            if (currentFileAttributes.hasAttribute(QStringLiteral("alias")))
                currentFileName = currentFileAttributes.value(QStringLiteral("alias")).toString();

            currentFileName = QDir::cleanPath(currentFileName);
            while (currentFileName.startsWith(QLatin1String("../")))
                currentFileName.remove(0, 3);

            const QString qrcPath = prefix + currentFileName;
            if (QFile::exists(fsPath))
                qrcPathToFileSystemPath.insert(qrcPath, fsPath);
            continue;
        }

        default: break;
    }
    }
}
