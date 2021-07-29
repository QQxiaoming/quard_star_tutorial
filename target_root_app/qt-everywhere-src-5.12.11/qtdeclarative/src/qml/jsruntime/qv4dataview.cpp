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

#include "qv4dataview_p.h"
#include "qv4arraybuffer_p.h"
#include "qv4string_p.h"
#include "qv4symbol_p.h"

#include <QtCore/private/qnumeric_p.h>
#include "qendian.h"

using namespace QV4;

DEFINE_OBJECT_VTABLE(DataViewCtor);
DEFINE_OBJECT_VTABLE(DataView);

void Heap::DataViewCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("DataView"));
}

static uint toIndex(ExecutionEngine *e, const Value &v)
{
    if (v.isUndefined())
        return 0;
    double index = v.toInteger();
    if (index < 0) {
        e->throwRangeError(QStringLiteral("index out of range"));
        return 0;
    }
    uint idx = static_cast<uint>(index);
    if (idx != index) {
        e->throwRangeError(QStringLiteral("index out of range"));
        return 0;
    }
    return idx;
}

ReturnedValue DataViewCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    Scope scope(f->engine());
    Scoped<SharedArrayBuffer> buffer(scope, argc ? argv[0] : Value::undefinedValue());
    if (!newTarget || !buffer)
        return scope.engine->throwTypeError();

    uint offset = ::toIndex(scope.engine, argc > 1 ? argv[1]: Value::undefinedValue());
    if (scope.hasException())
        return Encode::undefined();
    if (buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint bufferLength = buffer->d()->data->size;
    if (offset > bufferLength)
        return scope.engine->throwRangeError(QStringLiteral("DataView: constructor arguments out of range"));

    uint byteLength = (argc < 3 || argv[2].isUndefined()) ? (bufferLength - offset) : ::toIndex(scope.engine, argv[2]);
    if (scope.hasException())
        return Encode::undefined();
    if (offset + byteLength > bufferLength)
        return scope.engine->throwRangeError(QStringLiteral("DataView: constructor arguments out of range"));

    Scoped<DataView> a(scope, scope.engine->memoryManager->allocate<DataView>());
    a->d()->buffer.set(scope.engine, buffer->d());
    a->d()->byteLength = byteLength;
    a->d()->byteOffset = offset;
    return a.asReturnedValue();
}

ReturnedValue DataViewCtor::virtualCall(const FunctionObject *f, const Value *, const Value *, int)
{
    return f->engine()->throwTypeError();
}

void DataViewPrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(1));
    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));
    defineDefaultProperty(engine->id_constructor(), (o = ctor));
    defineAccessorProperty(QStringLiteral("buffer"), method_get_buffer, nullptr);
    defineAccessorProperty(QStringLiteral("byteLength"), method_get_byteLength, nullptr);
    defineAccessorProperty(QStringLiteral("byteOffset"), method_get_byteOffset, nullptr);

    defineDefaultProperty(QStringLiteral("getInt8"), method_getChar<signed char>, 1);
    defineDefaultProperty(QStringLiteral("getUint8"), method_getChar<unsigned char>, 1);
    defineDefaultProperty(QStringLiteral("getInt16"), method_get<short>, 1);
    defineDefaultProperty(QStringLiteral("getUint16"), method_get<unsigned short>, 1);
    defineDefaultProperty(QStringLiteral("getInt32"), method_get<int>, 1);
    defineDefaultProperty(QStringLiteral("getUint32"), method_get<unsigned int>, 1);
    defineDefaultProperty(QStringLiteral("getFloat32"), method_getFloat<float>, 1);
    defineDefaultProperty(QStringLiteral("getFloat64"), method_getFloat<double>, 1);

    defineDefaultProperty(QStringLiteral("setInt8"), method_setChar<signed char>, 2);
    defineDefaultProperty(QStringLiteral("setUint8"), method_setChar<unsigned char>, 2);
    defineDefaultProperty(QStringLiteral("setInt16"), method_set<short>, 2);
    defineDefaultProperty(QStringLiteral("setUint16"), method_set<unsigned short>, 2);
    defineDefaultProperty(QStringLiteral("setInt32"), method_set<int>, 2);
    defineDefaultProperty(QStringLiteral("setUint32"), method_set<unsigned int>, 2);
    defineDefaultProperty(QStringLiteral("setFloat32"), method_setFloat<float>, 2);
    defineDefaultProperty(QStringLiteral("setFloat64"), method_setFloat<double>, 2);

    ScopedString name(scope, engine->newString(QStringLiteral("DataView")));
    defineReadonlyConfigurableProperty(scope.engine->symbol_toStringTag(), name);

    // For backword compatibility
    defineDefaultProperty(QStringLiteral("getUInt8"), method_getChar<unsigned char>, 1);
    defineDefaultProperty(QStringLiteral("getUInt16"), method_get<unsigned short>, 1);
    defineDefaultProperty(QStringLiteral("getUInt32"), method_get<unsigned int>, 1);
    defineDefaultProperty(QStringLiteral("setUInt8"), method_setChar<unsigned char>, 1);
    defineDefaultProperty(QStringLiteral("setUInt16"), method_set<unsigned short>, 1);
    defineDefaultProperty(QStringLiteral("setUInt32"), method_set<unsigned int>, 1);
}

