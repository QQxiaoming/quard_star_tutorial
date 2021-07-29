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

#include <QtQml/qqmlprivate.h>
#include "qqmlmetatype_p.h"

#include <private/qqmlproxymetaobject_p.h>
#include <private/qqmlcustomparser_p.h>
#include <private/qhashedstring_p.h>
#include <private/qqmlimport_p.h>

#include <QtCore/qdebug.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qbitarray.h>
#include <QtCore/qreadwritelock.h>
#include <QtCore/private/qmetaobject_p.h>
#include <QtCore/qloggingcategory.h>

#include <qmetatype.h>
#include <qobjectdefs.h>
#include <qbytearray.h>
#include <qreadwritelock.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvector.h>

#include <ctype.h>
#include "qqmlcomponent.h"

Q_DECLARE_LOGGING_CATEGORY(DBG_DISK_CACHE)

QT_BEGIN_NAMESPACE

struct QQmlMetaTypeData
{
    QQmlMetaTypeData();
    ~QQmlMetaTypeData();
    void registerType(QQmlTypePrivate *priv);
    QList<QQmlType> types;
    QSet<QQmlType> undeletableTypes;
    typedef QHash<int, QQmlTypePrivate *> Ids;
    Ids idToType;
    typedef QHash<QHashedStringRef, QQmlTypePrivate *> Names;
    Names nameToType;
    typedef QHash<QUrl, QQmlTypePrivate *> Files; //For file imported composite types only
    Files urlToType;
    Files urlToNonFileImportType; // For non-file imported composite and composite
                                  // singleton types. This way we can locate any
                                  // of them by url, even if it was registered as
                                  // a module via QQmlPrivate::RegisterCompositeType
    typedef QHash<const QMetaObject *, QQmlTypePrivate *> MetaObjects;
    MetaObjects metaObjectToType;
    typedef QHash<int, QQmlMetaType::StringConverter> StringConverters;
    StringConverters stringConverters;

    struct VersionedUri {
        VersionedUri()
        : majorVersion(0) {}
        VersionedUri(const QHashedString &uri, int majorVersion)
        : uri(uri), majorVersion(majorVersion) {}
        bool operator==(const VersionedUri &other) const {
            return other.majorVersion == majorVersion && other.uri == uri;
        }
        QHashedString uri;
        int majorVersion;
    };
    typedef QHash<VersionedUri, QQmlTypeModule *> TypeModules;
    TypeModules uriToModule;

    QBitArray objects;
    QBitArray interfaces;
    QBitArray lists;

    QList<QQmlPrivate::AutoParentFunction> parentFunctions;
    QVector<QQmlPrivate::QmlUnitCacheLookupFunction> lookupCachedQmlUnit;

    QSet<QString> protectedNamespaces;

    QString typeRegistrationNamespace;

    QHash<int, int> qmlLists;

    QHash<const QMetaObject *, QQmlPropertyCache *> propertyCaches;
    QQmlPropertyCache *propertyCache(const QMetaObject *metaObject, int minorVersion);
    QQmlPropertyCache *propertyCache(const QQmlType &type, int minorVersion);

    void startRecordingTypeRegFailures(QStringList *storage)
    { typeRegistrationFailures = storage; }
    void stopRecordingTypeRegFailures()
    { startRecordingTypeRegFailures(nullptr); }
    void recordTypeRegFailure(const QString &message)
    {
        if (typeRegistrationFailures)
            typeRegistrationFailures->append(message);
        else
            qWarning("%s", message.toUtf8().constData());
    }

private:
    QStringList *typeRegistrationFailures = nullptr;
};

struct EnumInfo {
    QStringList path;
    QString metaObjectName;
    QString enumName;
    QString enumKey;
    QString metaEnumScope;
    bool scoped;
};

class QQmlTypeModulePrivate
{
public:
    QQmlTypeModulePrivate()
    : minMinorVersion(INT_MAX), maxMinorVersion(0), locked(false) {}

    static QQmlTypeModulePrivate* get(QQmlTypeModule* q) { return q->d; }

    QQmlMetaTypeData::VersionedUri uri;

    int minMinorVersion;
    int maxMinorVersion;
    bool locked;

    void add(QQmlTypePrivate *);
    void remove(const QQmlTypePrivate *type);

    typedef QStringHash<QList<QQmlTypePrivate *> > TypeHash;
    TypeHash typeHash;
};

Q_GLOBAL_STATIC(QQmlMetaTypeData, metaTypeData)
Q_GLOBAL_STATIC_WITH_ARGS(QMutex, metaTypeDataLock, (QMutex::Recursive))

static uint qHash(const QQmlMetaTypeData::VersionedUri &v)
{
    return v.uri.hash() ^ qHash(v.majorVersion);
}

QQmlMetaTypeRegistrationFailureRecorder::QQmlMetaTypeRegistrationFailureRecorder()
{
    metaTypeData()->startRecordingTypeRegFailures(&_failures);
}

QQmlMetaTypeRegistrationFailureRecorder::~QQmlMetaTypeRegistrationFailureRecorder()
{
    metaTypeData()->stopRecordingTypeRegFailures();
}

QQmlMetaTypeData::QQmlMetaTypeData()
{
}

QQmlMetaTypeData::~QQmlMetaTypeData()
{
    for (TypeModules::const_iterator i = uriToModule.constBegin(), cend = uriToModule.constEnd(); i != cend; ++i)
        delete *i;
    for (QHash<const QMetaObject *, QQmlPropertyCache *>::Iterator it = propertyCaches.begin(), end = propertyCaches.end();
         it != end; ++it)
        (*it)->release();
}

class QQmlTypePrivate
{
    Q_DISABLE_COPY(QQmlTypePrivate)
public:
    QQmlTypePrivate(QQmlType::RegistrationType type);
    ~QQmlTypePrivate();

    void init() const;
    void initEnums(const QQmlPropertyCache *cache = nullptr) const;
    void insertEnums(const QMetaObject *metaObject) const;
    void insertEnumsFromPropertyCache(const QQmlPropertyCache *cache) const;

    QAtomicInt refCount;
    QQmlType::RegistrationType regType;

    struct QQmlCppTypeData
    {
        int allocationSize;
        void (*newFunc)(void *);
        QString noCreationReason;
        int parserStatusCast;
        QObject *(*extFunc)(QObject *);
        const QMetaObject *extMetaObject;
        QQmlCustomParser *customParser;
        QQmlAttachedPropertiesFunc attachedPropertiesFunc;
        const QMetaObject *attachedPropertiesType;
        int propertyValueSourceCast;
        int propertyValueInterceptorCast;
        bool registerEnumClassesUnscoped;
    };

    struct QQmlSingletonTypeData
    {
        QQmlType::SingletonInstanceInfo *singletonInstanceInfo;
    };

    struct QQmlCompositeTypeData
    {
        QUrl url;
    };

    union extraData {
        QQmlCppTypeData* cd;
        QQmlSingletonTypeData* sd;
        QQmlCompositeTypeData* fd;
    } extraData;

    const char *iid;
    QHashedString module;
    QString name;
    QString elementName;
    int version_maj;
    int version_min;
    int typeId;
    int listId;
    int revision;
    mutable bool containsRevisionedAttributes;
    mutable QQmlType superType;
    const QMetaObject *baseMetaObject;

    int index;
    mutable volatile bool isSetup:1;
    mutable volatile bool isEnumFromCacheSetup:1;
    mutable volatile bool isEnumFromBaseSetup:1;
    mutable bool haveSuperType:1;
    mutable QList<QQmlProxyMetaObject::ProxyData> metaObjects;
    mutable QStringHash<int> enums;
    mutable QStringHash<int> scopedEnumIndex; // maps from enum name to index in scopedEnums
    mutable QList<QStringHash<int>*> scopedEnums;

    struct PropertyCacheByMinorVersion
    {
        PropertyCacheByMinorVersion() : cache(nullptr), minorVersion(-1) {}
        explicit PropertyCacheByMinorVersion(QQmlPropertyCache *pc, int ver) : cache(pc), minorVersion(ver) {}
        QQmlPropertyCachePtr cache;
        int minorVersion;
    };
    QVector<PropertyCacheByMinorVersion> propertyCaches;
    QQmlPropertyCache *propertyCacheForMinorVersion(int minorVersion) const;
    void setPropertyCacheForMinorVersion(int minorVersion, QQmlPropertyCache *cache);
private:
    void createListOfPossibleConflictingItems(const QMetaObject *metaObject, QList<EnumInfo> &enumInfoList, QStringList path) const;
    void createEnumConflictReport(const QMetaObject *metaObject, const QString &conflictingKey) const;
};

void QQmlMetaTypeData::registerType(QQmlTypePrivate *priv)
{
    for (int i = 0; i < types.count(); ++i) {
        if (!types.at(i).isValid()) {
            types[i] = QQmlType(priv);
            priv->index = i;
            return;
        }
    }
    types.append(QQmlType(priv));
    priv->index = types.count() - 1;
}

void QQmlType::SingletonInstanceInfo::init(QQmlEngine *e)
{
    if (scriptCallback && scriptApi(e).isUndefined()) {
        QJSValue value = scriptCallback(e, e);
        if (value.isQObject()) {
            QObject *o = value.toQObject();
            // even though the object is defined in C++, qmlContext(obj) and qmlEngine(obj)
            // should behave identically to QML singleton types.
            e->setContextForObject(o, new QQmlContext(e->rootContext(), e));
        }
        setScriptApi(e, value);
    } else if (qobjectCallback && !qobjectApi(e)) {
        QObject *o = qobjectCallback(e, e);
        setQObjectApi(e, o);
        if (!o) {
            qFatal("qmlRegisterSingletonType(): \"%s\" is not available because the callback function returns a null pointer.", qPrintable(typeName));
        }
        // if this object can use a property cache, create it now
        QQmlData::ensurePropertyCache(e, o);
        // even though the object is defined in C++, qmlContext(obj) and qmlEngine(obj)
        // should behave identically to QML singleton types.
        e->setContextForObject(o, new QQmlContext(e->rootContext(), e));
    } else if (!url.isEmpty() && !qobjectApi(e)) {
        QQmlComponent component(e, url, QQmlComponent::PreferSynchronous);
        QObject *o = component.beginCreate(e->rootContext());
        setQObjectApi(e, o);
        if (o)
            component.completeCreate();
    }
}

void QQmlType::SingletonInstanceInfo::destroy(QQmlEngine *e)
{
    // cleans up the engine-specific singleton instances if they exist.
    scriptApis.remove(e);
    QObject *o = qobjectApis.take(e);
    if (o) {
        QQmlData *ddata = QQmlData::get(o, false);
        if (url.isEmpty() && ddata && ddata->indestructible && ddata->explicitIndestructibleSet)
            return;
        delete o;
    }
}

void QQmlType::SingletonInstanceInfo::setQObjectApi(QQmlEngine *e, QObject *o)
{
    qobjectApis.insert(e, o);
}

QObject *QQmlType::SingletonInstanceInfo::qobjectApi(QQmlEngine *e) const
{
    return qobjectApis.value(e);
}

