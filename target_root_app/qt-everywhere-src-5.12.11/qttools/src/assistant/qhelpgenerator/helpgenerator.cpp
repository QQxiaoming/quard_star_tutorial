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

#include "helpgenerator.h"
#include "qhelpprojectdata_p.h"
#include <qhelp_global.h>

#include <QtCore/QtMath>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QSet>
#include <QtCore/QVariant>
#include <QtCore/QDateTime>
#include <QtCore/QTextCodec>
#include <QtCore/QDataStream>
#include <QtSql/QSqlQuery>

#include <stdio.h>

QT_BEGIN_NAMESPACE

class HelpGeneratorPrivate : public QObject
{
    Q_OBJECT

public:
    HelpGeneratorPrivate(QObject *parent = nullptr) : QObject(parent) {}

    bool generate(QHelpProjectData *helpData,
        const QString &outputFileName);
    bool checkLinks(const QHelpProjectData &helpData);
    QString error() const;

Q_SIGNALS:
    void statusChanged(const QString &msg);
    void progressChanged(double progress);
    void warning(const QString &msg);

private:
    struct FileNameTableData
    {
        QString name;
        int fileId;
        QString title;
    };

    void writeTree(QDataStream &s, QHelpDataContentItem *item, int depth);
    bool createTables();
    bool insertFileNotFoundFile();
    bool registerCustomFilter(const QString &filterName,
        const QStringList &filterAttribs, bool forceUpdate = false);
    bool registerVirtualFolder(const QString &folderName, const QString &ns);
    bool insertFilterAttributes(const QStringList &attributes);
    bool insertKeywords(const QList<QHelpDataIndexItem> &keywords,
        const QStringList &filterAttributes);
    bool insertFiles(const QStringList &files, const QString &rootPath,
        const QStringList &filterAttributes);
    bool insertContents(const QByteArray &ba,
        const QStringList &filterAttributes);
    bool insertMetaData(const QMap<QString, QVariant> &metaData);
    void cleanupDB();
    void setupProgress(QHelpProjectData *helpData);
    void addProgress(double step);

    QString m_error;
    QSqlQuery *m_query = nullptr;

    int m_namespaceId = -1;
    int m_virtualFolderId = -1;

    QMap<QString, int> m_fileMap;
    QMap<int, QSet<int> > m_fileFilterMap;

    double m_progress;
    double m_oldProgress;
    double m_contentStep;
    double m_fileStep;
    double m_indexStep;
};

/*!
    Takes the \a helpData and generates a new documentation
    set from it. The Qt compressed help file is written to \a
    outputFileName. Returns true on success, otherwise false.
*/
bool HelpGeneratorPrivate::generate(QHelpProjectData *helpData,
                              const QString &outputFileName)
{
    emit progressChanged(0);
    m_error.clear();
    if (!helpData || helpData->namespaceName().isEmpty()) {
        m_error = tr("Invalid help data.");
        return false;
    }

    QString outFileName = outputFileName;
    if (outFileName.isEmpty()) {
        m_error = tr("No output file name specified.");
        return false;
    }

    QFileInfo fi(outFileName);
    if (fi.exists()) {
        if (!fi.dir().remove(fi.fileName())) {
            m_error = tr("The file %1 cannot be overwritten.").arg(outFileName);
            return false;
        }
    }

    setupProgress(helpData);

    emit statusChanged(tr("Building up file structure..."));
    bool openingOk = true;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), QLatin1String("builder"));
        db.setDatabaseName(outFileName);
        openingOk = db.open();
        if (openingOk)
            m_query = new QSqlQuery(db);
    }

    if (!openingOk) {
        m_error = tr("Cannot open data base file %1.").arg(outFileName);
        cleanupDB();
        return false;
    }

    m_query->exec(QLatin1String("PRAGMA synchronous=OFF"));
    m_query->exec(QLatin1String("PRAGMA cache_size=3000"));

    addProgress(1.0);
    createTables();
    insertFileNotFoundFile();
    insertMetaData(helpData->metaData());

    if (!registerVirtualFolder(helpData->virtualFolder(), helpData->namespaceName())) {
        m_error = tr("Cannot register namespace \"%1\".").arg(helpData->namespaceName());
        cleanupDB();
        return false;
    }
    addProgress(1.0);

    emit statusChanged(tr("Insert custom filters..."));
    for (const QHelpDataCustomFilter &f : helpData->customFilters()) {
        if (!registerCustomFilter(f.name, f.filterAttributes, true)) {
            cleanupDB();
            return false;
        }
    }
    addProgress(1.0);

    int i = 1;
    for (const QHelpDataFilterSection &fs : helpData->filterSections()) {
        emit statusChanged(tr("Insert help data for filter section (%1 of %2)...")
            .arg(i++).arg(helpData->filterSections().count()));
        insertFilterAttributes(fs.filterAttributes());
        QByteArray ba;
        QDataStream s(&ba, QIODevice::WriteOnly);
        for (QHelpDataContentItem *itm : fs.contents())
            writeTree(s, itm, 0);
        if (!insertFiles(fs.files(), helpData->rootPath(), fs.filterAttributes())
            || !insertContents(ba, fs.filterAttributes())
            || !insertKeywords(fs.indices(), fs.filterAttributes())) {
            cleanupDB();
            return false;
        }
    }

    cleanupDB();
    emit progressChanged(100);
    emit statusChanged(tr("Documentation successfully generated."));
    return true;
}

