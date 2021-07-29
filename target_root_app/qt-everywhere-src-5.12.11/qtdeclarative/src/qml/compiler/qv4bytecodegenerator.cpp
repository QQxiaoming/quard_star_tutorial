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

#include <private/qv4bytecodegenerator_p.h>
#include <private/qv4compilercontext_p.h>
#include <private/qqmljsastfwd_p.h>

QT_USE_NAMESPACE
using namespace QV4;
using namespace Moth;

void BytecodeGenerator::setLocation(const QQmlJS::AST::SourceLocation &loc)
{
    currentLine = static_cast<int>(loc.startLine);
}

int BytecodeGenerator::newRegister()
{
    int t = currentReg++;
    if (regCount < currentReg)
        regCount = currentReg;
    return t;
}

int BytecodeGenerator::newRegisterArray(int n)
{
    int t = currentReg;
    currentReg += n;
    if (regCount < currentReg)
        regCount = currentReg;
    return t;
}

void BytecodeGenerator::packInstruction(I &i)
{
    Instr::Type type = Instr::unpack(i.packed);
    Q_ASSERT(int(type) < MOTH_NUM_INSTRUCTIONS());
    type = Instr::narrowInstructionType(type);
    int instructionsAsInts[sizeof(Instr)/sizeof(int)] = {};
    int nMembers = Moth::InstrInfo::argumentCount[static_cast<int>(i.type)];
    uchar *code = i.packed + Instr::encodedLength(type);
    for (int j = 0; j < nMembers; ++j) {
        instructionsAsInts[j] = qFromLittleEndian<qint32>(code + j * sizeof(int));
    }
    enum {
        Normal,
        Wide
    } width = Normal;
    for (int n = 0; n < nMembers; ++n) {
        if (width == Normal && (static_cast<qint8>(instructionsAsInts[n]) != instructionsAsInts[n])) {
            width = Wide;
            break;
        }
    }
    code = i.packed;
    switch (width) {
    case Normal:
        code = Instr::pack(code, type);
        for (int n = 0; n < nMembers; ++n) {
            qint8 v = static_cast<qint8>(instructionsAsInts[n]);
            memcpy(code, &v, 1);
            code += 1;
        }
        i.size = code - i.packed;
        if (i.offsetForJump != -1)
            i.offsetForJump = i.size - 1;
        break;
    case Wide:
        // nothing to do
        break;
    }
}

void BytecodeGenerator::adjustJumpOffsets()
{
    for (int index = 0; index < instructions.size(); ++index) {
        auto &i = instructions[index];
        if (i.offsetForJump == -1) // no jump
            continue;
        Q_ASSERT(i.linkedLabel != -1 && labels.at(i.linkedLabel) != -1);
        const auto &linkedInstruction = instructions.at(labels.at(i.linkedLabel));
        qint8 *c = reinterpret_cast<qint8*>(i.packed + i.offsetForJump);
        int jumpOffset = linkedInstruction.position - (i.position + i.size);
//        qDebug() << "adjusting jump offset for instruction" << index << i.position << i.size << "offsetForJump" << i.offsetForJump << "target"
//                 << labels.at(i.linkedLabel) << linkedInstruction.position << "jumpOffset" << jumpOffset;
        Instr::Type type = Instr::unpack(i.packed);
        if (Instr::isWide(type)) {
            Q_ASSERT(i.offsetForJump == i.size - 4);
            qToLittleEndian<qint32>(jumpOffset, c);
        } else {
            Q_ASSERT(i.offsetForJump == i.size - 1);
            qint8 o = jumpOffset;
            Q_ASSERT(o == jumpOffset);
            *c = o;
        }
    }
}

void BytecodeGenerator::compressInstructions()
{
    // first round: compress all non jump instructions
    int position = 0;
    for (auto &i : instructions) {
        i.position = position;
        if (i.offsetForJump == -1)
            packInstruction(i);
        position += i.size;
    }

    adjustJumpOffsets();

    // compress all jumps
    position = 0;
    for (auto &i : instructions) {
        i.position = position;
        if (i.offsetForJump != -1)
            packInstruction(i);
        position += i.size;
    }

    // adjust once again, as the packing above could have changed offsets
    adjustJumpOffsets();
}

void BytecodeGenerator::finalize(Compiler::Context *context)
{
    compressInstructions();

    // collect content and line numbers
    QByteArray code;
    QVector<CompiledData::CodeOffsetToLine> lineNumbers;
    currentLine = -1;
    Q_UNUSED(startLine);
    for (const auto &i : qAsConst(instructions)) {
        if (i.line != currentLine) {
            currentLine = i.line;
            CompiledData::CodeOffsetToLine entry;
            entry.codeOffset = code.size();
            entry.line = currentLine;
            lineNumbers.append(entry);
        }
        code.append(reinterpret_cast<const char *>(i.packed), i.size);
    }

    context->code = code;
    context->lineNumberMapping = lineNumbers;
}

int BytecodeGenerator::addInstructionHelper(Instr::Type type, const Instr &i, int offsetOfOffset) {
    if (lastInstrType == int(Instr::Type::StoreReg)) {
        if (type == Instr::Type::LoadReg) {
            if (i.LoadReg.reg == lastInstr.StoreReg.reg) {
                // value is already in the accumulator
                return -1;
            }
        }
        if (type == Instr::Type::MoveReg) {
            if (i.MoveReg.srcReg == lastInstr.StoreReg.reg) {
                Instruction::StoreReg store;
                store.reg = i.MoveReg.destReg;
                addInstruction(store);
                return -1;
            }
        }
    }
    lastInstrType = int(type);
    lastInstr = i;

#if QT_CONFIG(qml_debug)
    if (debugMode && type != Instr::Type::Debug) {
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmaybe-uninitialized") // broken gcc warns about Instruction::Debug()
        if (instructions.isEmpty() || currentLine != instructions.constLast().line) {
            addInstruction(Instruction::Debug());
        } else if (type == Instr::Type::Ret) {
            currentLine = -currentLine;
            addInstruction(Instruction::Debug());
            currentLine = -currentLine;
        }
QT_WARNING_POP
    }
#else
    Q_UNUSED(debugMode);
#endif

    const int pos = instructions.size();

    const int argCount = Moth::InstrInfo::argumentCount[static_cast<int>(type)];
    int s = argCount*sizeof(int);
    if (offsetOfOffset != -1)
        offsetOfOffset += Instr::encodedLength(type);
    I instr{type, static_cast<short>(s + Instr::encodedLength(type)), 0, currentLine, offsetOfOffset, -1, "\0\0" };
    uchar *code = instr.packed;
    code = Instr::pack(code, Instr::wideInstructionType(type));

    for (int j = 0; j < argCount; ++j) {
        qToLittleEndian<qint32>(i.argumentsAsInts[j], code);
        code += sizeof(int);
    }

    instructions.append(instr);

    return pos;
}
