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

#include "qv4setobject_p.h" // ### temporary
#include "qv4mapobject_p.h"
#include "qv4mapiterator_p.h"
#include "qv4estable_p.h"
#include "qv4symbol_p.h"

using namespace QV4;

DEFINE_OBJECT_VTABLE(WeakMapCtor);
DEFINE_OBJECT_VTABLE(MapCtor);
DEFINE_OBJECT_VTABLE(MapObject);

void Heap::WeakMapCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("WeakMap"));
}

void Heap::MapCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("Map"));
}

ReturnedValue WeakMapCtor::construct(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget, bool weakMap)
{
    Scope scope(f);
    Scoped<MapObject> a(scope, scope.engine->memoryManager->allocate<MapObject>());
    bool protoSet = false;
    if (newTarget)
        protoSet = a->setProtoFromNewTarget(newTarget);
    if (!protoSet && weakMap) {
        a->setPrototypeOf(scope.engine->weakMapPrototype());
        scope.engine->memoryManager->registerWeakMap(a->d());
    }
    a->d()->isWeakMap = weakMap;

    if (argc > 0) {
        ScopedValue iterable(scope, argv[0]);

        if (!iterable->isNullOrUndefined()) {
            ScopedFunctionObject adder(scope, a->get(ScopedString(scope, scope.engine->newString(QString::fromLatin1("set")))));
            if (!adder)
                return scope.engine->throwTypeError();

            ScopedObject iter(scope, Runtime::method_getIterator(scope.engine, iterable, true));
            if (scope.hasException())
                return Encode::undefined();
            Q_ASSERT(iter);

            ScopedValue obj(scope);
            Value *arguments = scope.alloc(2);
            ScopedValue done(scope);
            forever {
                done = Runtime::method_iteratorNext(scope.engine, iter, obj);
                if (scope.hasException())
                    break;
                if (done->toBoolean())
                    return a->asReturnedValue();
                const Object *o = obj->objectValue();
                if (!o) {
                    scope.engine->throwTypeError();
                    break;
                }

                arguments[0] = o->get(PropertyKey::fromArrayIndex(0));
                if (scope.hasException())
                    break;
                arguments[1] = o->get(PropertyKey::fromArrayIndex(1));
                if (scope.hasException())
                    break;

                adder->call(a, arguments, 2);
                if (scope.hasException())
                    break;
            }
            ScopedValue falsey(scope, Encode(false));
            return Runtime::method_iteratorClose(scope.engine, iter, falsey);
        }
    }
    return a->asReturnedValue();

}

ReturnedValue WeakMapCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    return construct(f, argv, argc, newTarget, true);
}

ReturnedValue WeakMapCtor::virtualCall(const FunctionObject *f, const Value *, const Value *, int)
{
    Scope scope(f);
    return scope.engine->throwTypeError(QString::fromLatin1("(Weak)Map requires new"));
}


ReturnedValue MapCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    return construct(f, argv, argc, newTarget, false);
}

void WeakMapPrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(0));
    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));
    defineDefaultProperty(engine->id_constructor(), (o = ctor));

    defineDefaultProperty(QStringLiteral("delete"), method_delete, 1);
    defineDefaultProperty(QStringLiteral("get"), method_get, 1);
    defineDefaultProperty(QStringLiteral("has"), method_has, 1);
    defineDefaultProperty(QStringLiteral("set"), method_set, 2);

    ScopedString val(scope, engine->newString(QLatin1String("WeakMap")));
    defineReadonlyConfigurableProperty(engine->symbol_toStringTag(), val);
}


void MapPrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(0));
    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));
    ctor->addSymbolSpecies();
    defineDefaultProperty(engine->id_constructor(), (o = ctor));

    defineDefaultProperty(QStringLiteral("clear"), method_clear, 0);
    defineDefaultProperty(QStringLiteral("delete"), method_delete, 1);
    defineDefaultProperty(QStringLiteral("forEach"), method_forEach, 1);
    defineDefaultProperty(QStringLiteral("get"), method_get, 1);
    defineDefaultProperty(QStringLiteral("has"), method_has, 1);
    defineDefaultProperty(QStringLiteral("keys"), method_keys, 0);
    defineDefaultProperty(QStringLiteral("set"), method_set, 2);
    defineAccessorProperty(QStringLiteral("size"), method_get_size, nullptr);
    defineDefaultProperty(QStringLiteral("values"), method_values, 0);

    // Per the spec, the value for entries/@@iterator is the same
    ScopedString valString(scope, scope.engine->newIdentifier(QStringLiteral("entries")));
    ScopedFunctionObject entriesFn(scope, FunctionObject::createBuiltinFunction(engine, valString, MapPrototype::method_entries, 0));
    defineDefaultProperty(QStringLiteral("entries"), entriesFn);
    defineDefaultProperty(engine->symbol_iterator(), entriesFn);

    ScopedString val(scope, engine->newString(QLatin1String("Map")));
    defineReadonlyConfigurableProperty(engine->symbol_toStringTag(), val);
}