void HelpGeneratorPrivate::setupProgress(QHelpProjectData *helpData)
{
    m_progress = 0;
    m_oldProgress = 0;

    int numberOfFiles = 0;
    int numberOfIndices = 0;
    for (const QHelpDataFilterSection &fs : helpData->filterSections()) {
        numberOfFiles += fs.files().count();
        numberOfIndices += fs.indices().count();
    }
    // init      2%
    // filters   1%
    // contents 10%
    // files    60%
    // indices  27%
    m_contentStep = 10.0 / qMax(helpData->customFilters().count(), 1);
    m_fileStep = 60.0 / qMax(numberOfFiles, 1);
    m_indexStep = 27.0 / qMax(numberOfIndices, 1);
}

void HelpGeneratorPrivate::addProgress(double step)
{
    m_progress += step;
    if ((m_progress - m_oldProgress) >= 1.0 && m_progress <= 100.0) {
        m_oldProgress = m_progress;
        emit progressChanged(qCeil(m_progress));
    }
}

void HelpGeneratorPrivate::cleanupDB()
{
    if (m_query) {
        m_query->clear();
        delete m_query;
        m_query = nullptr;
    }
    QSqlDatabase::removeDatabase(QLatin1String("builder"));
}

void HelpGeneratorPrivate::writeTree(QDataStream &s, QHelpDataContentItem *item, int depth)
{
    s << depth;
    s << item->reference();
    s << item->title();
    for (QHelpDataContentItem *i : item->children())
        writeTree(s, i, depth + 1);
}

/*!
    Returns the last error message.
*/
QString HelpGeneratorPrivate::error() const
{
    return m_error;
}

