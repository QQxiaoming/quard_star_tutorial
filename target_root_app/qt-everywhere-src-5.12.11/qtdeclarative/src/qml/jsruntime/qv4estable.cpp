/****************************************************************************
**
** Copyright (C) 2018 Crimson AS <info@crimson.no>
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

#include "qv4estable_p.h"
#include "qv4object_p.h"

using namespace QV4;

// The ES spec requires that Map/Set be implemented using a data structure that
// is a little different from most; it requires nonlinear access, and must also
// preserve the order of insertion of items in a deterministic way.
//
// This class implements those requirements, except for fast access: that
// will be addressed in a followup patch.

ESTable::ESTable()
    : m_capacity(8)
{
    m_keys = (Value*)malloc(m_capacity * sizeof(Value));
    m_values = (Value*)malloc(m_capacity * sizeof(Value));
    memset(m_keys, 0, m_capacity);
    memset(m_values, 0, m_capacity);
}

ESTable::~ESTable()
{
    free(m_keys);
    free(m_values);
    m_size = 0;
    m_capacity = 0;
    m_keys = nullptr;
    m_values = nullptr;
}

void ESTable::markObjects(MarkStack *s, bool isWeakMap)
{
    for (uint i = 0; i < m_size; ++i) {
        if (!isWeakMap)
            m_keys[i].mark(s);
        m_values[i].mark(s);
    }
}

// Pretends that there's nothing in the table. Doesn't actually free memory, as
// it will almost certainly be reused again anyway.
void ESTable::clear()
{
    m_size = 0;
}

// Update the table to contain \a value for a given \a key. The key is
// normalized, as required by the ES spec.
void ESTable::set(const Value &key, const Value &value)
{
    for (uint i = 0; i < m_size; ++i) {
        if (m_keys[i].sameValueZero(key)) {
            m_values[i] = value;
            return;
        }
    }

    if (m_capacity == m_size) {
        uint oldCap = m_capacity;
        m_capacity *= 2;
        m_keys = (Value*)realloc(m_keys, m_capacity * sizeof(Value));
        m_values = (Value*)realloc(m_values, m_capacity * sizeof(Value));
        memset(m_keys + oldCap, 0, m_capacity - oldCap);
        memset(m_values + oldCap, 0, m_capacity - oldCap);
    }

    Value nk = key;
    if (nk.isDouble()) {
        if (nk.doubleValue() == 0 && std::signbit(nk.doubleValue()))
            nk = Value::fromDouble(+0);
    }

    m_keys[m_size] = nk;
    m_values[m_size] = value;

    m_size++;
}

// Returns true if the table contains \a key, false otherwise.
bool ESTable::has(const Value &key) const
{
    for (uint i = 0; i < m_size; ++i) {
        if (m_keys[i].sameValueZero(key))
            return true;
    }

    return false;
}

// Fetches the value for the given \a key, and if \a hasValue is passed in,
// it is set depending on whether or not the given key was found.
ReturnedValue ESTable::get(const Value &key, bool *hasValue) const
{
    for (uint i = 0; i < m_size; ++i) {
        if (m_keys[i].sameValueZero(key)) {
            if (hasValue)
                *hasValue = true;
            return m_values[i].asReturnedValue();
        }
    }

    if (hasValue)
        *hasValue = false;
    return Encode::undefined();
}

// Removes the given \a key from the table
bool ESTable::remove(const Value &key)
{
    bool found = false;
    uint idx = 0;
    for (; idx < m_size; ++idx) {
        if (m_keys[idx].sameValueZero(key)) {
            found = true;
            break;
        }
    }

    if (found == true) {
        memmove(m_keys + idx, m_keys + idx + 1, (m_size - idx)*sizeof(Value));
        memmove(m_values + idx, m_values + idx + 1, (m_size - idx)*sizeof(Value));
        m_size--;
    }
    return found;
}

// Returns the size of the table. Note that the size may not match the underlying allocation.
uint ESTable::size() const
{
    return m_size;
}

// Retrieves a key and value for a given \a idx, and places them in \a key and
// \a value. They must be valid pointers.
void ESTable::iterate(uint idx, Value *key, Value *value)
{
    Q_ASSERT(idx < m_size);
    Q_ASSERT(key);
    Q_ASSERT(value);
    *key = m_keys[idx];
    *value = m_values[idx];
}

void ESTable::removeUnmarkedKeys()
{
    uint idx = 0;
    uint toIdx = 0;
    for (; idx < m_size; ++idx) {
        Q_ASSERT(m_keys[idx].isObject());
        Object &o = static_cast<Object &>(m_keys[idx]);
        if (o.d()->isMarked()) {
            m_keys[toIdx] = m_keys[idx];
            m_values[toIdx] = m_values[idx];
            ++toIdx;
        }
    }
    m_size = toIdx;
}

