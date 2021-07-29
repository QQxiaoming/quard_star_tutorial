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

#include "qqmltypewrapper_p.h"
#include <private/qv8engine_p.h>

#include <private/qqmlengine_p.h>
#include <private/qqmlcontext_p.h>

#include <private/qjsvalue_p.h>
#include <private/qv4functionobject_p.h>
#include <private/qv4objectproto_p.h>
#include <private/qv4qobjectwrapper_p.h>
#include <private/qv4identifiertable_p.h>
#include <private/qv4lookup_p.h>

QT_BEGIN_NAMESPACE

using namespace QV4;

DEFINE_OBJECT_VTABLE(QQmlTypeWrapper);
DEFINE_OBJECT_VTABLE(QQmlScopedEnumWrapper);

void Heap::QQmlTypeWrapper::init()
{
    Object::init();
    mode = IncludeEnums;
    object.init();
}

void Heap::QQmlTypeWrapper::destroy()
{
    QQmlType::derefHandle(typePrivate);
    typePrivate = nullptr;
    if (typeNamespace)
        typeNamespace->release();
    object.destroy();
    Object::destroy();
}

QQmlType Heap::QQmlTypeWrapper::type() const
{
    return QQmlType(typePrivate);
}

bool QQmlTypeWrapper::isSingleton() const
{
    return d()->type().isSingleton();
}

QObject* QQmlTypeWrapper::singletonObject() const
{
    if (!isSingleton())
        return nullptr;

    QQmlEngine *e = engine()->qmlEngine();
    QQmlType::SingletonInstanceInfo *siinfo = d()->type().singletonInstanceInfo();
    siinfo->init(e);
    return siinfo->qobjectApi(e);
}

QVariant QQmlTypeWrapper::toVariant() const
{
    if (!isSingleton())
        return QVariant::fromValue<QObject *>(d()->object);

    QQmlEngine *e = engine()->qmlEngine();
    QQmlType::SingletonInstanceInfo *siinfo = d()->type().singletonInstanceInfo();
    siinfo->init(e);
    if (QObject *qobjectSingleton = siinfo->qobjectApi(e))
        return QVariant::fromValue<QObject*>(qobjectSingleton);

    return QVariant::fromValue<QJSValue>(siinfo->scriptApi(e));
}


// Returns a type wrapper for type t on o.  This allows access of enums, and attached properties.
ReturnedValue QQmlTypeWrapper::create(QV4::ExecutionEngine *engine, QObject *o, const QQmlType &t,
                                     Heap::QQmlTypeWrapper::TypeNameMode mode)
{
    Q_ASSERT(t.isValid());
    Scope scope(engine);

    Scoped<QQmlTypeWrapper> w(scope, engine->memoryManager->allocate<QQmlTypeWrapper>());
    w->d()->mode = mode; w->d()->object = o;
    w->d()->typePrivate = t.priv();
    QQmlType::refHandle(w->d()->typePrivate);
    return w.asReturnedValue();
}

// Returns a type wrapper for importNamespace (of t) on o.  This allows nested resolution of a type in a
// namespace.
ReturnedValue QQmlTypeWrapper::create(QV4::ExecutionEngine *engine, QObject *o, const QQmlRefPointer<QQmlTypeNameCache> &t, const QQmlImportRef *importNamespace,
                                     Heap::QQmlTypeWrapper::TypeNameMode mode)
{
    Q_ASSERT(t);
    Q_ASSERT(importNamespace);
    Scope scope(engine);

    Scoped<QQmlTypeWrapper> w(scope, engine->memoryManager->allocate<QQmlTypeWrapper>());
    w->d()->mode = mode; w->d()->object = o; w->d()->typeNamespace = t.data(); w->d()->importNamespace = importNamespace;
    t->addref();
    return w.asReturnedValue();
}

static int enumForSingleton(QV4::ExecutionEngine *v4, String *name, QObject *qobjectSingleton,
                            const QQmlType &type, bool *ok)
{
    Q_ASSERT(ok != nullptr);
    int value = type.enumValue(QQmlEnginePrivate::get(v4->qmlEngine()), name, ok);
    if (*ok)
        return value;

    // ### Optimize
    QByteArray enumName = name->toQString().toUtf8();
    const QMetaObject *metaObject = qobjectSingleton->metaObject();
    for (int ii = metaObject->enumeratorCount() - 1; ii >= 0; --ii) {
        QMetaEnum e = metaObject->enumerator(ii);
        value = e.keyToValue(enumName.constData(), ok);
        if (*ok)
            return value;
    }
    *ok = false;
    return -1;
}

