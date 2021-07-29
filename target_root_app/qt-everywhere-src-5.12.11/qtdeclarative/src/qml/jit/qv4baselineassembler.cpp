/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include <QBuffer>
#include <QFile>

#include "qv4engine_p.h"
#include "qv4baselineassembler_p.h"
#include "qv4assemblercommon_p.h"
#include <private/qv4function_p.h>
#include <private/qv4runtime_p.h>
#include <private/qv4stackframe_p.h>

#include <wtf/Vector.h>
#include <assembler/MacroAssembler.h>
#include <assembler/MacroAssemblerCodeRef.h>
#include <assembler/LinkBuffer.h>
#include <WTFStubs.h>

#undef ENABLE_ALL_ASSEMBLERS_FOR_REFACTORING_PURPOSES

#ifdef V4_ENABLE_JIT

QT_BEGIN_NAMESPACE
namespace QV4 {
namespace JIT {

#define ASM_GENERATE_RUNTIME_CALL(function, destination) \
    pasm()->GENERATE_RUNTIME_CALL(function, destination)
#define callHelper(x) PlatformAssemblerCommon::callRuntimeUnchecked(#x, reinterpret_cast<void *>(&x))

const QV4::Value::ValueTypeInternal IntegerTag = QV4::Value::ValueTypeInternal::Integer;

static ReturnedValue toNumberHelper(ReturnedValue v)
{
    return Encode(Value::fromReturnedValue(v).toNumber());
}

static ReturnedValue toInt32Helper(ReturnedValue v)
{
    return Encode(Value::fromReturnedValue(v).toInt32());
}

#if QT_POINTER_SIZE == 8 || defined(ENABLE_ALL_ASSEMBLERS_FOR_REFACTORING_PURPOSES)
class PlatformAssembler64 : public PlatformAssemblerCommon
{
public:
    PlatformAssembler64(const Value *constantTable)
        : PlatformAssemblerCommon(constantTable)
    {}

    void callRuntime(const char *functionName, const void *funcPtr,
                     CallResultDestination dest)
    {
        PlatformAssemblerCommon::callRuntime(functionName, funcPtr);
        if (dest == CallResultDestination::InAccumulator)
            saveReturnValueInAccumulator();
        else if (AccumulatorRegister == ReturnValueRegister)
            loadUndefined();
    }

    void saveReturnValueInAccumulator()
    {
        move(ReturnValueRegister, AccumulatorRegister);
    }

    void loadUndefined(RegisterID dest = AccumulatorRegister)
    {
        move(TrustedImm64(0), dest);
    }

    void copyConst(int constIndex, Address dest)
    {
        //###
        if (constant(constIndex).isUndefined()) {
            loadUndefined(ScratchRegister);
        } else {
            load64(loadConstAddress(constIndex, ScratchRegister), ScratchRegister);
        }
        store64(ScratchRegister, dest);
    }

    void copyReg(Address src, Address dst)
    {
        load64(src, ScratchRegister);
        store64(ScratchRegister, dst);
    }

    void loadPointerFromValue(Address addr, RegisterID dest = AccumulatorRegister)
    {
        load64(addr, dest);
    }

    void loadAccumulator(Address addr)
    {
        load64(addr, AccumulatorRegister);
    }

    void storeAccumulator(Address addr)
    {
        store64(AccumulatorRegister, addr);
    }

    void moveReg(Address sourceRegAddress, Address destRegAddress)
    {
        load64(sourceRegAddress, ScratchRegister);
        store64(ScratchRegister, destRegAddress);
    }

    void loadString(int stringId)
    {
        loadAccumulator(loadStringAddress(stringId));
    }

    void loadValue(ReturnedValue value)
    {
        move(TrustedImm64(value), AccumulatorRegister);
    }

    void storeHeapObject(RegisterID source, Address addr)
    {
        store64(source, addr);
    }

    void generateCatchTrampoline()
    {
        PlatformAssemblerCommon::generateCatchTrampoline([this](){loadUndefined();});
    }

    void jumpNotUndefined(int offset)
    {
        auto jump = branch64(NotEqual, AccumulatorRegister, TrustedImm64(0));
        addJumpToOffset(jump, offset);
    }

    Jump jumpEmpty()
    {
        return branch64(Equal, AccumulatorRegister, TrustedImm64(Value::emptyValue().asReturnedValue()));
    }

    Jump jumpNotEmpty()
    {
        return branch64(NotEqual, AccumulatorRegister, TrustedImm64(Value::emptyValue().asReturnedValue()));
    }

    void toBoolean(std::function<void(RegisterID)> continuation)
    {
        urshift64(AccumulatorRegister, TrustedImm32(Value::IsIntegerConvertible_Shift), ScratchRegister);
        auto needsConversion = branch32(NotEqual, TrustedImm32(1), ScratchRegister);
        continuation(AccumulatorRegister);
        Jump done = jump();

        // slow path:
        needsConversion.link(this);
        push(AccumulatorRegister);
        move(AccumulatorRegister, registerForArg(0));
        callHelper(Value::toBooleanImpl);
        and32(TrustedImm32(1), ReturnValueRegister, ScratchRegister);
        pop(AccumulatorRegister);
        continuation(ScratchRegister);

        done.link(this);
    }

    void toNumber()
    {
        urshift64(AccumulatorRegister, TrustedImm32(Value::QuickType_Shift), ScratchRegister);
        auto isNumber = branch32(GreaterThanOrEqual, ScratchRegister, TrustedImm32(Value::QT_Int));

        move(AccumulatorRegister, registerForArg(0));
        callHelper(toNumberHelper);
        saveReturnValueInAccumulator();

        isNumber.link(this);
    }

    // this converts both the lhs and the accumulator to int32
    void toInt32LhsAcc(Address lhs, RegisterID lhsTarget)
    {
        load64(lhs, lhsTarget);
        urshift64(lhsTarget, TrustedImm32(Value::QuickType_Shift), ScratchRegister2);
        auto lhsIsInt = branch32(Equal, TrustedImm32(Value::QT_Int), ScratchRegister2);

        const Address accumulatorStackAddress(JSStackFrameRegister,
                                              offsetof(CallData, accumulator));
        storeAccumulator(accumulatorStackAddress);
        move(lhsTarget, registerForArg(0));
        callHelper(toInt32Helper);
        move(ReturnValueRegister, lhsTarget);
        loadAccumulator(accumulatorStackAddress);

        lhsIsInt.link(this);
        urshift64(AccumulatorRegister, TrustedImm32(Value::QuickType_Shift), ScratchRegister2);
        auto isInt = branch32(Equal, TrustedImm32(Value::QT_Int), ScratchRegister2);

        pushAligned(lhsTarget);
        move(AccumulatorRegister, registerForArg(0));
        callHelper(toInt32Helper);
        saveReturnValueInAccumulator();
        popAligned(lhsTarget);

        isInt.link(this);
    }

    void toInt32()
    {
        urshift64(AccumulatorRegister, TrustedImm32(Value::QuickType_Shift), ScratchRegister2);
        auto isInt = branch32(Equal, TrustedImm32(Value::QT_Int), ScratchRegister2);

        move(AccumulatorRegister, registerForArg(0));
        callRuntimeUnchecked("toInt32Helper", reinterpret_cast<void *>(&toInt32Helper));
        saveReturnValueInAccumulator();

        isInt.link(this);
    }

