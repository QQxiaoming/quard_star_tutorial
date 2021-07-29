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

#include "qv4jithelpers_p.h"
#include "qv4engine_p.h"
#include "qv4function_p.h"
#include "qv4value_p.h"
#include "qv4object_p.h"
#include "qv4functionobject_p.h"
#include "qv4lookup_p.h"
#include <QtCore/private/qnumeric_p.h>

#ifdef V4_ENABLE_JIT

QT_BEGIN_NAMESPACE
namespace QV4 {
namespace JIT {
namespace Helpers {

void convertThisToObject(ExecutionEngine *engine, Value *t)
{
    if (!t->isObject()) {
        if (t->isNullOrUndefined()) {
            *t = engine->globalObject->asReturnedValue();
        } else {
            *t = t->toObject(engine)->asReturnedValue();
        }
    }
}

ReturnedValue loadGlobalLookup(Function *f, ExecutionEngine *engine, int index)
{
    Lookup *l = f->compilationUnit->runtimeLookups + index;
    return l->globalGetter(l, engine);
}

ReturnedValue loadQmlContextPropertyLookup(Function *f, ExecutionEngine *engine, int index)
{
    Lookup *l = f->compilationUnit->runtimeLookups + index;
    return l->qmlContextPropertyGetter(l, engine, nullptr);
}

ReturnedValue toObject(ExecutionEngine *engine, const Value &obj)
{
    if (obj.isObject())
        return obj.asReturnedValue();

    return obj.toObject(engine)->asReturnedValue();
}

ReturnedValue exp(const Value &base, const Value &exp)
{
    double b = base.toNumber();
    double e = exp.toNumber();
    if (qt_is_inf(e) && (b == 1 || b == -1))
        return Encode(qt_snan());
    return Encode(pow(b,e));
}

ReturnedValue getLookup(Function *f, ExecutionEngine *engine, const Value &base, int index)
{
    Lookup *l = f->compilationUnit->runtimeLookups + index;
    return l->getter(l, engine, base);
}

void setLookupSloppy(Function *f, int index, Value &base, const Value &value)
{
    ExecutionEngine *engine = f->internalClass->engine;
    QV4::Lookup *l = f->compilationUnit->runtimeLookups + index;
    l->setter(l, engine, base, value);
}

void setLookupStrict(Function *f, int index, Value &base, const Value &value)
{
    ExecutionEngine *engine = f->internalClass->engine;
    QV4::Lookup *l = f->compilationUnit->runtimeLookups + index;
    if (!l->setter(l, engine, base, value))
        engine->throwTypeError();
}


void pushBlockContext(Value *stack, int index)
{
    ExecutionContext *c = static_cast<ExecutionContext *>(stack + CallData::Context);
    stack[CallData::Context] = Runtime::method_createBlockContext(c, index);
}

void cloneBlockContext(Value *contextSlot)
{
    *contextSlot = Runtime::method_cloneBlockContext(static_cast<QV4::ExecutionContext *>(contextSlot));
}

void pushScriptContext(Value *stack, ExecutionEngine *engine, int index)
{
    stack[CallData::Context] = Runtime::method_createScriptContext(engine, index);
}

void popScriptContext(Value *stack, ExecutionEngine *engine)
{
    stack[CallData::Context] = Runtime::method_popScriptContext(engine);
}

ReturnedValue deleteProperty(QV4::Function *function, const QV4::Value &base, const QV4::Value &index)
{
    auto engine = function->internalClass->engine;
    if (!Runtime::method_deleteProperty(engine, base, index)) {
        if (function->isStrict())
            engine->throwTypeError();
        return Encode(false);
    } else {
        return Encode(true);
    }
}

ReturnedValue deleteName(Function *function, int name)
{
    auto engine = function->internalClass->engine;
    if (!Runtime::method_deleteName(engine, name)) {
        if (function->isStrict())
            engine->throwTypeError();
        return Encode(false);
    } else {
        return Encode(true);
    }
}

void throwOnNullOrUndefined(ExecutionEngine *engine, const Value &v)
{
    if (v.isNullOrUndefined())
        engine->throwTypeError();
}

} // Helpers namespace
} // JIT namespace
} // QV4 namespace
QT_END_NAMESPACE

#endif // V4_ENABLE_JIT