static ReturnedValue throwLowercaseEnumError(QV4::ExecutionEngine *v4, String *name, const QQmlType &type)
{
    const QString message =
            QStringLiteral("Cannot access enum value '%1' of '%2', enum values need to start with an uppercase letter.")
                .arg(name->toQString()).arg(QLatin1String(type.typeName()));
    return v4->throwTypeError(message);
}

ReturnedValue QQmlTypeWrapper::virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty)
{
    // Keep this code in sync with ::virtualResolveLookupGetter
    Q_ASSERT(m->as<QQmlTypeWrapper>());

    if (!id.isString())
        return Object::virtualGet(m, id, receiver, hasProperty);

    QV4::ExecutionEngine *v4 = static_cast<const QQmlTypeWrapper *>(m)->engine();
    QV4::Scope scope(v4);
    ScopedString name(scope, id.asStringOrSymbol());

    Scoped<QQmlTypeWrapper> w(scope, static_cast<const QQmlTypeWrapper *>(m));

    if (hasProperty)
        *hasProperty = true;

    QQmlContextData *context = v4->callingQmlContext();

    QObject *object = w->d()->object;
    QQmlType type = w->d()->type();

    if (type.isValid()) {

        // singleton types are handled differently to other types.
        if (type.isSingleton()) {
            QQmlEngine *e = v4->qmlEngine();
            QQmlType::SingletonInstanceInfo *siinfo = type.singletonInstanceInfo();
            siinfo->init(e);

            QObject *qobjectSingleton = siinfo->qobjectApi(e);
            if (qobjectSingleton) {

                // check for enum value
                const bool includeEnums = w->d()->mode == Heap::QQmlTypeWrapper::IncludeEnums;
                if (includeEnums && name->startsWithUpper()) {
                    bool ok = false;
                    int value = enumForSingleton(v4, name, qobjectSingleton, type, &ok);
                    if (ok)
                        return QV4::Value::fromInt32(value).asReturnedValue();

                    value = type.scopedEnumIndex(QQmlEnginePrivate::get(v4->qmlEngine()), name, &ok);
                    if (ok) {
                        Scoped<QQmlScopedEnumWrapper> enumWrapper(scope, v4->memoryManager->allocate<QQmlScopedEnumWrapper>());
                        enumWrapper->d()->typePrivate = type.priv();
                        QQmlType::refHandle(enumWrapper->d()->typePrivate);
                        enumWrapper->d()->scopeEnumIndex = value;
                        return enumWrapper.asReturnedValue();
                    }
                }

                // check for property.
                bool ok;
                const ReturnedValue result = QV4::QObjectWrapper::getQmlProperty(v4, context, qobjectSingleton, name, QV4::QObjectWrapper::IgnoreRevision, &ok);
                if (hasProperty)
                    *hasProperty = ok;

                // Warn when attempting to access a lowercased enum value, singleton case
                if (!ok && includeEnums && !name->startsWithUpper()) {
                    enumForSingleton(v4, name, qobjectSingleton, type, &ok);
                    if (ok)
                        return throwLowercaseEnumError(v4, name, type);
                }

                return result;
            } else if (!siinfo->scriptApi(e).isUndefined()) {
                // NOTE: if used in a binding, changes will not trigger re-evaluation since non-NOTIFYable.
                QV4::ScopedObject o(scope, QJSValuePrivate::convertedToValue(v4, siinfo->scriptApi(e)));
                if (!!o)
                    return o->get(name);
            }

            // Fall through to base implementation

        } else {

            if (name->startsWithUpper()) {
                bool ok = false;
                int value = type.enumValue(QQmlEnginePrivate::get(v4->qmlEngine()), name, &ok);
                if (ok)
                    return QV4::Value::fromInt32(value).asReturnedValue();

                value = type.scopedEnumIndex(QQmlEnginePrivate::get(v4->qmlEngine()), name, &ok);
                if (ok) {
                    Scoped<QQmlScopedEnumWrapper> enumWrapper(scope, v4->memoryManager->allocate<QQmlScopedEnumWrapper>());
                    enumWrapper->d()->typePrivate = type.priv();
                    QQmlType::refHandle(enumWrapper->d()->typePrivate);
                    enumWrapper->d()->scopeEnumIndex = value;
                    return enumWrapper.asReturnedValue();
                }

                // Fall through to base implementation

            } else if (w->d()->object) {
                QObject *ao = qmlAttachedPropertiesObject(
                        object,
                        type.attachedPropertiesFunction(QQmlEnginePrivate::get(v4->qmlEngine())));
                if (ao)
                    return QV4::QObjectWrapper::getQmlProperty(v4, context, ao, name, QV4::QObjectWrapper::IgnoreRevision, hasProperty);

                // Fall through to base implementation
            }

            // Fall through to base implementation
        }

        // Fall through to base implementation

    } else if (w->d()->typeNamespace) {
        Q_ASSERT(w->d()->importNamespace);
        QQmlTypeNameCache::Result r = w->d()->typeNamespace->query(name, w->d()->importNamespace);

        if (r.isValid()) {
            if (r.type.isValid()) {
                return create(scope.engine, object, r.type, w->d()->mode);
            } else if (r.scriptIndex != -1) {
                QV4::ScopedObject scripts(scope, context->importedScripts.valueRef());
                return scripts->get(r.scriptIndex);
            } else if (r.importNamespace) {
                return create(scope.engine, object, context->imports, r.importNamespace);
            }

            return QV4::Encode::undefined();

        }

        // Fall through to base implementation

    } else {
        Q_ASSERT(!"Unreachable");
    }

    bool ok = false;
    const ReturnedValue result = Object::virtualGet(m, id, receiver, &ok);
    if (hasProperty)
        *hasProperty = ok;

    // Warn when attempting to access a lowercased enum value, non-singleton case
    if (!ok && type.isValid() && !type.isSingleton() && !name->startsWithUpper()) {
        bool enumOk = false;
        type.enumValue(QQmlEnginePrivate::get(v4->qmlEngine()), name, &enumOk);
        if (enumOk)
            return throwLowercaseEnumError(v4, name, type);
    }

    return result;
}


