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
#ifndef QV4RUNTIMEAPI_P_H
#define QV4RUNTIMEAPI_P_H

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

QT_BEGIN_NAMESPACE

namespace QV4 {

typedef uint Bool;
struct NoThrowEngine;
namespace {
template <typename T>
struct ExceptionCheck {
    enum { NeedsCheck = 1 };
};
// push_catch and pop context methods shouldn't check for exceptions
template <>
struct ExceptionCheck<void (*)(QV4::NoThrowEngine *)> {
    enum { NeedsCheck = 0 };
};
template <typename A>
struct ExceptionCheck<void (*)(A, QV4::NoThrowEngine)> {
    enum { NeedsCheck = 0 };
};
template <>
struct ExceptionCheck<QV4::ReturnedValue (*)(QV4::NoThrowEngine *)> {
    enum { NeedsCheck = 0 };
};
template <typename A>
struct ExceptionCheck<QV4::ReturnedValue (*)(QV4::NoThrowEngine *, A)> {
    enum { NeedsCheck = 0 };
};
template <typename A, typename B>
struct ExceptionCheck<QV4::ReturnedValue (*)(QV4::NoThrowEngine *, A, B)> {
    enum { NeedsCheck = 0 };
};
template <typename A, typename B, typename C>
struct ExceptionCheck<void (*)(QV4::NoThrowEngine *, A, B, C)> {
    enum { NeedsCheck = 0 };
};
} // anonymous namespace

#define FOR_EACH_RUNTIME_METHOD(F) \
    /* call */ \
    F(ReturnedValue, callGlobalLookup, (ExecutionEngine *engine, uint index, Value *argv, int argc)) \
    F(ReturnedValue, callQmlContextPropertyLookup, (ExecutionEngine *engine, uint index, Value *argv, int argc)) \
    F(ReturnedValue, callName, (ExecutionEngine *engine, int nameIndex, Value *argv, int argc)) \
    F(ReturnedValue, callProperty, (ExecutionEngine *engine, Value *base, int nameIndex, Value *argv, int argc)) \
    F(ReturnedValue, callPropertyLookup, (ExecutionEngine *engine, Value *base, uint index, Value *argv, int argc)) \
    F(ReturnedValue, callElement, (ExecutionEngine *engine, Value *base, const Value &index, Value *argv, int argc)) \
    F(ReturnedValue, callValue, (ExecutionEngine *engine, const Value &func, Value *argv, int argc)) \
    F(ReturnedValue, callWithReceiver, (ExecutionEngine *engine, const Value &func, const Value *thisObject, Value *argv, int argc)) \
    F(ReturnedValue, callPossiblyDirectEval, (ExecutionEngine *engine, Value *argv, int argc)) \
    F(ReturnedValue, callWithSpread, (ExecutionEngine *engine, const Value &func, const Value &thisObject, Value *argv, int argc)) \
    F(ReturnedValue, tailCall, (CppStackFrame *frame, ExecutionEngine *engine)) \
    \
    /* construct */ \
    F(ReturnedValue, construct, (ExecutionEngine *engine, const Value &func, const Value &newTarget, Value *argv, int argc)) \
    F(ReturnedValue, constructWithSpread, (ExecutionEngine *engine, const Value &func, const Value &newTarget, Value *argv, int argc)) \
    \
    /* load & store */ \
    F(void, storeNameStrict, (ExecutionEngine *engine, int nameIndex, const Value &value)) \
    F(void, storeNameSloppy, (ExecutionEngine *engine, int nameIndex, const Value &value)) \
    F(void, storeProperty, (ExecutionEngine *engine, const Value &object, int nameIndex, const Value &value)) \
    F(void, storeElement, (ExecutionEngine *engine, const Value &object, const Value &index, const Value &value)) \
    F(ReturnedValue, loadProperty, (ExecutionEngine *engine, const Value &object, int nameIndex)) \
    F(ReturnedValue, loadName, (ExecutionEngine *engine, int nameIndex)) \
    F(ReturnedValue, loadElement, (ExecutionEngine *engine, const Value &object, const Value &index)) \
    F(ReturnedValue, loadSuperProperty, (ExecutionEngine *engine, const Value &property)) \
    F(void, storeSuperProperty, (ExecutionEngine *engine, const Value &property, const Value &value)) \
    F(ReturnedValue, loadSuperConstructor, (ExecutionEngine *engine, const Value &t)) \
    \
    /* typeof */  \
    F(ReturnedValue, typeofValue, (ExecutionEngine *engine, const Value &val)) \
    F(ReturnedValue, typeofName, (ExecutionEngine *engine, int nameIndex)) \
    \
    /* delete */ \
    F(bool, deleteProperty, (ExecutionEngine *engine, const Value &base, const Value &index)) \
    F(bool, deleteName, (ExecutionEngine *engine, int nameIndex)) \
    \
    /* exceptions & scopes */ \
    F(void, throwException, (ExecutionEngine *engine, const Value &value)) \
    F(ReturnedValue, createWithContext, (ExecutionEngine *, Value *jsStackFrame)) \
    F(ReturnedValue, createCatchContext, (ExecutionContext *parent, int blockIndex, int exceptionVarNameIndex)) \
    F(ReturnedValue, createBlockContext, (ExecutionContext *parent, int index)) \
    F(ReturnedValue, createScriptContext, (ExecutionEngine *engine, int index)) \
    F(ReturnedValue, cloneBlockContext, (ExecutionContext *previous)) \
    F(ReturnedValue, popScriptContext, (ExecutionEngine *engine)) \
    F(void, throwReferenceError, (ExecutionEngine *engine, int nameIndex)) \
    \
    /* closures */ \
    F(ReturnedValue, closure, (ExecutionEngine *engine, int functionId)) \
    \
    /* function header */ \
    F(void, declareVar, (ExecutionEngine *engine, bool deletable, int nameIndex)) \
    F(ReturnedValue, createMappedArgumentsObject, (ExecutionEngine *engine)) \
    F(ReturnedValue, createUnmappedArgumentsObject, (ExecutionEngine *engine)) \
    F(ReturnedValue, createRestParameter, (ExecutionEngine *engine, int argIndex)) \
    \
    /* literals */ \
    F(ReturnedValue, arrayLiteral, (ExecutionEngine *engine, Value *values, uint length)) \
    F(ReturnedValue, objectLiteral, (ExecutionEngine *engine, int classId, const Value *args, int argc)) \
    F(ReturnedValue, createClass, (ExecutionEngine *engine, int classIndex, const Value &heritage, const Value *computedNames)) \
    \
    /* for-in, for-of and array destructuring */ \
    F(ReturnedValue, getIterator, (ExecutionEngine *engine, const Value &in, int iterator)) \
    F(ReturnedValue, iteratorNext, (ExecutionEngine *engine, const Value &iterator, Value *value)) \
    F(ReturnedValue, iteratorNextForYieldStar, (ExecutionEngine *engine, const Value &received, const Value &iterator, Value *object)) \
    F(ReturnedValue, iteratorClose, (ExecutionEngine *engine, const Value &iterator, const Value &done)) \
    F(ReturnedValue, destructureRestElement, (ExecutionEngine *engine, const Value &iterator)) \
    \
    /* unary operators */ \
    F(ReturnedValue, uMinus, (const Value &value)) \
    \
    /* binary operators */ \
    F(ReturnedValue, instanceof, (ExecutionEngine *engine, const Value &left, const Value &right)) \
    F(ReturnedValue, in, (ExecutionEngine *engine, const Value &left, const Value &right)) \
    F(ReturnedValue, add, (ExecutionEngine *engine, const Value &left, const Value &right)) \
    F(ReturnedValue, sub, (const Value &left, const Value &right)) \
    F(ReturnedValue, mul, (const Value &left, const Value &right)) \
    F(ReturnedValue, div, (const Value &left, const Value &right)) \
    F(ReturnedValue, mod, (const Value &left, const Value &right)) \
    F(ReturnedValue, shl, (const Value &left, const Value &right)) \
    F(ReturnedValue, shr, (const Value &left, const Value &right)) \
    F(ReturnedValue, ushr, (const Value &left, const Value &right)) \
    F(ReturnedValue, greaterThan, (const Value &left, const Value &right)) \
    F(ReturnedValue, lessThan, (const Value &left, const Value &right)) \
    F(ReturnedValue, greaterEqual, (const Value &left, const Value &right)) \
    F(ReturnedValue, lessEqual, (const Value &left, const Value &right)) \
    F(ReturnedValue, equal, (const Value &left, const Value &right)) \
    F(ReturnedValue, notEqual, (const Value &left, const Value &right)) \
    F(ReturnedValue, strictEqual, (const Value &left, const Value &right)) \
    F(ReturnedValue, strictNotEqual, (const Value &left, const Value &right)) \
    \
    /* comparisons */ \
    F(Bool, compareGreaterThan, (const Value &l, const Value &r)) \
    F(Bool, compareLessThan, (const Value &l, const Value &r)) \
    F(Bool, compareGreaterEqual, (const Value &l, const Value &r)) \
    F(Bool, compareLessEqual, (const Value &l, const Value &r)) \
    F(Bool, compareEqual, (const Value &left, const Value &right)) \
    F(Bool, compareNotEqual, (const Value &left, const Value &right)) \
    F(Bool, compareStrictEqual, (const Value &left, const Value &right)) \
    F(Bool, compareStrictNotEqual, (const Value &left, const Value &right)) \
    \
    F(Bool, compareInstanceof, (ExecutionEngine *engine, const Value &left, const Value &right)) \
    F(Bool, compareIn, (ExecutionEngine *engine, const Value &left, const Value &right)) \
    \
    F(ReturnedValue, regexpLiteral, (ExecutionEngine *engine, int id))

struct Q_QML_PRIVATE_EXPORT Runtime {
    Runtime();

