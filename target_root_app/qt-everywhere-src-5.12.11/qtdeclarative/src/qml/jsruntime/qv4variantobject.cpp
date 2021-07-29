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

#include "qv4variantobject_p.h"
#include "qv4functionobject_p.h"
#include "qv4objectproto_p.h"
#include <private/qqmlvaluetypewrapper_p.h>
#include <private/qv8engine_p.h>
#include <private/qv4qobjectwrapper_p.h>

QT_BEGIN_NAMESPACE

using namespace QV4;

DEFINE_OBJECT_VTABLE(VariantObject);

void Heap::VariantObject::init()
{
    Object::init();
    scarceData = new ExecutionEngine::ScarceResourceData;
}

void Heap::VariantObject::init(const QVariant &value)
{
    Object::init();
    scarceData = new ExecutionEngine::ScarceResourceData(value);
    if (isScarce())
        removeVmePropertyReference();
}

bool VariantObject::Data::isScarce() const
{
    QVariant::Type t = data().type();
    return t == QVariant::Pixmap || t == QVariant::Image;
}

bool VariantObject::virtualIsEqualTo(Managed *m, Managed *other)
{
    Q_ASSERT(m->as<QV4::VariantObject>());
    QV4::VariantObject *lv = static_cast<QV4::VariantObject *>(m);

    if (QV4::VariantObject *rv = other->as<QV4::VariantObject>())
        return lv->d()->data() == rv->d()->data();

    if (QV4::QQmlValueTypeWrapper *v = other->as<QQmlValueTypeWrapper>())
        return v->isEqual(lv->d()->data());

    return false;
}

void VariantObject::addVmePropertyReference() const
{
    if (d()->isScarce() && ++d()->vmePropertyReferenceCount == 1) {
        // remove from the ep->scarceResources list
        // since it is now no longer eligible to be
        // released automatically by the engine.
        d()->addVmePropertyReference();
    }
}

void VariantObject::removeVmePropertyReference() const
{
    if (d()->isScarce() && --d()->vmePropertyReferenceCount == 0) {
        // and add to the ep->scarceResources list
        // since it is now eligible to be released
        // automatically by the engine.
        d()->removeVmePropertyReference();
    }
}


void VariantPrototype::init()
{
    defineDefaultProperty(QStringLiteral("preserve"), method_preserve, 0);
    defineDefaultProperty(QStringLiteral("destroy"), method_destroy, 0);
    defineDefaultProperty(engine()->id_valueOf(), method_valueOf, 0);
    defineDefaultProperty(engine()->id_toString(), method_toString, 0);
}

ReturnedValue VariantPrototype::method_preserve(const FunctionObject *, const Value *thisObject, const Value *, int)
{
    const VariantObject *o = thisObject->as<QV4::VariantObject>();
    if (o && o->d()->isScarce())
        o->d()->addVmePropertyReference();
    RETURN_UNDEFINED();
}

ReturnedValue VariantPrototype::method_destroy(const FunctionObject *, const Value *thisObject, const Value *, int)
{
    const VariantObject *o = thisObject->as<QV4::VariantObject>();
    if (o) {
        if (o->d()->isScarce())
            o->d()->addVmePropertyReference();
        o->d()->data() = QVariant();
    }
    RETURN_UNDEFINED();
}

ReturnedValue VariantPrototype::method_toString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    const VariantObject *o = thisObject->as<QV4::VariantObject>();
    if (!o)
        RETURN_UNDEFINED();
    QString result = o->d()->data().toString();
    if (result.isEmpty() && !o->d()->data().canConvert(QVariant::String)) {
        result = QLatin1String("QVariant(")
                 + QLatin1String(o->d()->data().typeName())
                 + QLatin1Char(')');
    }
    return Encode(v4->newString(result));
}

ReturnedValue VariantPrototype::method_valueOf(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    const VariantObject *o = thisObject->as<QV4::VariantObject>();
    if (o) {
        QVariant v = o->d()->data();
        switch (v.type()) {
        case QVariant::Invalid:
            return Encode::undefined();
        case QVariant::String:
            return Encode(b->engine()->newString(v.toString()));
        case QVariant::Int:
            return Encode(v.toInt());
        case QVariant::Double:
        case QVariant::UInt:
            return Encode(v.toDouble());
        case QVariant::Bool:
            return Encode(v.toBool());
        default:
            if (QMetaType::typeFlags(v.userType()) & QMetaType::IsEnumeration)
                RETURN_RESULT(Encode(v.toInt()));
            break;
        }
    }
    return thisObject->asReturnedValue();
}

QT_END_NAMESPACE