bool HelpGeneratorPrivate::createTables()
{
    if (!m_query)
        return false;

    m_query->exec(QLatin1String("SELECT COUNT(*) FROM sqlite_master WHERE TYPE=\'table\'"
        "AND Name=\'NamespaceTable\'"));
    m_query->next();
    if (m_query->value(0).toInt() > 0) {
        m_error = tr("Some tables already exist.");
        return false;
    }

    const QStringList tables = QStringList()
            << QLatin1String("CREATE TABLE NamespaceTable ("
                             "Id INTEGER PRIMARY KEY,"
                             "Name TEXT )")
            << QLatin1String("CREATE TABLE FilterAttributeTable ("
                             "Id INTEGER PRIMARY KEY, "
                             "Name TEXT )")
            << QLatin1String("CREATE TABLE FilterNameTable ("
                             "Id INTEGER PRIMARY KEY, "
                             "Name TEXT )")
            << QLatin1String("CREATE TABLE FilterTable ("
                             "NameId INTEGER, "
                             "FilterAttributeId INTEGER )")
            << QLatin1String("CREATE TABLE IndexTable ("
                             "Id INTEGER PRIMARY KEY, "
                             "Name TEXT, "
                             "Identifier TEXT, "
                             "NamespaceId INTEGER, "
                             "FileId INTEGER, "
                             "Anchor TEXT )")
            << QLatin1String("CREATE TABLE IndexFilterTable ("
                             "FilterAttributeId INTEGER, "
                             "IndexId INTEGER )")
            << QLatin1String("CREATE TABLE ContentsTable ("
                             "Id INTEGER PRIMARY KEY, "
                             "NamespaceId INTEGER, "
                             "Data BLOB )")
            << QLatin1String("CREATE TABLE ContentsFilterTable ("
                             "FilterAttributeId INTEGER, "
                             "ContentsId INTEGER )")
            << QLatin1String("CREATE TABLE FileAttributeSetTable ("
                             "Id INTEGER, "
                             "FilterAttributeId INTEGER )")
            << QLatin1String("CREATE TABLE FileDataTable ("
                             "Id INTEGER PRIMARY KEY, "
                             "Data BLOB )")
            << QLatin1String("CREATE TABLE FileFilterTable ("
                             "FilterAttributeId INTEGER, "
                             "FileId INTEGER )")
            << QLatin1String("CREATE TABLE FileNameTable ("
                             "FolderId INTEGER, "
                             "Name TEXT, "
                             "FileId INTEGER, "
                             "Title TEXT )")
            << QLatin1String("CREATE TABLE FolderTable("
                             "Id INTEGER PRIMARY KEY, "
                             "Name Text, "
                             "NamespaceID INTEGER )")
            << QLatin1String("CREATE TABLE MetaDataTable("
                             "Name Text, "
                             "Value BLOB )");

    for (const QString &q : tables) {
        if (!m_query->exec(q)) {
            m_error = tr("Cannot create tables.");
            return false;
        }
    }

    m_query->exec(QLatin1String("INSERT INTO MetaDataTable VALUES('qchVersion', '1.0')"));

    return true;
}

bool HelpGeneratorPrivate::insertFileNotFoundFile()
{
    if (!m_query)
        return false;

    m_query->exec(QLatin1String("SELECT id FROM FileNameTable WHERE Name=\'\'"));
    if (m_query->next() && m_query->isValid())
        return true;

    m_query->prepare(QLatin1String("INSERT INTO FileDataTable VALUES (Null, ?)"));
    m_query->bindValue(0, QByteArray());
    if (!m_query->exec())
        return false;

    const int fileId = m_query->lastInsertId().toInt();
    m_query->prepare(QLatin1String("INSERT INTO FileNameTable (FolderId, Name, FileId, Title) "
        " VALUES (0, '', ?, '')"));
    m_query->bindValue(0, fileId);
    if (fileId > -1 && m_query->exec()) {
        m_fileMap.insert(QString(), fileId);
        return true;
    }
    return false;
}

bool HelpGeneratorPrivate::registerVirtualFolder(const QString &folderName, const QString &ns)
{
    if (!m_query || folderName.isEmpty() || ns.isEmpty())
        return false;

    m_query->prepare(QLatin1String("SELECT Id FROM FolderTable WHERE Name=?"));
    m_query->bindValue(0, folderName);
    m_query->exec();
    m_query->next();
    if (m_query->isValid() && m_query->value(0).toInt() > 0)
        return true;

    m_namespaceId = -1;
    m_query->prepare(QLatin1String("SELECT Id FROM NamespaceTable WHERE Name=?"));
    m_query->bindValue(0, ns);
    m_query->exec();
    while (m_query->next()) {
        m_namespaceId = m_query->value(0).toInt();
        break;
    }

    if (m_namespaceId < 0) {
        m_query->prepare(QLatin1String("INSERT INTO NamespaceTable VALUES(NULL, ?)"));
        m_query->bindValue(0, ns);
        if (m_query->exec())
            m_namespaceId = m_query->lastInsertId().toInt();
    }

    if (m_namespaceId > 0) {
        m_query->prepare(QLatin1String("SELECT Id FROM FolderTable WHERE Name=?"));
        m_query->bindValue(0, folderName);
        m_query->exec();
        while (m_query->next())
            m_virtualFolderId = m_query->value(0).toInt();

        if (m_virtualFolderId > 0)
            return true;

        m_query->prepare(QLatin1String("INSERT INTO FolderTable (NamespaceId, Name) "
            "VALUES (?, ?)"));
        m_query->bindValue(0, m_namespaceId);
        m_query->bindValue(1, folderName);
        if (m_query->exec()) {
            m_virtualFolderId = m_query->lastInsertId().toInt();
            return m_virtualFolderId > 0;
        }
    }
    m_error = tr("Cannot register virtual folder.");
    return false;
}

