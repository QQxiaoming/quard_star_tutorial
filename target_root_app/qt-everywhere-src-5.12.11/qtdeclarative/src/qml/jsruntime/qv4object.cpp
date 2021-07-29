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

#include "qv4object_p.h"
#include "qv4objectproto_p.h"
#include "qv4stringobject_p.h"
#include "qv4argumentsobject_p.h"
#include <private/qv4mm_p.h>
#include "qv4lookup_p.h"
#include "qv4scopedvalue_p.h"
#include "qv4memberdata_p.h"
#include "qv4objectiterator_p.h"
#include "qv4identifier_p.h"
#include "qv4string_p.h"
#include "qv4identifiertable_p.h"
#include "qv4jscall_p.h"
#include "qv4symbol_p.h"
#include "qv4proxy_p.h"

#include <stdint.h>

using namespace QV4;

DEFINE_OBJECT_VTABLE(Object);

void Object::setInternalClass(Heap::InternalClass *ic)
{
    d()->internalClass.set(engine(), ic);
    if (ic->isUsedAsProto)
        ic->updateProtoUsage(d());
    Q_ASSERT(ic && ic->vtable);
    uint nInline = d()->vtable()->nInlineProperties;
    if (ic->size <= nInline)
        return;
    bool hasMD = d()->memberData != nullptr;
    uint requiredSize = ic->size - nInline;
    if (!(hasMD && requiredSize) || (hasMD && d()->memberData->values.size < requiredSize))
        d()->memberData.set(ic->engine, MemberData::allocate(ic->engine, requiredSize, d()->memberData));
}

void Object::getProperty(const InternalClassEntry &entry, Property *p) const
{
    p->value = *propertyData(entry.index);
    if (entry.attributes.isAccessor())
        p->set = *propertyData(entry.setterIndex);
}

void Object::setProperty(const InternalClassEntry &entry, const Property *p)
{
    setProperty(entry.index, p->value);
    if (entry.attributes.isAccessor())
        setProperty(entry.setterIndex, p->set);
}

void Heap::Object::setUsedAsProto()
{
    internalClass.set(internalClass->engine, internalClass->asProtoClass());
}

ReturnedValue Object::getValueAccessor(const Value &thisObject, const Value &v, PropertyAttributes attrs)
{
    if (!attrs.isAccessor())
        return v.asReturnedValue();
    const QV4::FunctionObject *f = v.as<FunctionObject>();
    if (!f)
        return Encode::undefined();

    Scope scope(f->engine());
    JSCallData jsCallData(scope);
    *jsCallData->thisObject = thisObject;
    return checkedResult(scope.engine, f->call(jsCallData));
}

bool Object::putValue(uint memberIndex, PropertyAttributes attrs, const Value &value)
{
    Heap::InternalClass *ic = internalClass();
    if (ic->engine->hasException)
        return false;

    if (attrs.isAccessor()) {
        const FunctionObject *set = propertyData(memberIndex)->as<FunctionObject>();
        if (set) {
            Scope scope(ic->engine);
            ScopedFunctionObject setter(scope, set);
            JSCallData jsCallData(scope, 1);
            jsCallData->args[0] = value;
            *jsCallData->thisObject = this;
            setter->call(jsCallData);
            return !ic->engine->hasException;
        }
        return false;
    }

    if (!attrs.isWritable())
        return false;

    setProperty(memberIndex, value);
    return true;
}

void Object::defineDefaultProperty(const QString &name, const Value &value, PropertyAttributes attributes)
{
    ExecutionEngine *e = engine();
    Scope scope(e);
    ScopedString s(scope, e->newIdentifier(name));
    defineDefaultProperty(s, value, attributes);
}

void Object::defineDefaultProperty(const QString &name, VTable::Call code,
                                   int argumentCount, PropertyAttributes attributes)
{
    ExecutionEngine *e = engine();
    Scope scope(e);
    ScopedString s(scope, e->newIdentifier(name));
    ScopedFunctionObject function(scope, FunctionObject::createBuiltinFunction(e, s, code, argumentCount));
    defineDefaultProperty(s, function, attributes);
}

void Object::defineDefaultProperty(StringOrSymbol *nameOrSymbol, VTable::Call code,
                                   int argumentCount, PropertyAttributes attributes)
{
    ExecutionEngine *e = engine();
    Scope scope(e);
    ScopedFunctionObject function(scope, FunctionObject::createBuiltinFunction(e, nameOrSymbol, code, argumentCount));
    defineDefaultProperty(nameOrSymbol, function, attributes);
}

void Object::defineAccessorProperty(const QString &name, VTable::Call getter, VTable::Call setter)
{
    ExecutionEngine *e = engine();
    Scope scope(e);
    ScopedString s(scope, e->newIdentifier(name));
    defineAccessorProperty(s, getter, setter);
}

