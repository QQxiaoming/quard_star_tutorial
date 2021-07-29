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

#ifndef QQMLMETATYPE_P_H
#define QQMLMETATYPE_P_H

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

#include "qqml.h"
#include <private/qtqmlglobal_p.h>

#include <QtCore/qglobal.h>
#include <QtCore/qvariant.h>
#include <QtCore/qbitarray.h>
#include <QtQml/qjsvalue.h>

QT_BEGIN_NAMESPACE

class QQmlType;
class QQmlEngine;
class QQmlEnginePrivate;
class QQmlCustomParser;
class QQmlTypePrivate;
class QQmlTypeModule;
class QHashedString;
class QHashedStringRef;
class QMutex;
class QQmlPropertyCache;
class QQmlCompiledData;

namespace QV4 { struct String; }

void Q_QML_PRIVATE_EXPORT qmlUnregisterType(int type);

class Q_QML_PRIVATE_EXPORT QQmlMetaType
{
public:
    static QQmlType registerCompositeSingletonType(const QQmlPrivate::RegisterCompositeSingletonType &type);
    static QQmlType registerCompositeType(const QQmlPrivate::RegisterCompositeType &type);

    static void registerInternalCompositeType(QV4::CompiledData::CompilationUnit *compilationUnit);
    static void unregisterInternalCompositeType(QV4::CompiledData::CompilationUnit *compilationUnit);

    static QList<QString> qmlTypeNames();
    static QList<QQmlType> qmlTypes();
    static QList<QQmlType> qmlSingletonTypes();
    static QList<QQmlType> qmlAllTypes();

    enum class TypeIdCategory {
        MetaType,
        QmlType
    };

    static QQmlType qmlType(const QString &qualifiedName, int, int);
    static QQmlType qmlType(const QHashedStringRef &name, const QHashedStringRef &module, int, int);
    static QQmlType qmlType(const QMetaObject *);
    static QQmlType qmlType(const QMetaObject *metaObject, const QHashedStringRef &module, int version_major, int version_minor);
    static QQmlType qmlType(int typeId, TypeIdCategory category = TypeIdCategory::MetaType);
    static QQmlType qmlType(const QUrl &unNormalizedUrl, bool includeNonFileImports = false);

    static QQmlPropertyCache *propertyCache(const QMetaObject *metaObject, int minorVersion = -1);
    static QQmlPropertyCache *propertyCache(const QQmlType &type, int minorVersion);

    static void freeUnusedTypesAndCaches();

    static QMetaProperty defaultProperty(const QMetaObject *);
    static QMetaProperty defaultProperty(QObject *);
    static QMetaMethod defaultMethod(const QMetaObject *);
    static QMetaMethod defaultMethod(QObject *);

    static bool isQObject(int);
    static QObject *toQObject(const QVariant &, bool *ok = nullptr);

    static int listType(int);
    static int attachedPropertiesFuncId(QQmlEnginePrivate *engine, const QMetaObject *);
    static QQmlAttachedPropertiesFunc attachedPropertiesFuncById(QQmlEnginePrivate *, int);
    static QQmlAttachedPropertiesFunc attachedPropertiesFunc(QQmlEnginePrivate *,
                                                             const QMetaObject *);

    enum TypeCategory { Unknown, Object, List };
    static TypeCategory typeCategory(int);

    static bool isInterface(int);
    static const char *interfaceIId(int);
    static bool isList(int);

    typedef QVariant (*StringConverter)(const QString &);
    static void registerCustomStringConverter(int, StringConverter);
    static StringConverter customStringConverter(int);

    static bool isAnyModule(const QString &uri);
    static bool isLockedModule(const QString &uri, int majorVersion);
    static bool isModule(const QString &module, int versionMajor, int versionMinor);
    static QQmlTypeModule *typeModule(const QString &uri, int majorVersion);

    static QList<QQmlPrivate::AutoParentFunction> parentFunctions();

    enum class CachedUnitLookupError {
        NoError,
        NoUnitFound,
        VersionMismatch
    };

    static const QV4::CompiledData::Unit *findCachedCompilationUnit(const QUrl &uri, CachedUnitLookupError *status);

    // used by tst_qqmlcachegen.cpp
    static void prependCachedUnitLookupFunction(QQmlPrivate::QmlUnitCacheLookupFunction handler);
    static void removeCachedUnitLookupFunction(QQmlPrivate::QmlUnitCacheLookupFunction handler);

