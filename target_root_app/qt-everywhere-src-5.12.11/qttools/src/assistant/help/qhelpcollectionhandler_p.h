/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#ifndef QHELPCOLLECTIONHANDLER_H
#define QHELPCOLLECTIONHANDLER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

#include <QtSql/QSqlQuery>

#include "qhelpdbreader_p.h"

QT_BEGIN_NAMESPACE

class QHelpCollectionHandler : public QObject
{
    Q_OBJECT

public:
    struct FileInfo
    {
        QString fileName;
        QString folderName;
        QString namespaceName;
    };
    typedef QList<FileInfo> FileInfoList;

    struct TimeStamp
    {
        int namespaceId = -1;
        int folderId = -1;
        QString fileName;
        int size = 0;
        QString timeStamp;
    };

    struct ContentsData
    {
        QString namespaceName;
        QString folderName;
        QList<QByteArray> contentsList;
    };

    explicit QHelpCollectionHandler(const QString &collectionFile,
        QObject *parent = nullptr);
    ~QHelpCollectionHandler();

    QString collectionFile() const;

    bool openCollectionFile();
    bool copyCollectionFile(const QString &fileName);

    QStringList customFilters() const;
    bool removeCustomFilter(const QString &filterName);
    bool addCustomFilter(const QString &filterName,
        const QStringList &attributes);

    FileInfo registeredDocumentation(const QString &namespaceName) const;
    FileInfoList registeredDocumentations() const;
    bool registerDocumentation(const QString &fileName);
    bool unregisterDocumentation(const QString &namespaceName);

    bool fileExists(const QUrl &url) const;
    QStringList files(const QString &namespaceName,
                      const QStringList &filterAttributes = QStringList(),
                      const QString &extensionFilter = QString()) const;
    QString namespaceForFile(const QUrl &url,
                             const QStringList &filterAttributes = QStringList()) const;
    QUrl findFile(const QUrl &url,
                  const QStringList &filterAttributes = QStringList()) const;
    QByteArray fileData(const QUrl &url) const;

    QStringList indicesForFilter(const QStringList &filterAttributes) const;
    QList<ContentsData> contentsForFilter(const QStringList &filterAttributes) const;

    bool removeCustomValue(const QString &key);
    QVariant customValue(const QString &key, const QVariant &defaultValue) const;
    bool setCustomValue(const QString &key, const QVariant &value);

    QStringList filterAttributes() const;
    QStringList filterAttributes(const QString &filterName) const;
    QList<QStringList> filterAttributeSets(const QString &namespaceName) const;

    int registerNamespace(const QString &nspace, const QString &fileName);
    int registerVirtualFolder(const QString &folderName, int namespaceId);
    bool registerVersion(const QString &version, int namespaceId);

    QMap<QString, QUrl> linksForIdentifier(const QString &id,
                                           const QStringList &filterAttributes) const;
    QMap<QString, QUrl> linksForKeyword(const QString &keyword,
                                        const QStringList &filterAttributes) const;

    void setReadOnly(bool readOnly);

signals:
    void error(const QString &msg) const;

private:
    QString namespaceVersion(const QString &namespaceName) const;
    QMap<QString, QUrl> linksForField(const QString &fieldName,
                                      const QString &fieldValue,
                                      const QStringList &filterAttributes) const;
    bool isDBOpened() const;
    bool createTables(QSqlQuery *query);
    void closeDB();
    bool recreateIndexAndNamespaceFilterTables(QSqlQuery *query);
    bool registerIndexAndNamespaceFilterTables(const QString &nameSpace);
    bool registerFilterAttributes(const QList<QStringList> &attributeSets, int nsId);
    bool registerFileAttributeSets(const QList<QStringList> &attributeSets, int nsId);
    bool registerIndexTable(const QHelpDBReader::IndexTable &indexTable,
                            int nsId, int vfId, const QString &fileName);
    bool unregisterIndexTable(int nsId, int vfId);
    QString absoluteDocPath(const QString &fileName) const;
    bool isTimeStampCorrect(const TimeStamp &timeStamp) const;
    bool hasTimeStampInfo(const QString &nameSpace) const;
    void scheduleVacuum();
    void execVacuum();

    QString m_collectionFile;
    QString m_connectionName;
    QSqlQuery *m_query = nullptr;
    bool m_vacuumScheduled = false;
    bool m_readOnly = false;
};

QT_END_NAMESPACE

#endif  //QHELPCOLLECTIONHANDLER_H