void Object::defineAccessorProperty(StringOrSymbol *name, VTable::Call getter, VTable::Call setter)
{
    ExecutionEngine *v4 = engine();
    QV4::Scope scope(v4);
    ScopedProperty p(scope);
    QString n = name->toQString();
    if (n.at(0) == QLatin1Char('@'))
        n = QChar::fromLatin1('[') + n.midRef(1) + QChar::fromLatin1(']');
    if (getter) {
        ScopedString getName(scope, v4->newString(QString::fromLatin1("get ") + n));
        p->setGetter(ScopedFunctionObject(scope, FunctionObject::createBuiltinFunction(v4, getName, getter, 0)));
    } else {
        p->setGetter(nullptr);
    }
    if (setter) {
        ScopedString setName(scope, v4->newString(QString::fromLatin1("set ") + n));
        p->setSetter(ScopedFunctionObject(scope, FunctionObject::createBuiltinFunction(v4, setName, setter, 0)));
    } else {
        p->setSetter(nullptr);
    }
    insertMember(name, p, QV4::Attr_Accessor|QV4::Attr_NotEnumerable);
}



void Object::defineReadonlyProperty(const QString &name, const Value &value)
{
    QV4::ExecutionEngine *e = engine();
    Scope scope(e);
    ScopedString s(scope, e->newIdentifier(name));
    defineReadonlyProperty(s, value);
}

void Object::defineReadonlyProperty(String *name, const Value &value)
{
    insertMember(name, value, Attr_ReadOnly);
}

void Object::defineReadonlyConfigurableProperty(const QString &name, const Value &value)
{
    QV4::ExecutionEngine *e = engine();
    Scope scope(e);
    ScopedString s(scope, e->newIdentifier(name));
    defineReadonlyConfigurableProperty(s, value);
}

void Object::defineReadonlyConfigurableProperty(StringOrSymbol *name, const Value &value)
{
    insertMember(name, value, Attr_ReadOnly_ButConfigurable);
}

void Object::addSymbolSpecies()
{
    Scope scope(engine());
    ScopedProperty p(scope);
    p->setGetter(scope.engine->getSymbolSpecies());
    p->setSetter(nullptr);
    insertMember(scope.engine->symbol_species(), p, QV4::Attr_Accessor|QV4::Attr_NotWritable|QV4::Attr_NotEnumerable);
}

void Heap::Object::markObjects(Heap::Base *b, MarkStack *stack)
{
    Base::markObjects(b, stack);
    Object *o = static_cast<Object *>(b);
    if (o->memberData)
        o->memberData->mark(stack);
    if (o->arrayData)
        o->arrayData->mark(stack);
    uint nInline = o->vtable()->nInlineProperties;
    Value *v = reinterpret_cast<Value *>(o) + o->vtable()->inlinePropertyOffset;
    const Value *end = v + nInline;
    while (v < end) {
        v->mark(stack);
        ++v;
    }
}

void Object::insertMember(StringOrSymbol *s, const Property *p, PropertyAttributes attributes)
{
    InternalClassEntry idx;
    PropertyKey key = s->toPropertyKey();
    Heap::InternalClass::addMember(this, key, attributes, &idx);

    setProperty(idx.index, p->value);
    if (attributes.isAccessor())
        setProperty(idx.setterIndex, p->set);
}

void Object::setPrototypeUnchecked(const Object *p)
{
    setInternalClass(internalClass()->changePrototype(p ? p->d() : nullptr));
}

// Section 8.12.2
PropertyIndex Object::getValueOrSetter(PropertyKey id, PropertyAttributes *attrs)
{
    if (id.isArrayIndex()) {
        uint index = id.asArrayIndex();
        Heap::Object *o = d();
        while (o) {
            if (o->arrayData) {
                uint idx = o->arrayData->mappedIndex(index);
                if (idx != UINT_MAX) {
                    *attrs = o->arrayData->attributes(index);
                    return { o->arrayData , o->arrayData->values.values + (attrs->isAccessor() ? idx + SetterOffset : idx) };
                }
            }
            if (o->vtable()->type == Type_StringObject) {
                if (index < static_cast<const Heap::StringObject *>(o)->length()) {
                    // this is an evil hack, but it works, as the method is only ever called from put,
                    // where we don't use the returned pointer there for non writable attributes
                    *attrs = (Attr_NotWritable|Attr_NotConfigurable);
                    return { reinterpret_cast<Heap::ArrayData *>(0x1), nullptr };
                }
            }
            o = o->prototype();
        }
    } else {
        Heap::Object *o = d();
        while (o) {
            auto idx = o->internalClass->findValueOrSetter(id);
            if (idx.isValid()) {
                *attrs = idx.attrs;
                return o->writablePropertyData(idx.index);
            }

            o = o->prototype();
        }
    }
    *attrs = Attr_Invalid;
    return { nullptr, nullptr };
}

