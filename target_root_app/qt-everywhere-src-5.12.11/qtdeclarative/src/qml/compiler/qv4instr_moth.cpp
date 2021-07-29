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

#include "qv4instr_moth_p.h"
#include <private/qv4compileddata_p.h>
#include <private/qv4stackframe_p.h>

using namespace QV4;
using namespace QV4::Moth;

int InstrInfo::size(Instr::Type type)
{
#define MOTH_RETURN_INSTR_SIZE(I) case Instr::Type::I: case Instr::Type::I##_Wide: return InstrMeta<int(Instr::Type::I)>::Size;
    switch (type) {
    FOR_EACH_MOTH_INSTR_ALL(MOTH_RETURN_INSTR_SIZE)
    }
#undef MOTH_RETURN_INSTR_SIZE
    Q_UNREACHABLE();
}

static QByteArray alignedNumber(int n) {
    QByteArray number = QByteArray::number(n);
    while (number.size() < 8)
        number.prepend(' ');
    return number;
}

static QByteArray alignedLineNumber(int line) {
    if (line > 0)
        return alignedNumber(static_cast<int>(line));
    return QByteArray("        ");
}

static QByteArray rawBytes(const char *data, int n)
{
    QByteArray ba;
    while (n) {
        uint num = *reinterpret_cast<const uchar *>(data);
        if (num < 16)
            ba += '0';
        ba += QByteArray::number(num, 16) + " ";
        ++data;
        --n;
    }
    while (ba.size() < 25)
        ba += ' ';
    return ba;
}

static QString toString(QV4::ReturnedValue v)
{
#ifdef V4_BOOTSTRAP
    return QStringLiteral("string-const(%1)").arg(v);
#else // !V4_BOOTSTRAP
    Value val = Value::fromReturnedValue(v);
    QString result;
    if (val.isInt32())
        result = QLatin1String("int ");
    else if (val.isDouble())
        result = QLatin1String("double ");
    if (val.isEmpty())
        result += QLatin1String("empty");
    else
        result += val.toQStringNoThrow();
    return result;
#endif // V4_BOOTSTRAP
}

#define ABSOLUTE_OFFSET() \
    (code - start + offset)

#define MOTH_BEGIN_INSTR(instr) \
    { \
        INSTR_##instr(MOTH_DECODE_WITH_BASE) \
        QDebug d = qDebug(); \
        d.noquote(); \
        d.nospace(); \
        if (static_cast<int>(Instr::Type::instr) >= 0x100) \
            --base_ptr; \
        d << alignedLineNumber(line) << alignedNumber(codeOffset).constData() << ": " \
          << rawBytes(base_ptr, int(code - base_ptr)) << #instr << " ";

#define MOTH_END_INSTR(instr) \
        continue; \
    }

