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
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVector>
#include <QtEndian>
#include <QStack>
#include <QFileInfo>
#include <QSaveFile>

/*!
 * \internal
 * Mangles \a str to be a unique C++ identifier. Characters that are invalid for C++ identifiers
 * are replaced by the pattern \c _0x<hex>_ where <hex> is the hexadecimal unicode
 * representation of the character. As identifiers with leading underscores followed by either
 * another underscore or a capital letter are reserved in C++, we also escape those, by escaping
 * the first underscore, using the above method.
 *
 * \note
 * Although C++11 allows for non-ascii (unicode) characters to be used in identifiers,
 * many compilers forgot to read the spec and do not implement this. Some also do not
 * implement C99 identifiers, because that is \e {at the implementation's discretion}. So,
 * we are stuck with plain old boring identifiers.
 */
QString mangledIdentifier(const QString &str)
{
    Q_ASSERT(!str.isEmpty());

    QString mangled;
    mangled.reserve(str.size());

    int i = 0;
    if (str.startsWith(QLatin1Char('_')) && str.size() > 1) {
        QChar ch = str.at(1);
        if (ch == QLatin1Char('_')
                || (ch >= QLatin1Char('A') && ch <= QLatin1Char('Z'))) {
            mangled += QLatin1String("_0x5f_");
            ++i;
        }
    }

    for (int ei = str.length(); i != ei; ++i) {
        auto c = str.at(i).unicode();
        if ((c >= QLatin1Char('0') && c <= QLatin1Char('9'))
            || (c >= QLatin1Char('a') && c <= QLatin1Char('z'))
            || (c >= QLatin1Char('A') && c <= QLatin1Char('Z'))
            || c == QLatin1Char('_')) {
            mangled += c;
        } else {
            mangled += QLatin1String("_0x") + QString::number(c, 16) + QLatin1Char('_');
        }
    }

    return mangled;
}

QString symbolNamespaceForPath(const QString &relativePath)
{
    QFileInfo fi(relativePath);
    QString symbol = fi.path();
    if (symbol.length() == 1 && symbol.startsWith(QLatin1Char('.'))) {
        symbol.clear();
    } else {
        symbol.replace(QLatin1Char('/'), QLatin1Char('_'));
        symbol += QLatin1Char('_');
    }
    symbol += fi.baseName();
    symbol += QLatin1Char('_');
    symbol += fi.completeSuffix();
    return mangledIdentifier(symbol);
}

struct VirtualDirectoryEntry
{
    QString name;
    QVector<VirtualDirectoryEntry*> dirEntries;
    int firstChildIndex = -1; // node index inside generated data
    bool isDirectory = true;

    VirtualDirectoryEntry()
    {}

    ~VirtualDirectoryEntry()
    {
        qDeleteAll(dirEntries);
    }

    VirtualDirectoryEntry *append(const QString &name)
    {
        for (QVector<VirtualDirectoryEntry*>::Iterator it = dirEntries.begin(), end = dirEntries.end();
             it != end; ++it) {
            if ((*it)->name == name)
                return *it;
        }

        VirtualDirectoryEntry *subEntry = new VirtualDirectoryEntry;
        subEntry->name = name;
        dirEntries.append(subEntry);
        return subEntry;
    }

    void appendEmptyFile(const QString &name)
    {
        VirtualDirectoryEntry *subEntry = new VirtualDirectoryEntry;
        subEntry->name = name;
        subEntry->isDirectory = false;
        dirEntries.append(subEntry);
    }

    bool isEmpty() const { return dirEntries.isEmpty(); }
};

struct DataStream
{
    DataStream(QVector<unsigned char > *data = nullptr)
        : data(data)
    {}

    qint64 currentOffset() const { return data->size(); }

    DataStream &operator<<(quint16 value)
    {
        unsigned char d[2];
        qToBigEndian(value, d);
        data->append(d[0]);
        data->append(d[1]);
        return *this;
    }
    DataStream &operator<<(quint32 value)
    {
        unsigned char d[4];
        qToBigEndian(value, d);
        data->append(d[0]);
        data->append(d[1]);
        data->append(d[2]);
        data->append(d[3]);
        return *this;
    }
private:
    QVector<unsigned char> *data;
};

static bool resource_sort_order(const VirtualDirectoryEntry *lhs, const VirtualDirectoryEntry *rhs)
{
    return qt_hash(lhs->name) < qt_hash(rhs->name);
}

struct ResourceTree
{
    ResourceTree()
    {}

