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

#include "qv4codegen_p.h"
#include "qv4util_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QStack>
#include <QScopeGuard>
#include <private/qqmljsast_p.h>
#include <private/qv4string_p.h>
#include <private/qv4value_p.h>
#include <private/qv4compilercontext_p.h>
#include <private/qv4compilercontrolflow_p.h>
#include <private/qv4bytecodegenerator_p.h>
#include <private/qv4compilerscanfunctions_p.h>

#ifndef V4_BOOTSTRAP
#  include <qqmlerror.h>
#endif

#include <cmath>
#include <iostream>

static const bool disable_lookups = false;

#ifdef CONST
#undef CONST
#endif

QT_USE_NAMESPACE
using namespace QV4;
using namespace QV4::Compiler;
using namespace QQmlJS::AST;

static inline void setJumpOutLocation(QV4::Moth::BytecodeGenerator *bytecodeGenerator,
                                      const Statement *body, const SourceLocation &fallback)
{
    switch (body->kind) {
    // Statements where we might never execute the last line.
    // Use the fallback.
    case Statement::Kind_ConditionalExpression:
    case Statement::Kind_ForEachStatement:
    case Statement::Kind_ForStatement:
    case Statement::Kind_IfStatement:
    case Statement::Kind_WhileStatement:
        bytecodeGenerator->setLocation(fallback);
        break;
    default:
        bytecodeGenerator->setLocation(body->lastSourceLocation());
        break;
    }
}

Codegen::Codegen(QV4::Compiler::JSUnitGenerator *jsUnitGenerator, bool strict)
    : _module(nullptr)
    , _returnAddress(-1)
    , _context(nullptr)
    , _labelledStatement(nullptr)
    , jsUnitGenerator(jsUnitGenerator)
    , _strictMode(strict)
    , _fileNameIsUrl(false)
    , hasError(false)
{
    jsUnitGenerator->codeGeneratorName = QStringLiteral("moth");
    pushExpr();
}

const char *Codegen::s_globalNames[] = {
    "isNaN",
    "parseFloat",
    "String",
    "EvalError",
    "URIError",
    "Math",
    "encodeURIComponent",
    "RangeError",
    "eval",
    "isFinite",
    "ReferenceError",
    "Infinity",
    "Function",
    "RegExp",
    "Number",
    "parseInt",
    "Object",
    "decodeURI",
    "TypeError",
    "Boolean",
    "encodeURI",
    "NaN",
    "Error",
    "decodeURIComponent",
    "Date",
    "Array",
    "Symbol",
    "escape",
    "unescape",
    "SyntaxError",
    "undefined",
    "JSON",
    "ArrayBuffer",
    "SharedArrayBuffer",
    "DataView",
    "Int8Array",
    "Uint8Array",
    "Uint8ClampedArray",
    "Int16Array",
    "Uint16Array",
    "Int32Array",
    "Uint32Array",
    "Float32Array",
    "Float64Array",
    "WeakSet",
    "Set",
    "WeakMap",
    "Map",
    "Reflect",
    "Proxy",
    "Atomics",
    "Promise",
    nullptr
};

void Codegen::generateFromProgram(const QString &fileName,
                                  const QString &finalUrl,
                                  const QString &sourceCode,
                                  Program *node,
                                  Module *module,
                                  ContextType contextType)
{
    Q_ASSERT(node);

    _module = module;
    _context = nullptr;

    // ### should be set on the module outside of this method
    _module->fileName = fileName;
    _module->finalUrl = finalUrl;

    if (contextType == ContextType::ScriptImportedByQML) {
        // the global object is frozen, so we know that members of it are
        // pointing to the global object. This is important so that references
        // to Math etc. do not go through the expensive path in the context wrapper
        // that tries to see whether we have a matching type
        //
        // Since this can be called from the loader thread we can't get the list
        // directly from the engine, so let's hardcode the most important ones here
        for (const char **g = s_globalNames; *g != nullptr; ++g)
            m_globalNames << QString::fromLatin1(*g);
    }

    ScanFunctions scan(this, sourceCode, contextType);
    scan(node);

    if (hasError)
        return;

    defineFunction(QStringLiteral("%entry"), node, nullptr, node->statements);
}

void Codegen::generateFromModule(const QString &fileName,
                                 const QString &finalUrl,
                                 const QString &sourceCode,
                                 ESModule *node,
                                 Module *module)
{
    Q_ASSERT(node);

    _module = module;
    _context = nullptr;

    // ### should be set on the module outside of this method
    _module->fileName = fileName;
    _module->finalUrl = finalUrl;

    ScanFunctions scan(this, sourceCode, ContextType::ESModule);
    scan(node);

    if (hasError)
        return;

    {
        Compiler::Context *moduleContext = _module->contextMap.value(node);
        for (const auto &entry: moduleContext->exportEntries) {
            if (entry.moduleRequest.isEmpty()) {
                // ### check against imported bound names
                _module->localExportEntries << entry;
            } else if (entry.importName == QLatin1Char('*')) {
                _module->starExportEntries << entry;
            } else {
                _module->indirectExportEntries << entry;
            }
        }
        _module->importEntries = moduleContext->importEntries;

        _module->moduleRequests = std::move(moduleContext->moduleRequests);
        _module->moduleRequests.removeDuplicates();
    }

    std::sort(_module->localExportEntries.begin(), _module->localExportEntries.end(), ExportEntry::lessThan);
    std::sort(_module->starExportEntries.begin(), _module->starExportEntries.end(), ExportEntry::lessThan);
    std::sort(_module->indirectExportEntries.begin(), _module->indirectExportEntries.end(), ExportEntry::lessThan);

    defineFunction(QStringLiteral("%entry"), node, nullptr, node->body);
}

void Codegen::enterContext(Node *node)
{
    _context = _module->contextMap.value(node);
    Q_ASSERT(_context);
}

int Codegen::leaveContext()
{
    Q_ASSERT(_context);
    int functionIndex = _context->functionIndex;
    _context = _context->parent;
    return functionIndex;
}

Context *Codegen::enterBlock(Node *node)
{
    enterContext(node);
    return _context;
}

Codegen::Reference Codegen::unop(UnaryOperation op, const Reference &expr)
{
    if (hasError)
        return exprResult();

    if (expr.isConstant()) {
        auto v = Value::fromReturnedValue(expr.constant);
        if (v.isNumber()) {
            switch (op) {
            case Not:
                return Reference::fromConst(this, Encode(!v.toBoolean()));
            case UMinus:
                return Reference::fromConst(this, Runtime::method_uMinus(v));
            case UPlus:
                return expr;
            case Compl:
                return Reference::fromConst(this, Encode((int)~v.toInt32()));
            default:
                break;
            }
        }
    }

    switch (op) {
    case UMinus: {
        expr.loadInAccumulator();
        Instruction::UMinus uminus;
        bytecodeGenerator->addInstruction(uminus);
        return Reference::fromAccumulator(this);
    }
    case UPlus: {
        expr.loadInAccumulator();
        Instruction::UPlus uplus;
        bytecodeGenerator->addInstruction(uplus);
        return Reference::fromAccumulator(this);
    }
    case Not: {
        expr.loadInAccumulator();
        Instruction::UNot unot;
        bytecodeGenerator->addInstruction(unot);
        return Reference::fromAccumulator(this);
    }
    case Compl: {
        expr.loadInAccumulator();
        Instruction::UCompl ucompl;
        bytecodeGenerator->addInstruction(ucompl);
        return Reference::fromAccumulator(this);
    }
    case PostIncrement:
        if (!exprAccept(nx) || requiresReturnValue) {
            Reference e = expr.asLValue();
            e.loadInAccumulator();
            Instruction::UPlus uplus;
            bytecodeGenerator->addInstruction(uplus);
            Reference originalValue = Reference::fromStackSlot(this).storeRetainAccumulator();
            Instruction::Increment inc;
            bytecodeGenerator->addInstruction(inc);
            e.storeConsumeAccumulator();
            return originalValue;
        } else {
            // intentionally fall-through: the result is never used, so it's equivalent to
            // "expr += 1", which is what a pre-increment does as well.
            Q_FALLTHROUGH();
        }
    case PreIncrement: {
        Reference e = expr.asLValue();
        e.loadInAccumulator();
        Instruction::Increment inc;
        bytecodeGenerator->addInstruction(inc);
        if (exprAccept(nx))
            return e.storeConsumeAccumulator();
        else
            return e.storeRetainAccumulator();
    }
    case PostDecrement:
        if (!exprAccept(nx) || requiresReturnValue) {
            Reference e = expr.asLValue();
            e.loadInAccumulator();
            Instruction::UPlus uplus;
            bytecodeGenerator->addInstruction(uplus);
            Reference originalValue = Reference::fromStackSlot(this).storeRetainAccumulator();
            Instruction::Decrement dec;
            bytecodeGenerator->addInstruction(dec);
            e.storeConsumeAccumulator();
            return originalValue;
        } else {
            // intentionally fall-through: the result is never used, so it's equivalent to
            // "expr -= 1", which is what a pre-decrement does as well.
            Q_FALLTHROUGH();
        }
    case PreDecrement: {
        Reference e = expr.asLValue();
        e.loadInAccumulator();
        Instruction::Decrement dec;
        bytecodeGenerator->addInstruction(dec);
        if (exprAccept(nx))
            return e.storeConsumeAccumulator();
        else
            return e.storeRetainAccumulator();
    }
    }

    Q_UNREACHABLE();
}

void Codegen::addCJump()
{
    const Result &expression = currentExpr();
    bytecodeGenerator->addCJumpInstruction(expression.trueBlockFollowsCondition(),
                                           expression.iftrue(), expression.iffalse());
}

void Codegen::statement(Statement *ast)
{
    RegisterScope scope(this);

    bytecodeGenerator->setLocation(ast->firstSourceLocation());

    VolatileMemoryLocations vLocs = scanVolatileMemoryLocations(ast);
    qSwap(_volatileMemoryLocations, vLocs);
    accept(ast);
    qSwap(_volatileMemoryLocations, vLocs);
}

void Codegen::statement(ExpressionNode *ast)
{
    if (! ast) {
        return;
    } else {
        RegisterScope scope(this);

        pushExpr(Result(nx));
        VolatileMemoryLocations vLocs = scanVolatileMemoryLocations(ast);
        qSwap(_volatileMemoryLocations, vLocs);

        accept(ast);

        qSwap(_volatileMemoryLocations, vLocs);
        Reference result = popResult();

        if (hasError)
            return;
        if (result.loadTriggersSideEffect())
            result.loadInAccumulator(); // triggers side effects
    }
}

void Codegen::condition(ExpressionNode *ast, const BytecodeGenerator::Label *iftrue,
                        const BytecodeGenerator::Label *iffalse, bool trueBlockFollowsCondition)
{
    if (hasError)
        return;

    if (!ast)
        return;

    pushExpr(Result(iftrue, iffalse, trueBlockFollowsCondition));
    accept(ast);
    Result r = popExpr();

    if (hasError)
        return;

    if (r.format() == ex) {
        Q_ASSERT(iftrue == r.iftrue());
        Q_ASSERT(iffalse == r.iffalse());
        Q_ASSERT(r.result().isValid());
        bytecodeGenerator->setLocation(ast->firstSourceLocation());
        r.result().loadInAccumulator();
        if (r.trueBlockFollowsCondition())
            bytecodeGenerator->jumpFalse().link(*r.iffalse());
        else
            bytecodeGenerator->jumpTrue().link(*r.iftrue());
    }
}

void Codegen::program(Program *ast)
{
    if (ast) {
        statementList(ast->statements);
    }
}

enum class CompletionState {
    Empty,
    EmptyAbrupt,
    NonEmpty
};

static CompletionState completionState(StatementList *list)
{
    for (StatementList *it = list; it; it = it->next) {
        if (it->statement->kind == Statement::Kind_BreakStatement ||
            it->statement->kind == Statement::Kind_ContinueStatement)
            return CompletionState::EmptyAbrupt;
        if (it->statement->kind == Statement::Kind_EmptyStatement ||
            it->statement->kind == Statement::Kind_VariableDeclaration ||
            it->statement->kind == Statement::Kind_FunctionDeclaration)
            continue;
        if (it->statement->kind == Statement::Kind_Block) {
            CompletionState subState = completionState(static_cast<Block *>(it->statement)->statements);
            if (subState != CompletionState::Empty)
                return subState;
            continue;
        }
        return CompletionState::NonEmpty;
    }
    return CompletionState::Empty;
}

static Node *completionStatement(StatementList *list)
{
    Node *completionStatement = nullptr;
    for (StatementList *it = list; it; it = it->next) {
        if (it->statement->kind == Statement::Kind_BreakStatement ||
            it->statement->kind == Statement::Kind_ContinueStatement)
            return completionStatement;
        if (it->statement->kind == Statement::Kind_ThrowStatement ||
            it->statement->kind == Statement::Kind_ReturnStatement)
            return it->statement;
        if (it->statement->kind == Statement::Kind_EmptyStatement ||
            it->statement->kind == Statement::Kind_VariableStatement ||
            it->statement->kind == Statement::Kind_FunctionDeclaration)
            continue;
        if (it->statement->kind == Statement::Kind_Block) {
            CompletionState state = completionState(static_cast<Block *>(it->statement)->statements);
            switch (state) {
            case CompletionState::Empty:
                continue;
            case CompletionState::EmptyAbrupt:
                return it->statement;
            case CompletionState::NonEmpty:
                break;
            }
        }
        completionStatement = it->statement;
    }
    return completionStatement;
}

void Codegen::statementList(StatementList *ast)
{
    if (!ast)
        return;

    bool _requiresReturnValue = requiresReturnValue;
    // ### the next line is pessimizing a bit too much, as there are many cases, where the complietion from the break
    // statement will not be used, but it's at least spec compliant
    if (!controlFlow || !controlFlow->hasLoop())
        requiresReturnValue = false;

    Node *needsCompletion = nullptr;

    if (_requiresReturnValue && !requiresReturnValue)
        needsCompletion = completionStatement(ast);

    if (requiresReturnValue && !needsCompletion && !insideSwitch) {
        // break or continue is the first real statement, set the return value to undefined
        Reference::fromConst(this, Encode::undefined()).storeOnStack(_returnAddress);
    }

    bool _insideSwitch = insideSwitch;
    insideSwitch = false;

    for (StatementList *it = ast; it; it = it->next) {
        if (it->statement == needsCompletion)
            requiresReturnValue = true;
        if (Statement *s = it->statement->statementCast())
            statement(s);
        else
            statement(static_cast<ExpressionNode *>(it->statement));
        if (it->statement == needsCompletion)
            requiresReturnValue = false;
        if (it->statement->kind == Statement::Kind_ThrowStatement ||
            it->statement->kind == Statement::Kind_BreakStatement ||
            it->statement->kind == Statement::Kind_ContinueStatement ||
            it->statement->kind == Statement::Kind_ReturnStatement)
            // any code after those statements is unreachable
            break;
    }
    requiresReturnValue = _requiresReturnValue;
    insideSwitch = _insideSwitch;
}

void Codegen::variableDeclaration(PatternElement *ast)
{
    TailCallBlocker blockTailCalls(this);
    RegisterScope scope(this);

    if (!ast->initializer) {
        if (ast->isLexicallyScoped()) {
            Reference::fromConst(this, Encode::undefined()).loadInAccumulator();
            Reference varToStore = targetForPatternElement(ast);
            varToStore.storeConsumeAccumulator();
        }
        return;
    }
    initializeAndDestructureBindingElement(ast, Reference(), /*isDefinition*/ true);
}