bool QQmlTypeWrapper::virtualPut(Managed *m, PropertyKey id, const Value &value, Value *receiver)
{
    if (!id.isString())
        return Object::virtualPut(m, id, value, receiver);


    Q_ASSERT(m->as<QQmlTypeWrapper>());
    QQmlTypeWrapper *w = static_cast<QQmlTypeWrapper *>(m);
    QV4::Scope scope(w);
    if (scope.engine->hasException)
        return false;

    ScopedString name(scope, id.asStringOrSymbol());
    QQmlContextData *context = scope.engine->callingQmlContext();

    QQmlType type = w->d()->type();
    if (type.isValid() && !type.isSingleton() && w->d()->object) {
        QObject *object = w->d()->object;
        QQmlEngine *e = scope.engine->qmlEngine();
        QObject *ao = qmlAttachedPropertiesObject(
                object, type.attachedPropertiesFunction(QQmlEnginePrivate::get(e)));
        if (ao)
            return QV4::QObjectWrapper::setQmlProperty(scope.engine, context, ao, name, QV4::QObjectWrapper::IgnoreRevision, value);
        return false;
    } else if (type.isSingleton()) {
        QQmlEngine *e = scope.engine->qmlEngine();
        QQmlType::SingletonInstanceInfo *siinfo = type.singletonInstanceInfo();
        siinfo->init(e);

        QObject *qobjectSingleton = siinfo->qobjectApi(e);
        if (qobjectSingleton) {
            return QV4::QObjectWrapper::setQmlProperty(scope.engine, context, qobjectSingleton, name, QV4::QObjectWrapper::IgnoreRevision, value);
        } else if (!siinfo->scriptApi(e).isUndefined()) {
            QV4::ScopedObject apiprivate(scope, QJSValuePrivate::convertedToValue(scope.engine, siinfo->scriptApi(e)));
            if (!apiprivate) {
                QString error = QLatin1String("Cannot assign to read-only property \"") + name->toQString() + QLatin1Char('\"');
                scope.engine->throwError(error);
                return false;
            } else {
                return apiprivate->put(name, value);
            }
        }
    }

    return false;
}

