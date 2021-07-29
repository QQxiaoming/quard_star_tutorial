/****************************************************************************
**
** Copyright (C) 2018 Crimson AS <info@crimson.no>
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

#include "qv4arrayobject_p.h"
#include "qv4objectiterator_p.h"
#include "qv4arrayiterator_p.h"
#include "qv4sparsearray_p.h"
#include "qv4objectproto_p.h"
#include "qv4jscall_p.h"
#include "qv4argumentsobject_p.h"
#include "qv4runtime_p.h"
#include "qv4string_p.h"
#include "qv4symbol_p.h"
#include <QtCore/qscopedvaluerollback.h>
#include "qv4proxy_p.h"

using namespace QV4;

DEFINE_OBJECT_VTABLE(ArrayCtor);

void Heap::ArrayCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("Array"));
}

ReturnedValue ArrayCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    ExecutionEngine *v4 = static_cast<const ArrayCtor *>(f)->engine();
    Scope scope(v4);
    ScopedArrayObject a(scope, v4->newArrayObject());
    if (newTarget)
        a->setProtoFromNewTarget(newTarget);
    uint len;
    if (argc == 1 && argv[0].isNumber()) {
        bool ok;
        len = argv[0].asArrayLength(&ok);

        if (!ok)
            return v4->throwRangeError(argv[0]);

        if (len < 0x1000)
            a->arrayReserve(len);
    } else {
        len = argc;
        a->arrayReserve(len);
        a->arrayPut(0, argv, len);
    }
    a->setArrayLengthUnchecked(len);

    return a.asReturnedValue();
}

ReturnedValue ArrayCtor::virtualCall(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    return virtualCallAsConstructor(f, argv, argc, f);
}

void ArrayPrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(1));
    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));
    ctor->defineDefaultProperty(QStringLiteral("isArray"), method_isArray, 1);
    ctor->defineDefaultProperty(QStringLiteral("of"), method_of, 0);
    ctor->defineDefaultProperty(QStringLiteral("from"), method_from, 1);
    ctor->addSymbolSpecies();

    Scoped<InternalClass> ic(scope, engine->classes[EngineBase::Class_Empty]
                                            ->changeVTable(QV4::Object::staticVTable()));
    ScopedObject unscopables(scope, engine->newObject(ic->d()));
    ScopedString name(scope);
    defineDefaultProperty(QStringLiteral("constructor"), (o = ctor));
    defineDefaultProperty(engine->id_toString(), method_toString, 0);
    defineDefaultProperty(engine->id_toLocaleString(), method_toLocaleString, 0);
    defineDefaultProperty(QStringLiteral("concat"), method_concat, 1);
    name = engine->newIdentifier(QStringLiteral("copyWithin"));
    unscopables->put(name, Value::fromBoolean(true));
    defineDefaultProperty(name, method_copyWithin, 2);
    name = engine->newIdentifier(QStringLiteral("entries"));
    unscopables->put(name, Value::fromBoolean(true));
    defineDefaultProperty(name, method_entries, 0);
    name = engine->newIdentifier(QStringLiteral("fill"));
    unscopables->put(name, Value::fromBoolean(true));
    defineDefaultProperty(name, method_fill, 1);
    name = engine->newIdentifier(QStringLiteral("find"));
    unscopables->put(name, Value::fromBoolean(true));
    defineDefaultProperty(name, method_find, 1);
    name = engine->newIdentifier(QStringLiteral("findIndex"));
    unscopables->put(name, Value::fromBoolean(true));
    defineDefaultProperty(name, method_findIndex, 1);
    name = engine->newIdentifier(QStringLiteral("includes"));
    unscopables->put(name, Value::fromBoolean(true));
    defineDefaultProperty(name, method_includes, 1);
    defineDefaultProperty(QStringLiteral("join"), method_join, 1);
    name = engine->newIdentifier(QStringLiteral("keys"));
    unscopables->put(name, Value::fromBoolean(true));
    defineDefaultProperty(name, method_keys, 0);
    defineDefaultProperty(QStringLiteral("pop"), method_pop, 0);
    defineDefaultProperty(QStringLiteral("push"), method_push, 1);
    defineDefaultProperty(QStringLiteral("reverse"), method_reverse, 0);
    defineDefaultProperty(QStringLiteral("shift"), method_shift, 0);
    defineDefaultProperty(QStringLiteral("slice"), method_slice, 2);
    defineDefaultProperty(QStringLiteral("sort"), method_sort, 1);
    defineDefaultProperty(QStringLiteral("splice"), method_splice, 2);
    defineDefaultProperty(QStringLiteral("unshift"), method_unshift, 1);
    defineDefaultProperty(QStringLiteral("indexOf"), method_indexOf, 1);
    defineDefaultProperty(QStringLiteral("lastIndexOf"), method_lastIndexOf, 1);
    defineDefaultProperty(QStringLiteral("every"), method_every, 1);
    defineDefaultProperty(QStringLiteral("some"), method_some, 1);
    defineDefaultProperty(QStringLiteral("forEach"), method_forEach, 1);
    defineDefaultProperty(QStringLiteral("map"), method_map, 1);
    defineDefaultProperty(QStringLiteral("filter"), method_filter, 1);
    defineDefaultProperty(QStringLiteral("reduce"), method_reduce, 1);
    defineDefaultProperty(QStringLiteral("reduceRight"), method_reduceRight, 1);
    ScopedString valuesString(scope, engine->newIdentifier(QStringLiteral("values")));
    ScopedObject values(scope, FunctionObject::createBuiltinFunction(engine, valuesString, method_values, 0));
    engine->jsObjects[ExecutionEngine::ArrayProtoValues] = values;
    unscopables->put(valuesString, Value::fromBoolean(true));
    defineDefaultProperty(valuesString, values);
    defineDefaultProperty(engine->symbol_iterator(), values);

    defineReadonlyConfigurableProperty(engine->symbol_unscopables(), unscopables);
}

ReturnedValue ArrayPrototype::method_isArray(const FunctionObject *, const Value *, const Value *argv, int argc)
{
    if (!argc || !argv->objectValue())
        return Encode(false);
    return Encode(argv->objectValue()->isArray());
}

static ScopedObject createObjectFromCtorOrArray(Scope &scope, ScopedFunctionObject ctor, bool useLen, int len)
{
    ScopedObject a(scope, Value::undefinedValue());

    if (ctor && ctor->isConstructor()) {
        // this isn't completely kosher. for instance:
        // Array.from.call(Object, []).constructor == Object
        // is expected by the tests, but naturally, we get Number.
        ScopedValue argument(scope, useLen ? QV4::Encode(len) : Value::undefinedValue());
        a = ctor->callAsConstructor(argument, useLen ? 1 : 0);
    } else {
        a = scope.engine->newArrayObject(len);
    }

    return a;
}

ReturnedValue ArrayPrototype::method_from(const FunctionObject *builtin, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(builtin);
    ScopedFunctionObject thatCtor(scope, thisObject);
    ScopedObject itemsObject(scope, argv[0]);
    bool usingIterator = false;

    if (itemsObject) {
        // If the object claims to support iterators, then let's try use them.
        ScopedValue it(scope, itemsObject->get(scope.engine->symbol_iterator()));
        if (!it->isNullOrUndefined()) {
            ScopedFunctionObject itfunc(scope, it);
            if (!itfunc)
                return scope.engine->throwTypeError();
            usingIterator = true;
        }
    }

    ScopedFunctionObject mapfn(scope, Value::undefinedValue());
    Value *mapArguments = nullptr;
    if (argc > 1) {
        mapfn = ScopedFunctionObject(scope, argv[1]);
        if (!mapfn)
            return scope.engine->throwTypeError(QString::fromLatin1("%1 is not a function").arg(argv[1].toQStringNoThrow()));
        mapArguments = scope.alloc(2);
    }

    ScopedValue thisArg(scope);
    if (argc > 2)
        thisArg = argv[2];

    if (usingIterator) {
        // Item iteration supported, so let's go ahead and try use that.
        ScopedObject a(createObjectFromCtorOrArray(scope, thatCtor, false, 0));
        CHECK_EXCEPTION();
        ScopedObject iterator(scope, Runtime::method_getIterator(scope.engine, itemsObject, true));
        CHECK_EXCEPTION(); // symbol_iterator threw; whoops.
        if (!iterator) {
            return scope.engine->throwTypeError(); // symbol_iterator wasn't an object.
        }

        qint64 k = 0;
        ScopedValue mappedValue(scope);
        Value *nextValue = scope.alloc(1);
        ScopedValue done(scope);

        // The loop below pulls out all the properties using the iterator, and
        // sets them into the created array.
        forever {
            if (k > (static_cast<qint64>(1) << 53) - 1) {
                ScopedValue falsey(scope, Encode(false));
                ScopedValue error(scope, scope.engine->throwTypeError());
                return Runtime::method_iteratorClose(scope.engine, iterator, falsey);
            }

            // Retrieve the next value. If the iteration ends, we're done here.
            done = Value::fromReturnedValue(Runtime::method_iteratorNext(scope.engine, iterator, nextValue));
            CHECK_EXCEPTION();
            if (done->toBoolean()) {
                if (ArrayObject *ao = a->as<ArrayObject>()) {
                    ao->setArrayLengthUnchecked(k);
                } else {
                    a->set(scope.engine->id_length(), Value::fromDouble(k), QV4::Object::DoThrowOnRejection);
                    CHECK_EXCEPTION();
                }
                return a.asReturnedValue();
            }

            if (mapfn) {
                mapArguments[0] = *nextValue;
                mapArguments[1] = Value::fromDouble(k);
                mappedValue = mapfn->call(thisArg, mapArguments, 2);
                if (scope.engine->hasException)
                    return Runtime::method_iteratorClose(scope.engine, iterator, Value::fromBoolean(false));
            } else {
                mappedValue = *nextValue;
            }

            if (a->getOwnProperty(PropertyKey::fromArrayIndex(k)) == Attr_Invalid) {
                a->arraySet(k, mappedValue);
            } else {
                // Don't return: we need to close the iterator.
                scope.engine->throwTypeError(QString::fromLatin1("Cannot redefine property: %1").arg(k));
            }

            if (scope.engine->hasException) {
                ScopedValue falsey(scope, Encode(false));
                return Runtime::method_iteratorClose(scope.engine, iterator, falsey);
            }

            k++;
        }

        // the return is hidden up in the loop above, when iteration finishes.
    } else {
        // Array-like fallback. We request properties by index, and set them on
        // the return object.
        ScopedObject arrayLike(scope, argv[0].toObject(scope.engine));
        if (!arrayLike)
            return scope.engine->throwTypeError(QString::fromLatin1("Cannot convert %1 to object").arg(argv[0].toQStringNoThrow()));
        qint64 len = arrayLike->getLength();
        ScopedObject a(createObjectFromCtorOrArray(scope, thatCtor, true, len));
        CHECK_EXCEPTION();

        qint64 k = 0;
        ScopedValue mappedValue(scope, Value::undefinedValue());
        ScopedValue kValue(scope);
        while (k < len) {
            kValue = arrayLike->get(k);
            CHECK_EXCEPTION();

            if (mapfn) {
                mapArguments[0] = kValue;
                mapArguments[1] = Value::fromDouble(k);
                mappedValue = mapfn->call(thisArg, mapArguments, 2);
                CHECK_EXCEPTION();
            } else {
                mappedValue = kValue;
            }

            if (a->getOwnProperty(PropertyKey::fromArrayIndex(k)) != Attr_Invalid)
                return scope.engine->throwTypeError(QString::fromLatin1("Cannot redefine property: %1").arg(k));

            a->arraySet(k, mappedValue);
            CHECK_EXCEPTION();

            k++;
        }

        if (ArrayObject *ao = a->as<ArrayObject>()) {
            ao->setArrayLengthUnchecked(k);
        } else {
            a->set(scope.engine->id_length(), Value::fromDouble(k), QV4::Object::DoThrowOnRejection);
            CHECK_EXCEPTION();
        }
        return a.asReturnedValue();
    }

}

ReturnedValue ArrayPrototype::method_of(const FunctionObject *builtin, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(builtin);
    ScopedFunctionObject that(scope, thisObject);
    ScopedObject a(createObjectFromCtorOrArray(scope, that, true, argc));
    CHECK_EXCEPTION();

    int k = 0;
    while (k < argc) {
        if (a->getOwnProperty(PropertyKey::fromArrayIndex(k)) != Attr_Invalid) {
            return scope.engine->throwTypeError(QString::fromLatin1("Cannot redefine property: %1").arg(k));
        }
        a->arraySet(k, argv[k]);
        CHECK_EXCEPTION();

        k++;
    }

    // ArrayObject updates its own length, and will throw if we try touch it.
    if (!a->as<ArrayObject>()) {
        a->set(scope.engine->id_length(), Value::fromDouble(argc), QV4::Object::DoThrowOnRejection);
        CHECK_EXCEPTION();
    }

    return a.asReturnedValue();
}

ReturnedValue ArrayPrototype::method_toString(const FunctionObject *builtin, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(builtin);
    ScopedObject that(scope, thisObject->toObject(scope.engine));
    if (scope.hasException())
        return QV4::Encode::undefined();

    ScopedString string(scope, scope.engine->newString(QStringLiteral("join")));
    ScopedFunctionObject f(scope, that->get(string));
    if (f)
        return checkedResult(scope.engine, f->call(that, argv, argc));
    return ObjectPrototype::method_toString(builtin, that, argv, argc);
}

ReturnedValue ArrayPrototype::method_toLocaleString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject);
    if (!instance)
        return scope.engine->throwTypeError();

    uint len = instance->getLength();
    const QString separator = QStringLiteral(",");

    QString R;

    ScopedValue v(scope);
    ScopedString s(scope);

    for (uint k = 0; k < len; ++k) {
        if (k)
            R += separator;

        v = instance->get(k);
        if (v->isNullOrUndefined())
            continue;
        v = Runtime::method_callElement(scope.engine, v, *scope.engine->id_toLocaleString(), nullptr, 0);
        s = v->toString(scope.engine);
        if (scope.hasException())
            return Encode::undefined();

        R += s->toQString();
    }
    return scope.engine->newString(R)->asReturnedValue();
}

ReturnedValue ArrayPrototype::method_concat(const FunctionObject *b, const Value *that, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject thisObject(scope, that->toObject(scope.engine));
    if (!thisObject)
        RETURN_UNDEFINED();

    ScopedArrayObject result(scope, scope.engine->newArrayObject());

    ScopedArrayObject elt(scope);
    ScopedObject eltAsObj(scope);
    ScopedValue entry(scope);
    for (int i = -1; i < argc; ++i) {
        const Value *v = i == -1 ? thisObject.getPointer() : argv + i;
        eltAsObj = *v;
        elt = *v;
        if (elt) {
            uint n = elt->getLength();
            uint newLen = ArrayData::append(result, elt, n);
            result->setArrayLengthUnchecked(newLen);
        } else if (eltAsObj && eltAsObj->isConcatSpreadable()) {
            const uint startIndex = result->getLength();
            const uint len = eltAsObj->getLength();
            if (scope.engine->hasException)
                return Encode::undefined();

            for (uint i = 0; i < len; ++i) {
                bool hasProperty = false;
                entry = eltAsObj->get(i, &hasProperty);
                if (hasProperty) {
                    if (!result->put(startIndex + i, entry))
                        return scope.engine->throwTypeError();
                }
            }
        } else if (eltAsObj && eltAsObj->isListType()) {
            const uint startIndex = result->getLength();
            for (int i = 0, len = eltAsObj->getLength(); i < len; ++i) {
                entry = eltAsObj->get(i);
                // spec says not to throw if this fails
                result->put(startIndex + i, entry);
            }
        } else {
            result->arraySet(result->getLength(), *v);
        }
    }

    return result.asReturnedValue();
}

ReturnedValue ArrayPrototype::method_copyWithin(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    double len = instance->getLength();
    double target = argv[0].toInteger();
    double start = argc > 1 ? argv[1].toInteger() : 0;
    double end = len;

    if (argc > 2 && !argv[2].isUndefined()) {
        end = argv[2].toInteger();
    }

    double relativeTarget = target;
    double relativeStart = start;
    double relativeEnd = end;
    double from = 0;
    double to = 0;

    if (relativeTarget < 0) {
        to = std::max(len+relativeTarget, 0.0);
    } else {
        to = std::min(relativeTarget, len);
    }
    if (relativeStart < 0) {
        from = std::max(len+relativeStart, 0.0);
    } else {
        from = std::min(relativeStart, len);
    }

    double fin = 0;
    if (relativeEnd < 0) {
        fin = std::max(len+relativeEnd, 0.0);
    } else {
        fin = std::min(relativeEnd, len);
    }
    double count = std::min(fin-from, len-to);
    double direction = 1;
    if (from < to && to < from+count) {
        direction = -1;
        from = from + count - 1;
        to = to + count - 1;
    }

    while (count > 0) {
        bool fromPresent = false;
        ScopedValue fromVal(scope, instance->get(from, &fromPresent));

        if (fromPresent) {
            instance->setIndexed(to, fromVal, QV4::Object::DoThrowOnRejection);
            CHECK_EXCEPTION();
        } else {
            bool didDelete = instance->deleteProperty(PropertyKey::fromArrayIndex(to));
            CHECK_EXCEPTION();
            if (!didDelete) {
                return scope.engine->throwTypeError();
            }
        }

        from = from + direction;
        to = to + direction;
        count = count - 1;
    }

    return instance.asReturnedValue();
}

ReturnedValue ArrayPrototype::method_entries(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    ScopedObject O(scope, thisObject->toObject(scope.engine));
    if (!O)
        RETURN_UNDEFINED();

    Scoped<ArrayIteratorObject> ao(scope, scope.engine->newArrayIteratorObject(O));
    ao->d()->iterationKind = IteratorKind::KeyValueIteratorKind;
    return ao->asReturnedValue();
}

ReturnedValue ArrayPrototype::method_find(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();

    if (!argc || !argv[0].isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    ScopedValue result(scope);
    Value *arguments = scope.alloc(3);

    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());

    for (uint k = 0; k < len; ++k) {
        arguments[0] = instance->get(k);
        CHECK_EXCEPTION();

        arguments[1] = Value::fromDouble(k);
        arguments[2] = instance;
        result = callback->call(that, arguments, 3);

        CHECK_EXCEPTION();
        if (result->toBoolean())
            return arguments[0].asReturnedValue();
    }

    RETURN_UNDEFINED();
}

ReturnedValue ArrayPrototype::method_findIndex(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();

    if (!argc || !argv[0].isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    ScopedValue result(scope);
    Value *arguments = scope.alloc(3);

    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());

    for (uint k = 0; k < len; ++k) {
        arguments[0] = instance->get(k);
        CHECK_EXCEPTION();

        arguments[1] = Value::fromDouble(k);
        arguments[2] = instance;
        result = callback->call(that, arguments, 3);

        CHECK_EXCEPTION();
        if (result->toBoolean())
            return Encode(k);
    }

    return Encode(-1);
}

ReturnedValue ArrayPrototype::method_join(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));

    if (!instance)
        return Encode(scope.engine->newString());

    ScopedValue arg(scope, argc ? argv[0] : Value::undefinedValue());

    QString r4;
    if (arg->isUndefined())
        r4 = QStringLiteral(",");
    else
        r4 = arg->toQString();

    ScopedValue length(scope, instance->get(scope.engine->id_length()));
    const quint32 r2 = length->isUndefined() ? 0 : length->toUInt32();

    if (!r2)
        return Encode(scope.engine->newString());

    QString R;

    // ### FIXME
    if (ArrayObject *a = instance->as<ArrayObject>()) {
        ScopedValue e(scope);
        for (uint i = 0; i < a->getLength(); ++i) {
            if (i)
                R += r4;

            e = a->get(i);
            CHECK_EXCEPTION();
            if (!e->isNullOrUndefined())
                R += e->toQString();
        }
    } else {
        //
        // crazy!
        //
        ScopedString name(scope, scope.engine->newString(QStringLiteral("0")));
        ScopedValue r6(scope, instance->get(name));
        if (!r6->isNullOrUndefined())
            R = r6->toQString();

        ScopedValue r12(scope);
        for (quint32 k = 1; k < r2; ++k) {
            R += r4;

            name = Value::fromDouble(k).toString(scope.engine);
            r12 = instance->get(name);
            CHECK_EXCEPTION();

            if (!r12->isNullOrUndefined())
                R += r12->toQString();
        }
    }

    return Encode(scope.engine->newString(R));
}

ReturnedValue ArrayPrototype::method_pop(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();

    if (!len) {
        if (!instance->isArrayObject())
            instance->put(scope.engine->id_length(), ScopedValue(scope, Value::fromInt32(0)));
        RETURN_UNDEFINED();
    }

    ScopedValue result(scope, instance->get(len - 1));
    CHECK_EXCEPTION();

    if (!instance->deleteProperty(PropertyKey::fromArrayIndex(len - 1)))
        return scope.engine->throwTypeError();

    if (instance->isArrayObject())
        instance->setArrayLength(len - 1);
    else {
        if (!instance->put(scope.engine->id_length(), ScopedValue(scope, Value::fromDouble(len - 1))))
            return scope.engine->throwTypeError();
    }
    return result->asReturnedValue();
}

ReturnedValue ArrayPrototype::method_push(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    instance->arrayCreate();
    Q_ASSERT(instance->arrayData());

    qint64 len = instance->getLength();

    if (len + quint64(argc) >= UINT_MAX) {
        // ughh... this goes beyond UINT_MAX
        double l = len;
        ScopedString s(scope);
        for (int i = 0, ei = argc; i < ei; ++i) {
            s = Value::fromDouble(l + i).toString(scope.engine);
            if (!instance->put(s, argv[i]))
                return scope.engine->throwTypeError();
        }
        double newLen = l + argc;
        if (!instance->isArrayObject()) {
            if (!instance->put(scope.engine->id_length(), ScopedValue(scope, Value::fromDouble(newLen))))
                return scope.engine->throwTypeError();
        } else {
            ScopedString str(scope, scope.engine->newString(QStringLiteral("Array.prototype.push: Overflow")));
            return scope.engine->throwRangeError(str);
        }
        return Encode(newLen);
    }

    if (!argc)
        ;
    else if (!instance->protoHasArray() && instance->arrayData()->length() <= len && instance->arrayData()->type == Heap::ArrayData::Simple) {
        instance->arrayData()->vtable()->putArray(instance, len, argv, argc);
        len = instance->arrayData()->length();
    } else {
        for (int i = 0, ei = argc; i < ei; ++i) {
            if (!instance->put(len + i, argv[i]))
                return scope.engine->throwTypeError();
        }
        len += argc;
    }
    if (instance->isArrayObject())
        instance->setArrayLengthUnchecked(len);
    else {
        if (!instance->put(scope.engine->id_length(), ScopedValue(scope, Value::fromDouble(len))))
            return scope.engine->throwTypeError();
    }

    return Encode(uint(len));
}

ReturnedValue ArrayPrototype::method_reverse(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    qint64 length = instance->getLength();
    // ### FIXME
    if (length >= UINT_MAX)
        return scope.engine->throwRangeError(QLatin1String("Array.prototype.reverse: Length out of range."));

    int lo = 0, hi = length - 1;

    ScopedValue lval(scope);
    ScopedValue hval(scope);
    for (; lo < hi; ++lo, --hi) {
        bool loExists, hiExists;
        lval = instance->get(lo, &loExists);
        hval = instance->get(hi, &hiExists);
        CHECK_EXCEPTION();
        bool ok;
        if (hiExists)
            ok = instance->put(lo, hval);
        else
            ok = instance->deleteProperty(PropertyKey::fromArrayIndex(lo));
        if (ok) {
            if (loExists)
                ok = instance->put(hi, lval);
            else
                ok = instance->deleteProperty(PropertyKey::fromArrayIndex(hi));
        }
        if (!ok)
            return scope.engine->throwTypeError();
    }
    return instance->asReturnedValue();
}

ReturnedValue ArrayPrototype::method_shift(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    instance->arrayCreate();
    Q_ASSERT(instance->arrayData());

    uint len = instance->getLength();

    if (!len) {
        if (!instance->isArrayObject())
            if (!instance->put(scope.engine->id_length(), ScopedValue(scope, Value::fromInt32(0))))
                return scope.engine->throwTypeError();
        RETURN_UNDEFINED();
    }

    ScopedValue result(scope);
    if (!instance->protoHasArray() && !instance->arrayData()->attrs && instance->arrayData()->length() <= len && instance->arrayData()->type != Heap::ArrayData::Custom) {
        result = instance->arrayData()->vtable()->pop_front(instance);
    } else {
        result = instance->get(uint(0));
        CHECK_EXCEPTION();
        ScopedValue v(scope);
        // do it the slow way
        for (uint k = 1; k < len; ++k) {
            bool exists;
            v = instance->get(k, &exists);
            CHECK_EXCEPTION();
            bool ok;
            if (exists)
                ok = instance->put(k - 1, v);
            else
                ok = instance->deleteProperty(PropertyKey::fromArrayIndex(k - 1));
            if (!ok)
                return scope.engine->throwTypeError();
        }
        bool ok = instance->deleteProperty(PropertyKey::fromArrayIndex(len - 1));
        if (!ok)
            return scope.engine->throwTypeError();
    }

    if (instance->isArrayObject())
        instance->setArrayLengthUnchecked(len - 1);
    else {
        bool ok = instance->put(scope.engine->id_length(), ScopedValue(scope, Value::fromDouble(len - 1)));
        if (!ok)
            return scope.engine->throwTypeError();
    }

    return result->asReturnedValue();
}

ReturnedValue ArrayPrototype::method_slice(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject o(scope, thisObject->toObject(scope.engine));
    if (!o)
        RETURN_UNDEFINED();

    ScopedArrayObject result(scope, scope.engine->newArrayObject());
    uint len = o->getLength();
    double s = (argc ? argv[0] : Value::undefinedValue()).toInteger();
    uint start;
    if (s < 0)
        start = (uint)qMax(len + s, 0.);
    else if (s > len)
        start = len;
    else
        start = (uint) s;
    uint end = len;
    if (argc > 1 && !argv[1].isUndefined()) {
        double e = argv[1].toInteger();
        if (e < 0)
            end = (uint)qMax(len + e, 0.);
        else if (e > len)
            end = len;
        else
            end = (uint) e;
    }

    ScopedValue v(scope);
    uint n = 0;
    for (uint i = start; i < end; ++i) {
        bool exists;
        v = o->get(i, &exists);
        CHECK_EXCEPTION();
        if (exists)
            result->arraySet(n, v);
        ++n;
    }
    return result->asReturnedValue();
}

ReturnedValue ArrayPrototype::method_sort(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();

    ScopedValue comparefn(scope, argc ? argv[0] : Value::undefinedValue());
    ArrayData::sort(scope.engine, instance, comparefn, len);
    return thisObject->asReturnedValue();
}

ReturnedValue ArrayPrototype::method_splice(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    qint64 len = instance->getLength();

    double rs = (argc ? argv[0] : Value::undefinedValue()).toInteger();
    qint64 start;
    if (rs < 0)
        start = static_cast<qint64>(qMax(0., len + rs));
    else
        start = static_cast<qint64>(qMin(rs, static_cast<double>(len)));

    qint64 deleteCount = 0;
    qint64 itemCount = 0;
    if (argc == 1) {
        deleteCount = len - start;
    } else if (argc > 1){
        itemCount = argc - 2;
        double dc = argv[1].toInteger();
        deleteCount = static_cast<qint64>(qMin(qMax(dc, 0.), double(len - start)));
    }

    if (len + itemCount - deleteCount > /*(static_cast<qint64>(1) << 53) - 1*/ UINT_MAX - 1)
        return scope.engine->throwTypeError();
    if (deleteCount > /*(static_cast<qint64>(1) << 53) - 1*/ UINT_MAX - 1)
        return scope.engine->throwRangeError(QString::fromLatin1("Array length out of range."));

    ScopedArrayObject newArray(scope, scope.engine->newArrayObject());
    newArray->arrayReserve(deleteCount);
    ScopedValue v(scope);
    for (uint i = 0; i < deleteCount; ++i) {
        bool exists;
        v = instance->get(start + i, &exists);
        CHECK_EXCEPTION();
        if (exists)
            newArray->arrayPut(i, v);
    }
    newArray->setArrayLengthUnchecked(deleteCount);


    if (itemCount < deleteCount) {
        for (uint k = start; k < len - deleteCount; ++k) {
            bool exists;
            v = instance->get(k + deleteCount, &exists);
            CHECK_EXCEPTION();
            bool ok;
            if (exists)
                ok = instance->put(k + itemCount, v);
            else
                ok = instance->deleteProperty(PropertyKey::fromArrayIndex(k + itemCount));
            if (!ok)
                return scope.engine->throwTypeError();
        }
        for (uint k = len; k > len - deleteCount + itemCount; --k) {
            if (!instance->deleteProperty(PropertyKey::fromArrayIndex(k - 1)))
                return scope.engine->throwTypeError();
        }
    } else if (itemCount > deleteCount) {
        uint k = len - deleteCount;
        while (k > start) {
            bool exists;
            v = instance->get(k + deleteCount - 1, &exists);
            CHECK_EXCEPTION();
            bool ok;
            if (exists)
                ok = instance->put(k + itemCount - 1, v);
            else
                ok = instance->deleteProperty(PropertyKey::fromArrayIndex(k + itemCount - 1));
            if (!ok)
                return scope.engine->throwTypeError();
            --k;
        }
    }

    for (uint i = 0; i < itemCount; ++i)
        instance->put(start + i, argv[i + 2]);

    if (!instance->put(scope.engine->id_length(), ScopedValue(scope, Value::fromDouble(len - deleteCount + itemCount))))
        return scope.engine->throwTypeError();

    return newArray->asReturnedValue();
}

