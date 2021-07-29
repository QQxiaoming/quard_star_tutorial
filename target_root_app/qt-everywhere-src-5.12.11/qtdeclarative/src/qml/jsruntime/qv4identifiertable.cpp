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
#include "qv4identifiertable_p.h"
#include "qv4symbol_p.h"

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


IdentifierTable::IdentifierTable(ExecutionEngine *engine, int numBits)
    : engine(engine)
    , size(0)
    , numBits(numBits)
{
    alloc = primeForNumBits(numBits);
    entriesByHash = (Heap::StringOrSymbol **)malloc(alloc*sizeof(Heap::StringOrSymbol *));
    entriesById = (Heap::StringOrSymbol **)malloc(alloc*sizeof(Heap::StringOrSymbol *));
    memset(entriesByHash, 0, alloc*sizeof(Heap::String *));
    memset(entriesById, 0, alloc*sizeof(Heap::String *));
}

IdentifierTable::~IdentifierTable()
{
    free(entriesByHash);
    free(entriesById);
    for (const auto &h : qAsConst(idHashes))
        h->identifierTable = nullptr;
}

void IdentifierTable::addEntry(Heap::StringOrSymbol *str)
{
    uint hash = str->hashValue();

    if (str->subtype == Heap::String::StringType_ArrayIndex)
        return;

    str->identifier = PropertyKey::fromStringOrSymbol(str);

    bool grow = (alloc <= size*2);

    if (grow) {
        ++numBits;
        int newAlloc = primeForNumBits(numBits);
        Heap::StringOrSymbol **newEntries = (Heap::StringOrSymbol **)malloc(newAlloc*sizeof(Heap::String *));
        memset(newEntries, 0, newAlloc*sizeof(Heap::StringOrSymbol *));
        for (uint i = 0; i < alloc; ++i) {
            Heap::StringOrSymbol *e = entriesByHash[i];
            if (!e)
                continue;
            uint idx = e->stringHash % newAlloc;
            while (newEntries[idx]) {
                ++idx;
                idx %= newAlloc;
            }
            newEntries[idx] = e;
        }
        free(entriesByHash);
        entriesByHash = newEntries;

        newEntries = (Heap::StringOrSymbol **)malloc(newAlloc*sizeof(Heap::String *));
        memset(newEntries, 0, newAlloc*sizeof(Heap::StringOrSymbol *));
        for (uint i = 0; i < alloc; ++i) {
            Heap::StringOrSymbol *e = entriesById[i];
            if (!e)
                continue;
            uint idx = e->identifier.id() % newAlloc;
            while (newEntries[idx]) {
                ++idx;
                idx %= newAlloc;
            }
            newEntries[idx] = e;
        }
        free(entriesById);
        entriesById = newEntries;

        alloc = newAlloc;
    }

    uint idx = hash % alloc;
    while (entriesByHash[idx]) {
        ++idx;
        idx %= alloc;
    }
    entriesByHash[idx] = str;

    idx = str->identifier.id() % alloc;
    while (entriesById[idx]) {
        ++idx;
        idx %= alloc;
    }
    entriesById[idx] = str;

    ++size;
}



Heap::String *IdentifierTable::insertString(const QString &s)
{
    uint subtype;
    uint hash = String::createHashValue(s.constData(), s.length(), &subtype);
    if (subtype == Heap::String::StringType_ArrayIndex) {
        Heap::String *str = engine->newString(s);
        str->stringHash = hash;
        str->subtype = subtype;
        return str;
    }
    uint idx = hash % alloc;
    while (Heap::StringOrSymbol *e = entriesByHash[idx]) {
        if (e->stringHash == hash && e->toQString() == s)
            return static_cast<Heap::String *>(e);
        ++idx;
        idx %= alloc;
    }

    Heap::String *str = engine->newString(s);
    str->stringHash = hash;
    str->subtype = subtype;
    addEntry(str);
    return str;
}

Heap::Symbol *IdentifierTable::insertSymbol(const QString &s)
{
    Q_ASSERT(s.at(0) == QLatin1Char('@'));

    uint subtype;
    uint hash = String::createHashValue(s.constData(), s.length(), &subtype);
    uint idx = hash % alloc;
    while (Heap::StringOrSymbol *e = entriesByHash[idx]) {
        if (e->stringHash == hash && e->toQString() == s)
            return static_cast<Heap::Symbol *>(e);
        ++idx;
        idx %= alloc;
    }

    Heap::Symbol *str = Symbol::create(engine, s);
    str->stringHash = hash;
    str->subtype = subtype;
    addEntry(str);
    return str;

}


