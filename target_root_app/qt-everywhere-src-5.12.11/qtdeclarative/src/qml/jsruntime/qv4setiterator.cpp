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

#include <private/qv4iterator_p.h>
#include <private/qv4estable_p.h>
#include <private/qv4setiterator_p.h>
#include <private/qv4setobject_p.h>
#include <private/qv4symbol_p.h>

using namespace QV4;

DEFINE_OBJECT_VTABLE(SetIteratorObject);

void SetIteratorPrototype::init(ExecutionEngine *e)
{
    defineDefaultProperty(QStringLiteral("next"), method_next, 0);

    Scope scope(e);
    ScopedString val(scope, e->newString(QLatin1String("Set Iterator")));
    defineReadonlyConfigurableProperty(e->symbol_toStringTag(), val);
}

ReturnedValue SetIteratorPrototype::method_next(const FunctionObject *b, const Value *that, const Value *, int)
{
    Scope scope(b);
    const SetIteratorObject *thisObject = that->as<SetIteratorObject>();
    if (!thisObject)
        return scope.engine->throwTypeError(QLatin1String("Not a Set Iterator instance"));

    Scoped<SetObject> s(scope, thisObject->d()->iteratedSet);
    uint index = thisObject->d()->setNextIndex;
    IteratorKind itemKind = thisObject->d()->iterationKind;

    if (!s) {
        QV4::Value undefined = Value::undefinedValue();
        return IteratorPrototype::createIterResultObject(scope.engine, undefined, true);
    }

    Value *arguments = scope.alloc(2);

    while (index < s->d()->esTable->size()) {
        s->d()->esTable->iterate(index, &arguments[0], &arguments[1]);
        thisObject->d()->setNextIndex = index + 1;

        if (itemKind == KeyValueIteratorKind) {
            ScopedArrayObject resultArray(scope, scope.engine->newArrayObject());
            resultArray->arrayReserve(2);
            resultArray->arrayPut(0, arguments[0]);
            resultArray->arrayPut(1, arguments[0]); // yes, the key is repeated.
            resultArray->setArrayLengthUnchecked(2);

            return IteratorPrototype::createIterResultObject(scope.engine, resultArray, false);
        }

        return IteratorPrototype::createIterResultObject(scope.engine, arguments[0], false);
    }

    thisObject->d()->iteratedSet.set(scope.engine, nullptr);
    QV4::Value undefined = Value::undefinedValue();
    return IteratorPrototype::createIterResultObject(scope.engine, undefined, true);
}

