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
#include "qv4assemblercommon_p.h"
#include <private/qv4function_p.h>
#include <private/qv4runtime_p.h>

#include <assembler/MacroAssemblerCodeRef.h>
#include <assembler/LinkBuffer.h>
#include <WTFStubs.h>

#undef ENABLE_ALL_ASSEMBLERS_FOR_REFACTORING_PURPOSES

#ifdef V4_ENABLE_JIT

QT_BEGIN_NAMESPACE
namespace QV4 {
namespace JIT {

namespace {
class QIODevicePrintStream: public FilePrintStream
{
    Q_DISABLE_COPY(QIODevicePrintStream)

public:
    explicit QIODevicePrintStream(QIODevice *dest)
        : FilePrintStream(nullptr)
        , dest(dest)
        , buf(4096, '0')
    {
        Q_ASSERT(dest);
    }

    ~QIODevicePrintStream()
    {}

    void vprintf(const char* format, va_list argList) WTF_ATTRIBUTE_PRINTF(2, 0)
    {
        const int written = qvsnprintf(buf.data(), buf.size(), format, argList);
        if (written > 0)
            dest->write(buf.constData(), written);
        memset(buf.data(), 0, qMin(written, buf.size()));
    }

    void flush()
    {}

private:
    QIODevice *dest;
    QByteArray buf;
};
} // anonymous namespace

static void printDisassembledOutputWithCalls(QByteArray processedOutput,
                                             const QHash<const void*, const char*>& functions)
{
    for (QHash<const void*, const char*>::ConstIterator it = functions.begin(), end = functions.end();
         it != end; ++it) {
        const QByteArray ptrString = "0x" + QByteArray::number(quintptr(it.key()), 16);
        int idx = 0;
        while (idx >= 0) {
            idx = processedOutput.indexOf(ptrString, idx);
            if (idx < 0)
                break;
            idx = processedOutput.indexOf('\n', idx);
            if (idx < 0)
                break;
            processedOutput = processedOutput.insert(idx, QByteArrayLiteral("                          ; ") + it.value());
        }
    }

    qDebug("%s", processedOutput.constData());
}

static QByteArray functionName(Function *function)
{
    QByteArray name = function->name()->toQString().toUtf8();
    if (name.isEmpty()) {
        name = QByteArray::number(reinterpret_cast<quintptr>(function), 16);
        name.prepend("QV4::Function(0x");
        name.append(')');
    }
    return name;
}

JIT::PlatformAssemblerCommon::~PlatformAssemblerCommon()
{}

void PlatformAssemblerCommon::link(Function *function, const char *jitKind)
{
    for (const auto &jumpTarget : jumpsToLink)
        jumpTarget.jump.linkTo(labelForOffset[jumpTarget.offset], this);

    JSC::JSGlobalData dummy(function->internalClass->engine->executableAllocator);
    JSC::LinkBuffer<MacroAssembler> linkBuffer(dummy, this, nullptr);

    for (const auto &ehTarget : ehTargets) {
        auto targetLabel = labelForOffset.value(ehTarget.offset);
        linkBuffer.patch(ehTarget.label, linkBuffer.locationOf(targetLabel));
    }

    JSC::MacroAssemblerCodeRef codeRef;

    static const bool showCode = qEnvironmentVariableIsSet("QV4_SHOW_ASM");
    if (showCode) {
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        WTF::setDataFile(new QIODevicePrintStream(&buf));

        QByteArray name = functionName(function);
        codeRef = linkBuffer.finalizeCodeWithDisassembly(jitKind, "function %s", name.constData());

        WTF::setDataFile(stderr);
        printDisassembledOutputWithCalls(buf.data(), functions);
    } else {
        codeRef = linkBuffer.finalizeCodeWithoutDisassembly();
    }

    function->codeRef = new JSC::MacroAssemblerCodeRef(codeRef);
    function->jittedCode = reinterpret_cast<Function::JittedCode>(function->codeRef->code().executableAddress());

    // This implements writing of JIT'd addresses so that perf can find the
    // symbol names.
    //
    // Perf expects the mapping to be in a certain place and have certain
    // content, for more information, see:
    // https://github.com/torvalds/linux/blob/master/tools/perf/Documentation/jit-interface.txt
    static bool doProfile = !qEnvironmentVariableIsEmpty("QV4_PROFILE_WRITE_PERF_MAP");
    if (Q_UNLIKELY(doProfile)) {
        static QFile perfMapFile(QString::fromLatin1("/tmp/perf-%1.map")
                                 .arg(QCoreApplication::applicationPid()));
        static const bool isOpen = perfMapFile.open(QIODevice::WriteOnly);
        if (!isOpen) {
            qWarning("QV4::JIT::Assembler: Cannot write perf map file.");
            doProfile = false;
        } else {
            perfMapFile.write(QByteArray::number(reinterpret_cast<quintptr>(
                                                     codeRef.code().executableAddress()), 16));
            perfMapFile.putChar(' ');
            perfMapFile.write(QByteArray::number(static_cast<qsizetype>(codeRef.size()), 16));
            perfMapFile.putChar(' ');
            perfMapFile.write(functionName(function));
            perfMapFile.putChar('\n');
            perfMapFile.flush();
        }
    }
}

void PlatformAssemblerCommon::prepareCallWithArgCount(int argc)
{
#ifndef QT_NO_DEBUG
    Q_ASSERT(remainingArgcForCall == NoCall);
    remainingArgcForCall = argc;
#endif

    if (argc > ArgInRegCount) {
        argcOnStackForCall = int(WTF::roundUpToMultipleOf(16, size_t(argc - ArgInRegCount) * PointerSize));
        subPtr(TrustedImm32(argcOnStackForCall), StackPointerRegister);
    }
}

void PlatformAssemblerCommon::storeInstructionPointer(int instructionOffset)
{
    Address addr(CppStackFrameRegister, offsetof(QV4::CppStackFrame, instructionPointer));
    store32(TrustedImm32(instructionOffset), addr);
}

PlatformAssemblerCommon::Address PlatformAssemblerCommon::argStackAddress(int arg)
{
    int offset = arg - ArgInRegCount;
    Q_ASSERT(offset >= 0);
    return Address(StackPointerRegister, offset * PointerSize);
}

void PlatformAssemblerCommon::passAccumulatorAsArg(int arg)
{
#ifndef QT_NO_DEBUG
    Q_ASSERT(arg < remainingArgcForCall);
    --remainingArgcForCall;
#endif

    passAccumulatorAsArg_internal(arg, false);
}

void JIT::PlatformAssemblerCommon::pushAccumulatorAsArg(int arg)
{
    passAccumulatorAsArg_internal(arg, true);
}

void PlatformAssemblerCommon::passAccumulatorAsArg_internal(int arg, bool doPush)
{
    if (arg < ArgInRegCount) {
        addPtr(TrustedImm32(offsetof(CallData, accumulator)), JSStackFrameRegister, registerForArg(arg));
    } else {
        addPtr(TrustedImm32(offsetof(CallData, accumulator)), JSStackFrameRegister, ScratchRegister);
        if (doPush)
            push(ScratchRegister);
        else
            storePtr(ScratchRegister, argStackAddress(arg));
    }
}

void PlatformAssemblerCommon::passFunctionAsArg(int arg)
{
#ifndef QT_NO_DEBUG
    Q_ASSERT(arg < remainingArgcForCall);
    --remainingArgcForCall;
#endif

    if (arg < ArgInRegCount) {
        loadFunctionPtr(registerForArg(arg));
    } else {
        loadFunctionPtr(ScratchRegister);
        storePtr(ScratchRegister, argStackAddress(arg));
    }
}

void PlatformAssemblerCommon::passEngineAsArg(int arg)
{
#ifndef QT_NO_DEBUG
    Q_ASSERT(arg < remainingArgcForCall);
    --remainingArgcForCall;
#endif

    if (arg < ArgInRegCount) {
        move(EngineRegister, registerForArg(arg));
    } else {
        storePtr(EngineRegister, argStackAddress(arg));
    }
}

void PlatformAssemblerCommon::passJSSlotAsArg(int reg, int arg)
{
    Address addr(JSStackFrameRegister, reg * int(sizeof(QV4::Value)));
    passAddressAsArg(addr, arg);
}

void JIT::PlatformAssemblerCommon::passAddressAsArg(Address addr, int arg)
{
#ifndef QT_NO_DEBUG
    Q_ASSERT(arg < remainingArgcForCall);
    --remainingArgcForCall;
#endif

    if (arg < ArgInRegCount) {
        addPtr(TrustedImm32(addr.offset), addr.base, registerForArg(arg));
    } else {
        addPtr(TrustedImm32(addr.offset), addr.base, ScratchRegister);
        storePtr(ScratchRegister, argStackAddress(arg));
    }
}

void PlatformAssemblerCommon::passCppFrameAsArg(int arg)
{
#ifndef QT_NO_DEBUG
    Q_ASSERT(arg < remainingArgcForCall);
    --remainingArgcForCall;
#endif

    if (arg < ArgInRegCount)
        move(CppStackFrameRegister, registerForArg(arg));
    else
        store32(CppStackFrameRegister, argStackAddress(arg));
}

void PlatformAssemblerCommon::passInt32AsArg(int value, int arg)
{
#ifndef QT_NO_DEBUG
    Q_ASSERT(arg < remainingArgcForCall);
    --remainingArgcForCall;
#endif

    if (arg < ArgInRegCount)
        move(TrustedImm32(value), registerForArg(arg));
    else
        store32(TrustedImm32(value), argStackAddress(arg));
}

void PlatformAssemblerCommon::callRuntime(const char *functionName, const void *funcPtr)
{
#ifndef QT_NO_DEBUG
    Q_ASSERT(remainingArgcForCall == 0);
    remainingArgcForCall = NoCall;
#endif
    callRuntimeUnchecked(functionName, funcPtr);
    if (argcOnStackForCall > 0) {
        addPtr(TrustedImm32(argcOnStackForCall), StackPointerRegister);
        argcOnStackForCall = 0;
    }
}

void PlatformAssemblerCommon::callRuntimeUnchecked(const char *functionName, const void *funcPtr)
{
    functions.insert(funcPtr, functionName);
    callAbsolute(funcPtr);
}

void PlatformAssemblerCommon::tailCallRuntime(const char *functionName, const void *funcPtr)
{
    functions.insert(funcPtr, functionName);
    setTailCallArg(EngineRegister, 1);
    setTailCallArg(CppStackFrameRegister, 0);
    freeStackSpace();
    generatePlatformFunctionExit(/*tailCall =*/ true);
    jumpAbsolute(funcPtr);
}

void PlatformAssemblerCommon::setTailCallArg(RegisterID src, int arg)
{
    if (arg < ArgInRegCount) {
        move(src, registerForArg(arg));
    } else {
        // We never write to the incoming arguments space on the stack, and the tail call runtime
        // method has the same signature as the jitted function, so it is safe for us to just reuse
        // the arguments that we got in.
    }
}

JSC::MacroAssemblerBase::Address PlatformAssemblerCommon::jsAlloca(int slotCount)
{
    Address jsStackTopAddr(EngineRegister, offsetof(EngineBase, jsStackTop));
    RegisterID jsStackTop = AccumulatorRegisterValue;
    loadPtr(jsStackTopAddr, jsStackTop);
    addPtr(TrustedImm32(sizeof(Value) * slotCount), jsStackTop);
    storePtr(jsStackTop, jsStackTopAddr);
    return Address(jsStackTop, 0);
}

void PlatformAssemblerCommon::storeInt32AsValue(int srcInt, Address destAddr)
{
    store32(TrustedImm32(srcInt),
            Address(destAddr.base, destAddr.offset + QV4::Value::valueOffset()));
    store32(TrustedImm32(int(QV4::Value::ValueTypeInternal::Integer)),
            Address(destAddr.base, destAddr.offset + QV4::Value::tagOffset()));
}

} // JIT namespace
} // QV4 namepsace

QT_END_NAMESPACE

#endif // V4_ENABLE_JIT