ReturnedValue ArrayPrototype::method_unshift(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    instance->arrayCreate();
    Q_ASSERT(instance->arrayData());

    uint len = instance->getLength();

    if (!instance->protoHasArray() && !instance->arrayData()->attrs && instance->arrayData()->length() <= len &&
        instance->arrayData()->type != Heap::ArrayData::Custom) {
        instance->arrayData()->vtable()->push_front(instance, argv, argc);
    } else {
        ScopedValue v(scope);
        for (uint k = len; k > 0; --k) {
            bool exists;
            v = instance->get(k - 1, &exists);
            bool ok;
            if (exists)
                ok = instance->put(k + argc - 1, v);
            else
                ok = instance->deleteProperty(PropertyKey::fromArrayIndex(k + argc - 1));
            if (!ok)
                return scope.engine->throwTypeError();
        }
        for (int i = 0, ei = argc; i < ei; ++i) {
            bool ok = instance->put(i, argv[i]);
            if (!ok)
                return scope.engine->throwTypeError();
        }
    }

    uint newLen = len + argc;
    if (instance->isArrayObject())
        instance->setArrayLengthUnchecked(newLen);
    else {
        if (!instance->put(scope.engine->id_length(), ScopedValue(scope, Value::fromDouble(newLen))))
            return scope.engine->throwTypeError();
    }

    return Encode(newLen);
}