ReturnedValue Object::virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty)
{
    return static_cast<const Object *>(m)->internalGet(id, receiver, hasProperty);
}

bool Object::virtualPut(Managed *m, PropertyKey id, const Value &value, Value *receiver)
{
    return static_cast<Object *>(m)->internalPut(id, value, receiver);
}

bool Object::virtualDeleteProperty(Managed *m, PropertyKey id)
{
    return static_cast<Object *>(m)->internalDeleteProperty(id);
}

PropertyKey ObjectOwnPropertyKeyIterator::next(const Object *o, Property *pd, PropertyAttributes *attrs)
{
    if (arrayIndex != UINT_MAX && o->arrayData()) {
        SparseArrayNode *arrayNode = nullptr;
        if (o->arrayType() == Heap::ArrayData::Sparse) {
            SparseArray *sparse = o->arrayData()->sparse;
            arrayNode = arrayIndex ? sparse->lowerBound(arrayIndex) : sparse->begin();
        }

        // sparse arrays
        if (arrayNode) {
            while (arrayNode != o->sparseEnd()) {
                uint k = arrayNode->key();
                uint pidx = arrayNode->value;
                Heap::SparseArrayData *sa = o->d()->arrayData.cast<Heap::SparseArrayData>();
                const Property *p = reinterpret_cast<const Property *>(sa->values.data() + pidx);
                arrayNode = arrayNode->nextNode();
                PropertyAttributes a = sa->attrs ? sa->attrs[pidx] : Attr_Data;
                arrayIndex = k + 1;
                if (pd)
                    pd->copy(p, a);
                if (attrs)
                    *attrs = a;
                return PropertyKey::fromArrayIndex(k);
            }
            arrayIndex = UINT_MAX;
        }
        // dense arrays
        while (arrayIndex < o->d()->arrayData->values.size) {
            Heap::SimpleArrayData *sa = o->d()->arrayData.cast<Heap::SimpleArrayData>();
            const Value &val = sa->data(arrayIndex);
            PropertyAttributes a = o->arrayData()->attributes(arrayIndex);
            int index = arrayIndex;
            ++arrayIndex;
            if (!val.isEmpty()) {
                if (pd)
                    pd->value = val;
                if (attrs)
                    *attrs = a;
                return PropertyKey::fromArrayIndex(index);
            }
        }
        arrayIndex = UINT_MAX;
    }

    while (true) {
        while (memberIndex < o->internalClass()->size) {
            PropertyKey n = o->internalClass()->nameMap.at(memberIndex);
            ++memberIndex;
            if (!n.isStringOrSymbol())
                // accessor properties have a dummy entry with n == 0
                continue;
            if (!iterateOverSymbols && n.isSymbol())
                continue;
            if (iterateOverSymbols && !n.isSymbol())
                continue;

            InternalClassEntry e = o->internalClass()->find(n);
            if (!e.isValid())
                continue;
            if (pd) {
                pd->value = *o->propertyData(e.index);
                if (e.attributes.isAccessor())
                    pd->set = *o->propertyData(e.setterIndex);
            }
            if (attrs)
                *attrs = e.attributes;
            return n;
        }
        if (iterateOverSymbols)
            break;
        iterateOverSymbols = true;
        memberIndex = 0;
    }

    return PropertyKey::invalid();
}

OwnPropertyKeyIterator *Object::virtualOwnPropertyKeys(const Object *o, Value *target)
{
    *target = *o;
    return new ObjectOwnPropertyKeyIterator;
}

// Section 8.12.3
ReturnedValue Object::internalGet(PropertyKey id, const Value *receiver, bool *hasProperty) const
{
    Heap::Object *o = d();

    uint index = id.asArrayIndex();
    if (index != UINT_MAX) {
        Scope scope(this);
        PropertyAttributes attrs;
        ScopedProperty pd(scope);
        while (1) {
            if (o->arrayData && o->arrayData->getProperty(index, pd, &attrs)) {
                if (hasProperty)
                    *hasProperty = true;
                return Object::getValue(*receiver, pd->value, attrs);
            }
            if (o->internalClass->vtable->type == Type_StringObject) {
                ScopedString str(scope, static_cast<Heap::StringObject *>(o)->getIndex(index));
                if (str) {
                    attrs = (Attr_NotWritable|Attr_NotConfigurable);
                    if (hasProperty)
                        *hasProperty = true;
                    return str.asReturnedValue();
                }
            }
            o = o->prototype();
            if (!o || o->internalClass->vtable->get != Object::virtualGet)
                break;
        }
    } else {
        Q_ASSERT(!id.isArrayIndex());

        while (1) {
            auto idx = o->internalClass->findValueOrGetter(id);
            if (idx.isValid()) {
                if (hasProperty)
                    *hasProperty = true;
                return Object::getValue(*receiver, *o->propertyData(idx.index), idx.attrs);
            }
            o = o->prototype();
            if (!o || o->internalClass->vtable->get != Object::virtualGet)
                break;
        }
    }

    if (o) {
        const Value v = Value::fromHeapObject(o);
        const Object &obj = static_cast<const Object &>(v);
        return obj.get(id, receiver, hasProperty);
    }

    if (hasProperty)
        *hasProperty = false;
    return Encode::undefined();
}

