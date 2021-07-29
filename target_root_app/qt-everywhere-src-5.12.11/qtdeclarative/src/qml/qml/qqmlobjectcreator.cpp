/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include "qqmlobjectcreator_p.h"

#include <private/qqmlengine_p.h>
#include <private/qqmlvmemetaobject_p.h>
#include <private/qv4function_p.h>
#include <private/qv4functionobject_p.h>
#include <private/qv4qobjectwrapper_p.h>
#include <private/qqmlbinding_p.h>
#include <private/qqmlstringconverters_p.h>
#include <private/qqmlboundsignal_p.h>
#include <private/qqmlcomponentattached_p.h>
#include <private/qqmlcomponent_p.h>
#include <private/qqmlcustomparser_p.h>
#include <private/qqmlscriptstring_p.h>
#include <private/qqmlpropertyvalueinterceptor_p.h>
#include <private/qqmlvaluetypeproxybinding_p.h>
#include <private/qqmldebugconnector_p.h>
#include <private/qqmldebugserviceinterfaces_p.h>
#include <private/qjsvalue_p.h>

#include <qtqml_tracepoints_p.h>

QT_USE_NAMESPACE

namespace {
struct ActiveOCRestorer
{
    ActiveOCRestorer(QQmlObjectCreator *creator, QQmlEnginePrivate *ep)
    : ep(ep), oldCreator(ep->activeObjectCreator) { ep->activeObjectCreator = creator; }
    ~ActiveOCRestorer() { ep->activeObjectCreator = oldCreator; }

    QQmlEnginePrivate *ep;
    QQmlObjectCreator *oldCreator;
};
}

QQmlObjectCreator::QQmlObjectCreator(QQmlContextData *parentContext, const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &compilationUnit, QQmlContextData *creationContext,
                                     QQmlIncubatorPrivate *incubator)
    : phase(Startup)
    , compilationUnit(compilationUnit)
    , propertyCaches(&compilationUnit->propertyCaches)
    , sharedState(new QQmlObjectCreatorSharedState)
    , topLevelCreator(true)
    , incubator(incubator)
{
    init(parentContext);

    sharedState->componentAttached = nullptr;
    sharedState->allCreatedBindings.allocate(compilationUnit->totalBindingsCount);
    sharedState->allParserStatusCallbacks.allocate(compilationUnit->totalParserStatusCount);
    sharedState->allCreatedObjects.allocate(compilationUnit->totalObjectCount);
    sharedState->allJavaScriptObjects = nullptr;
    sharedState->creationContext = creationContext;
    sharedState->rootContext = nullptr;

    if (auto profiler = QQmlEnginePrivate::get(engine)->profiler) {
        Q_QML_PROFILE_IF_ENABLED(QQmlProfilerDefinitions::ProfileCreating, profiler,
                sharedState->profiler.init(profiler, compilationUnit->totalParserStatusCount));
    } else {
        Q_UNUSED(profiler);
    }
}

QQmlObjectCreator::QQmlObjectCreator(QQmlContextData *parentContext, const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &compilationUnit, QQmlObjectCreatorSharedState *inheritedSharedState)
    : phase(Startup)
    , compilationUnit(compilationUnit)
    , propertyCaches(&compilationUnit->propertyCaches)
    , sharedState(inheritedSharedState)
    , topLevelCreator(false)
    , incubator(nullptr)
{
    init(parentContext);
}

void QQmlObjectCreator::init(QQmlContextData *providedParentContext)
{
    parentContext = providedParentContext;
    engine = parentContext->engine;
    v4 = engine->handle();

    if (compilationUnit && !compilationUnit->engine)
        compilationUnit->linkToEngine(v4);

    qmlUnit = compilationUnit->unitData();
    context = nullptr;
    _qobject = nullptr;
    _scopeObject = nullptr;
    _bindingTarget = nullptr;
    _valueTypeProperty = nullptr;
    _compiledObject = nullptr;
    _compiledObjectIndex = -1;
    _ddata = nullptr;
    _propertyCache = nullptr;
    _vmeMetaObject = nullptr;
    _qmlContext = nullptr;
}

QQmlObjectCreator::~QQmlObjectCreator()
{
    if (topLevelCreator) {
        {
            QQmlObjectCreatorRecursionWatcher watcher(this);
        }
        for (int i = 0; i < sharedState->allParserStatusCallbacks.count(); ++i) {
            QQmlParserStatus *ps = sharedState->allParserStatusCallbacks.at(i);
            if (ps)
                ps->d = nullptr;
        }
        while (sharedState->componentAttached) {
            QQmlComponentAttached *a = sharedState->componentAttached;
            a->rem();
        }
    }
}

QObject *QQmlObjectCreator::create(int subComponentIndex, QObject *parent, QQmlInstantiationInterrupt *interrupt)
{
    if (phase == CreatingObjectsPhase2) {
        phase = ObjectsCreated;
        return context->contextObject;
    }
    Q_ASSERT(phase == Startup);
    phase = CreatingObjects;

    int objectToCreate;

    if (subComponentIndex == -1) {
        objectToCreate = /*root object*/0;
    } else {
        const QV4::CompiledData::Object *compObj = compilationUnit->objectAt(subComponentIndex);
        objectToCreate = compObj->bindingTable()->value.objectIndex;
    }

    context = new QQmlContextData;
    context->isInternal = true;
    context->imports = compilationUnit->typeNameCache;
    context->initFromTypeCompilationUnit(compilationUnit, subComponentIndex);
    context->setParent(parentContext);

    if (!sharedState->rootContext) {
        sharedState->rootContext = context;
        sharedState->rootContext->incubator = incubator;
        sharedState->rootContext->isRootObjectInCreation = true;
    }

    QV4::Scope scope(v4);

    Q_ASSERT(sharedState->allJavaScriptObjects || topLevelCreator);
    if (topLevelCreator)
        sharedState->allJavaScriptObjects = scope.alloc(compilationUnit->totalObjectCount);

    if (subComponentIndex == -1 && compilationUnit->dependentScripts.count()) {
        QV4::ScopedObject scripts(scope, v4->newArrayObject(compilationUnit->dependentScripts.count()));
        context->importedScripts.set(v4, scripts);
        QV4::ScopedValue v(scope);
        for (int i = 0; i < compilationUnit->dependentScripts.count(); ++i) {
            QQmlRefPointer<QQmlScriptData> s = compilationUnit->dependentScripts.at(i);
            scripts->put(i, (v = s->scriptValueForContext(context)));
        }
    } else if (sharedState->creationContext) {
        context->importedScripts = sharedState->creationContext->importedScripts;
    }

    QObject *instance = createInstance(objectToCreate, parent, /*isContextObject*/true);
    if (instance) {
        QQmlData *ddata = QQmlData::get(instance);
        Q_ASSERT(ddata);
        ddata->compilationUnit = compilationUnit;
    }

    if (topLevelCreator)
        sharedState->allJavaScriptObjects = nullptr;

    phase = CreatingObjectsPhase2;

    if (interrupt && interrupt->shouldInterrupt())
        return nullptr;

    phase = ObjectsCreated;

    if (instance) {
        if (QQmlEngineDebugService *service
                = QQmlDebugConnector::service<QQmlEngineDebugService>()) {
            if (!parentContext->isInternal)
                parentContext->asQQmlContextPrivate()->instances.append(instance);
            service->objectCreated(engine, instance);
        } else if (!parentContext->isInternal && QQmlDebugConnector::service<QV4DebugService>()) {
            parentContext->asQQmlContextPrivate()->instances.append(instance);
        }
    }

    return instance;
}