ReturnedValue ArrayPrototype::method_includes(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    qint64 len = instance->getLength();
    if (len == 0) {
        return Encode(false);
    }

    double n = 0;
    if (argc > 1 && !argv[1].isUndefined()) {
        n = argv[1].toInteger();
    }

    double k = 0;
    if (n >= 0) {
        k = n;
    } else {
        k = len + n;
        if (k < 0) {
            k = 0;
        }
    }

    while (k < len) {
        ScopedValue val(scope, instance->get(k));
        if (val->sameValueZero(argv[0])) {
            return Encode(true);
        }
        k++;
    }

    return Encode(false);
}

ReturnedValue ArrayPrototype::method_indexOf(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();
    if (!len)
        return Encode(-1);

    ScopedValue searchValue(scope, argc ? argv[0] : Value::undefinedValue());
    uint fromIndex = 0;

    if (argc >= 2) {
        double f = argv[1].toInteger();
        CHECK_EXCEPTION();
        if (f >= len)
            return Encode(-1);
        if (f < 0)
            f = qMax(len + f, 0.);
        fromIndex = (uint) f;
    }

    if (instance->isStringObject()) {
        ScopedValue v(scope);
        for (uint k = fromIndex; k < len; ++k) {
            bool exists;
            v = instance->get(k, &exists);
            if (exists && RuntimeHelpers::strictEqual(v, searchValue))
                return Encode(k);
        }
        return Encode(-1);
    }

    ScopedValue value(scope);

    if (ArgumentsObject::isNonStrictArgumentsObject(instance) ||
        (instance->arrayType() >= Heap::ArrayData::Sparse) || instance->protoHasArray()) {
        // lets be safe and slow
        for (uint i = fromIndex; i < len; ++i) {
            bool exists;
            value = instance->get(i, &exists);
            CHECK_EXCEPTION();
            if (exists && RuntimeHelpers::strictEqual(value, searchValue))
                return Encode(i);
        }
    } else if (!instance->arrayData()) {
        return Encode(-1);
    } else {
        Q_ASSERT(instance->arrayType() == Heap::ArrayData::Simple);
        Heap::SimpleArrayData *sa = instance->d()->arrayData.cast<Heap::SimpleArrayData>();
        if (len > sa->values.size)
            len = sa->values.size;
        uint idx = fromIndex;
        while (idx < len) {
            value = sa->data(idx);
            CHECK_EXCEPTION();
            if (RuntimeHelpers::strictEqual(value, searchValue))
                return Encode(idx);
            ++idx;
        }
    }
    return Encode(-1);
}