void QQmlType::SingletonInstanceInfo::setScriptApi(QQmlEngine *e, const QJSValue &v)
{
    scriptApis.insert(e, v);
}

QJSValue QQmlType::SingletonInstanceInfo::scriptApi(QQmlEngine *e) const
{
    return scriptApis.value(e);
}

QQmlTypePrivate::QQmlTypePrivate(QQmlType::RegistrationType type)
: refCount(1), regType(type), iid(nullptr), typeId(0), listId(0), revision(0),
    containsRevisionedAttributes(false), baseMetaObject(nullptr),
    index(-1), isSetup(false), isEnumFromCacheSetup(false), isEnumFromBaseSetup(false),
    haveSuperType(false)
{
    switch (type) {
    case QQmlType::CppType:
        extraData.cd = new QQmlCppTypeData;
        extraData.cd->allocationSize = 0;
        extraData.cd->newFunc = nullptr;
        extraData.cd->parserStatusCast = -1;
        extraData.cd->extFunc = nullptr;
        extraData.cd->extMetaObject = nullptr;
        extraData.cd->customParser = nullptr;
        extraData.cd->attachedPropertiesFunc = nullptr;
        extraData.cd->attachedPropertiesType = nullptr;
        extraData.cd->propertyValueSourceCast = -1;
        extraData.cd->propertyValueInterceptorCast = -1;
        extraData.cd->registerEnumClassesUnscoped = true;
        break;
    case QQmlType::SingletonType:
    case QQmlType::CompositeSingletonType:
        extraData.sd = new QQmlSingletonTypeData;
        extraData.sd->singletonInstanceInfo = nullptr;
        break;
    case QQmlType::InterfaceType:
        extraData.cd = nullptr;
        break;
    case QQmlType::CompositeType:
        extraData.fd = new QQmlCompositeTypeData;
        break;
    default: qFatal("QQmlTypePrivate Internal Error.");
    }
}

QQmlTypePrivate::~QQmlTypePrivate()
{
    qDeleteAll(scopedEnums);
    switch (regType) {
    case QQmlType::CppType:
        delete extraData.cd->customParser;
        delete extraData.cd;
        break;
    case QQmlType::SingletonType:
    case QQmlType::CompositeSingletonType:
        delete extraData.sd->singletonInstanceInfo;
        delete extraData.sd;
        break;
    case QQmlType::CompositeType:
        delete extraData.fd;
        break;
    default: //Also InterfaceType, because it has no extra data
        break;
    }
}

QQmlType::QQmlType(QQmlMetaTypeData *data, const QQmlPrivate::RegisterInterface &interface)
    : d(new QQmlTypePrivate(InterfaceType))
{
    d->iid = interface.iid;
    d->typeId = interface.typeId;
    d->listId = interface.listId;
    d->isSetup = true;
    d->version_maj = 0;
    d->version_min = 0;
    data->registerType(d);
}

QQmlType::QQmlType(QQmlMetaTypeData *data, const QString &elementName, const QQmlPrivate::RegisterSingletonType &type)
    : d(new QQmlTypePrivate(SingletonType))
{
    data->registerType(d);

    d->elementName = elementName;
    d->module = QString::fromUtf8(type.uri);

    d->version_maj = type.versionMajor;
    d->version_min = type.versionMinor;

    if (type.qobjectApi) {
        if (type.version >= 1) // static metaobject added in version 1
            d->baseMetaObject = type.instanceMetaObject;
        if (type.version >= 2) // typeId added in version 2
            d->typeId = type.typeId;
        if (type.version >= 2) // revisions added in version 2
            d->revision = type.revision;
    }

    d->extraData.sd->singletonInstanceInfo = new SingletonInstanceInfo;
    d->extraData.sd->singletonInstanceInfo->scriptCallback = type.scriptApi;
    d->extraData.sd->singletonInstanceInfo->qobjectCallback = type.qobjectApi;
    d->extraData.sd->singletonInstanceInfo->typeName = QString::fromUtf8(type.typeName);
    d->extraData.sd->singletonInstanceInfo->instanceMetaObject
        = (type.qobjectApi && type.version >= 1) ? type.instanceMetaObject : nullptr;
}

QQmlType::QQmlType(QQmlMetaTypeData *data, const QString &elementName, const QQmlPrivate::RegisterCompositeSingletonType &type)
    : d(new QQmlTypePrivate(CompositeSingletonType))
{
    data->registerType(d);

    d->elementName = elementName;
    d->module = QString::fromUtf8(type.uri);

    d->version_maj = type.versionMajor;
    d->version_min = type.versionMinor;

    d->extraData.sd->singletonInstanceInfo = new SingletonInstanceInfo;
    d->extraData.sd->singletonInstanceInfo->url = QQmlTypeLoader::normalize(type.url);
    d->extraData.sd->singletonInstanceInfo->typeName = QString::fromUtf8(type.typeName);
}

QQmlType::QQmlType(QQmlMetaTypeData *data, const QString &elementName, const QQmlPrivate::RegisterType &type)
    : d(new QQmlTypePrivate(CppType))
{
    data->registerType(d);

    d->elementName = elementName;
    d->module = QString::fromUtf8(type.uri);

    d->version_maj = type.versionMajor;
    d->version_min = type.versionMinor;
    if (type.version >= 1) // revisions added in version 1
        d->revision = type.revision;
    d->typeId = type.typeId;
    d->listId = type.listId;
    d->extraData.cd->allocationSize = type.objectSize;
    d->extraData.cd->newFunc = type.create;
    d->extraData.cd->noCreationReason = type.noCreationReason;
    d->baseMetaObject = type.metaObject;
    d->extraData.cd->attachedPropertiesFunc = type.attachedPropertiesFunction;
    d->extraData.cd->attachedPropertiesType = type.attachedPropertiesMetaObject;
    d->extraData.cd->parserStatusCast = type.parserStatusCast;
    d->extraData.cd->propertyValueSourceCast = type.valueSourceCast;
    d->extraData.cd->propertyValueInterceptorCast = type.valueInterceptorCast;
    d->extraData.cd->extFunc = type.extensionObjectCreate;
    d->extraData.cd->customParser = type.customParser;
    d->extraData.cd->registerEnumClassesUnscoped = true;

    if (type.extensionMetaObject)
        d->extraData.cd->extMetaObject = type.extensionMetaObject;

    // Check if the user wants only scoped enum classes
    if (d->baseMetaObject) {
        auto indexOfClassInfo = d->baseMetaObject->indexOfClassInfo("RegisterEnumClassesUnscoped");
        if (indexOfClassInfo != -1 && QString::fromUtf8(d->baseMetaObject->classInfo(indexOfClassInfo).value()) == QLatin1String("false"))
            d->extraData.cd->registerEnumClassesUnscoped = false;
    }
}

QQmlType::QQmlType(QQmlMetaTypeData *data, const QString &elementName, const QQmlPrivate::RegisterCompositeType &type)
    : d(new QQmlTypePrivate(CompositeType))
{
    data->registerType(d);

    d->elementName = elementName;

    d->module = QString::fromUtf8(type.uri);
    d->version_maj = type.versionMajor;
    d->version_min = type.versionMinor;

    d->extraData.fd->url = QQmlTypeLoader::normalize(type.url);
}

QQmlType::QQmlType()
    : d(nullptr)
{
}

QQmlType::QQmlType(const QQmlType &other)
    : d(other.d)
{
    if (d)
        d->refCount.ref();
}

QQmlType &QQmlType::operator =(const QQmlType &other)
{
    if (d != other.d) {
        if (d && !d->refCount.deref())
            delete d;
        d = other.d;
        if (d)
            d->refCount.ref();
    }
    return *this;
}

QQmlType::QQmlType(QQmlTypePrivate *priv)
    : d(priv)
{
    if (d)
        d->refCount.ref();
}

QQmlType::~QQmlType()
{
    if (d && !d->refCount.deref())
        delete d;
}

QHashedString QQmlType::module() const
{
    if (!d)
        return QHashedString();
    return d->module;
}

int QQmlType::majorVersion() const
{
    if (!d)
        return -1;
    return d->version_maj;
}

int QQmlType::minorVersion() const
{
    if (!d)
        return -1;
    return d->version_min;
}

bool QQmlType::availableInVersion(int vmajor, int vminor) const
{
    Q_ASSERT(vmajor >= 0 && vminor >= 0);
    if (!d)
        return false;
    return vmajor == d->version_maj && vminor >= d->version_min;
}

bool QQmlType::availableInVersion(const QHashedStringRef &module, int vmajor, int vminor) const
{
    Q_ASSERT(vmajor >= 0 && vminor >= 0);
    if (!d)
        return false;
    return module == d->module && vmajor == d->version_maj && vminor >= d->version_min;
}

// returns the nearest _registered_ super class
QQmlType QQmlType::superType() const
{
    if (!d)
        return QQmlType();
    if (!d->haveSuperType && d->baseMetaObject) {
        const QMetaObject *mo = d->baseMetaObject->superClass();
        while (mo && !d->superType.isValid()) {
            d->superType = QQmlMetaType::qmlType(mo, d->module, d->version_maj, d->version_min);
            mo = mo->superClass();
        }
        d->haveSuperType = true;
    }

    return d->superType;
}

QQmlType QQmlType::resolveCompositeBaseType(QQmlEnginePrivate *engine) const
{
    Q_ASSERT(isComposite());
    if (!engine || !d)
        return QQmlType();
    QQmlRefPointer<QQmlTypeData> td(engine->typeLoader.getType(sourceUrl()));
    if (td.isNull() || !td->isComplete())
        return QQmlType();
    QV4::CompiledData::CompilationUnit *compilationUnit = td->compilationUnit();
    const QMetaObject *mo = compilationUnit->rootPropertyCache()->firstCppMetaObject();
    return QQmlMetaType::qmlType(mo);
}

QQmlPropertyCache *QQmlType::compositePropertyCache(QQmlEnginePrivate *engine) const
{
    // similar logic to resolveCompositeBaseType
    Q_ASSERT(isComposite());
    if (!engine)
        return nullptr;
    QQmlRefPointer<QQmlTypeData> td(engine->typeLoader.getType(sourceUrl()));
    if (td.isNull() || !td->isComplete())
        return nullptr;
    QV4::CompiledData::CompilationUnit *compilationUnit = td->compilationUnit();
    return compilationUnit->rootPropertyCache().data();
}

