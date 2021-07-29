/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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


#include "qv4proxy_p.h"
#include "qv4symbol_p.h"
#include "qv4jscall_p.h"
#include "qv4objectproto_p.h"
#include "qv4persistent_p.h"
#include "qv4objectiterator_p.h"

using namespace QV4;

DEFINE_OBJECT_VTABLE(ProxyObject);
DEFINE_OBJECT_VTABLE(ProxyFunctionObject);

void Heap::ProxyObject::init(const QV4::Object *target, const QV4::Object *handler)
{
    Object::init();
    ExecutionEngine *e = internalClass->engine;
    this->target.set(e, target->d());
    this->handler.set(e, handler->d());
}

void Heap::ProxyFunctionObject::init(const QV4::FunctionObject *target, const QV4::Object *handler)
{
    ExecutionEngine *e = internalClass->engine;
    FunctionObject::init(e->rootContext());
    this->target.set(e, target->d());
    this->handler.set(e, handler->d());

    if (!target->isConstructor())
        jsConstruct = nullptr;
}


ReturnedValue ProxyObject::virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty)
{
    Scope scope(m);
    const ProxyObject *o = static_cast<const ProxyObject *>(m);
    if (!o->d()->handler)
        return scope.engine->throwTypeError();

    ScopedObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedValue trap(scope, handler->get(scope.engine->id_get()));
    if (scope.hasException())
        return Encode::undefined();
    if (trap->isNullOrUndefined())
        return target->get(id, receiver, hasProperty);
    if (!trap->isFunctionObject())
        return scope.engine->throwTypeError();
    if (hasProperty)
        *hasProperty = true;

    JSCallData cdata(scope, 3, nullptr, handler);
    cdata.args[0] = target;
    cdata.args[1] = id.toStringOrSymbol(scope.engine);
    cdata.args[2] = *receiver;

    ScopedValue trapResult(scope, static_cast<const FunctionObject *>(trap.ptr)->call(cdata));
    if (scope.engine->hasException)
        return Encode::undefined();
    ScopedProperty targetDesc(scope);
    PropertyAttributes attributes = target->getOwnProperty(id, targetDesc);
    if (attributes != Attr_Invalid && !attributes.isConfigurable()) {
        if (attributes.isData() && !attributes.isWritable()) {
            if (!trapResult->sameValue(targetDesc->value))
                return scope.engine->throwTypeError();
        }
        if (attributes.isAccessor() && targetDesc->value.isUndefined()) {
            if (!trapResult->isUndefined())
                return scope.engine->throwTypeError();
        }
   }
    return trapResult->asReturnedValue();
}

bool ProxyObject::virtualPut(Managed *m, PropertyKey id, const Value &value, Value *receiver)
{
    Scope scope(m);
    const ProxyObject *o = static_cast<const ProxyObject *>(m);
    if (!o->d()->handler)
        return scope.engine->throwTypeError();

    ScopedObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedValue trap(scope, handler->get(scope.engine->id_set()));
    if (scope.hasException())
        return Encode::undefined();
    if (trap->isNullOrUndefined())
        return target->put(id, value, receiver);
    if (!trap->isFunctionObject())
        return scope.engine->throwTypeError();

    JSCallData cdata(scope, 4, nullptr, handler);
    cdata.args[0] = target;
    cdata.args[1] = id.toStringOrSymbol(scope.engine);
    cdata.args[2] = value;
    cdata.args[3] = *receiver;

    ScopedValue trapResult(scope, static_cast<const FunctionObject *>(trap.ptr)->call(cdata));
    if (scope.engine->hasException || !trapResult->toBoolean())
        return false;
    ScopedProperty targetDesc(scope);
    PropertyAttributes attributes = target->getOwnProperty(id, targetDesc);
    if (attributes != Attr_Invalid && !attributes.isConfigurable()) {
        if (attributes.isData() && !attributes.isWritable()) {
            if (!value.sameValue(targetDesc->value))
                return scope.engine->throwTypeError();
        }
        if (attributes.isAccessor() && targetDesc->set.isUndefined())
            return scope.engine->throwTypeError();
    }
    return true;
}