bool HelpGeneratorPrivate::insertFiles(const QStringList &files, const QString &rootPath,
                                 const QStringList &filterAttributes)
{
    if (!m_query)
        return false;

    emit statusChanged(tr("Insert files..."));
    QList<int> filterAtts;
    for (const QString &filterAtt : filterAttributes) {
        m_query->prepare(QLatin1String("SELECT Id FROM FilterAttributeTable "
            "WHERE Name=?"));
        m_query->bindValue(0, filterAtt);
        m_query->exec();
        if (m_query->next())
            filterAtts.append(m_query->value(0).toInt());
    }

    int filterSetId = -1;
    m_query->exec(QLatin1String("SELECT MAX(Id) FROM FileAttributeSetTable"));
    if (m_query->next())
        filterSetId = m_query->value(0).toInt();
    if (filterSetId < 0)
        return false;
    ++filterSetId;
    for (int attId : qAsConst(filterAtts)) {
        m_query->prepare(QLatin1String("INSERT INTO FileAttributeSetTable "
            "VALUES(?, ?)"));
        m_query->bindValue(0, filterSetId);
        m_query->bindValue(1, attId);
        m_query->exec();
    }

    int tableFileId = 1;
    m_query->exec(QLatin1String("SELECT MAX(Id) FROM FileDataTable"));
    if (m_query->next())
        tableFileId = m_query->value(0).toInt() + 1;

    QString title;
    QString charSet;
    QList<QByteArray> fileDataList;
    QMap<int, QSet<int> > tmpFileFilterMap;
    QList<FileNameTableData> fileNameDataList;

    int i = 0;
    for (const QString &file : files) {
        const QString fileName = QDir::cleanPath(file);

        QFile fi(rootPath + QDir::separator() + fileName);
        if (!fi.exists()) {
            emit warning(tr("The file %1 does not exist, skipping it...")
                .arg(QDir::cleanPath(rootPath + QDir::separator() + fileName)));
            continue;
        }

        if (!fi.open(QIODevice::ReadOnly)) {
            emit warning(tr("Cannot open file %1, skipping it...")
                .arg(QDir::cleanPath(rootPath + QDir::separator() + fileName)));
            continue;
        }

        QByteArray data = fi.readAll();
        if (fileName.endsWith(QLatin1String(".html"))
            || fileName.endsWith(QLatin1String(".htm"))) {
                charSet = QHelpGlobal::codecFromData(data);
                QTextStream stream(&data);
                stream.setCodec(QTextCodec::codecForName(charSet.toLatin1().constData()));
                title = QHelpGlobal::documentTitle(stream.readAll());
        } else {
            title = fileName.mid(fileName.lastIndexOf(QLatin1Char('/')) + 1);
        }

        int fileId = -1;
        const auto &it = m_fileMap.constFind(fileName);
        if (it == m_fileMap.cend()) {
            fileDataList.append(qCompress(data));

            FileNameTableData fileNameData;
            fileNameData.name = fileName;
            fileNameData.fileId = tableFileId;
            fileNameData.title = title;
            fileNameDataList.append(fileNameData);

            m_fileMap.insert(fileName, tableFileId);
            m_fileFilterMap.insert(tableFileId, filterAtts.toSet());
            tmpFileFilterMap.insert(tableFileId, filterAtts.toSet());

            ++tableFileId;
        } else {
            fileId = it.value();
            QSet<int> &fileFilterSet = m_fileFilterMap[fileId];
            QSet<int> &tmpFileFilterSet = tmpFileFilterMap[fileId];
            for (int filter : qAsConst(filterAtts)) {
                if (!fileFilterSet.contains(filter)
                    && !tmpFileFilterSet.contains(filter)) {
                    fileFilterSet.insert(filter);
                    tmpFileFilterSet.insert(filter);
                }
            }
        }
    }

    if (!tmpFileFilterMap.isEmpty()) {
        m_query->exec(QLatin1String("BEGIN"));
        for (auto it = tmpFileFilterMap.cbegin(), end = tmpFileFilterMap.cend(); it != end; ++it) {
            QList<int> filterValues = it.value().toList();
            std::sort(filterValues.begin(), filterValues.end());
            for (int fv : qAsConst(filterValues)) {
                m_query->prepare(QLatin1String("INSERT INTO FileFilterTable "
                    "VALUES(?, ?)"));
                m_query->bindValue(0, fv);
                m_query->bindValue(1, it.key());
                m_query->exec();
            }
        }

        for (const QByteArray &fileData : qAsConst(fileDataList)) {
            m_query->prepare(QLatin1String("INSERT INTO FileDataTable VALUES "
                "(Null, ?)"));
            m_query->bindValue(0, fileData);
            m_query->exec();
            if (++i % 20 == 0)
                addProgress(m_fileStep * 20.0);
        }

        for (const FileNameTableData &fnd : qAsConst(fileNameDataList)) {
            m_query->prepare(QLatin1String("INSERT INTO FileNameTable "
                "(FolderId, Name, FileId, Title) VALUES (?, ?, ?, ?)"));
            m_query->bindValue(0, 1);
            m_query->bindValue(1, fnd.name);
            m_query->bindValue(2, fnd.fileId);
            m_query->bindValue(3, fnd.title);
            m_query->exec();
        }
        m_query->exec(QLatin1String("COMMIT"));
    }

    m_query->exec(QLatin1String("SELECT MAX(Id) FROM FileDataTable"));
    if (m_query->next()
            && m_query->value(0).toInt() == tableFileId - 1) {
        addProgress(m_fileStep*(i % 20));
        return true;
    }
    return false;
}

