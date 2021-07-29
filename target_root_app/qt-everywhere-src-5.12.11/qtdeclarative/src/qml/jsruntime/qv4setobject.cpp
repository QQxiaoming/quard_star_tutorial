/****************************************************************************
**
** Copyright (C) 2018 Crimson AS <info@crimson.no>
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


#include "qv4setobject_p.h"
#include "qv4setiterator_p.h"
#include "qv4estable_p.h"
#include "qv4symbol_p.h"

using namespace QV4;

DEFINE_OBJECT_VTABLE(SetCtor);
DEFINE_OBJECT_VTABLE(WeakSetCtor);
DEFINE_OBJECT_VTABLE(SetObject);

void Heap::WeakSetCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("WeakSet"));
}

void Heap::SetCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("Set"));
}

ReturnedValue WeakSetCtor::construct(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget, bool isWeak)
{
    Scope scope(f);
    Scoped<SetObject> a(scope, scope.engine->memoryManager->allocate<SetObject>());
    bool protoSet = false;
    if (newTarget)
        protoSet = a->setProtoFromNewTarget(newTarget);
    if (!protoSet && isWeak)
        a->setPrototypeOf(scope.engine->weakSetPrototype());
    a->d()->isWeakSet = isWeak;

    if (argc > 0) {
        ScopedValue iterable(scope, argv[0]);
        if (!iterable->isUndefined() && !iterable->isNull()) {
            ScopedFunctionObject adder(scope, a->get(ScopedString(scope, scope.engine->newString(QString::fromLatin1("add")))));
            if (!adder)
                return scope.engine->throwTypeError();
            ScopedObject iter(scope, Runtime::method_getIterator(scope.engine, iterable, true));
            CHECK_EXCEPTION();
            if (!iter)
                return a.asReturnedValue();

            Value *nextValue = scope.alloc(1);
            ScopedValue done(scope);
            forever {
                done = Runtime::method_iteratorNext(scope.engine, iter, nextValue);
                CHECK_EXCEPTION();
                if (done->toBoolean())
                    return a.asReturnedValue();

                adder->call(a, nextValue, 1);
                if (scope.engine->hasException) {
                    ScopedValue falsey(scope, Encode(false));
                    return Runtime::method_iteratorClose(scope.engine, iter, falsey);
                }
            }
        }
    }

    return a.asReturnedValue();
}

ReturnedValue WeakSetCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    return construct(f, argv, argc, newTarget, true);
}

ReturnedValue WeakSetCtor::virtualCall(const FunctionObject *f, const Value *, const Value *, int)
{
    Scope scope(f);
    return scope.engine->throwTypeError(QString::fromLatin1("Set requires new"));
}

ReturnedValue SetCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    return construct(f, argv, argc, newTarget, false);
}

void WeakSetPrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(0));
    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));
    defineDefaultProperty(engine->id_constructor(), (o = ctor));

    defineDefaultProperty(QStringLiteral("add"), method_add, 1);
    defineDefaultProperty(QStringLiteral("delete"), method_delete, 1);
    defineDefaultProperty(QStringLiteral("has"), method_has, 1);

    ScopedString val(scope, engine->newString(QLatin1String("WeakSet")));
    defineReadonlyConfigurableProperty(engine->symbol_toStringTag(), val);
}

ReturnedValue WeakSetPrototype::method_add(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<SetObject> that(scope, thisObject);
    if ((!that || !that->d()->isWeakSet) ||
        (!argc || !argv[0].isObject()))
        return scope.engine->throwTypeError();

    that->d()->esTable->set(argv[0], Value::undefinedValue());
    return that.asReturnedValue();
}

ReturnedValue WeakSetPrototype::method_delete(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<SetObject> that(scope, thisObject);
    if (!that || !that->d()->isWeakSet)
        return scope.engine->throwTypeError();
    if (!argc || !argv[0].isObject())
        return Encode(false);

    return Encode(that->d()->esTable->remove(argv[0]));
}

ReturnedValue WeakSetPrototype::method_has(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<SetObject> that(scope, thisObject);
    if (!that || !that->d()->isWeakSet)
        return scope.engine->throwTypeError();
    if (!argc || !argv[0].isObject())
        return Encode(false);

    return Encode(that->d()->esTable->has(argv[0]));
}

void SetPrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(0));
    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));
    ctor->addSymbolSpecies();
    defineDefaultProperty(engine->id_constructor(), (o = ctor));

    defineDefaultProperty(QStringLiteral("add"), method_add, 1);
    defineDefaultProperty(QStringLiteral("clear"), method_clear, 0);
    defineDefaultProperty(QStringLiteral("delete"), method_delete, 1);
    defineDefaultProperty(QStringLiteral("entries"), method_entries, 0);
    defineDefaultProperty(QStringLiteral("forEach"), method_forEach, 1);
    defineDefaultProperty(QStringLiteral("has"), method_has, 1);
    defineAccessorProperty(QStringLiteral("size"), method_get_size, nullptr);

    // Per the spec, the value for 'keys' is the same as 'values'.
    ScopedString valString(scope, scope.engine->newIdentifier(QStringLiteral("values")));
    ScopedFunctionObject valuesFn(scope, FunctionObject::createBuiltinFunction(engine, valString, SetPrototype::method_values, 0));
    defineDefaultProperty(QStringLiteral("keys"), valuesFn);
    defineDefaultProperty(QStringLiteral("values"), valuesFn);

    defineDefaultProperty(engine->symbol_iterator(), valuesFn);

    ScopedString val(scope, engine->newString(QLatin1String("Set")));
    defineReadonlyConfigurableProperty(engine->symbol_toStringTag(), val);
}

void Heap::SetObject::init()
{
    Object::init();
    esTable = new ESTable();
}

void Heap::SetObject::destroy()
{
    delete esTable;
    esTable = 0;
}

void Heap::SetObject::removeUnmarkedKeys()
{
    esTable->removeUnmarkedKeys();
}

void Heap::SetObject::markObjects(Heap::Base *that, MarkStack *markStack)
{
    SetObject *s = static_cast<SetObject *>(that);
    s->esTable->markObjects(markStack, s->isWeakSet);
    Object::markObjects(that, markStack);
}

ReturnedValue SetPrototype::method_add(const FunctionObject *b, const Value *thisObject, const Value *argv, int)
{
    Scope scope(b);
    Scoped<SetObject> that(scope, thisObject);
    if (!that || that->d()->isWeakSet)
        return scope.engine->throwTypeError();

    that->d()->esTable->set(argv[0], Value::undefinedValue());
    return that.asReturnedValue();
}

ReturnedValue SetPrototype::method_clear(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<SetObject> that(scope, thisObject);
    if (!that || that->d()->isWeakSet)
        return scope.engine->throwTypeError();

    that->d()->esTable->clear();
    return Encode::undefined();
}

ReturnedValue SetPrototype::method_delete(const FunctionObject *b, const Value *thisObject, const Value *argv, int)
{
    Scope scope(b);
    Scoped<SetObject> that(scope, thisObject);
    if (!that || that->d()->isWeakSet)
        return scope.engine->throwTypeError();

    return Encode(that->d()->esTable->remove(argv[0]));
}

ReturnedValue SetPrototype::method_entries(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<SetObject> that(scope, thisObject);
    if (!that || that->d()->isWeakSet)
        return scope.engine->throwTypeError();

    Scoped<SetIteratorObject> ao(scope, scope.engine->newSetIteratorObject(that));
    ao->d()->iterationKind = IteratorKind::KeyValueIteratorKind;
    return ao->asReturnedValue();
}

ReturnedValue SetPrototype::method_forEach(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<SetObject> that(scope, thisObject);
    if (!that || that->d()->isWeakSet)
        return scope.engine->throwTypeError();

    ScopedFunctionObject callbackfn(scope, argv[0]);
    if (!callbackfn)
        return scope.engine->throwTypeError();

    ScopedValue thisArg(scope, Value::undefinedValue());
    if (argc > 1)
        thisArg = ScopedValue(scope, argv[1]);

    Value *arguments = scope.alloc(3);
    for (uint i = 0; i < that->d()->esTable->size(); ++i) {
        that->d()->esTable->iterate(i, &arguments[0], &arguments[1]); // fill in key (0), value (1)
        arguments[1] = arguments[0]; // but for set, we want to return the key twice; value is always undefined.

        arguments[2] = that;
        callbackfn->call(thisArg, arguments, 3);
        CHECK_EXCEPTION();
    }
    return Encode::undefined();
}

ReturnedValue SetPrototype::method_has(const FunctionObject *b, const Value *thisObject, const Value *argv, int)
{
    Scope scope(b);
    Scoped<SetObject> that(scope, thisObject);
    if (!that || that->d()->isWeakSet)
        return scope.engine->throwTypeError();

    return Encode(that->d()->esTable->has(argv[0]));
}

ReturnedValue SetPrototype::method_get_size(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<SetObject> that(scope, thisObject);
    if (!that || that->d()->isWeakSet)
        return scope.engine->throwTypeError();

    return Encode(that->d()->esTable->size());
}

ReturnedValue SetPrototype::method_values(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<SetObject> that(scope, thisObject);
    if (!that || that->d()->isWeakSet)
        return scope.engine->throwTypeError();

    Scoped<SetIteratorObject> ao(scope, scope.engine->newSetIteratorObject(that));
    ao->d()->iterationKind = IteratorKind::ValueIteratorKind;
    return ao->asReturnedValue();
}