ReturnedValue ArrayPrototype::method_keys(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    Scope scope(f);
    ScopedObject O(scope, thisObject->toObject(scope.engine));
    if (!O)
        RETURN_UNDEFINED();

    Scoped<ArrayIteratorObject> ao(scope, scope.engine->newArrayIteratorObject(O));
    ao->d()->iterationKind = IteratorKind::KeyIteratorKind;
    return ao->asReturnedValue();
}

ReturnedValue ArrayPrototype::method_lastIndexOf(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();
    if (!len)
        return Encode(-1);

    ScopedValue searchValue(scope);
    uint fromIndex = len;

    if (argc >= 1)
        searchValue = argv[0];
    else
        searchValue = Value::undefinedValue();

    if (argc >= 2) {
        double f = argv[1].toInteger();
        CHECK_EXCEPTION();
        if (f > 0)
            f = qMin(f, (double)(len - 1));
        else if (f < 0) {
            f = len + f;
            if (f < 0)
                return Encode(-1);
        }
        fromIndex = (uint) f + 1;
    }

    ScopedValue v(scope);
    for (uint k = fromIndex; k > 0;) {
        --k;
        bool exists;
        v = instance->get(k, &exists);
        CHECK_EXCEPTION();
        if (exists && RuntimeHelpers::strictEqual(v, searchValue))
            return Encode(k);
    }
    return Encode(-1);
}