static void clone(QMetaObjectBuilder &builder, const QMetaObject *mo,
                  const QMetaObject *ignoreStart, const QMetaObject *ignoreEnd)
{
    // Set classname
    builder.setClassName(ignoreEnd->className());

    // Clone Q_CLASSINFO
    for (int ii = mo->classInfoOffset(); ii < mo->classInfoCount(); ++ii) {
        QMetaClassInfo info = mo->classInfo(ii);

        int otherIndex = ignoreEnd->indexOfClassInfo(info.name());
        if (otherIndex >= ignoreStart->classInfoOffset() + ignoreStart->classInfoCount()) {
            // Skip
        } else {
            builder.addClassInfo(info.name(), info.value());
        }
    }

    // Clone Q_PROPERTY
    for (int ii = mo->propertyOffset(); ii < mo->propertyCount(); ++ii) {
        QMetaProperty property = mo->property(ii);

        int otherIndex = ignoreEnd->indexOfProperty(property.name());
        if (otherIndex >= ignoreStart->propertyOffset() + ignoreStart->propertyCount()) {
            builder.addProperty(QByteArray("__qml_ignore__") + property.name(), QByteArray("void"));
            // Skip
        } else {
            builder.addProperty(property);
        }
    }

    // Clone Q_METHODS
    for (int ii = mo->methodOffset(); ii < mo->methodCount(); ++ii) {
        QMetaMethod method = mo->method(ii);

        // More complex - need to search name
        QByteArray name = method.name();


        bool found = false;

        for (int ii = ignoreStart->methodOffset() + ignoreStart->methodCount();
             !found && ii < ignoreEnd->methodOffset() + ignoreEnd->methodCount();
             ++ii) {

            QMetaMethod other = ignoreEnd->method(ii);

            found = name == other.name();
        }

        QMetaMethodBuilder m = builder.addMethod(method);
        if (found) // SKIP
            m.setAccess(QMetaMethod::Private);
    }

    // Clone Q_ENUMS
    for (int ii = mo->enumeratorOffset(); ii < mo->enumeratorCount(); ++ii) {
        QMetaEnum enumerator = mo->enumerator(ii);

        int otherIndex = ignoreEnd->indexOfEnumerator(enumerator.name());
        if (otherIndex >= ignoreStart->enumeratorOffset() + ignoreStart->enumeratorCount()) {
            // Skip
        } else {
            builder.addEnumerator(enumerator);
        }
    }
}

static bool isPropertyRevisioned(const QMetaObject *mo, int index)
{
    int i = index;
    i -= mo->propertyOffset();
    if (i < 0 && mo->d.superdata)
        return isPropertyRevisioned(mo->d.superdata, index);

    const QMetaObjectPrivate *mop = reinterpret_cast<const QMetaObjectPrivate*>(mo->d.data);
    if (i >= 0 && i < mop->propertyCount) {
        int handle = mop->propertyData + 3*i;
        int flags = mo->d.data[handle + 2];

        return (flags & Revisioned);
    }

    return false;
}

void QQmlTypePrivate::init() const
{
    if (isSetup)
        return;

    QMutexLocker lock(metaTypeDataLock());
    if (isSetup)
        return;

    const QMetaObject *mo = baseMetaObject;
    if (!mo) {
        // version 0 singleton type without metaobject information
        return;
    }

    if (regType == QQmlType::CppType) {
        // Setup extended meta object
        // XXX - very inefficient
        if (extraData.cd->extFunc) {
            QMetaObjectBuilder builder;
            clone(builder, extraData.cd->extMetaObject, extraData.cd->extMetaObject, extraData.cd->extMetaObject);
            builder.setFlags(QMetaObjectBuilder::DynamicMetaObject);
            QMetaObject *mmo = builder.toMetaObject();
            mmo->d.superdata = mo;
            QQmlProxyMetaObject::ProxyData data = { mmo, extraData.cd->extFunc, 0, 0 };
            metaObjects << data;
        }
    }

    mo = mo->d.superdata;
    while(mo) {
        QQmlTypePrivate *t = metaTypeData()->metaObjectToType.value(mo);
        if (t) {
            if (t->regType == QQmlType::CppType) {
                if (t->extraData.cd->extFunc) {
                    QMetaObjectBuilder builder;
                    clone(builder, t->extraData.cd->extMetaObject, t->baseMetaObject, baseMetaObject);
                    builder.setFlags(QMetaObjectBuilder::DynamicMetaObject);
                    QMetaObject *mmo = builder.toMetaObject();
                    mmo->d.superdata = baseMetaObject;
                    if (!metaObjects.isEmpty())
                        metaObjects.constLast().metaObject->d.superdata = mmo;
                    QQmlProxyMetaObject::ProxyData data = { mmo, t->extraData.cd->extFunc, 0, 0 };
                    metaObjects << data;
                }
            }
        }
        mo = mo->d.superdata;
    }

    for (int ii = 0; ii < metaObjects.count(); ++ii) {
        metaObjects[ii].propertyOffset =
            metaObjects.at(ii).metaObject->propertyOffset();
        metaObjects[ii].methodOffset =
            metaObjects.at(ii).metaObject->methodOffset();
    }

    // Check for revisioned details
    {
        const QMetaObject *mo = nullptr;
        if (metaObjects.isEmpty())
            mo = baseMetaObject;
        else
            mo = metaObjects.constFirst().metaObject;

        for (int ii = 0; !containsRevisionedAttributes && ii < mo->propertyCount(); ++ii) {
            if (isPropertyRevisioned(mo, ii))
                containsRevisionedAttributes = true;
        }

        for (int ii = 0; !containsRevisionedAttributes && ii < mo->methodCount(); ++ii) {
            if (mo->method(ii).revision() != 0)
                containsRevisionedAttributes = true;
        }
    }

    isSetup = true;
    lock.unlock();
}

void QQmlTypePrivate::initEnums(const QQmlPropertyCache *cache) const
{
    if ((isEnumFromBaseSetup || !baseMetaObject)
           && (isEnumFromCacheSetup || !cache)) {
        return;
    }

    init();

    QMutexLocker lock(metaTypeDataLock());

    if (!isEnumFromCacheSetup && cache) {
        insertEnumsFromPropertyCache(cache);
        isEnumFromCacheSetup = true;
    }

    if (!isEnumFromBaseSetup && baseMetaObject) { // could be singleton type without metaobject
        insertEnums(baseMetaObject);
        isEnumFromBaseSetup = true;
    }
}

void QQmlTypePrivate::insertEnums(const QMetaObject *metaObject) const
{
    // Add any enum values defined by 'related' classes
    if (metaObject->d.relatedMetaObjects) {
        const auto *related = metaObject->d.relatedMetaObjects;
        if (related) {
            while (*related)
                insertEnums(*related++);
        }
    }

    QSet<QString> localEnums;
    const QMetaObject *localMetaObject = nullptr;

    // Add any enum values defined by this class, overwriting any inherited values
    for (int ii = 0; ii < metaObject->enumeratorCount(); ++ii) {
        QMetaEnum e = metaObject->enumerator(ii);
        const bool isScoped = e.isScoped();
        QStringHash<int> *scoped = isScoped ? new QStringHash<int>() : nullptr;

        // We allow enums in sub-classes to overwrite enums from base-classes, such as
        // ListView.Center (from enum PositionMode) overwriting Item.Center (from enum TransformOrigin).
        // This is acceptable because the _use_ of the enum from the QML side requires qualification
        // anyway, i.e. ListView.Center vs. Item.Center.
        // However if a class defines two enums with the same value, then that must produce a warning
        // because it represents a valid conflict.
        if (e.enclosingMetaObject() != localMetaObject) {
            localEnums.clear();
            localMetaObject = e.enclosingMetaObject();
        }

        for (int jj = 0; jj < e.keyCount(); ++jj) {
            const QString key = QString::fromUtf8(e.key(jj));
            const int value = e.value(jj);
             if (!isScoped || (regType == QQmlType::CppType && extraData.cd->registerEnumClassesUnscoped)) {
                 if (localEnums.contains(key)) {
                     auto existingEntry = enums.find(key);
                     if (existingEntry != enums.end() && existingEntry.value() != value) {
                         qWarning("Previously registered enum will be overwritten due to name clash: %s.%s", metaObject->className(), key.toUtf8().constData());
                         createEnumConflictReport(metaObject, key);
                     }
                 } else {
                     localEnums.insert(key);
                 }
                 enums.insert(key, value);
            }
            if (isScoped)
                scoped->insert(key, value);
        }

        if (isScoped) {
            scopedEnums << scoped;
            scopedEnumIndex.insert(QString::fromUtf8(e.name()), scopedEnums.count()-1);
        }
    }
}

void QQmlTypePrivate::createListOfPossibleConflictingItems(const QMetaObject *metaObject, QList<EnumInfo> &enumInfoList, QStringList path) const
{
    path.append(QString::fromUtf8(metaObject->className()));

    if (metaObject->d.relatedMetaObjects) {
        const auto *related = metaObject->d.relatedMetaObjects;
        if (related) {
            while (*related)
                createListOfPossibleConflictingItems(*related++, enumInfoList, path);
        }
    }

    for (int ii = 0; ii < metaObject->enumeratorCount(); ++ii) {
        const auto e = metaObject->enumerator(ii);

        for (int jj = 0; jj < e.keyCount(); ++jj) {
            const QString key = QString::fromUtf8(e.key(jj));

            EnumInfo enumInfo;
            enumInfo.metaObjectName = QString::fromUtf8(metaObject->className());
            enumInfo.enumName = QString::fromUtf8(e.name());
            enumInfo.enumKey = key;
            enumInfo.scoped = e.isScoped();
            enumInfo.path = path;
            enumInfo.metaEnumScope = QString::fromUtf8(e.scope());
            enumInfoList.append(enumInfo);
        }
    }
}

void QQmlTypePrivate::createEnumConflictReport(const QMetaObject *metaObject, const QString &conflictingKey) const
{
    QList<EnumInfo> enumInfoList;

    if (baseMetaObject) // prefer baseMetaObject if available
        metaObject = baseMetaObject;

    if (!metaObject) { // If there is no metaObject at all return early
        qWarning() << "No meta object information available. Skipping conflict analysis.";
        return;
    }

    createListOfPossibleConflictingItems(metaObject, enumInfoList, QStringList());

    qWarning().noquote() << QLatin1String("Possible conflicting items:");
    // find items with conflicting key
    for (const auto i : enumInfoList) {
        if (i.enumKey == conflictingKey)
            qWarning().noquote().nospace() << "    " << i.metaObjectName << "." << i.enumName << "." << i.enumKey << " from scope "
                                           << i.metaEnumScope << " injected by " << i.path.join(QLatin1String("->"));
    }
}

void QQmlTypePrivate::insertEnumsFromPropertyCache(const QQmlPropertyCache *cache) const
{
    const QMetaObject *cppMetaObject = cache->firstCppMetaObject();

    while (cache && cache->metaObject() != cppMetaObject) {

        int count = cache->qmlEnumCount();
        for (int ii = 0; ii < count; ++ii) {
            QStringHash<int> *scoped = new QStringHash<int>();
            QQmlEnumData *enumData = cache->qmlEnum(ii);

            for (int jj = 0; jj < enumData->values.count(); ++jj) {
                const QQmlEnumValue &value = enumData->values.at(jj);
                enums.insert(value.namedValue, value.value);
                scoped->insert(value.namedValue, value.value);
            }
            scopedEnums << scoped;
            scopedEnumIndex.insert(enumData->name, scopedEnums.count()-1);
        }
        cache = cache->parent();
    }
    insertEnums(cppMetaObject);
}


