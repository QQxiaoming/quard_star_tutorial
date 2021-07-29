/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qv4propertykey_p.h"

#include <QtCore/qstring.h>
#include <qv4string_p.h>
#include <qv4engine_p.h>
#include <qv4scopedvalue_p.h>

QV4::Heap::StringOrSymbol *QV4::PropertyKey::toStringOrSymbol(QV4::ExecutionEngine *e)
{
    if (isArrayIndex())
        return Value::fromUInt32(asArrayIndex()).toString(e);
    return static_cast<Heap::StringOrSymbol *>(asStringOrSymbol());
}

bool QV4::PropertyKey::isString() const {
    Heap::StringOrSymbol *s = asStringOrSymbol();
    return s && s->internalClass->vtable->isString;
}

bool QV4::PropertyKey::isSymbol() const {
    Heap::Base *s = asStringOrSymbol();
    return s && !s->internalClass->vtable->isString && s->internalClass->vtable->isStringOrSymbol;
}

bool QV4::PropertyKey::isCanonicalNumericIndexString() const
{
    if (isArrayIndex())
        return true;
    if (isSymbol())
        return false;
    Heap::String *s = static_cast<Heap::String *>(asStringOrSymbol());
    Scope scope(s->internalClass->engine);
    ScopedString str(scope, s);
    double d = str->toNumber();
    if (d == 0. && std::signbit(d))
        return true;
    ScopedString converted(scope, Value::fromDouble(d).toString(scope.engine));
    if (converted->equals(str))
        return true;
    return false;
}

QString QV4::PropertyKey::toQString() const
{
    if (isArrayIndex())
        return QString::number(asArrayIndex());
    Heap::StringOrSymbol *s = asStringOrSymbol();
    Q_ASSERT(s->internalClass->vtable->isStringOrSymbol);
    return s->toQString();
}

QV4::Heap::String *QV4::PropertyKey::asFunctionName(ExecutionEngine *engine, FunctionNamePrefix prefix) const
{
    QString n;
    if (prefix == Getter)
        n = QStringLiteral("get ");
    else if (prefix == Setter)
        n = QStringLiteral("set ");
    if (isArrayIndex())
        n += QString::number(asArrayIndex());
    else {
        Heap::StringOrSymbol *s = asStringOrSymbol();
        QString str = s->toQString();
        if (s->internalClass->vtable->isString)
            n += s->toQString();
        else if (str.length() > 1)
            n += QChar::fromLatin1('[') + str.midRef(1) + QChar::fromLatin1(']');
    }
    return engine->newString(n);
}