// Section 8.12.5
bool Object::internalPut(PropertyKey id, const Value &value, Value *receiver)
{
    Scope scope(this);
    if (scope.engine->hasException)
        return false;

    Object *r = receiver->objectValue();
    if (r && r->d() == d()) {
        // receiver and this object are the same
        if (d()->internalClass->vtable->getOwnProperty == Object::virtualGetOwnProperty) {
            // This object standard methods in the vtable, so we can take a shortcut
            // and avoid the calls to getOwnProperty and defineOwnProperty
            uint index = id.asArrayIndex();

            PropertyAttributes attrs;
            PropertyIndex propertyIndex{nullptr, nullptr};

            if (index != UINT_MAX) {
                if (arrayData())
                    propertyIndex = arrayData()->getValueOrSetter(index, &attrs);
            } else {
                auto member = internalClass()->findValueOrSetter(id);
                if (member.isValid()) {
                    attrs = member.attrs;
                    propertyIndex = d()->writablePropertyData(member.index);
                }
            }

            if (!propertyIndex.isNull() && !attrs.isAccessor()) {
                if (!attrs.isWritable())
                    return false;
                else if (isArrayObject() && id == scope.engine->id_length()->propertyKey()) {
                    bool ok;
                    uint l = value.asArrayLength(&ok);
                    if (!ok) {
                        scope.engine->throwRangeError(value);
                        return false;
                    }
                    ok = setArrayLength(l);
                    if (!ok)
                        return false;
                } else {
                    propertyIndex.set(scope.engine, value);
                }
                return true;
            }
        }
    }

    ScopedProperty p(scope);
    PropertyAttributes attrs;
    attrs = getOwnProperty(id, p);
    if (attrs == Attr_Invalid) {
        ScopedObject p(scope, getPrototypeOf());
        if (p)
            return p->put(id, value, receiver);
        attrs = Attr_Data;
    }

    if (attrs.isAccessor()) {
        ScopedFunctionObject setter(scope, p->setter());
        if (!setter)
            return false;
        JSCallData jsCallData(scope, 1);
        jsCallData->args[0] = value;
        *jsCallData->thisObject = *receiver;
        setter->call(jsCallData);
        return !scope.engine->hasException;
    }

    // Data property
    if (!attrs.isWritable())
        return false;
    if (!r)
        return false;
    attrs = r->getOwnProperty(id, p);

    if (attrs != Attr_Invalid) {
        if (attrs.isAccessor() || !attrs.isWritable())
            return false;
    } else {
        if (!r->isExtensible())
            return false;
        attrs = Attr_Data;
    }

    if (r->internalClass()->vtable->defineOwnProperty == virtualDefineOwnProperty) {
        // standard object, we can avoid some more checks
        uint index = id.asArrayIndex();
        if (index == UINT_MAX) {
            ScopedStringOrSymbol s(scope, id.asStringOrSymbol());
            r->insertMember(s, value);
        } else {
            r->arraySet(index, value);
        }
        return true;
    }

    p->value = value;
    return r->defineOwnProperty(id, p, attrs);
}

// Section 8.12.7
bool Object::internalDeleteProperty(PropertyKey id)
{
    if (internalClass()->engine->hasException)
        return false;

    if (id.isArrayIndex()) {
        uint index = id.asArrayIndex();
        Scope scope(engine());
        if (scope.engine->hasException)
            return false;

        Scoped<ArrayData> ad(scope, arrayData());
        if (!ad || ad->vtable()->del(this, index))
            return true;

        return false;
    }

    auto memberIdx = internalClass()->findValueOrGetter(id);
    if (memberIdx.isValid()) {
        if (memberIdx.attrs.isConfigurable()) {
            Heap::InternalClass::removeMember(this, id);
            return true;
        }
        return false;
    }

    return true;
}