QQmlPropertyCache *QQmlTypePrivate::propertyCacheForMinorVersion(int minorVersion) const
{
    for (int i = 0; i < propertyCaches.count(); ++i)
        if (propertyCaches.at(i).minorVersion == minorVersion)
            return propertyCaches.at(i).cache.data();
    return nullptr;
}

void QQmlTypePrivate::setPropertyCacheForMinorVersion(int minorVersion, QQmlPropertyCache *cache)
{
    for (int i = 0; i < propertyCaches.count(); ++i) {
        if (propertyCaches.at(i).minorVersion == minorVersion) {
            propertyCaches[i].cache = cache;
            return;
        }
    }
    propertyCaches.append(PropertyCacheByMinorVersion(cache, minorVersion));
}

QByteArray QQmlType::typeName() const
{
    if (d) {
        if (d->regType == SingletonType || d->regType == CompositeSingletonType)
            return d->extraData.sd->singletonInstanceInfo->typeName.toUtf8();
        else if (d->baseMetaObject)
            return d->baseMetaObject->className();
    }
    return QByteArray();
}

QString QQmlType::elementName() const
{
    if (!d)
        return QString();
    return d->elementName;
}

QString QQmlType::qmlTypeName() const
{
    if (!d)
        return QString();
    if (d->name.isEmpty()) {
        if (!d->module.isEmpty())
            d->name = static_cast<QString>(d->module) + QLatin1Char('/') + d->elementName;
        else
            d->name = d->elementName;
    }

    return d->name;
}

QObject *QQmlType::create() const
{
    if (!d || !isCreatable())
        return nullptr;

    d->init();

    QObject *rv = (QObject *)operator new(d->extraData.cd->allocationSize);
    d->extraData.cd->newFunc(rv);

    if (rv && !d->metaObjects.isEmpty())
        (void)new QQmlProxyMetaObject(rv, &d->metaObjects);

    return rv;
}

void QQmlType::create(QObject **out, void **memory, size_t additionalMemory) const
{
    if (!d || !isCreatable())
        return;

    d->init();

    QObject *rv = (QObject *)operator new(d->extraData.cd->allocationSize + additionalMemory);
    d->extraData.cd->newFunc(rv);

    if (rv && !d->metaObjects.isEmpty())
        (void)new QQmlProxyMetaObject(rv, &d->metaObjects);

    *out = rv;
    *memory = ((char *)rv) + d->extraData.cd->allocationSize;
}

QQmlType::SingletonInstanceInfo *QQmlType::singletonInstanceInfo() const
{
    if (!d)
        return nullptr;
    if (d->regType != SingletonType && d->regType != CompositeSingletonType)
        return nullptr;
    return d->extraData.sd->singletonInstanceInfo;
}

QQmlCustomParser *QQmlType::customParser() const
{
    if (!d)
        return nullptr;
    if (d->regType != CppType)
        return nullptr;
    return d->extraData.cd->customParser;
}

QQmlType::CreateFunc QQmlType::createFunction() const
{
    if (!d || d->regType != CppType)
        return nullptr;
    return d->extraData.cd->newFunc;
}

QString QQmlType::noCreationReason() const
{
    if (!d || d->regType != CppType)
        return QString();
    return d->extraData.cd->noCreationReason;
}

int QQmlType::createSize() const
{
    if (!d || d->regType != CppType)
        return 0;
    return d->extraData.cd->allocationSize;
}

bool QQmlType::isCreatable() const
{
    return d && d->regType == CppType && d->extraData.cd->newFunc;
}

QQmlType::ExtensionFunc QQmlType::extensionFunction() const
{
    if (!d || d->regType != CppType)
        return nullptr;
    return d->extraData.cd->extFunc;
}

bool QQmlType::isExtendedType() const
{
    if (!d)
        return false;
    d->init();

    return !d->metaObjects.isEmpty();
}

bool QQmlType::isSingleton() const
{
    return d && (d->regType == SingletonType || d->regType == CompositeSingletonType);
}

bool QQmlType::isInterface() const
{
    return d && d->regType == InterfaceType;
}

bool QQmlType::isComposite() const
{
    return d && (d->regType == CompositeType || d->regType == CompositeSingletonType);
}

bool QQmlType::isCompositeSingleton() const
{
    return d && d->regType == CompositeSingletonType;
}

int QQmlType::typeId() const
{
    return d ? d->typeId : -1;
}

int QQmlType::qListTypeId() const
{
    return d ? d->listId : -1;
}

const QMetaObject *QQmlType::metaObject() const
{
    if (!d)
        return nullptr;
    d->init();

    if (d->metaObjects.isEmpty())
        return d->baseMetaObject;
    else
        return d->metaObjects.constFirst().metaObject;

}

const QMetaObject *QQmlType::baseMetaObject() const
{
    return d ? d->baseMetaObject : nullptr;
}

bool QQmlType::containsRevisionedAttributes() const
{
    if (!d)
        return false;
    d->init();

    return d->containsRevisionedAttributes;
}

int QQmlType::metaObjectRevision() const
{
    return d ? d->revision : -1;
}

QQmlAttachedPropertiesFunc QQmlType::attachedPropertiesFunction(QQmlEnginePrivate *engine) const
{
    if (!d)
        return nullptr;
    if (d->regType == CppType)
        return d->extraData.cd->attachedPropertiesFunc;

    QQmlType base;
    if (d->regType == CompositeType)
        base = resolveCompositeBaseType(engine);
    return base.attachedPropertiesFunction(engine);
}

const QMetaObject *QQmlType::attachedPropertiesType(QQmlEnginePrivate *engine) const
{
    if (!d)
        return nullptr;
    if (d->regType == CppType)
        return d->extraData.cd->attachedPropertiesType;

    QQmlType base;
    if (d->regType == CompositeType)
        base = resolveCompositeBaseType(engine);
    return base.attachedPropertiesType(engine);
}

/*
This is the id passed to qmlAttachedPropertiesById().  This is different from the index
for the case that a single class is registered under two or more names (eg. Item in
Qt 4.7 and QtQuick 1.0).
*/
int QQmlType::attachedPropertiesId(QQmlEnginePrivate *engine) const
{
    if (!d)
        return -1;
    if (d->regType == CppType)
        return d->extraData.cd->attachedPropertiesType ? d->index : -1;

    QQmlType base;
    if (d->regType == CompositeType)
        base = resolveCompositeBaseType(engine);
    return base.attachedPropertiesId(engine);
}

int QQmlType::parserStatusCast() const
{
    if (!d || d->regType != CppType)
        return -1;
    return d->extraData.cd->parserStatusCast;
}

int QQmlType::propertyValueSourceCast() const
{
    if (!d || d->regType != CppType)
        return -1;
    return d->extraData.cd->propertyValueSourceCast;
}

int QQmlType::propertyValueInterceptorCast() const
{
    if (!d || d->regType != CppType)
        return -1;
    return d->extraData.cd->propertyValueInterceptorCast;
}

const char *QQmlType::interfaceIId() const
{
    if (!d || d->regType != InterfaceType)
        return nullptr;
    return d->iid;
}

int QQmlType::index() const
{
    return d ? d->index : -1;
}

QUrl QQmlType::sourceUrl() const
{
    if (d) {
        if (d->regType == CompositeType)
            return d->extraData.fd->url;
        else if (d->regType == CompositeSingletonType)
            return d->extraData.sd->singletonInstanceInfo->url;
    }
    return QUrl();
}

int QQmlType::enumValue(QQmlEnginePrivate *engine, const QHashedStringRef &name, bool *ok) const
{
    Q_ASSERT(ok);
    if (d) {
        const QQmlPropertyCache *cache = isComposite() ? compositePropertyCache(engine) : nullptr;

        *ok = true;

        d->initEnums(cache);

        int *rv = d->enums.value(name);
        if (rv)
            return *rv;
    }

    *ok = false;
    return -1;
}

int QQmlType::enumValue(QQmlEnginePrivate *engine, const QHashedCStringRef &name, bool *ok) const
{
    Q_ASSERT(ok);
    if (d) {
        const QQmlPropertyCache *cache = isComposite() ? compositePropertyCache(engine) : nullptr;

        *ok = true;

        d->initEnums(cache);

        int *rv = d->enums.value(name);
        if (rv)
            return *rv;
    }

    *ok = false;
    return -1;
}

int QQmlType::enumValue(QQmlEnginePrivate *engine, const QV4::String *name, bool *ok) const
{
    Q_ASSERT(ok);
    if (d) {
        const QQmlPropertyCache *cache = isComposite() ? compositePropertyCache(engine) : nullptr;
        *ok = true;

        d->initEnums(cache);

        int *rv = d->enums.value(name);
        if (rv)
            return *rv;
    }

    *ok = false;
    return -1;
}

int QQmlType::scopedEnumIndex(QQmlEnginePrivate *engine, const QV4::String *name, bool *ok) const
{
    Q_ASSERT(ok);
    if (d) {
        const QQmlPropertyCache *cache = isComposite() ? compositePropertyCache(engine) : nullptr;
        *ok = true;

        d->initEnums(cache);

        int *rv = d->scopedEnumIndex.value(name);
        if (rv)
            return *rv;
    }

    *ok = false;
    return -1;
}

int QQmlType::scopedEnumIndex(QQmlEnginePrivate *engine, const QString &name, bool *ok) const
{
    Q_ASSERT(ok);
    if (d) {
        const QQmlPropertyCache *cache = isComposite() ? compositePropertyCache(engine) : nullptr;
        *ok = true;

        d->initEnums(cache);

        int *rv = d->scopedEnumIndex.value(name);
        if (rv)
            return *rv;
    }

    *ok = false;
    return -1;
}

int QQmlType::scopedEnumValue(QQmlEnginePrivate *engine, int index, const QV4::String *name, bool *ok) const
{
    Q_UNUSED(engine)
    Q_ASSERT(ok);
    *ok = true;

    if (d) {
        Q_ASSERT(index > -1 && index < d->scopedEnums.count());
        int *rv = d->scopedEnums.at(index)->value(name);
        if (rv)
            return *rv;
    }

    *ok = false;
    return -1;
}

int QQmlType::scopedEnumValue(QQmlEnginePrivate *engine, int index, const QString &name, bool *ok) const
{
    Q_UNUSED(engine)
    Q_ASSERT(ok);
    *ok = true;

    if (d) {
        Q_ASSERT(index > -1 && index < d->scopedEnums.count());
        int *rv = d->scopedEnums.at(index)->value(name);
        if (rv)
            return *rv;
    }

    *ok = false;
    return -1;
}

int QQmlType::scopedEnumValue(QQmlEnginePrivate *engine, const QByteArray &scopedEnumName, const QByteArray &name, bool *ok) const
{
    Q_ASSERT(ok);
    if (d) {
        const QQmlPropertyCache *cache = isComposite() ? compositePropertyCache(engine) : nullptr;
        *ok = true;

        d->initEnums(cache);

        int *rv = d->scopedEnumIndex.value(QHashedCStringRef(scopedEnumName.constData(), scopedEnumName.length()));
        if (rv) {
            int index = *rv;
            Q_ASSERT(index > -1 && index < d->scopedEnums.count());
            rv = d->scopedEnums.at(index)->value(QHashedCStringRef(name.constData(), name.length()));
            if (rv)
                return *rv;
        }
    }

    *ok = false;
    return -1;
}

