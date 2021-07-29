/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QMLMARKUPVISITOR_H
#define QMLMARKUPVISITOR_H

#include "node.h"
#include "tree.h"

#include <qstring.h>
#ifndef QT_NO_DECLARATIVE
#include <private/qqmljsastvisitor_p.h>
#include <private/qqmljsengine_p.h>
#endif

QT_BEGIN_NAMESPACE

#ifndef QT_NO_DECLARATIVE
class QmlMarkupVisitor : public QQmlJS::AST::Visitor
{
public:
    enum ExtraType{
        Comment,
        Pragma
    };

    QmlMarkupVisitor(const QString &code,
                     const QList<QQmlJS::AST::SourceLocation> &pragmas,
                     QQmlJS::Engine *engine);
    virtual ~QmlMarkupVisitor();

    QString markedUpCode();
    bool hasError() const;

    bool visit(QQmlJS::AST::UiImport *) override;
    void endVisit(QQmlJS::AST::UiImport *) override;

    bool visit(QQmlJS::AST::UiPublicMember *) override;
    bool visit(QQmlJS::AST::UiObjectDefinition *) override;

    bool visit(QQmlJS::AST::UiObjectInitializer *) override;
    void endVisit(QQmlJS::AST::UiObjectInitializer *) override;

    bool visit(QQmlJS::AST::UiObjectBinding *) override;
    bool visit(QQmlJS::AST::UiScriptBinding *) override;
    bool visit(QQmlJS::AST::UiArrayBinding *) override;
    bool visit(QQmlJS::AST::UiArrayMemberList *) override;
    bool visit(QQmlJS::AST::UiQualifiedId *) override;

    bool visit(QQmlJS::AST::ThisExpression *) override;
    bool visit(QQmlJS::AST::IdentifierExpression *) override;
    bool visit(QQmlJS::AST::NullExpression *) override;
    bool visit(QQmlJS::AST::TrueLiteral *) override;
    bool visit(QQmlJS::AST::FalseLiteral *) override;
    bool visit(QQmlJS::AST::NumericLiteral *) override;
    bool visit(QQmlJS::AST::StringLiteral *) override;
    bool visit(QQmlJS::AST::RegExpLiteral *) override;
    bool visit(QQmlJS::AST::ArrayPattern *) override;

    bool visit(QQmlJS::AST::ObjectPattern *) override;
    void endVisit(QQmlJS::AST::ObjectPattern *) override;

    bool visit(QQmlJS::AST::PatternElementList *) override;
    bool visit(QQmlJS::AST::Elision *) override;
    bool visit(QQmlJS::AST::PatternProperty *) override;
    bool visit(QQmlJS::AST::ArrayMemberExpression *) override;
    bool visit(QQmlJS::AST::FieldMemberExpression *) override;
    bool visit(QQmlJS::AST::NewMemberExpression *) override;
    bool visit(QQmlJS::AST::NewExpression *) override;
    bool visit(QQmlJS::AST::ArgumentList *) override;
    bool visit(QQmlJS::AST::PostIncrementExpression *) override;
    bool visit(QQmlJS::AST::PostDecrementExpression *) override;
    bool visit(QQmlJS::AST::DeleteExpression *) override;
    bool visit(QQmlJS::AST::VoidExpression *) override;
    bool visit(QQmlJS::AST::TypeOfExpression *) override;
    bool visit(QQmlJS::AST::PreIncrementExpression *) override;
    bool visit(QQmlJS::AST::PreDecrementExpression *) override;
    bool visit(QQmlJS::AST::UnaryPlusExpression *) override;
    bool visit(QQmlJS::AST::UnaryMinusExpression *) override;
    bool visit(QQmlJS::AST::TildeExpression *) override;
    bool visit(QQmlJS::AST::NotExpression *) override;
    bool visit(QQmlJS::AST::BinaryExpression *) override;
    bool visit(QQmlJS::AST::ConditionalExpression *) override;
    bool visit(QQmlJS::AST::Expression *) override;

    bool visit(QQmlJS::AST::Block *) override;
    void endVisit(QQmlJS::AST::Block *) override;

    bool visit(QQmlJS::AST::VariableStatement *) override;
    bool visit(QQmlJS::AST::VariableDeclarationList *) override;
    bool visit(QQmlJS::AST::EmptyStatement *) override;
    bool visit(QQmlJS::AST::ExpressionStatement *) override;
    bool visit(QQmlJS::AST::IfStatement *) override;
    bool visit(QQmlJS::AST::DoWhileStatement *) override;
    bool visit(QQmlJS::AST::WhileStatement *) override;
    bool visit(QQmlJS::AST::ForStatement *) override;
    bool visit(QQmlJS::AST::ForEachStatement *) override;
    bool visit(QQmlJS::AST::ContinueStatement *) override;
    bool visit(QQmlJS::AST::BreakStatement *) override;
    bool visit(QQmlJS::AST::ReturnStatement *) override;
    bool visit(QQmlJS::AST::WithStatement *) override;

    bool visit(QQmlJS::AST::CaseBlock *) override;
    void endVisit(QQmlJS::AST::CaseBlock *) override;

    bool visit(QQmlJS::AST::SwitchStatement *) override;
    bool visit(QQmlJS::AST::CaseClause *) override;
    bool visit(QQmlJS::AST::DefaultClause *) override;
    bool visit(QQmlJS::AST::LabelledStatement *) override;
    bool visit(QQmlJS::AST::ThrowStatement *) override;
    bool visit(QQmlJS::AST::TryStatement *) override;
    bool visit(QQmlJS::AST::Catch *) override;
    bool visit(QQmlJS::AST::Finally *) override;
    bool visit(QQmlJS::AST::FunctionDeclaration *) override;
    bool visit(QQmlJS::AST::FunctionExpression *) override;
    bool visit(QQmlJS::AST::FormalParameterList *) override;
    bool visit(QQmlJS::AST::DebuggerStatement *) override;

protected:
    QString protect(const QString &string);

private:
    typedef QHash<QString, QString> StringHash;
    void addExtra(quint32 start, quint32 finish);
    void addMarkedUpToken(QQmlJS::AST::SourceLocation &location,
                          const QString &text,
                          const StringHash &attributes = StringHash());
    void addVerbatim(QQmlJS::AST::SourceLocation first,
                     QQmlJS::AST::SourceLocation last = QQmlJS::AST::SourceLocation());
    QString sourceText(QQmlJS::AST::SourceLocation &location);
    void throwRecursionDepthError() final;

    QQmlJS::Engine *engine;
    QVector<ExtraType> extraTypes;
    QList<QQmlJS::AST::SourceLocation> extraLocations;
    QString source;
    QString output;
    quint32 cursor;
    int extraIndex;
    bool hasRecursionDepthError = false;
};
Q_DECLARE_TYPEINFO(QmlMarkupVisitor::ExtraType, Q_PRIMITIVE_TYPE);
#endif

QT_END_NAMESPACE

#endif
