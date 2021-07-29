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

#include "qv4memberdata_p.h"
#include <private/qv4mm_p.h>
#include "qv4value_p.h"

using namespace QV4;

DEFINE_MANAGED_VTABLE(MemberData);

static size_t nextPowerOfTwo(size_t s)
{
    --s;
    s |= s >> 1;
    s |= s >> 2;
    s |= s >> 4;
    s |= s >> 8;
    s |= s >> 16;
#if (QT_POINTER_SIZE == 8)
        s |= s >> 32;
#endif
    ++s;
    return s;
}

Heap::MemberData *MemberData::allocate(ExecutionEngine *e, uint n, Heap::MemberData *old)
{
    Q_ASSERT(!old || old->values.size <= n);
    if (!n)
        n = 4;

    size_t alloc = MemoryManager::align(sizeof(Heap::MemberData) + (n - 1)*sizeof(Value));
    // round up to next power of two to avoid quadratic behaviour for very large objects
    alloc = nextPowerOfTwo(alloc);

    // The above code can overflow in a number of interesting ways. All of those are unsigned,
    // and therefore defined behavior. Still, apply some sane bounds.
    const size_t intMax = std::numeric_limits<int>::max();
    if (alloc > intMax)
        alloc = intMax;

    Heap::MemberData *m;
    if (old) {
        const size_t oldSize = sizeof(Heap::MemberData) + (old->values.size - 1) * sizeof(Value);
        if (oldSize > alloc)
            alloc = oldSize;
        m = e->memoryManager->allocManaged<MemberData>(alloc);
        // no write barrier required here
        memcpy(m, old, oldSize);
    } else {
        m = e->memoryManager->allocManaged<MemberData>(alloc);
        m->init();
    }

    m->values.alloc = static_cast<uint>((alloc - sizeof(Heap::MemberData) + sizeof(Value))/sizeof(Value));
    m->values.size = m->values.alloc;
    return m;
}