int QQmlType::scopedEnumValue(QQmlEnginePrivate *engine, const QStringRef &scopedEnumName, const QStringRef &name, bool *ok) const
{
    Q_ASSERT(ok);
    if (d) {
        const QQmlPropertyCache *cache = isComposite() ? compositePropertyCache(engine) : nullptr;
        *ok = true;

        d->initEnums(cache);

        int *rv = d->scopedEnumIndex.value(QHashedStringRef(scopedEnumName));
        if (rv) {
            int index = *rv;
            Q_ASSERT(index > -1 && index < d->scopedEnums.count());
            rv = d->scopedEnums.at(index)->value(QHashedStringRef(name));
            if (rv)
                return *rv;
        }
    }

    *ok = false;
    return -1;
}

void QQmlType::refHandle(QQmlTypePrivate *priv)
{
    if (priv)
        priv->refCount.ref();
}

void QQmlType::derefHandle(QQmlTypePrivate *priv)
{
    if (priv && !priv->refCount.deref())
        delete priv;
}

int QQmlType::refCount(QQmlTypePrivate *priv)
{
    if (priv)
        return priv->refCount;
    return -1;
}

namespace {
template <typename QQmlTypeContainer>
void removeQQmlTypePrivate(QQmlTypeContainer &container, const QQmlTypePrivate *reference)
{
    for (typename QQmlTypeContainer::iterator it = container.begin(); it != container.end();) {
        if (*it == reference)
            it = container.erase(it);
        else
            ++it;
    }
}

struct IsQQmlTypePrivate
{
    const QQmlTypePrivate *reference;
    explicit IsQQmlTypePrivate(const QQmlTypePrivate *ref) : reference(ref) {}

    bool operator()(const QQmlTypePrivate *priv) const { return reference == priv; }
};
}

QQmlTypeModule::QQmlTypeModule()
: d(new QQmlTypeModulePrivate)
{
}

QQmlTypeModule::~QQmlTypeModule()
{
    delete d; d = nullptr;
}

QString QQmlTypeModule::module() const
{
    return d->uri.uri;
}

int QQmlTypeModule::majorVersion() const
{
    return d->uri.majorVersion;
}

int QQmlTypeModule::minimumMinorVersion() const
{
    return d->minMinorVersion;
}

int QQmlTypeModule::maximumMinorVersion() const
{
    return d->maxMinorVersion;
}

void QQmlTypeModulePrivate::add(QQmlTypePrivate *type)
{
    int minVersion = type->version_min;
    minMinorVersion = qMin(minMinorVersion, minVersion);
    maxMinorVersion = qMax(maxMinorVersion, minVersion);

    QList<QQmlTypePrivate *> &list = typeHash[type->elementName];
    for (int ii = 0; ii < list.count(); ++ii) {
        Q_ASSERT(list.at(ii));
        if (list.at(ii)->version_min < minVersion) {
            list.insert(ii, type);
            return;
        }
    }
    list.append(type);
}

void QQmlTypeModulePrivate::remove(const QQmlTypePrivate *type)
{
    for (TypeHash::ConstIterator elementIt = typeHash.begin(); elementIt != typeHash.end();) {
        QList<QQmlTypePrivate *> &list = const_cast<QList<QQmlTypePrivate *> &>(elementIt.value());

        removeQQmlTypePrivate(list, type);

#if 0
        if (list.isEmpty())
            elementIt = typeHash.erase(elementIt);
        else
            ++elementIt;
#else
        ++elementIt;
#endif
    }
}

QQmlType QQmlTypeModule::type(const QHashedStringRef &name, int minor) const
{
    QMutexLocker lock(metaTypeDataLock());

    QList<QQmlTypePrivate *> *types = d->typeHash.value(name);
    if (types) {
        for (int ii = 0; ii < types->count(); ++ii)
            if (types->at(ii)->version_min <= minor)
                return QQmlType(types->at(ii));
    }

    return QQmlType();
}

QQmlType QQmlTypeModule::type(const QV4::String *name, int minor) const
{
    QMutexLocker lock(metaTypeDataLock());

    QList<QQmlTypePrivate *> *types = d->typeHash.value(name);
    if (types) {
        for (int ii = 0; ii < types->count(); ++ii)
            if (types->at(ii)->version_min <= minor)
                return QQmlType(types->at(ii));
    }

    return QQmlType();
}

void QQmlTypeModule::walkCompositeSingletons(const std::function<void(const QQmlType &)> &callback) const
{
    QMutexLocker lock(metaTypeDataLock());
    for (auto typeCandidates = d->typeHash.begin(), end = d->typeHash.end();
         typeCandidates != end; ++typeCandidates) {
        for (auto type: typeCandidates.value()) {
            if (type->regType == QQmlType::CompositeSingletonType)
                callback(QQmlType(type));
        }
    }
}

QQmlTypeModuleVersion::QQmlTypeModuleVersion()
: m_module(nullptr), m_minor(0)
{
}

QQmlTypeModuleVersion::QQmlTypeModuleVersion(QQmlTypeModule *module, int minor)
: m_module(module), m_minor(minor)
{
    Q_ASSERT(m_module);
    Q_ASSERT(m_minor >= 0);
}

QQmlTypeModuleVersion::QQmlTypeModuleVersion(const QQmlTypeModuleVersion &o)
: m_module(o.m_module), m_minor(o.m_minor)
{
}

QQmlTypeModuleVersion &QQmlTypeModuleVersion::operator=(const QQmlTypeModuleVersion &o)
{
    m_module = o.m_module;
    m_minor = o.m_minor;
    return *this;
}

QQmlTypeModule *QQmlTypeModuleVersion::module() const
{
    return m_module;
}

int QQmlTypeModuleVersion::minorVersion() const
{
    return m_minor;
}

QQmlType QQmlTypeModuleVersion::type(const QHashedStringRef &name) const
{
    if (!m_module)
        return QQmlType();
    return m_module->type(name, m_minor);
}

QQmlType QQmlTypeModuleVersion::type(const QV4::String *name) const
{
    if (!m_module)
        return QQmlType();
    return m_module->type(name, m_minor);
}

void qmlClearTypeRegistrations() // Declared in qqml.h
{
    //Only cleans global static, assumed no running engine
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    for (QQmlMetaTypeData::TypeModules::const_iterator i = data->uriToModule.constBegin(), cend = data->uriToModule.constEnd(); i != cend; ++i)
        delete *i;

    data->types.clear();
    data->idToType.clear();
    data->nameToType.clear();
    data->urlToType.clear();
    data->urlToNonFileImportType.clear();
    data->metaObjectToType.clear();
    data->uriToModule.clear();
    data->undeletableTypes.clear();

    QQmlEnginePrivate::baseModulesUninitialized = true; //So the engine re-registers its types
#if QT_CONFIG(library)
    qmlClearEnginePlugins();
#endif
}

static void unregisterAutoParentFunction(const QQmlPrivate::AutoParentFunction &function)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    data->parentFunctions.removeOne(function);
}

static int registerAutoParentFunction(const QQmlPrivate::RegisterAutoParent &autoparent)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    data->parentFunctions.append(autoparent.function);

    return data->parentFunctions.count() - 1;
}

QQmlType registerInterface(const QQmlPrivate::RegisterInterface &interface)
{
    if (interface.version > 0)
        qFatal("qmlRegisterType(): Cannot mix incompatible QML versions.");

    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    QQmlType type(data, interface);
    QQmlTypePrivate *priv = type.priv();
    Q_ASSERT(priv);

    data->idToType.insert(priv->typeId, priv);
    data->idToType.insert(priv->listId, priv);
    // XXX No insertMulti, so no multi-version interfaces?
    if (!priv->elementName.isEmpty())
        data->nameToType.insert(priv->elementName, priv);

    if (data->interfaces.size() <= interface.typeId)
        data->interfaces.resize(interface.typeId + 16);
    if (data->lists.size() <= interface.listId)
        data->lists.resize(interface.listId + 16);
    data->interfaces.setBit(interface.typeId, true);
    data->lists.setBit(interface.listId, true);

    return type;
}

QString registrationTypeString(QQmlType::RegistrationType typeType)
{
    QString typeStr;
    if (typeType == QQmlType::CppType)
        typeStr = QStringLiteral("element");
    else if (typeType == QQmlType::SingletonType)
        typeStr = QStringLiteral("singleton type");
    else if (typeType == QQmlType::CompositeSingletonType)
        typeStr = QStringLiteral("composite singleton type");
    else
        typeStr = QStringLiteral("type");
    return typeStr;
}

// NOTE: caller must hold a QMutexLocker on "data"
bool checkRegistration(QQmlType::RegistrationType typeType, QQmlMetaTypeData *data, const char *uri, const QString &typeName, int majorVersion = -1)
{
    if (!typeName.isEmpty()) {
        if (typeName.at(0).isLower()) {
            QString failure(QCoreApplication::translate("qmlRegisterType", "Invalid QML %1 name \"%2\"; type names must begin with an uppercase letter"));
            data->recordTypeRegFailure(failure.arg(registrationTypeString(typeType)).arg(typeName));
            return false;
        }

        int typeNameLen = typeName.length();
        for (int ii = 0; ii < typeNameLen; ++ii) {
            if (!(typeName.at(ii).isLetterOrNumber() || typeName.at(ii) == '_')) {
                QString failure(QCoreApplication::translate("qmlRegisterType", "Invalid QML %1 name \"%2\""));
                data->recordTypeRegFailure(failure.arg(registrationTypeString(typeType)).arg(typeName));
                return false;
            }
        }
    }

    if (uri && !typeName.isEmpty()) {
        QString nameSpace = QString::fromUtf8(uri);

        if (data->typeRegistrationNamespace.isEmpty() && !nameSpace.isEmpty()) {
            // Is the target namespace protected against further registrations?
            if (data->protectedNamespaces.contains(nameSpace)) {
                QString failure(QCoreApplication::translate("qmlRegisterType",
                                                            "Cannot install %1 '%2' into protected namespace '%3'"));
                data->recordTypeRegFailure(failure.arg(registrationTypeString(typeType)).arg(typeName).arg(nameSpace));
                return false;
            }
        } else if (majorVersion >= 0) {
            QQmlMetaTypeData::VersionedUri versionedUri;
            versionedUri.uri = nameSpace;
            versionedUri.majorVersion = majorVersion;
            if (QQmlTypeModule* qqtm = data->uriToModule.value(versionedUri, 0)){
                if (QQmlTypeModulePrivate::get(qqtm)->locked){
                    QString failure(QCoreApplication::translate("qmlRegisterType",
                                                                "Cannot install %1 '%2' into protected module '%3' version '%4'"));
                    data->recordTypeRegFailure(failure.arg(registrationTypeString(typeType)).arg(typeName).arg(nameSpace).arg(majorVersion));
                    return false;
                }
            }
        }
    }

    return true;
}