bool Object::internalDefineOwnProperty(ExecutionEngine *engine, uint index, const InternalClassEntry *memberEntry, const Property *p, PropertyAttributes attrs)
{
    // clause 5
    if (attrs.isEmpty())
        return true;

    Scope scope(engine);
    ScopedProperty current(scope);
    PropertyAttributes cattrs;
    if (memberEntry) {
        getProperty(*memberEntry, current);
        cattrs = memberEntry->attributes;
    } else if (arrayData()) {
        arrayData()->getProperty(index, current, &cattrs);
        cattrs = arrayData()->attributes(index);
    }

    // clause 6
    if (p->isSubset(attrs, current, cattrs))
        return true;

    // clause 7
    if (!cattrs.isConfigurable()) {
        if (attrs.isConfigurable())
            return false;
        if (attrs.hasEnumerable() && attrs.isEnumerable() != cattrs.isEnumerable())
            return false;
    }

    // clause 8
    if (attrs.isGeneric() || current->value.isEmpty())
        goto accept;

    // clause 9
    if (cattrs.isData() != attrs.isData()) {
        // 9a
        if (!cattrs.isConfigurable())
            return false;
        if (cattrs.isData()) {
            // 9b
            cattrs.setType(PropertyAttributes::Accessor);
            cattrs.clearWritable();
            if (!memberEntry) {
                // need to convert the array and the slot
                initSparseArray();
                Q_ASSERT(arrayData());
                setArrayAttributes(index, cattrs);
            }
            current->setGetter(nullptr);
            current->setSetter(nullptr);
        } else {
            // 9c
            cattrs.setType(PropertyAttributes::Data);
            cattrs.setWritable(false);
            if (!memberEntry) {
                // need to convert the array and the slot
                setArrayAttributes(index, cattrs);
            }
            current->value = Value::undefinedValue();
        }
    } else if (cattrs.isData() && attrs.isData()) { // clause 10
        if (!cattrs.isConfigurable() && !cattrs.isWritable()) {
            if (attrs.isWritable() || !current->value.sameValue(p->value))
                return false;
        }
    } else { // clause 10
        Q_ASSERT(cattrs.isAccessor() && attrs.isAccessor());
        if (!cattrs.isConfigurable()) {
            if (!p->value.isEmpty() && current->value.rawValue() != p->value.rawValue())
                return false;
            if (!p->set.isEmpty() && current->set.rawValue() != p->set.rawValue())
                return false;
        }
    }

  accept:

    current->merge(cattrs, p, attrs);
    if (memberEntry) {
        PropertyKey key = internalClass()->nameMap.at(memberEntry->index);
        InternalClassEntry e;
        Heap::InternalClass::changeMember(this, key, cattrs, &e);
        setProperty(e, current);
    } else {
        setArrayAttributes(index, cattrs);
        arrayData()->setProperty(scope.engine, index, current);
    }
    return true;
}

void Object::copyArrayData(Object *other)
{
    Q_ASSERT(isArrayObject());
    Scope scope(engine());

    if (other->protoHasArray() || ArgumentsObject::isNonStrictArgumentsObject(other) ||
        (other->arrayType() == Heap::ArrayData::Sparse && other->arrayData()->attrs)) {
        uint len = other->getLength();
        Q_ASSERT(len);

        ScopedValue v(scope);
        for (uint i = 0; i < len; ++i) {
            arraySet(i, (v = other->get(i)));
        }
    } else if (!other->arrayData()) {
        ;
    } else {
        Q_ASSERT(!arrayData() && other->arrayData());
        ArrayData::realloc(this, static_cast<ArrayData::Type>(other->d()->arrayData->type),
                           other->d()->arrayData->values.alloc, false);
        if (other->arrayType() == Heap::ArrayData::Sparse) {
            Heap::ArrayData *od = other->d()->arrayData;
            Heap::ArrayData *dd = d()->arrayData;
            dd->sparse = new SparseArray(*od->sparse);
        } else {
            Heap::ArrayData *dd = d()->arrayData;
            dd->values.size = other->d()->arrayData->values.size;
            dd->offset = other->d()->arrayData->offset;
        }
        // ### need a write barrier
        memcpy(d()->arrayData->values.values, other->d()->arrayData->values.values, other->d()->arrayData->values.alloc*sizeof(Value));
    }
    setArrayLengthUnchecked(other->getLength());
}

qint64 Object::virtualGetLength(const Managed *m)
{
    Scope scope(static_cast<const Object *>(m)->engine());
    ScopedValue v(scope, static_cast<Object *>(const_cast<Managed *>(m))->get(scope.engine->id_length()));
    return v->toLength();
}

// 'var' is 'V' in 15.3.5.3.
ReturnedValue Object::virtualInstanceOf(const Object *typeObject, const Value &var)
{
    QV4::ExecutionEngine *engine = typeObject->internalClass()->engine;

    // 15.3.5.3, Assume F is a Function object.
    const FunctionObject *function = typeObject->as<FunctionObject>();
    if (!function)
        return engine->throwTypeError();

    return checkedInstanceOf(engine, function, var);
}