bool HelpGeneratorPrivate::registerCustomFilter(const QString &filterName,
    const QStringList &filterAttribs, bool forceUpdate)
{
    if (!m_query)
        return false;

    m_query->exec(QLatin1String("SELECT Id, Name FROM FilterAttributeTable"));
    QStringList idsToInsert = filterAttribs;
    QMap<QString, int> attributeMap;
    while (m_query->next()) {
        attributeMap.insert(m_query->value(1).toString(),
            m_query->value(0).toInt());
        idsToInsert.removeAll(m_query->value(1).toString());
    }

    for (const QString &id : qAsConst(idsToInsert)) {
        m_query->prepare(QLatin1String("INSERT INTO FilterAttributeTable VALUES(NULL, ?)"));
        m_query->bindValue(0, id);
        m_query->exec();
        attributeMap.insert(id, m_query->lastInsertId().toInt());
    }

    int nameId = -1;
    m_query->prepare(QLatin1String("SELECT Id FROM FilterNameTable WHERE Name=?"));
    m_query->bindValue(0, filterName);
    m_query->exec();
    while (m_query->next()) {
        nameId = m_query->value(0).toInt();
        break;
    }

    if (nameId < 0) {
        m_query->prepare(QLatin1String("INSERT INTO FilterNameTable VALUES(NULL, ?)"));
        m_query->bindValue(0, filterName);
        if (m_query->exec())
            nameId = m_query->lastInsertId().toInt();
    } else if (!forceUpdate) {
        m_error = tr("The filter %1 is already registered.").arg(filterName);
        return false;
    }

    if (nameId < 0) {
        m_error = tr("Cannot register filter %1.").arg(filterName);
        return false;
    }

    m_query->prepare(QLatin1String("DELETE FROM FilterTable WHERE NameId=?"));
    m_query->bindValue(0, nameId);
    m_query->exec();

    for (const QString &att : filterAttribs) {
        m_query->prepare(QLatin1String("INSERT INTO FilterTable VALUES(?, ?)"));
        m_query->bindValue(0, nameId);
        m_query->bindValue(1, attributeMap[att]);
        if (!m_query->exec())
            return false;
    }
    return true;
}