bool ProxyObject::virtualDeleteProperty(Managed *m, PropertyKey id)
{
    Scope scope(m);
    const ProxyObject *o = static_cast<const ProxyObject *>(m);
    if (!o->d()->handler)
        return scope.engine->throwTypeError();

    ScopedObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedString deleteProp(scope, scope.engine->newString(QStringLiteral("deleteProperty")));
    ScopedValue trap(scope, handler->get(deleteProp));
    if (scope.hasException())
        return Encode::undefined();
    if (trap->isNullOrUndefined())
        return target->deleteProperty(id);
    if (!trap->isFunctionObject())
        return scope.engine->throwTypeError();

    JSCallData cdata(scope, 3, nullptr, handler);
    cdata.args[0] = target;
    cdata.args[1] = id.toStringOrSymbol(scope.engine);
    cdata.args[2] = o->d(); // ### fix receiver handling

    ScopedValue trapResult(scope, static_cast<const FunctionObject *>(trap.ptr)->call(cdata));
    if (scope.engine->hasException || !trapResult->toBoolean())
        return false;
    ScopedProperty targetDesc(scope);
    PropertyAttributes attributes = target->getOwnProperty(id, targetDesc);
    if (attributes == Attr_Invalid)
        return true;
    if (!attributes.isConfigurable())
        return scope.engine->throwTypeError();
    return true;
}

bool ProxyObject::virtualHasProperty(const Managed *m, PropertyKey id)
{
    Scope scope(m);
    const ProxyObject *o = static_cast<const ProxyObject *>(m);
    if (!o->d()->handler)
        return scope.engine->throwTypeError();

    ScopedObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedString hasProp(scope, scope.engine->newString(QStringLiteral("has")));
    ScopedValue trap(scope, handler->get(hasProp));
    if (scope.hasException())
        return Encode::undefined();
    if (trap->isNullOrUndefined())
        return target->hasProperty(id);
    if (!trap->isFunctionObject())
        return scope.engine->throwTypeError();

    JSCallData cdata(scope, 2, nullptr, handler);
    cdata.args[0] = target;
    cdata.args[1] = id.isArrayIndex() ? Value::fromUInt32(id.asArrayIndex()).toString(scope.engine) : id.asStringOrSymbol();

    ScopedValue trapResult(scope, static_cast<const FunctionObject *>(trap.ptr)->call(cdata));
    if (scope.engine->hasException)
        return false;
    bool result = trapResult->toBoolean();
    if (!result) {
        ScopedProperty targetDesc(scope);
        PropertyAttributes attributes = target->getOwnProperty(id, targetDesc);
        if (attributes != Attr_Invalid) {
            if (!attributes.isConfigurable() || !target->isExtensible())
                return scope.engine->throwTypeError();
        }
    }
    return result;
}