ReturnedValue Object::virtualResolveLookupGetter(const Object *object, ExecutionEngine *engine, Lookup *lookup)
{
    Heap::Object *obj = object->d();
    PropertyKey name = engine->identifierTable->asPropertyKey(engine->currentStackFrame->v4Function->compilationUnit->runtimeStrings[lookup->nameIndex]);
    if (name.isArrayIndex()) {
        lookup->indexedLookup.index = name.asArrayIndex();
        lookup->getter = Lookup::getterIndexed;
        return lookup->getter(lookup, engine, *object);
    }

    auto index = obj->internalClass->findValueOrGetter(name);
    if (index.isValid()) {
        PropertyAttributes attrs = index.attrs;
        uint nInline = obj->vtable()->nInlineProperties;
        if (attrs.isData()) {
            if (index.index < obj->vtable()->nInlineProperties) {
                index.index += obj->vtable()->inlinePropertyOffset;
                lookup->getter = Lookup::getter0Inline;
            } else {
                index.index -= nInline;
                lookup->getter = Lookup::getter0MemberData;
            }
        } else {
            lookup->getter = Lookup::getterAccessor;
        }
        lookup->objectLookup.ic = obj->internalClass;
        lookup->objectLookup.offset = index.index;
        return lookup->getter(lookup, engine, *object);
    }

    lookup->protoLookup.protoId = obj->internalClass->protoId;
    lookup->resolveProtoGetter(name, obj->prototype());
    return lookup->getter(lookup, engine, *object);
}

bool Object::virtualResolveLookupSetter(Object *object, ExecutionEngine *engine, Lookup *lookup, const Value &value)
{
    Scope scope(engine);
    ScopedString name(scope, scope.engine->currentStackFrame->v4Function->compilationUnit->runtimeStrings[lookup->nameIndex]);

    Heap::InternalClass *c = object->internalClass();
    PropertyKey key = name->toPropertyKey();
    auto idx = c->findValueOrSetter(key);
    if (idx.isValid()) {
        if (object->isArrayObject() && idx.index == Heap::ArrayObject::LengthPropertyIndex) {
            Q_ASSERT(!idx.attrs.isAccessor());
            lookup->setter = Lookup::arrayLengthSetter;
            return lookup->setter(lookup, engine, *object, value);
        } else if (idx.attrs.isData() && idx.attrs.isWritable()) {
            lookup->objectLookup.ic = object->internalClass();
            lookup->objectLookup.offset = idx.index;
            lookup->setter = idx.index < object->d()->vtable()->nInlineProperties ? Lookup::setter0Inline : Lookup::setter0;
            return lookup->setter(lookup, engine, *object, value);
        } else {
            // ### handle setter
            lookup->setter = Lookup::setterFallback;
        }
        return lookup->setter(lookup, engine, *object, value);
    }

    lookup->insertionLookup.protoId = c->protoId;
    if (!object->put(key, value)) {
        lookup->setter = Lookup::setterFallback;
        return false;
    }

    if (object->internalClass() == c) {
        // ### setter in the prototype, should handle this
        lookup->setter = Lookup::setterFallback;
        return true;
    }
    idx = object->internalClass()->findValueOrSetter(key);
    if (!idx.isValid() || idx.attrs.isAccessor()) { // ### can this even happen?
        lookup->setter = Lookup::setterFallback;
        return false;
    }
    lookup->insertionLookup.newClass = object->internalClass();
    lookup->insertionLookup.offset = idx.index;
    lookup->setter = Lookup::setterInsert;
    return true;
}

ReturnedValue Object::checkedInstanceOf(ExecutionEngine *engine, const FunctionObject *f, const Value &var)
{
    Scope scope(engine);
    if (f->isBoundFunction()) {
        ScopedValue v(scope, static_cast<const BoundFunction *>(f)->target());
        f = v->as<FunctionObject>();
    }

    // 15.3.5.3, 1: HasInstance can only be used on an object
    const Object *lhs = var.as<Object>();
    if (!lhs)
        return Encode(false);

    // 15.3.5.3, 2
    Value p = Value::fromReturnedValue(f->protoProperty());
    const Object *o = p.objectValue();
    if (!o) // 15.3.5.3, 3
        return f->engine()->throwTypeError();

    Heap::Object *v = lhs->d();

    // 15.3.5.3, 4
    while (v) {
        // 15.3.5.3, 4, a
        v = v->prototype();

        // 15.3.5.3, 4, b
        if (!v)
            break; // will return false

        // 15.3.5.3, 4, c
        else if (o->d() == v)
            return Encode(true);
    }

    return Encode(false);
}

bool Object::virtualHasProperty(const Managed *m, PropertyKey id)
{
    Scope scope(m->engine());
    ScopedObject o(scope, m);
    ScopedProperty p(scope);

    if (o->getOwnProperty(id, p) != Attr_Invalid)
        return true;

    o = o->getPrototypeOf();
    if (o)
        return o->hasProperty(id);

    return false;
}

