/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#ifndef QQMLTYPELOADER_P_H
#define QQMLTYPELOADER_P_H

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

#include <QtQml/qtqmlglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qatomic.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qcache.h>
#if QT_CONFIG(qml_network)
#include <QtNetwork/qnetworkreply.h>
#endif
#include <QtQml/qqmlerror.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlfile.h>
#include <QtQml/qqmlabstracturlinterceptor.h>

#include <private/qhashedstring_p.h>
#include <private/qqmlimport_p.h>
#include <private/qqmlcleanup_p.h>
#include <private/qqmldirparser_p.h>
#include <private/qflagpointer_p.h>
#include <private/qqmlirbuilder_p.h>

#include <private/qv4value_p.h>
#include <private/qv4script_p.h>

QT_BEGIN_NAMESPACE

class QQmlScriptData;
class QQmlScriptBlob;
class QQmlQmldirData;
class QQmlTypeLoader;
class QQmlComponentPrivate;
class QQmlTypeData;
class QQmlTypeLoader;
class QQmlExtensionInterface;
class QQmlProfiler;
struct QQmlCompileError;

namespace QmlIR {
struct Document;
}

class Q_QML_PRIVATE_EXPORT QQmlDataBlob : public QQmlRefCount
{
public:
    enum Status {
        Null,                    // Prior to QQmlTypeLoader::load()
        Loading,                 // Prior to data being received and dataReceived() being called
        WaitingForDependencies,  // While there are outstanding addDependency()s
        ResolvingDependencies,   // While resolving outstanding dependencies, to detect cycles
        Complete,                // Finished
        Error                    // Error
    };

    enum Type { //Matched in QQmlAbstractUrlInterceptor
        QmlFile = QQmlAbstractUrlInterceptor::QmlFile,
        JavaScriptFile = QQmlAbstractUrlInterceptor::JavaScriptFile,
        QmldirFile = QQmlAbstractUrlInterceptor::QmldirFile
    };

    QQmlDataBlob(const QUrl &, Type, QQmlTypeLoader* manager);
    ~QQmlDataBlob() override;

    void startLoading();

    QQmlTypeLoader *typeLoader() const { return m_typeLoader; }

    Type type() const;

    Status status() const;
    bool isNull() const;
    bool isLoading() const;
    bool isWaiting() const;
    bool isComplete() const;
    bool isError() const;
    bool isCompleteOrError() const;

    qreal progress() const;

    QUrl url() const;
    QString urlString() const;
    QUrl finalUrl() const;
    QString finalUrlString() const;

    QList<QQmlError> errors() const;

    class SourceCodeData {
    public:
        QString readAll(QString *error) const;
        QDateTime sourceTimeStamp() const;
        bool exists() const;
        bool isEmpty() const;
    private:
        friend class QQmlDataBlob;
        friend class QQmlTypeLoader;
        QString inlineSourceCode;
        QFileInfo fileInfo;
        bool hasInlineSourceCode = false;
    };

protected:
    // Can be called from within callbacks
    void setError(const QQmlError &);
    void setError(const QList<QQmlError> &errors);
    void setError(const QQmlCompileError &error);
    void setError(const QVector<QQmlCompileError> &errors);
    void setError(const QString &description);
    void addDependency(QQmlDataBlob *);

    // Callbacks made in load thread
    virtual void dataReceived(const SourceCodeData &) = 0;
    virtual void initializeFromCachedUnit(const QV4::CompiledData::Unit*) = 0;
    virtual void done();
#if QT_CONFIG(qml_network)
    virtual void networkError(QNetworkReply::NetworkError);
#endif
    virtual void dependencyError(QQmlDataBlob *);
    virtual void dependencyComplete(QQmlDataBlob *);
    virtual void allDependenciesDone();

    // Callbacks made in main thread
    virtual void downloadProgressChanged(qreal);
    virtual void completed();

protected:
    // Manager that is currently fetching data for me
    QQmlTypeLoader *m_typeLoader;

private:
    friend class QQmlTypeLoader;
    friend class QQmlTypeLoaderThread;

