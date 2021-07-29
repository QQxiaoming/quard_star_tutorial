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
#ifndef QV4COMPILERSCANFUNCTIONS_P_H
#define QV4COMPILERSCANFUNCTIONS_P_H

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

#include "private/qv4global_p.h"
#include <private/qqmljsastvisitor_p.h>
#include <private/qqmljsast_p.h>
#include <private/qqmljsengine_p.h>
#include <private/qv4compilercontext_p.h>
#include <private/qv4util_p.h>
#include <QtCore/QStringList>
#include <QStack>
#include <QScopedValueRollback>

QT_BEGIN_NAMESPACE

using namespace QQmlJS;

namespace QV4 {

namespace Moth {
struct Instruction;
}

namespace CompiledData {
struct CompilationUnit;
}

namespace Compiler {

class Codegen;

class ScanFunctions: protected QQmlJS::AST::Visitor
{
    typedef QScopedValueRollback<bool> TemporaryBoolAssignment;
public:
    ScanFunctions(Codegen *cg, const QString &sourceCode, ContextType defaultProgramType);
    void operator()(AST::Node *node);

    void enterGlobalEnvironment(ContextType compilationMode);
    void enterEnvironment(AST::Node *node, ContextType compilationMode, const QString &name);
    void leaveEnvironment();

    void enterQmlFunction(AST::FunctionDeclaration *ast)
    { enterFunction(ast, false); }

protected:
    using Visitor::visit;
    using Visitor::endVisit;

    void checkDirectivePrologue(AST::StatementList *ast);

    void checkName(const QStringRef &name, const AST::SourceLocation &loc);

    bool visit(AST::Program *ast) override;
    void endVisit(AST::Program *) override;

    bool visit(AST::ESModule *ast) override;
    void endVisit(AST::ESModule *) override;

    bool visit(AST::ExportDeclaration *declaration) override;
    bool visit(AST::ImportDeclaration *declaration) override;

    bool visit(AST::CallExpression *ast) override;
    bool visit(AST::PatternElement *ast) override;
    bool visit(AST::IdentifierExpression *ast) override;
    bool visit(AST::ExpressionStatement *ast) override;
    bool visit(AST::FunctionExpression *ast) override;
    bool visit(AST::TemplateLiteral *ast) override;
    bool visit(AST::SuperLiteral *) override;
    bool visit(AST::FieldMemberExpression *) override;
    bool visit(AST::ArrayPattern *) override;

    bool enterFunction(AST::FunctionExpression *ast, bool enterName);

    void endVisit(AST::FunctionExpression *) override;

    bool visit(AST::ObjectPattern *ast) override;

    bool visit(AST::PatternProperty *ast) override;
    void endVisit(AST::PatternProperty *) override;

    bool visit(AST::FunctionDeclaration *ast) override;
    void endVisit(AST::FunctionDeclaration *) override;

    bool visit(AST::ClassExpression *ast) override;
    void endVisit(AST::ClassExpression *) override;

    bool visit(AST::ClassDeclaration *ast) override;
    void endVisit(AST::ClassDeclaration *) override;

    bool visit(AST::DoWhileStatement *ast) override;
    bool visit(AST::ForStatement *ast) override;
    void endVisit(AST::ForStatement *) override;
    bool visit(AST::ForEachStatement *ast) override;
    void endVisit(AST::ForEachStatement *) override;

    bool visit(AST::ThisExpression *ast) override;

    bool visit(AST::Block *ast) override;
    void endVisit(AST::Block *ast) override;

    bool visit(AST::CaseBlock *ast) override;
    void endVisit(AST::CaseBlock *ast) override;

    bool visit(AST::Catch *ast) override;
    void endVisit(AST::Catch *ast) override;

    bool visit(AST::WithStatement *ast) override;
    void endVisit(AST::WithStatement *ast) override;

    void throwRecursionDepthError() override;

protected:
    bool enterFunction(AST::Node *ast, const QString &name, AST::FormalParameterList *formals, AST::StatementList *body, bool enterName);

    void calcEscapingVariables();
// fields:
    Codegen *_cg;
    const QString _sourceCode;
    Context *_context;
    QStack<Context *> _contextStack;

    bool _allowFuncDecls;
    ContextType defaultProgramType;

private:
    static constexpr AST::Node *astNodeForGlobalEnvironment = nullptr;
};

}

}

QT_END_NAMESPACE

#endif // QV4CODEGEN_P_H