bool HelpGeneratorPrivate::insertKeywords(const QList<QHelpDataIndexItem> &keywords,
                                    const QStringList &filterAttributes)
{
    if (!m_query)
        return false;

    emit statusChanged(tr("Insert indices..."));
    int indexId = 1;
    m_query->exec(QLatin1String("SELECT MAX(Id) FROM IndexTable"));
    if (m_query->next())
        indexId = m_query->value(0).toInt() + 1;

    QList<int> filterAtts;
    for (const QString &filterAtt : filterAttributes) {
        m_query->prepare(QLatin1String("SELECT Id FROM FilterAttributeTable WHERE Name=?"));
        m_query->bindValue(0, filterAtt);
        m_query->exec();
        if (m_query->next())
            filterAtts.append(m_query->value(0).toInt());
    }

    QList<int> indexFilterTable;

    int i = 0;
    m_query->exec(QLatin1String("BEGIN"));
    QSet<QString> indices;
    for (const QHelpDataIndexItem &itm : keywords) {
         // Identical ids make no sense and just confuse the Assistant user,
         // so we ignore all repetitions.
        if (indices.contains(itm.identifier))
            continue;

        // Still empty ids should be ignored, as otherwise we will include only
        // the first keyword with an empty id.
        if (!itm.identifier.isEmpty())
            indices.insert(itm.identifier);

        const int pos = itm.reference.indexOf(QLatin1Char('#'));
        const QString &fileName = itm.reference.left(pos);
        const QString anchor = pos < 0 ? QString() : itm.reference.mid(pos + 1);

        const QString &fName = QDir::cleanPath(fileName);

        const auto &it = m_fileMap.constFind(fName);
        const int fileId = it == m_fileMap.cend() ? 1 : it.value();

        m_query->prepare(QLatin1String("INSERT INTO IndexTable (Name, Identifier, NamespaceId, FileId, Anchor) "
            "VALUES(?, ?, ?, ?, ?)"));
        m_query->bindValue(0, itm.name);
        m_query->bindValue(1, itm.identifier);
        m_query->bindValue(2, m_namespaceId);
        m_query->bindValue(3, fileId);
        m_query->bindValue(4, anchor);
        m_query->exec();

        indexFilterTable.append(indexId++);
        if (++i % 100 == 0)
            addProgress(m_indexStep * 100.0);
    }
    m_query->exec(QLatin1String("COMMIT"));

    m_query->exec(QLatin1String("BEGIN"));
    for (int idx : qAsConst(indexFilterTable)) {
        for (int a : qAsConst(filterAtts)) {
            m_query->prepare(QLatin1String("INSERT INTO IndexFilterTable (FilterAttributeId, IndexId) "
                "VALUES(?, ?)"));
            m_query->bindValue(0, a);
            m_query->bindValue(1, idx);
            m_query->exec();
        }
    }
    m_query->exec(QLatin1String("COMMIT"));

    m_query->exec(QLatin1String("SELECT COUNT(Id) FROM IndexTable"));
    if (m_query->next() && m_query->value(0).toInt() >= indices.count())
        return true;
    return false;
}

bool HelpGeneratorPrivate::insertContents(const QByteArray &ba,
                                    const QStringList &filterAttributes)
{
    if (!m_query)
        return false;

    emit statusChanged(tr("Insert contents..."));
    m_query->prepare(QLatin1String("INSERT INTO ContentsTable (NamespaceId, Data) "
        "VALUES(?, ?)"));
    m_query->bindValue(0, m_namespaceId);
    m_query->bindValue(1, ba);
    m_query->exec();
    int contentId = m_query->lastInsertId().toInt();
    if (contentId < 1) {
        m_error = tr("Cannot insert contents.");
        return false;
    }

    // associate the filter attributes
    for (const QString &filterAtt : filterAttributes) {
        m_query->prepare(QLatin1String("INSERT INTO ContentsFilterTable (FilterAttributeId, ContentsId) "
            "SELECT Id, ? FROM FilterAttributeTable WHERE Name=?"));
        m_query->bindValue(0, contentId);
        m_query->bindValue(1, filterAtt);
        m_query->exec();
        if (!m_query->isActive()) {
            m_error = tr("Cannot register contents.");
            return false;
        }
    }
    addProgress(m_contentStep);
    return true;
}

bool HelpGeneratorPrivate::insertFilterAttributes(const QStringList &attributes)
{
    if (!m_query)
        return false;

    m_query->exec(QLatin1String("SELECT Name FROM FilterAttributeTable"));
    QSet<QString> atts;
    while (m_query->next())
        atts.insert(m_query->value(0).toString());

    for (const QString &s : attributes) {
        if (!atts.contains(s)) {
            m_query->prepare(QLatin1String("INSERT INTO FilterAttributeTable VALUES(NULL, ?)"));
            m_query->bindValue(0, s);
            m_query->exec();
        }
    }
    return true;
}

