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
#include <qv4argumentsobject_p.h>
#include <qv4arrayobject_p.h>
#include <qv4alloca_p.h>
#include <qv4scopedvalue_p.h>
#include <qv4string_p.h>
#include <qv4function_p.h>
#include <qv4jscall_p.h>
#include <qv4symbol_p.h>

using namespace QV4;

DEFINE_OBJECT_VTABLE(ArgumentsObject);
DEFINE_OBJECT_VTABLE(StrictArgumentsObject);

void Heap::StrictArgumentsObject::init(QV4::CppStackFrame *frame)

{
    Q_ASSERT(vtable() == QV4::StrictArgumentsObject::staticVTable());
    ExecutionEngine *v4 = internalClass->engine;

    Object::init();

    Q_ASSERT(internalClass->verifyIndex(v4->id_callee()->propertyKey(), CalleePropertyIndex));
    Q_ASSERT(internalClass->findValueOrSetter(v4->id_callee()->propertyKey()).index == CalleeSetterPropertyIndex);
    Q_ASSERT(internalClass->verifyIndex(v4->symbol_iterator()->propertyKey(), SymbolIteratorPropertyIndex));
    setProperty(v4, SymbolIteratorPropertyIndex, *v4->arrayProtoValues());
    setProperty(v4, CalleePropertyIndex, *v4->thrower());
    setProperty(v4, CalleeSetterPropertyIndex, *v4->thrower());

    Scope scope(v4);
    Scoped<QV4::StrictArgumentsObject> args(scope, this);
    args->arrayReserve(frame->originalArgumentsCount);
    args->arrayPut(0, frame->originalArguments, frame->originalArgumentsCount);

    Q_ASSERT(args->internalClass()->verifyIndex(v4->id_length()->propertyKey(), LengthPropertyIndex));
    setProperty(v4, LengthPropertyIndex, Value::fromInt32(frame->originalArgumentsCount));
}

void Heap::ArgumentsObject::init(QV4::CppStackFrame *frame)
{
    ExecutionEngine *v4 = internalClass->engine;

    QV4::CallContext *context = static_cast<QV4::CallContext *>(frame->context());

    Object::init();
    this->context.set(v4, context->d());
    Q_ASSERT(vtable() == QV4::ArgumentsObject::staticVTable());

    Q_ASSERT(internalClass->verifyIndex(v4->id_callee()->propertyKey(), CalleePropertyIndex));
    setProperty(v4, CalleePropertyIndex, context->d()->function);
    Q_ASSERT(internalClass->verifyIndex(v4->id_length()->propertyKey(), LengthPropertyIndex));
    setProperty(v4, LengthPropertyIndex, Value::fromInt32(context->argc()));
    Q_ASSERT(internalClass->verifyIndex(v4->symbol_iterator()->propertyKey(), SymbolIteratorPropertyIndex));
    setProperty(v4, SymbolIteratorPropertyIndex, *v4->arrayProtoValues());

    fullyCreated = false;
    argCount = frame->originalArgumentsCount;
    uint nFormals = frame->v4Function->nFormals;
    mapped = nFormals > 63 ? std::numeric_limits<quint64>::max() : (1ull << nFormals) - 1;
}

void ArgumentsObject::fullyCreate()
{
    if (d()->fullyCreated)
        return;

    Scope scope(engine());

    arrayReserve(d()->argCount);
    arrayPut(0, context()->args(), d()->argCount);
    // Use a sparse array, so that method_getElement() doesn't shortcut
    initSparseArray();

    d()->fullyCreated = true;
}

bool ArgumentsObject::virtualDefineOwnProperty(Managed *m, PropertyKey id, const Property *desc, PropertyAttributes attrs)
{
    ArgumentsObject *args = static_cast<ArgumentsObject *>(m);
    args->fullyCreate();
    uint index = id.asArrayIndex();

    if (!args->isMapped(index))
        return Object::virtualDefineOwnProperty(m, id, desc, attrs);

    Scope scope(args);
    PropertyAttributes cAttrs = attrs;
    ScopedProperty cDesc(scope);
    cDesc->copy(desc, attrs);

    if (attrs.isData() && desc->value.isEmpty() && attrs.hasWritable() && !attrs.isWritable()) {
        cDesc->value = args->context()->args()[index];
        cAttrs.setType(PropertyAttributes::Data);
    }

    bool allowed = Object::virtualDefineOwnProperty(m, id, cDesc, cAttrs);
    if (!allowed)
        return false;

    if (attrs.isAccessor()) {
        args->removeMapping(index);
    } else {
        if (!desc->value.isEmpty())
            args->context()->setArg(index, desc->value);
        if (attrs.hasWritable() && !attrs.isWritable())
            args->removeMapping(index);
    }
    return true;
}

ReturnedValue ArgumentsObject::virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty)
{
    const ArgumentsObject *args = static_cast<const ArgumentsObject *>(m);
    uint index = id.asArrayIndex();
    if (index < args->d()->argCount && !args->d()->fullyCreated) {
        if (hasProperty)
            *hasProperty = true;
        return args->context()->args()[index].asReturnedValue();
    }

    if (!args->isMapped(index))
        return Object::virtualGet(m, id, receiver, hasProperty);
    Q_ASSERT(index < static_cast<uint>(args->context()->function->formalParameterCount()));
    if (hasProperty)
        *hasProperty = true;
    return args->context()->args()[index].asReturnedValue();
}

bool ArgumentsObject::virtualPut(Managed *m, PropertyKey id, const Value &value, Value *receiver)
{
    ArgumentsObject *args = static_cast<ArgumentsObject *>(m);
    uint index = id.asArrayIndex();

    if (args == receiver && index < args->d()->argCount && !args->d()->fullyCreated) {
        args->context()->setArg(index, value);
        return true;
    }

    bool isMapped = (args == receiver && args->isMapped(index));
    if (isMapped)
        args->context()->setArg(index, value);

    return Object::virtualPut(m, id, value, receiver);
}

bool ArgumentsObject::virtualDeleteProperty(Managed *m, PropertyKey id)
{
    ArgumentsObject *args = static_cast<ArgumentsObject *>(m);
    args->fullyCreate();
    bool result = Object::virtualDeleteProperty(m, id);
    if (result)
        args->removeMapping(id.asArrayIndex());
    return result;
}

PropertyAttributes ArgumentsObject::virtualGetOwnProperty(const Managed *m, PropertyKey id, Property *p)
{
    const ArgumentsObject *args = static_cast<const ArgumentsObject *>(m);
    uint index = id.asArrayIndex();
    if (index < args->d()->argCount && !args->d()->fullyCreated) {
        p->value = args->context()->args()[index];
        return Attr_Data;
    }

    PropertyAttributes attrs = Object::virtualGetOwnProperty(m, id, p);
    if (attrs.isEmpty() || !args->isMapped(index))
        return attrs;

    Q_ASSERT(index < static_cast<uint>(args->context()->function->formalParameterCount()));
    if (p)
        p->value = args->context()->args()[index];
    return attrs;
}

qint64 ArgumentsObject::virtualGetLength(const Managed *m)
{
    const ArgumentsObject *a = static_cast<const ArgumentsObject *>(m);
    return a->propertyData(Heap::ArgumentsObject::LengthPropertyIndex)->toLength();
}

OwnPropertyKeyIterator *ArgumentsObject::virtualOwnPropertyKeys(const Object *m, Value *target)
{
    static_cast<ArgumentsObject *>(const_cast<Object *>(m))->fullyCreate();
    return Object::virtualOwnPropertyKeys(m, target);
}
