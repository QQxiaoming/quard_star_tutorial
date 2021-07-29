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

#include "qv4managed_p.h"
#include <private/qv4mm_p.h>
#include "qv4errorobject_p.h"

using namespace QV4;

DEFINE_MANAGED_VTABLE(Managed);

DEFINE_MANAGED_VTABLE(InternalClass);


QString Managed::className() const
{
    const char *s = nullptr;
    switch (Type(vtable()->type)) {
    case Type_Invalid:
        return QString();
    case Type_String:
        s = "String";
        break;
    case Type_Symbol:
        s = "Symbol";
        break;
    case Type_Object:
        s = "Object";
        break;
    case Type_ArrayObject:
        s = "Array";
        break;
    case Type_FunctionObject:
        s = "Function";
        break;
    case Type_GeneratorObject:
        s = "Generator";
        break;
    case Type_BooleanObject:
        s = "Boolean";
        break;
    case Type_NumberObject:
        s = "Number";
        break;
    case Type_StringObject:
        s = "String";
        break;
    case Type_SymbolObject:
        s = "Symbol";
        break;
    case Type_DateObject:
        s = "Date";
        break;
    case Type_RegExpObject:
        s = "RegExp";
        break;
    case Type_ErrorObject:
        s = "Error";
        break;
    case Type_ArgumentsObject:
        s = "Arguments";
        break;
    case Type_JsonObject:
        s = "JSON";
        break;
    case Type_ProxyObject:
        s = "ProxyObject";
        break;
    case Type_MathObject:
        s = "Math";
        break;

    case Type_ExecutionContext:
        s = "__ExecutionContext";
        break;
    case Type_MapIteratorObject:
        s = "Map Iterator";
        break;
    case Type_SetIteratorObject:
        s = "Set Iterator";
        break;
    case Type_ArrayIteratorObject:
        s = "Array Iterator";
        break;
    case Type_StringIteratorObject:
        s = "String Iterator";
        break;
    case Type_ForInIterator:
        s = "__ForIn Iterator";
        break;
    case Type_InternalClass:
        s = "__InternalClass";
        break;
    case Type_RegExp:
        s = "__RegExp";
        break;

    case Type_QmlSequence:
        s = "QmlSequence";
        break;
    }
    return QString::fromLatin1(s);
}

bool Managed::virtualIsEqualTo(Managed *, Managed *)
{
    return false;
}


OwnPropertyKeyIterator::~OwnPropertyKeyIterator()
{
}