    void tryDone();
    void cancelAllWaitingFor();
    void notifyAllWaitingOnMe();
    void notifyComplete(QQmlDataBlob *);

    struct ThreadData {
        inline ThreadData();
        inline QQmlDataBlob::Status status() const;
        inline void setStatus(QQmlDataBlob::Status);
        inline bool isAsync() const;
        inline void setIsAsync(bool);
        inline quint8 progress() const;
        inline void setProgress(quint8);

    private:
        QAtomicInt _p;
    };
    ThreadData m_data;

    // m_errors should *always* be written before the status is set to Error.
    // We use the status change as a memory fence around m_errors so that locking
    // isn't required.  Once the status is set to Error (or Complete), m_errors
    // cannot be changed.
    QList<QQmlError> m_errors;

    Type m_type;

    QUrl m_url;
    QUrl m_finalUrl;
    mutable QString m_urlString;
    mutable QString m_finalUrlString;

    // List of QQmlDataBlob's that are waiting for me to complete.
    QList<QQmlDataBlob *> m_waitingOnMe;

    // List of QQmlDataBlob's that I am waiting for to complete.
    QVector<QQmlRefPointer<QQmlDataBlob>> m_waitingFor;

    int m_redirectCount:30;
    bool m_inCallback:1;
    bool m_isDone:1;
};

class QQmlTypeLoaderThread;

class QQmlTypeLoaderQmldirContent
{
private:
    friend class QQmlTypeLoader;

    void setContent(const QString &location, const QString &content);
    void setError(const QQmlError &);

public:
    QQmlTypeLoaderQmldirContent();
    QQmlTypeLoaderQmldirContent(const QQmlTypeLoaderQmldirContent &) = default;
    QQmlTypeLoaderQmldirContent &operator=(const QQmlTypeLoaderQmldirContent &) = default;

    bool hasContent() const { return m_hasContent; }
    bool hasError() const;
    QList<QQmlError> errors(const QString &uri) const;

    QString typeNamespace() const;

    QQmlDirComponents components() const;
    QQmlDirScripts scripts() const;
    QQmlDirPlugins plugins() const;

    QString pluginLocation() const;

    bool designerSupported() const;

private:
    QQmlDirParser m_parser;
    QString m_location;
    bool m_hasContent = false;
};

class Q_QML_PRIVATE_EXPORT QQmlTypeLoader
{
    Q_DECLARE_TR_FUNCTIONS(QQmlTypeLoader)
public:
    enum Mode { PreferSynchronous, Asynchronous, Synchronous };

    class Q_QML_PRIVATE_EXPORT Blob : public QQmlDataBlob
    {
    public:
        Blob(const QUrl &url, QQmlDataBlob::Type type, QQmlTypeLoader *loader);
        ~Blob() override;

        const QQmlImports &imports() const { return m_importCache; }

        void setCachedUnitStatus(QQmlMetaType::CachedUnitLookupError status) { m_cachedUnitStatus = status; }

    protected:
        bool addImport(const QV4::CompiledData::Import *import, QList<QQmlError> *errors);

        bool fetchQmldir(const QUrl &url, const QV4::CompiledData::Import *import, int priority, QList<QQmlError> *errors);
        bool updateQmldir(const QQmlRefPointer<QQmlQmldirData> &data, const QV4::CompiledData::Import *import, QList<QQmlError> *errors);

    private:
        virtual bool qmldirDataAvailable(const QQmlRefPointer<QQmlQmldirData> &, QList<QQmlError> *);

        virtual void scriptImported(const QQmlRefPointer<QQmlScriptBlob> &, const QV4::CompiledData::Location &, const QString &, const QString &) {}

        void dependencyComplete(QQmlDataBlob *) override;

    protected:
        virtual QString stringAt(int) const { return QString(); }

        bool isDebugging() const;

        QQmlImports m_importCache;
        QHash<const QV4::CompiledData::Import*, int> m_unresolvedImports;
        QVector<QQmlRefPointer<QQmlQmldirData>> m_qmldirs;
        QQmlMetaType::CachedUnitLookupError m_cachedUnitStatus = QQmlMetaType::CachedUnitLookupError::NoError;
    };