PropertyAttributes ProxyObject::virtualGetOwnProperty(const Managed *m, PropertyKey id, Property *p)
{
    Scope scope(m);
    const ProxyObject *o = static_cast<const ProxyObject *>(m);
    if (!o->d()->handler) {
        scope.engine->throwTypeError();
        return Attr_Invalid;
    }

    ScopedObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedString deleteProp(scope, scope.engine->newString(QStringLiteral("getOwnPropertyDescriptor")));
    ScopedValue trap(scope, handler->get(deleteProp));
    if (scope.hasException())
        return Attr_Invalid;
    if (trap->isNullOrUndefined())
        return target->getOwnProperty(id, p);
    if (!trap->isFunctionObject()) {
        scope.engine->throwTypeError();
        return Attr_Invalid;
    }

    JSCallData cdata(scope, 2, nullptr, handler);
    cdata.args[0] = target;
    cdata.args[1] = id.isArrayIndex() ? Value::fromUInt32(id.asArrayIndex()).toString(scope.engine) : id.asStringOrSymbol();

    ScopedValue trapResult(scope, static_cast<const FunctionObject *>(trap.ptr)->call(cdata));
    if (scope.engine->hasException)
        return Attr_Invalid;
    if (!trapResult->isObject() && !trapResult->isUndefined()) {
        scope.engine->throwTypeError();
        return Attr_Invalid;
    }

    ScopedProperty targetDesc(scope);
    PropertyAttributes targetAttributes = target->getOwnProperty(id, targetDesc);
    if (trapResult->isUndefined()) {
        p->value = Encode::undefined();
        if (targetAttributes == Attr_Invalid) {
            p->value = Encode::undefined();
            return Attr_Invalid;
        }
        if (!targetAttributes.isConfigurable() || !target->isExtensible()) {
            scope.engine->throwTypeError();
            return Attr_Invalid;
        }
        return Attr_Invalid;
    }

    //bool extensibleTarget = target->isExtensible();
    ScopedProperty resultDesc(scope);
    PropertyAttributes resultAttributes;
    ObjectPrototype::toPropertyDescriptor(scope.engine, trapResult, resultDesc, &resultAttributes);
    resultDesc->completed(&resultAttributes);

    if (!targetDesc->isCompatible(targetAttributes, resultDesc, resultAttributes)) {
        scope.engine->throwTypeError();
        return Attr_Invalid;
    }

    if (!resultAttributes.isConfigurable()) {
        if (targetAttributes == Attr_Invalid || targetAttributes.isConfigurable()) {
            scope.engine->throwTypeError();
            return Attr_Invalid;
        }
    }

    p->value = resultDesc->value;
    p->set = resultDesc->set;
    return resultAttributes;
}

bool ProxyObject::virtualDefineOwnProperty(Managed *m, PropertyKey id, const Property *p, PropertyAttributes attrs)
{
    Scope scope(m);
    const ProxyObject *o = static_cast<const ProxyObject *>(m);
    if (!o->d()->handler) {
        scope.engine->throwTypeError();
        return false;
    }

    ScopedObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedString prop(scope, scope.engine->newString(QStringLiteral("defineProperty")));
    ScopedValue trap(scope, handler->get(prop));
    if (scope.hasException())
        return false;
    if (trap->isNullOrUndefined())
        return target->defineOwnProperty(id, p, attrs);
    if (!trap->isFunctionObject()) {
        scope.engine->throwTypeError();
        return false;
    }

    JSCallData cdata(scope, 3, nullptr, handler);
    cdata.args[0] = target;
    cdata.args[1] = id.isArrayIndex() ? Value::fromUInt32(id.asArrayIndex()).toString(scope.engine) : id.asStringOrSymbol();
    cdata.args[2] = ObjectPrototype::fromPropertyDescriptor(scope.engine, p, attrs);

    ScopedValue trapResult(scope, static_cast<const FunctionObject *>(trap.ptr)->call(cdata));
    bool result = !scope.engine->hasException && trapResult->toBoolean();
    if (!result)
        return false;

    ScopedProperty targetDesc(scope);
    PropertyAttributes targetAttributes = target->getOwnProperty(id, targetDesc);
    bool extensibleTarget = target->isExtensible();
    bool settingConfigFalse = attrs.hasConfigurable() && !attrs.isConfigurable();
    if (targetAttributes == Attr_Invalid) {
        if (!extensibleTarget || settingConfigFalse) {
            scope.engine->throwTypeError();
            return false;
        }
    } else {
        if (!targetDesc->isCompatible(targetAttributes, p, attrs)) {
            scope.engine->throwTypeError();
            return false;
        }
        if (settingConfigFalse && targetAttributes.isConfigurable()) {
            scope.engine->throwTypeError();
            return false;
        }
    }

    return true;
}