    void serialize(VirtualDirectoryEntry &root, QVector<unsigned char> *treeData, QVector<unsigned char> *stringData)
    {
        treeStream = DataStream(treeData);
        stringStream = DataStream(stringData);

        QStack<VirtualDirectoryEntry *> directories;

        {
            directories.push(&root);
            while (!directories.isEmpty()) {
                VirtualDirectoryEntry *entry = directories.pop();
                registerString(entry->name);
                if (entry->isDirectory)
                    directories << entry->dirEntries;
            }
        }

        {
            quint32 currentDirectoryIndex = 1;
            directories.push(&root);
            while (!directories.isEmpty()) {
                VirtualDirectoryEntry *entry = directories.pop();
                entry->firstChildIndex = currentDirectoryIndex;
                currentDirectoryIndex += entry->dirEntries.count();
                std::sort(entry->dirEntries.begin(), entry->dirEntries.end(), resource_sort_order);

                for (QVector<VirtualDirectoryEntry*>::ConstIterator child = entry->dirEntries.constBegin(), end = entry->dirEntries.constEnd();
                     child != end; ++child) {
                    if ((*child)->isDirectory)
                        directories << *child;
                }
            }
        }

        {
            writeTreeEntry(&root);
            directories.push(&root);
            while (!directories.isEmpty()) {
                VirtualDirectoryEntry *entry = directories.pop();

                for (QVector<VirtualDirectoryEntry*>::ConstIterator child = entry->dirEntries.constBegin(), end = entry->dirEntries.constEnd();
                     child != end; ++child) {
                    writeTreeEntry(*child);
                    if ((*child)->isDirectory)
                        directories << (*child);
                }
            }
        }
    }

private:
    DataStream treeStream;
    DataStream stringStream;
    QHash<QString, qint64> stringOffsets;

    void registerString(const QString &name)
    {
        if (stringOffsets.contains(name))
            return;
        const qint64 offset = stringStream.currentOffset();
        stringOffsets.insert(name, offset);

        stringStream << quint16(name.length())
                     << quint32(qt_hash(name));
        for (int i = 0; i < name.length(); ++i)
            stringStream << quint16(name.at(i).unicode());
    }

    void writeTreeEntry(VirtualDirectoryEntry *entry)
    {
        treeStream << quint32(stringOffsets.value(entry->name))
                   << quint16(entry->isDirectory ? 0x2 : 0x0); // Flags: File or Directory

        if (entry->isDirectory) {
            treeStream << quint32(entry->dirEntries.count())
                       << quint32(entry->firstChildIndex);
        } else {
            treeStream << quint16(QLocale::AnyCountry) << quint16(QLocale::C)
                       << quint32(0x0);
        }
    }
};

static QByteArray generateResourceDirectoryTree(QTextStream &code, const QStringList &qrcFiles)
{
    QByteArray call;
    if (qrcFiles.isEmpty())
        return call;

    VirtualDirectoryEntry resourceDirs;
    resourceDirs.name = QStringLiteral("/");

    foreach (const QString &entry, qrcFiles) {
        const QStringList segments = entry.split(QLatin1Char('/'), QString::SkipEmptyParts);

        VirtualDirectoryEntry *dirEntry = &resourceDirs;

        for (int i = 0; i < segments.count() - 1; ++i)
            dirEntry = dirEntry->append(segments.at(i));
        dirEntry->appendEmptyFile(segments.last());
    }

    if (resourceDirs.isEmpty())
        return call;

    QVector<unsigned char> names;
    QVector<unsigned char> tree;
    ResourceTree().serialize(resourceDirs, &tree, &names);

    code << "static const unsigned char qt_resource_tree[] = {\n";
    for (int i = 0; i < tree.count(); ++i) {
        code << uint(tree.at(i));
        if (i < tree.count() - 1)
            code << ',';
        if (i % 16 == 0)
            code << '\n';
    }
    code << "};\n";

    code << "static const unsigned char qt_resource_names[] = {\n";
    for (int i = 0; i < names.count(); ++i) {
        code << uint(names.at(i));
        if (i < names.count() - 1)
            code << ',';
        if (i % 16 == 0)
            code << '\n';
    }
    code << "};\n";

    code << "static const unsigned char qt_resource_empty_payout[] = { 0, 0, 0, 0, 0 };\n";

    code << "QT_BEGIN_NAMESPACE\n";
    code << "extern Q_CORE_EXPORT bool qRegisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);\n";
    code << "QT_END_NAMESPACE\n";

    call = "QT_PREPEND_NAMESPACE(qRegisterResourceData)(/*version*/0x01, qt_resource_tree, qt_resource_names, qt_resource_empty_payout);\n";

    return call;
}

static QString qtResourceNameForFile(const QString &fileName)
{
    QFileInfo fi(fileName);
    QString name = fi.completeBaseName();
    if (name.isEmpty())
        name = fi.fileName();
    name.replace(QRegExp(QLatin1String("[^a-zA-Z0-9_]")), QLatin1String("_"));
    return name;
}

