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

#include "qqmllistwrapper_p.h"
#include <private/qv8engine_p.h>
#include <private/qqmllist_p.h>
#include <private/qv4objectproto_p.h>
#include <qv4objectiterator_p.h>

#include <private/qv4functionobject_p.h>
#include <private/qv4qobjectwrapper_p.h>

QT_BEGIN_NAMESPACE

using namespace QV4;

DEFINE_OBJECT_VTABLE(QmlListWrapper);

void Heap::QmlListWrapper::init()
{
    Object::init();
    object.init();
    QV4::Scope scope(internalClass->engine);
    QV4::ScopedObject o(scope, this);
    o->setArrayType(Heap::ArrayData::Custom);
}

void Heap::QmlListWrapper::destroy()
{
    object.destroy();
    Object::destroy();
}

ReturnedValue QmlListWrapper::create(ExecutionEngine *engine, QObject *object, int propId, int propType)
{
    if (!object || propId == -1)
        return Encode::null();

    Scope scope(engine);

    Scoped<QmlListWrapper> r(scope, engine->memoryManager->allocate<QmlListWrapper>());
    r->d()->object = object;
    r->d()->propertyType = propType;
    void *args[] = { &r->d()->property(), nullptr };
    QMetaObject::metacall(object, QMetaObject::ReadProperty, propId, args);
    return r.asReturnedValue();
}

ReturnedValue QmlListWrapper::create(ExecutionEngine *engine, const QQmlListProperty<QObject> &prop, int propType)
{
    Scope scope(engine);

    Scoped<QmlListWrapper> r(scope, engine->memoryManager->allocate<QmlListWrapper>());
    r->d()->object = prop.object;
    r->d()->property() = prop;
    r->d()->propertyType = propType;
    return r.asReturnedValue();
}

QVariant QmlListWrapper::toVariant() const
{
    if (!d()->object)
        return QVariant();

    return QVariant::fromValue(QQmlListReferencePrivate::init(d()->property(), d()->propertyType, engine()->qmlEngine()));
}


ReturnedValue QmlListWrapper::virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty)
{
    Q_ASSERT(m->as<QmlListWrapper>());
    const QmlListWrapper *w = static_cast<const QmlListWrapper *>(m);
    QV4::ExecutionEngine *v4 = w->engine();

    if (id.isArrayIndex()) {
        uint index = id.asArrayIndex();
        quint32 count = w->d()->property().count ? w->d()->property().count(&w->d()->property()) : 0;
        if (index < count && w->d()->property().at) {
            if (hasProperty)
                *hasProperty = true;
            return QV4::QObjectWrapper::wrap(v4, w->d()->property().at(&w->d()->property(), index));
        }

        if (hasProperty)
            *hasProperty = false;
        return Value::undefinedValue().asReturnedValue();
    } else if (id.isString()) {
        if (id == v4->id_length()->propertyKey() && !w->d()->object.isNull()) {
            quint32 count = w->d()->property().count ? w->d()->property().count(&w->d()->property()) : 0;
            return Value::fromUInt32(count).asReturnedValue();
        }
    }

    return Object::virtualGet(m, id, receiver, hasProperty);
}

bool QmlListWrapper::virtualPut(Managed *m, PropertyKey id, const Value &value, Value *receiver)
{
    // doesn't do anything. Should we throw?
    Q_UNUSED(m);
    Q_UNUSED(id);
    Q_UNUSED(value);
    Q_UNUSED(receiver);
    return false;
}

struct QmlListWrapperOwnPropertyKeyIterator : ObjectOwnPropertyKeyIterator
{
    ~QmlListWrapperOwnPropertyKeyIterator() override = default;
    PropertyKey next(const Object *o, Property *pd = nullptr, PropertyAttributes *attrs = nullptr) override;

};

PropertyKey QmlListWrapperOwnPropertyKeyIterator::next(const Object *o, Property *pd, PropertyAttributes *attrs)
{
    const QmlListWrapper *w = static_cast<const QmlListWrapper *>(o);

    quint32 count = w->d()->property().count ? w->d()->property().count(&w->d()->property()) : 0;
    if (arrayIndex < count) {
        uint index = arrayIndex;
        ++arrayIndex;
        if (attrs)
            *attrs = QV4::Attr_Data;
        if (pd)
            pd->value = QV4::QObjectWrapper::wrap(w->engine(), w->d()->property().at(&w->d()->property(), index));
        return PropertyKey::fromArrayIndex(index);
    }

    return ObjectOwnPropertyKeyIterator::next(o, pd, attrs);
}

OwnPropertyKeyIterator *QmlListWrapper::virtualOwnPropertyKeys(const Object *m, Value *target)
{
    *target = *m;
    return new QmlListWrapperOwnPropertyKeyIterator;
}

void PropertyListPrototype::init(ExecutionEngine *)
{
    defineDefaultProperty(QStringLiteral("push"), method_push, 1);
}

ReturnedValue PropertyListPrototype::method_push(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedObject instance(scope, thisObject->toObject(scope.engine));
    if (!instance)
        RETURN_UNDEFINED();
    QmlListWrapper *w = instance->as<QmlListWrapper>();
    if (!w)
        RETURN_UNDEFINED();
    if (!w->d()->property().append)
        THROW_GENERIC_ERROR("List doesn't define an Append function");

    QV4::ScopedObject so(scope);
    for (int i = 0, ei = argc; i < ei; ++i)
    {
        so = argv[i].toObject(scope.engine);
        if (QV4::QObjectWrapper *wrapper = so->as<QV4::QObjectWrapper>())
            w->d()->property().append(&w->d()->property(), wrapper->object() );
    }
    return Encode::undefined();
}

QT_END_NAMESPACE