ReturnedValue DataViewPrototype::method_get_buffer(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    const DataView *v = thisObject->as<DataView>();
    if (!v)
        return b->engine()->throwTypeError();

    return v->d()->buffer->asReturnedValue();
}

ReturnedValue DataViewPrototype::method_get_byteLength(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    const DataView *v = thisObject->as<DataView>();
    if (!v)
        return b->engine()->throwTypeError();

    if (v->d()->buffer->isDetachedBuffer())
        return b->engine()->throwTypeError();

    return Encode(v->d()->byteLength);
}

ReturnedValue DataViewPrototype::method_get_byteOffset(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    const DataView *v = thisObject->as<DataView>();
    if (!v)
        return b->engine()->throwTypeError();

    if (v->d()->buffer->isDetachedBuffer())
        return b->engine()->throwTypeError();

    return Encode(v->d()->byteOffset);
}

template <typename T>
ReturnedValue DataViewPrototype::method_getChar(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *e = b->engine();
    const DataView *v = thisObject->as<DataView>();
    if (!v)
        return e->throwTypeError();
    uint idx = ::toIndex(e, argc ? argv[0] : Value::undefinedValue());
    if (e->hasException)
        return Encode::undefined();
    if (v->d()->buffer->isDetachedBuffer())
        return e->throwTypeError();
    if (idx + sizeof(T) > v->d()->byteLength)
        return e->throwRangeError(QStringLiteral("index out of range"));
    idx += v->d()->byteOffset;

    T t = T(v->d()->buffer->data->data()[idx]);

    return Encode((int)t);
}

template <typename T>
ReturnedValue DataViewPrototype::method_get(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *e = b->engine();
    const DataView *v = thisObject->as<DataView>();
    if (!v)
        return e->throwTypeError();
    uint idx = ::toIndex(e, argc ? argv[0] : Value::undefinedValue());
    if (e->hasException)
        return Encode::undefined();
    if (v->d()->buffer->isDetachedBuffer())
        return e->throwTypeError();
    if (idx + sizeof(T) > v->d()->byteLength)
        return e->throwRangeError(QStringLiteral("index out of range"));
    idx += v->d()->byteOffset;

    bool littleEndian = argc < 2 ? false : argv[1].toBoolean();

    T t = littleEndian
            ? qFromLittleEndian<T>((uchar *)v->d()->buffer->data->data() + idx)
            : qFromBigEndian<T>((uchar *)v->d()->buffer->data->data() + idx);

    return Encode(t);
}

