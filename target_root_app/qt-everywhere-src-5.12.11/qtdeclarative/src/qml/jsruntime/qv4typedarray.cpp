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

#include "qv4typedarray_p.h"
#include "qv4arrayiterator_p.h"
#include "qv4arraybuffer_p.h"
#include "qv4string_p.h"
#include "qv4jscall_p.h"
#include "qv4symbol_p.h"
#include "qv4runtime_p.h"
#include <QtCore/qatomic.h>

#include <cmath>

using namespace QV4;

DEFINE_OBJECT_VTABLE(IntrinsicTypedArrayCtor);
DEFINE_OBJECT_VTABLE(IntrinsicTypedArrayPrototype);
DEFINE_OBJECT_VTABLE(TypedArrayCtor);
DEFINE_OBJECT_VTABLE(TypedArrayPrototype);
DEFINE_OBJECT_VTABLE(TypedArray);

Q_STATIC_ASSERT((int)ExecutionEngine::NTypedArrayTypes == (int)NTypedArrayTypes);

static inline int toInt32(Value v)
{
    Q_ASSERT(v.isNumber());
    if (v.isInteger())
        return v.integerValue();
    return Double::toInt32(v.doubleValue());
}

static inline double toDouble(Value v)
{
    Q_ASSERT(v.isNumber());
    if (v.isInteger())
        return v.integerValue();
    return v.doubleValue();
}

struct ClampedUInt8 {
    quint8 c;
};

template <typename T>
ReturnedValue typeToValue(T t) {
    return Encode(t);
}

template <>
ReturnedValue typeToValue(ClampedUInt8 t) {
    return Encode(t.c);
}

template <typename T>
T valueToType(Value value)
{
    Q_ASSERT(value.isNumber());
    int n = toInt32(value);
    return static_cast<T>(n);
}

template <>
ClampedUInt8 valueToType(Value value)
{
    Q_ASSERT(value.isNumber());
    if (value.isInteger())
        return { static_cast<quint8>(qBound(0, value.integerValue(), 255)) };
    Q_ASSERT(value.isDouble());
    double d = value.doubleValue();
    // ### is there a way to optimise this?
    if (d <= 0 || std::isnan(d))
        return { 0 };
    if (d >= 255)
        return { 255 };
    double f = std::floor(d);
    if (f + 0.5 < d)
        return { (quint8)(f + 1) };
    if (d < f + 0.5)
        return { (quint8)(f) };
    if (int(f) % 2)
        // odd number
        return { (quint8)(f + 1) };
    return { (quint8)(f) };
}

template <>
float valueToType(Value value)
{
    Q_ASSERT(value.isNumber());
    double d = toDouble(value);
    return static_cast<float>(d);
}

template <>
double valueToType(Value value)
{
    Q_ASSERT(value.isNumber());
    return toDouble(value);
}

template <typename T>
ReturnedValue read(const char *data) {
    return typeToValue(*reinterpret_cast<const T *>(data));
}
template <typename T>
void write(char *data, Value value)
{
    *reinterpret_cast<T *>(data) = valueToType<T>(value);
}

template <typename T>
ReturnedValue atomicAdd(char *data, Value v)
{
    T value = valueToType<T>(v);
    typename QAtomicOps<T>::Type *mem = reinterpret_cast<typename QAtomicOps<T>::Type *>(data);
    value = QAtomicOps<T>::fetchAndAddOrdered(*mem, value);
    return typeToValue(value);
}

template <typename T>
ReturnedValue atomicAnd(char *data, Value v)
{
    T value = valueToType<T>(v);
    typename QAtomicOps<T>::Type *mem = reinterpret_cast<typename QAtomicOps<T>::Type *>(data);
    value = QAtomicOps<T>::fetchAndAndOrdered(*mem, value);
    return typeToValue(value);
}

template <typename T>
ReturnedValue atomicExchange(char *data, Value v)
{
    T value = valueToType<T>(v);
    typename QAtomicOps<T>::Type *mem = reinterpret_cast<typename QAtomicOps<T>::Type *>(data);
    value = QAtomicOps<T>::fetchAndStoreOrdered(*mem, value);
    return typeToValue(value);
}

template <typename T>
ReturnedValue atomicOr(char *data, Value v)
{
    T value = valueToType<T>(v);
    typename QAtomicOps<T>::Type *mem = reinterpret_cast<typename QAtomicOps<T>::Type *>(data);
    value = QAtomicOps<T>::fetchAndOrOrdered(*mem, value);
    return typeToValue(value);
}

template <typename T>
ReturnedValue atomicSub(char *data, Value v)
{
    T value = valueToType<T>(v);
    typename QAtomicOps<T>::Type *mem = reinterpret_cast<typename QAtomicOps<T>::Type *>(data);
    value = QAtomicOps<T>::fetchAndSubOrdered(*mem, value);
    return typeToValue(value);
}

template <typename T>
ReturnedValue atomicXor(char *data, Value v)
{
    T value = valueToType<T>(v);
    typename QAtomicOps<T>::Type *mem = reinterpret_cast<typename QAtomicOps<T>::Type *>(data);
    value = QAtomicOps<T>::fetchAndXorOrdered(*mem, value);
    return typeToValue(value);
}

template <typename T>
ReturnedValue atomicCompareExchange(char *data, Value expected, Value v)
{
    T value = valueToType<T>(v);
    T exp = valueToType<T>(expected);
    typename QAtomicOps<T>::Type *mem = reinterpret_cast<typename QAtomicOps<T>::Type *>(data);
    T old;
    QAtomicOps<T>::testAndSetOrdered(*mem, exp, value, &old);
    return typeToValue(old);
}

template <typename T>
ReturnedValue atomicLoad(char *data)
{
    typename QAtomicOps<T>::Type *mem = reinterpret_cast<typename QAtomicOps<T>::Type *>(data);
    T val = QAtomicOps<T>::load(*mem);
    return typeToValue(val);
}