    QQmlTypeLoader(QQmlEngine *);
    ~QQmlTypeLoader();

    QQmlImportDatabase *importDatabase() const;

    static QUrl normalize(const QUrl &unNormalizedUrl);

    QQmlRefPointer<QQmlTypeData> getType(const QUrl &unNormalizedUrl, Mode mode = PreferSynchronous);
    QQmlRefPointer<QQmlTypeData> getType(const QByteArray &, const QUrl &url, Mode mode = PreferSynchronous);

    QQmlRefPointer<QQmlScriptBlob> getScript(const QUrl &unNormalizedUrl);
    QQmlRefPointer<QQmlQmldirData> getQmldir(const QUrl &);

    QString absoluteFilePath(const QString &path);
    bool fileExists(const QString &path, const QString &file);
    bool directoryExists(const QString &path);

    const QQmlTypeLoaderQmldirContent qmldirContent(const QString &filePath);
    void setQmldirContent(const QString &filePath, const QString &content);

    void clearCache();
    void trimCache();

    bool isTypeLoaded(const QUrl &url) const;
    bool isScriptLoaded(const QUrl &url) const;

    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }

    void load(QQmlDataBlob *, Mode = PreferSynchronous);
    void loadWithStaticData(QQmlDataBlob *, const QByteArray &, Mode = PreferSynchronous);
    void loadWithCachedUnit(QQmlDataBlob *blob, const QV4::CompiledData::Unit *unit, Mode mode = PreferSynchronous);

    QQmlEngine *engine() const;
    void initializeEngine(QQmlExtensionInterface *, const char *);
    void invalidate();

#if !QT_CONFIG(qml_debug)
    quintptr profiler() const { return 0; }
    void setProfiler(quintptr) {}
#else
    QQmlProfiler *profiler() const { return m_profiler.data(); }
    void setProfiler(QQmlProfiler *profiler);
#endif // QT_CONFIG(qml_debug)


private:
    friend class QQmlDataBlob;
    friend class QQmlTypeLoaderThread;
#if QT_CONFIG(qml_network)
    friend class QQmlTypeLoaderNetworkReplyProxy;
#endif // qml_network

    void shutdownThread();

    void loadThread(QQmlDataBlob *);
    void loadWithStaticDataThread(QQmlDataBlob *, const QByteArray &);
    void loadWithCachedUnitThread(QQmlDataBlob *blob, const QV4::CompiledData::Unit *unit);
#if QT_CONFIG(qml_network)
    void networkReplyFinished(QNetworkReply *);
    void networkReplyProgress(QNetworkReply *, qint64, qint64);

    typedef QHash<QNetworkReply *, QQmlDataBlob *> NetworkReplies;
#endif

    void setData(QQmlDataBlob *, const QByteArray &);
    void setData(QQmlDataBlob *, const QString &fileName);
    void setData(QQmlDataBlob *, const QQmlDataBlob::SourceCodeData &);
    void setCachedUnit(QQmlDataBlob *blob, const QV4::CompiledData::Unit *unit);

    template<typename T>
    struct TypedCallback
    {
        TypedCallback(T *object, void (T::*func)(QQmlTypeData *)) : o(object), mf(func) {}

        static void redirect(void *arg, QQmlTypeData *type)
        {
            TypedCallback<T> *self = reinterpret_cast<TypedCallback<T> *>(arg);
            ((self->o)->*(self->mf))(type);
        }

    private:
        T *o;
        void (T::*mf)(QQmlTypeData *);
    };

    typedef QHash<QUrl, QQmlTypeData *> TypeCache;
    typedef QHash<QUrl, QQmlScriptBlob *> ScriptCache;
    typedef QHash<QUrl, QQmlQmldirData *> QmldirCache;
    typedef QCache<QString, QCache<QString, bool> > ImportDirCache;
    typedef QStringHash<QQmlTypeLoaderQmldirContent *> ImportQmlDirCache;

    QQmlEngine *m_engine;
    QQmlTypeLoaderThread *m_thread;
    QMutex &m_mutex;

#if QT_CONFIG(qml_debug)
    QScopedPointer<QQmlProfiler> m_profiler;
#endif