    void regToInt32(Address srcReg, RegisterID targetReg)
    {
        load64(srcReg, targetReg);
        urshift64(targetReg, TrustedImm32(Value::QuickType_Shift), ScratchRegister2);
        auto isInt = branch32(Equal, TrustedImm32(Value::QT_Int), ScratchRegister2);

        pushAligned(AccumulatorRegister);
        move(targetReg, registerForArg(0));
        callHelper(toInt32Helper);
        move(ReturnValueRegister, targetReg);
        popAligned(AccumulatorRegister);

        isInt.link(this);
    }

    void isNullOrUndefined()
    {
        move(AccumulatorRegister, ScratchRegister);
        compare64(Equal, ScratchRegister, TrustedImm32(0), AccumulatorRegister);
        Jump isUndef = branch32(NotEqual, TrustedImm32(0), AccumulatorRegister);

        // not undefined
        rshift64(TrustedImm32(32), ScratchRegister);
        compare32(Equal, ScratchRegister, TrustedImm32(int(QV4::Value::ValueTypeInternal::Null)),
                  AccumulatorRegister);

        isUndef.link(this);
    }

    Jump isIntOrBool()
    {
        urshift64(AccumulatorRegister, TrustedImm32(Value::IsIntegerOrBool_Shift), ScratchRegister);
        return branch32(Equal, TrustedImm32(3), ScratchRegister);
    }

    void jumpStrictEqualStackSlotInt(int lhs, int rhs, int offset)
    {
        Address lhsAddr(JSStackFrameRegister, lhs * int(sizeof(Value)));
        load64(lhsAddr, ScratchRegister);
        Jump isUndef = branch64(Equal, ScratchRegister, TrustedImm64(0));
        Jump equal = branch32(Equal, TrustedImm32(rhs), ScratchRegister);
        addJumpToOffset(equal, offset);
        isUndef.link(this);
    }

    void jumpStrictNotEqualStackSlotInt(int lhs, int rhs, int offset)
    {
        Address lhsAddr(JSStackFrameRegister, lhs * int(sizeof(Value)));
        load64(lhsAddr, ScratchRegister);
        Jump isUndef = branch64(Equal, ScratchRegister, TrustedImm64(0));
        addJumpToOffset(isUndef, offset);
        Jump notEqual = branch32(NotEqual, TrustedImm32(rhs), ScratchRegister);
        addJumpToOffset(notEqual, offset);
    }

    void setAccumulatorTag(QV4::Value::ValueTypeInternal tag, RegisterID sourceReg = NoRegister)
    {
        if (sourceReg == NoRegister)
            or64(TrustedImm64(int64_t(tag) << 32), AccumulatorRegister);
        else
            or64(TrustedImm64(int64_t(tag) << 32), sourceReg, AccumulatorRegister);
    }

    void encodeDoubleIntoAccumulator(FPRegisterID src)
    {
        moveDoubleTo64(src, AccumulatorRegister);
        move(TrustedImm64(Value::NaNEncodeMask), ScratchRegister);
        xor64(ScratchRegister, AccumulatorRegister);
    }

    void pushValueAligned(ReturnedValue v)
    {
        loadValue(v);
        pushAligned(AccumulatorRegister);
    }

    void popValueAligned()
    {
        addPtr(TrustedImm32(2 * PointerSize), StackPointerRegister);
    }

    Jump binopBothIntPath(Address lhsAddr, std::function<Jump(void)> fastPath)
    {
        urshift64(AccumulatorRegister, TrustedImm32(32), ScratchRegister);
        Jump accNotInt = branch32(NotEqual, TrustedImm32(int(IntegerTag)), ScratchRegister);
        load64(lhsAddr, ScratchRegister);
        urshift64(ScratchRegister, TrustedImm32(32), ScratchRegister2);
        Jump lhsNotInt = branch32(NotEqual, TrustedImm32(int(IntegerTag)), ScratchRegister2);

        // both integer
        Jump failure = fastPath();
        Jump done = jump();

        // all other cases
        if (failure.isSet())
            failure.link(this);
        accNotInt.link(this);
        lhsNotInt.link(this);

        return done;
    }

    Jump unopIntPath(std::function<Jump(void)> fastPath)
    {
        urshift64(AccumulatorRegister, TrustedImm32(Value::IsIntegerConvertible_Shift), ScratchRegister);
        Jump accNotIntConvertible = branch32(NotEqual, TrustedImm32(1), ScratchRegister);

        // both integer
        Jump failure = fastPath();
        Jump done = jump();

        // all other cases
        if (failure.isSet())
            failure.link(this);
        accNotIntConvertible.link(this);

        return done;
    }

    void callWithAccumulatorByValueAsFirstArgument(std::function<void()> doCall)
    {
        passAsArg(AccumulatorRegister, 0);
        doCall();
    }
};

typedef PlatformAssembler64 PlatformAssembler;
#endif

#if QT_POINTER_SIZE == 4 || defined(ENABLE_ALL_ASSEMBLERS_FOR_REFACTORING_PURPOSES)
class PlatformAssembler32 : public PlatformAssemblerCommon
{
public:
    PlatformAssembler32(const Value *constantTable)
        : PlatformAssemblerCommon(constantTable)
    {}

    void callRuntime(const char *functionName, const void *funcPtr,
                     CallResultDestination dest)
    {
        PlatformAssemblerCommon::callRuntime(functionName, funcPtr);
        if (dest == CallResultDestination::InAccumulator)
            saveReturnValueInAccumulator();
        else if (AccumulatorRegisterValue == ReturnValueRegisterValue)
            loadUndefined();
    }

    void saveReturnValueInAccumulator()
    {
        move(ReturnValueRegisterValue, AccumulatorRegisterValue);
        move(ReturnValueRegisterTag, AccumulatorRegisterTag);
    }

    void loadUndefined()
    {
        move(TrustedImm32(0), AccumulatorRegisterValue);
        move(TrustedImm32(0), AccumulatorRegisterTag);
    }

    void copyConst(int constIndex, Address destRegAddr)
    {
        //###
        if (constant(constIndex).isUndefined()) {
            move(TrustedImm32(0), ScratchRegister);
            store32(ScratchRegister, destRegAddr);
            destRegAddr.offset += 4;
            store32(ScratchRegister, destRegAddr);
        } else {
            Address src = loadConstAddress(constIndex);
            loadDouble(src, FPScratchRegister);
            storeDouble(FPScratchRegister, destRegAddr);
        }
    }

    void copyReg(Address src, Address dest)
    {
        loadDouble(src, FPScratchRegister);
        storeDouble(FPScratchRegister, dest);
    }

    void loadPointerFromValue(Address addr, RegisterID dest = AccumulatorRegisterValue)
    {
        load32(addr, dest);
    }

    void loadAccumulator(Address src)
    {
        load32(src, AccumulatorRegisterValue);
        src.offset += 4;
        load32(src, AccumulatorRegisterTag);
    }

    void storeAccumulator(Address addr)
    {
        store32(AccumulatorRegisterValue, addr);
        addr.offset += 4;
        store32(AccumulatorRegisterTag, addr);
    }

    void moveReg(Address sourceRegAddress, Address destRegAddress)
    {
        load32(sourceRegAddress, ScratchRegister);
        store32(ScratchRegister, destRegAddress);
        sourceRegAddress.offset += 4;
        destRegAddress.offset += 4;
        load32(sourceRegAddress, ScratchRegister);
        store32(ScratchRegister, destRegAddress);
    }

    void loadString(int stringId)
    {
        load32(loadStringAddress(stringId), AccumulatorRegisterValue);
        move(TrustedImm32(0), AccumulatorRegisterTag);
    }

    void loadValue(ReturnedValue value)
    {
        move(TrustedImm32(Value::fromReturnedValue(value).value()), AccumulatorRegisterValue);
        move(TrustedImm32(Value::fromReturnedValue(value).tag()), AccumulatorRegisterTag);
    }