bool HelpGeneratorPrivate::insertMetaData(const QMap<QString, QVariant> &metaData)
{
    if (!m_query)
        return false;

    for (auto it = metaData.cbegin(), end = metaData.cend(); it != end; ++it) {
        m_query->prepare(QLatin1String("INSERT INTO MetaDataTable VALUES(?, ?)"));
        m_query->bindValue(0, it.key());
        m_query->bindValue(1, it.value());
        m_query->exec();
    }
    return true;
}

bool HelpGeneratorPrivate::checkLinks(const QHelpProjectData &helpData)
{
    /*
     * Step 1: Gather the canoncal file paths of all files in the project.
     *         We use a set, because there will be a lot of look-ups.
     */
    QSet<QString> files;
    for (const QHelpDataFilterSection &filterSection : helpData.filterSections()) {
        for (const QString &file : filterSection.files()) {
            const QFileInfo fileInfo(helpData.rootPath() + QDir::separator() + file);
            const QString &canonicalFileName = fileInfo.canonicalFilePath();
            if (!fileInfo.exists())
                emit warning(tr("File \"%1\" does not exist.").arg(file));
            else
                files.insert(canonicalFileName);
        }
    }

    /*
     * Step 2: Check the hypertext and image references of all HTML files.
     *         Note that we don't parse the files, but simply grep for the
     *         respective HTML elements. Therefore. contents that are e.g.
     *         commented out can cause false warning.
     */
    bool allLinksOk = true;
    for (const QString &fileName : qAsConst(files)) {
        if (!fileName.endsWith(QLatin1String("html"))
            && !fileName.endsWith(QLatin1String("htm")))
            continue;
        QFile htmlFile(fileName);
        if (!htmlFile.open(QIODevice::ReadOnly)) {
            emit warning(tr("File \"%1\" cannot be opened.").arg(fileName));
            continue;
        }
        const QRegExp linkPattern(QLatin1String("<(?:a href|img src)=\"?([^#\">]+)[#\">]"));
        QTextStream stream(&htmlFile);
        const QString codec = QHelpGlobal::codecFromData(htmlFile.read(1000));
        stream.setCodec(QTextCodec::codecForName(codec.toLatin1().constData()));
        const QString &content = stream.readAll();
        QStringList invalidLinks;
        for (int pos = linkPattern.indexIn(content); pos != -1;
             pos = linkPattern.indexIn(content, pos + 1)) {
            const QString &linkedFileName = linkPattern.cap(1);
            if (linkedFileName.contains(QLatin1String("://")))
                continue;
            const QString &curDir = QFileInfo(fileName).dir().path();
            const QString &canonicalLinkedFileName =
                QFileInfo(curDir + QDir::separator() + linkedFileName).canonicalFilePath();
            if (!files.contains(canonicalLinkedFileName)
                && !invalidLinks.contains(canonicalLinkedFileName)) {
                emit warning(tr("File \"%1\" contains an invalid link to file \"%2\"").
                         arg(fileName).arg(linkedFileName));
                allLinksOk = false;
                invalidLinks.append(canonicalLinkedFileName);
            }
        }
    }

    if (!allLinksOk)
        m_error = tr("Invalid links in HTML files.");
    return allLinksOk;
}

//////////////////////////////

HelpGenerator::HelpGenerator(bool silent)
{
    m_private = new HelpGeneratorPrivate(this);
    if (!silent) {
        connect(m_private, &HelpGeneratorPrivate::statusChanged,
                this, &HelpGenerator::printStatus);
    }
    connect(m_private, &HelpGeneratorPrivate::warning,
            this, &HelpGenerator::printWarning);
}

bool HelpGenerator::generate(QHelpProjectData *helpData,
                             const QString &outputFileName)
{
    return m_private->generate(helpData, outputFileName);
}

bool HelpGenerator::checkLinks(const QHelpProjectData &helpData)
{
    return m_private->checkLinks(helpData);
}

QString HelpGenerator::error() const
{
    return m_private->error();
}

void HelpGenerator::printStatus(const QString &msg)
{
    puts(qPrintable(msg));
}

void HelpGenerator::printWarning(const QString &msg)
{
    puts(qPrintable(tr("Warning: %1").arg(msg)));
}

QT_END_NAMESPACE

#include "helpgenerator.moc"