template <typename T>
ReturnedValue atomicStore(char *data, Value v)
{
    T value = valueToType<T>(v);
    typename QAtomicOps<T>::Type *mem = reinterpret_cast<typename QAtomicOps<T>::Type *>(data);
    QAtomicOps<T>::store(*mem, value);
    return typeToValue(value);
}


template<typename T>
constexpr TypedArrayOperations TypedArrayOperations::create(const char *name)
{
    return { sizeof(T),
             name,
             ::read<T>,
             ::write<T>,
             { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
             nullptr,
             nullptr,
             nullptr
    };
}

template<typename T>
constexpr TypedArrayOperations TypedArrayOperations::createWithAtomics(const char *name)
{
    return { sizeof(T),
             name,
             ::read<T>,
             ::write<T>,
             { ::atomicAdd<T>, ::atomicAnd<T>, ::atomicExchange<T>, ::atomicOr<T>, ::atomicSub<T>, ::atomicXor<T> },
             ::atomicCompareExchange<T>,
             ::atomicLoad<T>,
             ::atomicStore<T>
    };
}

const TypedArrayOperations operations[NTypedArrayTypes] = {
#ifdef Q_ATOMIC_INT8_IS_SUPPORTED
    TypedArrayOperations::createWithAtomics<qint8>("Int8Array"),
    TypedArrayOperations::createWithAtomics<quint8>("Uint8Array"),
#else
    TypedArrayOperations::create<qint8>("Int8Array"),
    TypedArrayOperations::create<quint8>("Uint8Array"),
#endif
    TypedArrayOperations::createWithAtomics<qint16>("Int16Array"),
    TypedArrayOperations::createWithAtomics<quint16>("Uint16Array"),
    TypedArrayOperations::createWithAtomics<qint32>("Int32Array"),
    TypedArrayOperations::createWithAtomics<quint32>("Uint32Array"),
    TypedArrayOperations::create<ClampedUInt8>("Uint8ClampedArray"),
    TypedArrayOperations::create<float>("Float32Array"),
    TypedArrayOperations::create<double>("Float64Array")
};


void Heap::TypedArrayCtor::init(QV4::ExecutionContext *scope, TypedArray::Type t)
{
    Heap::FunctionObject::init(scope, QLatin1String(operations[t].name));
    type = t;
}

ReturnedValue TypedArrayCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    Scope scope(f->engine());
    const TypedArrayCtor *that = static_cast<const TypedArrayCtor *>(f);

    auto updateProto = [=](Scope &scope, Scoped<TypedArray> &a) {
        if (newTarget->heapObject() != f->heapObject() && newTarget->isFunctionObject()) {
            const FunctionObject *nt = static_cast<const FunctionObject *>(newTarget);
            ScopedObject o(scope, nt->protoProperty());
            if (o)
                a->setPrototypeOf(o);
        }
    };

    if (!argc || !argv[0].isObject()) {
        // ECMA 6 22.2.1.1
        qint64 l = argc ? argv[0].toIndex() : 0;
        if (scope.engine->hasException)
            return Encode::undefined();
        // ### lift UINT_MAX restriction
        if (l < 0 || l > UINT_MAX)
            return scope.engine->throwRangeError(QLatin1String("Index out of range."));
        uint len = (uint)l;
        if (l != len)
            scope.engine->throwRangeError(QStringLiteral("Non integer length for typed array."));
        uint byteLength = len * operations[that->d()->type].bytesPerElement;
        Scoped<ArrayBuffer> buffer(scope, scope.engine->newArrayBuffer(byteLength));
        if (scope.engine->hasException)
            return Encode::undefined();

        Scoped<TypedArray> array(scope, TypedArray::create(scope.engine, that->d()->type));
        array->d()->buffer.set(scope.engine, buffer->d());
        array->d()->byteLength = byteLength;
        array->d()->byteOffset = 0;

        updateProto(scope, array);
        return array.asReturnedValue();
    }
    Scoped<TypedArray> typedArray(scope, argc ? argv[0] : Value::undefinedValue());
    if (!!typedArray) {
        // ECMA 6 22.2.1.2
        Scoped<ArrayBuffer> buffer(scope, typedArray->d()->buffer);
        if (!buffer || buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
        uint srcElementSize = typedArray->d()->type->bytesPerElement;
        uint destElementSize = operations[that->d()->type].bytesPerElement;
        uint byteLength = typedArray->d()->byteLength;
        uint destByteLength = byteLength*destElementSize/srcElementSize;

        Scoped<ArrayBuffer> newBuffer(scope, scope.engine->newArrayBuffer(destByteLength));
        if (scope.engine->hasException)
            return Encode::undefined();

        Scoped<TypedArray> array(scope, TypedArray::create(scope.engine, that->d()->type));
        array->d()->buffer.set(scope.engine, newBuffer->d());
        array->d()->byteLength = destByteLength;
        array->d()->byteOffset = 0;

        const char *src = buffer->d()->data->data() + typedArray->d()->byteOffset;
        char *dest = newBuffer->d()->data->data();

        // check if src and new type have the same size. In that case we can simply memcpy the data
        if (srcElementSize == destElementSize) {
            memcpy(dest, src, byteLength);
        } else {
            // not same size, we need to loop
            uint l = typedArray->length();
            TypedArrayOperations::Read read = typedArray->d()->type->read;
            TypedArrayOperations::Write write =array->d()->type->write;
            for (uint i = 0; i < l; ++i) {
                Value val;
                val.setRawValue(read(src + i*srcElementSize));
                write(dest + i*destElementSize, val);
            }
        }

        updateProto(scope, array);
        return array.asReturnedValue();
    }
    Scoped<ArrayBuffer> buffer(scope, argc ? argv[0] : Value::undefinedValue());
    if (!!buffer) {
        // ECMA 6 22.2.1.4

        double dbyteOffset = argc > 1 ? argv[1].toInteger() : 0;

        if (buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();

        uint byteOffset = (uint)dbyteOffset;
        uint elementSize = operations[that->d()->type].bytesPerElement;
        if (dbyteOffset < 0 || (byteOffset % elementSize) || dbyteOffset > buffer->byteLength())
            return scope.engine->throwRangeError(QStringLiteral("new TypedArray: invalid byteOffset"));

        uint byteLength;
        if (argc < 3 || argv[2].isUndefined()) {
            byteLength = buffer->byteLength() - byteOffset;
            if (buffer->byteLength() < byteOffset || byteLength % elementSize)
                return scope.engine->throwRangeError(QStringLiteral("new TypedArray: invalid length"));
        } else {
            double l = qBound(0., argv[2].toInteger(), (double)UINT_MAX);
            if (scope.engine->hasException)
                return Encode::undefined();
            if (buffer->isDetachedBuffer())
                return scope.engine->throwTypeError();
            l *= elementSize;
            if (buffer->byteLength() - byteOffset < l)
                return scope.engine->throwRangeError(QStringLiteral("new TypedArray: invalid length"));
            byteLength = (uint)l;
        }

        Scoped<TypedArray> array(scope, TypedArray::create(scope.engine, that->d()->type));
        array->d()->buffer.set(scope.engine, buffer->d());
        array->d()->byteLength = byteLength;
        array->d()->byteOffset = byteOffset;

        updateProto(scope, array);
        return array.asReturnedValue();
    }

    // ECMA 6 22.2.1.3

    ScopedObject o(scope, argc ? argv[0] : Value::undefinedValue());
    uint l = (uint) qBound(0., ScopedValue(scope, o->get(scope.engine->id_length()))->toInteger(), (double)UINT_MAX);
    if (scope.engine->hasException)
        return scope.engine->throwTypeError();

    uint elementSize = operations[that->d()->type].bytesPerElement;
    size_t bufferSize;
    if (mul_overflow(size_t(l), size_t(elementSize), &bufferSize))
        return scope.engine->throwRangeError(QLatin1String("new TypedArray: invalid length"));
    Scoped<ArrayBuffer> newBuffer(scope, scope.engine->newArrayBuffer(bufferSize));
    if (scope.engine->hasException)
        return Encode::undefined();

    Scoped<TypedArray> array(scope, TypedArray::create(scope.engine, that->d()->type));
    array->d()->buffer.set(scope.engine, newBuffer->d());
    array->d()->byteLength = l * elementSize;
    array->d()->byteOffset = 0;

    uint idx = 0;
    char *b = newBuffer->d()->data->data();
    ScopedValue val(scope);
    while (idx < l) {
        val = o->get(idx);
        val = val->convertedToNumber();
        if (scope.engine->hasException)
            return Encode::undefined();
        array->d()->type->write(b, val);
        if (scope.engine->hasException)
            return Encode::undefined();
        ++idx;
        b += elementSize;
    }

    updateProto(scope, array);
    return array.asReturnedValue();
}

ReturnedValue TypedArrayCtor::virtualCall(const FunctionObject *f, const Value *, const Value *, int)
{
    return f->engine()->throwTypeError(QStringLiteral("calling a TypedArray constructor without new is invalid"));
}

void Heap::TypedArray::init(Type t)
{
    Object::init();
    type = operations + static_cast<int>(t);
    arrayType = static_cast<int>(t);
}

Heap::TypedArray *TypedArray::create(ExecutionEngine *e, Heap::TypedArray::Type t)
{
    Scope scope(e);
    Scoped<InternalClass> ic(scope, e->newInternalClass(staticVTable(), e->typedArrayPrototype + static_cast<int>(t)));
    return e->memoryManager->allocObject<TypedArray>(ic->d(), t);
}

ReturnedValue TypedArray::virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty)
{
    uint index = id.asArrayIndex();
    if (index == UINT_MAX && !id.isCanonicalNumericIndexString())
        return Object::virtualGet(m, id, receiver, hasProperty);
    // fall through, with index == UINT_MAX it'll do the right thing.

    Scope scope(static_cast<const Object *>(m)->engine());
    Scoped<TypedArray> a(scope, static_cast<const TypedArray *>(m));
    if (a->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    if (index >= a->length()) {
        if (hasProperty)
            *hasProperty = false;
        return Encode::undefined();
    }

    uint bytesPerElement = a->d()->type->bytesPerElement;
    uint byteOffset = a->d()->byteOffset + index * bytesPerElement;
    Q_ASSERT(byteOffset + bytesPerElement <= (uint)a->d()->buffer->byteLength());

    if (hasProperty)
        *hasProperty = true;
    return a->d()->type->read(a->d()->buffer->data->data() + byteOffset);
}

bool TypedArray::virtualHasProperty(const Managed *m, PropertyKey id)
{
    uint index = id.asArrayIndex();
    if (index == UINT_MAX && !id.isCanonicalNumericIndexString())
        return Object::virtualHasProperty(m, id);
    // fall through, with index == UINT_MAX it'll do the right thing.

    const TypedArray *a = static_cast<const TypedArray *>(m);
    if (a->d()->buffer->isDetachedBuffer()) {
        a->engine()->throwTypeError();
        return false;
    }
    if (index >= a->length())
        return false;
    return true;
}

PropertyAttributes TypedArray::virtualGetOwnProperty(const Managed *m, PropertyKey id, Property *p)
{
    uint index = id.asArrayIndex();
    if (index == UINT_MAX && !id.isCanonicalNumericIndexString())
        return Object::virtualGetOwnProperty(m, id, p);
    // fall through, with index == UINT_MAX it'll do the right thing.

    bool hasProperty = false;
    ReturnedValue v = virtualGet(m, id, m, &hasProperty);
    if (p)
        p->value = v;
    return hasProperty ? Attr_NotConfigurable : PropertyAttributes();
}

bool TypedArray::virtualPut(Managed *m, PropertyKey id, const Value &value, Value *receiver)
{
    uint index = id.asArrayIndex();
    if (index == UINT_MAX && !id.isCanonicalNumericIndexString())
        return Object::virtualPut(m, id, value, receiver);
    // fall through, with index == UINT_MAX it'll do the right thing.

    ExecutionEngine *v4 = static_cast<Object *>(m)->engine();
    if (v4->hasException)
        return false;

    Scope scope(v4);
    Scoped<TypedArray> a(scope, static_cast<TypedArray *>(m));
    if (a->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    if (index >= a->length())
        return false;

    uint bytesPerElement = a->d()->type->bytesPerElement;
    uint byteOffset = a->d()->byteOffset + index * bytesPerElement;
    Q_ASSERT(byteOffset + bytesPerElement <= (uint)a->d()->buffer->byteLength());

    Value v = Value::fromReturnedValue(value.convertedToNumber());
    if (scope.hasException() || a->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();
    a->d()->type->write(a->d()->buffer->data->data() + byteOffset, v);
    return true;
}

bool TypedArray::virtualDefineOwnProperty(Managed *m, PropertyKey id, const Property *p, PropertyAttributes attrs)
{
    uint index = id.asArrayIndex();
    if (index == UINT_MAX && !id.isCanonicalNumericIndexString())
        return Object::virtualDefineOwnProperty(m, id, p, attrs);
    // fall through, with index == UINT_MAX it'll do the right thing.

    TypedArray *a = static_cast<TypedArray *>(m);
    if (index >= a->length() || attrs.isAccessor())
        return false;

    if (attrs.hasConfigurable() && attrs.isConfigurable())
        return false;
    if (attrs.hasEnumerable() && !attrs.isEnumerable())
        return false;
    if (attrs.hasWritable() && !attrs.isWritable())
        return false;
    if (!p->value.isEmpty()) {
        ExecutionEngine *engine = a->engine();

        Value v = Value::fromReturnedValue(p->value.convertedToNumber());
        if (engine->hasException || a->d()->buffer->isDetachedBuffer())
            return engine->throwTypeError();
        uint bytesPerElement = a->d()->type->bytesPerElement;
        uint byteOffset = a->d()->byteOffset + index * bytesPerElement;
        Q_ASSERT(byteOffset + bytesPerElement <= (uint)a->d()->buffer->byteLength());
        a->d()->type->write(a->d()->buffer->data->data() + byteOffset, v);
    }
    return true;
}

struct TypedArrayOwnPropertyKeyIterator : ObjectOwnPropertyKeyIterator
{
    ~TypedArrayOwnPropertyKeyIterator() override = default;
    PropertyKey next(const Object *o, Property *pd = nullptr, PropertyAttributes *attrs = nullptr) override;

};

PropertyKey TypedArrayOwnPropertyKeyIterator::next(const Object *o, Property *pd, PropertyAttributes *attrs)
{
    const TypedArray *a = static_cast<const TypedArray *>(o);
    if (arrayIndex < a->length()) {
        if (attrs)
            *attrs = Attr_NotConfigurable;
        PropertyKey id = PropertyKey::fromArrayIndex(arrayIndex);
        if (pd) {
            bool hasProperty = false;
            pd->value = TypedArray::virtualGet(a, id, a, &hasProperty);
        }
        ++arrayIndex;
        return id;
    }

    arrayIndex = UINT_MAX;
    return ObjectOwnPropertyKeyIterator::next(o, pd, attrs);
}

OwnPropertyKeyIterator *TypedArray::virtualOwnPropertyKeys(const Object *m, Value *target)
{
    *target = *m;
    return new TypedArrayOwnPropertyKeyIterator();
}

void TypedArrayPrototype::init(ExecutionEngine *engine, TypedArrayCtor *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);

    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(3));
    ctor->defineReadonlyProperty(engine->id_prototype(), *this);
    ctor->defineReadonlyProperty(QStringLiteral("BYTES_PER_ELEMENT"), Value::fromInt32(operations[static_cast<int>(ctor->d()->type)].bytesPerElement));
    ctor->setPrototypeOf(engine->intrinsicTypedArrayCtor());

    setPrototypeOf(engine->intrinsicTypedArrayPrototype());
    defineDefaultProperty(engine->id_constructor(), (o = ctor));
    defineReadonlyProperty(QStringLiteral("BYTES_PER_ELEMENT"), Value::fromInt32(operations[static_cast<int>(ctor->d()->type)].bytesPerElement));
}

ReturnedValue IntrinsicTypedArrayPrototype::method_get_buffer(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    const TypedArray *v = thisObject->as<TypedArray>();
    if (!v)
        return v4->throwTypeError();

    return v->d()->buffer->asReturnedValue();
}

ReturnedValue IntrinsicTypedArrayPrototype::method_get_byteLength(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    const TypedArray *v = thisObject->as<TypedArray>();
    if (!v)
        return v4->throwTypeError();

    if (v->d()->buffer->isDetachedBuffer())
        return Encode(0);

    return Encode(v->d()->byteLength);
}

ReturnedValue IntrinsicTypedArrayPrototype::method_get_byteOffset(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    const TypedArray *v = thisObject->as<TypedArray>();
    if (!v)
        return v4->throwTypeError();

    if (v->d()->buffer->isDetachedBuffer())
        return Encode(0);

    return Encode(v->d()->byteOffset);
}

ReturnedValue IntrinsicTypedArrayPrototype::method_get_length(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    const TypedArray *v = thisObject->as<TypedArray>();
    if (!v)
        return v4->throwTypeError();

    if (v->d()->buffer->isDetachedBuffer())
        return Encode(0);

    return Encode(v->d()->byteLength/v->d()->type->bytesPerElement);
}

ReturnedValue IntrinsicTypedArrayPrototype::method_copyWithin(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(f);
    Scoped<TypedArray> O(scope, thisObject);
    if (!O || O->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    if (!argc)
        return O->asReturnedValue();

    qint64 len = static_cast<uint>(O->length());

    qint64 to = static_cast<qint64>(argv[0].toInteger());
    if (to < 0)
        to = qMax(len + to, 0ll);
    else
        to = qMin(to, len);

    qint64 from = (argc > 1) ? static_cast<qint64>(argv[1].toInteger()) : 0ll;
    if (from < 0)
        from = qMax(len + from, 0ll);
    else
        from = qMin(from, len);

    double fend = argv[2].toInteger();
    if (fend > len)
        fend = len;
    qint64 end = (argc > 2 && !argv[2].isUndefined()) ? static_cast<qint64>(fend) : len;
    if (end < 0)
        end = qMax(len + end, 0ll);
    else
        end = qMin(end, len);

    qint64 count = qMin(end - from, len - to);

    if (count <= 0)
        return O->asReturnedValue();

    if (O->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    if (from != to) {
        int elementSize = O->d()->type->bytesPerElement;
        char *data = O->d()->buffer->data->data() + O->d()->byteOffset;
        memmove(data + to*elementSize, data + from*elementSize, count*elementSize);
    }

    return O->asReturnedValue();
}

ReturnedValue IntrinsicTypedArrayPrototype::method_entries(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<TypedArray> v(scope, thisObject);
    if (!v || v->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    Scoped<ArrayIteratorObject> ao(scope, scope.engine->newArrayIteratorObject(v));
    ao->d()->iterationKind = IteratorKind::KeyValueIteratorKind;
    return ao->asReturnedValue();
}

ReturnedValue IntrinsicTypedArrayPrototype::method_every(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> v(scope, thisObject);
    if (!v || v->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = v->length();

    if (!argc || !argv->isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());
    ScopedValue r(scope);
    Value *arguments = scope.alloc(3);

    const char *data = v->d()->buffer->data->data();
    uint bytesPerElement = v->d()->type->bytesPerElement;
    uint byteOffset = v->d()->byteOffset;

    bool ok = true;
    for (uint k = 0; ok && k < len; ++k) {
        if (v->d()->buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();

        arguments[0] = v->d()->type->read(data + byteOffset + k * bytesPerElement);

        arguments[1] = Value::fromDouble(k);
        arguments[2] = v;
        r = callback->call(that, arguments, 3);
        CHECK_EXCEPTION();
        ok = r->toBoolean();
    }
    return Encode(ok);
}

ReturnedValue IntrinsicTypedArrayPrototype::method_fill(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> v(scope, thisObject);
    if (!v || v->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = v->length();
    double dlen = len;
    double relativeStart = argc > 1 ? argv[1].toInteger() : 0.;
    double relativeEnd = len;
    if (argc > 2 && !argv[2].isUndefined())
        relativeEnd = argv[2].toInteger();

    uint k = 0;
    uint fin = 0;

    if (relativeStart < 0) {
        k = static_cast<uint>(std::max(len+relativeStart, 0.));
    } else {
        k = static_cast<uint>(std::min(relativeStart, dlen));
    }

    if (relativeEnd < 0) {
        fin = static_cast<uint>(std::max(len + relativeEnd, 0.));
    } else {
        fin = static_cast<uint>(std::min(relativeEnd, dlen));
    }

    double val = argc ? argv[0].toNumber() : std::numeric_limits<double>::quiet_NaN();
    Value value = Value::fromDouble(val);
    if (scope.hasException() || v->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    char *data = v->d()->buffer->data->data();
    uint bytesPerElement = v->d()->type->bytesPerElement;
    uint byteOffset = v->d()->byteOffset;

    while (k < fin) {
        v->d()->type->write(data + byteOffset + k * bytesPerElement, value);
        k++;
    }

    return v.asReturnedValue();
}

static TypedArray *typedArraySpeciesCreate(Scope &scope, const TypedArray *instance, uint len)
{
    const FunctionObject *constructor = instance->speciesConstructor(scope, scope.engine->typedArrayCtors + instance->d()->arrayType);
    if (!constructor) {
        scope.engine->throwTypeError();
        return nullptr;
    }

    Value *arguments = scope.alloc(1);
    arguments[0] = Encode(len);
    Scoped<TypedArray> a(scope, constructor->callAsConstructor(arguments, 1));
    if (!a || a->d()->buffer->isDetachedBuffer() || a->length() < len) {
        scope.engine->throwTypeError();
        return nullptr;
    }
    return a;
}

ReturnedValue IntrinsicTypedArrayPrototype::method_filter(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> instance(scope, thisObject);
    if (!instance || instance->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = instance->length();

    if (!argc || !argv->isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    ScopedValue selected(scope);
    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());
    Value *arguments = scope.alloc(3);
    Value *list = arguments;

    uint to = 0;
    for (uint k = 0; k < len; ++k) {
        if (instance->d()->buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
        bool exists;
        arguments[0] = instance->get(k, &exists);
        if (!exists)
            continue;

        arguments[1] = Value::fromDouble(k);
        arguments[2] = instance;
        selected = callback->call(that, arguments, 3);
        CHECK_EXCEPTION();
        if (selected->toBoolean()) {
            ++arguments;
            scope.alloc(1);
            ++to;
        }
    }

    TypedArray *a = typedArraySpeciesCreate(scope, instance, to);
    if (!a)
        return Encode::undefined();

    for (uint i = 0; i < to; ++i)
        a->put(i, list[i]);

    return a->asReturnedValue();
}

ReturnedValue IntrinsicTypedArrayPrototype::method_find(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> v(scope, thisObject);
    if (!v || v->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = v->length();

    if (!argc || !argv[0].isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    ScopedValue result(scope);
    Value *arguments = scope.alloc(3);

    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());

    for (uint k = 0; k < len; ++k) {
        if (v->d()->buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
        arguments[0] = v->get(k);
        CHECK_EXCEPTION();

        arguments[1] = Value::fromDouble(k);
        arguments[2] = v;
        result = callback->call(that, arguments, 3);

        CHECK_EXCEPTION();
        if (result->toBoolean())
            return arguments[0].asReturnedValue();
    }

    RETURN_UNDEFINED();
}

ReturnedValue IntrinsicTypedArrayPrototype::method_findIndex(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> v(scope, thisObject);
    if (!v || v->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = v->length();

    if (!argc || !argv[0].isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    ScopedValue result(scope);
    Value *arguments = scope.alloc(3);

    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());

    for (uint k = 0; k < len; ++k) {
        if (v->d()->buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
        arguments[0] = v->get(k);
        CHECK_EXCEPTION();

        arguments[1] = Value::fromDouble(k);
        arguments[2] = v;
        result = callback->call(that, arguments, 3);

        CHECK_EXCEPTION();
        if (result->toBoolean())
            return Encode(k);
    }

    return Encode(-1);
}

ReturnedValue IntrinsicTypedArrayPrototype::method_forEach(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> v(scope, thisObject);
    if (!v || v->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = v->length();

    if (!argc || !argv->isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());
    Value *arguments = scope.alloc(3);

    for (uint k = 0; k < len; ++k) {
        if (v->d()->buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
        bool exists;
        arguments[0] = v->get(k, &exists);
        if (!exists)
            continue;

        arguments[1] = Value::fromDouble(k);
        arguments[2] = v;
        callback->call(that, arguments, 3);
    }
    RETURN_UNDEFINED();
}


ReturnedValue IntrinsicTypedArrayPrototype::method_includes(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> v(scope, thisObject);
    if (!v || v->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = v->length();
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
        ScopedValue val(scope, v->get(k));
        if (val->sameValueZero(argv[0])) {
            return Encode(true);
        }
        k++;
    }

    return Encode(false);
}

ReturnedValue IntrinsicTypedArrayPrototype::method_indexOf(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> v(scope, thisObject);
    if (!v || v->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = v->length();
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

    if (v->isStringObject()) {
        ScopedValue value(scope);
        for (uint k = fromIndex; k < len; ++k) {
            bool exists;
            value = v->get(k, &exists);
            if (exists && RuntimeHelpers::strictEqual(value, searchValue))
                return Encode(k);
        }
        return Encode(-1);
    }

    ScopedValue value(scope);

    for (uint i = fromIndex; i < len; ++i) {
        bool exists;
        value = v->get(i, &exists);
        CHECK_EXCEPTION();
        if (exists && RuntimeHelpers::strictEqual(value, searchValue))
            return Encode(i);
    }
    return Encode(-1);
}

ReturnedValue IntrinsicTypedArrayPrototype::method_join(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> v(scope, thisObject);
    if (!v || v->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = v->length();

    ScopedValue arg(scope, argc ? argv[0] : Value::undefinedValue());

    QString r4;
    if (arg->isUndefined())
        r4 = QStringLiteral(",");
    else
        r4 = arg->toQString();

    const quint32 r2 = len;

    if (!r2)
        return Encode(scope.engine->newString());

    QString R;

    //
    // crazy!
    //
    ScopedString name(scope, scope.engine->newString(QStringLiteral("0")));
    ScopedValue r6(scope, v->get(name));
    if (!r6->isNullOrUndefined())
        R = r6->toQString();

    ScopedValue r12(scope);
    for (quint32 k = 1; k < r2; ++k) {
        R += r4;

        name = Value::fromDouble(k).toString(scope.engine);
        r12 = v->get(name);
        CHECK_EXCEPTION();

        if (!r12->isNullOrUndefined())
            R += r12->toQString();
    }

    return Encode(scope.engine->newString(R));
}

ReturnedValue IntrinsicTypedArrayPrototype::method_keys(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<TypedArray> v(scope, thisObject);
    if (!v || v->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    Scoped<ArrayIteratorObject> ao(scope, scope.engine->newArrayIteratorObject(v));
    ao->d()->iterationKind = IteratorKind::KeyIteratorKind;
    return ao->asReturnedValue();
}


ReturnedValue IntrinsicTypedArrayPrototype::method_lastIndexOf(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> instance(scope, thisObject);
    if (!instance || instance->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = instance->length();
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

    ScopedValue value(scope);
    for (uint k = fromIndex; k > 0;) {
        --k;
        bool exists;
        value = instance->get(k, &exists);
        if (exists && RuntimeHelpers::strictEqual(value, searchValue))
            return Encode(k);
    }
    return Encode(-1);
}

ReturnedValue IntrinsicTypedArrayPrototype::method_map(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> instance(scope, thisObject);
    if (!instance || instance->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = instance->length();

    if (!argc || !argv->isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    TypedArray *a = typedArraySpeciesCreate(scope, instance, len);
    if (!a)
        return Encode::undefined();

    ScopedValue v(scope);
    ScopedValue mapped(scope);
    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());
    Value *arguments = scope.alloc(3);

    for (uint k = 0; k < len; ++k) {
        if (instance->d()->buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
        arguments[0] = instance->get(k);

        arguments[1] = Value::fromDouble(k);
        arguments[2] = instance;
        mapped = callback->call(that, arguments, 3);
        CHECK_EXCEPTION();
        a->put(k, mapped);
    }
    return a->asReturnedValue();
}

ReturnedValue IntrinsicTypedArrayPrototype::method_reduce(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> instance(scope, thisObject);
    if (!instance || instance->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = instance->length();

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
        if (instance->d()->buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
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

ReturnedValue IntrinsicTypedArrayPrototype::method_reduceRight(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> instance(scope, thisObject);
    if (!instance || instance->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = instance->length();

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
        if (instance->d()->buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
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

ReturnedValue IntrinsicTypedArrayPrototype::method_reverse(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<TypedArray> instance(scope, thisObject);
    if (!instance || instance->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint length = instance->length();

    int lo = 0, hi = length - 1;

    ScopedValue lval(scope);
    ScopedValue hval(scope);
    for (; lo < hi; ++lo, --hi) {
        bool loExists, hiExists;
        lval = instance->get(lo, &loExists);
        hval = instance->get(hi, &hiExists);
        Q_ASSERT(hiExists && loExists);
        bool ok;
        ok = instance->put(lo, hval);
        Q_ASSERT(ok);
        ok = instance->put(hi, lval);
        Q_ASSERT(ok);
    }
    return instance->asReturnedValue();
}

ReturnedValue IntrinsicTypedArrayPrototype::method_some(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> instance(scope, thisObject);
    if (!instance || instance->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = instance->length();

    if (!argc || !argv->isFunctionObject())
        THROW_TYPE_ERROR();
    const FunctionObject *callback = static_cast<const FunctionObject *>(argv);

    ScopedValue that(scope, argc > 1 ? argv[1] : Value::undefinedValue());
    ScopedValue result(scope);
    Value *arguments = scope.alloc(3);

    for (uint k = 0; k < len; ++k) {
        if (instance->d()->buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
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


ReturnedValue IntrinsicTypedArrayPrototype::method_values(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<TypedArray> v(scope, thisObject);
    if (!v || v->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    Scoped<ArrayIteratorObject> ao(scope, scope.engine->newArrayIteratorObject(v));
    ao->d()->iterationKind = IteratorKind::ValueIteratorKind;
    return ao->asReturnedValue();
}

ReturnedValue IntrinsicTypedArrayPrototype::method_set(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> a(scope, *thisObject);
    if (!a)
        return scope.engine->throwTypeError();
    Scoped<ArrayBuffer> buffer(scope, a->d()->buffer);

    double doffset = argc >= 2 ? argv[1].toInteger() : 0;
    if (scope.engine->hasException)
        RETURN_UNDEFINED();
    if (!buffer || buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    if (doffset < 0 || doffset >= UINT_MAX)
        RETURN_RESULT(scope.engine->throwRangeError(QStringLiteral("TypedArray.set: out of range")));
    uint offset = (uint)doffset;
    uint elementSize = a->d()->type->bytesPerElement;

    Scoped<TypedArray> srcTypedArray(scope, argv[0]);
    if (!srcTypedArray) {
        // src is a regular object
        ScopedObject o(scope, argv[0].toObject(scope.engine));
        if (scope.engine->hasException || !o)
            return scope.engine->throwTypeError();

        double len = ScopedValue(scope, o->get(scope.engine->id_length()))->toNumber();
        uint l = (uint)len;
        if (scope.engine->hasException || l != len)
            return scope.engine->throwTypeError();

        if (offset + l > a->length())
            RETURN_RESULT(scope.engine->throwRangeError(QStringLiteral("TypedArray.set: out of range")));

        uint idx = 0;
        if (buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
        char *b = buffer->d()->data->data() + a->d()->byteOffset + offset*elementSize;
        ScopedValue val(scope);
        while (idx < l) {
            val = o->get(idx);
            if (scope.hasException())
                return Encode::undefined();
            val = val->convertedToNumber();
            if (scope.hasException() || buffer->isDetachedBuffer())
                return scope.engine->throwTypeError();
            a->d()->type->write(b, val);
            if (scope.engine->hasException)
                RETURN_UNDEFINED();
            ++idx;
            b += elementSize;
        }
        RETURN_UNDEFINED();
    }

    // src is a typed array
    Scoped<ArrayBuffer> srcBuffer(scope, srcTypedArray->d()->buffer);
    if (!srcBuffer || srcBuffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint l = srcTypedArray->length();
    if (offset + l > a->length())
        RETURN_RESULT(scope.engine->throwRangeError(QStringLiteral("TypedArray.set: out of range")));

    char *dest = buffer->d()->data->data() + a->d()->byteOffset + offset*elementSize;
    const char *src = srcBuffer->d()->data->data() + srcTypedArray->d()->byteOffset;
    if (srcTypedArray->d()->type == a->d()->type) {
        // same type of typed arrays, use memmove (as srcbuffer and buffer could be the same)
        memmove(dest, src, srcTypedArray->d()->byteLength);
        RETURN_UNDEFINED();
    }

    char *srcCopy = nullptr;
    if (buffer->d() == srcBuffer->d()) {
        // same buffer, need to take a temporary copy, to not run into problems
        srcCopy = new char[srcTypedArray->d()->byteLength];
        memcpy(srcCopy, src, srcTypedArray->d()->byteLength);
        src = srcCopy;
    }

    // typed arrays of different kind, need to manually loop
    uint srcElementSize = srcTypedArray->d()->type->bytesPerElement;
    TypedArrayOperations::Read read = srcTypedArray->d()->type->read;
    TypedArrayOperations::Write write = a->d()->type->write;
    for (uint i = 0; i < l; ++i) {
        Value val;
        val.setRawValue(read(src + i*srcElementSize));
        write(dest + i*elementSize, val);
    }

    if (srcCopy)
        delete [] srcCopy;

    RETURN_UNDEFINED();
}

ReturnedValue IntrinsicTypedArrayPrototype::method_slice(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<TypedArray> instance(scope, thisObject);
    if (!instance || instance->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = instance->length();

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
    uint count = start > end ? 0 : end - start;

    TypedArray *a = typedArraySpeciesCreate(scope, instance, count);
    if (!a)
        return Encode::undefined();

    ScopedValue v(scope);
    uint n = 0;
    for (uint i = start; i < end; ++i) {
        if (instance->d()->buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
        v = instance->get(i);
        if (a->d()->buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
        a->put(n, v);
        ++n;
    }
    return a->asReturnedValue();
}

ReturnedValue IntrinsicTypedArrayPrototype::method_subarray(const FunctionObject *builtin, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(builtin);
    Scoped<TypedArray> a(scope, *thisObject);

    if (!a)
        return scope.engine->throwTypeError();

    Scoped<ArrayBuffer> buffer(scope, a->d()->buffer);
    Q_ASSERT(buffer);

    int len = a->length();
    double b = argc > 0 ? argv[0].toInteger() : 0;
    if (b < 0)
        b = len + b;
    uint begin = (uint)qBound(0., b, (double)len);

    double e = argc < 2 || argv[1].isUndefined() ? len : argv[1].toInteger();
    if (e < 0)
        e = len + e;
    uint end = (uint)qBound(0., e, (double)len);
    if (end < begin)
        end = begin;

    if (scope.engine->hasException)
        RETURN_UNDEFINED();

    int newLen = end - begin;

    ScopedFunctionObject constructor(scope, a->speciesConstructor(scope, scope.engine->typedArrayCtors + a->d()->arrayType));
    if (!constructor)
        return scope.engine->throwTypeError();

    Value *arguments = scope.alloc(3);
    arguments[0] = buffer;
    arguments[1] = Encode(a->d()->byteOffset + begin*a->d()->type->bytesPerElement);
    arguments[2] = Encode(newLen);
    a = constructor->callAsConstructor(arguments, 3);
    if (!a || a->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();
    return a->asReturnedValue();
}

ReturnedValue IntrinsicTypedArrayPrototype::method_toLocaleString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<TypedArray> instance(scope, thisObject);
    if (!instance || instance->d()->buffer->isDetachedBuffer())
        return scope.engine->throwTypeError();

    uint len = instance->length();
    const QString separator = QStringLiteral(",");

    QString R;

    ScopedValue v(scope);
    ScopedString s(scope);

    for (uint k = 0; k < len; ++k) {
        if (instance->d()->buffer->isDetachedBuffer())
            return scope.engine->throwTypeError();
        if (k)
            R += separator;

        v = instance->get(k);
        v = Runtime::method_callElement(scope.engine, v, *scope.engine->id_toLocaleString(), nullptr, 0);
        s = v->toString(scope.engine);
        if (scope.hasException())
            return Encode::undefined();

        R += s->toQString();
    }
    return scope.engine->newString(R)->asReturnedValue();
}

ReturnedValue IntrinsicTypedArrayPrototype::method_get_toStringTag(const FunctionObject *, const Value *thisObject, const Value *, int)
{
    const TypedArray *a = thisObject->as<TypedArray>();
    if (!a)
        return Encode::undefined();

    return a->engine()->newString(QString::fromLatin1(a->d()->type->name))->asReturnedValue();
}

static bool validateTypedArray(const Object *o)
{
    const TypedArray *a = o->as<TypedArray>();
    if (!a)
        return false;
    if (a->d()->buffer->isDetachedBuffer())
        return false;
    return true;
}

ReturnedValue IntrinsicTypedArrayCtor::method_of(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(f);
    int len = argc;
    const Value *items = argv;
    const FunctionObject *C = thisObject->as<FunctionObject>();
    if (!C || !C->isConstructor())
        return scope.engine->throwTypeError();

    Value lenValue = Value::fromInt32(len);
    ScopedObject newObj(scope, C->callAsConstructor(&lenValue, 1));
    if (scope.hasException())
        return Encode::undefined();
    if (!::validateTypedArray(newObj))
        return scope.engine->throwTypeError();
    TypedArray *a = newObj->as<TypedArray>();
    Q_ASSERT(a);
    if (a->length() < static_cast<uint>(len))
        return scope.engine->throwTypeError();

    for (int k = 0; k < len; ++k) {
        newObj->put(PropertyKey::fromArrayIndex(k), items[k]);
    }
    return newObj->asReturnedValue();
}

void IntrinsicTypedArrayPrototype::init(ExecutionEngine *engine, IntrinsicTypedArrayCtor *ctor)
{
    Scope scope(engine);
    ctor->defineReadonlyProperty(engine->id_prototype(), *this);
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(0));
    ScopedString s(scope, engine->newString(QStringLiteral("TypedArray")));
    ctor->defineReadonlyConfigurableProperty(engine->id_name(), s);
    s = scope.engine->newString(QStringLiteral("of"));
    ctor->defineDefaultProperty(s, IntrinsicTypedArrayCtor::method_of);
    ctor->addSymbolSpecies();

    defineAccessorProperty(QStringLiteral("buffer"), method_get_buffer, nullptr);
    defineAccessorProperty(QStringLiteral("byteLength"), method_get_byteLength, nullptr);
    defineAccessorProperty(QStringLiteral("byteOffset"), method_get_byteOffset, nullptr);
    defineAccessorProperty(QStringLiteral("length"), method_get_length, nullptr);

    defineDefaultProperty(QStringLiteral("copyWithin"), method_copyWithin, 2);
    defineDefaultProperty(QStringLiteral("entries"), method_entries, 0);
    defineDefaultProperty(QStringLiteral("every"), method_every, 1);
    defineDefaultProperty(QStringLiteral("fill"), method_fill, 1);
    defineDefaultProperty(QStringLiteral("filter"), method_filter, 1);
    defineDefaultProperty(QStringLiteral("find"), method_find, 1);
    defineDefaultProperty(QStringLiteral("findIndex"), method_findIndex, 1);
    defineDefaultProperty(QStringLiteral("forEach"), method_forEach, 1);
    defineDefaultProperty(QStringLiteral("includes"), method_includes, 1);
    defineDefaultProperty(QStringLiteral("indexOf"), method_indexOf, 1);
    defineDefaultProperty(QStringLiteral("join"), method_join, 1);
    defineDefaultProperty(QStringLiteral("keys"), method_keys, 0);
    defineDefaultProperty(QStringLiteral("lastIndexOf"), method_lastIndexOf, 1);
    defineDefaultProperty(QStringLiteral("map"), method_map, 1);
    defineDefaultProperty(QStringLiteral("reduce"), method_reduce, 1);
    defineDefaultProperty(QStringLiteral("reduceRight"), method_reduceRight, 1);
    defineDefaultProperty(QStringLiteral("reverse"), method_reverse, 0);
    defineDefaultProperty(QStringLiteral("some"), method_some, 1);
    defineDefaultProperty(QStringLiteral("set"), method_set, 1);
    defineDefaultProperty(QStringLiteral("slice"), method_slice, 2);
    defineDefaultProperty(QStringLiteral("subarray"), method_subarray, 2);
    defineDefaultProperty(engine->id_toLocaleString(), method_toLocaleString, 0);
    ScopedObject f(scope, engine->arrayPrototype()->get(engine->id_toString()));
    defineDefaultProperty(engine->id_toString(), f);

    ScopedString valuesString(scope, engine->newIdentifier(QStringLiteral("values")));
    ScopedObject values(scope, FunctionObject::createBuiltinFunction(engine, valuesString, method_values, 0));
    defineDefaultProperty(QStringLiteral("values"), values);
    defineDefaultProperty(engine->symbol_iterator(), values);

    defineAccessorProperty(engine->symbol_toStringTag(), method_get_toStringTag, nullptr);
}
