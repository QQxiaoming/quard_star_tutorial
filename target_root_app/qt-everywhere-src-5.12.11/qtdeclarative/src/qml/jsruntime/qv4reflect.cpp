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

#include "qv4reflect_p.h"
#include "qv4symbol_p.h"
#include "qv4runtimeapi_p.h"
#include "qv4objectproto_p.h"
#include "qv4propertykey_p.h"
#include "qv4objectiterator_p.h"

using namespace QV4;

DEFINE_OBJECT_VTABLE(Reflect);

void Heap::Reflect::init()
{
    Object::init();
    Scope scope(internalClass->engine);
    ScopedObject r(scope, this);

    r->defineDefaultProperty(QStringLiteral("apply"), QV4::Reflect::method_apply, 3);
    r->defineDefaultProperty(QStringLiteral("construct"), QV4::Reflect::method_construct, 2);
    r->defineDefaultProperty(QStringLiteral("defineProperty"), QV4::Reflect::method_defineProperty, 3);
    r->defineDefaultProperty(QStringLiteral("deleteProperty"), QV4::Reflect::method_deleteProperty, 2);
    r->defineDefaultProperty(QStringLiteral("get"), QV4::Reflect::method_get, 2);
    r->defineDefaultProperty(QStringLiteral("getOwnPropertyDescriptor"), QV4::Reflect::method_getOwnPropertyDescriptor, 2);
    r->defineDefaultProperty(QStringLiteral("getPrototypeOf"), QV4::Reflect::method_getPrototypeOf, 1);
    r->defineDefaultProperty(QStringLiteral("has"), QV4::Reflect::method_has, 2);
    r->defineDefaultProperty(QStringLiteral("isExtensible"), QV4::Reflect::method_isExtensible, 1);
    r->defineDefaultProperty(QStringLiteral("ownKeys"), QV4::Reflect::method_ownKeys, 1);
    r->defineDefaultProperty(QStringLiteral("preventExtensions"), QV4::Reflect::method_preventExtensions, 1);
    r->defineDefaultProperty(QStringLiteral("set"), QV4::Reflect::method_set, 3);
    r->defineDefaultProperty(QStringLiteral("setPrototypeOf"), QV4::Reflect::method_setPrototypeOf, 2);
}

struct CallArgs {
    Value *argv;
    int argc;
};

static CallArgs createListFromArrayLike(Scope &scope, const Object *o)
{
    int len = o->getLength();
    Value *arguments = scope.alloc(len);

    for (int i = 0; i < len; ++i) {
        arguments[i] = o->get(i);
        if (scope.hasException())
            return { nullptr, 0 };
    }
    return { arguments, len };
}

ReturnedValue Reflect::method_apply(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    if (argc < 3 || !argv[0].isFunctionObject() || !argv[2].isObject())
        return scope.engine->throwTypeError();

    const Object *o = static_cast<const Object *>(argv + 2);
    CallArgs arguments = createListFromArrayLike(scope, o);
    if (scope.hasException())
        return Encode::undefined();

    return checkedResult(scope.engine, static_cast<const FunctionObject &>(argv[0]).call(
                &argv[1], arguments.argv, arguments.argc));
}

ReturnedValue Reflect::method_construct(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    if (argc < 2 || !argv[1].isObject())
        return scope.engine->throwTypeError();
    const FunctionObject *target = argv[0].as<FunctionObject>();
    const FunctionObject *newTarget = argc == 3 ? argv[2].as<FunctionObject>() : target;
    if (!target || !target->isConstructor() || !newTarget || !newTarget->isConstructor())
        return scope.engine->throwTypeError();

    const Object *o = static_cast<const Object *>(argv + 1);
    CallArgs arguments = createListFromArrayLike(scope, o);
    if (scope.hasException())
        return Encode::undefined();

    return target->callAsConstructor(arguments.argv, arguments.argc, newTarget);
}

ReturnedValue Reflect::method_defineProperty(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    if (!argc || !argv[0].isObject())
        return scope.engine->throwTypeError();

    ScopedObject O(scope, argv[0]);
    ScopedPropertyKey name(scope, (argc > 1 ? argv[1] : Value::undefinedValue()).toPropertyKey(scope.engine));
    if (scope.engine->hasException)
        return QV4::Encode::undefined();

    ScopedValue attributes(scope, argc > 2 ? argv[2] : Value::undefinedValue());
    ScopedProperty pd(scope);
    PropertyAttributes attrs;
    ObjectPrototype::toPropertyDescriptor(scope.engine, attributes, pd, &attrs);
    if (scope.engine->hasException)
        return QV4::Encode::undefined();

    bool result = O->defineOwnProperty(name, pd, attrs);

    return Encode(result);
}

