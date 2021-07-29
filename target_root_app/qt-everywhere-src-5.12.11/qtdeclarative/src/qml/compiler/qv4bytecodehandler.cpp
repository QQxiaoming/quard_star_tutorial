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

#include <private/qv4bytecodehandler_p.h>

QT_USE_NAMESPACE
using namespace QV4;
using namespace Moth;

ByteCodeHandler::~ByteCodeHandler()
{
}

#define DISPATCH_INSTRUCTION(name, nargs, ...) \
    generate_##name( \
        __VA_ARGS__ \
    );

#define DECODE_AND_DISPATCH(instr) \
    { \
        INSTR_##instr(MOTH_DECODE_WITH_BASE) \
        Q_UNUSED(base_ptr); \
        _currentOffset = _nextOffset; \
        _nextOffset = code - start; \
        startInstruction(Instr::Type::instr); \
        INSTR_##instr(DISPATCH) \
        endInstruction(Instr::Type::instr); \
        continue; \
    }

void ByteCodeHandler::decode(const char *code, uint len)
{
    MOTH_JUMP_TABLE;

    const char *start = code;
    const char *end = code + len;
    while (code < end) {
        MOTH_DISPATCH()

        FOR_EACH_MOTH_INSTR(DECODE_AND_DISPATCH)
    }
}

#undef DECODE_AND_DISPATCH
#undef DISPATCH_INSTRUCTION

#define MOTH_UNUSED_ARGS0()
#define MOTH_UNUSED_ARGS1(arg) \
    Q_UNUSED(arg);
#define MOTH_UNUSED_ARGS2(arg1, arg2) \
    Q_UNUSED(arg1); \
    Q_UNUSED(arg2);
#define MOTH_UNUSED_ARGS3(arg1, arg2, arg3) \
    Q_UNUSED(arg1); \
    Q_UNUSED(arg2); \
    Q_UNUSED(arg3);
#define MOTH_UNUSED_ARGS4(arg1, arg2, arg3, arg4) \
    Q_UNUSED(arg1); \
    Q_UNUSED(arg2); \
    Q_UNUSED(arg3); \
    Q_UNUSED(arg4);