PropertyAttributes QQmlTypeWrapper::virtualGetOwnProperty(const Managed *m, PropertyKey id, Property *p)
{
    if (id.isString()) {
        Scope scope(m);
        ScopedString n(scope, id.asStringOrSymbol());
        // ### Implement more efficiently.
        bool hasProperty = false;
        static_cast<const Object *>(m)->get(n, &hasProperty);
        return hasProperty ? Attr_Data : Attr_Invalid;
    }

    return QV4::Object::virtualGetOwnProperty(m, id, p);
}

bool QQmlTypeWrapper::virtualIsEqualTo(Managed *a, Managed *b)
{
    Q_ASSERT(a->as<QV4::QQmlTypeWrapper>());
    QV4::QQmlTypeWrapper *qmlTypeWrapperA = static_cast<QV4::QQmlTypeWrapper *>(a);
    if (QV4::QQmlTypeWrapper *qmlTypeWrapperB = b->as<QV4::QQmlTypeWrapper>())
        return qmlTypeWrapperA->toVariant() == qmlTypeWrapperB->toVariant();
    else if (QV4::QObjectWrapper *qobjectWrapper = b->as<QV4::QObjectWrapper>())
        return qmlTypeWrapperA->toVariant().value<QObject*>() == qobjectWrapper->object();

    return false;
}

ReturnedValue QQmlTypeWrapper::virtualInstanceOf(const Object *typeObject, const Value &var)
{
    Q_ASSERT(typeObject->as<QV4::QQmlTypeWrapper>());
    const QV4::QQmlTypeWrapper *typeWrapper = static_cast<const QV4::QQmlTypeWrapper *>(typeObject);
    QV4::ExecutionEngine *engine = typeObject->internalClass()->engine;
    QQmlEnginePrivate *qenginepriv = QQmlEnginePrivate::get(engine->qmlEngine());

    // can only compare a QObject* against a QML type
    const QObjectWrapper *wrapper = var.as<QObjectWrapper>();
    if (!wrapper)
        return engine->throwTypeError();

    // in case the wrapper outlived the QObject*
    const QObject *wrapperObject = wrapper->object();
    if (!wrapperObject)
        return engine->throwTypeError();

    const int myTypeId = typeWrapper->d()->type().typeId();
    QQmlMetaObject myQmlType;
    if (myTypeId == 0) {
        // we're a composite type; a composite type cannot be equal to a
        // non-composite object instance (Rectangle{} is never an instance of
        // CustomRectangle)
        QQmlData *theirDData = QQmlData::get(wrapperObject, /*create=*/false);
        Q_ASSERT(theirDData); // must exist, otherwise how do we have a QObjectWrapper for it?!
        if (!theirDData->compilationUnit)
            return Encode(false);

        QQmlRefPointer<QQmlTypeData> td = qenginepriv->typeLoader.getType(typeWrapper->d()->type().sourceUrl());
        CompiledData::CompilationUnit *cu = td->compilationUnit();
        myQmlType = qenginepriv->metaObjectForType(cu->metaTypeId);
    } else {
        myQmlType = qenginepriv->metaObjectForType(myTypeId);
    }

    const QMetaObject *theirType = wrapperObject->metaObject();

    return QV4::Encode(QQmlMetaObject::canConvert(theirType, myQmlType));
}