bool ProxyObject::virtualIsExtensible(const Managed *m)
{
    Scope scope(m);
    const ProxyObject *o = static_cast<const ProxyObject *>(m);
    if (!o->d()->handler)
        return scope.engine->throwTypeError();

    ScopedObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedString hasProp(scope, scope.engine->newString(QStringLiteral("isExtensible")));
    ScopedValue trap(scope, handler->get(hasProp));
    if (scope.hasException())
        return Encode::undefined();
    if (trap->isNullOrUndefined())
        return target->isExtensible();
    if (!trap->isFunctionObject())
        return scope.engine->throwTypeError();

    JSCallData cdata(scope, 1, nullptr, handler);
    cdata.args[0] = target;

    ScopedValue trapResult(scope, static_cast<const FunctionObject *>(trap.ptr)->call(cdata));
    if (scope.engine->hasException)
        return false;
    bool result = trapResult->toBoolean();
    if (result != target->isExtensible()) {
        scope.engine->throwTypeError();
        return false;
    }
    return result;
}

bool ProxyObject::virtualPreventExtensions(Managed *m)
{
    Scope scope(m);
    const ProxyObject *o = static_cast<const ProxyObject *>(m);
    if (!o->d()->handler)
        return scope.engine->throwTypeError();

    ScopedObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedString hasProp(scope, scope.engine->newString(QStringLiteral("preventExtensions")));
    ScopedValue trap(scope, handler->get(hasProp));
    if (scope.hasException())
        return Encode::undefined();
    if (trap->isNullOrUndefined())
        return target->preventExtensions();
    if (!trap->isFunctionObject())
        return scope.engine->throwTypeError();

    JSCallData cdata(scope, 1, nullptr, handler);
    cdata.args[0] = target;

    ScopedValue trapResult(scope, static_cast<const FunctionObject *>(trap.ptr)->call(cdata));
    if (scope.engine->hasException)
        return false;
    bool result = trapResult->toBoolean();
    if (result && target->isExtensible()) {
        scope.engine->throwTypeError();
        return false;
    }
    return result;
}

Heap::Object *ProxyObject::virtualGetPrototypeOf(const Managed *m)
{
    Scope scope(m);
    const ProxyObject *o = static_cast<const ProxyObject *>(m);
    if (!o->d()->handler) {
        scope.engine->throwTypeError();
        return nullptr;
    }

    ScopedObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedString name(scope, scope.engine->newString(QStringLiteral("getPrototypeOf")));
    ScopedValue trap(scope, handler->get(name));
    if (scope.hasException())
        return nullptr;
    if (trap->isNullOrUndefined())
        return target->getPrototypeOf();
    if (!trap->isFunctionObject()) {
        scope.engine->throwTypeError();
        return nullptr;
    }

    JSCallData cdata(scope, 1, nullptr, handler);
    cdata.args[0] = target;

    ScopedValue trapResult(scope, static_cast<const FunctionObject *>(trap.ptr)->call(cdata));
    if (scope.engine->hasException)
        return nullptr;
    if (!trapResult->isNull() && !trapResult->isObject()) {
        scope.engine->throwTypeError();
        return nullptr;
    }
    Heap::Object *proto = trapResult->isNull() ? nullptr : static_cast<Heap::Object *>(trapResult->heapObject());
    if (!target->isExtensible()) {
        Heap::Object *targetProto = target->getPrototypeOf();
        if (proto != targetProto) {
            scope.engine->throwTypeError();
            return nullptr;
        }
    }
    return proto;
}