PropertyAttributes Object::virtualGetOwnProperty(const Managed *m, PropertyKey id, Property *p)
{
    PropertyAttributes attrs;
    const Object *o = static_cast<const Object *>(m);
    if (id.isArrayIndex()) {
        uint index = id.asArrayIndex();
        if (o->arrayData()) {
            if (o->arrayData()->getProperty(index, p, &attrs))
                return attrs;
        }
    } else {
        Q_ASSERT(id.asStringOrSymbol());

        auto member = o->internalClass()->find(id);
        if (member.isValid()) {
            attrs = member.attributes;
            if (p) {
                p->value = *o->propertyData(member.index);
                if (attrs.isAccessor())
                    p->set = *o->propertyData(member.setterIndex);
            }
            return attrs;
        }
    }

    return Attr_Invalid;
}

bool Object::virtualDefineOwnProperty(Managed *m, PropertyKey id, const Property *p, PropertyAttributes attrs)
{
    Object *o = static_cast<Object *>(m);
    Scope scope(o);

    if (id.isArrayIndex()) {
        uint index = id.asArrayIndex();

        bool hasProperty = false;

        if (o->arrayData()) {
            hasProperty = o->arrayData()->mappedIndex(index) != UINT_MAX;
            if (!hasProperty && o->isStringObject())
                hasProperty = (index < static_cast<StringObject *>(o)->length());
        }

        if (!hasProperty) {
            if (!o->isExtensible())
                return false;

            ScopedProperty pp(scope);
            pp->copy(p, attrs);
            pp->fullyPopulated(&attrs);
            if (attrs == Attr_Data) {
                ScopedValue v(scope, pp->value);
                o->arraySet(index, v);
            } else {
                o->arraySet(index, pp, attrs);
            }
            return true;
        }

        return o->internalDefineOwnProperty(scope.engine, index, nullptr, p, attrs);
    }

    auto memberIndex = o->internalClass()->find(id);

    if (!memberIndex.isValid()) {
        if (!o->isExtensible())
            return false;

        Scoped<StringOrSymbol> name(scope, id.asStringOrSymbol());
        ScopedProperty pd(scope);
        pd->copy(p, attrs);
        pd->fullyPopulated(&attrs);
        o->insertMember(name, pd, attrs);
        return true;
    }

    return o->internalDefineOwnProperty(scope.engine, UINT_MAX, &memberIndex, p, attrs);
}

bool Object::virtualIsExtensible(const Managed *m)
{
    return m->d()->internalClass->extensible;
}

bool Object::virtualPreventExtensions(Managed *m)
{
    Q_ASSERT(m->isObject());
    Object *o = static_cast<Object *>(m);
    o->setInternalClass(o->internalClass()->nonExtensible());
    return true;
}

Heap::Object *Object::virtualGetPrototypeOf(const Managed *m)
{
    return m->internalClass()->prototype;
}

bool Object::virtualSetPrototypeOf(Managed *m, const Object *proto)
{
    Q_ASSERT(m->isObject());
    Object *o = static_cast<Object *>(m);
    Heap::Object *current = o->internalClass()->prototype;
    Heap::Object *protod = proto ? proto->d() : nullptr;
    if (current == protod)
        return true;
    if (!o->internalClass()->extensible)
        return false;
    Heap::Object *p = protod;
    while (p) {
        if (p == o->d())
            return false;
        if (p->vtable()->getPrototypeOf != Object::staticVTable()->getPrototypeOf)
            break;
        p = p->prototype();
    }
    o->setInternalClass(o->internalClass()->changePrototype(protod));
    return true;
}

bool Object::setArrayLength(uint newLen)
{
    Q_ASSERT(isArrayObject());
    if (!internalClass()->propertyData[Heap::ArrayObject::LengthPropertyIndex].isWritable())
        return false;
    uint oldLen = getLength();
    bool ok = true;
    if (newLen < oldLen) {
        if (arrayData()) {
            uint l = arrayData()->vtable()->truncate(this, newLen);
            if (l != newLen)
                ok = false;
            newLen = l;
        }
    } else {
        if (newLen >= 0x100000)
            initSparseArray();
    }
    setArrayLengthUnchecked(newLen);
    return ok;
}

void Object::initSparseArray()
{
    if (arrayType() == Heap::ArrayData::Sparse)
        return;

    ArrayData::realloc(this, Heap::ArrayData::Sparse, 0, false);
}

bool Object::isConcatSpreadable() const
{
    Scope scope(this);
    ScopedValue spreadable(scope, get(scope.engine->symbol_isConcatSpreadable()));
    if (!spreadable->isUndefined())
        return spreadable->toBoolean();
    return isArray();
}

bool Object::isArray() const
{
    if (isArrayObject())
        return true;
    if (vtable() == ProxyObject::staticVTable()) {
        const ProxyObject *p = static_cast<const ProxyObject *>(this);
        Scope scope(this);
        if (!p->d()->handler) {
            scope.engine->throwTypeError();
            return false;
        }
        ScopedObject o(scope, p->d()->target);
        return o->isArray();
    }
    return false;
}

