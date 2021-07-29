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
#include "qv4arraybuffer_p.h"
#include "qv4typedarray_p.h"
#include "qv4atomics_p.h"
#include "qv4symbol_p.h"

using namespace QV4;

DEFINE_OBJECT_VTABLE(Atomics);

void Heap::Atomics::init()
{
    Object::init();
    Scope scope(internalClass->engine);
    ScopedObject m(scope, this);

    m->defineDefaultProperty(QStringLiteral("add"), QV4::Atomics::method_add, 3);
    m->defineDefaultProperty(QStringLiteral("and"), QV4::Atomics::method_and, 3);
    m->defineDefaultProperty(QStringLiteral("compareExchange"), QV4::Atomics::method_compareExchange, 4);
    m->defineDefaultProperty(QStringLiteral("exchange"), QV4::Atomics::method_exchange, 3);
    m->defineDefaultProperty(QStringLiteral("isLockFree"), QV4::Atomics::method_isLockFree, 1);
    m->defineDefaultProperty(QStringLiteral("load"), QV4::Atomics::method_load, 2);
    m->defineDefaultProperty(QStringLiteral("or"), QV4::Atomics::method_or, 3);
    m->defineDefaultProperty(QStringLiteral("store"), QV4::Atomics::method_store, 3);
    m->defineDefaultProperty(QStringLiteral("sub"), QV4::Atomics::method_sub, 3);
    m->defineDefaultProperty(QStringLiteral("wait"), QV4::Atomics::method_wait, 4);
    m->defineDefaultProperty(QStringLiteral("wake"), QV4::Atomics::method_wake, 3);
    m->defineDefaultProperty(QStringLiteral("xor"), QV4::Atomics::method_xor, 3);

    ScopedString name(scope, scope.engine->newString(QStringLiteral("Atomics")));
    m->defineReadonlyConfigurableProperty(scope.engine->symbol_toStringTag(), name);
}

static SharedArrayBuffer *validateSharedIntegerTypedArray(Scope &scope, const Value &typedArray, bool onlyInt32 = false)
{
    const TypedArray *a = typedArray.as<TypedArray>();
    if (!a) {
        scope.engine->throwTypeError();
        return nullptr;
    }

    TypedArrayType t(a->arrayType());
    if (!a->d()->type->atomicLoad || (onlyInt32 && t != TypedArrayType::Int32Array)) {
        scope.engine->throwTypeError();
        return nullptr;
    }

    Scoped<SharedArrayBuffer> buffer(scope, a->d()->buffer);
    if (!buffer->isSharedArrayBuffer()) {
        scope.engine->throwTypeError();
        return nullptr;
    }
    Q_ASSERT(!buffer->isDetachedBuffer());
    return buffer;
}

static int validateAtomicAccess(Scope &scope, const TypedArray &typedArray, const Value &index)
{
    const TypedArray &a = static_cast<const TypedArray &>(typedArray);
    qint64 idx = index.toIndex();
    if (scope.hasException())
        return -1;
    if (idx < 0 || idx >= a.length()) {
        scope.engine->throwRangeError(QStringLiteral("index out of range."));
        return -1;
    }
    return static_cast<int>(idx);
}

ReturnedValue atomicReadModifyWrite(const FunctionObject *f, const Value *argv, int argc, AtomicModifyOps modify)
{
    Scope scope(f);
    if (!argc)
        return scope.engine->throwTypeError();

    SharedArrayBuffer *buffer = validateSharedIntegerTypedArray(scope, argv[0]);
    if (!buffer)
        return Encode::undefined();
    const TypedArray &a = static_cast<const TypedArray &>(argv[0]);
    int index = validateAtomicAccess(scope, a, argc > 1 ? argv[1] : Value::undefinedValue());
    if (index < 0)
        return Encode::undefined();

    Value v = Value::fromReturnedValue((argc > 2 ? argv[2] : Value::undefinedValue()).convertedToNumber());
    if (scope.hasException())
        return Encode::undefined();

    int bytesPerElement = a.d()->type->bytesPerElement;
    int byteOffset = a.d()->byteOffset + index * bytesPerElement;

    return a.d()->type->atomicModifyOps[modify](buffer->data() + byteOffset, v);
}

ReturnedValue Atomics::method_add(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    return atomicReadModifyWrite(f, argv, argc, AtomicAdd);
}