bool ProxyObject::virtualSetPrototypeOf(Managed *m, const Object *p)
{
    Scope scope(m);
    const ProxyObject *o = static_cast<const ProxyObject *>(m);
    if (!o->d()->handler) {
        scope.engine->throwTypeError();
        return false;
    }

    ScopedObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedString name(scope, scope.engine->newString(QStringLiteral("setPrototypeOf")));
    ScopedValue trap(scope, handler->get(name));
    if (scope.hasException())
        return false;
    if (trap->isNullOrUndefined())
        return target->setPrototypeOf(p);
    if (!trap->isFunctionObject()) {
        scope.engine->throwTypeError();
        return false;
    }

    JSCallData cdata(scope, 2, nullptr, handler);
    cdata.args[0] = target;
    cdata.args[1] = p ? p->asReturnedValue() : Encode::null();

    ScopedValue trapResult(scope, static_cast<const FunctionObject *>(trap.ptr)->call(cdata));
    bool result = !scope.engine->hasException && trapResult->toBoolean();
    if (!result)
        return false;
    if (!target->isExtensible()) {
        Heap::Object *targetProto = target->getPrototypeOf();
        if (p->d() != targetProto) {
            scope.engine->throwTypeError();
            return false;
        }
    }
    return true;
}

struct ProxyObjectOwnPropertyKeyIterator : OwnPropertyKeyIterator
{
    PersistentValue ownKeys;
    uint index = 0;
    uint len = 0;

    ProxyObjectOwnPropertyKeyIterator(ArrayObject *keys);
    ~ProxyObjectOwnPropertyKeyIterator() override = default;
    PropertyKey next(const Object *o, Property *pd = nullptr, PropertyAttributes *attrs = nullptr) override;

};

ProxyObjectOwnPropertyKeyIterator::ProxyObjectOwnPropertyKeyIterator(ArrayObject *keys)
{
    ownKeys = keys;
    len = keys->getLength();
}

PropertyKey ProxyObjectOwnPropertyKeyIterator::next(const Object *m, Property *pd, PropertyAttributes *attrs)
{
    if (index >= len)
        return PropertyKey::invalid();

    Scope scope(m);
    ScopedObject keys(scope, ownKeys.asManaged());
    PropertyKey key = PropertyKey::fromId(keys->get(PropertyKey::fromArrayIndex(index)));
    ++index;

    if (pd || attrs) {
        ScopedProperty p(scope);
        PropertyAttributes a = const_cast<Object *>(m)->getOwnProperty(key, pd ? pd : p);
        if (attrs)
            *attrs = a;
    }

    return key;
}

static bool removeAllOccurrences(ArrayObject *target, ReturnedValue val) {
    uint len = target->getLength();
    bool found = false;
    for (uint i = 0; i < len; ++i) {
        ReturnedValue v = target->get(i);
        if (v == val) {
            found = true;
            target->put(i, Value::undefinedValue());
        }
    }
    return  found;
}