    typedef ReturnedValue (*UnaryOperation)(const Value &value);
    typedef ReturnedValue (*BinaryOperation)(const Value &left, const Value &right);
    typedef ReturnedValue (*BinaryOperationContext)(ExecutionEngine *engine, const Value &left, const Value &right);

#define DEFINE_RUNTIME_METHOD_ENUM(returnvalue, name, args) name,
    enum RuntimeMethods {
        FOR_EACH_RUNTIME_METHOD(DEFINE_RUNTIME_METHOD_ENUM)
        RuntimeMethodCount,
        InvalidRuntimeMethod = RuntimeMethodCount
    };
#undef DEFINE_RUNTIME_METHOD_ENUM

    void *runtimeMethods[RuntimeMethodCount];

    static uint runtimeMethodOffset(RuntimeMethods method) { return method*QT_POINTER_SIZE; }

#define RUNTIME_METHOD(returnvalue, name, args) \
    typedef returnvalue (*Method_##name)args; \
    enum { Method_##name##_NeedsExceptionCheck = ExceptionCheck<Method_##name>::NeedsCheck }; \
    static returnvalue method_##name args;
    FOR_EACH_RUNTIME_METHOD(RUNTIME_METHOD)
#undef RUNTIME_METHOD

    struct StackOffsets {
        static const int tailCall_function   = -1;
        static const int tailCall_thisObject = -2;
        static const int tailCall_argv       = -3;
        static const int tailCall_argc       = -4;
    };
};

static_assert(std::is_standard_layout<Runtime>::value, "Runtime needs to be standard layout in order for us to be able to use offsetof");
static_assert(offsetof(Runtime, runtimeMethods) == 0, "JIT expects this to be the first member");
static_assert(sizeof(Runtime::BinaryOperation) == sizeof(void*), "JIT expects a function pointer to fit into a regular pointer, for cross-compilation offset translation");

} // namespace QV4

QT_END_NAMESPACE

#endif // QV4RUNTIMEAPI_P_H