ReturnedValue QQmlTypeWrapper::virtualResolveLookupGetter(const Object *object, ExecutionEngine *engine, Lookup *lookup)
{
    // Keep this code in sync with ::virtualGet
    PropertyKey id = engine->identifierTable->asPropertyKey(engine->currentStackFrame->v4Function->compilationUnit->runtimeStrings[lookup->nameIndex]);
    if (!id.isString())
        return Object::virtualResolveLookupGetter(object, engine, lookup);
    Scope scope(engine);

    const QQmlTypeWrapper *This = static_cast<const QQmlTypeWrapper *>(object);
    ScopedString name(scope, id.asStringOrSymbol());
    QQmlContextData *qmlContext = engine->callingQmlContext();

    Scoped<QQmlTypeWrapper> w(scope, static_cast<const QQmlTypeWrapper *>(This));
    QQmlType type = w->d()->type();

    if (type.isValid()) {

        if (type.isSingleton()) {
            QQmlEngine *e = engine->qmlEngine();
            QQmlType::SingletonInstanceInfo *siinfo = type.singletonInstanceInfo();
            siinfo->init(e);

            QObject *qobjectSingleton = siinfo->qobjectApi(e);
            if (qobjectSingleton) {

                const bool includeEnums = w->d()->mode == Heap::QQmlTypeWrapper::IncludeEnums;
                if (!includeEnums || !name->startsWithUpper()) {
                    QQmlData *ddata = QQmlData::get(qobjectSingleton, false);
                    if (ddata && ddata->propertyCache) {
                        QQmlPropertyData *property = ddata->propertyCache->property(name.getPointer(), qobjectSingleton, qmlContext);
                        if (property) {
                            ScopedValue val(scope, Value::fromReturnedValue(QV4::QObjectWrapper::wrap(engine, qobjectSingleton)));
                            lookup->qobjectLookup.qmlTypeIc = This->internalClass();
                            lookup->qobjectLookup.ic = val->objectValue()->internalClass();
                            lookup->qobjectLookup.propertyCache = ddata->propertyCache;
                            lookup->qobjectLookup.propertyCache->addref();
                            lookup->qobjectLookup.propertyData = property;
                            lookup->getter = QQmlTypeWrapper::lookupSingletonProperty;
                            return lookup->getter(lookup, engine, *object);
                        }
                        // Fall through to base implementation
                    }
                    // Fall through to base implementation
                }
                // Fall through to base implementation
            }
            // Fall through to base implementation
        }
        // Fall through to base implementation
    }
    return QV4::Object::virtualResolveLookupGetter(object, engine, lookup);
}

bool QQmlTypeWrapper::virtualResolveLookupSetter(Object *object, ExecutionEngine *engine, Lookup *lookup, const Value &value)
{
    return Object::virtualResolveLookupSetter(object, engine, lookup, value);
}

ReturnedValue QQmlTypeWrapper::lookupSingletonProperty(Lookup *l, ExecutionEngine *engine, const Value &object)
{
    const auto revertLookup = [l, engine, &object]() {
        l->qobjectLookup.propertyCache->release();
        l->qobjectLookup.propertyCache = nullptr;
        l->getter = Lookup::getterGeneric;
        return Lookup::getterGeneric(l, engine, object);
    };

    // we can safely cast to a QV4::Object here. If object is something else,
    // the internal class won't match
    Heap::Object *o = static_cast<Heap::Object *>(object.heapObject());
    if (!o || o->internalClass != l->qobjectLookup.qmlTypeIc)
        return revertLookup();

    Heap::QQmlTypeWrapper *This = static_cast<Heap::QQmlTypeWrapper *>(o);

    QQmlType type = This->type();
    if (!type.isValid())
        return revertLookup();

    if (!type.isSingleton())
        return revertLookup();

    QQmlEngine *e = engine->qmlEngine();
    QQmlType::SingletonInstanceInfo *siinfo = type.singletonInstanceInfo();
    siinfo->init(e);

    QObject *qobjectSingleton = siinfo->qobjectApi(e);
    if (!qobjectSingleton)
        return revertLookup();

    Scope scope(engine);
    ScopedValue obj(scope, QV4::QObjectWrapper::wrap(engine, qobjectSingleton));
    return QObjectWrapper::lookupGetterImpl(l, engine, obj, /*useOriginalProperty*/ true, revertLookup);
}

void Heap::QQmlScopedEnumWrapper::destroy()
{
    QQmlType::derefHandle(typePrivate);
    typePrivate = nullptr;
    Object::destroy();
}

QQmlType Heap::QQmlScopedEnumWrapper::type() const
{
    return QQmlType(typePrivate);
}

ReturnedValue QQmlScopedEnumWrapper::virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty)
{
    Q_ASSERT(m->as<QQmlScopedEnumWrapper>());
    if (!id.isString())
        return Object::virtualGet(m, id, receiver, hasProperty);

    const QQmlScopedEnumWrapper *resource = static_cast<const QQmlScopedEnumWrapper *>(m);
    QV4::ExecutionEngine *v4 = resource->engine();
    QV4::Scope scope(v4);
    ScopedString name(scope, id.asStringOrSymbol());

    QQmlType type = resource->d()->type();
    int index = resource->d()->scopeEnumIndex;

    bool ok = false;
    int value = type.scopedEnumValue(QQmlEnginePrivate::get(v4->qmlEngine()), index, name, &ok);
    if (hasProperty)
        *hasProperty = ok;
    if (ok)
        return QV4::Value::fromInt32(value).asReturnedValue();

    return Encode::undefined();
}

QT_END_NAMESPACE