OwnPropertyKeyIterator *ProxyObject::virtualOwnPropertyKeys(const Object *m, Value *iteratorTarget)
{
    Scope scope(m);
    const ProxyObject *o = static_cast<const ProxyObject *>(m);
    if (!o->d()->handler) {
        scope.engine->throwTypeError();
        return nullptr;
    }

    ScopedObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedString name(scope, scope.engine->newString(QStringLiteral("ownKeys")));
    ScopedValue trap(scope, handler->get(name));

    if (scope.hasException())
        return nullptr;
    if (trap->isUndefined())
        return target->ownPropertyKeys(iteratorTarget);
    if (!trap->isFunctionObject()) {
        scope.engine->throwTypeError();
        return nullptr;
    }

    JSCallData cdata(scope, 1, nullptr, handler);
    cdata.args[0] = target;
    ScopedObject trapResult(scope, static_cast<const FunctionObject *>(trap.ptr)->call(cdata));
    if (scope.engine->hasException)
        return nullptr;
    if (!trapResult) {
        scope.engine->throwTypeError();
        return nullptr;
    }

    uint len = trapResult->getLength();
    ScopedArrayObject trapKeys(scope, scope.engine->newArrayObject());
    ScopedStringOrSymbol key(scope);
    for (uint i = 0; i < len; ++i) {
        key = trapResult->get(i);
        if (scope.engine->hasException)
            return nullptr;
        if (!key) {
            scope.engine->throwTypeError();
            return nullptr;
        }
        Value keyAsValue = Value::fromReturnedValue(key->toPropertyKey().id());
        trapKeys->push_back(keyAsValue);
    }

    ScopedArrayObject targetConfigurableKeys(scope, scope.engine->newArrayObject());
    ScopedArrayObject targetNonConfigurableKeys(scope, scope.engine->newArrayObject());
    ObjectIterator it(scope, target, ObjectIterator::EnumerableOnly);
    ScopedPropertyKey k(scope);
    while (1) {
        PropertyAttributes attrs;
        k = it.next(nullptr, &attrs);
        if (!k->isValid())
            break;
        Value keyAsValue = Value::fromReturnedValue(k->id());
        if (attrs.isConfigurable())
            targetConfigurableKeys->push_back(keyAsValue);
        else
            targetNonConfigurableKeys->push_back(keyAsValue);
    }
    if (target->isExtensible() && targetNonConfigurableKeys->getLength() == 0)
        return new ProxyObjectOwnPropertyKeyIterator(trapKeys);

    ScopedArrayObject uncheckedResultKeys(scope, scope.engine->newArrayObject());
    uncheckedResultKeys->copyArrayData(trapKeys);

    len = targetNonConfigurableKeys->getLength();
    for (uint i = 0; i < len; ++i) {
        k = PropertyKey::fromId(targetNonConfigurableKeys->get(i));
        if (!removeAllOccurrences(uncheckedResultKeys, k->id())) {
            scope.engine->throwTypeError();
            return nullptr;
        }
    }

    if (target->isExtensible())
        return new ProxyObjectOwnPropertyKeyIterator(trapKeys);

    len = targetConfigurableKeys->getLength();
    for (uint i = 0; i < len; ++i) {
        k = PropertyKey::fromId(targetConfigurableKeys->get(i));
        if (!removeAllOccurrences(uncheckedResultKeys, k->id())) {
            scope.engine->throwTypeError();
            return nullptr;
        }
    }

    len = uncheckedResultKeys->getLength();
    for (uint i = 0; i < len; ++i) {
        if (uncheckedResultKeys->get(i) != Encode::undefined()) {
            scope.engine->throwTypeError();
            return nullptr;
        }
    }

    *iteratorTarget = *m;
    return new ProxyObjectOwnPropertyKeyIterator(trapKeys);
}


ReturnedValue ProxyFunctionObject::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    Scope scope(f);
    const ProxyObject *o = static_cast<const ProxyObject *>(f);
    if (!o->d()->handler)
        return scope.engine->throwTypeError();

    ScopedFunctionObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedString name(scope, scope.engine->newString(QStringLiteral("construct")));
    ScopedValue trap(scope, handler->get(name));

    if (scope.hasException())
        return Encode::undefined();
    if (trap->isNullOrUndefined()) {
        Q_ASSERT(target->isConstructor());
        return target->callAsConstructor(argv, argc, newTarget);
    }
    if (!trap->isFunctionObject())
        return scope.engine->throwTypeError();

    ScopedFunctionObject trapFunction(scope, trap);
    Value *arguments = scope.alloc(3);
    arguments[0] = target;
    arguments[1] = scope.engine->newArrayObject(argv, argc);
    arguments[2] = newTarget ? *newTarget : Value::undefinedValue();
    ScopedObject result(scope, trapFunction->call(handler, arguments, 3));

    if (!result)
        return scope.engine->throwTypeError();
    return result->asReturnedValue();
}