    static bool namespaceContainsRegistrations(const QString &, int majorVersion);

    static void protectNamespace(const QString &);

    static void setTypeRegistrationNamespace(const QString &);

    static QMutex *typeRegistrationLock();

    static QString prettyTypeName(const QObject *object);
};

struct QQmlMetaTypeData;
class QHashedCStringRef;
class QQmlPropertyCache;
class Q_QML_PRIVATE_EXPORT QQmlType
{
public:
    QQmlType();
    QQmlType(const QQmlType &other);
    QQmlType &operator =(const QQmlType &other);
    explicit QQmlType(QQmlTypePrivate *priv);
    ~QQmlType();

    bool operator ==(const QQmlType &other) const {
        return d == other.d;
    }

    bool isValid() const { return d != nullptr; }
    const QQmlTypePrivate *key() const { return d; }

    QByteArray typeName() const;
    QString qmlTypeName() const;
    QString elementName() const;

    QHashedString module() const;
    int majorVersion() const;
    int minorVersion() const;

    bool availableInVersion(int vmajor, int vminor) const;
    bool availableInVersion(const QHashedStringRef &module, int vmajor, int vminor) const;

    QObject *create() const;
    void create(QObject **, void **, size_t) const;

    typedef void (*CreateFunc)(void *);
    CreateFunc createFunction() const;
    int createSize() const;

    QQmlCustomParser *customParser() const;

    bool isCreatable() const;
    typedef QObject *(*ExtensionFunc)(QObject *);
    ExtensionFunc extensionFunction() const;
    bool isExtendedType() const;
    QString noCreationReason() const;

    bool isSingleton() const;
    bool isInterface() const;
    bool isComposite() const;
    bool isCompositeSingleton() const;

    int typeId() const;
    int qListTypeId() const;

    const QMetaObject *metaObject() const;
    const QMetaObject *baseMetaObject() const;
    int metaObjectRevision() const;
    bool containsRevisionedAttributes() const;

    QQmlAttachedPropertiesFunc attachedPropertiesFunction(QQmlEnginePrivate *engine) const;
    const QMetaObject *attachedPropertiesType(QQmlEnginePrivate *engine) const;
    int attachedPropertiesId(QQmlEnginePrivate *engine) const;

    int parserStatusCast() const;
    const char *interfaceIId() const;
    int propertyValueSourceCast() const;
    int propertyValueInterceptorCast() const;

    int index() const;

    class Q_QML_PRIVATE_EXPORT SingletonInstanceInfo
    {
    public:
        SingletonInstanceInfo()
            : scriptCallback(nullptr), qobjectCallback(nullptr), instanceMetaObject(nullptr) {}

        QJSValue (*scriptCallback)(QQmlEngine *, QJSEngine *);
        QObject *(*qobjectCallback)(QQmlEngine *, QJSEngine *);
        const QMetaObject *instanceMetaObject;
        QString typeName;
        QUrl url; // used by composite singletons

        void setQObjectApi(QQmlEngine *, QObject *);
        QObject *qobjectApi(QQmlEngine *) const;
        void setScriptApi(QQmlEngine *, const QJSValue &);
        QJSValue scriptApi(QQmlEngine *) const;

        void init(QQmlEngine *);
        void destroy(QQmlEngine *);

        QHash<QQmlEngine *, QJSValue> scriptApis;
        QHash<QQmlEngine *, QObject *> qobjectApis;
    };
    SingletonInstanceInfo *singletonInstanceInfo() const;

    QUrl sourceUrl() const;

    int enumValue(QQmlEnginePrivate *engine, const QHashedStringRef &, bool *ok) const;
    int enumValue(QQmlEnginePrivate *engine, const QHashedCStringRef &, bool *ok) const;
    int enumValue(QQmlEnginePrivate *engine, const QV4::String *, bool *ok) const;

    int scopedEnumIndex(QQmlEnginePrivate *engine, const QV4::String *, bool *ok) const;
    int scopedEnumIndex(QQmlEnginePrivate *engine, const QString &, bool *ok) const;
    int scopedEnumValue(QQmlEnginePrivate *engine, int index, const QV4::String *, bool *ok) const;
    int scopedEnumValue(QQmlEnginePrivate *engine, int index, const QString &, bool *ok) const;
    int scopedEnumValue(QQmlEnginePrivate *engine, const QByteArray &, const QByteArray &, bool *ok) const;
    int scopedEnumValue(QQmlEnginePrivate *engine, const QStringRef &, const QStringRef &, bool *ok) const;