ReturnedValue Atomics::method_and(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    return atomicReadModifyWrite(f, argv, argc, AtomicAnd);
}

ReturnedValue Atomics::method_compareExchange(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    if (!argc)
        return scope.engine->throwTypeError();

    SharedArrayBuffer *buffer = validateSharedIntegerTypedArray(scope, argv[0]);
    if (!buffer)
        return Encode::undefined();
    const TypedArray &a = static_cast<const TypedArray &>(argv[0]);
    int index = validateAtomicAccess(scope, a, argc > 1 ? argv[1] : Value::undefinedValue());
    if (index < 0)
        return Encode::undefined();

    Value expected = Value::fromReturnedValue((argc > 2 ? argv[2] : Value::undefinedValue()).convertedToNumber());
    if (scope.hasException())
        return Encode::undefined();
    Value v = Value::fromReturnedValue((argc > 3 ? argv[3] : Value::undefinedValue()).convertedToNumber());
    if (scope.hasException())
        return Encode::undefined();

    int bytesPerElement = a.d()->type->bytesPerElement;
    int byteOffset = a.d()->byteOffset + index * bytesPerElement;

    return a.d()->type->atomicCompareExchange(buffer->data() + byteOffset, expected, v);
}

ReturnedValue Atomics::method_exchange(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    return atomicReadModifyWrite(f, argv, argc, AtomicExchange);
}

ReturnedValue Atomics::method_isLockFree(const FunctionObject *, const Value *, const Value *argv, int argc)
{
    if (!argc)
        return Encode(false);
    double n = argv[0].toInteger();
    if (n == 4.)
        return Encode(true);
    if (n == 2.)
        return Encode(QAtomicOps<unsigned short>::isTestAndSetNative());
#ifdef Q_ATOMIC_INT8_IS_SUPPORTED
    if (n == 1.)
        return Encode(QAtomicOps<unsigned char>::isTestAndSetNative());
#endif
    return Encode(false);
}

ReturnedValue Atomics::method_load(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    if (!argc)
        return scope.engine->throwTypeError();

    SharedArrayBuffer *buffer = validateSharedIntegerTypedArray(scope, argv[0]);
    if (!buffer)
        return Encode::undefined();
    const TypedArray &a = static_cast<const TypedArray &>(argv[0]);
    int index = validateAtomicAccess(scope, a, argc > 1 ? argv[1] : Value::undefinedValue());
    if (index < 0)
        return Encode::undefined();

    int bytesPerElement = a.d()->type->bytesPerElement;
    int byteOffset = a.d()->byteOffset + index * bytesPerElement;

    return a.d()->type->atomicLoad(buffer->data() + byteOffset);
}

ReturnedValue Atomics::method_or(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    return atomicReadModifyWrite(f, argv, argc, AtomicOr);
}

ReturnedValue Atomics::method_store(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    if (!argc)
        return scope.engine->throwTypeError();

    SharedArrayBuffer *buffer = validateSharedIntegerTypedArray(scope, argv[0]);
    if (!buffer)
        return Encode::undefined();
    const TypedArray &a = static_cast<const TypedArray &>(argv[0]);
    int index = validateAtomicAccess(scope, a, argc > 1 ? argv[1] : Value::undefinedValue());
    if (index < 0)
        return Encode::undefined();

    Value v = Value::fromReturnedValue((argc > 2 ? argv[2] : Value::undefinedValue()).convertedToNumber());
    if (scope.hasException())
        return Encode::undefined();

    int bytesPerElement = a.d()->type->bytesPerElement;
    int byteOffset = a.d()->byteOffset + index * bytesPerElement;

    return a.d()->type->atomicStore(buffer->data() + byteOffset, v);
}

ReturnedValue Atomics::method_sub(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    return atomicReadModifyWrite(f, argv, argc, AtomicSub);
}

ReturnedValue Atomics::method_wait(const FunctionObject *f, const Value *, const Value *, int)
{
    return f->engine()->throwTypeError();
}

ReturnedValue Atomics::method_wake(const FunctionObject *f, const Value *, const Value *, int)
{
    return f->engine()->throwTypeError();
}

ReturnedValue Atomics::method_xor(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    return atomicReadModifyWrite(f, argv, argc, AtomicXor);

}