    void storeHeapObject(RegisterID source, Address addr)
    {
        store32(source, addr);
        addr.offset += 4;
        store32(TrustedImm32(0), addr);
    }


    void generateCatchTrampoline()
    {
        PlatformAssemblerCommon::generateCatchTrampoline([this](){loadUndefined();});
    }

    void toNumber()
    {
        urshift32(AccumulatorRegisterTag, TrustedImm32(Value::QuickType_Shift - 32), ScratchRegister);
        auto isNumber = branch32(GreaterThanOrEqual, ScratchRegister, TrustedImm32(Value::QT_Int));

        if (ArgInRegCount < 2) {
            subPtr(TrustedImm32(2 * PointerSize), StackPointerRegister); // stack alignment
            push(AccumulatorRegisterTag);
            push(AccumulatorRegisterValue);
        } else {
            move(AccumulatorRegisterValue, registerForArg(0));
            move(AccumulatorRegisterTag, registerForArg(1));
        }
        callRuntimeUnchecked("toNumberHelper", reinterpret_cast<void *>(&toNumberHelper));
        saveReturnValueInAccumulator();
        if (ArgInRegCount < 2)
            addPtr(TrustedImm32(4 * PointerSize), StackPointerRegister);

        isNumber.link(this);
    }

    // this converts both the lhs and the accumulator to int32
    void toInt32LhsAcc(Address lhs, RegisterID lhsTarget)
    {
        bool accumulatorNeedsSaving = AccumulatorRegisterValue == ReturnValueRegisterValue
                || AccumulatorRegisterTag == ReturnValueRegisterTag;
        lhs.offset += 4;
        load32(lhs, lhsTarget);
        lhs.offset -= 4;
        auto lhsIsNotInt = branch32(NotEqual, TrustedImm32(int(IntegerTag)), lhsTarget);
        load32(lhs, lhsTarget);
        auto lhsIsInt = jump();

        lhsIsNotInt.link(this);

        // Save accumulator from being garbage collected, no matter if we will reuse the register.
        const Address accumulatorStackAddress(JSStackFrameRegister,
                                              offsetof(CallData, accumulator));
        storeAccumulator(accumulatorStackAddress);

        if (ArgInRegCount < 2) {
            subPtr(TrustedImm32(2 * PointerSize), StackPointerRegister);
            push(lhsTarget);
            load32(lhs, lhsTarget);
            push(lhsTarget);
        } else {
            move(lhsTarget, registerForArg(1));
            load32(lhs, registerForArg(0));
        }
        callHelper(toInt32Helper);
        move(ReturnValueRegisterValue, lhsTarget);
        if (ArgInRegCount < 2)
            addPtr(TrustedImm32(4 * PointerSize), StackPointerRegister);

        if (accumulatorNeedsSaving) // otherwise it's still the same
            loadAccumulator(accumulatorStackAddress);

        lhsIsInt.link(this);

        auto rhsIsInt = branch32(Equal, TrustedImm32(int(IntegerTag)), AccumulatorRegisterTag);

        pushAligned(lhsTarget);
        if (ArgInRegCount < 2) {
            subPtr(TrustedImm32(2 * PointerSize), StackPointerRegister);
            push(AccumulatorRegisterTag);
            push(AccumulatorRegisterValue);
        } else {
            move(AccumulatorRegisterValue, registerForArg(0));
            move(AccumulatorRegisterTag, registerForArg(1));
        }
        callRuntimeUnchecked("toInt32Helper", reinterpret_cast<void *>(&toInt32Helper));
        saveReturnValueInAccumulator();
        if (ArgInRegCount < 2)
            addPtr(TrustedImm32(4 * PointerSize), StackPointerRegister);
        popAligned(lhsTarget);

        rhsIsInt.link(this);
    }

    void toInt32()
    {
        urshift32(AccumulatorRegisterTag, TrustedImm32(Value::QuickType_Shift - 32), ScratchRegister);
        auto isInt = branch32(Equal, TrustedImm32(Value::QT_Int), ScratchRegister);

        if (ArgInRegCount < 2) {
            subPtr(TrustedImm32(2 * PointerSize), StackPointerRegister); // align the stack on a 16-byte boundary
            push(AccumulatorRegisterTag);
            push(AccumulatorRegisterValue);
        } else {
            move(AccumulatorRegisterValue, registerForArg(0));
            move(AccumulatorRegisterTag, registerForArg(1));
        }
        callRuntimeUnchecked("toInt32Helper", reinterpret_cast<void *>(&toInt32Helper));
        saveReturnValueInAccumulator();
        if (ArgInRegCount < 2)
            addPtr(TrustedImm32(4 * PointerSize), StackPointerRegister);

        isInt.link(this);
    }

    void regToInt32(Address srcReg, RegisterID targetReg)
    {
        bool accumulatorNeedsSaving = AccumulatorRegisterValue == ReturnValueRegisterValue
                || AccumulatorRegisterTag == ReturnValueRegisterTag;
        if (accumulatorNeedsSaving) {
            push(AccumulatorRegisterTag);
            push(AccumulatorRegisterValue);
        }
        if (ArgInRegCount < 2) {
            if (!accumulatorNeedsSaving)
                subPtr(TrustedImm32(2 * PointerSize), StackPointerRegister);
            srcReg.offset += 4;
            load32(srcReg, targetReg);
            push(targetReg);
            srcReg.offset -= 4;
            load32(srcReg, targetReg);
            push(targetReg);
        } else {
            if (accumulatorNeedsSaving)
                subPtr(TrustedImm32(2 * PointerSize), StackPointerRegister);
            load32(srcReg, registerForArg(0));
            srcReg.offset += 4;
            load32(srcReg, registerForArg(1));
        }
        callHelper(toInt32Helper);
        move(ReturnValueRegisterValue, targetReg);
        if (accumulatorNeedsSaving) {
            addPtr(TrustedImm32(2 * PointerSize), StackPointerRegister);
            pop(AccumulatorRegisterValue);
            pop(AccumulatorRegisterTag);
        } else if (ArgInRegCount < 2) {
            addPtr(TrustedImm32(4 * PointerSize), StackPointerRegister);
        }
    }

    void isNullOrUndefined()
    {
        Jump notUndefOrPtr = branch32(NotEqual, TrustedImm32(0), AccumulatorRegisterTag);
        compare32(Equal, AccumulatorRegisterValue, TrustedImm32(0), AccumulatorRegisterValue);
        auto done = jump();

        // not undefined or managed
        notUndefOrPtr.link(this);
        compare32(Equal, AccumulatorRegisterTag, TrustedImm32(int(QV4::Value::ValueTypeInternal::Null)),
                  AccumulatorRegisterValue);

        done.link(this);
    }

    Jump isIntOrBool()
    {
        urshift32(AccumulatorRegisterTag, TrustedImm32(Value::IsIntegerOrBool_Shift - 32), ScratchRegister);
        return branch32(Equal, TrustedImm32(3), ScratchRegister);
    }

    void pushValue(ReturnedValue v)
    {
        push(TrustedImm32(v >> 32));
        push(TrustedImm32(v));
    }

    void jumpNotUndefined(int offset)
    {
        move(AccumulatorRegisterTag, ScratchRegister);
        or32(AccumulatorRegisterValue, ScratchRegister);
        auto jump = branch32(NotEqual, ScratchRegister, TrustedImm32(0));
        addJumpToOffset(jump, offset);
    }

    Jump jumpEmpty()
    {
        return branch32(Equal, AccumulatorRegisterTag, TrustedImm32(Value::emptyValue().asReturnedValue() >> 32));
    }