ReturnedValue ArrayPrototype::method_every(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();

    if (!argc || !argv->isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());
    ScopedValue r(scope);
    Value *arguments = scope.alloc(3);

    bool ok = true;
    for (uint k = 0; ok && k < len; ++k) {
        bool exists;
        arguments[0] = instance->get(k, &exists);
        if (!exists)
            continue;

        arguments[1] = Value::fromDouble(k);
        arguments[2] = instance;
        r = callback->call(that, arguments, 3);
        CHECK_EXCEPTION();
        ok = r->toBoolean();
    }
    return Encode(ok);
}

ReturnedValue ArrayPrototype::method_fill(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();
    int relativeStart = argc > 1 ? argv[1].toInteger() : 0;
    int relativeEnd = len;
    if (argc > 2 && !argv[2].isUndefined()) {
        relativeEnd = argv[2].toInteger();
    }
    uint k = 0;
    uint fin = 0;

    if (relativeStart < 0) {
        k = std::max(len+relativeStart, uint(0));
    } else {
        k = std::min(uint(relativeStart), len);
    }

    if (relativeEnd < 0) {
        fin = std::max(len + relativeEnd, uint(0));
    } else {
        fin = std::min(uint(relativeEnd), len);
    }

    while (k < fin) {
        instance->setIndexed(k, argv[0], QV4::Object::DoThrowOnRejection);
        k++;
    }

    return instance.asReturnedValue();
}

