/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QML preview debug service.
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

#ifndef QQMLPREVIEWFILEENGINE_H
#define QQMLPREVIEWFILEENGINE_H

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

#include "qqmlpreviewfileloader.h"

#include <private/qabstractfileengine_p.h>
#include <private/qfsfileengine_p.h>
#include <QtCore/qbuffer.h>

QT_BEGIN_NAMESPACE

class QQmlPreviewFileEngine : public QAbstractFileEngine
{
public:
    QQmlPreviewFileEngine(const QString &file, const QString &absolute,
                          QQmlPreviewFileLoader *loader);

    void setFileName(const QString &file) override;

    bool open(QIODevice::OpenMode flags) override ;
    bool close() override;
    qint64 size() const override;
    qint64 pos() const override;
    bool seek(qint64) override;
    qint64 read(char *data, qint64 maxlen) override;

    FileFlags fileFlags(FileFlags type) const override;
    QString fileName(QAbstractFileEngine::FileName file) const override;
    uint ownerId(FileOwner) const override;

    Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames) override;
    Iterator *endEntryList() override;

    // Forwarding to fallback if exists
    bool flush() override;
    bool syncToDisk() override;
    bool isSequential() const override;
    bool remove() override;
    bool copy(const QString &newName) override;
    bool rename(const QString &newName) override;
    bool renameOverwrite(const QString &newName) override;
    bool link(const QString &newName) override;
    bool mkdir(const QString &dirName, bool createParentDirectories) const override;
    bool rmdir(const QString &dirName, bool recurseParentDirectories) const override;
    bool setSize(qint64 size) override;
    bool caseSensitive() const override;
    bool isRelativePath() const override;
    QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const override;
    bool setPermissions(uint perms) override;
    QByteArray id() const override;
    QString owner(FileOwner) const override;
    QDateTime fileTime(FileTime time) const override;
    int handle() const override;
    qint64 readLine(char *data, qint64 maxlen) override;
    qint64 write(const char *data, qint64 len) override;
    bool extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output) override;
    bool supportsExtension(Extension extension) const override;

private:
    void load() const;

    QString m_name;
    QString m_absolute;
    QPointer<QQmlPreviewFileLoader> m_loader;

    mutable QBuffer m_contents;
    mutable QStringList m_entries;
    mutable QScopedPointer<QAbstractFileEngine> m_fallback;
    mutable QQmlPreviewFileLoader::Result m_result = QQmlPreviewFileLoader::Unknown;
};

class QQmlPreviewFileEngineHandler : public QAbstractFileEngineHandler
{
public:
    QQmlPreviewFileEngineHandler(QQmlPreviewFileLoader *loader);
    QAbstractFileEngine *create(const QString &fileName) const override;

private:
    QPointer<QQmlPreviewFileLoader> m_loader;
};



QT_END_NAMESPACE

#endif // QQMLPREVIEWFILEENGINE_H