// NOTE: caller must hold a QMutexLocker on "data"
QQmlTypeModule *getTypeModule(const QHashedString &uri, int majorVersion, QQmlMetaTypeData *data)
{
    QQmlMetaTypeData::VersionedUri versionedUri(uri, majorVersion);
    QQmlTypeModule *module = data->uriToModule.value(versionedUri);
    if (!module) {
        module = new QQmlTypeModule;
        module->d->uri = versionedUri;
        data->uriToModule.insert(versionedUri, module);
    }
    return module;
}

// NOTE: caller must hold a QMutexLocker on "data"
void addTypeToData(QQmlTypePrivate *type, QQmlMetaTypeData *data)
{
    Q_ASSERT(type);

    if (!type->elementName.isEmpty())
        data->nameToType.insertMulti(type->elementName, type);

    if (type->baseMetaObject)
        data->metaObjectToType.insertMulti(type->baseMetaObject, type);

    if (type->typeId) {
        data->idToType.insert(type->typeId, type);
        if (data->objects.size() <= type->typeId)
            data->objects.resize(type->typeId + 16);
        data->objects.setBit(type->typeId, true);
    }

    if (type->listId) {
        if (data->lists.size() <= type->listId)
            data->lists.resize(type->listId + 16);
        data->lists.setBit(type->listId, true);
        data->idToType.insert(type->listId, type);
    }

    if (!type->module.isEmpty()) {
        const QHashedString &mod = type->module;

        QQmlTypeModule *module = getTypeModule(mod, type->version_maj, data);
        Q_ASSERT(module);
        module->d->add(type);
    }
}

QQmlType registerType(const QQmlPrivate::RegisterType &type)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    QString elementName = QString::fromUtf8(type.elementName);
    if (!checkRegistration(QQmlType::CppType, data, type.uri, elementName, type.versionMajor))
        return QQmlType();

    QQmlType dtype(data, elementName, type);

    addTypeToData(dtype.priv(), data);
    if (!type.typeId)
        data->idToType.insert(dtype.typeId(), dtype.priv());

    return dtype;
}

QQmlType registerSingletonType(const QQmlPrivate::RegisterSingletonType &type)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    QString typeName = QString::fromUtf8(type.typeName);
    if (!checkRegistration(QQmlType::SingletonType, data, type.uri, typeName, type.versionMajor))
        return QQmlType();

    QQmlType dtype(data, typeName, type);

    addTypeToData(dtype.priv(), data);

    return dtype;
}

QQmlType QQmlMetaType::registerCompositeSingletonType(const QQmlPrivate::RegisterCompositeSingletonType &type)
{
    // Assumes URL is absolute and valid. Checking of user input should happen before the URL enters type.
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    QString typeName = QString::fromUtf8(type.typeName);
    bool fileImport = false;
    if (*(type.uri) == '\0')
        fileImport = true;
    if (!checkRegistration(QQmlType::CompositeSingletonType, data, fileImport ? nullptr : type.uri, typeName))
        return QQmlType();

    QQmlType dtype(data, typeName, type);

    addTypeToData(dtype.priv(), data);

    QQmlMetaTypeData::Files *files = fileImport ? &(data->urlToType) : &(data->urlToNonFileImportType);
    files->insertMulti(QQmlTypeLoader::normalize(type.url), dtype.priv());

    return dtype;
}

QQmlType QQmlMetaType::registerCompositeType(const QQmlPrivate::RegisterCompositeType &type)
{
    // Assumes URL is absolute and valid. Checking of user input should happen before the URL enters type.
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    QString typeName = QString::fromUtf8(type.typeName);
    bool fileImport = false;
    if (*(type.uri) == '\0')
        fileImport = true;
    if (!checkRegistration(QQmlType::CompositeType, data, fileImport?nullptr:type.uri, typeName, type.versionMajor))
        return QQmlType();

    QQmlType dtype(data, typeName, type);
    addTypeToData(dtype.priv(), data);

    QQmlMetaTypeData::Files *files = fileImport ? &(data->urlToType) : &(data->urlToNonFileImportType);
    files->insertMulti(QQmlTypeLoader::normalize(type.url), dtype.priv());

    return dtype;
}

void QQmlMetaType::registerInternalCompositeType(QV4::CompiledData::CompilationUnit *compilationUnit)
{
    QByteArray name = compilationUnit->rootPropertyCache()->className();

    QByteArray ptr = name + '*';
    QByteArray lst = "QQmlListProperty<" + name + '>';

    int ptr_type = QMetaType::registerNormalizedType(ptr,
                                                     QtMetaTypePrivate::QMetaTypeFunctionHelper<QObject*>::Destruct,
                                                     QtMetaTypePrivate::QMetaTypeFunctionHelper<QObject*>::Construct,
                                                     sizeof(QObject*),
                                                     static_cast<QFlags<QMetaType::TypeFlag> >(QtPrivate::QMetaTypeTypeFlags<QObject*>::Flags),
                                                     nullptr);
    int lst_type = QMetaType::registerNormalizedType(lst,
                                                     QtMetaTypePrivate::QMetaTypeFunctionHelper<QQmlListProperty<QObject> >::Destruct,
                                                     QtMetaTypePrivate::QMetaTypeFunctionHelper<QQmlListProperty<QObject> >::Construct,
                                                     sizeof(QQmlListProperty<QObject>),
                                                     static_cast<QFlags<QMetaType::TypeFlag> >(QtPrivate::QMetaTypeTypeFlags<QQmlListProperty<QObject> >::Flags),
                                                     static_cast<QMetaObject*>(nullptr));

    compilationUnit->metaTypeId = ptr_type;
    compilationUnit->listMetaTypeId = lst_type;

    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *d = metaTypeData();
    d->qmlLists.insert(lst_type, ptr_type);
}

void QQmlMetaType::unregisterInternalCompositeType(QV4::CompiledData::CompilationUnit *compilationUnit)
{
    int ptr_type = compilationUnit->metaTypeId;
    int lst_type = compilationUnit->listMetaTypeId;

    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *d = metaTypeData();
    d->qmlLists.remove(lst_type);

    QMetaType::unregisterType(ptr_type);
    QMetaType::unregisterType(lst_type);
}

int registerQmlUnitCacheHook(const QQmlPrivate::RegisterQmlUnitCacheHook &hookRegistration)
{
    if (hookRegistration.version > 0)
        qFatal("qmlRegisterType(): Cannot mix incompatible QML versions.");
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    data->lookupCachedQmlUnit << hookRegistration.lookupCachedQmlUnit;
    return 0;
}

/*
This method is "over generalized" to allow us to (potentially) register more types of things in
the future without adding exported symbols.
*/
int QQmlPrivate::qmlregister(RegistrationType type, void *data)
{
    if (type == AutoParentRegistration)
        return registerAutoParentFunction(*reinterpret_cast<RegisterAutoParent *>(data));
    else if (type == QmlUnitCacheHookRegistration)
        return registerQmlUnitCacheHook(*reinterpret_cast<RegisterQmlUnitCacheHook *>(data));

    QQmlType dtype;
    if (type == TypeRegistration)
        dtype = registerType(*reinterpret_cast<RegisterType *>(data));
    else if (type == InterfaceRegistration)
        dtype = registerInterface(*reinterpret_cast<RegisterInterface *>(data));
    else if (type == SingletonRegistration)
        dtype = registerSingletonType(*reinterpret_cast<RegisterSingletonType *>(data));
    else if (type == CompositeRegistration)
        dtype = QQmlMetaType::registerCompositeType(*reinterpret_cast<RegisterCompositeType *>(data));
    else if (type == CompositeSingletonRegistration)
        dtype = QQmlMetaType::registerCompositeSingletonType(*reinterpret_cast<RegisterCompositeSingletonType *>(data));
    else
        return -1;

    if (!dtype.isValid())
        return -1;

    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *typeData = metaTypeData();
    typeData->undeletableTypes.insert(dtype);

    return dtype.index();
}

void QQmlPrivate::qmlunregister(RegistrationType type, quintptr data)
{
    switch (type) {
        case AutoParentRegistration:
            unregisterAutoParentFunction(reinterpret_cast<AutoParentFunction>(data));
            break;
        case QmlUnitCacheHookRegistration:
            QQmlMetaType::removeCachedUnitLookupFunction(
                    reinterpret_cast<QmlUnitCacheLookupFunction>(data));
            break;
        case TypeRegistration:
        case InterfaceRegistration:
        case SingletonRegistration:
        case CompositeRegistration:
        case CompositeSingletonRegistration:
            qmlUnregisterType(data);
            break;
    }
}

//From qqml.h
bool qmlProtectModule(const char *uri, int majVersion)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    QQmlMetaTypeData::VersionedUri versionedUri;
    versionedUri.uri = QString::fromUtf8(uri);
    versionedUri.majorVersion = majVersion;

    if (QQmlTypeModule* qqtm = data->uriToModule.value(versionedUri, 0)) {
        QQmlTypeModulePrivate::get(qqtm)->locked = true;
        return true;
    }
    return false;
}

//From qqml.h
void qmlRegisterModule(const char *uri, int versionMajor, int versionMinor)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    QQmlTypeModule *module = getTypeModule(QString::fromUtf8(uri), versionMajor, data);
    Q_ASSERT(module);

    QQmlTypeModulePrivate *p = QQmlTypeModulePrivate::get(module);
    p->minMinorVersion = qMin(p->minMinorVersion, versionMinor);
    p->maxMinorVersion = qMax(p->maxMinorVersion, versionMinor);
}

//From qqml.h
int qmlTypeId(const char *uri, int versionMajor, int versionMinor, const char *qmlName)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    QQmlTypeModule *module = getTypeModule(QString::fromUtf8(uri), versionMajor, data);
    if (!module)
        return -1;

    QQmlType type = module->type(QHashedStringRef(QString::fromUtf8(qmlName)), versionMinor);
    if (!type.isValid())
        return -1;

    return type.index();
}

bool QQmlMetaType::namespaceContainsRegistrations(const QString &uri, int majorVersion)
{
    const QQmlMetaTypeData *data = metaTypeData();

    // Has any type previously been installed to this namespace?
    QHashedString nameSpace(uri);
    for (const QQmlType &type : data->types)
        if (type.module() == nameSpace && type.majorVersion() == majorVersion)
            return true;

    return false;
}

void QQmlMetaType::protectNamespace(const QString &uri)
{
    QQmlMetaTypeData *data = metaTypeData();

    data->protectedNamespaces.insert(uri);
}

void QQmlMetaType::setTypeRegistrationNamespace(const QString &uri)
{
    QQmlMetaTypeData *data = metaTypeData();

    data->typeRegistrationNamespace = uri;
}

QMutex *QQmlMetaType::typeRegistrationLock()
{
    return metaTypeDataLock();
}

/*
    Returns true if a module \a uri of any version is installed.
*/
bool QQmlMetaType::isAnyModule(const QString &uri)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    for (QQmlMetaTypeData::TypeModules::ConstIterator iter = data->uriToModule.cbegin();
         iter != data->uriToModule.cend(); ++iter) {
        if ((*iter)->module() == uri)
            return true;
    }

    return false;
}