PropertyKey IdentifierTable::asPropertyKeyImpl(const Heap::String *str)
{
    if (str->identifier.isValid())
        return str->identifier;
    uint hash = str->hashValue();
    if (str->subtype == Heap::String::StringType_ArrayIndex) {
        str->identifier = PropertyKey::fromArrayIndex(hash);
        return str->identifier;
    }

    uint idx = hash % alloc;
    while (Heap::StringOrSymbol *e = entriesByHash[idx]) {
        if (e->stringHash == hash && e->toQString() == str->toQString()) {
            str->identifier = e->identifier;
            return e->identifier;
        }
        ++idx;
        idx %= alloc;
    }

    addEntry(const_cast<QV4::Heap::String *>(str));
    return str->identifier;
}

Heap::StringOrSymbol *IdentifierTable::resolveId(PropertyKey i) const
{
    uint arrayIdx = i.asArrayIndex();
    if (arrayIdx < UINT_MAX)
        return engine->newString(QString::number(arrayIdx));
    if (!i.isValid())
        return nullptr;

    uint idx = i.id() % alloc;
    while (1) {
        Heap::StringOrSymbol *e = entriesById[idx];
        if (!e || e->identifier == i)
            return e;
        ++idx;
        idx %= alloc;
    }
}

Heap::String *IdentifierTable::stringForId(PropertyKey i) const
{
    Heap::StringOrSymbol *s = resolveId(i);
    Q_ASSERT(s && s->internalClass->vtable->isString);
    return static_cast<Heap::String *>(s);
}

Heap::Symbol *IdentifierTable::symbolForId(PropertyKey i) const
{
    Heap::StringOrSymbol *s = resolveId(i);
    Q_ASSERT(!s || !s->internalClass->vtable->isString);
    return static_cast<Heap::Symbol *>(s);
}

void IdentifierTable::markObjects(MarkStack *markStack)
{
    for (const auto &h : idHashes)
        h->markObjects(markStack);
}

void IdentifierTable::sweep()
{
    int freed = 0;

    Heap::StringOrSymbol **newTable = (Heap::StringOrSymbol **)malloc(alloc*sizeof(Heap::String *));
    memset(newTable, 0, alloc*sizeof(Heap::StringOrSymbol *));
    memset(entriesById, 0, alloc*sizeof(Heap::StringOrSymbol *));
    for (uint i = 0; i < alloc; ++i) {
        Heap::StringOrSymbol *e = entriesByHash[i];
        if (!e)
            continue;
        if (!e->isMarked()) {
            ++freed;
            continue;
        }
        uint idx = e->hashValue() % alloc;
        while (newTable[idx]) {
            ++idx;
            if (idx == alloc)
                idx = 0;
        }
        newTable[idx] = e;

        idx = e->identifier.id() % alloc;
        while (entriesById[idx]) {
            ++idx;
            if (idx == alloc)
                idx = 0;
        }
        entriesById[idx] = e;
    }
    free(entriesByHash);
    entriesByHash = newTable;

    size -= freed;
}

PropertyKey IdentifierTable::asPropertyKey(const QString &s)
{
    return insertString(s)->identifier;
}

PropertyKey IdentifierTable::asPropertyKey(const char *s, int len)
{
    uint subtype;
    uint hash = String::createHashValue(s, len, &subtype);
    if (hash == UINT_MAX)
        return asPropertyKey(QString::fromUtf8(s, len));

    QLatin1String latin(s, len);
    uint idx = hash % alloc;
    while (Heap::StringOrSymbol *e = entriesByHash[idx]) {
        if (e->stringHash == hash && e->toQString() == latin)
            return e->identifier;
        ++idx;
        idx %= alloc;
    }

    Heap::String *str = engine->newString(QString::fromLatin1(s, len));
    str->stringHash = hash;
    str->subtype = subtype;
    addEntry(str);
    return str->identifier;
}

}

QT_END_NAMESPACE