const FunctionObject *Object::speciesConstructor(Scope &scope, const FunctionObject *defaultConstructor) const
{
    ScopedValue C(scope, get(scope.engine->id_constructor()));
    if (C->isUndefined())
        return defaultConstructor;
    const Object *c = C->objectValue();
    if (!c) {
        scope.engine->throwTypeError();
        return nullptr;
    }
    ScopedValue S(scope, c->get(scope.engine->symbol_species()));
    if (S->isNullOrUndefined())
        return defaultConstructor;
    const FunctionObject *f = S->as<FunctionObject>();
    if (!f || !f->isConstructor()) {
        scope.engine->throwTypeError();
        return nullptr;
    }
    Q_ASSERT(f->isFunctionObject());
    return static_cast<const FunctionObject *>(f);
}

bool Object::setProtoFromNewTarget(const Value *newTarget)
{
    if (!newTarget || newTarget->isUndefined())
        return false;

    Q_ASSERT(newTarget->isFunctionObject());
    Scope scope(this);
    ScopedObject proto(scope, static_cast<const FunctionObject *>(newTarget)->protoProperty());
    if (proto) {
        setPrototypeOf(proto);
        return true;
    }
    return false;
}


DEFINE_OBJECT_VTABLE(ArrayObject);

void Heap::ArrayObject::init(const QStringList &list)
{
    Object::init();
    commonInit();
    Scope scope(internalClass->engine);
    ScopedObject a(scope, this);

    // Converts a QStringList to JS.
    // The result is a new Array object with length equal to the length
    // of the QStringList, and the elements being the QStringList's
    // elements converted to JS Strings.
    int len = list.count();
    a->arrayReserve(len);
    ScopedValue v(scope);
    for (int ii = 0; ii < len; ++ii)
        a->arrayPut(ii, (v = scope.engine->newString(list.at(ii))));
    a->setArrayLengthUnchecked(len);
}

qint64 ArrayObject::virtualGetLength(const Managed *m)
{
    const ArrayObject *a = static_cast<const ArrayObject *>(m);
    return a->propertyData(Heap::ArrayObject::LengthPropertyIndex)->toLength();
}

QStringList ArrayObject::toQStringList() const
{
    QStringList result;

    QV4::ExecutionEngine *engine = internalClass()->engine;
    Scope scope(engine);
    ScopedValue v(scope);

    uint length = getLength();
    for (uint i = 0; i < length; ++i) {
        v = const_cast<ArrayObject *>(this)->get(i);
        result.append(v->toQStringNoThrow());
    }
    return result;
}

bool ArrayObject::virtualDefineOwnProperty(Managed *m, PropertyKey id, const Property *p, PropertyAttributes attrs)
{
    Q_ASSERT(m->isArrayObject());
    ArrayObject *a = static_cast<ArrayObject *>(m);

    if (id.isArrayIndex()) {
        uint index = id.asArrayIndex();
        uint len = a->getLength();
        if (index >= len && !a->internalClass()->propertyData[Heap::ArrayObject::LengthPropertyIndex].isWritable())
            return false;

        bool succeeded = Object::virtualDefineOwnProperty(m, id, p, attrs);
        if (!succeeded)
            return false;

        if (index >= len)
            a->setArrayLengthUnchecked(index + 1);

        return true;
    }

    ExecutionEngine *engine = m->engine();
    if (id == engine->id_length()->propertyKey()) {
        Scope scope(engine);
        Q_ASSERT(a->internalClass()->verifyIndex(engine->id_length()->propertyKey(), Heap::ArrayObject::LengthPropertyIndex));
        ScopedProperty lp(scope);
        InternalClassEntry e = a->internalClass()->find(scope.engine->id_length()->propertyKey());
        a->getProperty(e, lp);
        if (attrs.isEmpty() || p->isSubset(attrs, lp, e.attributes))
            return true;
        if (!e.attributes.isWritable() || attrs.type() == PropertyAttributes::Accessor || attrs.isConfigurable() || attrs.isEnumerable())
            return false;
        bool succeeded = true;
        if (attrs.type() == PropertyAttributes::Data) {
            bool ok;
            uint l = p->value.asArrayLength(&ok);
            if (!ok) {
                ScopedValue v(scope, p->value);
                engine->throwRangeError(v);
                return false;
            }
            succeeded = a->setArrayLength(l);
        }
        if (attrs.hasWritable() && !attrs.isWritable()) {
            e.attributes.setWritable(false);
            Heap::InternalClass::changeMember(a, engine->id_length()->propertyKey(), e.attributes);
        }
        if (!succeeded)
            return false;
        return true;
    }
    return Object::virtualDefineOwnProperty(m, id, p, attrs);
}