/*
    Returns true if a module \a uri of this version is installed and locked;
*/
bool QQmlMetaType::isLockedModule(const QString &uri, int majVersion)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    QQmlMetaTypeData::VersionedUri versionedUri;
    versionedUri.uri = uri;
    versionedUri.majorVersion = majVersion;
    if (QQmlTypeModule* qqtm = data->uriToModule.value(versionedUri, 0))
        return QQmlTypeModulePrivate::get(qqtm)->locked;
    return false;
}

/*
    Returns true if any type or API has been registered for the given \a module with at least
    versionMajor.versionMinor, or if types have been registered for \a module with at most
    versionMajor.versionMinor.

    So if only 4.7 and 4.9 have been registered, 4.7,4.8, and 4.9 are valid, but not 4.6 nor 4.10.
*/
bool QQmlMetaType::isModule(const QString &module, int versionMajor, int versionMinor)
{
    Q_ASSERT(versionMajor >= 0 && versionMinor >= 0);
    QMutexLocker lock(metaTypeDataLock());

    QQmlMetaTypeData *data = metaTypeData();

    // first, check Types
    QQmlTypeModule *tm =
        data->uriToModule.value(QQmlMetaTypeData::VersionedUri(module, versionMajor));
    if (tm && tm->minimumMinorVersion() <= versionMinor && tm->maximumMinorVersion() >= versionMinor)
        return true;

    return false;
}

QQmlTypeModule *QQmlMetaType::typeModule(const QString &uri, int majorVersion)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    return data->uriToModule.value(QQmlMetaTypeData::VersionedUri(uri, majorVersion));
}

QList<QQmlPrivate::AutoParentFunction> QQmlMetaType::parentFunctions()
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    return data->parentFunctions;
}

QObject *QQmlMetaType::toQObject(const QVariant &v, bool *ok)
{
    if (!isQObject(v.userType())) {
        if (ok) *ok = false;
        return nullptr;
    }

    if (ok) *ok = true;

    return *(QObject *const *)v.constData();
}

bool QQmlMetaType::isQObject(int userType)
{
    if (userType == QMetaType::QObjectStar)
        return true;

    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    return userType >= 0 && userType < data->objects.size() && data->objects.testBit(userType);
}

/*
    Returns the item type for a list of type \a id.
 */
int QQmlMetaType::listType(int id)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    QHash<int, int>::ConstIterator iter = data->qmlLists.constFind(id);
    if (iter != data->qmlLists.cend())
        return *iter;
    QQmlTypePrivate *type = data->idToType.value(id);
    if (type && type->listId == id)
        return type->typeId;
    else
        return 0;
}

int QQmlMetaType::attachedPropertiesFuncId(QQmlEnginePrivate *engine, const QMetaObject *mo)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    QQmlType type(data->metaObjectToType.value(mo));
    if (type.attachedPropertiesFunction(engine))
        return type.attachedPropertiesId(engine);
    else
        return -1;
}

QQmlAttachedPropertiesFunc QQmlMetaType::attachedPropertiesFuncById(QQmlEnginePrivate *engine, int id)
{
    if (id < 0)
        return nullptr;
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    return data->types.at(id).attachedPropertiesFunction(engine);
}

QQmlAttachedPropertiesFunc QQmlMetaType::attachedPropertiesFunc(QQmlEnginePrivate *engine,
                                                                const QMetaObject *mo)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    QQmlType type(data->metaObjectToType.value(mo));
    return type.attachedPropertiesFunction(engine);
}

QMetaProperty QQmlMetaType::defaultProperty(const QMetaObject *metaObject)
{
    int idx = metaObject->indexOfClassInfo("DefaultProperty");
    if (-1 == idx)
        return QMetaProperty();

    QMetaClassInfo info = metaObject->classInfo(idx);
    if (!info.value())
        return QMetaProperty();

    idx = metaObject->indexOfProperty(info.value());
    if (-1 == idx)
        return QMetaProperty();

    return metaObject->property(idx);
}

QMetaProperty QQmlMetaType::defaultProperty(QObject *obj)
{
    if (!obj)
        return QMetaProperty();

    const QMetaObject *metaObject = obj->metaObject();
    return defaultProperty(metaObject);
}

QMetaMethod QQmlMetaType::defaultMethod(const QMetaObject *metaObject)
{
    int idx = metaObject->indexOfClassInfo("DefaultMethod");
    if (-1 == idx)
        return QMetaMethod();

    QMetaClassInfo info = metaObject->classInfo(idx);
    if (!info.value())
        return QMetaMethod();

    idx = metaObject->indexOfMethod(info.value());
    if (-1 == idx)
        return QMetaMethod();

    return metaObject->method(idx);
}

QMetaMethod QQmlMetaType::defaultMethod(QObject *obj)
{
    if (!obj)
        return QMetaMethod();

    const QMetaObject *metaObject = obj->metaObject();
    return defaultMethod(metaObject);
}

QQmlMetaType::TypeCategory QQmlMetaType::typeCategory(int userType)
{
    if (userType < 0)
        return Unknown;
    if (userType == QMetaType::QObjectStar)
        return Object;

    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    if (data->qmlLists.contains(userType))
        return List;
    else if (userType < data->objects.size() && data->objects.testBit(userType))
        return Object;
    else if (userType < data->lists.size() && data->lists.testBit(userType))
        return List;
    else
        return Unknown;
}

/*!
    See qmlRegisterInterface() for information about when this will return true.
*/
bool QQmlMetaType::isInterface(int userType)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    return userType >= 0 && userType < data->interfaces.size() && data->interfaces.testBit(userType);
}

const char *QQmlMetaType::interfaceIId(int userType)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    QQmlType type(data->idToType.value(userType));
    lock.unlock();
    if (type.isInterface() && type.typeId() == userType)
        return type.interfaceIId();
    else
        return nullptr;
}

bool QQmlMetaType::isList(int userType)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    if (data->qmlLists.contains(userType))
        return true;
    return userType >= 0 && userType < data->lists.size() && data->lists.testBit(userType);
}

/*!
    A custom string convertor allows you to specify a function pointer that
    returns a variant of \a type. For example, if you have written your own icon
    class that you want to support as an object property assignable in QML:

    \code
    int type = qRegisterMetaType<SuperIcon>("SuperIcon");
    QML::addCustomStringConvertor(type, &SuperIcon::pixmapFromString);
    \endcode

    The function pointer must be of the form:
    \code
    QVariant (*StringConverter)(const QString &);
    \endcode
 */
void QQmlMetaType::registerCustomStringConverter(int type, StringConverter converter)
{
    QMutexLocker lock(metaTypeDataLock());

    QQmlMetaTypeData *data = metaTypeData();
    if (data->stringConverters.contains(type))
        return;
    data->stringConverters.insert(type, converter);
}

/*!
    Return the custom string converter for \a type, previously installed through
    registerCustomStringConverter()
 */
QQmlMetaType::StringConverter QQmlMetaType::customStringConverter(int type)
{
    QMutexLocker lock(metaTypeDataLock());

    QQmlMetaTypeData *data = metaTypeData();
    return data->stringConverters.value(type);
}

/*!
    Returns the type (if any) of URI-qualified named \a qualifiedName and version specified
    by \a version_major and \a version_minor.
*/
QQmlType QQmlMetaType::qmlType(const QString &qualifiedName, int version_major, int version_minor)
{
    int slash = qualifiedName.indexOf(QLatin1Char('/'));
    if (slash <= 0)
        return QQmlType();

    QHashedStringRef module(qualifiedName.constData(), slash);
    QHashedStringRef name(qualifiedName.constData() + slash + 1, qualifiedName.length() - slash - 1);

    return qmlType(name, module, version_major, version_minor);
}

/*!
    Returns the type (if any) of \a name in \a module and version specified
    by \a version_major and \a version_minor.
*/
QQmlType QQmlMetaType::qmlType(const QHashedStringRef &name, const QHashedStringRef &module, int version_major, int version_minor)
{
    Q_ASSERT(version_major >= 0 && version_minor >= 0);
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    QQmlMetaTypeData::Names::ConstIterator it = data->nameToType.constFind(name);
    while (it != data->nameToType.cend() && it.key() == name) {
        QQmlType t(*it);
        // XXX version_major<0 just a kludge for QQmlPropertyPrivate::initProperty
        if (version_major < 0 || module.isEmpty() || t.availableInVersion(module, version_major,version_minor))
            return t;
        ++it;
    }

    return QQmlType();
}

/*!
    Returns the type (if any) that corresponds to the \a metaObject.  Returns null if no
    type is registered.
*/
QQmlType QQmlMetaType::qmlType(const QMetaObject *metaObject)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    return QQmlType(data->metaObjectToType.value(metaObject));
}

/*!
    Returns the type (if any) that corresponds to the \a metaObject in version specified
    by \a version_major and \a version_minor in module specified by \a uri.  Returns null if no
    type is registered.
*/
QQmlType QQmlMetaType::qmlType(const QMetaObject *metaObject, const QHashedStringRef &module, int version_major, int version_minor)
{
    Q_ASSERT(version_major >= 0 && version_minor >= 0);
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    QQmlMetaTypeData::MetaObjects::const_iterator it = data->metaObjectToType.constFind(metaObject);
    while (it != data->metaObjectToType.cend() && it.key() == metaObject) {
        QQmlType t(*it);
        if (version_major < 0 || module.isEmpty() || t.availableInVersion(module, version_major,version_minor))
            return t;
        ++it;
    }

    return QQmlType();
}

/*!
    Returns the type (if any) that corresponds to \a typeId.  Depending on \a category, the
    \a typeId is interpreted either as QVariant::Type or as QML type id returned by one of the
    qml type registration functions.  Returns null if no type is registered.
*/
QQmlType QQmlMetaType::qmlType(int typeId, TypeIdCategory category)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    if (category == TypeIdCategory::MetaType) {
        QQmlTypePrivate *type = data->idToType.value(typeId);
        if (type && type->typeId == typeId)
            return QQmlType(type);
    } else if (category == TypeIdCategory::QmlType) {
        QQmlType type = data->types.value(typeId);
        if (type.isValid())
            return type;
    }
    return QQmlType();
}

/*!
    Returns the type (if any) that corresponds to the given \a url in the set of
    composite types added through file imports.

    Returns null if no such type is registered.
*/
QQmlType QQmlMetaType::qmlType(const QUrl &unNormalizedUrl, bool includeNonFileImports /* = false */)
{
    const QUrl url = QQmlTypeLoader::normalize(unNormalizedUrl);
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    QQmlType type(data->urlToType.value(url));
    if (!type.isValid() && includeNonFileImports)
        type = QQmlType(data->urlToNonFileImportType.value(url));

    if (type.sourceUrl() == url)
        return type;
    else
        return QQmlType();
}

