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
#include "qv4dataview_p.h"
#include "qv4string_p.h"
#include "qv4jscall_p.h"
#include "qv4symbol_p.h"

using namespace QV4;

DEFINE_OBJECT_VTABLE(SharedArrayBufferCtor);
DEFINE_OBJECT_VTABLE(ArrayBufferCtor);
DEFINE_OBJECT_VTABLE(SharedArrayBuffer);
DEFINE_OBJECT_VTABLE(ArrayBuffer);

void Heap::SharedArrayBufferCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("SharedArrayBuffer"));
}

void Heap::ArrayBufferCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("ArrayBuffer"));
}

ReturnedValue SharedArrayBufferCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    Scope scope(f);
    if (newTarget->isUndefined())
        return scope.engine->throwTypeError();

    qint64 len = argc ? argv[0].toIndex() : 0;
    if (scope.engine->hasException)
        return Encode::undefined();
    if (len < 0 || len >= INT_MAX)
        return scope.engine->throwRangeError(QStringLiteral("SharedArrayBuffer: Invalid length."));

    Scoped<SharedArrayBuffer> a(scope, scope.engine->memoryManager->allocate<SharedArrayBuffer>(len));
    if (scope.engine->hasException)
        return Encode::undefined();

    return a->asReturnedValue();
}

ReturnedValue SharedArrayBufferCtor::virtualCall(const FunctionObject *f, const Value *, const Value *, int)
{
    return f->engine()->throwTypeError();
}


ReturnedValue ArrayBufferCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    ExecutionEngine *v4 = f->engine();
    Scope scope(v4);

    ScopedValue l(scope, argc ? argv[0] : Value::undefinedValue());
    double dl = l->toInteger();
    if (v4->hasException)
        return Encode::undefined();
    uint len = (uint)qBound(0., dl, (double)UINT_MAX);
    if (len != dl)
        return v4->throwRangeError(QLatin1String("ArrayBuffer constructor: invalid length"));

    Scoped<ArrayBuffer> a(scope, v4->newArrayBuffer(len));
    if (newTarget->heapObject() != f->heapObject() && newTarget->isFunctionObject()) {
        const FunctionObject *nt = static_cast<const FunctionObject *>(newTarget);
        ScopedObject o(scope, nt->protoProperty());
        if (o)
            a->setPrototypeOf(o);
    }
    if (scope.engine->hasException)
        return Encode::undefined();

    return a->asReturnedValue();
}

ReturnedValue ArrayBufferCtor::method_isView(const FunctionObject *, const Value *, const Value *argv, int argc)
{
    if (argc < 1)
        return Encode(false);

    if (argv[0].as<TypedArray>() ||
        argv[0].as<DataView>())
        return Encode(true);

    return Encode(false);
}


void Heap::SharedArrayBuffer::init(size_t length)
{
    Object::init();
    if (length < UINT_MAX)
        data = QTypedArrayData<char>::allocate(length + 1);
    if (!data) {
        internalClass->engine->throwRangeError(QStringLiteral("ArrayBuffer: out of memory"));
        return;
    }
    data->size = int(length);
    memset(data->data(), 0, length + 1);
    isShared = true;
}

void Heap::SharedArrayBuffer::init(const QByteArray& array)
{
    Object::init();
    data = const_cast<QByteArray&>(array).data_ptr();
    data->ref.ref();
    isShared = true;
}

void Heap::SharedArrayBuffer::destroy()
{
    if (data && !data->ref.deref())
        QTypedArrayData<char>::deallocate(data);
    Object::destroy();
}

QByteArray ArrayBuffer::asByteArray() const
{
    QByteArrayDataPtr ba = { d()->data };
    ba.ptr->ref.ref();
    return QByteArray(ba);
}

void ArrayBuffer::detach() {
    if (!d()->data->ref.isShared())
        return;

    QTypedArrayData<char> *oldData = d()->data;

    d()->data = QTypedArrayData<char>::allocate(oldData->size + 1);
    if (!d()->data) {
        engine()->throwRangeError(QStringLiteral("ArrayBuffer: out of memory"));
        return;
    }

    memcpy(d()->data->data(), oldData->data(), oldData->size + 1);

    if (!oldData->ref.deref())
        QTypedArrayData<char>::deallocate(oldData);
}


void SharedArrayBufferPrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(1));
    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));
    ctor->addSymbolSpecies();

    defineDefaultProperty(engine->id_constructor(), (o = ctor));
    defineAccessorProperty(QStringLiteral("byteLength"), method_get_byteLength, nullptr);
    defineDefaultProperty(QStringLiteral("slice"), method_slice, 2);
    ScopedString name(scope, engine->newString(QStringLiteral("SharedArrayBuffer")));
    defineReadonlyConfigurableProperty(scope.engine->symbol_toStringTag(), name);
}

ReturnedValue SharedArrayBufferPrototype::method_get_byteLength(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    const SharedArrayBuffer *a = thisObject->as<SharedArrayBuffer>();
    if (!a || a->isDetachedBuffer() || !a->isSharedArrayBuffer())
        return b->engine()->throwTypeError();

    return Encode(a->d()->data->size);
}

ReturnedValue SharedArrayBufferPrototype::method_slice(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    return slice(b, thisObject, argv, argc, true);
}

ReturnedValue SharedArrayBufferPrototype::slice(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc, bool shared)
{
    Scope scope(b);
    const SharedArrayBuffer *a = thisObject->as<SharedArrayBuffer>();
    if (!a || a->isDetachedBuffer() || (a->isSharedArrayBuffer() != shared))
        return scope.engine->throwTypeError();

    double start = argc > 0 ? argv[0].toInteger() : 0;
    double end = (argc < 2 || argv[1].isUndefined()) ?
                a->d()->data->size : argv[1].toInteger();
    if (scope.hasException())
        return QV4::Encode::undefined();

    double first = (start < 0) ? qMax(a->d()->data->size + start, 0.) : qMin(start, (double)a->d()->data->size);
    double final = (end < 0) ? qMax(a->d()->data->size + end, 0.) : qMin(end, (double)a->d()->data->size);

    const FunctionObject *constructor = a->speciesConstructor(scope, shared ? scope.engine->sharedArrayBufferCtor() : scope.engine->arrayBufferCtor());
    if (!constructor)
        return scope.engine->throwTypeError();

    double newLen = qMax(final - first, 0.);
    ScopedValue argument(scope, QV4::Encode(newLen));
    QV4::Scoped<SharedArrayBuffer> newBuffer(scope, constructor->callAsConstructor(argument, 1));
    if (!newBuffer || newBuffer->d()->data->size < (int)newLen ||
        newBuffer->isDetachedBuffer() || (newBuffer->isSharedArrayBuffer() != shared) ||
        newBuffer->sameValue(*a) ||
        a->isDetachedBuffer())
        return scope.engine->throwTypeError();

    memcpy(newBuffer->d()->data->data(), a->d()->data->data() + (uint)first, newLen);
    return newBuffer->asReturnedValue();
}


void ArrayBufferPrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(1));
    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));
    ctor->defineDefaultProperty(QStringLiteral("isView"), ArrayBufferCtor::method_isView, 1);
    ctor->addSymbolSpecies();

    defineDefaultProperty(engine->id_constructor(), (o = ctor));
    defineAccessorProperty(QStringLiteral("byteLength"), method_get_byteLength, nullptr);
    defineDefaultProperty(QStringLiteral("slice"), method_slice, 2);
    defineDefaultProperty(QStringLiteral("toString"), method_toString, 0);
    ScopedString name(scope, engine->newString(QStringLiteral("ArrayBuffer")));
    defineReadonlyConfigurableProperty(scope.engine->symbol_toStringTag(), name);
}

ReturnedValue ArrayBufferPrototype::method_get_byteLength(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    const ArrayBuffer *a = thisObject->as<ArrayBuffer>();
    if (!a || a->isDetachedBuffer() || a->isSharedArrayBuffer())
        return f->engine()->throwTypeError();

    return Encode(a->d()->data->size);
}

ReturnedValue ArrayBufferPrototype::method_slice(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    return slice(b, thisObject, argv, argc, false);
}

ReturnedValue ArrayBufferPrototype::method_toString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    const ArrayBuffer *a = thisObject->as<ArrayBuffer>();
    if (!a)
        RETURN_UNDEFINED();
    return Encode(v4->newString(QString::fromUtf8(a->asByteArray())));
}