#if QT_CONFIG(qml_network)
    NetworkReplies m_networkReplies;
#endif
    TypeCache m_typeCache;
    int m_typeCacheTrimThreshold;
    ScriptCache m_scriptCache;
    QmldirCache m_qmldirCache;
    ImportDirCache m_importDirCache;
    ImportQmlDirCache m_importQmlDirCache;

    template<typename Loader>
    void doLoad(const Loader &loader, QQmlDataBlob *blob, Mode mode);
    void updateTypeCacheTrimThreshold();

    friend struct PlainLoader;
    friend struct CachedLoader;
    friend struct StaticLoader;
};

class Q_AUTOTEST_EXPORT QQmlTypeData : public QQmlTypeLoader::Blob
{
    Q_DECLARE_TR_FUNCTIONS(QQmlTypeData)
public:
    struct TypeReference
    {
        TypeReference() : majorVersion(0), minorVersion(0), needsCreation(true) {}

        QV4::CompiledData::Location location;
        QQmlType type;
        int majorVersion;
        int minorVersion;
        QQmlRefPointer<QQmlTypeData> typeData;
        QString prefix; // used by CompositeSingleton types
        QString qualifiedName() const;
        bool needsCreation;
    };

    struct ScriptReference
    {
        QV4::CompiledData::Location location;
        QString qualifier;
        QQmlRefPointer<QQmlScriptBlob> script;
    };

private:
    friend class QQmlTypeLoader;

    QQmlTypeData(const QUrl &, QQmlTypeLoader *);

public:
    ~QQmlTypeData() override;

    const QList<ScriptReference> &resolvedScripts() const;

    QV4::CompiledData::CompilationUnit *compilationUnit() const;

    // Used by QQmlComponent to get notifications
    struct TypeDataCallback {
        virtual ~TypeDataCallback();
        virtual void typeDataProgress(QQmlTypeData *, qreal) {}
        virtual void typeDataReady(QQmlTypeData *) {}
    };
    void registerCallback(TypeDataCallback *);
    void unregisterCallback(TypeDataCallback *);

protected:
    void done() override;
    void completed() override;
    void dataReceived(const SourceCodeData &) override;
    void initializeFromCachedUnit(const QV4::CompiledData::Unit *unit) override;
    void allDependenciesDone() override;
    void downloadProgressChanged(qreal) override;

    QString stringAt(int index) const override;

private:
    bool tryLoadFromDiskCache();
    bool loadFromSource();
    void restoreIR(QQmlRefPointer<QV4::CompiledData::CompilationUnit> unit);
    void continueLoadFromIR();
    void resolveTypes();
    QQmlCompileError buildTypeResolutionCaches(
            QQmlRefPointer<QQmlTypeNameCache> *typeNameCache,
            QV4::CompiledData::ResolvedTypeReferenceMap *resolvedTypeCache
            ) const;
    void compile(const QQmlRefPointer<QQmlTypeNameCache> &typeNameCache,
                 QV4::CompiledData::ResolvedTypeReferenceMap *resolvedTypeCache,
                 const QV4::CompiledData::DependentTypesHasher &dependencyHasher);
    void createTypeAndPropertyCaches(const QQmlRefPointer<QQmlTypeNameCache> &typeNameCache,
                                      const QV4::CompiledData::ResolvedTypeReferenceMap &resolvedTypeCache);
    bool resolveType(const QString &typeName, int &majorVersion, int &minorVersion,
                     TypeReference &ref, int lineNumber = -1, int columnNumber = -1,
                     bool reportErrors = true,
                     QQmlType::RegistrationType registrationType = QQmlType::AnyRegistrationType);

    void scriptImported(const QQmlRefPointer<QQmlScriptBlob> &blob, const QV4::CompiledData::Location &location, const QString &qualifier, const QString &nameSpace) override;


    SourceCodeData m_backupSourceCode; // used when cache verification fails.
    QScopedPointer<QmlIR::Document> m_document;
    QV4::CompiledData::TypeReferenceMap m_typeReferences;

    QList<ScriptReference> m_scripts;

    QSet<QString> m_namespaces;
    QList<TypeReference> m_compositeSingletons;

