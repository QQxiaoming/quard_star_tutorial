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

#include "qv4string_p.h"
#include "qv4value_p.h"
#ifndef V4_BOOTSTRAP
#include "qv4identifiertable_p.h"
#include "qv4runtime_p.h"
#include "qv4objectproto_p.h"
#include "qv4stringobject_p.h"
#endif
#include <QtCore/QHash>
#include <QtCore/private/qnumeric_p.h>

using namespace QV4;

#ifndef V4_BOOTSTRAP

void Heap::StringOrSymbol::markObjects(Heap::Base *that, MarkStack *markStack)
{
    StringOrSymbol *s = static_cast<StringOrSymbol *>(that);
    Heap::StringOrSymbol *id = s->identifier.asStringOrSymbol();
    if (id)
        id->mark(markStack);
}

void Heap::String::markObjects(Heap::Base *that, MarkStack *markStack)
{
    StringOrSymbol::markObjects(that, markStack);
    String *s = static_cast<String *>(that);
    if (s->subtype < StringType_Complex)
        return;

    ComplexString *cs = static_cast<ComplexString *>(s);
    if (cs->subtype == StringType_AddedString) {
        cs->left->mark(markStack);
        cs->right->mark(markStack);
    } else {
        Q_ASSERT(cs->subtype == StringType_SubString);
        cs->left->mark(markStack);
    }
}

DEFINE_MANAGED_VTABLE(StringOrSymbol);
DEFINE_MANAGED_VTABLE(String);


bool String::virtualIsEqualTo(Managed *t, Managed *o)
{
    if (t == o)
        return true;

    if (!o->vtable()->isString)
        return false;

    return static_cast<String *>(t)->isEqualTo(static_cast<String *>(o));
}


void Heap::String::init(const QString &t)
{
    Base::init();

    subtype = String::StringType_Unknown;

    text = const_cast<QString &>(t).data_ptr();
    text->ref.ref();
}

void Heap::ComplexString::init(String *l, String *r)
{
    Base::init();

    subtype = String::StringType_AddedString;

    left = l;
    right = r;
    len = left->length() + right->length();
    if (left->subtype >= StringType_Complex)
        largestSubLength = static_cast<ComplexString *>(left)->largestSubLength;
    else
        largestSubLength = left->length();
    if (right->subtype >= StringType_Complex)
        largestSubLength = qMax(largestSubLength, static_cast<ComplexString *>(right)->largestSubLength);
    else
        largestSubLength = qMax(largestSubLength, right->length());

    // make sure we don't get excessive depth in our strings
    if (len > 256 && len >= 2*largestSubLength)
        simplifyString();
}

void Heap::ComplexString::init(Heap::String *ref, int from, int len)
{
    Q_ASSERT(ref->length() >= from + len);
    Base::init();

    subtype = String::StringType_SubString;

    left = ref;
    this->from = from;
    this->len = len;
}

void Heap::StringOrSymbol::destroy()
{
    if (text) {
        internalClass->engine->memoryManager->changeUnmanagedHeapSizeUsage(qptrdiff(-text->size) * (int)sizeof(QChar));
        if (!text->ref.deref())
            QStringData::deallocate(text);
    }
    Base::destroy();
}

uint String::toUInt(bool *ok) const
{
    *ok = true;

    if (subtype() >= Heap::String::StringType_Unknown)
        d()->createHashValue();
    if (subtype() == Heap::String::StringType_ArrayIndex)
        return d()->stringHash;

    // required for UINT_MAX or numbers starting with a leading 0
    double d = RuntimeHelpers::stringToNumber(toQString());
    uint l = (uint)d;
    if (d == l)
        return l;
    *ok = false;
    return UINT_MAX;
}

void String::createPropertyKeyImpl() const
{
    if (!d()->text)
        d()->simplifyString();
    Q_ASSERT(d()->text);
    engine()->identifierTable->asPropertyKey(this);
}

void Heap::String::simplifyString() const
{
    Q_ASSERT(!text);

    int l = length();
    QString result(l, Qt::Uninitialized);
    QChar *ch = const_cast<QChar *>(result.constData());
    append(this, ch);
    text = result.data_ptr();
    text->ref.ref();
    const ComplexString *cs = static_cast<const ComplexString *>(this);
    identifier = PropertyKey::invalid();
    cs->left = cs->right = nullptr;

    internalClass->engine->memoryManager->changeUnmanagedHeapSizeUsage(qptrdiff(text->size) * (qptrdiff)sizeof(QChar));
    subtype = StringType_Unknown;
}

bool Heap::String::startsWithUpper() const
{
    if (subtype == StringType_AddedString)
        return static_cast<const Heap::ComplexString *>(this)->left->startsWithUpper();

    const Heap::String *str = this;
    int offset = 0;
    if (subtype == StringType_SubString) {
        const ComplexString *cs = static_cast<const Heap::ComplexString *>(this);
        if (!cs->len)
            return false;
        // simplification here is not ideal, but hopefully not a common case.
        if (cs->left->subtype >= Heap::String::StringType_Complex)
            cs->left->simplifyString();
        str = cs->left;
        offset = cs->from;
    }
    Q_ASSERT(str->subtype < Heap::String::StringType_Complex);
    return str->text->size > offset && QChar::isUpper(str->text->data()[offset]);
}

void Heap::String::append(const String *data, QChar *ch)
{
    std::vector<const String *> worklist;
    worklist.reserve(32);
    worklist.push_back(data);

    while (!worklist.empty()) {
        const String *item = worklist.back();
        worklist.pop_back();

        if (item->subtype == StringType_AddedString) {
            const ComplexString *cs = static_cast<const ComplexString *>(item);
            worklist.push_back(cs->right);
            worklist.push_back(cs->left);
        } else if (item->subtype == StringType_SubString) {
            const ComplexString *cs = static_cast<const ComplexString *>(item);
            memcpy(ch, cs->left->toQString().constData() + cs->from, cs->len*sizeof(QChar));
            ch += cs->len;
        } else {
            memcpy(static_cast<void *>(ch), static_cast<const void *>(item->text->data()), item->text->size * sizeof(QChar));
            ch += item->text->size;
        }
    }
}

void Heap::StringOrSymbol::createHashValue() const
{
    if (!text) {
        Q_ASSERT(internalClass->vtable->isString);
        static_cast<const Heap::String *>(this)->simplifyString();
    }
    Q_ASSERT(text);
    const QChar *ch = reinterpret_cast<const QChar *>(text->data());
    const QChar *end = ch + text->size;
    stringHash = QV4::String::calculateHashValue(ch, end, &subtype);
}

qint64 String::virtualGetLength(const Managed *m)
{
    return static_cast<const String *>(m)->d()->length();
}

#endif // V4_BOOTSTRAP

uint String::toArrayIndex(const QString &str)
{
    return QV4::String::toArrayIndex(str.constData(), str.constData() + str.length());
}
