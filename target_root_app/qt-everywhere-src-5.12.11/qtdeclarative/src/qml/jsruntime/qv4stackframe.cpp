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
#include "qv4stackframe_p.h"
#include <QtCore/qstring.h>

using namespace  QV4;

QString CppStackFrame::source() const
{
    return v4Function ? v4Function->sourceFile() : QString();
}

QString CppStackFrame::function() const
{
    return v4Function ? v4Function->name()->toQString() : QString();
}

int CppStackFrame::lineNumber() const
{
    if (!v4Function)
        return -1;

    auto findLine = [](const CompiledData::CodeOffsetToLine &entry, uint offset) {
        return entry.codeOffset < offset;
    };

    const QV4::CompiledData::Function *cf = v4Function->compiledFunction;
    uint offset = instructionPointer;
    const CompiledData::CodeOffsetToLine *lineNumbers = cf->lineNumberTable();
    uint nLineNumbers = cf->nLineNumbers;
    const CompiledData::CodeOffsetToLine *line = std::lower_bound(lineNumbers, lineNumbers + nLineNumbers, offset, findLine) - 1;
    return line->line;
}

ReturnedValue CppStackFrame::thisObject() const {
    return jsFrame->thisObject.asReturnedValue();
}