QT_BEGIN_NAMESPACE
namespace QV4 {
namespace Moth {

const int InstrInfo::argumentCount[] = {
    FOR_EACH_MOTH_INSTR_ALL(MOTH_COLLECT_NARGS)
};


void dumpConstantTable(const Value *constants, uint count)
{
    QDebug d = qDebug();
    d.nospace();
    for (uint i = 0; i < count; ++i)
        d << alignedNumber(int(i)).constData() << ":    "
          << toString(constants[i].asReturnedValue()).toUtf8().constData() << "\n";
}

QString dumpRegister(int reg, int nFormals)
{
    Q_STATIC_ASSERT(offsetof(CallData, function) == 0);
    Q_STATIC_ASSERT(offsetof(CallData, context) == sizeof(Value));
    Q_STATIC_ASSERT(offsetof(CallData, accumulator) == 2*sizeof(Value));
    Q_STATIC_ASSERT(offsetof(CallData, thisObject) == 3*sizeof(Value));
    if (reg == CallData::Function)
        return QStringLiteral("(function)");
    else if (reg == CallData::Context)
        return QStringLiteral("(context)");
    else if (reg == CallData::Accumulator)
        return QStringLiteral("(accumulator)");
    else if (reg == CallData::NewTarget)
        return QStringLiteral("(new.target)");
    else if (reg == CallData::This)
        return QStringLiteral("(this)");
    else if (reg == CallData::Argc)
        return QStringLiteral("(argc)");
    reg -= CallData::OffsetCount;
    if (reg < nFormals)
        return QStringLiteral("a%1").arg(reg);
    reg -= nFormals;
    return QStringLiteral("r%1").arg(reg);

}

QString dumpArguments(int argc, int argv, int nFormals)
{
    if (!argc)
        return QStringLiteral("()");
    return QStringLiteral("(") + dumpRegister(argv, nFormals) + QStringLiteral(", ") + QString::number(argc) + QStringLiteral(")");
}


void dumpBytecode(const char *code, int len, int nLocals, int nFormals, int /*startLine*/, const QVector<CompiledData::CodeOffsetToLine> &lineNumberMapping)
{
    MOTH_JUMP_TABLE;

    auto findLine = [](const CompiledData::CodeOffsetToLine &entry, uint offset) {
        return entry.codeOffset < offset;
    };

    int lastLine = -1;
    const char *start = code;
    const char *end = code + len;
    while (code < end) {
        const CompiledData::CodeOffsetToLine *codeToLine = std::lower_bound(lineNumberMapping.constBegin(), lineNumberMapping.constEnd(), static_cast<uint>(code - start) + 1, findLine) - 1;
        int line = int(codeToLine->line);
        if (line != lastLine)
            lastLine = line;
        else
            line = -1;

        int codeOffset = int(code - start);

        MOTH_DISPATCH()

        MOTH_BEGIN_INSTR(LoadReg)
            d << dumpRegister(reg, nFormals);
        MOTH_END_INSTR(LoadReg)

        MOTH_BEGIN_INSTR(StoreReg)
            d << dumpRegister(reg, nFormals);
        MOTH_END_INSTR(StoreReg)

        MOTH_BEGIN_INSTR(MoveReg)
            d << dumpRegister(destReg, nFormals) << ", " << dumpRegister(srcReg, nFormals);
        MOTH_END_INSTR(MoveReg)

        MOTH_BEGIN_INSTR(LoadImport)
            d << "i" << index;
        MOTH_END_INSTR(LoadImport)

        MOTH_BEGIN_INSTR(LoadConst)
            d << "C" << index;
        MOTH_END_INSTR(LoadConst)

        MOTH_BEGIN_INSTR(LoadNull)
        MOTH_END_INSTR(LoadNull)

        MOTH_BEGIN_INSTR(LoadZero)
        MOTH_END_INSTR(LoadZero)

        MOTH_BEGIN_INSTR(LoadTrue)
        MOTH_END_INSTR(LoadTrue)

        MOTH_BEGIN_INSTR(LoadFalse)
        MOTH_END_INSTR(LoadFalse)

        MOTH_BEGIN_INSTR(LoadUndefined)
        MOTH_END_INSTR(LoadUndefined)

        MOTH_BEGIN_INSTR(LoadInt)
            d << value;
        MOTH_END_INSTR(LoadInt)

        MOTH_BEGIN_INSTR(MoveConst)
            d << dumpRegister(destTemp, nFormals) << ", C" << constIndex;
        MOTH_END_INSTR(MoveConst)

        MOTH_BEGIN_INSTR(LoadLocal)
            if (index < nLocals)
                d << "l" << index;
            else
                d << "a" << (index - nLocals);
        MOTH_END_INSTR(LoadLocal)

        MOTH_BEGIN_INSTR(StoreLocal)
            if (index < nLocals)
                d << "l" << index;
            else
                d << "a" << (index - nLocals);
        MOTH_END_INSTR(StoreLocal)

        MOTH_BEGIN_INSTR(LoadScopedLocal)
            if (index < nLocals)
                d << "l" << index << "@" << scope;
            else
                d << "a" << (index - nLocals) << "@" << scope;
        MOTH_END_INSTR(LoadScopedLocal)

        MOTH_BEGIN_INSTR(StoreScopedLocal)
            if (index < nLocals)
                d << ", " << "l" << index << "@" << scope;
            else
                d << ", " << "a" << (index - nLocals) << "@" << scope;
        MOTH_END_INSTR(StoreScopedLocal)

        MOTH_BEGIN_INSTR(LoadRuntimeString)
            d << stringId;
        MOTH_END_INSTR(LoadRuntimeString)

        MOTH_BEGIN_INSTR(MoveRegExp)
            d << dumpRegister(destReg, nFormals) << ", " <<regExpId;
        MOTH_END_INSTR(MoveRegExp)

        MOTH_BEGIN_INSTR(LoadClosure)
            d << value;
        MOTH_END_INSTR(LoadClosure)

        MOTH_BEGIN_INSTR(LoadName)
            d << name;
        MOTH_END_INSTR(LoadName)

        MOTH_BEGIN_INSTR(LoadGlobalLookup)
            d << index;
        MOTH_END_INSTR(LoadGlobalLookup)

        MOTH_BEGIN_INSTR(LoadQmlContextPropertyLookup)
            d << index;
        MOTH_END_INSTR(LoadQmlContextPropertyLookup)

        MOTH_BEGIN_INSTR(StoreNameSloppy)
            d << name;
        MOTH_END_INSTR(StoreNameSloppy)

        MOTH_BEGIN_INSTR(StoreNameStrict)
            d << name;
        MOTH_END_INSTR(StoreNameStrict)

        MOTH_BEGIN_INSTR(LoadElement)
            d << dumpRegister(base, nFormals) << "[acc]";
        MOTH_END_INSTR(LoadElement)

        MOTH_BEGIN_INSTR(StoreElement)
            d << dumpRegister(base, nFormals) << "[" << dumpRegister(index, nFormals) << "]";
        MOTH_END_INSTR(StoreElement)

        MOTH_BEGIN_INSTR(LoadProperty)
            d << "acc[" << name << "]";
        MOTH_END_INSTR(LoadProperty)

        MOTH_BEGIN_INSTR(GetLookup)
            d << "acc(" << index << ")";
        MOTH_END_INSTR(GetLookup)

        MOTH_BEGIN_INSTR(StoreProperty)
            d << dumpRegister(base, nFormals) << "[" << name<< "]";
        MOTH_END_INSTR(StoreProperty)

        MOTH_BEGIN_INSTR(SetLookup)
            d << dumpRegister(base, nFormals) << "(" << index << ")";
        MOTH_END_INSTR(SetLookup)

        MOTH_BEGIN_INSTR(LoadSuperProperty)
            d << dumpRegister(property, nFormals);
        MOTH_END_INSTR(LoadSuperProperty)

        MOTH_BEGIN_INSTR(StoreSuperProperty)
            d << dumpRegister(property, nFormals);
        MOTH_END_INSTR(StoreSuperProperty)

        MOTH_BEGIN_INSTR(Yield)
        MOTH_END_INSTR(Yield)

        MOTH_BEGIN_INSTR(YieldStar)
        MOTH_END_INSTR(YieldStar)

        MOTH_BEGIN_INSTR(Resume)
            d << ABSOLUTE_OFFSET();
        MOTH_END_INSTR(Resume)

        MOTH_BEGIN_INSTR(CallValue)
            d << dumpRegister(name, nFormals) << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(CallValue)

        MOTH_BEGIN_INSTR(CallWithReceiver)
            d << dumpRegister(name, nFormals) << dumpRegister(thisObject, nFormals) << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(CallWithReceiver)

        MOTH_BEGIN_INSTR(CallProperty)
            d << dumpRegister(base, nFormals) << "." << name << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(CallProperty)

        MOTH_BEGIN_INSTR(CallPropertyLookup)
            d << dumpRegister(base, nFormals) << "." << lookupIndex << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(CallPropertyLookup)

        MOTH_BEGIN_INSTR(CallElement)
            d << dumpRegister(base, nFormals) << "[" << dumpRegister(index, nFormals) << "]" << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(CallElement)

        MOTH_BEGIN_INSTR(CallName)
            d << name << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(CallName)

        MOTH_BEGIN_INSTR(CallPossiblyDirectEval)
            d << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(CallPossiblyDirectEval)

        MOTH_BEGIN_INSTR(CallGlobalLookup)
            d << index << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(CallGlobalLookup)

        MOTH_BEGIN_INSTR(CallQmlContextPropertyLookup)
            d << index << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(CallQmlContextPropertyLookup)

        MOTH_BEGIN_INSTR(CallWithSpread)
            d << "new" << dumpRegister(func, nFormals) << dumpRegister(thisObject, nFormals) << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(CallWithSpread)

        MOTH_BEGIN_INSTR(Construct)
            d << "new" << dumpRegister(func, nFormals) << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(Construct)

        MOTH_BEGIN_INSTR(ConstructWithSpread)
            d << "new" << dumpRegister(func, nFormals) << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(ConstructWithSpread)

        MOTH_BEGIN_INSTR(SetUnwindHandler)
            if (offset)
                d << ABSOLUTE_OFFSET();
            else
                d << "<null>";
        MOTH_END_INSTR(SetUnwindHandler)

        MOTH_BEGIN_INSTR(UnwindDispatch)
        MOTH_END_INSTR(UnwindDispatch)

        MOTH_BEGIN_INSTR(UnwindToLabel)
                d << "(" << level << ") " << ABSOLUTE_OFFSET();
        MOTH_END_INSTR(UnwindToLabel)

        MOTH_BEGIN_INSTR(DeadTemporalZoneCheck)
                d << name;
        MOTH_END_INSTR(DeadTemporalZoneCheck)

        MOTH_BEGIN_INSTR(ThrowException)
        MOTH_END_INSTR(ThrowException)

        MOTH_BEGIN_INSTR(GetException)
        MOTH_END_INSTR(HasException)

        MOTH_BEGIN_INSTR(SetException)
        MOTH_END_INSTR(SetExceptionFlag)

        MOTH_BEGIN_INSTR(CreateCallContext)
        MOTH_END_INSTR(CreateCallContext)

        MOTH_BEGIN_INSTR(PushCatchContext)
            d << index << ", " << name;
        MOTH_END_INSTR(PushCatchContext)

        MOTH_BEGIN_INSTR(PushWithContext)
        MOTH_END_INSTR(PushWithContext)

        MOTH_BEGIN_INSTR(PushBlockContext)
            d << index;
        MOTH_END_INSTR(PushBlockContext)

        MOTH_BEGIN_INSTR(CloneBlockContext)
        MOTH_END_INSTR(CloneBlockContext)

        MOTH_BEGIN_INSTR(PushScriptContext)
            d << index;
        MOTH_END_INSTR(PushScriptContext)

        MOTH_BEGIN_INSTR(PopScriptContext)
        MOTH_END_INSTR(PopScriptContext)

        MOTH_BEGIN_INSTR(PopContext)
        MOTH_END_INSTR(PopContext)

        MOTH_BEGIN_INSTR(GetIterator)
            d << iterator;
        MOTH_END_INSTR(GetIterator)

        MOTH_BEGIN_INSTR(IteratorNext)
            d << dumpRegister(value, nFormals) << ", " << dumpRegister(done, nFormals);
        MOTH_END_INSTR(IteratorNext)

        MOTH_BEGIN_INSTR(IteratorNextForYieldStar)
            d << dumpRegister(iterator, nFormals) << ", " << dumpRegister(object, nFormals);
        MOTH_END_INSTR(IteratorNextForYieldStar)

        MOTH_BEGIN_INSTR(IteratorClose)
            d << dumpRegister(done, nFormals);
        MOTH_END_INSTR(IteratorClose)

        MOTH_BEGIN_INSTR(DestructureRestElement)
        MOTH_END_INSTR(DestructureRestElement)

        MOTH_BEGIN_INSTR(DeleteProperty)
            d << dumpRegister(base, nFormals) << "[" << dumpRegister(index, nFormals) << "]";
        MOTH_END_INSTR(DeleteProperty)

        MOTH_BEGIN_INSTR(DeleteName)
            d << name;
        MOTH_END_INSTR(DeleteName)

        MOTH_BEGIN_INSTR(TypeofName)
            d << name;
        MOTH_END_INSTR(TypeofName)

        MOTH_BEGIN_INSTR(TypeofValue)
        MOTH_END_INSTR(TypeofValue)

        MOTH_BEGIN_INSTR(DeclareVar)
            d << isDeletable << ", " << varName;
        MOTH_END_INSTR(DeclareVar)

        MOTH_BEGIN_INSTR(DefineArray)
            d << dumpRegister(args, nFormals) << ", " << argc;
        MOTH_END_INSTR(DefineArray)

        MOTH_BEGIN_INSTR(DefineObjectLiteral)
            d << internalClassId
              << ", " << argc
              << ", " << dumpRegister(args, nFormals);
        MOTH_END_INSTR(DefineObjectLiteral)

        MOTH_BEGIN_INSTR(CreateClass)
            d << classIndex
              << ", " << dumpRegister(heritage, nFormals)
              << ", " << dumpRegister(computedNames, nFormals);
        MOTH_END_INSTR(CreateClass)

        MOTH_BEGIN_INSTR(CreateMappedArgumentsObject)
        MOTH_END_INSTR(CreateMappedArgumentsObject)

        MOTH_BEGIN_INSTR(CreateUnmappedArgumentsObject)
        MOTH_END_INSTR(CreateUnmappedArgumentsObject)

        MOTH_BEGIN_INSTR(CreateRestParameter)
            d << argIndex;
        MOTH_END_INSTR(CreateRestParameter)

        MOTH_BEGIN_INSTR(ConvertThisToObject)
        MOTH_END_INSTR(ConvertThisToObject)

        MOTH_BEGIN_INSTR(LoadSuperConstructor)
        MOTH_END_INSTR(LoadSuperConstructor)

        MOTH_BEGIN_INSTR(ToObject)
        MOTH_END_INSTR(ToObject)

        MOTH_BEGIN_INSTR(Jump)
            d << ABSOLUTE_OFFSET();
        MOTH_END_INSTR(Jump)

        MOTH_BEGIN_INSTR(JumpTrue)
            d << ABSOLUTE_OFFSET();
        MOTH_END_INSTR(JumpTrue)

        MOTH_BEGIN_INSTR(JumpFalse)
            d << ABSOLUTE_OFFSET();
        MOTH_END_INSTR(JumpFalse)

        MOTH_BEGIN_INSTR(JumpNotUndefined)
            d << ABSOLUTE_OFFSET();
        MOTH_END_INSTR(JumpNotUndefined)

        MOTH_BEGIN_INSTR(JumpNoException)
            d << ABSOLUTE_OFFSET();
        MOTH_END_INSTR(JumpNoException)

        MOTH_BEGIN_INSTR(CmpEqNull)
        MOTH_END_INSTR(CmpEqNull)

        MOTH_BEGIN_INSTR(CmpNeNull)
        MOTH_END_INSTR(CmpNeNull)

        MOTH_BEGIN_INSTR(CmpEqInt)
            d << lhs;
        MOTH_END_INSTR(CmpEq)

        MOTH_BEGIN_INSTR(CmpNeInt)
                d << lhs;
        MOTH_END_INSTR(CmpNeInt)

        MOTH_BEGIN_INSTR(CmpEq)
            d << dumpRegister(lhs, nFormals);
        MOTH_END_INSTR(CmpEq)

        MOTH_BEGIN_INSTR(CmpNe)
                d << dumpRegister(lhs, nFormals);
        MOTH_END_INSTR(CmpNe)

        MOTH_BEGIN_INSTR(CmpGt)
                d << dumpRegister(lhs, nFormals);
        MOTH_END_INSTR(CmpGt)

        MOTH_BEGIN_INSTR(CmpGe)
                d << dumpRegister(lhs, nFormals);
        MOTH_END_INSTR(CmpGe)

        MOTH_BEGIN_INSTR(CmpLt)
                d << dumpRegister(lhs, nFormals);
        MOTH_END_INSTR(CmpLt)

        MOTH_BEGIN_INSTR(CmpLe)
                d << dumpRegister(lhs, nFormals);
        MOTH_END_INSTR(CmpLe)

        MOTH_BEGIN_INSTR(CmpStrictEqual)
            d << dumpRegister(lhs, nFormals);
        MOTH_END_INSTR(CmpStrictEqual)

        MOTH_BEGIN_INSTR(CmpStrictNotEqual)
            d << dumpRegister(lhs, nFormals);
        MOTH_END_INSTR(CmpStrictNotEqual)

        MOTH_BEGIN_INSTR(UNot)
        MOTH_END_INSTR(UNot)

        MOTH_BEGIN_INSTR(UPlus)
        MOTH_END_INSTR(UPlus)

        MOTH_BEGIN_INSTR(UMinus)
        MOTH_END_INSTR(UMinus)

        MOTH_BEGIN_INSTR(UCompl)
        MOTH_END_INSTR(UCompl)

        MOTH_BEGIN_INSTR(Increment)
        MOTH_END_INSTR(PreIncrement)

        MOTH_BEGIN_INSTR(Decrement)
        MOTH_END_INSTR(PreDecrement)

        MOTH_BEGIN_INSTR(Add)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(Add)

        MOTH_BEGIN_INSTR(BitAnd)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(BitAnd)

        MOTH_BEGIN_INSTR(BitOr)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(BitOr)

        MOTH_BEGIN_INSTR(BitXor)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(BitXor)

        MOTH_BEGIN_INSTR(UShr)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(UShr)

        MOTH_BEGIN_INSTR(Shr)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(Shr)

        MOTH_BEGIN_INSTR(Shl)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(Shl)

        MOTH_BEGIN_INSTR(BitAndConst)
            d << "acc, " << rhs;
        MOTH_END_INSTR(BitAndConst)

        MOTH_BEGIN_INSTR(BitOrConst)
            d << "acc, " << rhs;
        MOTH_END_INSTR(BitOr)

        MOTH_BEGIN_INSTR(BitXorConst)
            d << "acc, " << rhs;
        MOTH_END_INSTR(BitXor)

        MOTH_BEGIN_INSTR(UShrConst)
            d << "acc, " << rhs;
        MOTH_END_INSTR(UShrConst)

        MOTH_BEGIN_INSTR(ShrConst)
            d << "acc, " << rhs;
        MOTH_END_INSTR(ShrConst)

        MOTH_BEGIN_INSTR(ShlConst)
            d << "acc, " << rhs;
        MOTH_END_INSTR(ShlConst)

        MOTH_BEGIN_INSTR(Exp)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(Exp)

        MOTH_BEGIN_INSTR(Mul)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(Mul)

        MOTH_BEGIN_INSTR(Div)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(Div)

        MOTH_BEGIN_INSTR(Mod)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(Mod)

        MOTH_BEGIN_INSTR(Sub)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(Sub)

        MOTH_BEGIN_INSTR(CmpIn)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(CmpIn)

        MOTH_BEGIN_INSTR(CmpInstanceOf)
            d << dumpRegister(lhs, nFormals) << ", acc";
        MOTH_END_INSTR(CmpInstanceOf)

        MOTH_BEGIN_INSTR(Ret)
        MOTH_END_INSTR(Ret)

        MOTH_BEGIN_INSTR(Debug)
        MOTH_END_INSTR(Debug)

        MOTH_BEGIN_INSTR(InitializeBlockDeadTemporalZone)
            d << dumpRegister(firstReg, nFormals) << ", " << count;
        MOTH_END_INSTR(InitializeBlockDeadTemporalZone)

        MOTH_BEGIN_INSTR(ThrowOnNullOrUndefined)
        MOTH_END_INSTR(ThrowOnNullOrUndefined)

        MOTH_BEGIN_INSTR(GetTemplateObject)
            d << index;
        MOTH_END_INSTR(GetTemplateObject)

        MOTH_BEGIN_INSTR(TailCall)
            d << dumpRegister(func, nFormals) << dumpRegister(thisObject, nFormals) << dumpArguments(argc, argv, nFormals);
        MOTH_END_INSTR(TailCall)
    }
}

}
}
QT_END_NAMESPACE
