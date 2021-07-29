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

#include "qv4runtimecodegen_p.h"
#include "qv4compilerscanfunctions_p.h"

using namespace QV4;

void RuntimeCodegen::generateFromFunctionExpression(const QString &fileName,
                                                    const QString &sourceCode,
                                                    AST::FunctionExpression *ast,
                                                    Compiler::Module *module)
{
    _module = module;
    _module->fileName = fileName;
    _module->finalUrl = fileName;
    _context = nullptr;

    Compiler::ScanFunctions scan(this, sourceCode, Compiler::ContextType::Global);
    // fake a global environment
    scan.enterEnvironment(nullptr, Compiler::ContextType::Function, QString());
    scan(ast);
    scan.leaveEnvironment();

    if (hasError)
        return;

    int index = defineFunction(ast->name.toString(), ast, ast->formals, ast->body);
    _module->rootContext = _module->functions.at(index);
}

void RuntimeCodegen::throwSyntaxError(const AST::SourceLocation &loc, const QString &detail)
{
    if (hasError)
        return;
    hasError = true;
    engine->throwSyntaxError(detail, _module->fileName, loc.startLine, loc.startColumn);
}

void RuntimeCodegen::throwReferenceError(const AST::SourceLocation &loc, const QString &detail)
{
    if (hasError)
        return;
    hasError = true;
    engine->throwReferenceError(detail, _module->fileName, loc.startLine, loc.startColumn);
}