void Codegen::variableDeclarationList(VariableDeclarationList *ast)
{
    for (VariableDeclarationList *it = ast; it; it = it->next) {
        variableDeclaration(it->declaration);
    }
}

Codegen::Reference Codegen::targetForPatternElement(AST::PatternElement *p)
{
    if (!p->bindingIdentifier.isNull())
        return referenceForName(p->bindingIdentifier.toString(), true, p->firstSourceLocation());
    if (!p->bindingTarget || p->destructuringPattern())
        return Codegen::Reference::fromStackSlot(this);
    Reference lhs = expression(p->bindingTarget);
    if (hasError)
        return lhs;
    if (!lhs.isLValue()) {
        throwReferenceError(p->bindingTarget->firstSourceLocation(), QStringLiteral("Binding target is not a reference."));
        return lhs;
    }
    lhs = lhs.asLValue();
    return lhs;
}

void Codegen::initializeAndDestructureBindingElement(AST::PatternElement *e, const Reference &base, bool isDefinition)
{
    Q_ASSERT(e->type == AST::PatternElement::Binding || e->type == AST::PatternElement::RestElement);
    RegisterScope scope(this);
    Reference baseRef = (base.isAccumulator()) ? base.storeOnStack() : base;
    Reference varToStore = targetForPatternElement(e);
    if (isDefinition)
        varToStore.isReferenceToConst = false;
    if (hasError)
        return;

    if (e->initializer) {
        if (!baseRef.isValid()) {
            // assignment
            Reference expr = expression(e->initializer);
            if (hasError)
                return;
            expr.loadInAccumulator();
            varToStore.storeConsumeAccumulator();
        } else if (baseRef == varToStore) {
            baseRef.loadInAccumulator();
            BytecodeGenerator::Jump jump = bytecodeGenerator->jumpNotUndefined();
            Reference expr = expression(e->initializer);
            if (hasError) {
                jump.link();
                return;
            }
            expr.loadInAccumulator();
            varToStore.storeConsumeAccumulator();
            jump.link();
        } else {
            baseRef.loadInAccumulator();
            BytecodeGenerator::Jump jump = bytecodeGenerator->jumpNotUndefined();
            Reference expr = expression(e->initializer);
            if (hasError) {
                jump.link();
                return;
            }
            expr.loadInAccumulator();
            jump.link();
            varToStore.storeConsumeAccumulator();
        }
    } else if (baseRef != varToStore && baseRef.isValid()) {
        baseRef.loadInAccumulator();
        varToStore.storeConsumeAccumulator();
    }
    Pattern *p = e->destructuringPattern();
    if (!p)
        return;

    if (!varToStore.isStackSlot())
        varToStore = varToStore.storeOnStack();
    if (PatternElementList *l = e->elementList()) {
        destructureElementList(varToStore, l, isDefinition);
    } else if (PatternPropertyList *p = e->propertyList()) {
        destructurePropertyList(varToStore, p, isDefinition);
    } else if (e->bindingTarget) {
        // empty binding pattern. For spec compatibility, try to coerce the argument to an object
        varToStore.loadInAccumulator();
        Instruction::ToObject toObject;
        bytecodeGenerator->addInstruction(toObject);
        return;
    }
}

Codegen::Reference Codegen::referenceForPropertyName(const Codegen::Reference &object, AST::PropertyName *name)
{
    AST::ComputedPropertyName *cname = AST::cast<AST::ComputedPropertyName *>(name);
    Reference property;
    if (cname) {
        Reference computedName = expression(cname->expression);
        if (hasError)
            return Reference();
        computedName = computedName.storeOnStack();
        property = Reference::fromSubscript(object, computedName).asLValue();
    } else {
        QString propertyName = name->asString();
        property = Reference::fromMember(object, propertyName);
    }
    return property;
}

void Codegen::destructurePropertyList(const Codegen::Reference &object, PatternPropertyList *bindingList, bool isDefinition)
{
    RegisterScope scope(this);

    object.loadInAccumulator();
    Instruction::ThrowOnNullOrUndefined t;
    bytecodeGenerator->addInstruction(t);

    for (PatternPropertyList *it = bindingList; it; it = it->next) {
        PatternProperty *p = it->property;
        RegisterScope scope(this);
        Reference property = referenceForPropertyName(object, p->name);
        if (hasError)
            return;
        initializeAndDestructureBindingElement(p, property, isDefinition);
        if (hasError)
            return;
    }
}

void Codegen::destructureElementList(const Codegen::Reference &array, PatternElementList *bindingList, bool isDefinition)
{
    RegisterScope scope(this);

    Reference iterator = Reference::fromStackSlot(this);
    Reference iteratorValue = Reference::fromStackSlot(this);
    Reference iteratorDone = Reference::fromStackSlot(this);
    Reference::storeConstOnStack(this, Encode(false), iteratorDone.stackSlot());

    array.loadInAccumulator();
    Instruction::GetIterator iteratorObjInstr;
    iteratorObjInstr.iterator = static_cast<int>(AST::ForEachType::Of);
    bytecodeGenerator->addInstruction(iteratorObjInstr);
    iterator.storeConsumeAccumulator();

    {
        auto cleanup = [this, iterator, iteratorDone]() {
            iterator.loadInAccumulator();
            Instruction::IteratorClose close;
            close.done = iteratorDone.stackSlot();
            bytecodeGenerator->addInstruction(close);
        };

        ControlFlowUnwindCleanup flow(this, cleanup);

        for (PatternElementList *p = bindingList; p; p = p->next) {
            PatternElement *e = p->element;
            for (Elision *elision = p->elision; elision; elision = elision->next) {
                iterator.loadInAccumulator();
                Instruction::IteratorNext next;
                next.value = iteratorValue.stackSlot();
                next.done = iteratorDone.stackSlot();
                bytecodeGenerator->addInstruction(next);
            }

            if (!e)
                continue;

            RegisterScope scope(this);
            iterator.loadInAccumulator();

            if (e->type == PatternElement::RestElement) {
                Reference::fromConst(this, Encode(true)).storeOnStack(iteratorDone.stackSlot());
                bytecodeGenerator->addInstruction(Instruction::DestructureRestElement());
                initializeAndDestructureBindingElement(e, Reference::fromAccumulator(this), isDefinition);
            } else {
                Instruction::IteratorNext next;
                next.value = iteratorValue.stackSlot();
                next.done = iteratorDone.stackSlot();
                bytecodeGenerator->addInstruction(next);
                initializeAndDestructureBindingElement(e, iteratorValue, isDefinition);
                if (hasError)
                    return;
            }
        }
    }
}

void Codegen::destructurePattern(Pattern *p, const Reference &rhs)
{
    RegisterScope scope(this);
    if (auto *o = AST::cast<ObjectPattern *>(p))
        destructurePropertyList(rhs, o->properties);
    else if (auto *a = AST::cast<ArrayPattern *>(p))
        destructureElementList(rhs, a->elements);
    else
        Q_UNREACHABLE();
}