    QQmlTypePrivate *priv() const { return d; }
    static void refHandle(QQmlTypePrivate *priv);
    static void derefHandle(QQmlTypePrivate *priv);
    static int refCount(QQmlTypePrivate *priv);

    enum RegistrationType {
        CppType = 0,
        SingletonType = 1,
        InterfaceType = 2,
        CompositeType = 3,
        CompositeSingletonType = 4,
        AnyRegistrationType = 255
    };

private:
    QQmlType superType() const;
    QQmlType resolveCompositeBaseType(QQmlEnginePrivate *engine) const;
    int resolveCompositeEnumValue(QQmlEnginePrivate *engine, const QString &name, bool *ok) const;
    QQmlPropertyCache *compositePropertyCache(QQmlEnginePrivate *engine) const;
    friend class QQmlTypePrivate;

    friend QString registrationTypeString(RegistrationType);
    friend bool checkRegistration(RegistrationType, QQmlMetaTypeData *, const char *, const QString &, int);
    friend QQmlType registerType(const QQmlPrivate::RegisterType &);
    friend QQmlType registerSingletonType(const QQmlPrivate::RegisterSingletonType &);
    friend QQmlType registerInterface(const QQmlPrivate::RegisterInterface &);
    friend int registerQmlUnitCacheHook(const QQmlPrivate::RegisterQmlUnitCacheHook &);
    friend uint qHash(const QQmlType &t, uint seed);
    friend Q_QML_EXPORT void qmlClearTypeRegistrations();
    friend class QQmlMetaType;

    QQmlType(QQmlMetaTypeData *data, const QQmlPrivate::RegisterInterface &);
    QQmlType(QQmlMetaTypeData *data, const QString &, const QQmlPrivate::RegisterSingletonType &);
    QQmlType(QQmlMetaTypeData *data, const QString &, const QQmlPrivate::RegisterType &);
    QQmlType(QQmlMetaTypeData *data, const QString &, const QQmlPrivate::RegisterCompositeType &);
    QQmlType(QQmlMetaTypeData *data, const QString &, const QQmlPrivate::RegisterCompositeSingletonType &);

    QQmlTypePrivate *d;
};

Q_DECLARE_TYPEINFO(QQmlMetaType, Q_MOVABLE_TYPE);


inline uint qHash(const QQmlType &t, uint seed = 0) { return qHash(reinterpret_cast<quintptr>(t.d), seed); }


class QQmlTypeModulePrivate;
class QQmlTypeModule
{
public:
    QString module() const;
    int majorVersion() const;

    int minimumMinorVersion() const;
    int maximumMinorVersion() const;

    QQmlType type(const QHashedStringRef &, int) const;
    QQmlType type(const QV4::String *, int) const;

    void walkCompositeSingletons(const std::function<void(const QQmlType &)> &callback) const;

    QQmlTypeModulePrivate *priv() { return d; }
private:
    //Used by register functions and creates the QQmlTypeModule for them
    friend QQmlTypeModule *getTypeModule(const QHashedString &uri, int majorVersion, QQmlMetaTypeData *data);
    friend void addTypeToData(QQmlTypePrivate *type, QQmlMetaTypeData *data);
    friend struct QQmlMetaTypeData;
    friend Q_QML_EXPORT void qmlClearTypeRegistrations();
    friend class QQmlTypeModulePrivate;

    QQmlTypeModule();
    ~QQmlTypeModule();
    QQmlTypeModulePrivate *d;
};

class QQmlTypeModuleVersion
{
public:
    QQmlTypeModuleVersion();
    QQmlTypeModuleVersion(QQmlTypeModule *, int);
    QQmlTypeModuleVersion(const QQmlTypeModuleVersion &);
    QQmlTypeModuleVersion &operator=(const QQmlTypeModuleVersion &);

    QQmlTypeModule *module() const;
    int minorVersion() const;

    QQmlType type(const QHashedStringRef &) const;
    QQmlType type(const QV4::String *) const;

private:
    QQmlTypeModule *m_module;
    int m_minor;
};

class Q_AUTOTEST_EXPORT QQmlMetaTypeRegistrationFailureRecorder
{
    QStringList _failures;

public:
    QQmlMetaTypeRegistrationFailureRecorder();
    ~QQmlMetaTypeRegistrationFailureRecorder();

    QStringList failures() const
    { return _failures; }
};

QT_END_NAMESPACE

#endif // QQMLMETATYPE_P_H