ReturnedValue ArrayPrototype::method_some(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();

    if (!argc || !argv->isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());
    ScopedValue result(scope);
    Value *arguments = scope.alloc(3);

    for (uint k = 0; k < len; ++k) {
        bool exists;
        arguments[0] = instance->get(k, &exists);
        if (!exists)
            continue;

        arguments[1] = Value::fromDouble(k);
        arguments[2] = instance;
        result = callback->call(that, arguments, 3);
        CHECK_EXCEPTION();
        if (result->toBoolean())
            return Encode(true);
    }
    return Encode(false);
}

ReturnedValue ArrayPrototype::method_forEach(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();

    if (!argc || !argv->isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());
    Value *arguments = scope.alloc(3);

    for (uint k = 0; k < len; ++k) {
        bool exists;
        arguments[0] = instance->get(k, &exists);
        if (!exists)
            continue;

        arguments[1] = Value::fromDouble(k);
        arguments[2] = instance;
        callback->call(that, arguments, 3);
    }
    RETURN_UNDEFINED();
}

ReturnedValue ArrayPrototype::method_map(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    qint64 len = instance->getLength();

    if (!argc || !argv->isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    if (len > UINT_MAX - 1)
        return scope.engine->throwRangeError(QString::fromLatin1("Array length out of range."));

    ScopedArrayObject a(scope, scope.engine->newArrayObject());
    a->arrayReserve(len);
    a->setArrayLengthUnchecked(len);

    ScopedValue v(scope);
    ScopedValue mapped(scope);
    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());
    Value *arguments = scope.alloc(3);

    for (uint k = 0; k < len; ++k) {
        bool exists;
        arguments[0] = instance->get(k, &exists);
        if (!exists)
            continue;

        arguments[1] = Value::fromDouble(k);
        arguments[2] = instance;
        mapped = callback->call(that, arguments, 3);
        CHECK_EXCEPTION();
        a->arraySet(k, mapped);
    }
    return a.asReturnedValue();
}

