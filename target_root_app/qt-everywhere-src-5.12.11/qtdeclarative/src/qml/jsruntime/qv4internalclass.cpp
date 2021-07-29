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

#include <qv4internalclass_p.h>
#include <qv4string_p.h>
#include <qv4engine_p.h>
#include <qv4identifier_p.h>
#include "qv4object_p.h"
#include "qv4identifiertable_p.h"
#include "qv4value_p.h"
#include "qv4mm_p.h"

QT_BEGIN_NAMESPACE

namespace QV4 {

static const uchar prime_deltas[] = {
    0,  0,  1,  3,  1,  5,  3,  3,  1,  9,  7,  5,  3,  9, 25,  3,
    1, 21,  3, 21,  7, 15,  9,  5,  3, 29, 15,  0,  0,  0,  0,  0
};

static inline int primeForNumBits(int numBits)
{
    return (1 << numBits) + prime_deltas[numBits];
}

PropertyHashData::PropertyHashData(int numBits)
    : refCount(1)
    , size(0)
    , numBits(numBits)
{
    alloc = primeForNumBits(numBits);
    entries = (PropertyHash::Entry *)malloc(alloc*sizeof(PropertyHash::Entry));
    memset(entries, 0, alloc*sizeof(PropertyHash::Entry));
}

void PropertyHash::addEntry(const PropertyHash::Entry &entry, int classSize)
{
    // fill up to max 50%
    bool grow = (d->alloc <= d->size*2);

    if (classSize < d->size || grow)
        detach(grow, classSize);

    uint idx = entry.identifier.id() % d->alloc;
    while (d->entries[idx].identifier.isValid()) {
        ++idx;
        idx %= d->alloc;
    }
    d->entries[idx] = entry;
    ++d->size;
}

int PropertyHash::removeIdentifier(PropertyKey identifier, int classSize)
{
    int val = -1;
    PropertyHashData *dd = new PropertyHashData(d->numBits);
    for (int i = 0; i < d->alloc; ++i) {
        const Entry &e = d->entries[i];
        if (!e.identifier.isValid() || e.index >= static_cast<unsigned>(classSize))
            continue;
        if (e.identifier == identifier) {
            val = e.index;
            continue;
        }
        uint idx = e.identifier.id() % dd->alloc;
        while (dd->entries[idx].identifier.isValid()) {
            ++idx;
            idx %= dd->alloc;
        }
        dd->entries[idx] = e;
    }
    dd->size = classSize;
    if (!--d->refCount)
        delete d;
    d = dd;

    Q_ASSERT(val != -1);
    return val;
}

void PropertyHash::detach(bool grow, int classSize)
{
    if (d->refCount == 1 && !grow)
        return;

    PropertyHashData *dd = new PropertyHashData(grow ? d->numBits + 1 : d->numBits);
    for (int i = 0; i < d->alloc; ++i) {
        const Entry &e = d->entries[i];
        if (!e.identifier.isValid() || e.index >= static_cast<unsigned>(classSize))
            continue;
        uint idx = e.identifier.id() % dd->alloc;
        while (dd->entries[idx].identifier.isValid()) {
            ++idx;
            idx %= dd->alloc;
        }
        dd->entries[idx] = e;
    }
    dd->size = classSize;
    if (!--d->refCount)
        delete d;
    d = dd;
}


SharedInternalClassDataPrivate<PropertyKey>::SharedInternalClassDataPrivate(const SharedInternalClassDataPrivate<PropertyKey> &other)
    : refcount(1),
      engine(other.engine),
      data(nullptr)
{
    if (other.alloc()) {
        const uint s = other.size();
        data = MemberData::allocate(engine, other.alloc(), other.data);
        setSize(s);
    }
}

SharedInternalClassDataPrivate<PropertyKey>::SharedInternalClassDataPrivate(const SharedInternalClassDataPrivate<PropertyKey> &other,
                                                                            uint pos, PropertyKey value)
    : refcount(1),
      engine(other.engine)
{
    data = MemberData::allocate(engine, other.alloc(), nullptr);
    memcpy(data, other.data, sizeof(Heap::MemberData) - sizeof(Value) + pos*sizeof(Value));
    data->values.size = pos + 1;
    data->values.set(engine, pos, Value::fromReturnedValue(value.id()));
}

void SharedInternalClassDataPrivate<PropertyKey>::grow()
{
    const uint a = alloc() * 2;
    const uint s = size();
    data = MemberData::allocate(engine, a, data);
    setSize(s);
    Q_ASSERT(alloc() >= a);
}

uint SharedInternalClassDataPrivate<PropertyKey>::alloc() const
{
    return data ? data->values.alloc : 0;
}

uint SharedInternalClassDataPrivate<PropertyKey>::size() const
{
    return data ? data->values.size : 0;
}

void SharedInternalClassDataPrivate<PropertyKey>::setSize(uint s)
{
    Q_ASSERT(data && s <= alloc());
    data->values.size = s;
}

PropertyKey SharedInternalClassDataPrivate<PropertyKey>::at(uint i)
{
    Q_ASSERT(data && i < size());
    return PropertyKey::fromId(data->values.values[i].rawValue());
}

void SharedInternalClassDataPrivate<PropertyKey>::set(uint i, PropertyKey t)
{
    Q_ASSERT(data && i < size());
    data->values.values[i].rawValueRef() = t.id();
}

void SharedInternalClassDataPrivate<PropertyKey>::mark(MarkStack *s)
{
    if (data)
        data->mark(s);
}

SharedInternalClassDataPrivate<PropertyAttributes>::SharedInternalClassDataPrivate(
        const SharedInternalClassDataPrivate<PropertyAttributes> &other, uint pos,
        PropertyAttributes value)
    : refcount(1),
      m_alloc(qMin(other.m_alloc, pos + 8)),
      m_size(pos + 1),
      m_engine(other.m_engine)
{
    Q_ASSERT(m_size <= m_alloc);
    m_engine->memoryManager->changeUnmanagedHeapSizeUsage(m_alloc * sizeof(PropertyAttributes));
    data = new PropertyAttributes[m_alloc];
    if (other.data)
        memcpy(data, other.data, (m_size - 1) * sizeof(PropertyAttributes));
    data[pos] = value;
}

SharedInternalClassDataPrivate<PropertyAttributes>::SharedInternalClassDataPrivate(
        const SharedInternalClassDataPrivate<PropertyAttributes> &other)
    : refcount(1),
      m_alloc(other.m_alloc),
      m_size(other.m_size),
      m_engine(other.m_engine)
{
    if (m_alloc) {
        m_engine->memoryManager->changeUnmanagedHeapSizeUsage(m_alloc * sizeof(PropertyAttributes));
        data = new PropertyAttributes[m_alloc];
        memcpy(data, other.data, m_size*sizeof(PropertyAttributes));
    } else {
        data = nullptr;
    }
}

SharedInternalClassDataPrivate<PropertyAttributes>::~SharedInternalClassDataPrivate()
{
    m_engine->memoryManager->changeUnmanagedHeapSizeUsage(
            -qptrdiff(m_alloc * sizeof(PropertyAttributes)));
    delete [] data;
}

void SharedInternalClassDataPrivate<PropertyAttributes>::grow() {
    uint alloc;
    if (!m_alloc) {
        alloc = 8;
        m_engine->memoryManager->changeUnmanagedHeapSizeUsage(alloc * sizeof(PropertyAttributes));
    } else {
        // yes, signed. We don't want to deal with stuff > 2G
        const uint currentSize = m_alloc * sizeof(PropertyAttributes);
        if (currentSize < uint(std::numeric_limits<int>::max() / 2))
            alloc = m_alloc * 2;
        else
            alloc = std::numeric_limits<int>::max() / sizeof(PropertyAttributes);

        m_engine->memoryManager->changeUnmanagedHeapSizeUsage(
                (alloc - m_alloc) * sizeof(PropertyAttributes));
    }

    auto *n = new PropertyAttributes[alloc];
    if (data) {
        memcpy(n, data, m_alloc*sizeof(PropertyAttributes));
        delete [] data;
    }
    data = n;
    m_alloc = alloc;
}

namespace Heap {

void InternalClass::init(ExecutionEngine *engine)
{
    Base::init();
    new (&propertyTable) PropertyHash();
    new (&nameMap) SharedInternalClassData<PropertyKey>(engine);
    new (&propertyData) SharedInternalClassData<PropertyAttributes>(engine);
    new (&transitions) std::vector<Transition>();

    this->engine = engine;
    vtable = QV4::InternalClass::staticVTable();
//    prototype = nullptr;
//    parent = nullptr;
//    size = 0;
    extensible = true;
    isFrozen = false;
    isSealed = false;
    isUsedAsProto = false;
    protoId = engine->newProtoId();

    // Also internal classes need an internal class pointer. Simply make it point to itself
    internalClass.set(engine, this);
}


void InternalClass::init(Heap::InternalClass *other)
{
    Base::init();
    Q_ASSERT(!other->isFrozen);
    new (&propertyTable) PropertyHash(other->propertyTable);
    new (&nameMap) SharedInternalClassData<PropertyKey>(other->nameMap);
    new (&propertyData) SharedInternalClassData<PropertyAttributes>(other->propertyData);
    new (&transitions) std::vector<Transition>();

    engine = other->engine;
    vtable = other->vtable;
    prototype = other->prototype;
    parent = other;
    size = other->size;
    extensible = other->extensible;
    isSealed = other->isSealed;
    isFrozen = other->isFrozen;
    isUsedAsProto = other->isUsedAsProto;
    protoId = engine->newProtoId();

    internalClass.set(engine, other->internalClass);
}

void InternalClass::destroy()
{
    for (const auto &t : transitions) {
        if (t.lookup) {
#ifndef QT_NO_DEBUG
            Q_ASSERT(t.lookup->parent == this);
#endif
            t.lookup->parent = nullptr;
        }
    }

    if (parent && parent->engine && parent->isMarked())
        parent->removeChildEntry(this);

    propertyTable.~PropertyHash();
    nameMap.~SharedInternalClassData<PropertyKey>();
    propertyData.~SharedInternalClassData<PropertyAttributes>();
    transitions.~vector<Transition>();
    engine = nullptr;
    Base::destroy();
}

QString InternalClass::keyAt(uint index) const
{
    return nameMap.at(index).toQString();
}

void InternalClass::changeMember(QV4::Object *object, PropertyKey id, PropertyAttributes data, InternalClassEntry *entry)
{
    Q_ASSERT(id.isStringOrSymbol());

    Heap::InternalClass *oldClass = object->internalClass();
    Heap::InternalClass *newClass = oldClass->changeMember(id, data, entry);
    object->setInternalClass(newClass);
}

InternalClassTransition &InternalClass::lookupOrInsertTransition(const InternalClassTransition &t)
{
    std::vector<Transition>::iterator it = std::lower_bound(transitions.begin(), transitions.end(), t);
    if (it != transitions.end() && *it == t) {
        return *it;
    } else {
        it = transitions.insert(it, t);
        return *it;
    }
}

static void addDummyEntry(InternalClass *newClass, PropertyHash::Entry e)
{
    // add a dummy entry, since we need two entries for accessors
    newClass->propertyTable.addEntry(e, newClass->size);
    newClass->nameMap.add(newClass->size, PropertyKey::invalid());
    newClass->propertyData.add(newClass->size, PropertyAttributes());
    ++newClass->size;
}

Heap::InternalClass *InternalClass::changeMember(PropertyKey identifier, PropertyAttributes data, InternalClassEntry *entry)
{
    if (!data.isEmpty())
        data.resolve();
    PropertyHash::Entry *e = findEntry(identifier);
    Q_ASSERT(e && e->index != UINT_MAX);
    uint idx = e->index;
    Q_ASSERT(idx != UINT_MAX);

    if (entry) {
        entry->index = idx;
        entry->setterIndex = e->setterIndex;
        entry->attributes = data;
    }

    if (data == propertyData.at(idx))
        return static_cast<Heap::InternalClass *>(this);

    Transition temp = { { identifier }, nullptr, int(data.all()) };
    Transition &t = lookupOrInsertTransition(temp);
    if (t.lookup)
        return t.lookup;

    // create a new class and add it to the tree
    Heap::InternalClass *newClass = engine->newClass(this);
    if (data.isAccessor() && e->setterIndex == UINT_MAX) {
        Q_ASSERT(!propertyData.at(idx).isAccessor());

        // add a dummy entry for the accessor
        entry->setterIndex = newClass->size;
        e->setterIndex = newClass->size;
        addDummyEntry(newClass, *e);
    }

    newClass->propertyData.set(idx, data);

    t.lookup = newClass;
    Q_ASSERT(t.lookup);
    return newClass;
}

Heap::InternalClass *InternalClass::changePrototypeImpl(Heap::Object *proto)
{
    Scope scope(engine);
    ScopedValue protectThis(scope, this);
    if (proto)
        proto->setUsedAsProto();
    Q_ASSERT(prototype != proto);
    Q_ASSERT(!proto || proto->internalClass->isUsedAsProto);

    Transition temp = { { PropertyKey::invalid() }, nullptr, Transition::PrototypeChange };
    temp.prototype = proto;

    Transition &t = lookupOrInsertTransition(temp);
    if (t.lookup)
        return t.lookup;

    // create a new class and add it to the tree
    Heap::InternalClass *newClass = engine->newClass(this);
    newClass->prototype = proto;

    t.lookup = newClass;

    return newClass;
}

Heap::InternalClass *InternalClass::changeVTableImpl(const VTable *vt)
{
    Q_ASSERT(vtable != vt);

    Transition temp = { { PropertyKey::invalid() }, nullptr, Transition::VTableChange };
    temp.vtable = vt;

    Transition &t = lookupOrInsertTransition(temp);
    if (t.lookup)
        return t.lookup;

    // create a new class and add it to the tree
    Heap::InternalClass *newClass = engine->newClass(this);
    newClass->vtable = vt;

    t.lookup = newClass;
    Q_ASSERT(t.lookup);
    Q_ASSERT(newClass->vtable);
    return newClass;
}

Heap::InternalClass *InternalClass::nonExtensible()
{
    if (!extensible)
        return this;

    Transition temp = { { PropertyKey::invalid() }, nullptr, Transition::NotExtensible};
    Transition &t = lookupOrInsertTransition(temp);
    if (t.lookup)
        return t.lookup;

    Heap::InternalClass *newClass = engine->newClass(this);
    newClass->extensible = false;

    t.lookup = newClass;
    Q_ASSERT(t.lookup);
    return newClass;
}

void InternalClass::addMember(QV4::Object *object, PropertyKey id, PropertyAttributes data, InternalClassEntry *entry)
{
    Q_ASSERT(id.isStringOrSymbol());
    if (!data.isEmpty())
        data.resolve();
    PropertyHash::Entry *e = object->internalClass()->findEntry(id);
    if (e) {
        changeMember(object, id, data, entry);
        return;
    }

    Heap::InternalClass *newClass = object->internalClass()->addMemberImpl(id, data, entry);
    object->setInternalClass(newClass);
}

Heap::InternalClass *InternalClass::addMember(PropertyKey identifier, PropertyAttributes data, InternalClassEntry *entry)
{
    Q_ASSERT(identifier.isStringOrSymbol());
    if (!data.isEmpty())
        data.resolve();

    PropertyHash::Entry *e = findEntry(identifier);
    if (e)
        return changeMember(identifier, data, entry);

    return addMemberImpl(identifier, data, entry);
}

Heap::InternalClass *InternalClass::addMemberImpl(PropertyKey identifier, PropertyAttributes data, InternalClassEntry *entry)
{
    Transition temp = { { identifier }, nullptr, (int)data.flags() };
    Transition &t = lookupOrInsertTransition(temp);

    if (entry) {
        entry->index = size;
        entry->setterIndex = data.isAccessor() ? size + 1 : UINT_MAX;
        entry->attributes = data;
    }

    if (t.lookup)
        return t.lookup;

    // create a new class and add it to the tree
    Scope scope(engine);
    Scoped<QV4::InternalClass> ic(scope, engine->newClass(this));
    InternalClass *newClass = ic->d();
    PropertyHash::Entry e = { identifier, newClass->size, data.isAccessor() ? newClass->size + 1 : UINT_MAX };
    newClass->propertyTable.addEntry(e, newClass->size);

    newClass->nameMap.add(newClass->size, identifier);
    newClass->propertyData.add(newClass->size, data);
    ++newClass->size;
    if (data.isAccessor())
        addDummyEntry(newClass, e);

    t.lookup = newClass;
    Q_ASSERT(t.lookup);
    return newClass;
}

void InternalClass::removeChildEntry(InternalClass *child)
{
    Q_ASSERT(engine);
    for (auto &t : transitions) {
        if (t.lookup == child) {
            t.lookup = nullptr;
            return;
        }
    }
    Q_UNREACHABLE();

}

void InternalClass::removeMember(QV4::Object *object, PropertyKey identifier)
{
#ifndef QT_NO_DEBUG
    Heap::InternalClass *oldClass = object->internalClass();
    Q_ASSERT(oldClass->findEntry(identifier) != nullptr);
#endif

    changeMember(object, identifier, Attr_Invalid);

#ifndef QT_NO_DEBUG
    // we didn't remove the data slot, just made it inaccessible
    Q_ASSERT(object->internalClass()->size == oldClass->size);
#endif
}

Heap::InternalClass *InternalClass::sealed()
{
    if (isSealed)
        return this;

    bool alreadySealed = !extensible;
    for (uint i = 0; i < size; ++i) {
        PropertyAttributes attrs = propertyData.at(i);
        if (attrs.isEmpty())
            continue;
        if (attrs.isConfigurable()) {
            alreadySealed = false;
            break;
        }
    }

    if (alreadySealed) {
        isSealed = true;
        return this;
    }

    Transition temp = { { PropertyKey::invalid() }, nullptr, InternalClassTransition::Sealed };
    Transition &t = lookupOrInsertTransition(temp);

    if (t.lookup) {
        Q_ASSERT(t.lookup && t.lookup->isSealed);
        return t.lookup;
    }

    Scope scope(engine);
    Scoped<QV4::InternalClass> ic(scope, engine->newClass(this));
    Heap::InternalClass *s = ic->d();

    for (uint i = 0; i < size; ++i) {
        PropertyAttributes attrs = propertyData.at(i);
        if (attrs.isEmpty())
            continue;
        attrs.setConfigurable(false);
        s->propertyData.set(i, attrs);
    }
    s->extensible = false;
    s->isSealed = true;

    t.lookup = s;
    return s;
}

Heap::InternalClass *InternalClass::frozen()
{
    if (isFrozen)
        return this;

    bool alreadyFrozen = !extensible;
    for (uint i = 0; i < size; ++i) {
        PropertyAttributes attrs = propertyData.at(i);
        if (attrs.isEmpty())
            continue;
        if ((attrs.isData() && attrs.isWritable()) || attrs.isConfigurable()) {
            alreadyFrozen = false;
            break;
        }
    }

    if (alreadyFrozen) {
        isSealed = true;
        isFrozen = true;
        return this;
    }

    Transition temp = { { PropertyKey::invalid() }, nullptr, InternalClassTransition::Frozen };
    Transition &t = lookupOrInsertTransition(temp);

    if (t.lookup) {
        Q_ASSERT(t.lookup && t.lookup->isSealed && t.lookup->isFrozen);
        return t.lookup;
    }

    Scope scope(engine);
    Scoped<QV4::InternalClass> ic(scope, engine->newClass(this));
    Heap::InternalClass *f = ic->d();

    for (uint i = 0; i < size; ++i) {
        PropertyAttributes attrs = propertyData.at(i);
        if (attrs.isEmpty())
            continue;
        if (attrs.isData())
            attrs.setWritable(false);
        attrs.setConfigurable(false);
        f->propertyData.set(i, attrs);
    }
    f->extensible = false;
    f->isSealed = true;
    f->isFrozen = true;

    t.lookup = f;
    return f;
}

Heap::InternalClass *InternalClass::propertiesFrozen()
{
    Scope scope(engine);
    Scoped<QV4::InternalClass> frozen(scope, this);
    for (uint i = 0; i < size; ++i) {
        PropertyAttributes attrs = propertyData.at(i);
        if (!nameMap.at(i).isValid())
            continue;
        if (!attrs.isEmpty()) {
            attrs.setWritable(false);
            attrs.setConfigurable(false);
        }
        frozen = frozen->changeMember(nameMap.at(i), attrs);
    }
    return frozen->d();
}

Heap::InternalClass *InternalClass::asProtoClass()
{
    if (isUsedAsProto)
        return this;

    Transition temp = { { PropertyKey::invalid() }, nullptr, Transition::ProtoClass };
    Transition &t = lookupOrInsertTransition(temp);
    if (t.lookup)
        return t.lookup;

    Heap::InternalClass *newClass = engine->newClass(this);
    newClass->isUsedAsProto = true;

    t.lookup = newClass;
    Q_ASSERT(t.lookup);
    return newClass;
}

static void updateProtoUsage(Heap::Object *o, Heap::InternalClass *ic)
{
    if (ic->prototype == o)
        ic->protoId = ic->engine->newProtoId();
    for (auto &t : ic->transitions) {
        if (t.lookup)
            updateProtoUsage(o, t.lookup);
    }
}


void InternalClass::updateProtoUsage(Heap::Object *o)
{
    Q_ASSERT(isUsedAsProto);
    Heap::InternalClass *ic = engine->internalClasses(EngineBase::Class_Empty);
    Q_ASSERT(!ic->prototype);

    Heap::updateProtoUsage(o, ic);
}

void InternalClass::markObjects(Heap::Base *b, MarkStack *stack)
{
    Heap::InternalClass *ic = static_cast<Heap::InternalClass *>(b);
    if (ic->prototype)
        ic->prototype->mark(stack);

    ic->nameMap.mark(stack);
}

}

}

QT_END_NAMESPACE