#define MOTH_MARK_ARGS_UNUSED_PLEASE(nargs, ...) \
    MOTH_EXPAND_FOR_MSVC(MOTH_UNUSED_ARGS##nargs(__VA_ARGS__))

#define MOTH_MARK_ARGS_UNUSED_INSTRUCTION(name, nargs, ...) \
    MOTH_MARK_ARGS_UNUSED_PLEASE(nargs, __VA_ARGS__)

#define COLLECTOR_BEGIN_INSTR(instr) \
    { \
        INSTR_##instr(MOTH_DECODE_WITH_BASE) \
        INSTR_##instr(MOTH_MARK_ARGS_UNUSED) \
        Q_UNUSED(base_ptr);

#define COLLECTOR_END_INSTR(instr) \
        continue; \
    }

std::vector<int> ByteCodeHandler::collectLabelsInBytecode(const char *code, uint len)
{
    MOTH_JUMP_TABLE;

    std::vector<int> labels;

    const auto addLabel = [&labels,len](int offset) {
        Q_ASSERT(offset >= 0 && offset < static_cast<int>(len));
        labels.push_back(offset);
    };

    const char *start = code;
    const char *end = code + len;
    while (code < end) {
        MOTH_DISPATCH()
        Q_UNREACHABLE();

        COLLECTOR_BEGIN_INSTR(LoadReg)
        COLLECTOR_END_INSTR(LoadReg)

        COLLECTOR_BEGIN_INSTR(StoreReg)
        COLLECTOR_END_INSTR(StoreReg)

        COLLECTOR_BEGIN_INSTR(MoveReg)
        COLLECTOR_END_INSTR(MoveReg)

        COLLECTOR_BEGIN_INSTR(LoadConst)
        COLLECTOR_END_INSTR(LoadConst)

        COLLECTOR_BEGIN_INSTR(LoadNull)
        COLLECTOR_END_INSTR(LoadNull)

        COLLECTOR_BEGIN_INSTR(LoadZero)
        COLLECTOR_END_INSTR(LoadZero)

        COLLECTOR_BEGIN_INSTR(LoadTrue)
        COLLECTOR_END_INSTR(LoadTrue)

        COLLECTOR_BEGIN_INSTR(LoadFalse)
        COLLECTOR_END_INSTR(LoadFalse)

        COLLECTOR_BEGIN_INSTR(LoadUndefined)
        COLLECTOR_END_INSTR(LoadUndefined)

        COLLECTOR_BEGIN_INSTR(LoadInt)
        COLLECTOR_END_INSTR(LoadInt)

        COLLECTOR_BEGIN_INSTR(MoveConst)
        COLLECTOR_END_INSTR(MoveConst)

        COLLECTOR_BEGIN_INSTR(LoadImport)
        COLLECTOR_END_INSTR(LoadImport)

        COLLECTOR_BEGIN_INSTR(LoadLocal)
        COLLECTOR_END_INSTR(LoadLocal)

        COLLECTOR_BEGIN_INSTR(StoreLocal)
        COLLECTOR_END_INSTR(StoreLocal)

        COLLECTOR_BEGIN_INSTR(LoadScopedLocal)
        COLLECTOR_END_INSTR(LoadScopedLocal)

        COLLECTOR_BEGIN_INSTR(StoreScopedLocal)
        COLLECTOR_END_INSTR(StoreScopedLocal)

        COLLECTOR_BEGIN_INSTR(LoadRuntimeString)
        COLLECTOR_END_INSTR(LoadRuntimeString)

        COLLECTOR_BEGIN_INSTR(MoveRegExp)
        COLLECTOR_END_INSTR(MoveRegExp)

        COLLECTOR_BEGIN_INSTR(LoadClosure)
        COLLECTOR_END_INSTR(LoadClosure)

        COLLECTOR_BEGIN_INSTR(LoadName)
        COLLECTOR_END_INSTR(LoadName)

        COLLECTOR_BEGIN_INSTR(LoadGlobalLookup)
        COLLECTOR_END_INSTR(LoadGlobalLookup)

        COLLECTOR_BEGIN_INSTR(LoadQmlContextPropertyLookup)
        COLLECTOR_END_INSTR(LoadQmlContextPropertyLookup)

        COLLECTOR_BEGIN_INSTR(StoreNameSloppy)
        COLLECTOR_END_INSTR(StoreNameSloppy)

        COLLECTOR_BEGIN_INSTR(StoreNameStrict)
        COLLECTOR_END_INSTR(StoreNameStrict)

        COLLECTOR_BEGIN_INSTR(LoadElement)
        COLLECTOR_END_INSTR(LoadElement)

        COLLECTOR_BEGIN_INSTR(StoreElement)
        COLLECTOR_END_INSTR(StoreElement)

        COLLECTOR_BEGIN_INSTR(LoadProperty)
        COLLECTOR_END_INSTR(LoadProperty)

        COLLECTOR_BEGIN_INSTR(GetLookup)
        COLLECTOR_END_INSTR(GetLookup)

        COLLECTOR_BEGIN_INSTR(StoreProperty)
        COLLECTOR_END_INSTR(StoreProperty)

        COLLECTOR_BEGIN_INSTR(SetLookup)
        COLLECTOR_END_INSTR(SetLookup)

        COLLECTOR_BEGIN_INSTR(LoadSuperProperty)
        COLLECTOR_END_INSTR(LoadSuperProperty)

        COLLECTOR_BEGIN_INSTR(StoreSuperProperty)
        COLLECTOR_END_INSTR(StoreSuperProperty)

        COLLECTOR_BEGIN_INSTR(Yield)
        COLLECTOR_END_INSTR(Yield)

        COLLECTOR_BEGIN_INSTR(YieldStar)
        COLLECTOR_END_INSTR(YieldStar)

        COLLECTOR_BEGIN_INSTR(Resume)
        COLLECTOR_END_INSTR(Resume)

        COLLECTOR_BEGIN_INSTR(CallValue)
        COLLECTOR_END_INSTR(CallValue)

        COLLECTOR_BEGIN_INSTR(CallWithReceiver)
        COLLECTOR_END_INSTR(CallWithReceiver)

        COLLECTOR_BEGIN_INSTR(CallProperty)
        COLLECTOR_END_INSTR(CallProperty)

        COLLECTOR_BEGIN_INSTR(CallPropertyLookup)
        COLLECTOR_END_INSTR(CallPropertyLookup)

        COLLECTOR_BEGIN_INSTR(CallElement)
        COLLECTOR_END_INSTR(CallElement)

        COLLECTOR_BEGIN_INSTR(CallName)
        COLLECTOR_END_INSTR(CallName)

        COLLECTOR_BEGIN_INSTR(CallPossiblyDirectEval)
        COLLECTOR_END_INSTR(CallPossiblyDirectEval)

        COLLECTOR_BEGIN_INSTR(CallGlobalLookup)
        COLLECTOR_END_INSTR(CallGlobalLookup)

        COLLECTOR_BEGIN_INSTR(CallQmlContextPropertyLookup)
        COLLECTOR_END_INSTR(CallQmlContextPropertyLookup)

        COLLECTOR_BEGIN_INSTR(CallWithSpread)
        COLLECTOR_END_INSTR(CallWithSpread)

        COLLECTOR_BEGIN_INSTR(Construct)
        COLLECTOR_END_INSTR(Construct)

        COLLECTOR_BEGIN_INSTR(ConstructWithSpread)
        COLLECTOR_END_INSTR(ConstructWithSpread)

        COLLECTOR_BEGIN_INSTR(SetUnwindHandler)
            addLabel(code - start + offset);
        COLLECTOR_END_INSTR(SetUnwindHandler)

        COLLECTOR_BEGIN_INSTR(UnwindDispatch)
        COLLECTOR_END_INSTR(UnwindDispatch)

        COLLECTOR_BEGIN_INSTR(UnwindToLabel)
            addLabel(code - start + offset);
        COLLECTOR_END_INSTR(UnwindToLabel)

        COLLECTOR_BEGIN_INSTR(DeadTemporalZoneCheck)
        COLLECTOR_END_INSTR(DeadTemporalZoneCheck)

        COLLECTOR_BEGIN_INSTR(ThrowException)
        COLLECTOR_END_INSTR(ThrowException)

        COLLECTOR_BEGIN_INSTR(GetException)
        COLLECTOR_END_INSTR(HasException)

        COLLECTOR_BEGIN_INSTR(SetException)
        COLLECTOR_END_INSTR(SetExceptionFlag)

        COLLECTOR_BEGIN_INSTR(CreateCallContext)
        COLLECTOR_END_INSTR(CreateCallContext)

        COLLECTOR_BEGIN_INSTR(PushCatchContext)
        COLLECTOR_END_INSTR(PushCatchContext)

        COLLECTOR_BEGIN_INSTR(PushWithContext)
        COLLECTOR_END_INSTR(PushWithContext)

        COLLECTOR_BEGIN_INSTR(PushBlockContext)
        COLLECTOR_END_INSTR(PushBlockContext)

        COLLECTOR_BEGIN_INSTR(CloneBlockContext)
        COLLECTOR_END_INSTR(CloneBlockContext)

        COLLECTOR_BEGIN_INSTR(PushScriptContext)
        COLLECTOR_END_INSTR(PushScriptContext)

        COLLECTOR_BEGIN_INSTR(PopScriptContext)
        COLLECTOR_END_INSTR(PopScriptContext)

        COLLECTOR_BEGIN_INSTR(PopContext)
        COLLECTOR_END_INSTR(PopContext)

        COLLECTOR_BEGIN_INSTR(GetIterator)
        COLLECTOR_END_INSTR(GetIterator)

        COLLECTOR_BEGIN_INSTR(IteratorNext)
        COLLECTOR_END_INSTR(IteratorNext)

        COLLECTOR_BEGIN_INSTR(IteratorNextForYieldStar)
        COLLECTOR_END_INSTR(IteratorNextForYieldStar)

        COLLECTOR_BEGIN_INSTR(IteratorClose)
        COLLECTOR_END_INSTR(IteratorClose)

        COLLECTOR_BEGIN_INSTR(DestructureRestElement)
        COLLECTOR_END_INSTR(DestructureRestElement)

        COLLECTOR_BEGIN_INSTR(DeleteProperty)
        COLLECTOR_END_INSTR(DeleteProperty)

        COLLECTOR_BEGIN_INSTR(DeleteName)
        COLLECTOR_END_INSTR(DeleteName)

        COLLECTOR_BEGIN_INSTR(TypeofName)
        COLLECTOR_END_INSTR(TypeofName)

        COLLECTOR_BEGIN_INSTR(TypeofValue)
        COLLECTOR_END_INSTR(TypeofValue)

        COLLECTOR_BEGIN_INSTR(DeclareVar)
        COLLECTOR_END_INSTR(DeclareVar)

        COLLECTOR_BEGIN_INSTR(DefineArray)
        COLLECTOR_END_INSTR(DefineArray)

        COLLECTOR_BEGIN_INSTR(DefineObjectLiteral)
        COLLECTOR_END_INSTR(DefineObjectLiteral)

        COLLECTOR_BEGIN_INSTR(CreateClass)
        COLLECTOR_END_INSTR(CreateClass)

        COLLECTOR_BEGIN_INSTR(CreateMappedArgumentsObject)
        COLLECTOR_END_INSTR(CreateMappedArgumentsObject)

        COLLECTOR_BEGIN_INSTR(CreateUnmappedArgumentsObject)
        COLLECTOR_END_INSTR(CreateUnmappedArgumentsObject)

        COLLECTOR_BEGIN_INSTR(CreateRestParameter)
        COLLECTOR_END_INSTR(CreateRestParameter)

        COLLECTOR_BEGIN_INSTR(ConvertThisToObject)
        COLLECTOR_END_INSTR(ConvertThisToObject)

        COLLECTOR_BEGIN_INSTR(LoadSuperConstructor)
        COLLECTOR_END_INSTR(LoadSuperConstructor)

        COLLECTOR_BEGIN_INSTR(ToObject)
        COLLECTOR_END_INSTR(ToObject)

        COLLECTOR_BEGIN_INSTR(Jump)
            addLabel(code - start + offset);
        COLLECTOR_END_INSTR(Jump)

        COLLECTOR_BEGIN_INSTR(JumpTrue)
            addLabel(code - start + offset);
        COLLECTOR_END_INSTR(JumpTrue)

        COLLECTOR_BEGIN_INSTR(JumpFalse)
            addLabel(code - start + offset);
        COLLECTOR_END_INSTR(JumpFalse)

        COLLECTOR_BEGIN_INSTR(JumpNoException)
            addLabel(code - start + offset);
        COLLECTOR_END_INSTR(JumpNoException)

        COLLECTOR_BEGIN_INSTR(JumpNotUndefined)
            addLabel(code - start + offset);
        COLLECTOR_END_INSTR(JumpNotUndefined)

        COLLECTOR_BEGIN_INSTR(CmpEqNull)
        COLLECTOR_END_INSTR(CmpEqNull)

        COLLECTOR_BEGIN_INSTR(CmpNeNull)
        COLLECTOR_END_INSTR(CmpNeNull)

        COLLECTOR_BEGIN_INSTR(CmpEqInt)
        COLLECTOR_END_INSTR(CmpEq)

        COLLECTOR_BEGIN_INSTR(CmpNeInt)
        COLLECTOR_END_INSTR(CmpNeInt)

        COLLECTOR_BEGIN_INSTR(CmpEq)
        COLLECTOR_END_INSTR(CmpEq)

        COLLECTOR_BEGIN_INSTR(CmpNe)
        COLLECTOR_END_INSTR(CmpNe)

        COLLECTOR_BEGIN_INSTR(CmpGt)
        COLLECTOR_END_INSTR(CmpGt)

        COLLECTOR_BEGIN_INSTR(CmpGe)
        COLLECTOR_END_INSTR(CmpGe)

        COLLECTOR_BEGIN_INSTR(CmpLt)
        COLLECTOR_END_INSTR(CmpLt)

        COLLECTOR_BEGIN_INSTR(CmpLe)
        COLLECTOR_END_INSTR(CmpLe)

        COLLECTOR_BEGIN_INSTR(CmpStrictEqual)
        COLLECTOR_END_INSTR(CmpStrictEqual)

        COLLECTOR_BEGIN_INSTR(CmpStrictNotEqual)
        COLLECTOR_END_INSTR(CmpStrictNotEqual)

        COLLECTOR_BEGIN_INSTR(CmpIn)
        COLLECTOR_END_INSTR(CmpIn)

        COLLECTOR_BEGIN_INSTR(CmpInstanceOf)
        COLLECTOR_END_INSTR(CmpInstanceOf)

        COLLECTOR_BEGIN_INSTR(UNot)
        COLLECTOR_END_INSTR(UNot)

        COLLECTOR_BEGIN_INSTR(UPlus)
        COLLECTOR_END_INSTR(UPlus)

        COLLECTOR_BEGIN_INSTR(UMinus)
        COLLECTOR_END_INSTR(UMinus)

        COLLECTOR_BEGIN_INSTR(UCompl)
        COLLECTOR_END_INSTR(UCompl)

        COLLECTOR_BEGIN_INSTR(Increment)
        COLLECTOR_END_INSTR(PreIncrement)

        COLLECTOR_BEGIN_INSTR(Decrement)
        COLLECTOR_END_INSTR(PreDecrement)

        COLLECTOR_BEGIN_INSTR(Add)
        COLLECTOR_END_INSTR(Add)

        COLLECTOR_BEGIN_INSTR(BitAnd)
        COLLECTOR_END_INSTR(BitAnd)

        COLLECTOR_BEGIN_INSTR(BitOr)
        COLLECTOR_END_INSTR(BitOr)

        COLLECTOR_BEGIN_INSTR(BitXor)
        COLLECTOR_END_INSTR(BitXor)

        COLLECTOR_BEGIN_INSTR(UShr)
        COLLECTOR_END_INSTR(UShr)

        COLLECTOR_BEGIN_INSTR(Shr)
        COLLECTOR_END_INSTR(Shr)

        COLLECTOR_BEGIN_INSTR(Shl)
        COLLECTOR_END_INSTR(Shl)

        COLLECTOR_BEGIN_INSTR(BitAndConst)
        COLLECTOR_END_INSTR(BitAndConst)

        COLLECTOR_BEGIN_INSTR(BitOrConst)
        COLLECTOR_END_INSTR(BitOr)

        COLLECTOR_BEGIN_INSTR(BitXorConst)
        COLLECTOR_END_INSTR(BitXor)

        COLLECTOR_BEGIN_INSTR(UShrConst)
        COLLECTOR_END_INSTR(UShrConst)

        COLLECTOR_BEGIN_INSTR(ShrConst)
        COLLECTOR_END_INSTR(ShrConst)

        COLLECTOR_BEGIN_INSTR(ShlConst)
        COLLECTOR_END_INSTR(ShlConst)

        COLLECTOR_BEGIN_INSTR(Exp)
        COLLECTOR_END_INSTR(Exp)

        COLLECTOR_BEGIN_INSTR(Mul)
        COLLECTOR_END_INSTR(Mul)

        COLLECTOR_BEGIN_INSTR(Div)
        COLLECTOR_END_INSTR(Div)

        COLLECTOR_BEGIN_INSTR(Mod)
        COLLECTOR_END_INSTR(Mod)

        COLLECTOR_BEGIN_INSTR(Sub)
        COLLECTOR_END_INSTR(Sub)

        COLLECTOR_BEGIN_INSTR(Ret)
        COLLECTOR_END_INSTR(Ret)

#ifndef QT_NO_QML_DEBUGGER
        COLLECTOR_BEGIN_INSTR(Debug)
        COLLECTOR_END_INSTR(Debug)
#endif // QT_NO_QML_DEBUGGER

        COLLECTOR_BEGIN_INSTR(InitializeBlockDeadTemporalZone)
        COLLECTOR_END_INSTR(InitializeBlockDeadTemporalZone)

        COLLECTOR_BEGIN_INSTR(ThrowOnNullOrUndefined)
        COLLECTOR_END_INSTR(ThrowOnNullOrUndefined)

        COLLECTOR_BEGIN_INSTR(GetTemplateObject)
        COLLECTOR_END_INSTR(GetTemplateObject)

        COLLECTOR_BEGIN_INSTR(TailCall)
        COLLECTOR_END_INSTR(TailCall)
    }

    return labels;
}

#undef COLLECTOR_BEGIN_INSTR
#undef COLLECTOR_END_INSTR