    Jump jumpNotEmpty()
    {
        return branch32(NotEqual, AccumulatorRegisterTag, TrustedImm32(Value::emptyValue().asReturnedValue() >> 32));
    }

    void toBoolean(std::function<void(RegisterID)> continuation)
    {
        urshift32(AccumulatorRegisterTag, TrustedImm32(Value::IsIntegerConvertible_Shift - 32),
                  ScratchRegister);
        auto needsConversion = branch32(NotEqual, TrustedImm32(1), ScratchRegister);
        continuation(AccumulatorRegisterValue);
        Jump done = jump();

        // slow path:
        needsConversion.link(this);

        bool accumulatorNeedsSaving = AccumulatorRegisterValue == ReturnValueRegisterValue
                || AccumulatorRegisterTag == ReturnValueRegisterTag;
        if (accumulatorNeedsSaving) {
            push(AccumulatorRegisterTag);
            push(AccumulatorRegisterValue);
        }

        if (ArgInRegCount < 2) {
            if (!accumulatorNeedsSaving)
                subPtr(TrustedImm32(2 * PointerSize), StackPointerRegister);
            push(AccumulatorRegisterTag);
            push(AccumulatorRegisterValue);
        } else {
            if (accumulatorNeedsSaving)
                subPtr(TrustedImm32(2 * PointerSize), StackPointerRegister);
            move(AccumulatorRegisterValue, registerForArg(0));
            move(AccumulatorRegisterTag, registerForArg(1));
        }
        callHelper(Value::toBooleanImpl);
        and32(TrustedImm32(1), ReturnValueRegisterValue, ScratchRegister);
        if (accumulatorNeedsSaving) {
            addPtr(TrustedImm32(2 * PointerSize), StackPointerRegister);
            pop(AccumulatorRegisterValue);
            pop(AccumulatorRegisterTag);
        } else if (ArgInRegCount < 2) {
            addPtr(TrustedImm32(4 * PointerSize), StackPointerRegister);
        }
        continuation(ScratchRegister);

        done.link(this);
    }

    void jumpStrictEqualStackSlotInt(int lhs, int rhs, int offset)
    {
        Address lhsAddr(JSStackFrameRegister, lhs * int(sizeof(Value)));
        load32(lhsAddr, ScratchRegister);
        Jump notEqInt = branch32(NotEqual, ScratchRegister, TrustedImm32(rhs));
        Jump notEqUndefVal = branch32(NotEqual, ScratchRegister, TrustedImm32(0));
        addJumpToOffset(notEqUndefVal, offset);
        lhsAddr.offset += 4;
        load32(lhsAddr, ScratchRegister);
        Jump notEqUndefTag = branch32(NotEqual, ScratchRegister, TrustedImm32(0));
        addJumpToOffset(notEqUndefTag, offset);
        notEqInt.link(this);
    }

    void jumpStrictNotEqualStackSlotInt(int lhs, int rhs, int offset)
    {
        Address lhsAddr(JSStackFrameRegister, lhs * int(sizeof(Value)));
        load32(lhsAddr, ScratchRegister);
        Jump notEqual = branch32(NotEqual, TrustedImm32(rhs), ScratchRegister);
        addJumpToOffset(notEqual, offset);
        Jump notUndefValue = branch32(NotEqual, TrustedImm32(0), ScratchRegister);
        lhsAddr.offset += 4;
        load32(lhsAddr, ScratchRegister);
        Jump equalUndef = branch32(Equal, TrustedImm32(0), ScratchRegister);
        addJumpToOffset(equalUndef, offset);
        notUndefValue.link(this);
    }

    void setAccumulatorTag(QV4::Value::ValueTypeInternal tag, RegisterID sourceReg = NoRegister)
    {
        if (sourceReg != NoRegister)
            move(sourceReg, AccumulatorRegisterValue);
        move(TrustedImm32(int(tag)), AccumulatorRegisterTag);
    }

    void encodeDoubleIntoAccumulator(FPRegisterID src)
    {
        moveDoubleToInts(src, AccumulatorRegisterValue, AccumulatorRegisterTag);
        xor32(TrustedImm32(Value::NaNEncodeMask >> 32), AccumulatorRegisterTag);
    }

    void pushValueAligned(ReturnedValue v)
    {
        pushValue(v);
    }

    void popValueAligned()
    {
        popValue();
    }

    Jump binopBothIntPath(Address lhsAddr, std::function<Jump(void)> fastPath)
    {
        Jump accNotInt = branch32(NotEqual, TrustedImm32(int(IntegerTag)), AccumulatorRegisterTag);
        Address lhsAddrTag = lhsAddr; lhsAddrTag.offset += Value::tagOffset();
        load32(lhsAddrTag, ScratchRegister);
        Jump lhsNotInt = branch32(NotEqual, TrustedImm32(int(IntegerTag)), ScratchRegister);

        // both integer
        Address lhsAddrValue = lhsAddr; lhsAddrValue.offset += Value::valueOffset();
        load32(lhsAddrValue, ScratchRegister);
        Jump failure = fastPath();
        Jump done = jump();

        // all other cases
        if (failure.isSet())
            failure.link(this);
        accNotInt.link(this);
        lhsNotInt.link(this);

        return done;
    }

    Jump unopIntPath(std::function<Jump(void)> fastPath)
    {
        Jump accNotInt = branch32(NotEqual, TrustedImm32(int(IntegerTag)), AccumulatorRegisterTag);

        // both integer
        Jump failure = fastPath();
        Jump done = jump();

        // all other cases
        if (failure.isSet())
            failure.link(this);
        accNotInt.link(this);

        return done;
    }