ReturnedValue ArrayPrototype::method_filter(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();

    if (!argc || !argv->isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    ScopedArrayObject a(scope, scope.engine->newArrayObject());
    a->arrayReserve(len);

    ScopedValue selected(scope);
    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());
    Value *arguments = scope.alloc(3);

    uint to = 0;
    for (uint k = 0; k < len; ++k) {
        bool exists;
        arguments[0] = instance->get(k, &exists);
        if (!exists)
            continue;

        arguments[1] = Value::fromDouble(k);
        arguments[2] = instance;
        selected = callback->call(that, arguments, 3);
        CHECK_EXCEPTION();
        if (selected->toBoolean()) {
            a->arraySet(to, arguments[0]);
            ++to;
        }
    }
    return a.asReturnedValue();
}

ReturnedValue ArrayPrototype::method_reduce(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();

    if (!argc || !argv->isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    uint k = 0;
    ScopedValue acc(scope);
    ScopedValue v(scope);

    if (argc > 1) {
        acc = argv[1];
    } else {
        bool kPresent = false;
        while (k < len && !kPresent) {
            v = instance->get(k, &kPresent);
            if (kPresent)
                acc = v;
            ++k;
        }
        if (!kPresent)
            THROW_TYPE_ERROR();
    }

    Value *arguments = scope.alloc(4);

    while (k < len) {
        bool kPresent;
        v = instance->get(k, &kPresent);
        if (kPresent) {
            arguments[0] = acc;
            arguments[1] = v;
            arguments[2] = Value::fromDouble(k);
            arguments[3] = instance;
            acc = callback->call(nullptr, arguments, 4);
            CHECK_EXCEPTION();
        }
        ++k;
    }
    return acc->asReturnedValue();
}

ReturnedValue ArrayPrototype::method_reduceRight(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();

    uint len = instance->getLength();

    if (!argc || !argv->isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    if (len == 0) {
        if (argc == 1)
            THROW_TYPE_ERROR();
        return argv[1].asReturnedValue();
    }

    uint k = len;
    ScopedValue acc(scope);
    ScopedValue v(scope);
    if (argc > 1) {
        acc = argv[1];
    } else {
        bool kPresent = false;
        while (k > 0 && !kPresent) {
            v = instance->get(k - 1, &kPresent);
            if (kPresent)
                acc = v;
            --k;
        }
        if (!kPresent)
            THROW_TYPE_ERROR();
    }

    Value *arguments = scope.alloc(4);

    while (k > 0) {
        bool kPresent;
        v = instance->get(k - 1, &kPresent);
        if (kPresent) {
            arguments[0] = acc;
            arguments[1] = v;
            arguments[2] = Value::fromDouble(k - 1);
            arguments[3] = instance;
            acc = callback->call(nullptr, arguments, 4);
            CHECK_EXCEPTION();
        }
        --k;
    }
    return acc->asReturnedValue();
}

ReturnedValue ArrayPrototype::method_values(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    ScopedObject O(scope, thisObject->toObject(scope.engine));
    if (!O)
        RETURN_UNDEFINED();

    Scoped<ArrayIteratorObject> ao(scope, scope.engine->newArrayIteratorObject(O));
    ao->d()->iterationKind = IteratorKind::ValueIteratorKind;
    return ao->asReturnedValue();
}

ReturnedValue ArrayPrototype::method_get_species(const FunctionObject *, const Value *thisObject, const Value *, int)
{
    return thisObject->asReturnedValue();
}