ReturnedValue ProxyFunctionObject::virtualCall(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(f);

    const ProxyObject *o = static_cast<const ProxyObject *>(f);
    if (!o->d()->handler)
        return scope.engine->throwTypeError();

    ScopedFunctionObject target(scope, o->d()->target);
    Q_ASSERT(target);
    ScopedObject handler(scope, o->d()->handler);
    ScopedString name(scope, scope.engine->newString(QStringLiteral("apply")));
    ScopedValue trap(scope, handler->get(name));

    if (scope.hasException())
        return Encode::undefined();
    if (trap->isNullOrUndefined())
        return checkedResult(scope.engine, target->call(thisObject, argv, argc));
    if (!trap->isFunctionObject())
        return scope.engine->throwTypeError();

    ScopedFunctionObject trapFunction(scope, trap);
    Value *arguments = scope.alloc(3);
    arguments[0] = target;
    arguments[1] = thisObject ? *thisObject : Value::undefinedValue();
    arguments[2] = scope.engine->newArrayObject(argv, argc);
    return trapFunction->call(handler, arguments, 3);
}

DEFINE_OBJECT_VTABLE(Proxy);

void Heap::Proxy::init(QV4::ExecutionContext *ctx)
{
    Heap::FunctionObject::init(ctx, QStringLiteral("Proxy"));

    Scope scope(ctx);
    Scoped<QV4::Proxy> ctor(scope, this);
    ctor->defineDefaultProperty(QStringLiteral("revocable"), QV4::Proxy::method_revocable, 2);
    ctor->defineReadonlyConfigurableProperty(scope.engine->id_length(), Value::fromInt32(2));
}

ReturnedValue Proxy::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *)
{
    Scope scope(f);
    if (argc < 2 || !argv[0].isObject() || !argv[1].isObject())
        return scope.engine->throwTypeError();

    const Object *target = static_cast<const Object *>(argv);
    const Object *handler = static_cast<const Object *>(argv + 1);
    if (const ProxyObject *ptarget = target->as<ProxyObject>())
        if (!ptarget->d()->handler)
            return scope.engine->throwTypeError();
    if (const ProxyObject *phandler = handler->as<ProxyObject>())
        if (!phandler->d()->handler)
            return scope.engine->throwTypeError();

    const FunctionObject *targetFunction = target->as<FunctionObject>();
    if (targetFunction)
        return scope.engine->memoryManager->allocate<ProxyFunctionObject>(targetFunction, handler)->asReturnedValue();
    return scope.engine->memoryManager->allocate<ProxyObject>(target, handler)->asReturnedValue();
}

ReturnedValue Proxy::virtualCall(const FunctionObject *f, const Value *, const Value *, int)
{
    return f->engine()->throwTypeError();
}

ReturnedValue Proxy::method_revocable(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    ScopedObject proxy(scope, Proxy::virtualCallAsConstructor(f, argv, argc, f));
    if (scope.hasException())
        return Encode::undefined();
    Q_ASSERT(proxy);

    ScopedString revoke(scope, scope.engine->newString(QStringLiteral("revoke")));
    ScopedFunctionObject revoker(scope, scope.engine->memoryManager->allocate<FunctionObject>(scope.engine->rootContext(), nullptr, method_revoke));
    revoker->defineReadonlyConfigurableProperty(scope.engine->id_length(), Value::fromInt32(0));
    revoker->defineDefaultProperty(scope.engine->symbol_revokableProxy(), proxy);

    ScopedObject o(scope, scope.engine->newObject());
    ScopedString p(scope, scope.engine->newString(QStringLiteral("proxy")));
    o->defineDefaultProperty(p, proxy);
    o->defineDefaultProperty(revoke, revoker);
    return o->asReturnedValue();
}

ReturnedValue Proxy::method_revoke(const FunctionObject *f, const Value *, const Value *, int)
{
    Scope scope(f);
    ScopedObject o(scope, f->get(scope.engine->symbol_revokableProxy()));
    Q_ASSERT(o);
    ProxyObject *proxy = o->cast<ProxyObject>();

    proxy->d()->target.set(scope.engine, nullptr);
    proxy->d()->handler.set(scope.engine, nullptr);
    return Encode::undefined();
}