bool Codegen::visit(ArgumentList *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(CaseBlock *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(CaseClause *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(CaseClauses *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(Catch *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(DefaultClause *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(Elision *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(Finally *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(FormalParameterList *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(Program *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(PatternElement *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(PatternElementList *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(PatternProperty *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(PatternPropertyList *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(ExportDeclaration *ast)
{
    if (!ast->exportDefault)
        return true;

    TailCallBlocker blockTailCalls(this);
    Reference exportedValue;

    if (auto *fdecl = AST::cast<FunctionDeclaration*>(ast->variableStatementOrDeclaration)) {
        pushExpr();
        visit(static_cast<FunctionExpression*>(fdecl));
        exportedValue = popResult();
    } else if (auto *classDecl = AST::cast<ClassDeclaration*>(ast->variableStatementOrDeclaration)) {
        pushExpr();
        visit(static_cast<ClassExpression*>(classDecl));
        exportedValue = popResult();
    } else if (ExpressionNode *expr = ast->variableStatementOrDeclaration->expressionCast()) {
        exportedValue = expression(expr);
    }

    exportedValue.loadInAccumulator();

    const int defaultExportIndex = _context->locals.indexOf(_context->localNameForDefaultExport);
    Q_ASSERT(defaultExportIndex != -1);
    Reference defaultExportSlot = Reference::fromScopedLocal(this, defaultExportIndex, /*scope*/0);
    defaultExportSlot.storeConsumeAccumulator();

    return false;
}

bool Codegen::visit(StatementList *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(UiArrayMemberList *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(UiImport *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(UiHeaderItemList *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(UiPragma *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(UiObjectInitializer *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(UiObjectMemberList *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(UiParameterList *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(UiProgram *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(UiQualifiedId *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(VariableDeclarationList *)
{
    Q_UNREACHABLE();
    return false;
}

bool Codegen::visit(ClassExpression *ast)
{
    TailCallBlocker blockTailCalls(this);

    Compiler::Class jsClass;
    jsClass.nameIndex = registerString(ast->name.toString());

    ClassElementList *constructor = nullptr;
    int nComputedNames = 0;
    int nStaticComputedNames = 0;

    RegisterScope scope(this);
    ControlFlowBlock controlFlow(this, ast);

    for (auto *member = ast->elements; member; member = member->next) {
        PatternProperty *p = member->property;
        FunctionExpression *f = p->initializer->asFunctionDefinition();
        Q_ASSERT(f);
        AST::ComputedPropertyName *cname = AST::cast<ComputedPropertyName *>(p->name);
        if (cname) {
            ++nComputedNames;
            if (member->isStatic)
                ++nStaticComputedNames;
        }
        QString name = p->name->asString();
        uint nameIndex = cname ? UINT_MAX : registerString(name);
        Compiler::Class::Method::Type type = Compiler::Class::Method::Regular;
        if (p->type == PatternProperty::Getter)
            type = Compiler::Class::Method::Getter;
        else if (p->type == PatternProperty::Setter)
            type = Compiler::Class::Method::Setter;
        Compiler::Class::Method m{ nameIndex, type, static_cast<uint>(defineFunction(name, f, f->formals, f->body)) };

        if (member->isStatic) {
            if (name == QStringLiteral("prototype")) {
                throwSyntaxError(ast->firstSourceLocation(), QLatin1String("Cannot declare a static method named 'prototype'."));
                return false;
            }
            jsClass.staticMethods << m;
        } else {
            if (name == QStringLiteral("constructor")) {
                if (constructor) {
                    throwSyntaxError(ast->firstSourceLocation(), QLatin1String("Cannot declare a multiple constructors in a class."));
                    return false;
                }
                if (m.type != Compiler::Class::Method::Regular) {
                    throwSyntaxError(ast->firstSourceLocation(), QLatin1String("Cannot declare a getter or setter named 'constructor'."));
                    return false;
                }
                constructor = member;
                jsClass.constructorIndex = m.functionIndex;
                continue;
            }

            jsClass.methods << m;
        }
    }

    int classIndex = _module->classes.size();
    _module->classes.append(jsClass);

    Reference heritage = Reference::fromStackSlot(this);
    if (ast->heritage) {
        bytecodeGenerator->setLocation(ast->heritage->firstSourceLocation());
        Reference r = expression(ast->heritage);
        if (hasError)
            return false;
        r.storeOnStack(heritage.stackSlot());
    } else {
        Reference::fromConst(this, Value::emptyValue().asReturnedValue()).loadInAccumulator();
        heritage.storeConsumeAccumulator();
    }

    int computedNames = nComputedNames ? bytecodeGenerator->newRegisterArray(nComputedNames) : 0;
    int currentStaticName = computedNames;
    int currentNonStaticName = computedNames + nStaticComputedNames;

    for (auto *member = ast->elements; member; member = member->next) {
        AST::ComputedPropertyName *cname = AST::cast<AST::ComputedPropertyName *>(member->property->name);
        if (!cname)
            continue;
        RegisterScope scope(this);
        bytecodeGenerator->setLocation(cname->firstSourceLocation());
        Reference computedName = expression(cname->expression);
        if (hasError)
            return false;
        computedName.storeOnStack(member->isStatic ? currentStaticName++ : currentNonStaticName++);
    }

    Instruction::CreateClass createClass;
    createClass.classIndex = classIndex;
    createClass.heritage = heritage.stackSlot();
    createClass.computedNames = computedNames;

    bytecodeGenerator->addInstruction(createClass);

    if (!ast->name.isEmpty()) {
        Reference ctor = referenceForName(ast->name.toString(), true);
        ctor.isReferenceToConst = false; // this is the definition
        (void) ctor.storeRetainAccumulator();
    }

    setExprResult(Reference::fromAccumulator(this));
    return false;
}

bool Codegen::visit(ClassDeclaration *ast)
{
    TailCallBlocker blockTailCalls(this);
    Reference outerVar = referenceForName(ast->name.toString(), true);
    visit(static_cast<ClassExpression *>(ast));
    (void) outerVar.storeRetainAccumulator();
    return false;
}

bool Codegen::visit(Expression *ast)
{
    if (hasError)
        return false;

    TailCallBlocker blockTailCalls(this);
    statement(ast->left);
    blockTailCalls.unblock();
    clearExprResultName(); // The name only holds for the left part
    accept(ast->right);
    return false;
}

bool Codegen::visit(ArrayPattern *ast)
{
    if (hasError)
        return false;

    TailCallBlocker blockTailCalls(this);

    PatternElementList *it = ast->elements;

    int argc = 0;
    {
        RegisterScope scope(this);

        int args = -1;
        auto push = [this, &argc, &args](AST::ExpressionNode *arg) {
            int temp = bytecodeGenerator->newRegister();
            if (args == -1)
                args = temp;
            if (!arg) {
                auto c = Reference::fromConst(this, Value::emptyValue().asReturnedValue());
                (void) c.storeOnStack(temp);
            } else {
                RegisterScope scope(this);
                Reference r = expression(arg);
                if (hasError)
                    return;
                (void) r.storeOnStack(temp);
            }
            ++argc;
        };

        for (; it; it = it->next) {
            PatternElement *e = it->element;
            if (e && e->type == PatternElement::SpreadElement)
                break;
            for (Elision *elision = it->elision; elision; elision = elision->next)
                push(nullptr);

            if (!e)
                continue;

            push(e->initializer);
            if (hasError)
                return false;
        }

        if (args == -1) {
            Q_ASSERT(argc == 0);
            args = 0;
        }

        Instruction::DefineArray call;
        call.argc = argc;
        call.args = Moth::StackSlot::createRegister(args);
        bytecodeGenerator->addInstruction(call);
    }

    if (!it) {
        setExprResult(Reference::fromAccumulator(this));
        return false;
    }
    Q_ASSERT(it->element && it->element->type == PatternElement::SpreadElement);

    RegisterScope scope(this);
    Reference array = Reference::fromStackSlot(this);
    array.storeConsumeAccumulator();
    Reference index = Reference::storeConstOnStack(this, Encode(argc));

    auto pushAccumulator = [&]() {
        Reference slot = Reference::fromSubscript(array, index);
        slot.storeConsumeAccumulator();

        index.loadInAccumulator();
        Instruction::Increment inc;
        bytecodeGenerator->addInstruction(inc);
        index.storeConsumeAccumulator();
    };

    while (it) {
        for (Elision *elision = it->elision; elision; elision = elision->next) {
            Reference::fromConst(this, Value::emptyValue().asReturnedValue()).loadInAccumulator();
            pushAccumulator();
        }

        if (!it->element) {
            it = it->next;
            continue;
        }

        // handle spread element
        if (it->element->type == PatternElement::SpreadElement) {
            RegisterScope scope(this);

            Reference iterator = Reference::fromStackSlot(this);
            Reference iteratorDone = Reference::fromConst(this, Encode(false)).storeOnStack();
            Reference lhsValue = Reference::fromStackSlot(this);

            // There should be a temporal block, so that variables declared in lhs shadow outside vars.
            // This block should define a temporal dead zone for those variables, which is not yet implemented.
            {
                RegisterScope innerScope(this);
                Reference expr = expression(it->element->initializer);
                if (hasError)
                    return false;

                expr.loadInAccumulator();
                Instruction::GetIterator iteratorObjInstr;
                iteratorObjInstr.iterator = static_cast<int>(AST::ForEachType::Of);
                bytecodeGenerator->addInstruction(iteratorObjInstr);
                iterator.storeConsumeAccumulator();
            }

            BytecodeGenerator::Label in = bytecodeGenerator->newLabel();
            BytecodeGenerator::Label end = bytecodeGenerator->newLabel();

            {
                auto cleanup = [this, iterator, iteratorDone]() {
                    iterator.loadInAccumulator();
                    Instruction::IteratorClose close;
                    close.done = iteratorDone.stackSlot();
                    bytecodeGenerator->addInstruction(close);
                };
                ControlFlowLoop flow(this, &end, &in, cleanup);

                in.link();
                iterator.loadInAccumulator();
                Instruction::IteratorNext next;
                next.value = lhsValue.stackSlot();
                next.done = iteratorDone.stackSlot();
                bytecodeGenerator->addInstruction(next);
                bytecodeGenerator->addJumpInstruction(Instruction::JumpTrue()).link(end);

                lhsValue.loadInAccumulator();
                pushAccumulator();

                bytecodeGenerator->jump().link(in);
                end.link();
            }
        } else {
            RegisterScope innerScope(this);
            Reference expr = expression(it->element->initializer);
            if (hasError)
                return false;

            expr.loadInAccumulator();
            pushAccumulator();
        }

        it = it->next;
    }

    array.loadInAccumulator();
    setExprResult(Reference::fromAccumulator(this));

    return false;
}

bool Codegen::visit(ArrayMemberExpression *ast)
{
    if (hasError)
        return false;

    TailCallBlocker blockTailCalls(this);
    Reference base = expression(ast->base);
    if (hasError)
        return false;
    if (base.isSuper()) {
        Reference index = expression(ast->expression).storeOnStack();
        setExprResult(Reference::fromSuperProperty(index));
        return false;
    }
    base = base.storeOnStack();
    if (hasError)
        return false;
    if (AST::StringLiteral *str = AST::cast<AST::StringLiteral *>(ast->expression)) {
        QString s = str->value.toString();
        uint arrayIndex = QV4::String::toArrayIndex(s);
        if (arrayIndex == UINT_MAX) {
            setExprResult(Reference::fromMember(base, str->value.toString()));
            return false;
        }
        Reference index = Reference::fromConst(this, QV4::Encode(arrayIndex));
        setExprResult(Reference::fromSubscript(base, index));
        return false;
    }
    Reference index = expression(ast->expression);
    if (hasError)
        return false;
    setExprResult(Reference::fromSubscript(base, index));
    return false;
}

static QSOperator::Op baseOp(int op)
{
    switch ((QSOperator::Op) op) {
    case QSOperator::InplaceAnd: return QSOperator::BitAnd;
    case QSOperator::InplaceSub: return QSOperator::Sub;
    case QSOperator::InplaceDiv: return QSOperator::Div;
    case QSOperator::InplaceAdd: return QSOperator::Add;
    case QSOperator::InplaceLeftShift: return QSOperator::LShift;
    case QSOperator::InplaceMod: return QSOperator::Mod;
    case QSOperator::InplaceExp: return QSOperator::Exp;
    case QSOperator::InplaceMul: return QSOperator::Mul;
    case QSOperator::InplaceOr: return QSOperator::BitOr;
    case QSOperator::InplaceRightShift: return QSOperator::RShift;
    case QSOperator::InplaceURightShift: return QSOperator::URShift;
    case QSOperator::InplaceXor: return QSOperator::BitXor;
    default: return QSOperator::Invalid;
    }
}

bool Codegen::visit(BinaryExpression *ast)
{
    if (hasError)
        return false;

    TailCallBlocker blockTailCalls(this);

    if (ast->op == QSOperator::And) {
        if (exprAccept(cx)) {
            auto iftrue = bytecodeGenerator->newLabel();
            condition(ast->left, &iftrue, currentExpr().iffalse(), true);
            iftrue.link();
            blockTailCalls.unblock();
            const Result &expr = currentExpr();
            condition(ast->right, expr.iftrue(), expr.iffalse(), expr.trueBlockFollowsCondition());
        } else {
            auto iftrue = bytecodeGenerator->newLabel();
            auto endif = bytecodeGenerator->newLabel();

            Reference left = expression(ast->left);
            if (hasError)
                return false;
            left.loadInAccumulator();

            bytecodeGenerator->setLocation(ast->operatorToken);
            bytecodeGenerator->jumpFalse().link(endif);
            iftrue.link();

            blockTailCalls.unblock();
            Reference right = expression(ast->right);
            if (hasError)
                return false;
            right.loadInAccumulator();

            endif.link();

            setExprResult(Reference::fromAccumulator(this));
        }
        return false;
    } else if (ast->op == QSOperator::Or) {
        if (exprAccept(cx)) {
            auto iffalse = bytecodeGenerator->newLabel();
            condition(ast->left, currentExpr().iftrue(), &iffalse, false);
            iffalse.link();
            const Result &expr = currentExpr();
            condition(ast->right, expr.iftrue(), expr.iffalse(), expr.trueBlockFollowsCondition());
        } else {
            auto iffalse = bytecodeGenerator->newLabel();
            auto endif = bytecodeGenerator->newLabel();

            Reference left = expression(ast->left);
            if (hasError)
                return false;
            left.loadInAccumulator();

            bytecodeGenerator->setLocation(ast->operatorToken);
            bytecodeGenerator->jumpTrue().link(endif);
            iffalse.link();

            blockTailCalls.unblock();
            Reference right = expression(ast->right);
            if (hasError)
                return false;
            right.loadInAccumulator();

            endif.link();

            setExprResult(Reference::fromAccumulator(this));
        }
        return false;
    } else if (ast->op == QSOperator::Assign) {
        if (AST::Pattern *p = ast->left->patternCast()) {
            RegisterScope scope(this);
            Reference right = expression(ast->right);
            if (hasError)
                return false;
            right = right.storeOnStack();
            destructurePattern(p, right);
            if (!exprAccept(nx)) {
                right.loadInAccumulator();
                setExprResult(Reference::fromAccumulator(this));
            }
            return false;
        }
        Reference left = expression(ast->left);
        if (hasError)
            return false;

        if (!left.isLValue()) {
            throwReferenceError(ast->operatorToken, QStringLiteral("left-hand side of assignment operator is not an lvalue"));
            return false;
        }
        left = left.asLValue();
        if (throwSyntaxErrorOnEvalOrArgumentsInStrictMode(left, ast->left->lastSourceLocation()))
            return false;
        blockTailCalls.unblock();
        Reference r = expression(ast->right);
        if (hasError)
            return false;
        r.loadInAccumulator();
        if (exprAccept(nx))
            setExprResult(left.storeConsumeAccumulator());
        else
            setExprResult(left.storeRetainAccumulator());
        return false;
    }

    Reference left = expression(ast->left);
    if (hasError)
        return false;

    switch (ast->op) {
    case QSOperator::Or:
    case QSOperator::And:
    case QSOperator::Assign:
        Q_UNREACHABLE(); // handled separately above
        break;

    case QSOperator::InplaceAnd:
    case QSOperator::InplaceSub:
    case QSOperator::InplaceDiv:
    case QSOperator::InplaceAdd:
    case QSOperator::InplaceLeftShift:
    case QSOperator::InplaceMod:
    case QSOperator::InplaceExp:
    case QSOperator::InplaceMul:
    case QSOperator::InplaceOr:
    case QSOperator::InplaceRightShift:
    case QSOperator::InplaceURightShift:
    case QSOperator::InplaceXor: {
        if (throwSyntaxErrorOnEvalOrArgumentsInStrictMode(left, ast->left->lastSourceLocation()))
            return false;

        if (!left.isLValue()) {
            throwSyntaxError(ast->operatorToken, QStringLiteral("left-hand side of inplace operator is not an lvalue"));
            return false;
        }
        left = left.asLValue();

        Reference tempLeft = left.storeOnStack();
        Reference right = expression(ast->right);

        if (hasError)
            return false;

        binopHelper(baseOp(ast->op), tempLeft, right).loadInAccumulator();
        setExprResult(left.storeRetainAccumulator());

        break;
    }

    case QSOperator::BitAnd:
    case QSOperator::BitOr:
    case QSOperator::BitXor:
        if (left.isConstant()) {
            Reference right = expression(ast->right);
            if (hasError)
                return false;
            setExprResult(binopHelper(static_cast<QSOperator::Op>(ast->op), right, left));
            break;
        }
        // intentional fall-through!
    case QSOperator::In:
    case QSOperator::InstanceOf:
    case QSOperator::Equal:
    case QSOperator::NotEqual:
    case QSOperator::Ge:
    case QSOperator::Gt:
    case QSOperator::Le:
    case QSOperator::Lt:
    case QSOperator::StrictEqual:
    case QSOperator::StrictNotEqual:
    case QSOperator::Add:
    case QSOperator::Div:
    case QSOperator::Exp:
    case QSOperator::Mod:
    case QSOperator::Mul:
    case QSOperator::Sub:
    case QSOperator::LShift:
    case QSOperator::RShift:
    case QSOperator::URShift: {
        Reference right;
        if (AST::NumericLiteral *rhs = AST::cast<AST::NumericLiteral *>(ast->right)) {
            visit(rhs);
            right = exprResult();
        } else {
            left = left.storeOnStack(); // force any loads of the lhs, so the rhs won't clobber it
            right = expression(ast->right);
        }
        if (hasError)
            return false;

        setExprResult(binopHelper(static_cast<QSOperator::Op>(ast->op), left, right));

        break;
    }

    } // switch

    return false;
}

Codegen::Reference Codegen::binopHelper(QSOperator::Op oper, Reference &left, Reference &right)
{
    switch (oper) {
    case QSOperator::Add: {
        //### Todo: when we add type hints, we can generate an Increment when both the lhs is a number and the rhs == 1
        left = left.storeOnStack();
        right.loadInAccumulator();
        Instruction::Add add;
        add.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(add);
        break;
    }
    case QSOperator::Sub: {
        if (right.isConstant() && right.constant == Encode(int(1))) {
            left.loadInAccumulator();
            bytecodeGenerator->addInstruction(Instruction::Decrement());
        } else {
            left = left.storeOnStack();
            right.loadInAccumulator();
            Instruction::Sub sub;
            sub.lhs = left.stackSlot();
            bytecodeGenerator->addInstruction(sub);
        }
        break;
    }
    case QSOperator::Exp: {
        left = left.storeOnStack();
        right.loadInAccumulator();
        Instruction::Exp exp;
        exp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(exp);
        break;
    }
    case QSOperator::Mul: {
        left = left.storeOnStack();
        right.loadInAccumulator();
        Instruction::Mul mul;
        mul.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(mul);
        break;
    }
    case QSOperator::Div: {
        left = left.storeOnStack();
        right.loadInAccumulator();
        Instruction::Div div;
        div.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(div);
        break;
    }
    case QSOperator::Mod: {
        left = left.storeOnStack();
        right.loadInAccumulator();
        Instruction::Mod mod;
        mod.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(mod);
        break;
    }
    case QSOperator::BitAnd:
        if (right.isConstant()) {
            int rightAsInt = Value::fromReturnedValue(right.constant).toInt32();
            if (left.isConstant()) {
                int result = Value::fromReturnedValue(left.constant).toInt32() & rightAsInt;
                return Reference::fromConst(this, Encode(result));
            }
            left.loadInAccumulator();
            Instruction::BitAndConst bitAnd;
            bitAnd.rhs = rightAsInt;
            bytecodeGenerator->addInstruction(bitAnd);
        } else {
            right.loadInAccumulator();
            Instruction::BitAnd bitAnd;
            bitAnd.lhs = left.stackSlot();
            bytecodeGenerator->addInstruction(bitAnd);
        }
        break;
    case QSOperator::BitOr:
        if (right.isConstant()) {
            int rightAsInt = Value::fromReturnedValue(right.constant).toInt32();
            if (left.isConstant()) {
                int result = Value::fromReturnedValue(left.constant).toInt32() | rightAsInt;
                return Reference::fromConst(this, Encode(result));
            }
            left.loadInAccumulator();
            Instruction::BitOrConst bitOr;
            bitOr.rhs = rightAsInt;
            bytecodeGenerator->addInstruction(bitOr);
        } else {
            right.loadInAccumulator();
            Instruction::BitOr bitOr;
            bitOr.lhs = left.stackSlot();
            bytecodeGenerator->addInstruction(bitOr);
        }
        break;
    case QSOperator::BitXor:
        if (right.isConstant()) {
            int rightAsInt = Value::fromReturnedValue(right.constant).toInt32();
            if (left.isConstant()) {
                int result = Value::fromReturnedValue(left.constant).toInt32() ^ rightAsInt;
                return Reference::fromConst(this, Encode(result));
            }
            left.loadInAccumulator();
            Instruction::BitXorConst bitXor;
            bitXor.rhs = rightAsInt;
            bytecodeGenerator->addInstruction(bitXor);
        } else {
            right.loadInAccumulator();
            Instruction::BitXor bitXor;
            bitXor.lhs = left.stackSlot();
            bytecodeGenerator->addInstruction(bitXor);
        }
        break;
    case QSOperator::URShift:
        if (right.isConstant()) {
            left.loadInAccumulator();
            Instruction::UShrConst ushr;
            ushr.rhs = Value::fromReturnedValue(right.constant).toInt32() & 0x1f;
            bytecodeGenerator->addInstruction(ushr);
        } else {
            right.loadInAccumulator();
            Instruction::UShr ushr;
            ushr.lhs = left.stackSlot();
            bytecodeGenerator->addInstruction(ushr);
        }
        break;
    case QSOperator::RShift:
        if (right.isConstant()) {
            left.loadInAccumulator();
            Instruction::ShrConst shr;
            shr.rhs = Value::fromReturnedValue(right.constant).toInt32() & 0x1f;
            bytecodeGenerator->addInstruction(shr);
        } else {
            right.loadInAccumulator();
            Instruction::Shr shr;
            shr.lhs = left.stackSlot();
            bytecodeGenerator->addInstruction(shr);
        }
        break;
    case QSOperator::LShift:
        if (right.isConstant()) {
            left.loadInAccumulator();
            Instruction::ShlConst shl;
            shl.rhs = Value::fromReturnedValue(right.constant).toInt32() & 0x1f;
            bytecodeGenerator->addInstruction(shl);
        } else {
            right.loadInAccumulator();
            Instruction::Shl shl;
            shl.lhs = left.stackSlot();
            bytecodeGenerator->addInstruction(shl);
        }
        break;
    case QSOperator::InstanceOf: {
        Instruction::CmpInstanceOf binop;
        left = left.storeOnStack();
        right.loadInAccumulator();
        binop.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(binop);
        break;
    }
    case QSOperator::In: {
        Instruction::CmpIn binop;
        left = left.storeOnStack();
        right.loadInAccumulator();
        binop.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(binop);
        break;
    }
    case QSOperator::StrictEqual: {
        if (exprAccept(cx))
            return jumpBinop(oper, left, right);

        Instruction::CmpStrictEqual cmp;
        left = left.storeOnStack();
        right.loadInAccumulator();
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        break;
    }
    case QSOperator::StrictNotEqual: {
        if (exprAccept(cx))
            return jumpBinop(oper, left, right);

        Instruction::CmpStrictNotEqual cmp;
        left = left.storeOnStack();
        right.loadInAccumulator();
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        break;
    }
    case QSOperator::Equal: {
        if (exprAccept(cx))
            return jumpBinop(oper, left, right);

        Instruction::CmpEq cmp;
        left = left.storeOnStack();
        right.loadInAccumulator();
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        break;
    }
    case QSOperator::NotEqual: {
        if (exprAccept(cx))
            return jumpBinop(oper, left, right);

        Instruction::CmpNe cmp;
        left = left.storeOnStack();
        right.loadInAccumulator();
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        break;
    }
    case QSOperator::Gt: {
        if (exprAccept(cx))
            return jumpBinop(oper, left, right);

        Instruction::CmpGt cmp;
        left = left.storeOnStack();
        right.loadInAccumulator();
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        break;
    }
    case QSOperator::Ge: {
        if (exprAccept(cx))
            return jumpBinop(oper, left, right);

        Instruction::CmpGe cmp;
        left = left.storeOnStack();
        right.loadInAccumulator();
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        break;
    }
    case QSOperator::Lt: {
        if (exprAccept(cx))
            return jumpBinop(oper, left, right);

        Instruction::CmpLt cmp;
        left = left.storeOnStack();
        right.loadInAccumulator();
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        break;
    }
    case QSOperator::Le:
        if (exprAccept(cx))
            return jumpBinop(oper, left, right);

        Instruction::CmpLe cmp;
        left = left.storeOnStack();
        right.loadInAccumulator();
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        break;
    default:
        Q_UNREACHABLE();
    }

    return Reference::fromAccumulator(this);
}

Codegen::Reference Codegen::jumpBinop(QSOperator::Op oper, Reference &left, Reference &right)
{
    // See if we can generate specialized comparison instructions:
    if (oper == QSOperator::Equal || oper == QSOperator::NotEqual) {
        // Because == and != are reflexive, we can do the following:
        if (left.isConstant() && !right.isConstant())
            qSwap(left, right); // null==a -> a==null

        if (right.isConstant()) {
            Value c = Value::fromReturnedValue(right.constant);
            if (c.isNull() || c.isUndefined()) {
                left.loadInAccumulator();
                if (oper == QSOperator::Equal) {
                    Instruction::CmpEqNull cmp;
                    bytecodeGenerator->addInstruction(cmp);
                    addCJump();
                    return Reference();
                } else if (oper == QSOperator::NotEqual) {
                    Instruction::CmpNeNull cmp;
                    bytecodeGenerator->addInstruction(cmp);
                    addCJump();
                    return Reference();
                }
            } else if (c.isInt32()) {
                left.loadInAccumulator();
                if (oper == QSOperator::Equal) {
                    Instruction::CmpEqInt cmp;
                    cmp.lhs = c.int_32();
                    bytecodeGenerator->addInstruction(cmp);
                    addCJump();
                    return Reference();
                } else if (oper == QSOperator::NotEqual) {
                    Instruction::CmpNeInt cmp;
                    cmp.lhs = c.int_32();
                    bytecodeGenerator->addInstruction(cmp);
                    addCJump();
                    return Reference();
                }

            }
        }
    }

    left = left.storeOnStack();
    right.loadInAccumulator();

    switch (oper) {
    case QSOperator::StrictEqual: {
        Instruction::CmpStrictEqual cmp;
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        addCJump();
        break;
    }
    case QSOperator::StrictNotEqual: {
        Instruction::CmpStrictNotEqual cmp;
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        addCJump();
        break;
    }
    case QSOperator::Equal: {
        Instruction::CmpEq cmp;
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        addCJump();
        break;
    }
    case QSOperator::NotEqual: {
        Instruction::CmpNe cmp;
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        addCJump();
        break;
    }
    case QSOperator::Gt: {
        Instruction::CmpGt cmp;
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        addCJump();
        break;
    }
    case QSOperator::Ge: {
        Instruction::CmpGe cmp;
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        addCJump();
        break;
    }
    case QSOperator::Lt: {
        Instruction::CmpLt cmp;
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        addCJump();
        break;
    }
    case QSOperator::Le: {
        Instruction::CmpLe cmp;
        cmp.lhs = left.stackSlot();
        bytecodeGenerator->addInstruction(cmp);
        addCJump();
        break;
    }
    default:
        Q_UNREACHABLE();
    }
    return Reference();
}

bool Codegen::visit(CallExpression *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    Reference base = expression(ast->base);

    if (hasError)
        return false;
    switch (base.type) {
    case Reference::Member:
    case Reference::Subscript:
        base = base.asLValue();
        break;
    case Reference::Name:
        break;
    case Reference::Super:
        handleConstruct(base, ast->arguments);
        return false;
    case Reference::SuperProperty:
        break;
    default:
        base = base.storeOnStack();
        break;
    }

    int thisObject = bytecodeGenerator->newRegister();
    int functionObject = bytecodeGenerator->newRegister();

    auto calldata = pushArgs(ast->arguments);
    if (hasError)
        return false;

    blockTailCalls.unblock();
    if (calldata.hasSpread || _tailCallsAreAllowed) {
        Reference baseObject = base.baseObject();
        if (!baseObject.isStackSlot()) {
            baseObject.storeOnStack(thisObject);
            baseObject = Reference::fromStackSlot(this, thisObject);
        }
        if (!base.isStackSlot()) {
            base.storeOnStack(functionObject);
            base = Reference::fromStackSlot(this, functionObject);
        }

        if (calldata.hasSpread) {
            Instruction::CallWithSpread call;
            call.func = base.stackSlot();
            call.thisObject = baseObject.stackSlot();
            call.argc = calldata.argc;
            call.argv = calldata.argv;
            bytecodeGenerator->addInstruction(call);
        } else {
            Instruction::TailCall call;
            call.func = base.stackSlot();
            call.thisObject = baseObject.stackSlot();
            call.argc = calldata.argc;
            call.argv = calldata.argv;
            bytecodeGenerator->addInstruction(call);
        }

        setExprResult(Reference::fromAccumulator(this));
        return false;

    }

    handleCall(base, calldata, functionObject, thisObject);
    return false;
}

void Codegen::handleCall(Reference &base, Arguments calldata, int slotForFunction, int slotForThisObject)
{
    //### Do we really need all these call instructions? can's we load the callee in a temp?
    if (base.type == Reference::Member) {
        if (!disable_lookups && useFastLookups) {
            Instruction::CallPropertyLookup call;
            call.base = base.propertyBase.stackSlot();
            call.lookupIndex = registerGetterLookup(base.propertyNameIndex);
            call.argc = calldata.argc;
            call.argv = calldata.argv;
            bytecodeGenerator->addInstruction(call);
        } else {
            Instruction::CallProperty call;
            call.base = base.propertyBase.stackSlot();
            call.name = base.propertyNameIndex;
            call.argc = calldata.argc;
            call.argv = calldata.argv;
            bytecodeGenerator->addInstruction(call);
        }
    } else if (base.type == Reference::Subscript) {
        Instruction::CallElement call;
        call.base = base.elementBase;
        call.index = base.elementSubscript.stackSlot();
        call.argc = calldata.argc;
        call.argv = calldata.argv;
        bytecodeGenerator->addInstruction(call);
    } else if (base.type == Reference::Name) {
        if (base.name == QStringLiteral("eval")) {
            Instruction::CallPossiblyDirectEval call;
            call.argc = calldata.argc;
            call.argv = calldata.argv;
            bytecodeGenerator->addInstruction(call);
        } else if (!disable_lookups && useFastLookups && base.global) {
            if (base.qmlGlobal) {
                Instruction::CallQmlContextPropertyLookup call;
                call.index = registerQmlContextPropertyGetterLookup(base.nameAsIndex());
                call.argc = calldata.argc;
                call.argv = calldata.argv;
                bytecodeGenerator->addInstruction(call);
            } else {
                Instruction::CallGlobalLookup call;
                call.index = registerGlobalGetterLookup(base.nameAsIndex());
                call.argc = calldata.argc;
                call.argv = calldata.argv;
                bytecodeGenerator->addInstruction(call);
            }
        } else {
            Instruction::CallName call;
            call.name = base.nameAsIndex();
            call.argc = calldata.argc;
            call.argv = calldata.argv;
            bytecodeGenerator->addInstruction(call);
        }
    } else if (base.type == Reference::SuperProperty) {
        Reference receiver = base.baseObject();
        if (!base.isStackSlot()) {
            base.storeOnStack(slotForFunction);
            base = Reference::fromStackSlot(this, slotForFunction);
        }
        if (!receiver.isStackSlot()) {
            receiver.storeOnStack(slotForThisObject);
            receiver = Reference::fromStackSlot(this, slotForThisObject);
        }
        Instruction::CallWithReceiver call;
        call.name = base.stackSlot();
        call.thisObject = receiver.stackSlot();
        call.argc = calldata.argc;
        call.argv = calldata.argv;
        bytecodeGenerator->addInstruction(call);
    } else {
        Q_ASSERT(base.isStackSlot());
        Instruction::CallValue call;
        call.name = base.stackSlot();
        call.argc = calldata.argc;
        call.argv = calldata.argv;
        bytecodeGenerator->addInstruction(call);
    }

    setExprResult(Reference::fromAccumulator(this));
}

Codegen::Arguments Codegen::pushArgs(ArgumentList *args)
{
    bool hasSpread = false;
    int argc = 0;
    for (ArgumentList *it = args; it; it = it->next) {
        if (it->isSpreadElement) {
            hasSpread = true;
            ++argc;
        }
        ++argc;
    }

    if (!argc)
        return { 0, 0, false };

    int calldata = bytecodeGenerator->newRegisterArray(argc);

    argc = 0;
    for (ArgumentList *it = args; it; it = it->next) {
        if (it->isSpreadElement) {
            Reference::fromConst(this, Value::emptyValue().asReturnedValue()).storeOnStack(calldata + argc);
            ++argc;
        }
        RegisterScope scope(this);
        Reference e = expression(it->expression);
        if (hasError)
            break;
        if (!argc && !it->next && !hasSpread) {
            // avoid copy for functions taking a single argument
            if (e.isStackSlot())
                return { 1, e.stackSlot(), hasSpread };
        }
        (void) e.storeOnStack(calldata + argc);
        ++argc;
    }

    return { argc, calldata, hasSpread };
}

Codegen::Arguments Codegen::pushTemplateArgs(TemplateLiteral *args)
{
    int argc = 0;
    for (TemplateLiteral *it = args; it; it = it->next)
        ++argc;

    if (!argc)
        return { 0, 0, false };

    int calldata = bytecodeGenerator->newRegisterArray(argc);

    argc = 0;
    for (TemplateLiteral *it = args; it && it->expression; it = it->next) {
        RegisterScope scope(this);
        Reference e = expression(it->expression);
        if (hasError)
            break;
        (void) e.storeOnStack(calldata + argc);
        ++argc;
    }

    return { argc, calldata, false };
}

bool Codegen::visit(ConditionalExpression *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    BytecodeGenerator::Label iftrue = bytecodeGenerator->newLabel();
    BytecodeGenerator::Label iffalse = bytecodeGenerator->newLabel();
    condition(ast->expression, &iftrue, &iffalse, true);

    blockTailCalls.unblock();

    iftrue.link();
    Reference ok = expression(ast->ok);
    if (hasError)
        return false;
    ok.loadInAccumulator();
    BytecodeGenerator::Jump jump_endif = bytecodeGenerator->jump();

    iffalse.link();
    Reference ko = expression(ast->ko);
    if (hasError) {
        jump_endif.link(); // dummy link, to prevent assert in Jump destructor from triggering
        return false;
    }
    ko.loadInAccumulator();

    jump_endif.link();
    setExprResult(Reference::fromAccumulator(this));

    return false;
}

bool Codegen::visit(DeleteExpression *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);
    Reference expr = expression(ast->expression);
    if (hasError)
        return false;

    switch (expr.type) {
    case Reference::SuperProperty:
        // ### this should throw a reference error at runtime.
        return false;
    case Reference::StackSlot:
        if (!expr.stackSlotIsLocalOrArgument)
            break;
        // fall through
    case Reference::ScopedLocal:
        // Trying to delete a function argument might throw.
        if (_context->isStrict) {
            throwSyntaxError(ast->deleteToken, QStringLiteral("Delete of an unqualified identifier in strict mode."));
            return false;
        }
        setExprResult(Reference::fromConst(this, QV4::Encode(false)));
        return false;
    case Reference::Name: {
        if (_context->isStrict) {
            throwSyntaxError(ast->deleteToken, QStringLiteral("Delete of an unqualified identifier in strict mode."));
            return false;
        }
        Instruction::DeleteName del;
        del.name = expr.nameAsIndex();
        bytecodeGenerator->addInstruction(del);
        setExprResult(Reference::fromAccumulator(this));
        return false;
    }
    case Reference::Member: {
        //### maybe add a variant where the base can be in the accumulator?
        expr = expr.asLValue();
        Instruction::LoadRuntimeString instr;
        instr.stringId = expr.propertyNameIndex;
        bytecodeGenerator->addInstruction(instr);
        Reference index = Reference::fromStackSlot(this);
        index.storeConsumeAccumulator();
        Instruction::DeleteProperty del;
        del.base = expr.propertyBase.stackSlot();
        del.index = index.stackSlot();
        bytecodeGenerator->addInstruction(del);
        setExprResult(Reference::fromAccumulator(this));
        return false;
    }
    case Reference::Subscript: {
        //### maybe add a variant where the index can be in the accumulator?
        expr = expr.asLValue();
        Instruction::DeleteProperty del;
        del.base = expr.elementBase;
        del.index = expr.elementSubscript.stackSlot();
        bytecodeGenerator->addInstruction(del);
        setExprResult(Reference::fromAccumulator(this));
        return false;
    }
    default:
        break;
    }
    // [[11.4.1]] Return true if it's not a reference
    setExprResult(Reference::fromConst(this, QV4::Encode(true)));
    return false;
}

bool Codegen::visit(FalseLiteral *)
{
    if (hasError)
        return false;

    setExprResult(Reference::fromConst(this, QV4::Encode(false)));
    return false;
}

bool Codegen::visit(SuperLiteral *)
{
    if (hasError)
        return false;

    setExprResult(Reference::fromSuper(this));
    return false;
}

bool Codegen::visit(FieldMemberExpression *ast)
{
    if (hasError)
        return false;

    TailCallBlocker blockTailCalls(this);
    if (AST::IdentifierExpression *id = AST::cast<AST::IdentifierExpression *>(ast->base)) {
        if (id->name == QLatin1String("new")) {
            // new.target
            Q_ASSERT(ast->name == QLatin1String("target"));

            if (_context->isArrowFunction || _context->contextType == ContextType::Eval) {
                Reference r = referenceForName(QStringLiteral("new.target"), false);
                r.isReadonly = true;
                setExprResult(r);
                return false;
            }

            Reference r = Reference::fromStackSlot(this, CallData::NewTarget);
            setExprResult(r);
            return false;
        }
    }

    Reference base = expression(ast->base);
    if (hasError)
        return false;
    if (base.isSuper()) {
        Instruction::LoadRuntimeString load;
        load.stringId = registerString(ast->name.toString());
        bytecodeGenerator->addInstruction(load);
        Reference property = Reference::fromAccumulator(this).storeOnStack();
        setExprResult(Reference::fromSuperProperty(property));
        return false;
    }
    setExprResult(Reference::fromMember(base, ast->name.toString()));
    return false;
}

bool Codegen::visit(TaggedTemplate *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    return handleTaggedTemplate(expression(ast->base), ast);
}

bool Codegen::handleTaggedTemplate(Reference base, TaggedTemplate *ast)
{
    if (hasError)
        return false;

    int functionObject = -1, thisObject = -1;
    switch (base.type) {
    case Reference::Member:
    case Reference::Subscript:
        base = base.asLValue();
        break;
    case Reference::Name:
        break;
    case Reference::SuperProperty:
        thisObject = bytecodeGenerator->newRegister();
        functionObject = bytecodeGenerator->newRegister();
        break;
    default:
        base = base.storeOnStack();
        break;
    }

    createTemplateObject(ast->templateLiteral);
    int templateObjectTemp = Reference::fromAccumulator(this).storeOnStack().stackSlot();
    Q_UNUSED(templateObjectTemp);
    auto calldata = pushTemplateArgs(ast->templateLiteral);
    if (hasError)
        return false;
    ++calldata.argc;
    Q_ASSERT(calldata.argv == templateObjectTemp + 1);
    --calldata.argv;

    handleCall(base, calldata, functionObject, thisObject);
    return false;
}

void Codegen::createTemplateObject(TemplateLiteral *t)
{
    TemplateObject obj;

    for (TemplateLiteral *it = t; it; it = it->next) {
        obj.strings.append(registerString(it->value.toString()));
        obj.rawStrings.append(registerString(it->rawValue.toString()));
    }

    int index = _module->templateObjects.size();
    _module->templateObjects.append(obj);

    Instruction::GetTemplateObject getTemplateObject;
    getTemplateObject.index = index;
    bytecodeGenerator->addInstruction(getTemplateObject);
}

bool Codegen::visit(FunctionExpression *ast)
{
    if (hasError)
        return false;

    TailCallBlocker blockTailCalls(this);

    RegisterScope scope(this);

    int function = defineFunction(ast->name.toString(), ast, ast->formals, ast->body);
    if (hasError)
        return false;
    loadClosure(function);
    setExprResult(Reference::fromAccumulator(this));
    return false;
}

Codegen::Reference Codegen::referenceForName(const QString &name, bool isLhs, const SourceLocation &accessLocation)
{
    Context::ResolvedName resolved = _context->resolveName(name, accessLocation);

    if (resolved.type == Context::ResolvedName::Local || resolved.type == Context::ResolvedName::Stack
        || resolved.type == Context::ResolvedName::Import) {
        if (resolved.isArgOrEval && isLhs)
            // ### add correct source location
            throwSyntaxError(SourceLocation(), QStringLiteral("Variable name may not be eval or arguments in strict mode"));
        Reference r;
        switch (resolved.type) {
        case Context::ResolvedName::Local:
            r = Reference::fromScopedLocal(this, resolved.index, resolved.scope); break;
        case Context::ResolvedName::Stack:
            r = Reference::fromStackSlot(this, resolved.index, true /*isLocal*/); break;
        case Context::ResolvedName::Import:
            r = Reference::fromImport(this, resolved.index); break;
        default: Q_UNREACHABLE();
        }
        if (r.isStackSlot() && _volatileMemoryLocations.isVolatile(name))
            r.isVolatile = true;
        r.isArgOrEval = resolved.isArgOrEval;
        r.isReferenceToConst = resolved.isConst;
        r.requiresTDZCheck = resolved.requiresTDZCheck;
        r.name = name; // used to show correct name at run-time when TDZ check fails.
        return r;
    }

    Reference r = Reference::fromName(this, name);
    r.global = useFastLookups && (resolved.type == Context::ResolvedName::Global || resolved.type == Context::ResolvedName::QmlGlobal);
    r.qmlGlobal = resolved.type == Context::ResolvedName::QmlGlobal;
    if (!r.global && !r.qmlGlobal && m_globalNames.contains(name))
        r.global = true;
    return r;
}

void Codegen::loadClosure(int closureId)
{
    if (closureId >= 0) {
        Instruction::LoadClosure load;
        load.value = closureId;
        bytecodeGenerator->addInstruction(load);
    } else {
        Reference::fromConst(this, Encode::undefined()).loadInAccumulator();
    }
}

bool Codegen::visit(IdentifierExpression *ast)
{
    if (hasError)
        return false;

    setExprResult(referenceForName(ast->name.toString(), false, ast->firstSourceLocation()));
    return false;
}

bool Codegen::visit(NestedExpression *ast)
{
    if (hasError)
        return false;

    accept(ast->expression);
    return false;
}

void Codegen::handleConstruct(const Reference &base, ArgumentList *arguments)
{
    Reference constructor;
    if (base.isSuper()) {
        Instruction::LoadSuperConstructor super;
        bytecodeGenerator->addInstruction(super);
        constructor = Reference::fromAccumulator(this).storeOnStack();
    } else {
        constructor = base.storeOnStack();
    }

    auto calldata = pushArgs(arguments);
    if (hasError)
        return;

    if (base.isSuper())
        Reference::fromStackSlot(this, CallData::NewTarget).loadInAccumulator();
    else
        constructor.loadInAccumulator();

    if (calldata.hasSpread) {
        Instruction::ConstructWithSpread create;
        create.func = constructor.stackSlot();
        create.argc = calldata.argc;
        create.argv = calldata.argv;
        bytecodeGenerator->addInstruction(create);
    } else {
        Instruction::Construct create;
        create.func = constructor.stackSlot();
        create.argc = calldata.argc;
        create.argv = calldata.argv;
        bytecodeGenerator->addInstruction(create);
    }
    if (base.isSuper())
        // set the result up as the thisObject
        Reference::fromAccumulator(this).storeOnStack(CallData::This);

    setExprResult(Reference::fromAccumulator(this));
}

bool Codegen::visit(NewExpression *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    Reference base = expression(ast->expression);
    if (hasError)
        return false;
    if (base.isSuper()) {
        throwSyntaxError(ast->expression->firstSourceLocation(), QStringLiteral("Cannot use new with super."));
        return false;
    }

    handleConstruct(base, nullptr);
    return false;
}

bool Codegen::visit(NewMemberExpression *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    Reference base = expression(ast->base);
    if (hasError)
        return false;
    if (base.isSuper()) {
        throwSyntaxError(ast->base->firstSourceLocation(), QStringLiteral("Cannot use new with super."));
        return false;
    }

    handleConstruct(base, ast->arguments);
    return false;
}

bool Codegen::visit(NotExpression *ast)
{
    if (hasError)
        return false;

    TailCallBlocker blockTailCalls(this);
    setExprResult(unop(Not, expression(ast->expression)));
    return false;
}

bool Codegen::visit(NullExpression *)
{
    if (hasError)
        return false;

    if (exprAccept(cx))
        bytecodeGenerator->jump().link(*currentExpr().iffalse());
    else
        setExprResult(Reference::fromConst(this, Encode::null()));

    return false;
}

bool Codegen::visit(NumericLiteral *ast)
{
    if (hasError)
        return false;

    setExprResult(Reference::fromConst(this, QV4::Encode::smallestNumber(ast->value)));
    return false;
}

bool Codegen::visit(ObjectPattern *ast)
{
    if (hasError)
        return false;

    TailCallBlocker blockTailCalls(this);

    QVector<QPair<Reference, ObjectPropertyValue>> computedProperties;
    QMap<QString, ObjectPropertyValue> valueMap;

    RegisterScope scope(this);

    QStringList members;

    int argc = 0;
    int args = 0;
    auto push = [this, &args, &argc](const Reference &arg) {
        int temp = bytecodeGenerator->newRegister();
        if (argc == 0)
            args = temp;
        (void) arg.storeOnStack(temp);
        ++argc;
    };

    PatternPropertyList *it = ast->properties;
    for (; it; it = it->next) {
        PatternProperty *p = it->property;
        AST::ComputedPropertyName *cname = AST::cast<AST::ComputedPropertyName *>(p->name);
        if (cname || p->type != PatternProperty::Literal)
            break;
        QString name = p->name->asString();
        uint arrayIndex = QV4::String::toArrayIndex(name);
        if (arrayIndex != UINT_MAX)
            break;
        if (members.contains(name))
            break;
        members.append(name);

        {
            RegisterScope innerScope(this);
            Reference value = expression(p->initializer, name);
            if (hasError)
                return false;
            value.loadInAccumulator();
        }
        push(Reference::fromAccumulator(this));
    }

    int classId = jsUnitGenerator->registerJSClass(members);

    // handle complex property setters
    for (; it; it = it->next) {
        PatternProperty *p = it->property;
        AST::ComputedPropertyName *cname = AST::cast<AST::ComputedPropertyName *>(p->name);
        ObjectLiteralArgument argType = ObjectLiteralArgument::Value;
        if (p->type == PatternProperty::Method)
            argType = ObjectLiteralArgument::Method;
        else if (p->type == PatternProperty::Getter)
            argType = ObjectLiteralArgument::Getter;
        else if (p->type == PatternProperty::Setter)
            argType = ObjectLiteralArgument::Setter;

        Reference::fromConst(this, Encode(int(argType))).loadInAccumulator();
        push(Reference::fromAccumulator(this));

        if (cname) {
            RegisterScope innerScope(this);
            Reference name = expression(cname->expression);
            if (hasError)
                return false;
            name.loadInAccumulator();
        } else {
            QString name = p->name->asString();
#if 0
            uint arrayIndex = QV4::String::toArrayIndex(name);
            if (arrayIndex != UINT_MAX) {
                Reference::fromConst(this, Encode(arrayIndex)).loadInAccumulator();
            } else
#endif
            {
                Instruction::LoadRuntimeString instr;
                instr.stringId = registerString(name);
                bytecodeGenerator->addInstruction(instr);
            }
        }
        push(Reference::fromAccumulator(this));
        {
            RegisterScope innerScope(this);
            if (p->type != PatternProperty::Literal) {
                // need to get the closure id for the method
                FunctionExpression *f = p->initializer->asFunctionDefinition();
                Q_ASSERT(f);
                int function = defineFunction(f->name.toString(), f, f->formals, f->body);
                if (hasError)
                    return false;
                Reference::fromConst(this, Encode(function)).loadInAccumulator();
            } else {
                Reference value = expression(p->initializer);
                if (hasError)
                    return false;
                value.loadInAccumulator();
            }
        }
        push(Reference::fromAccumulator(this));
    }

    Instruction::DefineObjectLiteral call;
    call.internalClassId = classId;
    call.argc = argc;
    call.args = Moth::StackSlot::createRegister(args);
    bytecodeGenerator->addInstruction(call);
    setExprResult(Reference::fromAccumulator(this));
    return false;
}

bool Codegen::visit(PostDecrementExpression *ast)
{
    if (hasError)
        return false;

    Reference expr = expression(ast->base);
    if (hasError)
        return false;
    if (!expr.isLValue()) {
        throwReferenceError(ast->base->lastSourceLocation(), QStringLiteral("Invalid left-hand side expression in postfix operation"));
        return false;
    }
    if (throwSyntaxErrorOnEvalOrArgumentsInStrictMode(expr, ast->decrementToken))
        return false;

    setExprResult(unop(PostDecrement, expr));

    return false;
}

bool Codegen::visit(PostIncrementExpression *ast)
{
    if (hasError)
        return false;

    Reference expr = expression(ast->base);
    if (hasError)
        return false;
    if (!expr.isLValue()) {
        throwReferenceError(ast->base->lastSourceLocation(), QStringLiteral("Invalid left-hand side expression in postfix operation"));
        return false;
    }
    if (throwSyntaxErrorOnEvalOrArgumentsInStrictMode(expr, ast->incrementToken))
        return false;

    setExprResult(unop(PostIncrement, expr));
    return false;
}

bool Codegen::visit(PreDecrementExpression *ast)
{    if (hasError)
        return false;

    Reference expr = expression(ast->expression);
    if (hasError)
        return false;
    if (!expr.isLValue()) {
        throwReferenceError(ast->expression->lastSourceLocation(), QStringLiteral("Prefix ++ operator applied to value that is not a reference."));
        return false;
    }

    if (throwSyntaxErrorOnEvalOrArgumentsInStrictMode(expr, ast->decrementToken))
        return false;
    setExprResult(unop(PreDecrement, expr));
    return false;
}

bool Codegen::visit(PreIncrementExpression *ast)
{
    if (hasError)
        return false;

    Reference expr = expression(ast->expression);
    if (hasError)
        return false;
    if (!expr.isLValue()) {
        throwReferenceError(ast->expression->lastSourceLocation(), QStringLiteral("Prefix ++ operator applied to value that is not a reference."));
        return false;
    }

    if (throwSyntaxErrorOnEvalOrArgumentsInStrictMode(expr, ast->incrementToken))
        return false;
    setExprResult(unop(PreIncrement, expr));
    return false;
}

bool Codegen::visit(RegExpLiteral *ast)
{
    if (hasError)
        return false;

    auto r = Reference::fromStackSlot(this);
    r.isReadonly = true;
    setExprResult(r);

    Instruction::MoveRegExp instr;
    instr.regExpId = jsUnitGenerator->registerRegExp(ast);
    instr.destReg = r.stackSlot();
    bytecodeGenerator->addInstruction(instr);
    return false;
}

bool Codegen::visit(StringLiteral *ast)
{
    if (hasError)
        return false;

    auto r = Reference::fromAccumulator(this);
    r.isReadonly = true;
    setExprResult(r);

    Instruction::LoadRuntimeString instr;
    instr.stringId = registerString(ast->value.toString());
    bytecodeGenerator->addInstruction(instr);
    return false;
}

bool Codegen::visit(TemplateLiteral *ast)
{
    if (hasError)
        return false;

    TailCallBlocker blockTailCalls(this);

    Instruction::LoadRuntimeString instr;
    instr.stringId = registerString(ast->value.toString());
    bytecodeGenerator->addInstruction(instr);

    if (ast->expression) {
        RegisterScope scope(this);
        int temp = bytecodeGenerator->newRegister();
        Instruction::StoreReg store;
        store.reg = temp;
        bytecodeGenerator->addInstruction(store);

        Reference expr = expression(ast->expression);
        if (hasError)
            return false;

        if (ast->next) {
            int temp2 = bytecodeGenerator->newRegister();
            expr.storeOnStack(temp2);
            visit(ast->next);

            Instruction::Add instr;
            instr.lhs = temp2;
            bytecodeGenerator->addInstruction(instr);
        } else {
            expr.loadInAccumulator();
        }

        Instruction::Add instr;
        instr.lhs = temp;
        bytecodeGenerator->addInstruction(instr);
    }

    auto r = Reference::fromAccumulator(this);
    r.isReadonly = true;

    setExprResult(r);
    return false;

}

bool Codegen::visit(ThisExpression *)
{
    if (hasError)
        return false;

    if (_context->isArrowFunction) {
        Reference r = referenceForName(QStringLiteral("this"), false);
        r.isReadonly = true;
        setExprResult(r);
        return false;
    }
    setExprResult(Reference::fromThis(this));
    return false;
}

bool Codegen::visit(TildeExpression *ast)
{
    if (hasError)
        return false;

    TailCallBlocker blockTailCalls(this);
    setExprResult(unop(Compl, expression(ast->expression)));
    return false;
}

bool Codegen::visit(TrueLiteral *)
{
    if (hasError)
        return false;

    setExprResult(Reference::fromConst(this, QV4::Encode(true)));
    return false;
}

bool Codegen::visit(TypeOfExpression *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    Reference expr = expression(ast->expression);
    if (hasError)
        return false;

    if (expr.type == Reference::Name) {
        // special handling as typeof doesn't throw here
        Instruction::TypeofName instr;
        instr.name = expr.nameAsIndex();
        bytecodeGenerator->addInstruction(instr);
    } else {
        expr.loadInAccumulator();
        Instruction::TypeofValue instr;
        bytecodeGenerator->addInstruction(instr);
    }
    setExprResult(Reference::fromAccumulator(this));

    return false;
}

bool Codegen::visit(UnaryMinusExpression *ast)
{
    if (hasError)
        return false;

    TailCallBlocker blockTailCalls(this);
    setExprResult(unop(UMinus, expression(ast->expression)));
    return false;
}

bool Codegen::visit(UnaryPlusExpression *ast)
{
    if (hasError)
        return false;

    TailCallBlocker blockTailCalls(this);
    setExprResult(unop(UPlus, expression(ast->expression)));
    return false;
}

bool Codegen::visit(VoidExpression *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    statement(ast->expression);
    setExprResult(Reference::fromConst(this, Encode::undefined()));
    return false;
}

bool Codegen::visit(FunctionDeclaration * ast)
{
    if (hasError)
        return false;

    // no need to block tail calls: the function body isn't visited here.
    RegisterScope scope(this);

    if (_functionContext->contextType == ContextType::Binding)
        referenceForName(ast->name.toString(), true).loadInAccumulator();
    exprAccept(nx);
    return false;
}

bool Codegen::visit(YieldExpression *ast)
{
    if (inFormalParameterList) {
        throwSyntaxError(ast->firstSourceLocation(), QLatin1String("yield is not allowed inside parameter lists"));
        return false;
    }

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);
    Reference expr = ast->expression ? expression(ast->expression) : Reference::fromConst(this, Encode::undefined());
    if (hasError)
        return false;

    Reference acc = Reference::fromAccumulator(this);

    if (ast->isYieldStar) {
        Reference iterator = Reference::fromStackSlot(this);
        Reference lhsValue = Reference::fromConst(this, Encode::undefined()).storeOnStack();

        expr.loadInAccumulator();
        Instruction::GetIterator getIterator;
        getIterator.iterator = static_cast<int>(AST::ForEachType::Of);
        bytecodeGenerator->addInstruction(getIterator);
        iterator.storeConsumeAccumulator();
        Instruction::LoadUndefined load;
        bytecodeGenerator->addInstruction(load);

        BytecodeGenerator::Label in = bytecodeGenerator->newLabel();
        bytecodeGenerator->jump().link(in);

        BytecodeGenerator::Label loop = bytecodeGenerator->label();

        lhsValue.loadInAccumulator();
        Instruction::YieldStar yield;
        bytecodeGenerator->addInstruction(yield);

        in.link();

        Instruction::IteratorNextForYieldStar next;
        next.object = lhsValue.stackSlot();
        next.iterator = iterator.stackSlot();
        bytecodeGenerator->addInstruction(next);

        BytecodeGenerator::Jump done = bytecodeGenerator->jumpTrue();
        bytecodeGenerator->jumpNotUndefined().link(loop);
        lhsValue.loadInAccumulator();
        emitReturn(acc);


        done.link();

        lhsValue.loadInAccumulator();
        setExprResult(acc);
        return false;
    }

    expr.loadInAccumulator();
    Instruction::Yield yield;
    bytecodeGenerator->addInstruction(yield);
    Instruction::Resume resume;
    BytecodeGenerator::Jump jump = bytecodeGenerator->addJumpInstruction(resume);
    emitReturn(acc);
    jump.link();
    setExprResult(acc);
    return false;
}

static bool endsWithReturn(Module *module, Node *node)
{
    if (!node)
        return false;
    if (AST::cast<ReturnStatement *>(node))
        return true;
    if (AST::cast<ThrowStatement *>(node))
        return true;
    if (Program *p = AST::cast<Program *>(node))
        return endsWithReturn(module, p->statements);
    if (StatementList *sl = AST::cast<StatementList *>(node)) {
        while (sl->next)
            sl = sl->next;
        return endsWithReturn(module, sl->statement);
    }
    if (Block *b = AST::cast<Block *>(node)) {
        Context *blockContext = module->contextMap.value(node);
        if (blockContext->requiresExecutionContext)
            // we need to emit a return statement here, because of the
            // unwind handler
            return false;
        return endsWithReturn(module, b->statements);
    }
    if (IfStatement *is = AST::cast<IfStatement *>(node))
        return is->ko && endsWithReturn(module, is->ok) && endsWithReturn(module, is->ko);
    return false;
}

int Codegen::defineFunction(const QString &name, AST::Node *ast,
                            AST::FormalParameterList *formals,
                            AST::StatementList *body)
{
    enterContext(ast);

    if (_context->functionIndex >= 0)
        // already defined
        return leaveContext();

    _context->name = name.isEmpty() ? currentExpr().result().name : name;
    _module->functions.append(_context);
    _context->functionIndex = _module->functions.count() - 1;

    Context *savedFunctionContext = _functionContext;
    _functionContext = _context;
    ControlFlow *savedControlFlow = controlFlow;
    controlFlow = nullptr;

    if (_context->contextType == ContextType::Global || _context->contextType == ContextType::ScriptImportedByQML) {
        _module->blocks.append(_context);
        _context->blockIndex = _module->blocks.count() - 1;
    }
    if (_module->debugMode) // allow the debugger to see overwritten arguments
        _context->argumentsCanEscape = true;

    // When a user writes the following QML signal binding:
    //    onSignal: function() { doSomethingUsefull }
    // we will generate a binding function that just returns the closure. However, that's not useful
    // at all, because if the onSignal is a signal handler, the user is actually making it explicit
    // that the binding is a function, so we should execute that. However, we don't know that during
    // AOT compilation, so mark the surrounding function as only-returning-a-closure.
    _context->returnsClosure = body && body->statement && cast<ExpressionStatement *>(body->statement) && cast<FunctionExpression *>(cast<ExpressionStatement *>(body->statement)->expression);

    BytecodeGenerator bytecode(_context->line, _module->debugMode);
    BytecodeGenerator *savedBytecodeGenerator;
    savedBytecodeGenerator = bytecodeGenerator;
    bytecodeGenerator = &bytecode;
    bytecodeGenerator->setLocation(ast->firstSourceLocation());
    BytecodeGenerator::Label *savedReturnLabel = _returnLabel;
    _returnLabel = nullptr;

    bool savedFunctionEndsWithReturn = functionEndsWithReturn;
    functionEndsWithReturn = endsWithReturn(_module, body);

    // reserve the js stack frame (Context & js Function & accumulator)
    bytecodeGenerator->newRegisterArray(sizeof(CallData)/sizeof(Value) - 1 + _context->arguments.size());

    bool _inFormalParameterList = false;
    qSwap(_inFormalParameterList, inFormalParameterList);

    int returnAddress = -1;
    bool _requiresReturnValue = _context->requiresImplicitReturnValue();
    qSwap(requiresReturnValue, _requiresReturnValue);
    returnAddress = bytecodeGenerator->newRegister();
    qSwap(_returnAddress, returnAddress);

    // register the lexical scope for global code
    if (!_context->parent && _context->requiresExecutionContext) {
        _module->blocks.append(_context);
        _context->blockIndex = _module->blocks.count() - 1;
    }

    TailCallBlocker maybeBlockTailCalls(this, _context->canHaveTailCalls());

    RegisterScope registerScope(this);
    _context->emitBlockHeader(this);

    {
        QScopedValueRollback<bool> inFormals(inFormalParameterList, true);
        TailCallBlocker blockTailCalls(this); // we're not in the FunctionBody or ConciseBody yet

        int argc = 0;
        while (formals) {
            PatternElement *e = formals->element;
            if (!e) {
                if (!formals->next)
                    // trailing comma
                    break;
                Q_UNREACHABLE();
            }

            Reference arg = referenceForName(e->bindingIdentifier.toString(), true);
            if (e->type == PatternElement::RestElement) {
                Q_ASSERT(!formals->next);
                Instruction::CreateRestParameter rest;
                rest.argIndex = argc;
                bytecodeGenerator->addInstruction(rest);
                arg.storeConsumeAccumulator();
            } else {
                if (e->bindingTarget || e->initializer) {
                    initializeAndDestructureBindingElement(e, arg);
                    if (hasError)
                        break;
                }
            }
            formals = formals->next;
            ++argc;
        }
    }

    if (_context->isGenerator) {
        Instruction::Yield yield;
        bytecodeGenerator->addInstruction(yield);
    }

    statementList(body);

    if (!hasError) {
        bytecodeGenerator->setLocation(ast->lastSourceLocation());
        _context->emitBlockFooter(this);

        if (_returnLabel || !functionEndsWithReturn) {
            if (_returnLabel)
                _returnLabel->link();

            if (_returnLabel || requiresReturnValue) {
                Instruction::LoadReg load;
                load.reg = Moth::StackSlot::createRegister(_returnAddress);
                bytecodeGenerator->addInstruction(load);
            } else {
                Reference::fromConst(this, Encode::undefined()).loadInAccumulator();
            }

            bytecodeGenerator->addInstruction(Instruction::Ret());
        }

        Q_ASSERT(_context == _functionContext);
        bytecodeGenerator->finalize(_context);
        _context->registerCountInFunction = bytecodeGenerator->registerCount();
        static const bool showCode = qEnvironmentVariableIsSet("QV4_SHOW_BYTECODE");
        if (showCode) {
            qDebug() << "=== Bytecode for" << _context->name << "strict mode" << _context->isStrict
                     << "register count" << _context->registerCountInFunction << "implicit return" << requiresReturnValue;
            QV4::Moth::dumpBytecode(_context->code, _context->locals.size(), _context->arguments.size(),
                                    _context->line, _context->lineNumberMapping);
            qDebug();
        }
    }

    qSwap(_returnAddress, returnAddress);
    qSwap(requiresReturnValue, _requiresReturnValue);
    qSwap(_inFormalParameterList, inFormalParameterList);
    bytecodeGenerator = savedBytecodeGenerator;
    delete _returnLabel;
    _returnLabel = savedReturnLabel;
    controlFlow = savedControlFlow;
    functionEndsWithReturn = savedFunctionEndsWithReturn;
    _functionContext = savedFunctionContext;

    return leaveContext();
}

bool Codegen::visit(Block *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);

    ControlFlowBlock controlFlow(this, ast);
    statementList(ast->statements);
    return false;
}

bool Codegen::visit(BreakStatement *ast)
{
    if (hasError)
        return false;

    // no need to block tail calls here: children aren't visited
    if (!controlFlow) {
        throwSyntaxError(ast->lastSourceLocation(), QStringLiteral("Break outside of loop"));
        return false;
    }

    ControlFlow::UnwindTarget target = controlFlow->unwindTarget(ControlFlow::Break, ast->label.toString());
    if (!target.linkLabel.isValid()) {
        if (ast->label.isEmpty())
            throwSyntaxError(ast->lastSourceLocation(), QStringLiteral("Break outside of loop"));
        else
            throwSyntaxError(ast->lastSourceLocation(), QStringLiteral("Undefined label '%1'").arg(ast->label.toString()));
        return false;
    }

    bytecodeGenerator->unwindToLabel(target.unwindLevel, target.linkLabel);

    return false;
}

bool Codegen::visit(ContinueStatement *ast)
{
    if (hasError)
        return false;

    // no need to block tail calls here: children aren't visited
    RegisterScope scope(this);

    if (!controlFlow) {
        throwSyntaxError(ast->lastSourceLocation(), QStringLiteral("Continue outside of loop"));
        return false;
    }

    ControlFlow::UnwindTarget target = controlFlow->unwindTarget(ControlFlow::Continue, ast->label.toString());
    if (!target.linkLabel.isValid()) {
        if (ast->label.isEmpty())
            throwSyntaxError(ast->lastSourceLocation(), QStringLiteral("Undefined label '%1'").arg(ast->label.toString()));
        else
            throwSyntaxError(ast->lastSourceLocation(), QStringLiteral("continue outside of loop"));
        return false;
    }

    bytecodeGenerator->unwindToLabel(target.unwindLevel, target.linkLabel);

    return false;
}

bool Codegen::visit(DebuggerStatement *)
{
    Q_UNIMPLEMENTED();
    return false;
}

bool Codegen::visit(DoWhileStatement *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);

    BytecodeGenerator::Label body = bytecodeGenerator->label();
    BytecodeGenerator::Label cond = bytecodeGenerator->newLabel();
    BytecodeGenerator::Label end = bytecodeGenerator->newLabel();

    ControlFlowLoop flow(this, &end, &cond);

    statement(ast->statement);
    setJumpOutLocation(bytecodeGenerator, ast->statement, ast->semicolonToken);

    cond.link();

    TailCallBlocker blockTailCalls(this);
    if (!AST::cast<FalseLiteral *>(ast->expression)) {
        if (AST::cast<TrueLiteral *>(ast->expression))
            bytecodeGenerator->jump().link(body);
        else
            condition(ast->expression, &body, &end, false);
    }

    end.link();

    return false;
}

bool Codegen::visit(EmptyStatement *)
{
    return false;
}

bool Codegen::visit(ExpressionStatement *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    if (requiresReturnValue) {
        Reference e = expression(ast->expression);
        if (hasError)
            return false;
        (void) e.storeOnStack(_returnAddress);
    } else {
        statement(ast->expression);
    }
    return false;
}

bool Codegen::visit(ForEachStatement *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    Reference iterator = Reference::fromStackSlot(this);
    Reference iteratorDone = Reference::fromConst(this, Encode(false)).storeOnStack();
    Reference lhsValue = Reference::fromStackSlot(this);

    // There should be a temporal block, so that variables declared in lhs shadow outside vars.
    // This block should define a temporal dead zone for those variables.
    {
        RegisterScope innerScope(this);
        ControlFlowBlock controlFlow(this, ast);
        Reference expr = expression(ast->expression);
        if (hasError)
            return false;

        expr.loadInAccumulator();
        Instruction::GetIterator iteratorObjInstr;
        iteratorObjInstr.iterator = static_cast<int>(ast->type);
        bytecodeGenerator->addInstruction(iteratorObjInstr);
        iterator.storeConsumeAccumulator();
    }

    BytecodeGenerator::Label in = bytecodeGenerator->newLabel();
    BytecodeGenerator::Label end = bytecodeGenerator->newLabel();

    {
        auto cleanup = [ast, iterator, iteratorDone, this]() {
            if (ast->type == ForEachType::Of) {
                iterator.loadInAccumulator();
                Instruction::IteratorClose close;
                close.done = iteratorDone.stackSlot();
                bytecodeGenerator->addInstruction(close);
            }
        };
        ControlFlowLoop flow(this, &end, &in, cleanup);
        bytecodeGenerator->jump().link(in);

        BytecodeGenerator::Label body = bytecodeGenerator->label();

        // each iteration gets it's own context, as per spec
        {
            RegisterScope innerScope(this);
            ControlFlowBlock controlFlow(this, ast);

            if (ExpressionNode *e = ast->lhs->expressionCast()) {
                if (AST::Pattern *p = e->patternCast()) {
                    RegisterScope scope(this);
                    destructurePattern(p, lhsValue);
                } else {
                    Reference lhs = expression(e);
                    if (hasError)
                        goto error;
                    if (!lhs.isLValue()) {
                        throwReferenceError(e->firstSourceLocation(), QStringLiteral("Invalid left-hand side expression for 'in' expression"));
                        goto error;
                    }
                    lhs = lhs.asLValue();
                    lhsValue.loadInAccumulator();
                    lhs.storeConsumeAccumulator();
                }
            } else if (PatternElement *p = AST::cast<PatternElement *>(ast->lhs)) {
                initializeAndDestructureBindingElement(p, lhsValue, /*isDefinition =*/ true);
                if (hasError)
                    goto error;
            } else {
                Q_UNREACHABLE();
            }

            statement(ast->statement);
            setJumpOutLocation(bytecodeGenerator, ast->statement, ast->forToken);

        }

      error:
        in.link();
        iterator.loadInAccumulator();
        Instruction::IteratorNext next;
        next.value = lhsValue.stackSlot();
        next.done = iteratorDone.stackSlot();
        bytecodeGenerator->addInstruction(next);
        bytecodeGenerator->addJumpInstruction(Instruction::JumpFalse()).link(body);
        end.link();
        // all execution paths need to end up here (normal loop exit, break, and exceptions) in
        // order to reset the unwind handler, and to close the iterator in calse of an for-of loop.
    }

    return false;
}

bool Codegen::visit(ForStatement *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    ControlFlowBlock controlFlow(this, ast);

    if (ast->initialiser)
        statement(ast->initialiser);
    else if (ast->declarations)
        variableDeclarationList(ast->declarations);

    BytecodeGenerator::Label cond = bytecodeGenerator->label();
    BytecodeGenerator::Label body = bytecodeGenerator->newLabel();
    BytecodeGenerator::Label step = bytecodeGenerator->newLabel();
    BytecodeGenerator::Label end = bytecodeGenerator->newLabel();

    ControlFlowLoop flow(this, &end, &step);

    condition(ast->condition, &body, &end, true);

    body.link();
    blockTailCalls.unblock();
    statement(ast->statement);
    blockTailCalls.reblock();
    setJumpOutLocation(bytecodeGenerator, ast->statement, ast->forToken);

    step.link();
    if (_context->requiresExecutionContext) {
        Instruction::CloneBlockContext clone;
        bytecodeGenerator->addInstruction(clone);
    }
    statement(ast->expression);
    bytecodeGenerator->jump().link(cond);

    end.link();

    return false;
}

bool Codegen::visit(IfStatement *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    BytecodeGenerator::Label trueLabel = bytecodeGenerator->newLabel();
    BytecodeGenerator::Label falseLabel = bytecodeGenerator->newLabel();
    condition(ast->expression, &trueLabel, &falseLabel, true);
    blockTailCalls.unblock();

    trueLabel.link();
    statement(ast->ok);
    if (ast->ko) {
        if (endsWithReturn(_module, ast)) {
            falseLabel.link();
            statement(ast->ko);
        } else {
            BytecodeGenerator::Jump jump_endif = bytecodeGenerator->jump();
            falseLabel.link();
            statement(ast->ko);
            jump_endif.link();
        }
    } else {
        falseLabel.link();
    }

    return false;
}

bool Codegen::visit(LabelledStatement *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);

    // check that no outer loop contains the label
    ControlFlow *l = controlFlow;
    while (l) {
        if (l->label() == ast->label) {
            QString error = QString(QStringLiteral("Label '%1' has already been declared")).arg(ast->label.toString());
            throwSyntaxError(ast->firstSourceLocation(), error);
            return false;
        }
        l = l->parent;
    }
    _labelledStatement = ast;

    if (AST::cast<AST::SwitchStatement *>(ast->statement) ||
            AST::cast<AST::WhileStatement *>(ast->statement) ||
            AST::cast<AST::DoWhileStatement *>(ast->statement) ||
            AST::cast<AST::ForStatement *>(ast->statement) ||
            AST::cast<AST::ForEachStatement *>(ast->statement)) {
        statement(ast->statement); // labelledStatement will be associated with the ast->statement's loop.
    } else {
        BytecodeGenerator::Label breakLabel = bytecodeGenerator->newLabel();
        ControlFlowLoop flow(this, &breakLabel);
        statement(ast->statement);
        breakLabel.link();
    }

    return false;
}

void Codegen::emitReturn(const Reference &expr)
{
    ControlFlow::UnwindTarget target = controlFlow ? controlFlow->unwindTarget(ControlFlow::Return) : ControlFlow::UnwindTarget();
    if (target.linkLabel.isValid() && target.unwindLevel) {
        Q_ASSERT(_returnAddress >= 0);
        (void) expr.storeOnStack(_returnAddress);
        bytecodeGenerator->unwindToLabel(target.unwindLevel, target.linkLabel);
    } else {
        expr.loadInAccumulator();
        bytecodeGenerator->addInstruction(Instruction::Ret());
    }
}

bool Codegen::visit(ReturnStatement *ast)
{
    if (hasError)
        return false;

    if (_functionContext->contextType != ContextType::Function && _functionContext->contextType != ContextType::Binding) {
        throwSyntaxError(ast->returnToken, QStringLiteral("Return statement outside of function"));
        return false;
    }
    Reference expr;
    if (ast->expression) {
         expr = expression(ast->expression);
        if (hasError)
            return false;
    } else {
        expr = Reference::fromConst(this, Encode::undefined());
    }

    emitReturn(expr);

    return false;
}

bool Codegen::visit(SwitchStatement *ast)
{
    if (hasError)
        return false;

    if (requiresReturnValue)
        Reference::fromConst(this, Encode::undefined()).storeOnStack(_returnAddress);

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    if (ast->block) {
        BytecodeGenerator::Label switchEnd = bytecodeGenerator->newLabel();

        Reference lhs = expression(ast->expression);
        if (hasError)
            return false;
        lhs = lhs.storeOnStack();

        ControlFlowBlock controlFlow(this, ast->block);

        // set up labels for all clauses
        QHash<Node *, BytecodeGenerator::Label> blockMap;
        for (CaseClauses *it = ast->block->clauses; it; it = it->next)
            blockMap[it->clause] = bytecodeGenerator->newLabel();
        if (ast->block->defaultClause)
            blockMap[ast->block->defaultClause] = bytecodeGenerator->newLabel();
        for (CaseClauses *it = ast->block->moreClauses; it; it = it->next)
            blockMap[it->clause] = bytecodeGenerator->newLabel();

        // do the switch conditions
        for (CaseClauses *it = ast->block->clauses; it; it = it->next) {
            CaseClause *clause = it->clause;
            Reference rhs = expression(clause->expression);
            if (hasError)
                return false;
            rhs.loadInAccumulator();
            bytecodeGenerator->jumpStrictEqual(lhs.stackSlot(), blockMap.value(clause));
        }

        for (CaseClauses *it = ast->block->moreClauses; it; it = it->next) {
            CaseClause *clause = it->clause;
            Reference rhs = expression(clause->expression);
            if (hasError)
                return false;
            rhs.loadInAccumulator();
            bytecodeGenerator->jumpStrictEqual(lhs.stackSlot(), blockMap.value(clause));
        }

        if (DefaultClause *defaultClause = ast->block->defaultClause)
            bytecodeGenerator->jump().link(blockMap.value(defaultClause));
        else
            bytecodeGenerator->jump().link(switchEnd);

        ControlFlowLoop flow(this, &switchEnd);

        insideSwitch = true;
        blockTailCalls.unblock();
        for (CaseClauses *it = ast->block->clauses; it; it = it->next) {
            CaseClause *clause = it->clause;
            blockMap[clause].link();

            statementList(clause->statements);
        }

        if (ast->block->defaultClause) {
            DefaultClause *clause = ast->block->defaultClause;
            blockMap[clause].link();

            statementList(clause->statements);
        }

        for (CaseClauses *it = ast->block->moreClauses; it; it = it->next) {
            CaseClause *clause = it->clause;
            blockMap[clause].link();

            statementList(clause->statements);
        }
        insideSwitch = false;

        switchEnd.link();

    }

    return false;
}

bool Codegen::visit(ThrowStatement *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    Reference expr = expression(ast->expression);
    if (hasError)
        return false;

    expr.loadInAccumulator();
    Instruction::ThrowException instr;
    bytecodeGenerator->addInstruction(instr);
    return false;
}

void Codegen::handleTryCatch(TryStatement *ast)
{
    Q_ASSERT(ast);
    RegisterScope scope(this);
    {
        ControlFlowCatch catchFlow(this, ast->catchExpression);
        RegisterScope scope(this);
        TailCallBlocker blockTailCalls(this); // IMPORTANT: destruction will unblock tail calls before catch is generated
        statement(ast->statement);
    }
}

void Codegen::handleTryFinally(TryStatement *ast)
{
    RegisterScope scope(this);
    ControlFlowFinally finally(this, ast->finallyExpression);
    TailCallBlocker blockTailCalls(this); // IMPORTANT: destruction will unblock tail calls before finally is generated

    if (ast->catchExpression) {
        handleTryCatch(ast);
    } else {
        RegisterScope scope(this);
        statement(ast->statement);
    }
}

bool Codegen::visit(TryStatement *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);

    if (ast->finallyExpression && ast->finallyExpression->statement) {
        handleTryFinally(ast);
    } else {
        handleTryCatch(ast);
    }

    return false;
}

bool Codegen::visit(VariableStatement *ast)
{
    if (hasError)
        return false;

    variableDeclarationList(ast->declarations);
    return false;
}

bool Codegen::visit(WhileStatement *ast)
{
    if (hasError)
        return false;

    if (AST::cast<FalseLiteral *>(ast->expression))
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    BytecodeGenerator::Label start = bytecodeGenerator->newLabel();
    BytecodeGenerator::Label end = bytecodeGenerator->newLabel();
    BytecodeGenerator::Label cond = bytecodeGenerator->label();
    ControlFlowLoop flow(this, &end, &cond);

    if (!AST::cast<TrueLiteral *>(ast->expression))
        condition(ast->expression, &start, &end, true);
    blockTailCalls.unblock();

    start.link();
    statement(ast->statement);
    setJumpOutLocation(bytecodeGenerator, ast->statement, ast->whileToken);
    bytecodeGenerator->jump().link(cond);

    end.link();
    return false;
}

bool Codegen::visit(WithStatement *ast)
{
    if (hasError)
        return false;

    RegisterScope scope(this);
    TailCallBlocker blockTailCalls(this);

    Reference src = expression(ast->expression);
    if (hasError)
        return false;
    src = src.storeOnStack(); // trigger load before we setup the exception handler, so exceptions here go to the right place
    src.loadInAccumulator();

    enterContext(ast);
    {
        blockTailCalls.unblock();
        ControlFlowWith flow(this);
        statement(ast->statement);
    }
    leaveContext();

    return false;
}

bool Codegen::visit(UiArrayBinding *)
{
    Q_UNIMPLEMENTED();
    return false;
}

bool Codegen::visit(UiObjectBinding *)
{
    Q_UNIMPLEMENTED();
    return false;
}

bool Codegen::visit(UiObjectDefinition *)
{
    Q_UNIMPLEMENTED();
    return false;
}

bool Codegen::visit(UiPublicMember *)
{
    Q_UNIMPLEMENTED();
    return false;
}

bool Codegen::visit(UiScriptBinding *)
{
    Q_UNIMPLEMENTED();
    return false;
}

bool Codegen::visit(UiSourceElement *)
{
    Q_UNIMPLEMENTED();
    return false;
}

bool Codegen::throwSyntaxErrorOnEvalOrArgumentsInStrictMode(const Reference &r, const SourceLocation& loc)
{
    if (!_context->isStrict)
        return false;
    bool isArgOrEval = false;
    if (r.type == Reference::Name) {
        QString str = jsUnitGenerator->stringForIndex(r.nameAsIndex());
        if (str == QLatin1String("eval") || str == QLatin1String("arguments")) {
            isArgOrEval = true;
        }
    } else if (r.type == Reference::ScopedLocal || r.isRegister()) {
        isArgOrEval = r.isArgOrEval;
    }
    if (isArgOrEval)
        throwSyntaxError(loc, QStringLiteral("Variable name may not be eval or arguments in strict mode"));
    return isArgOrEval;
}

void Codegen::throwSyntaxError(const SourceLocation &loc, const QString &detail)
{
    if (hasError)
        return;

    hasError = true;
    QQmlJS::DiagnosticMessage error;
    error.message = detail;
    error.loc = loc;
    _errors << error;
}

void Codegen::throwReferenceError(const SourceLocation &loc, const QString &detail)
{
    if (hasError)
        return;

    hasError = true;
    QQmlJS::DiagnosticMessage error;
    error.message = detail;
    error.loc = loc;
    _errors << error;
}

QList<QQmlJS::DiagnosticMessage> Codegen::errors() const
{
    return _errors;
}

QQmlRefPointer<CompiledData::CompilationUnit> Codegen::generateCompilationUnit(bool generateUnitData)
{
    CompiledData::Unit *unitData = nullptr;
    if (generateUnitData)
        unitData = jsUnitGenerator->generateUnit();
    CompiledData::CompilationUnit *compilationUnit = new CompiledData::CompilationUnit(unitData);

    QQmlRefPointer<CompiledData::CompilationUnit> unit;
    unit.adopt(compilationUnit);
    return unit;
}

QQmlRefPointer<CompiledData::CompilationUnit> Codegen::createUnitForLoading()
{
    QQmlRefPointer<CompiledData::CompilationUnit> result;
    result.adopt(new CompiledData::CompilationUnit);
    return result;
}

class Codegen::VolatileMemoryLocationScanner: protected QQmlJS::AST::Visitor
{
    VolatileMemoryLocations locs;
    Codegen *parent;

public:
    VolatileMemoryLocationScanner(Codegen *parent) :
        QQmlJS::AST::Visitor(parent->recursionDepth()),
        parent(parent)
    {}

    Codegen::VolatileMemoryLocations scan(AST::Node *s)
    {
        s->accept(this);
        return locs;
    }

    bool visit(ArrayMemberExpression *) override
    {
        locs.setAllVolatile();
        return false;
    }

    bool visit(FieldMemberExpression *) override
    {
        locs.setAllVolatile();
        return false;
    }

    bool visit(PostIncrementExpression *e) override
    {
        collectIdentifiers(locs.specificLocations, e->base);
        return false;
    }

    bool visit(PostDecrementExpression *e) override
    {
        collectIdentifiers(locs.specificLocations, e->base);
        return false;
    }

    bool visit(PreIncrementExpression *e) override
    {
        collectIdentifiers(locs.specificLocations, e->expression);
        return false;
    }

    bool visit(PreDecrementExpression *e) override
    {
        collectIdentifiers(locs.specificLocations, e->expression);
        return false;
    }

    bool visit(BinaryExpression *e) override
    {
        switch (e->op) {
        case QSOperator::InplaceAnd:
        case QSOperator::InplaceSub:
        case QSOperator::InplaceDiv:
        case QSOperator::InplaceAdd:
        case QSOperator::InplaceLeftShift:
        case QSOperator::InplaceMod:
        case QSOperator::InplaceMul:
        case QSOperator::InplaceOr:
        case QSOperator::InplaceRightShift:
        case QSOperator::InplaceURightShift:
        case QSOperator::InplaceXor:
            collectIdentifiers(locs.specificLocations, e);
            return false;

        default:
            return true;
        }
    }

    void throwRecursionDepthError() override
    {
        parent->throwRecursionDepthError();
    }

private:
    void collectIdentifiers(QVector<QStringView> &ids, AST::Node *node) {
        class Collector: public QQmlJS::AST::Visitor {
        private:
            QVector<QStringView> &ids;
            VolatileMemoryLocationScanner *parent;

        public:
            Collector(QVector<QStringView> &ids, VolatileMemoryLocationScanner *parent) :
                QQmlJS::AST::Visitor(parent->recursionDepth()), ids(ids), parent(parent)
            {}

            bool visit(IdentifierExpression *ie) final {
                ids.append(ie->name);
                return false;
            }

            void throwRecursionDepthError() final
            {
                parent->throwRecursionDepthError();
            }
        };
        Collector collector(ids, this);
        node->accept(&collector);
    }
};

Codegen::VolatileMemoryLocations Codegen::scanVolatileMemoryLocations(AST::Node *ast)
{
    VolatileMemoryLocationScanner scanner(this);
    return scanner.scan(ast);
}


#ifndef V4_BOOTSTRAP

QList<QQmlError> Codegen::qmlErrors() const
{
    QList<QQmlError> qmlErrors;

    // Short circuit to avoid costly (de)heap allocation of QUrl if there are no errors.
    if (_errors.size() == 0)
        return qmlErrors;

    qmlErrors.reserve(_errors.size());

    QUrl url(_fileNameIsUrl ? QUrl(_module->fileName) : QUrl::fromLocalFile(_module->fileName));
    for (const QQmlJS::DiagnosticMessage &msg: qAsConst(_errors)) {
        QQmlError e;
        e.setUrl(url);
        e.setLine(msg.loc.startLine);
        e.setColumn(msg.loc.startColumn);
        e.setDescription(msg.message);
        qmlErrors << e;
    }

    return qmlErrors;
}

#endif // V4_BOOTSTRAP

bool Codegen::RValue::operator==(const RValue &other) const
{
    switch (type) {
    case Accumulator:
        return other.isAccumulator();
    case StackSlot:
        return other.isStackSlot() && theStackSlot == other.theStackSlot;
    case Const:
        return other.isConst() && constant == other.constant;
    default:
        return false;
    }
}

Codegen::RValue Codegen::RValue::storeOnStack() const
{
    switch (type) {
    case Accumulator:
        return RValue::fromStackSlot(codegen, Reference::fromAccumulator(codegen).storeOnStack().stackSlot());
    case StackSlot:
        return *this;
    case Const:
        return RValue::fromStackSlot(codegen, Reference::storeConstOnStack(codegen, constant).stackSlot());
    default:
        Q_UNREACHABLE();
    }
}

void Codegen::RValue::loadInAccumulator() const
{
    switch (type) {
    case Accumulator:
        // nothing to do
        return;
    case StackSlot:
        return Reference::fromStackSlot(codegen, theStackSlot).loadInAccumulator();
    case Const:
        return Reference::fromConst(codegen, constant).loadInAccumulator();
    default:
        Q_UNREACHABLE();
    }

}

bool Codegen::Reference::operator==(const Codegen::Reference &other) const
{
    if (type != other.type)
        return false;
    switch (type) {
    case Invalid:
    case Accumulator:
        break;
    case Super:
        return true;
    case SuperProperty:
        return property == other.property;
    case StackSlot:
        return theStackSlot == other.theStackSlot;
    case ScopedLocal:
        return index == other.index && scope == other.scope;
    case Name:
        return nameAsIndex() == other.nameAsIndex();
    case Member:
        return propertyBase == other.propertyBase && propertyNameIndex == other.propertyNameIndex;
    case Subscript:
        return elementBase == other.elementBase && elementSubscript == other.elementSubscript;
    case Import:
        return index == other.index;
    case Const:
        return constant == other.constant;
    }
    return true;
}

Codegen::RValue Codegen::Reference::asRValue() const
{
    switch (type) {
    case Invalid:
        Q_UNREACHABLE();
    case Accumulator:
        return RValue::fromAccumulator(codegen);
    case StackSlot:
        return RValue::fromStackSlot(codegen, stackSlot());
    case Const:
        return RValue::fromConst(codegen, constant);
    default:
        loadInAccumulator();
        return RValue::fromAccumulator(codegen);
    }
}

Codegen::Reference Codegen::Reference::asLValue() const
{
    switch (type) {
    case Invalid:
    case Accumulator:
        Q_UNREACHABLE();
    case Super:
        codegen->throwSyntaxError(AST::SourceLocation(), QStringLiteral("Super lvalues not implemented."));
        return *this;
    case Member:
        if (!propertyBase.isStackSlot()) {
            Reference r = *this;
            r.propertyBase = propertyBase.storeOnStack();
            return r;
        }
        return *this;
    case Subscript:
        if (!elementSubscript.isStackSlot()) {
            Reference r = *this;
            r.elementSubscript = elementSubscript.storeOnStack();
            return r;
        }
        return *this;
    default:
        return *this;
    }
}

Codegen::Reference Codegen::Reference::storeConsumeAccumulator() const
{
    storeAccumulator(); // it doesn't matter what happens here, just do it.
    return Reference();
}

Codegen::Reference Codegen::Reference::baseObject() const
{
    if (type == Reference::Member) {
        RValue rval = propertyBase;
        if (!rval.isValid())
            return Reference::fromConst(codegen, Encode::undefined());
        if (rval.isAccumulator())
            return Reference::fromAccumulator(codegen);
        if (rval.isStackSlot())
            return Reference::fromStackSlot(codegen, rval.stackSlot());
        if (rval.isConst())
            return Reference::fromConst(codegen, rval.constantValue());
        Q_UNREACHABLE();
    } else if (type == Reference::Subscript) {
        return Reference::fromStackSlot(codegen, elementBase.stackSlot());
    } else if (type == Reference::SuperProperty) {
        return Reference::fromStackSlot(codegen, CallData::This);
    } else {
        return Reference::fromConst(codegen, Encode::undefined());
    }
}

Codegen::Reference Codegen::Reference::storeOnStack() const
{ return doStoreOnStack(-1); }

void Codegen::Reference::storeOnStack(int slotIndex) const
{ doStoreOnStack(slotIndex); }

Codegen::Reference Codegen::Reference::doStoreOnStack(int slotIndex) const
{
    Q_ASSERT(isValid());

    if (isStackSlot() && slotIndex == -1 && !(stackSlotIsLocalOrArgument && isVolatile) && !requiresTDZCheck)
        return *this;

    if (isStackSlot() && !requiresTDZCheck) { // temp-to-temp move
        Reference dest = Reference::fromStackSlot(codegen, slotIndex);
        Instruction::MoveReg move;
        move.srcReg = stackSlot();
        move.destReg = dest.stackSlot();
        codegen->bytecodeGenerator->addInstruction(move);
        return dest;
    }

    Reference slot = Reference::fromStackSlot(codegen, slotIndex);
    if (isConstant()) {
        Instruction::MoveConst move;
        move.constIndex = codegen->registerConstant(constant);
        move.destTemp = slot.stackSlot();
        codegen->bytecodeGenerator->addInstruction(move);
    } else {
        loadInAccumulator();
        slot.storeConsumeAccumulator();
    }
    return slot;
}

Codegen::Reference Codegen::Reference::storeRetainAccumulator() const
{
    if (storeWipesAccumulator()) {
        // a store will
        auto tmp = Reference::fromStackSlot(codegen);
        tmp.storeAccumulator(); // this is safe, and won't destory the accumulator
        storeAccumulator();
        return tmp;
    } else {
        // ok, this is safe, just do the store.
        storeAccumulator();
        return *this;
    }
}

bool Codegen::Reference::storeWipesAccumulator() const
{
    switch (type) {
    default:
    case Invalid:
    case Const:
    case Accumulator:
        Q_UNREACHABLE();
        return false;
    case StackSlot:
    case ScopedLocal:
        return false;
    case Name:
    case Member:
    case Subscript:
        return true;
    }
}

void Codegen::Reference::storeAccumulator() const
{
    if (isReferenceToConst) {
        // throw a type error
        RegisterScope scope(codegen);
        Reference r = codegen->referenceForName(QStringLiteral("TypeError"), false);
        r = r.storeOnStack();
        Instruction::Construct construct;
        construct.func = r.stackSlot();
        construct.argc = 0;
        construct.argv = 0;
        codegen->bytecodeGenerator->addInstruction(construct);
        Instruction::ThrowException throwException;
        codegen->bytecodeGenerator->addInstruction(throwException);
        return;
    }
    switch (type) {
    case Super:
        Q_UNREACHABLE();
        return;
    case SuperProperty:
        Instruction::StoreSuperProperty store;
        store.property = property.stackSlot();
        codegen->bytecodeGenerator->addInstruction(store);
        return;
    case StackSlot: {
        Instruction::StoreReg store;
        store.reg = theStackSlot;
        codegen->bytecodeGenerator->addInstruction(store);
        return;
    }
    case ScopedLocal: {
        if (scope == 0) {
            Instruction::StoreLocal store;
            store.index = index;
            codegen->bytecodeGenerator->addInstruction(store);
        } else {
            Instruction::StoreScopedLocal store;
            store.index = index;
            store.scope = scope;
            codegen->bytecodeGenerator->addInstruction(store);
        }
        return;
    }
    case Name: {
        Context *c = codegen->currentContext();
        if (c->isStrict) {
            Instruction::StoreNameStrict store;
            store.name = nameAsIndex();
            codegen->bytecodeGenerator->addInstruction(store);
        } else {
            Instruction::StoreNameSloppy store;
            store.name = nameAsIndex();
            codegen->bytecodeGenerator->addInstruction(store);
        }
    } return;
    case Member:
        if (!disable_lookups && codegen->useFastLookups) {
            Instruction::SetLookup store;
            store.base = propertyBase.stackSlot();
            store.index = codegen->registerSetterLookup(propertyNameIndex);
            codegen->bytecodeGenerator->addInstruction(store);
        } else {
            Instruction::StoreProperty store;
            store.base = propertyBase.stackSlot();
            store.name = propertyNameIndex;
            codegen->bytecodeGenerator->addInstruction(store);
        }
        return;
    case Subscript: {
        Instruction::StoreElement store;
        store.base = elementBase;
        store.index = elementSubscript.stackSlot();
        codegen->bytecodeGenerator->addInstruction(store);
    } return;
    case Invalid:
    case Accumulator:
    case Const:
    case Import:
        break;
    }

    Q_UNREACHABLE();
}

void Codegen::Reference::loadInAccumulator() const
{
    auto tdzCheck = [this](bool requiresCheck){
        if (!requiresCheck)
            return;
        Instruction::DeadTemporalZoneCheck check;
        check.name = codegen->registerString(name);
        codegen->bytecodeGenerator->addInstruction(check);
    };
    auto tdzCheckStackSlot = [this, tdzCheck](Moth::StackSlot slot, bool requiresCheck){
        if (!requiresCheck)
            return;
        Instruction::LoadReg load;
        load.reg = slot;
        codegen->bytecodeGenerator->addInstruction(load);
        tdzCheck(true);
    };

    switch (type) {
    case Accumulator:
        return;
    case Super:
        Q_UNREACHABLE();
        return;
    case SuperProperty:
        tdzCheckStackSlot(property, subscriptRequiresTDZCheck);
        Instruction::LoadSuperProperty load;
        load.property = property.stackSlot();
        codegen->bytecodeGenerator->addInstruction(load);
        return;
    case Const: {
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmaybe-uninitialized") // the loads below are empty structs.
        if (constant == Encode::null()) {
            Instruction::LoadNull load;
            codegen->bytecodeGenerator->addInstruction(load);
        } else if (constant == Encode(true)) {
            Instruction::LoadTrue load;
            codegen->bytecodeGenerator->addInstruction(load);
        } else if (constant == Encode(false)) {
            Instruction::LoadFalse load;
            codegen->bytecodeGenerator->addInstruction(load);
        } else if (constant == Encode::undefined()) {
            Instruction::LoadUndefined load;
            codegen->bytecodeGenerator->addInstruction(load);
        } else {
            Value p = Value::fromReturnedValue(constant);
            if (p.isNumber()) {
                double d = p.asDouble();
                int i = static_cast<int>(d);
                if (d == i && (d != 0 || !std::signbit(d))) {
                    if (!i) {
                        Instruction::LoadZero load;
                        codegen->bytecodeGenerator->addInstruction(load);
                        return;
                    }
                    Instruction::LoadInt load;
                    load.value = Value::fromReturnedValue(constant).toInt32();
                    codegen->bytecodeGenerator->addInstruction(load);
                    return;
                }
            }
            Instruction::LoadConst load;
            load.index = codegen->registerConstant(constant);
            codegen->bytecodeGenerator->addInstruction(load);
        }
QT_WARNING_POP
    } return;
    case StackSlot: {
        Instruction::LoadReg load;
        load.reg = stackSlot();
        codegen->bytecodeGenerator->addInstruction(load);
        tdzCheck(requiresTDZCheck);
    } return;
    case ScopedLocal: {
        if (!scope) {
            Instruction::LoadLocal load;
            load.index = index;
            codegen->bytecodeGenerator->addInstruction(load);
        } else {
            Instruction::LoadScopedLocal load;
            load.index = index;
            load.scope = scope;
            codegen->bytecodeGenerator->addInstruction(load);
        }
        tdzCheck(requiresTDZCheck);
        return;
    }
    case Name:
        if (global) {
            // these value properties of the global object are immutable, we we can directly convert them
            // to their numeric value here
            if (name == QStringLiteral("undefined")) {
                Reference::fromConst(codegen, Encode::undefined()).loadInAccumulator();
                return;
            } else if (name == QStringLiteral("Infinity")) {
                Reference::fromConst(codegen, Encode(qInf())).loadInAccumulator();
                return;
            } else if (name == QStringLiteral("Nan")) {
                Reference::fromConst(codegen, Encode(qQNaN())).loadInAccumulator();
                return;
            }
        }
        if (!disable_lookups && global) {
            if (qmlGlobal) {
                Instruction::LoadQmlContextPropertyLookup load;
                load.index = codegen->registerQmlContextPropertyGetterLookup(nameAsIndex());
                codegen->bytecodeGenerator->addInstruction(load);
            } else {
                Instruction::LoadGlobalLookup load;
                load.index = codegen->registerGlobalGetterLookup(nameAsIndex());
                codegen->bytecodeGenerator->addInstruction(load);
            }
        } else {
            Instruction::LoadName load;
            load.name = nameAsIndex();
            codegen->bytecodeGenerator->addInstruction(load);
        }
        return;
    case Member:
        propertyBase.loadInAccumulator();
        tdzCheck(requiresTDZCheck);
        if (!disable_lookups && codegen->useFastLookups) {
            Instruction::GetLookup load;
            load.index = codegen->registerGetterLookup(propertyNameIndex);
            codegen->bytecodeGenerator->addInstruction(load);
        } else {
            Instruction::LoadProperty load;
            load.name = propertyNameIndex;
            codegen->bytecodeGenerator->addInstruction(load);
        }
        return;
    case Import: {
        Instruction::LoadImport load;
        load.index = index;
        codegen->bytecodeGenerator->addInstruction(load);
        tdzCheck(requiresTDZCheck);
    } return;
    case Subscript: {
        tdzCheckStackSlot(elementBase, requiresTDZCheck);
        elementSubscript.loadInAccumulator();
        tdzCheck(subscriptRequiresTDZCheck);
        Instruction::LoadElement load;
        load.base = elementBase;
        codegen->bytecodeGenerator->addInstruction(load);
    } return;
    case Invalid:
        break;
    }
    Q_UNREACHABLE();
}
