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

#ifndef TEMPLATE_H
#define TEMPLATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qv4global_p.h>

//QT_REQUIRE_CONFIG(qml_jit);

QT_BEGIN_NAMESPACE

namespace QV4 {

#ifdef V4_ENABLE_JIT

namespace JIT {
namespace Helpers {

void convertThisToObject(ExecutionEngine *engine, Value *t);
ReturnedValue loadGlobalLookup(Function *f, ExecutionEngine *engine, int index);
ReturnedValue loadQmlContextPropertyLookup(Function *f, ExecutionEngine *engine, int index);
ReturnedValue toObject(ExecutionEngine *engine, const Value &obj);
ReturnedValue exp(const Value &base, const Value &exp);
ReturnedValue getLookup(Function *f, ExecutionEngine *engine, const Value &base, int index);
void setLookupStrict(Function *f, int index, Value &base, const Value &value);
void setLookupSloppy(Function *f, int index, Value &base, const Value &value);
void pushBlockContext(Value *stack, int index);
void cloneBlockContext(Value *contextSlot);
void pushScriptContext(Value *stack, ExecutionEngine *engine, int index);
void popScriptContext(Value *stack, ExecutionEngine *engine);
ReturnedValue deleteProperty(QV4::Function *function, const QV4::Value &base, const QV4::Value &index);
ReturnedValue deleteName(Function *function, int name);
void throwOnNullOrUndefined(ExecutionEngine *engine, const Value &v);

} // Helpers namespace
} // JIT namespace

#endif // V4_ENABLE_JIT

} // QV4 namespace

QT_END_NAMESPACE

#endif // TEMPLATE_H