bool generateLoader(const QStringList &compiledFiles, const QString &outputFileName, const QStringList &resourceFileMappings, QString *errorString)
{
    QByteArray generatedLoaderCode;

    {
        QTextStream stream(&generatedLoaderCode);
        stream << "#include <QtQml/qqmlprivate.h>\n";
        stream << "#include <QtCore/qdir.h>\n";
        stream << "#include <QtCore/qurl.h>\n";
        stream << "\n";

        QByteArray resourceRegisterCall = generateResourceDirectoryTree(stream, compiledFiles);

        stream << "namespace QmlCacheGeneratedCode {\n";
        for (int i = 0; i < compiledFiles.count(); ++i) {
            const QString compiledFile = compiledFiles.at(i);
            const QString ns = symbolNamespaceForPath(compiledFile);
            stream << "namespace " << symbolNamespaceForPath(compiledFile) << " { \n";
            stream << "    extern const unsigned char qmlData[];\n";
            stream << "    const QQmlPrivate::CachedQmlUnit unit = {\n";
            stream << "        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), nullptr, nullptr\n";
            stream << "    };\n";
            stream << "}\n";
        }

        stream << "\n}\n";
        stream << "namespace {\n";

        stream << "struct Registry {\n";
        stream << "    Registry();\n";
        stream << "    ~Registry();\n";
        stream << "    QHash<QString, const QQmlPrivate::CachedQmlUnit*> resourcePathToCachedUnit;\n";
        stream << "    static const QQmlPrivate::CachedQmlUnit *lookupCachedUnit(const QUrl &url);\n";
        stream << "};\n\n";
        stream << "Q_GLOBAL_STATIC(Registry, unitRegistry)\n";
        stream << "\n\n";

        stream << "Registry::Registry() {\n";

        for (int i = 0; i < compiledFiles.count(); ++i) {
            const QString qrcFile = compiledFiles.at(i);
            const QString ns = symbolNamespaceForPath(qrcFile);
            stream << "        resourcePathToCachedUnit.insert(QStringLiteral(\"" << qrcFile << "\"), &QmlCacheGeneratedCode::" << ns << "::unit);\n";
        }

        stream << "    QQmlPrivate::RegisterQmlUnitCacheHook registration;\n";
        stream << "    registration.version = 0;\n";
        stream << "    registration.lookupCachedQmlUnit = &lookupCachedUnit;\n";
        stream << "    QQmlPrivate::qmlregister(QQmlPrivate::QmlUnitCacheHookRegistration, &registration);\n";

        if (!resourceRegisterCall.isEmpty())
            stream << resourceRegisterCall;

        stream << "}\n\n";
        stream << "Registry::~Registry() {\n";
        stream << "    QQmlPrivate::qmlunregister(QQmlPrivate::QmlUnitCacheHookRegistration, quintptr(&lookupCachedUnit));\n";
        stream << "}\n\n";

        stream << "const QQmlPrivate::CachedQmlUnit *Registry::lookupCachedUnit(const QUrl &url) {\n";
        stream << "    if (url.scheme() != QLatin1String(\"qrc\"))\n";
        stream << "        return nullptr;\n";
        stream << "    QString resourcePath = QDir::cleanPath(url.path());\n";
        stream << "    if (resourcePath.isEmpty())\n";
        stream << "        return nullptr;\n";
        stream << "    if (!resourcePath.startsWith(QLatin1Char('/')))\n";
        stream << "        resourcePath.prepend(QLatin1Char('/'));\n";
        stream << "    return unitRegistry()->resourcePathToCachedUnit.value(resourcePath, nullptr);\n";
        stream << "}\n";
        stream << "}\n";

        for (const QString &mapping: resourceFileMappings) {
            QString originalResourceFile = mapping;
            QString newResourceFile;
            const int mappingSplit = originalResourceFile.indexOf(QLatin1Char('='));
            if (mappingSplit != -1) {
                newResourceFile = originalResourceFile.mid(mappingSplit + 1);
                originalResourceFile.truncate(mappingSplit);
            }

            const QString suffix = qtResourceNameForFile(originalResourceFile);
            const QString initFunction = QLatin1String("qInitResources_") + suffix;

            stream << QStringLiteral("int QT_MANGLE_NAMESPACE(%1)() {\n").arg(initFunction);
            stream << "    ::unitRegistry();\n";
            if (!newResourceFile.isEmpty())
                stream << "    Q_INIT_RESOURCE(" << qtResourceNameForFile(newResourceFile) << ");\n";
            stream << "    return 1;\n";
            stream << "}\n";
            stream << "Q_CONSTRUCTOR_FUNCTION(QT_MANGLE_NAMESPACE(" << initFunction << "))\n";

            const QString cleanupFunction = QLatin1String("qCleanupResources_") + suffix;
            stream << QStringLiteral("int QT_MANGLE_NAMESPACE(%1)() {\n").arg(cleanupFunction);
            if (!newResourceFile.isEmpty())
                stream << "    Q_CLEANUP_RESOURCE(" << qtResourceNameForFile(newResourceFile) << ");\n";
            stream << "    return 1;\n";
            stream << "}\n";
        }
    }

    QSaveFile f(outputFileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        *errorString = f.errorString();
        return false;
    }

    if (f.write(generatedLoaderCode) != generatedLoaderCode.size()) {
        *errorString = f.errorString();
        return false;
    }

    if (!f.commit()) {
        *errorString = f.errorString();
        return false;
    }

    return true;
}