// ### unify or keep in sync with populateDeferredBinding()
bool QQmlObjectCreator::populateDeferredProperties(QObject *instance, QQmlData::DeferredData *deferredData)
{
    QQmlData *declarativeData = QQmlData::get(instance);
    context = deferredData->context;
    sharedState->rootContext = context;

    QObject *bindingTarget = instance;

    QQmlRefPointer<QQmlPropertyCache> cache = declarativeData->propertyCache;
    QQmlVMEMetaObject *vmeMetaObject = QQmlVMEMetaObject::get(instance);

    QObject *scopeObject = instance;
    qSwap(_scopeObject, scopeObject);

    QV4::Scope valueScope(v4);

    Q_ASSERT(topLevelCreator);
    Q_ASSERT(!sharedState->allJavaScriptObjects);
    sharedState->allJavaScriptObjects = valueScope.alloc(compilationUnit->totalObjectCount);

    QV4::QmlContext *qmlContext = static_cast<QV4::QmlContext *>(valueScope.alloc());

    qSwap(_qmlContext, qmlContext);

    qSwap(_propertyCache, cache);
    qSwap(_qobject, instance);

    int objectIndex = deferredData->deferredIdx;
    qSwap(_compiledObjectIndex, objectIndex);

    const QV4::CompiledData::Object *obj = compilationUnit->objectAt(_compiledObjectIndex);
    qSwap(_compiledObject, obj);

    qSwap(_ddata, declarativeData);
    qSwap(_bindingTarget, bindingTarget);
    qSwap(_vmeMetaObject, vmeMetaObject);

    setupBindings(/*applyDeferredBindings=*/true);

    qSwap(_vmeMetaObject, vmeMetaObject);
    qSwap(_bindingTarget, bindingTarget);
    qSwap(_ddata, declarativeData);
    qSwap(_compiledObject, obj);
    qSwap(_compiledObjectIndex, objectIndex);
    qSwap(_qobject, instance);
    qSwap(_propertyCache, cache);

    qSwap(_qmlContext, qmlContext);
    qSwap(_scopeObject, scopeObject);

    deferredData->bindings.clear();
    phase = ObjectsCreated;

    return errors.isEmpty();
}

// ### unify or keep in sync with populateDeferredProperties()
bool QQmlObjectCreator::populateDeferredBinding(const QQmlProperty &qmlProperty, QQmlData::DeferredData *deferredData, const QV4::CompiledData::Binding *binding)
{
    Q_ASSERT(binding->flags & QV4::CompiledData::Binding::IsDeferredBinding);

    QObject *instance = qmlProperty.object();
    QQmlData *declarativeData = QQmlData::get(instance);
    context = deferredData->context;
    sharedState->rootContext = context;

    QObject *bindingTarget = instance;

    QQmlRefPointer<QQmlPropertyCache> cache = declarativeData->propertyCache;
    QQmlVMEMetaObject *vmeMetaObject = QQmlVMEMetaObject::get(instance);

    QObject *scopeObject = instance;
    qSwap(_scopeObject, scopeObject);

    QV4::Scope valueScope(v4);

    Q_ASSERT(topLevelCreator);
    if (!sharedState->allJavaScriptObjects)
        sharedState->allJavaScriptObjects = valueScope.alloc(compilationUnit->totalObjectCount);

    QV4::QmlContext *qmlContext = static_cast<QV4::QmlContext *>(valueScope.alloc());

    qSwap(_qmlContext, qmlContext);

    qSwap(_propertyCache, cache);
    qSwap(_qobject, instance);

    int objectIndex = deferredData->deferredIdx;
    qSwap(_compiledObjectIndex, objectIndex);

    const QV4::CompiledData::Object *obj = compilationUnit->objectAt(_compiledObjectIndex);
    qSwap(_compiledObject, obj);

    qSwap(_ddata, declarativeData);
    qSwap(_bindingTarget, bindingTarget);
    qSwap(_vmeMetaObject, vmeMetaObject);

    QQmlListProperty<void> savedList;
    qSwap(_currentList, savedList);

    const QQmlPropertyData &property = QQmlPropertyPrivate::get(qmlProperty)->core;

    if (property.isQList()) {
        void *argv[1] = { (void*)&_currentList };
        QMetaObject::metacall(_qobject, QMetaObject::ReadProperty, property.coreIndex(), argv);
    } else if (_currentList.object) {
        _currentList = QQmlListProperty<void>();
    }

    setPropertyBinding(&property, binding);

    qSwap(_currentList, savedList);

    qSwap(_vmeMetaObject, vmeMetaObject);
    qSwap(_bindingTarget, bindingTarget);
    qSwap(_ddata, declarativeData);
    qSwap(_compiledObject, obj);
    qSwap(_compiledObjectIndex, objectIndex);
    qSwap(_qobject, instance);
    qSwap(_propertyCache, cache);

    qSwap(_qmlContext, qmlContext);
    qSwap(_scopeObject, scopeObject);

    phase = ObjectsCreated;

    return errors.isEmpty();
}