ReturnedValue Reflect::method_deleteProperty(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    ExecutionEngine *e = f->engine();
    if (!argc || !argv[0].isObject())
        return e->throwTypeError();

    bool result =  Runtime::method_deleteProperty(e, argv[0], argc > 1 ? argv[1] : Value::undefinedValue());
    return Encode(result);
}

ReturnedValue Reflect::method_get(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    if (!argc || !argv[0].isObject())
        return scope.engine->throwTypeError();

    ScopedObject o(scope, static_cast<const Object *>(argv));
    Value undef = Value::undefinedValue();
    const Value *index = argc > 1 ? &argv[1] : &undef;
    ScopedPropertyKey name(scope, index->toPropertyKey(scope.engine));
    if (scope.hasException())
        return Encode::undefined();
    ScopedValue receiver(scope, argc > 2 ? argv[2] : *o);

    return Encode(o->get(name, receiver));
}

ReturnedValue Reflect::method_getOwnPropertyDescriptor(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    if (!argc || !argv[0].isObject())
        return f->engine()->throwTypeError();

    return ObjectPrototype::method_getOwnPropertyDescriptor(f, thisObject, argv, argc);
}

ReturnedValue Reflect::method_getPrototypeOf(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    if (!argc || !argv[0].isObject())
        return f->engine()->throwTypeError();

    const Object *o = static_cast<const Object *>(argv);
    Heap::Object *p = o->getPrototypeOf();
    return (p ? p->asReturnedValue() : Encode::null());
}

ReturnedValue Reflect::method_has(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    if (!argc || !argv[0].isObject())
        return scope.engine->throwTypeError();

    ScopedObject o(scope, static_cast<const Object *>(argv));
    Value undef = Value::undefinedValue();
    const Value *index = argc > 1 ? &argv[1] : &undef;

    ScopedPropertyKey name(scope, index->toPropertyKey(scope.engine));
    if (scope.engine->hasException)
        return false;

    return Encode(o->hasProperty(name));
}

ReturnedValue Reflect::method_isExtensible(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    if (!argc || !argv[0].isObject())
        return f->engine()->throwTypeError();

    const Object *o = static_cast<const Object *>(argv);
    return Encode(o->isExtensible());
}


ReturnedValue Reflect::method_ownKeys(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    if (!argc || !argv[0].isObject())
        return f->engine()->throwTypeError();

    Scope scope(f);
    if (!argc)
        return scope.engine->throwTypeError();

    ScopedObject O(scope, argv[0].toObject(scope.engine));
    if (!O)
        return Encode::undefined();

    ScopedArrayObject keys(scope, scope.engine->newArrayObject());

    ObjectIterator it(scope, O, ObjectIterator::WithSymbols);
    ScopedPropertyKey key(scope);
    ScopedValue v(scope);
    while (1) {
        key = it.next();
        if (!key->isValid())
            break;
        v = key->toStringOrSymbol(scope.engine);
        keys->push_back(v);
    }

    return keys->asReturnedValue();

}

ReturnedValue Reflect::method_preventExtensions(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    if (!argc || !argv[0].isObject())
        return scope.engine->throwTypeError();

    ScopedObject o(scope, static_cast<const Object *>(argv));
    return Encode(o->preventExtensions());
}

ReturnedValue Reflect::method_set(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    if (!argc || !argv[0].isObject())
        return scope.engine->throwTypeError();

    ScopedObject o(scope, static_cast<const Object *>(argv));
    Value undef = Value::undefinedValue();
    const Value *index = argc > 1 ? &argv[1] : &undef;
    const Value &val = argc > 2 ? argv[2] : undef;
    ScopedValue receiver(scope, argc >3 ? argv[3] : argv[0]);

    ScopedPropertyKey propertyKey(scope, index->toPropertyKey(scope.engine));
    if (scope.engine->hasException)
        return false;
    bool result = o->put(propertyKey, val, receiver);
    return Encode(result);
}

ReturnedValue Reflect::method_setPrototypeOf(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    if (argc < 2 || !argv[0].isObject() || (!argv[1].isNull() && !argv[1].isObject()))
        return f->engine()->throwTypeError();

    Scope scope(f);
    ScopedObject o(scope, static_cast<const Object *>(argv));
    const Object *proto = argv[1].isNull() ? nullptr : static_cast<const Object *>(argv + 1);
    return Encode(o->setPrototypeOf(proto));
}