template <typename T>
ReturnedValue DataViewPrototype::method_getFloat(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *e = b->engine();
    const DataView *v = thisObject->as<DataView>();
    if (!v)
        return e->throwTypeError();
    uint idx = ::toIndex(e, argc ? argv[0] : Value::undefinedValue());
    if (e->hasException)
        return Encode::undefined();
    if (v->d()->buffer->isDetachedBuffer())
        return e->throwTypeError();
    if (idx + sizeof(T) > v->d()->byteLength)
        return e->throwRangeError(QStringLiteral("index out of range"));
    idx += v->d()->byteOffset;

    bool littleEndian = argc < 2 ? false : argv[1].toBoolean();

    if (sizeof(T) == 4) {
        // float
        union {
            uint i;
            float f;
        } u;
        u.i = littleEndian
                ? qFromLittleEndian<uint>((uchar *)v->d()->buffer->data->data() + idx)
                : qFromBigEndian<uint>((uchar *)v->d()->buffer->data->data() + idx);
        return Encode(u.f);
    } else {
        Q_ASSERT(sizeof(T) == 8);
        union {
            quint64 i;
            double d;
        } u;
        u.i = littleEndian
                ? qFromLittleEndian<quint64>((uchar *)v->d()->buffer->data->data() + idx)
                : qFromBigEndian<quint64>((uchar *)v->d()->buffer->data->data() + idx);
        return Encode(u.d);
    }
}

template <typename T>
ReturnedValue DataViewPrototype::method_setChar(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *e = b->engine();
    const DataView *v = thisObject->as<DataView>();
    if (!v)
        return e->throwTypeError();
    uint idx = ::toIndex(e, argc ? argv[0] : Value::undefinedValue());
    if (e->hasException)
        return Encode::undefined();

    int val = argc >= 2 ? argv[1].toInt32() : 0;

    if (v->d()->buffer->isDetachedBuffer())
        return e->throwTypeError();

    if (idx + sizeof(T) > v->d()->byteLength)
        return e->throwRangeError(QStringLiteral("index out of range"));
    idx += v->d()->byteOffset;

    v->d()->buffer->data->data()[idx] = (char)val;

    RETURN_UNDEFINED();
}

template <typename T>
ReturnedValue DataViewPrototype::method_set(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *e = b->engine();
    const DataView *v = thisObject->as<DataView>();
    if (!v)
        return e->throwTypeError();
    uint idx = ::toIndex(e, argc ? argv[0] : Value::undefinedValue());
    if (e->hasException)
        return Encode::undefined();

    int val = argc >= 2 ? argv[1].toInt32() : 0;
    bool littleEndian = argc < 3 ? false : argv[2].toBoolean();

    if (v->d()->buffer->isDetachedBuffer())
        return e->throwTypeError();

    if (idx + sizeof(T) > v->d()->byteLength)
        return e->throwRangeError(QStringLiteral("index out of range"));
    idx += v->d()->byteOffset;


    if (littleEndian)
        qToLittleEndian<T>(val, (uchar *)v->d()->buffer->data->data() + idx);
    else
        qToBigEndian<T>(val, (uchar *)v->d()->buffer->data->data() + idx);

    RETURN_UNDEFINED();
}

template <typename T>
ReturnedValue DataViewPrototype::method_setFloat(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *e = b->engine();
    const DataView *v = thisObject->as<DataView>();
    if (!v)
        return e->throwTypeError();
    uint idx = ::toIndex(e, argc ? argv[0] : Value::undefinedValue());
    if (e->hasException)
        return Encode::undefined();

    double val = argc >= 2 ? argv[1].toNumber() : qt_qnan();
    bool littleEndian = argc < 3 ? false : argv[2].toBoolean();

    if (v->d()->buffer->isDetachedBuffer())
        return e->throwTypeError();

    if (idx + sizeof(T) > v->d()->byteLength)
        return e->throwRangeError(QStringLiteral("index out of range"));
    idx += v->d()->byteOffset;

    if (sizeof(T) == 4) {
        // float
        union {
            uint i;
            float f;
        } u;
        u.f = val;
        if (littleEndian)
            qToLittleEndian(u.i, (uchar *)v->d()->buffer->data->data() + idx);
        else
            qToBigEndian(u.i, (uchar *)v->d()->buffer->data->data() + idx);
    } else {
        Q_ASSERT(sizeof(T) == 8);
        union {
            quint64 i;
            double d;
        } u;
        u.d = val;
        if (littleEndian)
            qToLittleEndian(u.i, (uchar *)v->d()->buffer->data->data() + idx);
        else
            qToBigEndian(u.i, (uchar *)v->d()->buffer->data->data() + idx);
    }
    RETURN_UNDEFINED();
}