void QQmlObjectCreator::setPropertyValue(const QQmlPropertyData *property, const QV4::CompiledData::Binding *binding)
{
    QQmlPropertyData::WriteFlags propertyWriteFlags = QQmlPropertyData::BypassInterceptor | QQmlPropertyData::RemoveBindingOnAliasWrite;
    QV4::Scope scope(v4);

    int propertyType = property->propType();

    if (property->isEnum()) {
        if (binding->flags & QV4::CompiledData::Binding::IsResolvedEnum) {
            propertyType = QMetaType::Int;
        } else {
            // ### This should be resolved earlier at compile time and the binding value should be changed accordingly.
            QVariant value = binding->valueAsString(compilationUnit.data());
            bool ok = QQmlPropertyPrivate::write(_qobject, *property, value, context);
            Q_ASSERT(ok);
            Q_UNUSED(ok);
            return;
        }
    }

    switch (propertyType) {
    case QMetaType::QVariant: {
        if (binding->type == QV4::CompiledData::Binding::Type_Number) {
            double n = binding->valueAsNumber(compilationUnit->constants);
            if (double(int(n)) == n) {
                if (property->isVarProperty()) {
                    _vmeMetaObject->setVMEProperty(property->coreIndex(), QV4::Value::fromInt32(int(n)));
                } else {
                    int i = int(n);
                    QVariant value(i);
                    property->writeProperty(_qobject, &value, propertyWriteFlags);
                }
            } else {
                if (property->isVarProperty()) {
                    _vmeMetaObject->setVMEProperty(property->coreIndex(), QV4::Value::fromDouble(n));
                } else {
                    QVariant value(n);
                    property->writeProperty(_qobject, &value, propertyWriteFlags);
                }
            }
        } else if (binding->type == QV4::CompiledData::Binding::Type_Boolean) {
            if (property->isVarProperty()) {
                _vmeMetaObject->setVMEProperty(property->coreIndex(), QV4::Value::fromBoolean(binding->valueAsBoolean()));
            } else {
                QVariant value(binding->valueAsBoolean());
                property->writeProperty(_qobject, &value, propertyWriteFlags);
            }
        } else {
            QString stringValue = binding->valueAsString(compilationUnit.data());
            if (property->isVarProperty()) {
                QV4::ScopedString s(scope, v4->newString(stringValue));
                _vmeMetaObject->setVMEProperty(property->coreIndex(), s);
            } else {
                QVariant value = QQmlStringConverters::variantFromString(stringValue);
                property->writeProperty(_qobject, &value, propertyWriteFlags);
            }
        }
    }
    break;
    case QVariant::String: {
        Q_ASSERT(binding->evaluatesToString());
        QString value = binding->valueAsString(compilationUnit.data());
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::StringList: {
        Q_ASSERT(binding->evaluatesToString());
        QStringList value(binding->valueAsString(compilationUnit.data()));
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::ByteArray: {
        Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_String);
        QByteArray value(binding->valueAsString(compilationUnit.data()).toUtf8());
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::Url: {
        Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_String);
        QString string = binding->valueAsString(compilationUnit.data());
        // Encoded dir-separators defeat QUrl processing - decode them first
        string.replace(QLatin1String("%2f"), QLatin1String("/"), Qt::CaseInsensitive);
        QUrl value = string.isEmpty() ? QUrl() : compilationUnit->finalUrl().resolved(QUrl(string));
        // Apply URL interceptor
        if (engine->urlInterceptor())
            value = engine->urlInterceptor()->intercept(value, QQmlAbstractUrlInterceptor::UrlString);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::UInt: {
        Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_Number);
        double d = binding->valueAsNumber(compilationUnit->constants);
        uint value = uint(d);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
        break;
    }
    break;
    case QVariant::Int: {
        Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_Number);
        double d = binding->valueAsNumber(compilationUnit->constants);
        int value = int(d);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
        break;
    }
    break;
    case QMetaType::Float: {
        Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_Number);
        float value = float(binding->valueAsNumber(compilationUnit->constants));
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::Double: {
        Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_Number);
        double value = binding->valueAsNumber(compilationUnit->constants);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::Color: {
        bool ok = false;
        uint colorValue = QQmlStringConverters::rgbaFromString(binding->valueAsString(compilationUnit.data()), &ok);
        Q_ASSERT(ok);
        struct { void *data[4]; } buffer;
        if (QQml_valueTypeProvider()->storeValueType(property->propType(), &colorValue, &buffer, sizeof(buffer))) {
            property->writeProperty(_qobject, &buffer, propertyWriteFlags);
        }
    }
    break;
#if QT_CONFIG(datestring)
    case QVariant::Date: {
        bool ok = false;
        QDate value = QQmlStringConverters::dateFromString(binding->valueAsString(compilationUnit.data()), &ok);
        Q_ASSERT(ok);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::Time: {
        bool ok = false;
        QTime value = QQmlStringConverters::timeFromString(binding->valueAsString(compilationUnit.data()), &ok);
        Q_ASSERT(ok);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::DateTime: {
        bool ok = false;
        QDateTime value = QQmlStringConverters::dateTimeFromString(binding->valueAsString(compilationUnit.data()), &ok);
        // ### VME compatibility :(
        {
            const qint64 date = value.date().toJulianDay();
            const int msecsSinceStartOfDay = value.time().msecsSinceStartOfDay();
            value = QDateTime(QDate::fromJulianDay(date), QTime::fromMSecsSinceStartOfDay(msecsSinceStartOfDay));
        }
        Q_ASSERT(ok);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
#endif // datestring
    case QVariant::Point: {
        bool ok = false;
        QPoint value = QQmlStringConverters::pointFFromString(binding->valueAsString(compilationUnit.data()), &ok).toPoint();
        Q_ASSERT(ok);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::PointF: {
        bool ok = false;
        QPointF value = QQmlStringConverters::pointFFromString(binding->valueAsString(compilationUnit.data()), &ok);
        Q_ASSERT(ok);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::Size: {
        bool ok = false;
        QSize value = QQmlStringConverters::sizeFFromString(binding->valueAsString(compilationUnit.data()), &ok).toSize();
        Q_ASSERT(ok);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::SizeF: {
        bool ok = false;
        QSizeF value = QQmlStringConverters::sizeFFromString(binding->valueAsString(compilationUnit.data()), &ok);
        Q_ASSERT(ok);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::Rect: {
        bool ok = false;
        QRect value = QQmlStringConverters::rectFFromString(binding->valueAsString(compilationUnit.data()), &ok).toRect();
        Q_ASSERT(ok);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::RectF: {
        bool ok = false;
        QRectF value = QQmlStringConverters::rectFFromString(binding->valueAsString(compilationUnit.data()), &ok);
        Q_ASSERT(ok);
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::Bool: {
        Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_Boolean);
        bool value = binding->valueAsBoolean();
        property->writeProperty(_qobject, &value, propertyWriteFlags);
    }
    break;
    case QVariant::Vector2D: {
        struct {
            float xp;
            float yp;
        } vec;
        bool ok = QQmlStringConverters::createFromString(QMetaType::QVector2D, binding->valueAsString(compilationUnit.data()), &vec, sizeof(vec));
        Q_ASSERT(ok);
        Q_UNUSED(ok);
        property->writeProperty(_qobject, &vec, propertyWriteFlags);
    }
    break;
    case QVariant::Vector3D: {
        struct {
            float xp;
            float yp;
            float zy;
        } vec;
        bool ok = QQmlStringConverters::createFromString(QMetaType::QVector3D, binding->valueAsString(compilationUnit.data()), &vec, sizeof(vec));
        Q_ASSERT(ok);
        Q_UNUSED(ok);
        property->writeProperty(_qobject, &vec, propertyWriteFlags);
    }
    break;
    case QVariant::Vector4D: {
        struct {
            float xp;
            float yp;
            float zy;
            float wp;
        } vec;
        bool ok = QQmlStringConverters::createFromString(QMetaType::QVector4D, binding->valueAsString(compilationUnit.data()), &vec, sizeof(vec));
        Q_ASSERT(ok);
        Q_UNUSED(ok);
        property->writeProperty(_qobject, &vec, propertyWriteFlags);
    }
    break;
    case QVariant::Quaternion: {
        struct {
            float wp;
            float xp;
            float yp;
            float zp;
        } vec;
        bool ok = QQmlStringConverters::createFromString(QMetaType::QQuaternion, binding->valueAsString(compilationUnit.data()), &vec, sizeof(vec));
        Q_ASSERT(ok);
        Q_UNUSED(ok);
        property->writeProperty(_qobject, &vec, propertyWriteFlags);
    }
    break;
    case QVariant::RegExp:
        Q_ASSERT(!"not possible");
        break;
    default: {
        // generate single literal value assignment to a list property if required
        if (property->propType() == qMetaTypeId<QList<qreal> >()) {
            Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_Number);
            QList<qreal> value;
            value.append(binding->valueAsNumber(compilationUnit->constants));
            property->writeProperty(_qobject, &value, propertyWriteFlags);
            break;
        } else if (property->propType() == qMetaTypeId<QList<int> >()) {
            Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_Number);
            double n = binding->valueAsNumber(compilationUnit->constants);
            QList<int> value;
            value.append(int(n));
            property->writeProperty(_qobject, &value, propertyWriteFlags);
            break;
        } else if (property->propType() == qMetaTypeId<QList<bool> >()) {
            Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_Boolean);
            QList<bool> value;
            value.append(binding->valueAsBoolean());
            property->writeProperty(_qobject, &value, propertyWriteFlags);
            break;
        } else if (property->propType() == qMetaTypeId<QList<QUrl> >()) {
            Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_String);
            QString urlString = binding->valueAsString(compilationUnit.data());
            QUrl u = urlString.isEmpty() ? QUrl()
                                         : compilationUnit->finalUrl().resolved(QUrl(urlString));
            QList<QUrl> value;
            value.append(u);
            property->writeProperty(_qobject, &value, propertyWriteFlags);
            break;
        } else if (property->propType() == qMetaTypeId<QList<QString> >()) {
            Q_ASSERT(binding->evaluatesToString());
            QList<QString> value;
            value.append(binding->valueAsString(compilationUnit.data()));
            property->writeProperty(_qobject, &value, propertyWriteFlags);
            break;
        } else if (property->propType() == qMetaTypeId<QJSValue>()) {
            QJSValue value;
            if (binding->type == QV4::CompiledData::Binding::Type_Boolean) {
                value = QJSValue(binding->valueAsBoolean());
            } else if (binding->type == QV4::CompiledData::Binding::Type_Number) {
                double n = binding->valueAsNumber(compilationUnit->constants);
                if (double(int(n)) == n) {
                    value = QJSValue(int(n));
                } else
                    value = QJSValue(n);
            } else {
                value = QJSValue(binding->valueAsString(compilationUnit.data()));
            }
            property->writeProperty(_qobject, &value, propertyWriteFlags);
            break;
        }

        // otherwise, try a custom type assignment
        QString stringValue = binding->valueAsString(compilationUnit.data());
        QQmlMetaType::StringConverter converter = QQmlMetaType::customStringConverter(property->propType());
        Q_ASSERT(converter);
        QVariant value = (*converter)(stringValue);

        QMetaProperty metaProperty = _qobject->metaObject()->property(property->coreIndex());
        if (value.isNull() || ((int)metaProperty.type() != property->propType() && metaProperty.userType() != property->propType())) {
            recordError(binding->location, tr("Cannot assign value %1 to property %2").arg(stringValue).arg(QString::fromUtf8(metaProperty.name())));
            break;
        }

        property->writeProperty(_qobject, value.data(), propertyWriteFlags);
    }
    break;
    }
}

static QQmlType qmlTypeForObject(QObject *object)
{
    QQmlType type;
    const QMetaObject *mo = object->metaObject();
    while (mo && !type.isValid()) {
        type = QQmlMetaType::qmlType(mo);
        mo = mo->superClass();
    }
    return type;
}

void QQmlObjectCreator::setupBindings(bool applyDeferredBindings)
{
    QQmlListProperty<void> savedList;
    qSwap(_currentList, savedList);

    const QV4::CompiledData::BindingPropertyData &propertyData = compilationUnit->bindingPropertyDataPerObject.at(_compiledObjectIndex);

    if (_compiledObject->idNameIndex) {
        const QQmlPropertyData *idProperty = propertyData.last();
        Q_ASSERT(!idProperty || !idProperty->isValid() || idProperty->name(_qobject) == QLatin1String("id"));
        if (idProperty && idProperty->isValid() && idProperty->isWritable() && idProperty->propType() == QMetaType::QString) {
            QV4::CompiledData::Binding idBinding;
            idBinding.propertyNameIndex = 0; // Not used
            idBinding.flags = 0;
            idBinding.type = QV4::CompiledData::Binding::Type_String;
            idBinding.stringIndex = _compiledObject->idNameIndex;
            idBinding.location = _compiledObject->location; // ###
            setPropertyValue(idProperty, &idBinding);
        }
    }

    // ### this is best done through type-compile-time binding skip lists.
    if (_valueTypeProperty) {
        QQmlAbstractBinding *binding = QQmlPropertyPrivate::binding(_bindingTarget, QQmlPropertyIndex(_valueTypeProperty->coreIndex()));

        if (binding && !binding->isValueTypeProxy()) {
            QQmlPropertyPrivate::removeBinding(_bindingTarget, QQmlPropertyIndex(_valueTypeProperty->coreIndex()));
        } else if (binding) {
            QQmlValueTypeProxyBinding *proxy = static_cast<QQmlValueTypeProxyBinding *>(binding);

            if (qmlTypeForObject(_bindingTarget).isValid()) {
                quint32 bindingSkipList = 0;

                QQmlPropertyData *defaultProperty = _compiledObject->indexOfDefaultPropertyOrAlias != -1 ? _propertyCache->parent()->defaultProperty() : _propertyCache->defaultProperty();

                const QV4::CompiledData::Binding *binding = _compiledObject->bindingTable();
                for (quint32 i = 0; i < _compiledObject->nBindings; ++i, ++binding) {
                    QQmlPropertyData *property = binding->propertyNameIndex != 0 ? _propertyCache->property(stringAt(binding->propertyNameIndex), _qobject, context) : defaultProperty;
                    if (property)
                        bindingSkipList |= (1 << property->coreIndex());
                }

                proxy->removeBindings(bindingSkipList);
            }
        }
    }

    int currentListPropertyIndex = -1;

    const QV4::CompiledData::Binding *binding = _compiledObject->bindingTable();
    for (quint32 i = 0; i < _compiledObject->nBindings; ++i, ++binding) {
        if (binding->flags & QV4::CompiledData::Binding::IsCustomParserBinding)
            continue;

        if (binding->flags & QV4::CompiledData::Binding::IsDeferredBinding) {
            if (!applyDeferredBindings)
                continue;
        } else {
            if (applyDeferredBindings)
                continue;
        }

        const QQmlPropertyData *property = propertyData.at(i);

        if (property && property->isQList()) {
            if (property->coreIndex() != currentListPropertyIndex) {
                void *argv[1] = { (void*)&_currentList };
                QMetaObject::metacall(_qobject, QMetaObject::ReadProperty, property->coreIndex(), argv);
                currentListPropertyIndex = property->coreIndex();
            }
        } else if (_currentList.object) {
            _currentList = QQmlListProperty<void>();
            currentListPropertyIndex = -1;
        }

        if (!setPropertyBinding(property, binding))
            return;
    }

    qSwap(_currentList, savedList);
}

bool QQmlObjectCreator::setPropertyBinding(const QQmlPropertyData *bindingProperty, const QV4::CompiledData::Binding *binding)
{
    if (binding->type == QV4::CompiledData::Binding::Type_AttachedProperty) {
        Q_ASSERT(stringAt(compilationUnit->objectAt(binding->value.objectIndex)->inheritedTypeNameIndex).isEmpty());
        QV4::CompiledData::ResolvedTypeReference *tr = resolvedType(binding->propertyNameIndex);
        Q_ASSERT(tr);
        QQmlType attachedType = tr->type;
        if (!attachedType.isValid()) {
            QQmlTypeNameCache::Result res = context->imports->query(stringAt(binding->propertyNameIndex));
            if (res.isValid())
                attachedType = res.type;
            else
                return false;
        }
        QObject *qmlObject = qmlAttachedPropertiesObject(
                _qobject, attachedType.attachedPropertiesFunction(QQmlEnginePrivate::get(engine)));
        if (!populateInstance(binding->value.objectIndex, qmlObject, qmlObject, /*value type property*/nullptr))
            return false;
        return true;
    }

    // ### resolve this at compile time
    if (bindingProperty && bindingProperty->propType() == qMetaTypeId<QQmlScriptString>()) {
        QQmlScriptString ss(binding->valueAsScriptString(compilationUnit.data()), context->asQQmlContext(), _scopeObject);
        ss.d.data()->bindingId = binding->type == QV4::CompiledData::Binding::Type_Script ? binding->value.compiledScriptIndex : (quint32)QQmlBinding::Invalid;
        ss.d.data()->lineNumber = binding->location.line;
        ss.d.data()->columnNumber = binding->location.column;
        ss.d.data()->isStringLiteral = binding->type == QV4::CompiledData::Binding::Type_String;
        ss.d.data()->isNumberLiteral = binding->type == QV4::CompiledData::Binding::Type_Number;
        ss.d.data()->numberValue = binding->valueAsNumber(compilationUnit->constants);

        QQmlPropertyData::WriteFlags propertyWriteFlags = QQmlPropertyData::BypassInterceptor |
                                                            QQmlPropertyData::RemoveBindingOnAliasWrite;
        int propertyWriteStatus = -1;
        void *argv[] = { &ss, nullptr, &propertyWriteStatus, &propertyWriteFlags };
        QMetaObject::metacall(_qobject, QMetaObject::WriteProperty, bindingProperty->coreIndex(), argv);
        return true;
    }

    QObject *createdSubObject = nullptr;
    if (binding->type == QV4::CompiledData::Binding::Type_Object) {
        createdSubObject = createInstance(binding->value.objectIndex, _bindingTarget);
        if (!createdSubObject)
            return false;
    }

    if (!bindingProperty) // ### error
        return true;

    if (binding->type == QV4::CompiledData::Binding::Type_GroupProperty) {
        const QV4::CompiledData::Object *obj = compilationUnit->objectAt(binding->value.objectIndex);
        if (stringAt(obj->inheritedTypeNameIndex).isEmpty()) {

            QObject *groupObject = nullptr;
            QQmlValueType *valueType = nullptr;
            const QQmlPropertyData *valueTypeProperty = nullptr;
            QObject *bindingTarget = _bindingTarget;

            if (QQmlValueTypeFactory::isValueType(bindingProperty->propType())) {
                valueType = QQmlValueTypeFactory::valueType(bindingProperty->propType());
                if (!valueType) {
                    recordError(binding->location, tr("Cannot set properties on %1 as it is null").arg(stringAt(binding->propertyNameIndex)));
                    return false;
                }

                valueType->read(_qobject, bindingProperty->coreIndex());

                groupObject = valueType;
                valueTypeProperty = bindingProperty;
            } else {
                void *argv[1] = { &groupObject };
                QMetaObject::metacall(_qobject, QMetaObject::ReadProperty, bindingProperty->coreIndex(), argv);
                if (!groupObject) {
                    recordError(binding->location, tr("Cannot set properties on %1 as it is null").arg(stringAt(binding->propertyNameIndex)));
                    return false;
                }

                bindingTarget = groupObject;
            }

            if (!populateInstance(binding->value.objectIndex, groupObject, bindingTarget, valueTypeProperty))
                return false;

            if (valueType)
                valueType->write(_qobject, bindingProperty->coreIndex(), QQmlPropertyData::BypassInterceptor);

            return true;
        }
    }

    if (_ddata->hasBindingBit(bindingProperty->coreIndex()) && !(binding->flags & QV4::CompiledData::Binding::IsSignalHandlerExpression)
        && !(binding->flags & QV4::CompiledData::Binding::IsOnAssignment)
        && !_valueTypeProperty)
        QQmlPropertyPrivate::removeBinding(_bindingTarget, QQmlPropertyIndex(bindingProperty->coreIndex()));

    if (binding->type == QV4::CompiledData::Binding::Type_Script || binding->isTranslationBinding()) {
        if (binding->flags & QV4::CompiledData::Binding::IsSignalHandlerExpression) {
            QV4::Function *runtimeFunction = compilationUnit->runtimeFunctions[binding->value.compiledScriptIndex];
            int signalIndex = _propertyCache->methodIndexToSignalIndex(bindingProperty->coreIndex());
            QQmlBoundSignal *bs = new QQmlBoundSignal(_bindingTarget, signalIndex, _scopeObject, engine);
            QQmlBoundSignalExpression *expr = new QQmlBoundSignalExpression(_bindingTarget, signalIndex,
                                                                            context, _scopeObject, runtimeFunction, currentQmlContext());

            bs->takeExpression(expr);
        } else {
            // When writing bindings to grouped properties implemented as value types,
            // such as point.x: { someExpression; }, then the binding is installed on
            // the point property (_qobjectForBindings) and after evaluating the expression,
            // the result is written to a value type virtual property, that contains the sub-index
            // of the "x" property.
            QQmlBinding::Ptr qmlBinding;
            const QQmlPropertyData *targetProperty = bindingProperty;
            const QQmlPropertyData *subprop = nullptr;
            if (_valueTypeProperty) {
                targetProperty = _valueTypeProperty;
                subprop = bindingProperty;
            }
            if (binding->isTranslationBinding()) {
                qmlBinding = QQmlBinding::createTranslationBinding(compilationUnit, binding, _scopeObject, context);
            } else {
                QV4::Function *runtimeFunction = compilationUnit->runtimeFunctions[binding->value.compiledScriptIndex];
                qmlBinding = QQmlBinding::create(targetProperty, runtimeFunction, _scopeObject, context, currentQmlContext());
            }

            auto bindingTarget = _bindingTarget;
            auto valueTypeProperty = _valueTypeProperty;
            auto assignBinding = [qmlBinding, bindingTarget, targetProperty, subprop, bindingProperty, valueTypeProperty](QQmlObjectCreatorSharedState *sharedState) -> bool {
                if (!qmlBinding->setTarget(bindingTarget, *targetProperty, subprop) && targetProperty->isAlias())
                    return false;

                sharedState->allCreatedBindings.push(qmlBinding);

                if (bindingProperty->isAlias()) {
                    QQmlPropertyPrivate::setBinding(qmlBinding.data(), QQmlPropertyPrivate::DontEnable);
                } else {
                    qmlBinding->addToObject();

                    if (!valueTypeProperty) {
                        QQmlData *targetDeclarativeData = QQmlData::get(bindingTarget);
                        Q_ASSERT(targetDeclarativeData);
                        targetDeclarativeData->setPendingBindingBit(bindingTarget, bindingProperty->coreIndex());
                    }
                }

                return true;
            };
            if (!assignBinding(sharedState.data()))
                pendingAliasBindings.push_back(assignBinding);
        }
        return true;
    }

    if (binding->type == QV4::CompiledData::Binding::Type_Object) {
        if (binding->flags & QV4::CompiledData::Binding::IsOnAssignment) {
            // ### determine value source and interceptor casts ahead of time.
            QQmlType type = qmlTypeForObject(createdSubObject);
            Q_ASSERT(type.isValid());

            int valueSourceCast = type.propertyValueSourceCast();
            if (valueSourceCast != -1) {
                QQmlPropertyValueSource *vs = reinterpret_cast<QQmlPropertyValueSource *>(reinterpret_cast<char *>(createdSubObject) + valueSourceCast);
                QObject *target = createdSubObject->parent();
                QQmlProperty prop;
                if (_valueTypeProperty)
                    prop = QQmlPropertyPrivate::restore(target, *_valueTypeProperty, bindingProperty, context);
                else
                    prop = QQmlPropertyPrivate::restore(target, *bindingProperty, nullptr, context);
                vs->setTarget(prop);
                return true;
            }
            int valueInterceptorCast = type.propertyValueInterceptorCast();
            if (valueInterceptorCast != -1) {
                QQmlPropertyValueInterceptor *vi = reinterpret_cast<QQmlPropertyValueInterceptor *>(reinterpret_cast<char *>(createdSubObject) + valueInterceptorCast);
                QObject *target = createdSubObject->parent();

                QQmlPropertyIndex propertyIndex;
                if (bindingProperty->isAlias()) {
                    QQmlPropertyIndex originalIndex(bindingProperty->coreIndex(), _valueTypeProperty ? _valueTypeProperty->coreIndex() : -1);
                    QQmlPropertyIndex propIndex;
                    QQmlPropertyPrivate::findAliasTarget(target, originalIndex, &target, &propIndex);
                    QQmlData *data = QQmlData::get(target);
                    if (!data || !data->propertyCache) {
                        qWarning() << "can't resolve property alias for 'on' assignment";
                        return false;
                    }

                    // we can't have aliasses on subproperties of value types, so:
                    QQmlPropertyData targetPropertyData = *data->propertyCache->property(propIndex.coreIndex());
                    auto prop = QQmlPropertyPrivate::restore(target, targetPropertyData, nullptr, context);
                    vi->setTarget(prop);
                    propertyIndex = QQmlPropertyPrivate::propertyIndex(prop);
                } else {
                    QQmlProperty prop;
                    if (_valueTypeProperty)
                        prop = QQmlPropertyPrivate::restore(target, *_valueTypeProperty, bindingProperty, context);
                    else
                        prop = QQmlPropertyPrivate::restore(target, *bindingProperty, nullptr, context);
                    vi->setTarget(prop);
                    propertyIndex = QQmlPropertyPrivate::propertyIndex(prop);
                }

                QQmlInterceptorMetaObject *mo = QQmlInterceptorMetaObject::get(target);
                if (!mo)
                    mo = new QQmlInterceptorMetaObject(target, QQmlData::get(target)->propertyCache);
                mo->registerInterceptor(propertyIndex, vi);
                return true;
            }
            return false;
        }

        // Assigning object to signal property?
        if (binding->flags & QV4::CompiledData::Binding::IsSignalHandlerObject) {
            if (!bindingProperty->isFunction()) {
                recordError(binding->valueLocation, tr("Cannot assign an object to signal property %1").arg(bindingProperty->name(_qobject)));
                return false;
            }
            QMetaMethod method = QQmlMetaType::defaultMethod(createdSubObject);
            if (!method.isValid()) {
                recordError(binding->valueLocation, tr("Cannot assign object type %1 with no default method").arg(QString::fromLatin1(createdSubObject->metaObject()->className())));
                return false;
            }

            QMetaMethod signalMethod = _qobject->metaObject()->method(bindingProperty->coreIndex());
            if (!QMetaObject::checkConnectArgs(signalMethod, method)) {
                recordError(binding->valueLocation,
                            tr("Cannot connect mismatched signal/slot %1 %vs. %2")
                            .arg(QString::fromUtf8(method.methodSignature()))
                            .arg(QString::fromUtf8(signalMethod.methodSignature())));
                return false;
            }

            QQmlPropertyPrivate::connect(_qobject, bindingProperty->coreIndex(), createdSubObject, method.methodIndex());
            return true;
        }

        QQmlPropertyData::WriteFlags propertyWriteFlags = QQmlPropertyData::BypassInterceptor |
                                                            QQmlPropertyData::RemoveBindingOnAliasWrite;
        int propertyWriteStatus = -1;
        void *argv[] = { nullptr, nullptr, &propertyWriteStatus, &propertyWriteFlags };

        if (const char *iid = QQmlMetaType::interfaceIId(bindingProperty->propType())) {
            void *ptr = createdSubObject->qt_metacast(iid);
            if (ptr) {
                argv[0] = &ptr;
                QMetaObject::metacall(_qobject, QMetaObject::WriteProperty, bindingProperty->coreIndex(), argv);
            } else {
                recordError(binding->location, tr("Cannot assign object to interface property"));
                return false;
            }
        } else if (bindingProperty->propType() == QMetaType::QVariant) {
            if (bindingProperty->isVarProperty()) {
                QV4::Scope scope(v4);
                QV4::ScopedValue wrappedObject(scope, QV4::QObjectWrapper::wrap(engine->handle(), createdSubObject));
                _vmeMetaObject->setVMEProperty(bindingProperty->coreIndex(), wrappedObject);
            } else {
                QVariant value = QVariant::fromValue(createdSubObject);
                argv[0] = &value;
                QMetaObject::metacall(_qobject, QMetaObject::WriteProperty, bindingProperty->coreIndex(), argv);
            }
        } else if (bindingProperty->propType() == qMetaTypeId<QJSValue>()) {
            QV4::Scope scope(v4);
            QV4::ScopedValue wrappedObject(scope, QV4::QObjectWrapper::wrap(engine->handle(), createdSubObject));
            if (bindingProperty->isVarProperty()) {
                _vmeMetaObject->setVMEProperty(bindingProperty->coreIndex(), wrappedObject);
            } else {
                QJSValue value;
                QJSValuePrivate::setValue(&value, v4, wrappedObject);
                argv[0] = &value;
                QMetaObject::metacall(_qobject, QMetaObject::WriteProperty, bindingProperty->coreIndex(), argv);
            }
        } else if (bindingProperty->isQList()) {
            Q_ASSERT(_currentList.object);

            void *itemToAdd = createdSubObject;

            const char *iid = nullptr;
            int listItemType = QQmlEnginePrivate::get(engine)->listType(bindingProperty->propType());
            if (listItemType != -1)
                iid = QQmlMetaType::interfaceIId(listItemType);
            if (iid)
                itemToAdd = createdSubObject->qt_metacast(iid);

            if (_currentList.append)
                _currentList.append(&_currentList, itemToAdd);
            else {
                recordError(binding->location, tr("Cannot assign object to read only list"));
                return false;
            }

        } else {
            // pointer compatibility was tested in QQmlPropertyValidator at type compile time
            argv[0] = &createdSubObject;
            QMetaObject::metacall(_qobject, QMetaObject::WriteProperty, bindingProperty->coreIndex(), argv);
        }
        return true;
    }

    if (bindingProperty->isQList()) {
        recordError(binding->location, tr("Cannot assign primitives to lists"));
        return false;
    }

    setPropertyValue(bindingProperty, binding);
    return true;
}

void QQmlObjectCreator::setupFunctions()
{
    QV4::Scope scope(v4);
    QV4::ScopedValue function(scope);
    QV4::ScopedContext qmlContext(scope, currentQmlContext());

    const quint32_le *functionIdx = _compiledObject->functionOffsetTable();
    for (quint32 i = 0; i < _compiledObject->nFunctions; ++i, ++functionIdx) {
        QV4::Function *runtimeFunction = compilationUnit->runtimeFunctions[*functionIdx];
        const QString name = runtimeFunction->name()->toQString();

        QQmlPropertyData *property = _propertyCache->property(name, _qobject, context);
        if (!property->isVMEFunction())
            continue;

        function = QV4::FunctionObject::createScriptFunction(qmlContext, runtimeFunction);
        _vmeMetaObject->setVmeMethod(property->coreIndex(), function);
    }
}

void QQmlObjectCreator::recordError(const QV4::CompiledData::Location &location, const QString &description)
{
    QQmlError error;
    error.setUrl(compilationUnit->url());
    error.setLine(location.line);
    error.setColumn(location.column);
    error.setDescription(description);
    errors << error;
}

void QQmlObjectCreator::registerObjectWithContextById(const QV4::CompiledData::Object *object, QObject *instance) const
{
    if (object->id >= 0)
        context->setIdProperty(object->id, instance);
}

void QQmlObjectCreator::createQmlContext()
{
    _qmlContext->setM(QV4::QmlContext::create(v4->rootContext(), context, _scopeObject));
}

QObject *QQmlObjectCreator::createInstance(int index, QObject *parent, bool isContextObject)
{
    const QV4::CompiledData::Object *obj = compilationUnit->objectAt(index);
    QQmlObjectCreationProfiler profiler(sharedState->profiler.profiler, obj);
    Q_TRACE(QQmlObjectCreator_createInstance_entry, compilationUnit.data(), obj, context->url());
    QString typeName;
    Q_TRACE_EXIT(QQmlObjectCreator_createInstance_exit, typeName);

    ActiveOCRestorer ocRestorer(this, QQmlEnginePrivate::get(engine));

    bool isComponent = false;
    QObject *instance = nullptr;
    QQmlData *ddata = nullptr;
    QQmlCustomParser *customParser = nullptr;
    QQmlParserStatus *parserStatus = nullptr;
    bool installPropertyCache = true;

    if (obj->flags & QV4::CompiledData::Object::IsComponent) {
        isComponent = true;
        QQmlComponent *component = new QQmlComponent(engine, compilationUnit.data(), index, parent);
        typeName = QStringLiteral("<component>");
        QQmlComponentPrivate::get(component)->creationContext = context;
        instance = component;
        ddata = QQmlData::get(instance, /*create*/true);
    } else {
        QV4::CompiledData::ResolvedTypeReference *typeRef
                = resolvedType(obj->inheritedTypeNameIndex);
        Q_ASSERT(typeRef);
        installPropertyCache = !typeRef->isFullyDynamicType;
        QQmlType type = typeRef->type;
        if (type.isValid()) {
            typeName = type.qmlTypeName();

            void *ddataMemory = nullptr;
            type.create(&instance, &ddataMemory, sizeof(QQmlData));
            if (!instance) {
                recordError(obj->location, tr("Unable to create object of type %1").arg(stringAt(obj->inheritedTypeNameIndex)));
                return nullptr;
            }

            {
                QQmlData *ddata = new (ddataMemory) QQmlData;
                ddata->ownMemory = false;
                QObjectPrivate* p = QObjectPrivate::get(instance);
                Q_ASSERT(!p->isDeletingChildren);
                p->declarativeData = ddata;
            }

            const int parserStatusCast = type.parserStatusCast();
            if (parserStatusCast != -1)
                parserStatus = reinterpret_cast<QQmlParserStatus*>(reinterpret_cast<char *>(instance) + parserStatusCast);

            customParser = type.customParser();

            if (sharedState->rootContext && sharedState->rootContext->isRootObjectInCreation) {
                QQmlData *ddata = QQmlData::get(instance, /*create*/true);
                ddata->rootObjectInCreation = true;
                sharedState->rootContext->isRootObjectInCreation = false;
            }

            sharedState->allCreatedObjects.push(instance);
        } else {
            Q_ASSERT(typeRef->compilationUnit);
            typeName = typeRef->compilationUnit->fileName();
            if (typeRef->compilationUnit->unitData()->isSingleton())
            {
                recordError(obj->location, tr("Composite Singleton Type %1 is not creatable").arg(stringAt(obj->inheritedTypeNameIndex)));
                return nullptr;
            }

            QQmlObjectCreator subCreator(context, typeRef->compilationUnit, sharedState.data());
            instance = subCreator.create();
            if (!instance) {
                errors += subCreator.errors;
                return nullptr;
            }
        }
        if (instance->isWidgetType()) {
            if (parent && parent->isWidgetType()) {
                QAbstractDeclarativeData::setWidgetParent(instance, parent);
            } else {
                // No parent! Layouts need to handle this through a default property that
                // reparents accordingly. Otherwise the garbage collector will collect.
            }
        } else if (parent) {
            QQml_setParent_noEvent(instance, parent);
        }

        ddata = QQmlData::get(instance, /*create*/true);
    }

    Q_QML_OC_PROFILE(sharedState->profiler, profiler.update(
        compilationUnit.data(), obj, typeName, context->url()));
    Q_UNUSED(typeName); // only relevant for tracing

    ddata->lineNumber = obj->location.line;
    ddata->columnNumber = obj->location.column;

    ddata->setImplicitDestructible();
    if (static_cast<quint32>(index) == /*root object*/0 || ddata->rootObjectInCreation) {
        if (ddata->context) {
            Q_ASSERT(ddata->context != context);
            Q_ASSERT(ddata->outerContext);
            Q_ASSERT(ddata->outerContext != context);
            QQmlContextData *c = ddata->context;
            while (c->linkedContext) c = c->linkedContext;
            c->linkedContext = context;
        } else {
            ddata->context = context;
        }
        ddata->ownContext = ddata->context;
    } else if (!ddata->context) {
        ddata->context = context;
    }

    context->addObject(ddata);

    if (parserStatus) {
        parserStatus->classBegin();
        // push() the profiler state here, together with the parserStatus, as we'll pop() them
        // together, too.
        Q_QML_OC_PROFILE(sharedState->profiler, sharedState->profiler.push(obj));
        sharedState->allParserStatusCallbacks.push(parserStatus);
        parserStatus->d = &sharedState->allParserStatusCallbacks.top();
    }

    // Register the context object in the context early on in order for pending binding
    // initialization to find it available.
    if (isContextObject)
        context->contextObject = instance;

    if (customParser && obj->flags & QV4::CompiledData::Object::HasCustomParserBindings) {
        customParser->engine = QQmlEnginePrivate::get(engine);
        customParser->imports = compilationUnit->typeNameCache.data();

        QList<const QV4::CompiledData::Binding *> bindings;
        const QV4::CompiledData::Object *obj = compilationUnit->objectAt(index);
        const QV4::CompiledData::Binding *binding = obj->bindingTable();
        for (quint32 i = 0; i < obj->nBindings; ++i, ++binding) {
            if (binding->flags & QV4::CompiledData::Binding::IsCustomParserBinding) {
                bindings << binding;
            }
        }
        customParser->applyBindings(instance, compilationUnit.data(), bindings);

        customParser->engine = nullptr;
        customParser->imports = (QQmlTypeNameCache*)nullptr;
    }

    if (isComponent) {
        registerObjectWithContextById(obj, instance);
        return instance;
    }

    QQmlRefPointer<QQmlPropertyCache> cache = propertyCaches->at(index);
    Q_ASSERT(!cache.isNull());
    if (installPropertyCache) {
        if (ddata->propertyCache)
            ddata->propertyCache->release();;
        ddata->propertyCache = cache.data();
        ddata->propertyCache->addref();
    }

    QObject *scopeObject = instance;
    qSwap(_scopeObject, scopeObject);

    Q_ASSERT(sharedState->allJavaScriptObjects);
    *sharedState->allJavaScriptObjects = QV4::QObjectWrapper::wrap(v4, instance);
    ++sharedState->allJavaScriptObjects;

    QV4::Scope valueScope(v4);
    QV4::QmlContext *qmlContext = static_cast<QV4::QmlContext *>(valueScope.alloc());

    qSwap(_qmlContext, qmlContext);

    bool ok = populateInstance(index, instance, /*binding target*/instance, /*value type property*/nullptr);
    if (ok) {
        if (isContextObject && !pendingAliasBindings.empty()) {
            bool processedAtLeastOneBinding = false;
            do {
                processedAtLeastOneBinding = false;
                for (std::vector<PendingAliasBinding>::iterator it = pendingAliasBindings.begin();
                        it != pendingAliasBindings.end(); ) {
                    if ((*it)(sharedState.data())) {
                        it = pendingAliasBindings.erase(it);
                        processedAtLeastOneBinding = true;
                    } else {
                        ++it;
                    }
                }
            } while (processedAtLeastOneBinding && pendingAliasBindings.empty());
            Q_ASSERT(pendingAliasBindings.empty());
        }
    } else {
        // an error occurred, so we can't setup the pending alias bindings
        pendingAliasBindings.clear();
    }

    qSwap(_qmlContext, qmlContext);
    qSwap(_scopeObject, scopeObject);

    return ok ? instance : nullptr;
}

QQmlContextData *QQmlObjectCreator::finalize(QQmlInstantiationInterrupt &interrupt)
{
    Q_ASSERT(phase == ObjectsCreated || phase == Finalizing);
    phase = Finalizing;

    QQmlObjectCreatorRecursionWatcher watcher(this);
    ActiveOCRestorer ocRestorer(this, QQmlEnginePrivate::get(engine));

    while (!sharedState->allCreatedBindings.isEmpty()) {
        QQmlAbstractBinding::Ptr b = sharedState->allCreatedBindings.pop();
        Q_ASSERT(b);
        // skip, if b is not added to an object
        if (!b->isAddedToObject())
            continue;
        QQmlData *data = QQmlData::get(b->targetObject());
        Q_ASSERT(data);
        data->clearPendingBindingBit(b->targetPropertyIndex().coreIndex());
        b->setEnabled(true, QQmlPropertyData::BypassInterceptor |
                      QQmlPropertyData::DontRemoveBinding);
        if (!b->isValueTypeProxy()) {
            QQmlBinding *binding = static_cast<QQmlBinding*>(b.data());
            if (!binding->hasError() && !binding->hasDependencies()
                && binding->context() && !binding->context()->unresolvedNames)
                b->removeFromObject();
        }

        if (watcher.hasRecursed() || interrupt.shouldInterrupt())
            return nullptr;
    }

    if (QQmlVME::componentCompleteEnabled()) { // the qml designer does the component complete later
        while (!sharedState->allParserStatusCallbacks.isEmpty()) {
            QQmlObjectCompletionProfiler profiler(&sharedState->profiler);
            QQmlParserStatus *status = sharedState->allParserStatusCallbacks.pop();

            if (status && status->d) {
                status->d = nullptr;
                status->componentComplete();
            }

            if (watcher.hasRecursed() || interrupt.shouldInterrupt())
                return nullptr;
        }
    }

    for (int ii = 0; ii < sharedState->finalizeCallbacks.count(); ++ii) {
        QQmlEnginePrivate::FinalizeCallback callback = sharedState->finalizeCallbacks.at(ii);
        QObject *obj = callback.first;
        if (obj) {
            void *args[] = { nullptr };
            QMetaObject::metacall(obj, QMetaObject::InvokeMetaMethod, callback.second, args);
        }
        if (watcher.hasRecursed())
            return nullptr;
    }
    sharedState->finalizeCallbacks.clear();

    while (sharedState->componentAttached) {
        QQmlComponentAttached *a = sharedState->componentAttached;
        a->rem();
        QQmlData *d = QQmlData::get(a->parent());
        Q_ASSERT(d);
        Q_ASSERT(d->context);
        a->add(&d->context->componentAttached);
        if (QQmlVME::componentCompleteEnabled())
            emit a->completed();

        if (watcher.hasRecursed() || interrupt.shouldInterrupt())
            return nullptr;
    }

    phase = Done;

    return sharedState->rootContext;
}

void QQmlObjectCreator::clear()
{
    if (phase == Done || phase == Finalizing || phase == Startup)
        return;
    Q_ASSERT(phase != Startup);

    while (!sharedState->allCreatedObjects.isEmpty())
        delete sharedState->allCreatedObjects.pop();

    while (sharedState->componentAttached) {
        QQmlComponentAttached *a = sharedState->componentAttached;
        a->rem();
    }

    phase = Done;
}

bool QQmlObjectCreator::populateInstance(int index, QObject *instance, QObject *bindingTarget, const QQmlPropertyData *valueTypeProperty)
{
    QQmlData *declarativeData = QQmlData::get(instance, /*create*/true);

    qSwap(_qobject, instance);
    qSwap(_valueTypeProperty, valueTypeProperty);
    qSwap(_compiledObjectIndex, index);
    const QV4::CompiledData::Object *obj = compilationUnit->objectAt(_compiledObjectIndex);
    qSwap(_compiledObject, obj);
    qSwap(_ddata, declarativeData);
    qSwap(_bindingTarget, bindingTarget);

    QV4::Scope valueScope(v4);
    QV4::ScopedValue scopeObjectProtector(valueScope);

    QQmlRefPointer<QQmlPropertyCache> cache = propertyCaches->at(_compiledObjectIndex);

    QQmlVMEMetaObject *vmeMetaObject = nullptr;
    if (propertyCaches->needsVMEMetaObject(_compiledObjectIndex)) {
        Q_ASSERT(!cache.isNull());
        // install on _object
        vmeMetaObject = new QQmlVMEMetaObject(v4, _qobject, cache, compilationUnit, _compiledObjectIndex);
        if (_ddata->propertyCache)
            _ddata->propertyCache->release();
        _ddata->propertyCache = cache.data();
        _ddata->propertyCache->addref();
        scopeObjectProtector = _ddata->jsWrapper.value();
    } else {
        vmeMetaObject = QQmlVMEMetaObject::get(_qobject);
    }

    registerObjectWithContextById(_compiledObject, _qobject);

    qSwap(_propertyCache, cache);
    qSwap(_vmeMetaObject, vmeMetaObject);

    if (_compiledObject->flags & QV4::CompiledData::Object::HasDeferredBindings)
        _ddata->deferData(_compiledObjectIndex, compilationUnit, context);

    if (_compiledObject->nFunctions > 0)
        setupFunctions();
    setupBindings();

    qSwap(_vmeMetaObject, vmeMetaObject);
    qSwap(_bindingTarget, bindingTarget);
    qSwap(_ddata, declarativeData);
    qSwap(_compiledObject, obj);
    qSwap(_compiledObjectIndex, index);
    qSwap(_valueTypeProperty, valueTypeProperty);
    qSwap(_qobject, instance);
    qSwap(_propertyCache, cache);

    return errors.isEmpty();
}




QQmlObjectCreatorRecursionWatcher::QQmlObjectCreatorRecursionWatcher(QQmlObjectCreator *creator)
    : sharedState(creator->sharedState)
    , watcher(creator->sharedState.data())
{
}