    // map from name index to resolved type
    // While this could be a hash, a map is chosen here to provide a stable
    // order, which is used to calculating a check-sum on dependent meta-objects.
    QMap<int, TypeReference> m_resolvedTypes;
    bool m_typesResolved:1;

    QQmlRefPointer<QV4::CompiledData::CompilationUnit> m_compiledData;

    QList<TypeDataCallback *> m_callbacks;

    bool m_implicitImportLoaded;
    bool loadImplicitImport();
};

// QQmlScriptData instances are created, uninitialized, by the loader in the
// load thread.  The first time they are used by the VME, they are initialized which
// creates their v8 objects and they are referenced and added to the  engine's cleanup
// list.  During QQmlCleanup::clear() all v8 resources are destroyed, and the
// reference that was created is released but final deletion only occurs once all the
// references as released.  This is all intended to ensure that the v8 resources are
// only created and destroyed in the main thread :)
class Q_AUTOTEST_EXPORT QQmlScriptData : public QQmlCleanup, public QQmlRefCount
{
private:
    friend class QQmlTypeLoader;

    QQmlScriptData();

public:
    QUrl url;
    QString urlString;
    QQmlTypeNameCache *typeNameCache;
    QVector<QQmlRefPointer<QQmlScriptBlob>> scripts;

    QV4::ReturnedValue scriptValueForContext(QQmlContextData *parentCtxt);

    QQmlRefPointer<QV4::CompiledData::CompilationUnit> compilationUnit() const { return m_precompiledScript; }

protected:
    void clear() override; // From QQmlCleanup

private:
    friend class QQmlScriptBlob;

    void initialize(QQmlEngine *);
    QQmlContextData *qmlContextDataForContext(QQmlContextData *parentQmlContextData);

    bool m_loaded;
    QQmlRefPointer<QV4::CompiledData::CompilationUnit> m_precompiledScript;
    QV4::PersistentValue m_value;
};

class Q_AUTOTEST_EXPORT QQmlScriptBlob : public QQmlTypeLoader::Blob
{
private:
    friend class QQmlTypeLoader;

    QQmlScriptBlob(const QUrl &, QQmlTypeLoader *);

public:
    ~QQmlScriptBlob() override;

    struct ScriptReference
    {
        QV4::CompiledData::Location location;
        QString qualifier;
        QString nameSpace;
        QQmlRefPointer<QQmlScriptBlob> script;
    };

    QQmlRefPointer<QQmlScriptData> scriptData() const;

protected:
    void dataReceived(const SourceCodeData &) override;
    void initializeFromCachedUnit(const QV4::CompiledData::Unit *unit) override;
    void done() override;

    QString stringAt(int index) const override;

private:
    void scriptImported(const QQmlRefPointer<QQmlScriptBlob> &blob, const QV4::CompiledData::Location &location, const QString &qualifier, const QString &nameSpace) override;
    void initializeFromCompilationUnit(const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &unit);

    QList<ScriptReference> m_scripts;
    QQmlRefPointer<QQmlScriptData> m_scriptData;
    const bool m_isModule;
};

class Q_AUTOTEST_EXPORT QQmlQmldirData : public QQmlTypeLoader::Blob
{
private:
    friend class QQmlTypeLoader;

    QQmlQmldirData(const QUrl &, QQmlTypeLoader *);

public:
    const QString &content() const;

    const QV4::CompiledData::Import *import(QQmlTypeLoader::Blob *) const;
    void setImport(QQmlTypeLoader::Blob *, const QV4::CompiledData::Import *);

    int priority(QQmlTypeLoader::Blob *) const;
    void setPriority(QQmlTypeLoader::Blob *, int);

protected:
    void dataReceived(const SourceCodeData &) override;
    void initializeFromCachedUnit(const QV4::CompiledData::Unit *) override;

private:
    QString m_content;
    QHash<QQmlTypeLoader::Blob *, const QV4::CompiledData::Import *> m_imports;
    QHash<QQmlTypeLoader::Blob *, int> m_priorities;
};


QT_END_NAMESPACE

#endif // QQMLTYPELOADER_P_H