    void callWithAccumulatorByValueAsFirstArgument(std::function<void()> doCall)
    {
        if (ArgInRegCount < 2) {
            subPtr(TrustedImm32(2 * PointerSize), StackPointerRegister);
            push(AccumulatorRegisterTag);
            push(AccumulatorRegisterValue);
        } else {
            move(AccumulatorRegisterValue, registerForArg(0));
            move(AccumulatorRegisterTag, registerForArg(1));
        }
        doCall();
        if (ArgInRegCount < 2)
            addPtr(TrustedImm32(4 * PointerSize), StackPointerRegister);
    }
};

typedef PlatformAssembler32 PlatformAssembler;
#endif

#define pasm() reinterpret_cast<PlatformAssembler *>(this->d)

typedef PlatformAssembler::TrustedImmPtr TrustedImmPtr;
typedef PlatformAssembler::TrustedImm32 TrustedImm32;
typedef PlatformAssembler::TrustedImm64 TrustedImm64;
typedef PlatformAssembler::Address Address;
typedef PlatformAssembler::RegisterID RegisterID;
typedef PlatformAssembler::FPRegisterID FPRegisterID;

static Address regAddr(int reg)
{
    return Address(PlatformAssembler::JSStackFrameRegister, reg * int(sizeof(QV4::Value)));
}

BaselineAssembler::BaselineAssembler(const Value *constantTable)
    : d(new PlatformAssembler(constantTable))
{
}

BaselineAssembler::~BaselineAssembler()
{
    delete pasm();
}

void BaselineAssembler::generatePrologue()
{
    pasm()->generateFunctionEntry();
}

void BaselineAssembler::generateEpilogue()
{
    pasm()->generateCatchTrampoline();
}

void BaselineAssembler::link(Function *function)
{
    pasm()->link(function, "BaselineJIT");
}

void BaselineAssembler::addLabel(int offset)
{
    pasm()->addLabelForOffset(offset);
}

void BaselineAssembler::loadConst(int constIndex)
{
    //###
    if (pasm()->constant(constIndex).isUndefined()) {
        pasm()->loadUndefined();
    } else {
        pasm()->loadAccumulator(pasm()->loadConstAddress(constIndex));
    }
}

void BaselineAssembler::copyConst(int constIndex, int destReg)
{
    pasm()->copyConst(constIndex, regAddr(destReg));
}

void BaselineAssembler::loadReg(int reg)
{
    pasm()->loadAccumulator(regAddr(reg));
}

void JIT::BaselineAssembler::moveReg(int sourceReg, int destReg)
{
    pasm()->moveReg(regAddr(sourceReg), regAddr(destReg));
}

void BaselineAssembler::storeReg(int reg)
{
    pasm()->storeAccumulator(regAddr(reg));
}

void BaselineAssembler::loadLocal(int index, int level)
{
    Heap::CallContext ctx;
    Q_UNUSED(ctx)
    pasm()->loadPointerFromValue(regAddr(CallData::Context), PlatformAssembler::ScratchRegister);
    while (level) {
        pasm()->loadPtr(Address(PlatformAssembler::ScratchRegister, ctx.outer.offset), PlatformAssembler::ScratchRegister);
        --level;
    }
    pasm()->loadAccumulator(Address(PlatformAssembler::ScratchRegister, ctx.locals.offset + offsetof(ValueArray<0>, values) + sizeof(Value)*index));
}

void BaselineAssembler::storeLocal(int index, int level)
{
    Heap::CallContext ctx;
    Q_UNUSED(ctx)
    pasm()->loadPtr(regAddr(CallData::Context), PlatformAssembler::ScratchRegister);
    while (level) {
        pasm()->loadPtr(Address(PlatformAssembler::ScratchRegister, ctx.outer.offset), PlatformAssembler::ScratchRegister);
        --level;
    }
    pasm()->storeAccumulator(Address(PlatformAssembler::ScratchRegister, ctx.locals.offset + offsetof(ValueArray<0>, values) + sizeof(Value)*index));
}

void BaselineAssembler::loadString(int stringId)
{
    pasm()->loadString(stringId);
}

void BaselineAssembler::loadValue(ReturnedValue value)
{
    pasm()->loadValue(value);
}

void BaselineAssembler::storeHeapObject(int reg)
{
    pasm()->storeHeapObject(PlatformAssembler::AccumulatorRegisterValue, regAddr(reg));
}

void BaselineAssembler::loadImport(int index)
{
    Address addr = pasm()->loadCompilationUnitPtr(PlatformAssembler::ScratchRegister);
    addr.offset = offsetof(QV4::CompiledData::CompilationUnitBase, imports);
    pasm()->loadPtr(addr, PlatformAssembler::ScratchRegister);
    addr.offset = index * int(sizeof(QV4::Value*));
    pasm()->loadPtr(addr, PlatformAssembler::ScratchRegister);
    pasm()->loadAccumulator(Address(PlatformAssembler::ScratchRegister));
}

void BaselineAssembler::toNumber()
{
    pasm()->toNumber();
}

void BaselineAssembler::uminus()
{
    saveAccumulatorInFrame();
    pasm()->prepareCallWithArgCount(1);
    pasm()->passAccumulatorAsArg(0);
    ASM_GENERATE_RUNTIME_CALL(Runtime::method_uMinus, CallResultDestination::InAccumulator);
    checkException();
}

void BaselineAssembler::ucompl()
{
    pasm()->toInt32();
    pasm()->xor32(TrustedImm32(-1), PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(IntegerTag);
}

static ReturnedValue incHelper(const Value v)
{
    double d;
    if (Q_LIKELY(v.isDouble()))
        d =  v.doubleValue();
    else
        d = v.toNumberImpl();
    return Encode(d + 1.);
}

void BaselineAssembler::inc()
{
    auto done = pasm()->unopIntPath([this](){
        auto overflowed = pasm()->branchAdd32(PlatformAssembler::Overflow,
                                              PlatformAssembler::AccumulatorRegisterValue,
                                              TrustedImm32(1),
                                              PlatformAssembler::ScratchRegister);
        pasm()->setAccumulatorTag(IntegerTag, PlatformAssembler::ScratchRegister);
        return overflowed;
    });

    // slow path:
    pasm()->callWithAccumulatorByValueAsFirstArgument([this]() {
        pasm()->callHelper(incHelper);
        pasm()->saveReturnValueInAccumulator();
    });
    checkException();

    // done.
    done.link(pasm());
}

static ReturnedValue decHelper(const Value v)
{
    double d;
    if (Q_LIKELY(v.isDouble()))
        d =  v.doubleValue();
    else
        d = v.toNumberImpl();
    return Encode(d - 1.);
}

void BaselineAssembler::dec()
{
    auto done = pasm()->unopIntPath([this](){
        auto overflowed = pasm()->branchSub32(PlatformAssembler::Overflow,
                                              PlatformAssembler::AccumulatorRegisterValue,
                                              TrustedImm32(1),
                                              PlatformAssembler::ScratchRegister);
        pasm()->setAccumulatorTag(IntegerTag, PlatformAssembler::ScratchRegister);
        return overflowed;
    });

    // slow path:
    pasm()->callWithAccumulatorByValueAsFirstArgument([this]() {
        pasm()->callHelper(decHelper);
        pasm()->saveReturnValueInAccumulator();
    });
    checkException();

    // done.
    done.link(pasm());
}

void BaselineAssembler::unot()
{
    pasm()->toBoolean([this](PlatformAssembler::RegisterID resultReg){
        pasm()->compare32(PlatformAssembler::Equal, resultReg,
                          TrustedImm32(0), PlatformAssembler::AccumulatorRegisterValue);
        pasm()->setAccumulatorTag(QV4::Value::ValueTypeInternal::Boolean);
    });
}

void BaselineAssembler::add(int lhs)
{
    auto done = pasm()->binopBothIntPath(regAddr(lhs), [this](){
        auto overflowed = pasm()->branchAdd32(PlatformAssembler::Overflow,
                                              PlatformAssembler::AccumulatorRegisterValue,
                                              PlatformAssembler::ScratchRegister);
        pasm()->setAccumulatorTag(IntegerTag,
                                  PlatformAssembler::ScratchRegister);
        return overflowed;
    });

    // slow path:
    saveAccumulatorInFrame();
    pasm()->prepareCallWithArgCount(3);
    pasm()->passAccumulatorAsArg(2);
    pasm()->passJSSlotAsArg(lhs, 1);
    pasm()->passEngineAsArg(0);
    ASM_GENERATE_RUNTIME_CALL(Runtime::method_add, CallResultDestination::InAccumulator);
    checkException();

    // done.
    done.link(pasm());
}

void BaselineAssembler::bitAnd(int lhs)
{
    PlatformAssembler::Address lhsAddr = regAddr(lhs);
    pasm()->toInt32LhsAcc(lhsAddr, PlatformAssembler::ScratchRegister);
    pasm()->and32(PlatformAssembler::ScratchRegister, PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(IntegerTag);
}

void BaselineAssembler::bitOr(int lhs)
{
    PlatformAssembler::Address lhsAddr = regAddr(lhs);
    pasm()->toInt32LhsAcc(lhsAddr, PlatformAssembler::ScratchRegister);
    pasm()->or32(PlatformAssembler::ScratchRegister, PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(IntegerTag);
}

void BaselineAssembler::bitXor(int lhs)
{
    PlatformAssembler::Address lhsAddr = regAddr(lhs);
    pasm()->toInt32LhsAcc(lhsAddr, PlatformAssembler::ScratchRegister);
    pasm()->xor32(PlatformAssembler::ScratchRegister, PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(IntegerTag);
}

void BaselineAssembler::ushr(int lhs)
{
    PlatformAssembler::Address lhsAddr = regAddr(lhs);
    pasm()->toInt32LhsAcc(lhsAddr, PlatformAssembler::ScratchRegister);
    pasm()->and32(TrustedImm32(0x1f), PlatformAssembler::AccumulatorRegisterValue);
    pasm()->urshift32(PlatformAssembler::AccumulatorRegisterValue, PlatformAssembler::ScratchRegister);
    pasm()->move(PlatformAssembler::ScratchRegister, PlatformAssembler::AccumulatorRegisterValue);
    auto doubleEncode = pasm()->branch32(PlatformAssembler::LessThan,
                                         PlatformAssembler::AccumulatorRegisterValue,
                                         TrustedImm32(0));
    pasm()->setAccumulatorTag(IntegerTag);
    auto done = pasm()->jump();

    doubleEncode.link(pasm());
    pasm()->convertUInt32ToDouble(PlatformAssembler::AccumulatorRegisterValue,
                                  PlatformAssembler::FPScratchRegister,
                                  PlatformAssembler::ScratchRegister);
    pasm()->encodeDoubleIntoAccumulator(PlatformAssembler::FPScratchRegister);
    done.link(pasm());
}

void BaselineAssembler::shr(int lhs)
{
    PlatformAssembler::Address lhsAddr = regAddr(lhs);
    pasm()->toInt32LhsAcc(lhsAddr, PlatformAssembler::ScratchRegister);
    pasm()->and32(TrustedImm32(0x1f), PlatformAssembler::AccumulatorRegisterValue);
    pasm()->rshift32(PlatformAssembler::AccumulatorRegisterValue, PlatformAssembler::ScratchRegister);
    pasm()->move(PlatformAssembler::ScratchRegister, PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(IntegerTag);
}

void BaselineAssembler::shl(int lhs)
{
    PlatformAssembler::Address lhsAddr = regAddr(lhs);
    pasm()->toInt32LhsAcc(lhsAddr, PlatformAssembler::ScratchRegister);
    pasm()->and32(TrustedImm32(0x1f), PlatformAssembler::AccumulatorRegisterValue);
    pasm()->lshift32(PlatformAssembler::AccumulatorRegisterValue, PlatformAssembler::ScratchRegister);
    pasm()->move(PlatformAssembler::ScratchRegister, PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(IntegerTag);
}

void BaselineAssembler::bitAndConst(int rhs)
{
    pasm()->toInt32();
    pasm()->and32(TrustedImm32(rhs), PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(IntegerTag);
}

void BaselineAssembler::bitOrConst(int rhs)
{
    pasm()->toInt32();
    pasm()->or32(TrustedImm32(rhs), PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(IntegerTag);
}

void BaselineAssembler::bitXorConst(int rhs)
{
    pasm()->toInt32();
    pasm()->xor32(TrustedImm32(rhs), PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(IntegerTag);
}

void BaselineAssembler::ushrConst(int rhs)
{
    rhs &= 0x1f;
    pasm()->toInt32();
    if (rhs) {
        // a non zero shift will always give a number encodable as an int
        pasm()->urshift32(TrustedImm32(rhs), PlatformAssembler::AccumulatorRegisterValue);
        pasm()->setAccumulatorTag(IntegerTag);
    } else {
        // shift with 0 can lead to a negative result
        auto doubleEncode = pasm()->branch32(PlatformAssembler::LessThan,
                                             PlatformAssembler::AccumulatorRegisterValue,
                                             TrustedImm32(0));
        pasm()->setAccumulatorTag(IntegerTag);
        auto done = pasm()->jump();

        doubleEncode.link(pasm());
        pasm()->convertUInt32ToDouble(PlatformAssembler::AccumulatorRegisterValue,
                                      PlatformAssembler::FPScratchRegister,
                                      PlatformAssembler::ScratchRegister);
        pasm()->encodeDoubleIntoAccumulator(PlatformAssembler::FPScratchRegister);
        done.link(pasm());
    }
}

void BaselineAssembler::shrConst(int rhs)
{
    rhs &= 0x1f;
    pasm()->toInt32();
    if (rhs)
        pasm()->rshift32(TrustedImm32(rhs), PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(IntegerTag);
}

void BaselineAssembler::shlConst(int rhs)
{
    rhs &= 0x1f;
    pasm()->toInt32();
    if (rhs)
        pasm()->lshift32(TrustedImm32(rhs), PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(IntegerTag);
}

void BaselineAssembler::mul(int lhs)
{
    auto done = pasm()->binopBothIntPath(regAddr(lhs), [this](){
        auto overflowed = pasm()->branchMul32(PlatformAssembler::Overflow,
                                              PlatformAssembler::AccumulatorRegisterValue,
                                              PlatformAssembler::ScratchRegister);
        pasm()->setAccumulatorTag(IntegerTag,
                                  PlatformAssembler::ScratchRegister);
        return overflowed;
    });

    // slow path:
    saveAccumulatorInFrame();
    pasm()->prepareCallWithArgCount(2);
    pasm()->passAccumulatorAsArg(1);
    pasm()->passJSSlotAsArg(lhs, 0);
    ASM_GENERATE_RUNTIME_CALL(Runtime::method_mul, CallResultDestination::InAccumulator);
    checkException();

    // done.
    done.link(pasm());
}

void BaselineAssembler::div(int lhs)
{
    saveAccumulatorInFrame();
    pasm()->prepareCallWithArgCount(2);
    pasm()->passAccumulatorAsArg(1);
    pasm()->passJSSlotAsArg(lhs, 0);
    ASM_GENERATE_RUNTIME_CALL(Runtime::method_div, CallResultDestination::InAccumulator);
    checkException();
}

void BaselineAssembler::mod(int lhs)
{
    saveAccumulatorInFrame();
    pasm()->prepareCallWithArgCount(2);
    pasm()->passAccumulatorAsArg(1);
    pasm()->passJSSlotAsArg(lhs, 0);
    ASM_GENERATE_RUNTIME_CALL(Runtime::method_mod, CallResultDestination::InAccumulator);
    checkException();
}

void BaselineAssembler::sub(int lhs)
{
    auto done = pasm()->binopBothIntPath(regAddr(lhs), [this](){
        auto overflowed = pasm()->branchSub32(PlatformAssembler::Overflow,
                                              PlatformAssembler::AccumulatorRegisterValue,
                                              PlatformAssembler::ScratchRegister);
        pasm()->setAccumulatorTag(IntegerTag,
                                  PlatformAssembler::ScratchRegister);
        return overflowed;
    });

    // slow path:
    saveAccumulatorInFrame();
    pasm()->prepareCallWithArgCount(2);
    pasm()->passAccumulatorAsArg(1);
    pasm()->passJSSlotAsArg(lhs, 0);
    ASM_GENERATE_RUNTIME_CALL(Runtime::method_sub, CallResultDestination::InAccumulator);
    checkException();

    // done.
    done.link(pasm());
}

void BaselineAssembler::cmpeqNull()
{
    pasm()->isNullOrUndefined();
    pasm()->setAccumulatorTag(QV4::Value::ValueTypeInternal::Boolean);
}

void BaselineAssembler::cmpneNull()
{
    pasm()->isNullOrUndefined();
    pasm()->xor32(TrustedImm32(1), PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(QV4::Value::ValueTypeInternal::Boolean);
}

void BaselineAssembler::cmpeqInt(int lhs)
{
    auto isIntOrBool = pasm()->isIntOrBool();
    saveAccumulatorInFrame();
    pasm()->pushValueAligned(Encode(lhs));
    if (PlatformAssembler::ArgInRegCount < 2)
        pasm()->push(PlatformAssembler::StackPointerRegister);
    else
        pasm()->move(PlatformAssembler::StackPointerRegister, pasm()->registerForArg(1));
    pasm()->pushAccumulatorAsArg(0);
    pasm()->callRuntimeUnchecked("Runtime::method_equal", (void*)Runtime::method_equal);
    pasm()->saveReturnValueInAccumulator();
    if (PlatformAssembler::ArgInRegCount < 2)
        pasm()->addPtr(TrustedImm32(2 * PlatformAssembler::PointerSize), PlatformAssembler::StackPointerRegister);
    pasm()->popValueAligned();
    auto done = pasm()->jump();
    isIntOrBool.link(pasm());
    pasm()->compare32(PlatformAssembler::Equal, PlatformAssembler::AccumulatorRegisterValue,
                      TrustedImm32(lhs),
                      PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(QV4::Value::ValueTypeInternal::Boolean);
    done.link(pasm());
}

void BaselineAssembler::cmpneInt(int lhs)
{
    auto isIntOrBool = pasm()->isIntOrBool();
    saveAccumulatorInFrame();
    pasm()->pushValueAligned(Encode(lhs));
    if (PlatformAssembler::ArgInRegCount < 2)
        pasm()->push(PlatformAssembler::StackPointerRegister);
    else
        pasm()->move(PlatformAssembler::StackPointerRegister, pasm()->registerForArg(1));
    pasm()->pushAccumulatorAsArg(0);
    pasm()->callRuntimeUnchecked("Runtime::method_notEqual", (void*)Runtime::method_notEqual);
    pasm()->saveReturnValueInAccumulator();
    if (PlatformAssembler::ArgInRegCount < 2)
        pasm()->addPtr(TrustedImm32(2 * PlatformAssembler::PointerSize), PlatformAssembler::StackPointerRegister);
    pasm()->popValueAligned();
    auto done = pasm()->jump();
    isIntOrBool.link(pasm());
    pasm()->compare32(PlatformAssembler::NotEqual, PlatformAssembler::AccumulatorRegisterValue,
                      TrustedImm32(lhs),
                      PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(QV4::Value::ValueTypeInternal::Boolean);
    done.link(pasm());
}

void BaselineAssembler::cmp(int cond, CmpFunc function, const char *functionName, int lhs)
{
    auto c = static_cast<PlatformAssembler::RelationalCondition>(cond);
    auto done = pasm()->binopBothIntPath(regAddr(lhs), [this, c](){
        pasm()->compare32(c, PlatformAssembler::ScratchRegister,
                          PlatformAssembler::AccumulatorRegisterValue,
                          PlatformAssembler::AccumulatorRegisterValue);
        pasm()->setAccumulatorTag(QV4::Value::ValueTypeInternal::Boolean);
        return PlatformAssembler::Jump();
    });

    // slow path:
    saveAccumulatorInFrame();
    pasm()->prepareCallWithArgCount(2);
    pasm()->passAccumulatorAsArg(1);
    pasm()->passJSSlotAsArg(lhs, 0);

    callRuntime(functionName, reinterpret_cast<void*>(function), CallResultDestination::InAccumulator);
    checkException();
    pasm()->setAccumulatorTag(QV4::Value::ValueTypeInternal::Boolean);

    // done.
    done.link(pasm());
}

void BaselineAssembler::cmpeq(int lhs)
{
    cmp(PlatformAssembler::Equal, &Runtime::method_compareEqual,
        "Runtime::method_compareEqual", lhs);
}

void BaselineAssembler::cmpne(int lhs)
{
    cmp(PlatformAssembler::NotEqual, &Runtime::method_compareNotEqual,
        "Runtime::method_compareNotEqual", lhs);
}

void BaselineAssembler::cmpgt(int lhs)
{
    cmp(PlatformAssembler::GreaterThan, &Runtime::method_compareGreaterThan,
        "Runtime::method_compareGreaterThan", lhs);
}

void BaselineAssembler::cmpge(int lhs)
{
    cmp(PlatformAssembler::GreaterThanOrEqual, &Runtime::method_compareGreaterEqual,
        "Runtime::method_compareGreaterEqual", lhs);
}

void BaselineAssembler::cmplt(int lhs)
{
    cmp(PlatformAssembler::LessThan, &Runtime::method_compareLessThan,
        "Runtime::method_compareLessThan", lhs);
}

void BaselineAssembler::cmple(int lhs)
{
    cmp(PlatformAssembler::LessThanOrEqual, &Runtime::method_compareLessEqual,
        "Runtime::method_compareLessEqual", lhs);
}

void BaselineAssembler::cmpStrictEqual(int lhs)
{
    cmp(PlatformAssembler::Equal, &RuntimeHelpers::strictEqual,
        "RuntimeHelpers::strictEqual", lhs);
}

void BaselineAssembler::cmpStrictNotEqual(int lhs)
{
    cmp(PlatformAssembler::Equal, &RuntimeHelpers::strictEqual,
        "RuntimeHelpers::strictEqual", lhs);
    pasm()->xor32(TrustedImm32(1), PlatformAssembler::AccumulatorRegisterValue);
    pasm()->setAccumulatorTag(QV4::Value::ValueTypeInternal::Boolean);
}

void BaselineAssembler::jump(int offset)
{
    pasm()->addJumpToOffset(pasm()->jump(), offset);
}

void BaselineAssembler::jumpTrue(int offset)
{
    pasm()->toBoolean([this, offset](PlatformAssembler::RegisterID resultReg) {
        auto jump = pasm()->branch32(PlatformAssembler::NotEqual, TrustedImm32(0), resultReg);
        pasm()->addJumpToOffset(jump, offset);
    });
}

void BaselineAssembler::jumpFalse(int offset)
{
    pasm()->toBoolean([this, offset](PlatformAssembler::RegisterID resultReg) {
        auto jump = pasm()->branch32(PlatformAssembler::Equal, TrustedImm32(0), resultReg);
        pasm()->addJumpToOffset(jump, offset);
    });
}

void BaselineAssembler::jumpNoException(int offset)
{
    auto jump = pasm()->branch32(
        PlatformAssembler::Equal,
        PlatformAssembler::Address(PlatformAssembler::EngineRegister,
                                   offsetof(EngineBase, hasException)),
        TrustedImm32(0));
    pasm()->addJumpToOffset(jump, offset);
}

void BaselineAssembler::jumpNotUndefined(int offset)
{
    pasm()->jumpNotUndefined(offset);
}

void BaselineAssembler::prepareCallWithArgCount(int argc)
{
    pasm()->prepareCallWithArgCount(argc);
}

void BaselineAssembler::storeInstructionPointer(int instructionOffset)
{
    pasm()->storeInstructionPointer(instructionOffset);
}

void BaselineAssembler::passAccumulatorAsArg(int arg)
{
    pasm()->passAccumulatorAsArg(arg);
}

void BaselineAssembler::passFunctionAsArg(int arg)
{
    pasm()->passFunctionAsArg(arg);
}

void BaselineAssembler::passEngineAsArg(int arg)
{
    pasm()->passEngineAsArg(arg);
}

void BaselineAssembler::passJSSlotAsArg(int reg, int arg)
{
    pasm()->passJSSlotAsArg(reg, arg);
}

void BaselineAssembler::passCppFrameAsArg(int arg)
{
    pasm()->passCppFrameAsArg(arg);
}

void BaselineAssembler::passInt32AsArg(int value, int arg)
{
    pasm()->passInt32AsArg(value, arg);
}

void BaselineAssembler::callRuntime(const char *functionName, const void *funcPtr, CallResultDestination dest)
{
    pasm()->callRuntime(functionName, funcPtr, dest);
}

void BaselineAssembler::saveAccumulatorInFrame()
{
    pasm()->storeAccumulator(PlatformAssembler::Address(PlatformAssembler::JSStackFrameRegister,
                                                        offsetof(CallData, accumulator)));
}

void BaselineAssembler::loadAccumulatorFromFrame()
{
    pasm()->loadAccumulator(PlatformAssembler::Address(PlatformAssembler::JSStackFrameRegister,
                                                       offsetof(CallData, accumulator)));
}

static ReturnedValue TheJitIs__Tail_Calling__ToTheRuntimeSoTheJitFrameIsMissing(CppStackFrame *frame, ExecutionEngine *engine)
{
    return Runtime::method_tailCall(frame, engine);
}

void BaselineAssembler::jsTailCall(int func, int thisObject, int argc, int argv)
{
    Address tos = pasm()->jsAlloca(4);

    int32_t argcOffset = tos.offset + int32_t(sizeof(Value)) * Runtime::StackOffsets::tailCall_argc;
    int32_t argvOffset = tos.offset + int32_t(sizeof(Value)) * Runtime::StackOffsets::tailCall_argv;
    int32_t thisOffset = tos.offset + int32_t(sizeof(Value)) * Runtime::StackOffsets::tailCall_thisObject;
    int32_t funcOffset = tos.offset + int32_t(sizeof(Value)) * Runtime::StackOffsets::tailCall_function;

    pasm()->storeInt32AsValue(argc,      Address(tos.base, argcOffset));
    pasm()->storeInt32AsValue(argv,      Address(tos.base, argvOffset));
    pasm()->moveReg(regAddr(thisObject), Address(tos.base, thisOffset));
    pasm()->moveReg(regAddr(func),       Address(tos.base, funcOffset));
    pasm()->tailCallRuntime("TheJitIs__Tail_Calling__ToTheRuntimeSoTheJitFrameIsMissing",
                            reinterpret_cast<void *>(TheJitIs__Tail_Calling__ToTheRuntimeSoTheJitFrameIsMissing));
}

void BaselineAssembler::checkException()
{
    pasm()->checkException();
}

void BaselineAssembler::gotoCatchException()
{
    pasm()->addCatchyJump(pasm()->jump());
}

void BaselineAssembler::getException()
{
    Q_STATIC_ASSERT(sizeof(QV4::EngineBase::hasException) == 1);

    Address hasExceptionAddr(PlatformAssembler::EngineRegister,
                             offsetof(EngineBase, hasException));
    PlatformAssembler::Jump nope = pasm()->branch8(PlatformAssembler::Equal,
                                                  hasExceptionAddr,
                                                  TrustedImm32(0));
    pasm()->loadPtr(Address(PlatformAssembler::EngineRegister,
                            offsetof(EngineBase, exceptionValue)),
                    PlatformAssembler::ScratchRegister);
    pasm()->loadAccumulator(Address(PlatformAssembler::ScratchRegister));
    pasm()->store8(TrustedImm32(0), hasExceptionAddr);
    auto done = pasm()->jump();
    nope.link(pasm());
    pasm()->loadValue(Value::emptyValue().asReturnedValue());

    done.link(pasm());
}

void BaselineAssembler::setException()
{
    auto noException = pasm()->jumpEmpty();
    Address addr(PlatformAssembler::EngineRegister, offsetof(EngineBase, exceptionValue));
    pasm()->loadPtr(addr, PlatformAssembler::ScratchRegister);
    pasm()->storeAccumulator(Address(PlatformAssembler::ScratchRegister));
    addr.offset = offsetof(EngineBase, hasException);
    Q_STATIC_ASSERT(sizeof(QV4::EngineBase::hasException) == 1);
    pasm()->store8(TrustedImm32(1), addr);
    noException.link(pasm());
}

void BaselineAssembler::setUnwindHandler(int offset)
{
    auto l = pasm()->storePtrWithPatch(TrustedImmPtr(nullptr), pasm()->exceptionHandlerAddress());
    pasm()->addEHTarget(l, offset);
}


void BaselineAssembler::clearUnwindHandler()
{
    pasm()->storePtr(TrustedImmPtr(nullptr), pasm()->exceptionHandlerAddress());
}

void JIT::BaselineAssembler::unwindDispatch()
{
    checkException();
    pasm()->load32(Address(PlatformAssembler::CppStackFrameRegister, offsetof(CppStackFrame, unwindLevel)), PlatformAssembler::ScratchRegister);
    auto noUnwind = pasm()->branch32(PlatformAssembler::Equal, PlatformAssembler::ScratchRegister, TrustedImm32(0));
    pasm()->sub32(TrustedImm32(1), PlatformAssembler::ScratchRegister);
    pasm()->store32(PlatformAssembler::ScratchRegister, Address(PlatformAssembler::CppStackFrameRegister, offsetof(CppStackFrame, unwindLevel)));
    auto jump = pasm()->branch32(PlatformAssembler::Equal, PlatformAssembler::ScratchRegister, TrustedImm32(0));
    gotoCatchException();
    jump.link(pasm());

    pasm()->loadPtr(Address(PlatformAssembler::CppStackFrameRegister, offsetof(CppStackFrame, unwindLabel)), PlatformAssembler::ScratchRegister);
    pasm()->jump(PlatformAssembler::ScratchRegister);

    noUnwind.link(pasm());
}

void JIT::BaselineAssembler::unwindToLabel(int level, int offset)
{
    auto l = pasm()->storePtrWithPatch(TrustedImmPtr(nullptr), Address(PlatformAssembler::CppStackFrameRegister, offsetof(CppStackFrame, unwindLabel)));
    pasm()->addEHTarget(l, offset);
    pasm()->store32(TrustedImm32(level), Address(PlatformAssembler::CppStackFrameRegister, offsetof(CppStackFrame, unwindLevel)));
    gotoCatchException();
}

void BaselineAssembler::pushCatchContext(int index, int name)
{
    pasm()->prepareCallWithArgCount(3);
    pasm()->passInt32AsArg(name, 2);
    pasm()->passInt32AsArg(index, 1);
    pasm()->passJSSlotAsArg(CallData::Context, 0);
    ASM_GENERATE_RUNTIME_CALL(Runtime::method_createCatchContext, CallResultDestination::InAccumulator);
    pasm()->storeAccumulator(pasm()->contextAddress());
}

void BaselineAssembler::popContext()
{
    Heap::CallContext ctx;
    Q_UNUSED(ctx)
    pasm()->loadPointerFromValue(regAddr(CallData::Context), PlatformAssembler::ScratchRegister);
    pasm()->loadPtr(Address(PlatformAssembler::ScratchRegister, ctx.outer.offset), PlatformAssembler::ScratchRegister);
    pasm()->storeHeapObject(PlatformAssembler::ScratchRegister, regAddr(CallData::Context));
}

void BaselineAssembler::deadTemporalZoneCheck(int offsetForSavedIP, int variableName)
{
    auto valueIsAliveJump = pasm()->jumpNotEmpty();
    storeInstructionPointer(offsetForSavedIP);
    prepareCallWithArgCount(2);
    passInt32AsArg(variableName, 1);
    passEngineAsArg(0);
    ASM_GENERATE_RUNTIME_CALL(Runtime::method_throwReferenceError, CallResultDestination::Ignore);
    gotoCatchException();
    valueIsAliveJump.link(pasm());
}

void BaselineAssembler::ret()
{
    pasm()->generateFunctionExit();
}

} // JIT namespace
} // QV4 namepsace

QT_END_NAMESPACE

#endif // V4_ENABLE_JIT