QQmlPropertyCache *QQmlMetaTypeData::propertyCache(const QMetaObject *metaObject, int minorVersion)
{
    if (QQmlPropertyCache *rv = propertyCaches.value(metaObject))
        return rv;

    if (!metaObject->superClass()) {
        QQmlPropertyCache *rv = new QQmlPropertyCache(metaObject);
        propertyCaches.insert(metaObject, rv);
        return rv;
    }
    QQmlPropertyCache *super = propertyCache(metaObject->superClass(), minorVersion);
    QQmlPropertyCache *rv = super->copyAndAppend(metaObject, minorVersion);
    propertyCaches.insert(metaObject, rv);
    return rv;
}

QQmlPropertyCache *QQmlMetaType::propertyCache(const QMetaObject *metaObject, int minorVersion)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    return data->propertyCache(metaObject, minorVersion);
}

QQmlPropertyCache *QQmlMetaTypeData::propertyCache(const QQmlType &type, int minorVersion)
{
    Q_ASSERT(type.isValid());

    if (QQmlPropertyCache *pc = type.key()->propertyCacheForMinorVersion(minorVersion))
        return pc;

    QVector<QQmlType> types;

    int maxMinorVersion = 0;

    const QMetaObject *metaObject = type.metaObject();

    while (metaObject) {
        QQmlType t = QQmlMetaType::qmlType(metaObject, type.module(), type.majorVersion(), minorVersion);
        if (t.isValid()) {
            maxMinorVersion = qMax(maxMinorVersion, t.minorVersion());
            types << t;
        } else {
            types << QQmlType();
        }

        metaObject = metaObject->superClass();
    }

    if (QQmlPropertyCache *pc = type.key()->propertyCacheForMinorVersion(maxMinorVersion)) {
        const_cast<QQmlTypePrivate*>(type.key())->setPropertyCacheForMinorVersion(minorVersion, pc);
        return pc;
    }

    QQmlPropertyCache *raw = propertyCache(type.metaObject(), minorVersion);

    bool hasCopied = false;

    for (int ii = 0; ii < types.count(); ++ii) {
        QQmlType currentType = types.at(ii);
        if (!currentType.isValid())
            continue;

        int rev = currentType.metaObjectRevision();
        int moIndex = types.count() - 1 - ii;

        if (raw->allowedRevisionCache[moIndex] != rev) {
            if (!hasCopied) {
                raw = raw->copy();
                hasCopied = true;
            }
            raw->allowedRevisionCache[moIndex] = rev;
        }
    }

    // Test revision compatibility - the basic rule is:
    //    * Anything that is excluded, cannot overload something that is not excluded *

    // Signals override:
    //    * other signals and methods of the same name.
    //    * properties named on<Signal Name>
    //    * automatic <property name>Changed notify signals

    // Methods override:
    //    * other methods of the same name

    // Properties override:
    //    * other elements of the same name

#if 0
    bool overloadError = false;
    QString overloadName;

    for (QQmlPropertyCache::StringCache::ConstIterator iter = raw->stringCache.begin();
         !overloadError && iter != raw->stringCache.end();
         ++iter) {

        QQmlPropertyData *d = *iter;
        if (raw->isAllowedInRevision(d))
            continue; // Not excluded - no problems

        // check that a regular "name" overload isn't happening
        QQmlPropertyData *current = d;
        while (!overloadError && current) {
            current = d->overrideData(current);
            if (current && raw->isAllowedInRevision(current))
                overloadError = true;
        }
    }

    if (overloadError) {
        if (hasCopied) raw->release();

        error.setDescription(QLatin1String("Type ") + type.qmlTypeName() + QLatin1Char(' ') + QString::number(type.majorVersion()) + QLatin1Char('.') + QString::number(minorVersion) + QLatin1String(" contains an illegal property \"") + overloadName + QLatin1String("\".  This is an error in the type's implementation."));
        return 0;
    }
#endif

    const_cast<QQmlTypePrivate*>(type.key())->setPropertyCacheForMinorVersion(minorVersion, raw);

    if (hasCopied)
        raw->release();

    if (minorVersion != maxMinorVersion)
        const_cast<QQmlTypePrivate*>(type.key())->setPropertyCacheForMinorVersion(maxMinorVersion, raw);

    return raw;
}

QQmlPropertyCache *QQmlMetaType::propertyCache(const QQmlType &type, int minorVersion)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    return data->propertyCache(type, minorVersion);
}

void qmlUnregisterType(int typeIndex)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    {
        const QQmlType type = data->types.value(typeIndex);
        const QQmlTypePrivate *d = type.priv();
        if (d) {
            removeQQmlTypePrivate(data->idToType, d);
            removeQQmlTypePrivate(data->nameToType, d);
            removeQQmlTypePrivate(data->urlToType, d);
            removeQQmlTypePrivate(data->urlToNonFileImportType, d);
            removeQQmlTypePrivate(data->metaObjectToType, d);
            for (QQmlMetaTypeData::TypeModules::Iterator module = data->uriToModule.begin(); module != data->uriToModule.end(); ++module) {
                 QQmlTypeModulePrivate *modulePrivate = (*module)->priv();
                 modulePrivate->remove(d);
            }
            data->types[typeIndex] = QQmlType();
            data->undeletableTypes.remove(type);
        }
    }
}

void QQmlMetaType::freeUnusedTypesAndCaches()
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    // in case this is being called during program exit, `data` might be destructed already
    if (!data)
        return;

    {
        bool deletedAtLeastOneType;
        do {
            deletedAtLeastOneType = false;
            QList<QQmlType>::Iterator it = data->types.begin();
            while (it != data->types.end()) {
                const QQmlTypePrivate *d = (*it).priv();
                if (d && d->refCount == 1) {
                    deletedAtLeastOneType = true;

                    removeQQmlTypePrivate(data->idToType, d);
                    removeQQmlTypePrivate(data->nameToType, d);
                    removeQQmlTypePrivate(data->urlToType, d);
                    removeQQmlTypePrivate(data->urlToNonFileImportType, d);
                    removeQQmlTypePrivate(data->metaObjectToType, d);

                    for (QQmlMetaTypeData::TypeModules::Iterator module = data->uriToModule.begin(); module != data->uriToModule.end(); ++module) {
                        QQmlTypeModulePrivate *modulePrivate = (*module)->priv();
                        modulePrivate->remove(d);
                    }

                    *it = QQmlType();
                } else {
                    ++it;
                }
            }
        } while (deletedAtLeastOneType);
    }

    {
        bool deletedAtLeastOneCache;
        do {
            deletedAtLeastOneCache = false;
            QHash<const QMetaObject *, QQmlPropertyCache *>::Iterator it = data->propertyCaches.begin();
            while (it != data->propertyCaches.end()) {

                if ((*it)->count() == 1) {
                    QQmlPropertyCache *pc = nullptr;
                    qSwap(pc, *it);
                    it = data->propertyCaches.erase(it);
                    pc->release();
                    deletedAtLeastOneCache = true;
                } else {
                    ++it;
                }
            }
        } while (deletedAtLeastOneCache);
    }
}

/*!
    Returns the list of registered QML type names.
*/
QList<QString> QQmlMetaType::qmlTypeNames()
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    QList<QString> names;
    names.reserve(data->nameToType.count());
    QQmlMetaTypeData::Names::ConstIterator it = data->nameToType.cbegin();
    while (it != data->nameToType.cend()) {
        QQmlType t(*it);
        names += t.qmlTypeName();
        ++it;
    }

    return names;
}

/*!
    Returns the list of registered QML types.
*/
QList<QQmlType> QQmlMetaType::qmlTypes()
{
    QMutexLocker lock(metaTypeDataLock());
    const QQmlMetaTypeData *data = metaTypeData();

    QList<QQmlType> types;
    for (QQmlTypePrivate *t : data->nameToType)
        types.append(QQmlType(t));

    return types;
}

/*!
    Returns the list of all registered types.
*/
QList<QQmlType> QQmlMetaType::qmlAllTypes()
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    return data->types;
}

/*!
    Returns the list of registered QML singleton types.
*/
QList<QQmlType> QQmlMetaType::qmlSingletonTypes()
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    QList<QQmlType> retn;
    for (const auto t : qAsConst(data->nameToType)) {
        QQmlType type(t);
        if (type.isSingleton())
            retn.append(type);
    }
    return retn;
}

const QV4::CompiledData::Unit *QQmlMetaType::findCachedCompilationUnit(const QUrl &uri, CachedUnitLookupError *status)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();

    for (const auto lookup : qAsConst(data->lookupCachedQmlUnit)) {
        if (const QQmlPrivate::CachedQmlUnit *unit = lookup(uri)) {
            QString error;
            if (!unit->qmlData->verifyHeader(QDateTime(), &error)) {
                qCDebug(DBG_DISK_CACHE) << "Error loading pre-compiled file " << uri << ":" << error;
                if (status)
                    *status = CachedUnitLookupError::VersionMismatch;
                return nullptr;
            }
            if (status)
                *status = CachedUnitLookupError::NoError;
            return unit->qmlData;
        }
    }

    if (status)
        *status = CachedUnitLookupError::NoUnitFound;

    return nullptr;
}

void QQmlMetaType::prependCachedUnitLookupFunction(QQmlPrivate::QmlUnitCacheLookupFunction handler)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    data->lookupCachedQmlUnit.prepend(handler);
}

void QQmlMetaType::removeCachedUnitLookupFunction(QQmlPrivate::QmlUnitCacheLookupFunction handler)
{
    QMutexLocker lock(metaTypeDataLock());
    QQmlMetaTypeData *data = metaTypeData();
    data->lookupCachedQmlUnit.removeAll(handler);
}

/*!
    Returns the pretty QML type name (e.g. 'Item' instead of 'QtQuickItem') for the given object.
 */
QString QQmlMetaType::prettyTypeName(const QObject *object)
{
    QString typeName;

    if (!object)
        return typeName;

    QQmlType type = QQmlMetaType::qmlType(object->metaObject());
    if (type.isValid()) {
        typeName = type.qmlTypeName();
        const int lastSlash = typeName.lastIndexOf(QLatin1Char('/'));
        if (lastSlash != -1)
            typeName = typeName.mid(lastSlash + 1);
    }

    if (typeName.isEmpty()) {
        typeName = QString::fromUtf8(object->metaObject()->className());
        int marker = typeName.indexOf(QLatin1String("_QMLTYPE_"));
        if (marker != -1)
            typeName = typeName.left(marker);

        marker = typeName.indexOf(QLatin1String("_QML_"));
        if (marker != -1) {
            typeName = typeName.leftRef(marker) + QLatin1Char('*');
            type = QQmlMetaType::qmlType(QMetaType::type(typeName.toLatin1()));
            if (type.isValid()) {
                QString qmlTypeName = type.qmlTypeName();
                const int lastSlash = qmlTypeName.lastIndexOf(QLatin1Char('/'));
                if (lastSlash != -1)
                    qmlTypeName = qmlTypeName.mid(lastSlash + 1);
                if (!qmlTypeName.isEmpty())
                    typeName = qmlTypeName;
            }
        }
    }

    return typeName;
}

QT_END_NAMESPACE