void Heap::MapObject::init()
{
    Object::init();
    esTable = new ESTable();
}

void Heap::MapObject::destroy()
{
    delete esTable;
    esTable = nullptr;
}

void Heap::MapObject::removeUnmarkedKeys()
{
    esTable->removeUnmarkedKeys();
}

void Heap::MapObject::markObjects(Heap::Base *that, MarkStack *markStack)
{
    MapObject *m = static_cast<MapObject *>(that);
    m->esTable->markObjects(markStack, m->isWeakMap);
    Object::markObjects(that, markStack);
}

ReturnedValue WeakMapPrototype::method_delete(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || !that->d()->isWeakMap)
        return scope.engine->throwTypeError();
    if (!argc || !argv[0].isObject())
        return Encode(false);

    return Encode(that->d()->esTable->remove(argv[0]));

}

ReturnedValue WeakMapPrototype::method_get(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || !that->d()->isWeakMap)
        return scope.engine->throwTypeError();
    if (!argc || !argv[0].isObject())
        return Encode::undefined();

    return that->d()->esTable->get(argv[0]);
}

ReturnedValue WeakMapPrototype::method_has(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || !that->d()->isWeakMap)
        return scope.engine->throwTypeError();
    if (!argc || !argv[0].isObject())
        return Encode(false);

    return Encode(that->d()->esTable->has(argv[0]));
}

ReturnedValue WeakMapPrototype::method_set(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if ((!that || !that->d()->isWeakMap) ||
        (!argc || !argv[0].isObject()))
        return scope.engine->throwTypeError();

    that->d()->esTable->set(argv[0], argc > 1 ? argv[1] : Value::undefinedValue());
    return that.asReturnedValue();
}


ReturnedValue MapPrototype::method_clear(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || that->d()->isWeakMap)
        return scope.engine->throwTypeError();

    that->d()->esTable->clear();
    return Encode::undefined();
}

ReturnedValue MapPrototype::method_delete(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || that->d()->isWeakMap)
        return scope.engine->throwTypeError();

    return Encode(that->d()->esTable->remove(argc ? argv[0] : Value::undefinedValue()));
}

ReturnedValue MapPrototype::method_entries(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || that->d()->isWeakMap)
        return scope.engine->throwTypeError();

    Scoped<MapIteratorObject> ao(scope, scope.engine->newMapIteratorObject(that));
    ao->d()->iterationKind = IteratorKind::KeyValueIteratorKind;
    return ao->asReturnedValue();
}

ReturnedValue MapPrototype::method_forEach(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || that->d()->isWeakMap)
        return scope.engine->throwTypeError();

    ScopedFunctionObject callbackfn(scope, argv[0]);
    if (!callbackfn)
        return scope.engine->throwTypeError();

    ScopedValue thisArg(scope, Value::undefinedValue());
    if (argc > 1)
        thisArg = ScopedValue(scope, argv[1]);

    Value *arguments = scope.alloc(3);
    arguments[2] = that;
    for (uint i = 0; i < that->d()->esTable->size(); ++i) {
        that->d()->esTable->iterate(i, &arguments[1], &arguments[0]); // fill in key (0), value (1)

        callbackfn->call(thisArg, arguments, 3);
        CHECK_EXCEPTION();
    }
    return Encode::undefined();
}

ReturnedValue MapPrototype::method_get(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || that->d()->isWeakMap)
        return scope.engine->throwTypeError();

    return that->d()->esTable->get(argc ? argv[0] : Value::undefinedValue());
}

ReturnedValue MapPrototype::method_has(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || that->d()->isWeakMap)
        return scope.engine->throwTypeError();

    return Encode(that->d()->esTable->has(argc ? argv[0] : Value::undefinedValue()));
}

ReturnedValue MapPrototype::method_keys(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || that->d()->isWeakMap)
        return scope.engine->throwTypeError();

    Scoped<MapIteratorObject> ao(scope, scope.engine->newMapIteratorObject(that));
    ao->d()->iterationKind = IteratorKind::KeyIteratorKind;
    return ao->asReturnedValue();
}

ReturnedValue MapPrototype::method_set(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || that->d()->isWeakMap)
        return scope.engine->throwTypeError();

    that->d()->esTable->set(argc ? argv[0] : Value::undefinedValue(), argc > 1 ? argv[1] : Value::undefinedValue());
    return that.asReturnedValue();
}

ReturnedValue MapPrototype::method_get_size(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || that->d()->isWeakMap)
        return scope.engine->throwTypeError();

    return Encode(that->d()->esTable->size());
}

ReturnedValue MapPrototype::method_values(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<MapObject> that(scope, thisObject);
    if (!that || that->d()->isWeakMap)
        return scope.engine->throwTypeError();

    Scoped<MapIteratorObject> ao(scope, scope.engine->newMapIteratorObject(that));
    ao->d()->iterationKind = IteratorKind::ValueIteratorKind;
    return ao->asReturnedValue();
}
