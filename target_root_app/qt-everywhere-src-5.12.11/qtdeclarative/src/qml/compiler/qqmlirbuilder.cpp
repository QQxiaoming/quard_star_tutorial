/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include "qqmlirbuilder_p.h"

#include <private/qv4value_p.h>
#include <private/qv4compileddata_p.h>
#include <private/qqmljsparser_p.h>
#include <private/qqmljslexer_p.h>
#include <private/qv4compilerscanfunctions_p.h>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <cmath>

#ifndef V4_BOOTSTRAP
#include <private/qqmlglobal_p.h>
#include <private/qqmltypeloader_p.h>
#include <private/qqmlengine_p.h>
#endif

#ifdef CONST
#undef CONST
#endif

QT_USE_NAMESPACE

static const quint32 emptyStringIndex = 0;

#if 0 //ndef V4_BOOTSTRAP
DEFINE_BOOL_CONFIG_OPTION(lookupHints, QML_LOOKUP_HINTS);
#endif // V4_BOOTSTRAP

using namespace QmlIR;

#define COMPILE_EXCEPTION(location, desc) \
    { \
        recordError(location, desc); \
        return false; \
    }

void Object::init(QQmlJS::MemoryPool *pool, int typeNameIndex, int idIndex, const QQmlJS::AST::SourceLocation &loc)
{
    inheritedTypeNameIndex = typeNameIndex;

    location.line = loc.startLine;
    location.column = loc.startColumn;

    idNameIndex = idIndex;
    id = -1;
    indexOfDefaultPropertyOrAlias = -1;
    defaultPropertyIsAlias = false;
    flags = QV4::CompiledData::Object::NoFlag;
    properties = pool->New<PoolList<Property> >();
    aliases = pool->New<PoolList<Alias> >();
    qmlEnums = pool->New<PoolList<Enum>>();
    qmlSignals = pool->New<PoolList<Signal> >();
    bindings = pool->New<PoolList<Binding> >();
    functions = pool->New<PoolList<Function> >();
    functionsAndExpressions = pool->New<PoolList<CompiledFunctionOrExpression> >();
    declarationsOverride = nullptr;
}

QString IRBuilder::sanityCheckFunctionNames(Object *obj, const QSet<QString> &illegalNames, QQmlJS::AST::SourceLocation *errorLocation)
{
    QSet<int> functionNames;
    for (auto functionit = obj->functionsBegin(); functionit != obj->functionsEnd(); ++functionit) {
        Function *f = functionit.ptr;
        errorLocation->startLine = f->location.line;
        errorLocation->startColumn = f->location.column;
        if (functionNames.contains(f->nameIndex))
            return tr("Duplicate method name");
        functionNames.insert(f->nameIndex);

        for (auto signalit = obj->signalsBegin(); signalit != obj->signalsEnd(); ++signalit) {
            QmlIR::Signal *s = signalit.ptr;
            if (s->nameIndex == f->nameIndex)
                return tr("Duplicate method name");
        }

        const QString name = stringAt(f->nameIndex);
        if (name.at(0).isUpper())
            return tr("Method names cannot begin with an upper case letter");
        if (illegalNames.contains(name))
            return tr("Illegal method name");
    }
    return QString(); // no error
}

QString Object::appendEnum(Enum *enumeration)
{
    Object *target = declarationsOverride;
    if (!target)
        target = this;

    for (Enum *e = qmlEnums->first; e; e = e->next) {
        if (e->nameIndex == enumeration->nameIndex)
            return tr("Duplicate scoped enum name");
    }

    target->qmlEnums->append(enumeration);
    return QString(); // no error
}

QString Object::appendSignal(Signal *signal)
{
    Object *target = declarationsOverride;
    if (!target)
        target = this;

    for (Signal *s = qmlSignals->first; s; s = s->next) {
        if (s->nameIndex == signal->nameIndex)
            return tr("Duplicate signal name");
    }

    target->qmlSignals->append(signal);
    return QString(); // no error
}

QString Object::appendProperty(Property *prop, const QString &propertyName, bool isDefaultProperty, const QQmlJS::AST::SourceLocation &defaultToken, QQmlJS::AST::SourceLocation *errorLocation)
{
    Object *target = declarationsOverride;
    if (!target)
        target = this;

    for (Property *p = target->properties->first; p; p = p->next)
        if (p->nameIndex == prop->nameIndex)
            return tr("Duplicate property name");

    if (propertyName.constData()->isUpper())
        return tr("Property names cannot begin with an upper case letter");

    const int index = target->properties->append(prop);
    if (isDefaultProperty) {
        if (target->indexOfDefaultPropertyOrAlias != -1) {
            *errorLocation = defaultToken;
            return tr("Duplicate default property");
        }
        target->indexOfDefaultPropertyOrAlias = index;
    }
    return QString(); // no error
}

QString Object::appendAlias(Alias *alias, const QString &aliasName, bool isDefaultProperty, const QQmlJS::AST::SourceLocation &defaultToken, QQmlJS::AST::SourceLocation *errorLocation)
{
    Object *target = declarationsOverride;
    if (!target)
        target = this;

    for (Alias *p = target->aliases->first; p; p = p->next)
        if (p->nameIndex == alias->nameIndex)
            return tr("Duplicate alias name");

    if (aliasName.constData()->isUpper())
        return tr("Alias names cannot begin with an upper case letter");

    const int index = target->aliases->append(alias);

    if (isDefaultProperty) {
        if (target->indexOfDefaultPropertyOrAlias != -1) {
            *errorLocation = defaultToken;
            return tr("Duplicate default property");
        }
        target->indexOfDefaultPropertyOrAlias = index;
        target->defaultPropertyIsAlias = true;
    }

    return QString(); // no error
}

void Object::appendFunction(QmlIR::Function *f)
{
    Object *target = declarationsOverride;
    if (!target)
        target = this;
    target->functions->append(f);
}

QString Object::appendBinding(Binding *b, bool isListBinding)
{
    const bool bindingToDefaultProperty = (b->propertyNameIndex == quint32(0));
    if (!isListBinding && !bindingToDefaultProperty
        && b->type != QV4::CompiledData::Binding::Type_GroupProperty
        && b->type != QV4::CompiledData::Binding::Type_AttachedProperty
        && !(b->flags & QV4::CompiledData::Binding::IsOnAssignment)) {
        Binding *existing = findBinding(b->propertyNameIndex);
        if (existing && existing->isValueBinding() == b->isValueBinding() && !(existing->flags & QV4::CompiledData::Binding::IsOnAssignment))
            return tr("Property value set multiple times");
    }
    if (bindingToDefaultProperty)
        insertSorted(b);
    else
        bindings->prepend(b);
    return QString(); // no error
}

Binding *Object::findBinding(quint32 nameIndex) const
{
    for (Binding *b = bindings->first; b; b = b->next)
        if (b->propertyNameIndex == nameIndex)
            return b;
    return nullptr;
}

void Object::insertSorted(Binding *b)
{
    Binding *insertionPoint = bindings->findSortedInsertionPoint<quint32, Binding, &Binding::offset>(b);
    bindings->insertAfter(insertionPoint, b);
}

QString Object::bindingAsString(Document *doc, int scriptIndex) const
{
    CompiledFunctionOrExpression *foe = functionsAndExpressions->slowAt(scriptIndex);
    QQmlJS::AST::Node *node = foe->node;
    if (QQmlJS::AST::ExpressionStatement *exprStmt = QQmlJS::AST::cast<QQmlJS::AST::ExpressionStatement *>(node))
        node = exprStmt->expression;
    QQmlJS::AST::SourceLocation start = node->firstSourceLocation();
    QQmlJS::AST::SourceLocation end = node->lastSourceLocation();
    return doc->code.mid(start.offset, end.offset + end.length - start.offset);
}

QStringList Signal::parameterStringList(const QV4::Compiler::StringTableGenerator *stringPool) const
{
    QStringList result;
    result.reserve(parameters->count);
    for (SignalParameter *param = parameters->first; param; param = param->next)
        result << stringPool->stringForIndex(param->nameIndex);
    return result;
}

static void replaceWithSpace(QString &str, int idx, int n)
{
    QChar *data = str.data() + idx;
    const QChar space(QLatin1Char(' '));
    for (int ii = 0; ii < n; ++ii)
        *data++ = space;
}

void Document::removeScriptPragmas(QString &script)
{
    const QLatin1String pragma("pragma");
    const QLatin1String library("library");

    QQmlJS::Lexer l(nullptr);
    l.setCode(script, 0);

    int token = l.lex();

    while (true) {
        if (token != QQmlJSGrammar::T_DOT)
            return;

        int startOffset = l.tokenOffset();
        int startLine = l.tokenStartLine();

        token = l.lex();

        if (token != QQmlJSGrammar::T_PRAGMA ||
            l.tokenStartLine() != startLine ||
            script.midRef(l.tokenOffset(), l.tokenLength()) != pragma)
            return;

        token = l.lex();

        if (token != QQmlJSGrammar::T_IDENTIFIER ||
            l.tokenStartLine() != startLine)
            return;

        const QStringRef pragmaValue = script.midRef(l.tokenOffset(), l.tokenLength());
        int endOffset = l.tokenLength() + l.tokenOffset();

        token = l.lex();
        if (l.tokenStartLine() == startLine)
            return;

        if (pragmaValue == library) {
            replaceWithSpace(script, startOffset, endOffset - startOffset);
        } else {
            return;
        }
    }
}

Document::Document(bool debugMode)
    : jsModule(debugMode)
    , program(nullptr)
    , jsGenerator(&jsModule)
{
}

ScriptDirectivesCollector::ScriptDirectivesCollector(Document *doc)
    : document(doc)
    , engine(&doc->jsParserEngine)
    , jsGenerator(&doc->jsGenerator)
{
}

void ScriptDirectivesCollector::pragmaLibrary()
{
    document->jsModule.unitFlags |= QV4::CompiledData::Unit::IsSharedLibrary;
}

void ScriptDirectivesCollector::importFile(const QString &jsfile, const QString &module, int lineNumber, int column)
{
    QV4::CompiledData::Import *import = engine->pool()->New<QV4::CompiledData::Import>();
    import->type = QV4::CompiledData::Import::ImportScript;
    import->uriIndex = jsGenerator->registerString(jsfile);
    import->qualifierIndex = jsGenerator->registerString(module);
    import->location.line = lineNumber;
    import->location.column = column;
    document->imports << import;
}

void ScriptDirectivesCollector::importModule(const QString &uri, const QString &version, const QString &module, int lineNumber, int column)
{
    QV4::CompiledData::Import *import = engine->pool()->New<QV4::CompiledData::Import>();
    import->type = QV4::CompiledData::Import::ImportLibrary;
    import->uriIndex = jsGenerator->registerString(uri);
    int vmaj;
    int vmin;
    IRBuilder::extractVersion(QStringRef(&version), &vmaj, &vmin);
    import->majorVersion = vmaj;
    import->minorVersion = vmin;
    import->qualifierIndex = jsGenerator->registerString(module);
    import->location.line = lineNumber;
    import->location.column = column;
    document->imports << import;
}

IRBuilder::IRBuilder(const QSet<QString> &illegalNames)
    : illegalNames(illegalNames)
    , _object(nullptr)
    , _propertyDeclaration(nullptr)
    , pool(nullptr)
    , jsGenerator(nullptr)
{
}

bool IRBuilder::generateFromQml(const QString &code, const QString &url, Document *output)
{
    QQmlJS::AST::UiProgram *program = nullptr;
    {
        QQmlJS::Lexer lexer(&output->jsParserEngine);
        lexer.setCode(code, /*line = */ 1);

        QQmlJS::Parser parser(&output->jsParserEngine);

        const bool parseResult = parser.parse();
        const auto diagnosticMessages = parser.diagnosticMessages();
        if (!parseResult || !diagnosticMessages.isEmpty()) {
            // Extract errors from the parser
            for (const QQmlJS::DiagnosticMessage &m : diagnosticMessages) {

                if (m.isWarning()) {
                    qWarning("%s:%d : %s", qPrintable(url), m.loc.startLine, qPrintable(m.message));
                    continue;
                }

                recordError(m.loc, m.message);
            }
            return false;
        }
        program = parser.ast();
        Q_ASSERT(program);
    }

    output->code = code;
    output->program = program;

    qSwap(_imports, output->imports);
    qSwap(_pragmas, output->pragmas);
    qSwap(_objects, output->objects);
    this->pool = output->jsParserEngine.pool();
    this->jsGenerator = &output->jsGenerator;

    Q_ASSERT(registerString(QString()) == emptyStringIndex);

    sourceCode = code;

    accept(program->headers);

    if (program->members->next) {
        QQmlJS::AST::SourceLocation loc = program->members->next->firstSourceLocation();
        recordError(loc, QCoreApplication::translate("QQmlParser", "Unexpected object definition"));
        return false;
    }

    QQmlJS::AST::UiObjectDefinition *rootObject = QQmlJS::AST::cast<QQmlJS::AST::UiObjectDefinition*>(program->members->member);
    Q_ASSERT(rootObject);
    int rootObjectIndex = -1;
    if (defineQMLObject(&rootObjectIndex, rootObject)) {
        Q_ASSERT(rootObjectIndex == 0);
    }

    qSwap(_imports, output->imports);
    qSwap(_pragmas, output->pragmas);
    qSwap(_objects, output->objects);
    return errors.isEmpty();
}

bool IRBuilder::isSignalPropertyName(const QString &name)
{
    if (name.length() < 3) return false;
    if (!name.startsWith(QLatin1String("on"))) return false;
    int ns = name.length();
    for (int i = 2; i < ns; ++i) {
        const QChar curr = name.at(i);
        if (curr.unicode() == '_') continue;
        if (curr.isUpper()) return true;
        return false;
    }
    return false; // consists solely of underscores - invalid.
}

bool IRBuilder::visit(QQmlJS::AST::UiArrayMemberList *ast)
{
    return QQmlJS::AST::Visitor::visit(ast);
}

bool IRBuilder::visit(QQmlJS::AST::UiProgram *)
{
    Q_ASSERT(!"should not happen");
    return false;
}

bool IRBuilder::visit(QQmlJS::AST::UiObjectDefinition *node)
{
    // The grammar can't distinguish between two different definitions here:
    //     Item { ... }
    // versus
    //     font { ... }
    // The former is a new binding with no property name and "Item" as type name,
    // and the latter is a binding to the font property with no type name but
    // only initializer.

    QQmlJS::AST::UiQualifiedId *lastId = node->qualifiedTypeNameId;
    while (lastId->next)
        lastId = lastId->next;
    bool isType = lastId->name.unicode()->isUpper();
    if (isType) {
        int idx = 0;
        if (!defineQMLObject(&idx, node))
            return false;
        const QQmlJS::AST::SourceLocation nameLocation = node->qualifiedTypeNameId->identifierToken;
        appendBinding(nameLocation, nameLocation, emptyStringIndex, idx);
    } else {
        int idx = 0;
        if (!defineQMLObject(&idx, /*qualfied type name id*/nullptr, node->qualifiedTypeNameId->firstSourceLocation(), node->initializer, /*declarations should go here*/_object))
            return false;
        appendBinding(node->qualifiedTypeNameId, idx);
    }
    return false;
}

bool IRBuilder::visit(QQmlJS::AST::UiObjectBinding *node)
{
    int idx = 0;
    if (!defineQMLObject(&idx, node->qualifiedTypeNameId, node->qualifiedTypeNameId->firstSourceLocation(), node->initializer))
        return false;
    appendBinding(node->qualifiedId, idx, node->hasOnToken);
    return false;
}

bool IRBuilder::visit(QQmlJS::AST::UiScriptBinding *node)
{
    appendBinding(node->qualifiedId, node->statement, node);
    return false;
}

bool IRBuilder::visit(QQmlJS::AST::UiArrayBinding *node)
{
    const QQmlJS::AST::SourceLocation qualifiedNameLocation = node->qualifiedId->identifierToken;
    Object *object = nullptr;
    QQmlJS::AST::UiQualifiedId *name = node->qualifiedId;
    if (!resolveQualifiedId(&name, &object))
        return false;

    qSwap(_object, object);

    const int propertyNameIndex = registerString(name->name.toString());

    if (bindingsTarget()->findBinding(propertyNameIndex) != nullptr) {
        recordError(name->identifierToken, tr("Property value set multiple times"));
        return false;
    }

    QVarLengthArray<QQmlJS::AST::UiArrayMemberList *, 16> memberList;
    QQmlJS::AST::UiArrayMemberList *member = node->members;
    while (member) {
        memberList.append(member);
        member = member->next;
    }
    for (int i = memberList.count() - 1; i >= 0; --i) {
        member = memberList.at(i);
        QQmlJS::AST::UiObjectDefinition *def = QQmlJS::AST::cast<QQmlJS::AST::UiObjectDefinition*>(member->member);

        int idx = 0;
        if (!defineQMLObject(&idx, def))
            return false;
        appendBinding(qualifiedNameLocation, name->identifierToken, propertyNameIndex, idx, /*isListItem*/ true);
    }

    qSwap(_object, object);
    return false;
}

bool IRBuilder::visit(QQmlJS::AST::UiHeaderItemList *list)
{
    return QQmlJS::AST::Visitor::visit(list);
}

bool IRBuilder::visit(QQmlJS::AST::UiObjectInitializer *ast)
{
    return QQmlJS::AST::Visitor::visit(ast);
}

bool IRBuilder::visit(QQmlJS::AST::UiObjectMemberList *ast)
{
    return QQmlJS::AST::Visitor::visit(ast);
}

bool IRBuilder::visit(QQmlJS::AST::UiParameterList *ast)
{
    return QQmlJS::AST::Visitor::visit(ast);
}

bool IRBuilder::visit(QQmlJS::AST::UiQualifiedId *id)
{
    return QQmlJS::AST::Visitor::visit(id);
}

void IRBuilder::accept(QQmlJS::AST::Node *node)
{
    QQmlJS::AST::Node::accept(node, this);
}

bool IRBuilder::defineQMLObject(int *objectIndex, QQmlJS::AST::UiQualifiedId *qualifiedTypeNameId, const QQmlJS::AST::SourceLocation &location, QQmlJS::AST::UiObjectInitializer *initializer, Object *declarationsOverride)
{
    if (QQmlJS::AST::UiQualifiedId *lastName = qualifiedTypeNameId) {
        while (lastName->next)
            lastName = lastName->next;
        if (!lastName->name.constData()->isUpper()) {
            recordError(lastName->identifierToken, tr("Expected type name"));
            return false;
        }
    }

    Object *obj = New<Object>();
    _objects.append(obj);
    *objectIndex = _objects.size() - 1;
    qSwap(_object, obj);

    _object->init(pool, registerString(asString(qualifiedTypeNameId)), emptyStringIndex, location);
    _object->declarationsOverride = declarationsOverride;

    // A new object is also a boundary for property declarations.
    Property *declaration = nullptr;
    qSwap(_propertyDeclaration, declaration);

    accept(initializer);

    qSwap(_propertyDeclaration, declaration);

    qSwap(_object, obj);

    if (!errors.isEmpty())
        return false;

    QQmlJS::AST::SourceLocation loc;
    QString error = sanityCheckFunctionNames(obj, illegalNames, &loc);
    if (!error.isEmpty()) {
        recordError(loc, error);
        return false;
    }

    return true;
}

bool IRBuilder::visit(QQmlJS::AST::UiImport *node)
{
    QString uri;
    QV4::CompiledData::Import *import = New<QV4::CompiledData::Import>();

    if (!node->fileName.isNull()) {
        uri = node->fileName.toString();

        if (uri.endsWith(QLatin1String(".js")) || uri.endsWith(QLatin1String(".mjs"))) {
            import->type = QV4::CompiledData::Import::ImportScript;
        } else {
            import->type = QV4::CompiledData::Import::ImportFile;
        }
    } else {
        import->type = QV4::CompiledData::Import::ImportLibrary;
        uri = asString(node->importUri);
    }

    import->qualifierIndex = emptyStringIndex;

    // Qualifier
    if (!node->importId.isNull()) {
        QString qualifier = node->importId.toString();
        if (!qualifier.at(0).isUpper()) {
            recordError(node->importIdToken, QCoreApplication::translate("QQmlParser","Invalid import qualifier ID"));
            return false;
        }
        if (qualifier == QLatin1String("Qt")) {
            recordError(node->importIdToken, QCoreApplication::translate("QQmlParser","Reserved name \"Qt\" cannot be used as an qualifier"));
            return false;
        }
        import->qualifierIndex = registerString(qualifier);

        // Check for script qualifier clashes
        bool isScript = import->type == QV4::CompiledData::Import::ImportScript;
        for (int ii = 0; ii < _imports.count(); ++ii) {
            const QV4::CompiledData::Import *other = _imports.at(ii);
            bool otherIsScript = other->type == QV4::CompiledData::Import::ImportScript;

            if ((isScript || otherIsScript) && qualifier == jsGenerator->stringForIndex(other->qualifierIndex)) {
                recordError(node->importIdToken, QCoreApplication::translate("QQmlParser","Script import qualifiers must be unique."));
                return false;
            }
        }

    } else if (import->type == QV4::CompiledData::Import::ImportScript) {
        recordError(node->fileNameToken, QCoreApplication::translate("QQmlParser","Script import requires a qualifier"));
        return false;
    }

    if (node->versionToken.isValid()) {
        int major, minor;
        extractVersion(textRefAt(node->versionToken), &major, &minor);
        import->majorVersion = major;
        import->minorVersion = minor;
    } else if (import->type == QV4::CompiledData::Import::ImportLibrary) {
        recordError(node->importIdToken, QCoreApplication::translate("QQmlParser","Library import requires a version"));
        return false;
    } else {
        // For backward compatibility in how the imports are loaded we
        // must otherwise initialize the major and minor version to -1.
        import->majorVersion = -1;
        import->minorVersion = -1;
    }

    import->location.line = node->importToken.startLine;
    import->location.column = node->importToken.startColumn;

    import->uriIndex = registerString(uri);

    _imports.append(import);

    return false;
}

bool IRBuilder::visit(QQmlJS::AST::UiPragma *node)
{
    Pragma *pragma = New<Pragma>();

    // For now the only valid pragma is Singleton, so lets validate the input
    if (!node->name.isNull())
    {
        if (QLatin1String("Singleton") == node->name)
        {
            pragma->type = Pragma::PragmaSingleton;
        } else {
            recordError(node->pragmaToken, QCoreApplication::translate("QQmlParser","Pragma requires a valid qualifier"));
            return false;
        }
    } else {
        recordError(node->pragmaToken, QCoreApplication::translate("QQmlParser","Pragma requires a valid qualifier"));
        return false;
    }

    pragma->location.line = node->pragmaToken.startLine;
    pragma->location.column = node->pragmaToken.startColumn;
    _pragmas.append(pragma);

    return false;
}

static QStringList astNodeToStringList(QQmlJS::AST::Node *node)
{
    if (node->kind == QQmlJS::AST::Node::Kind_IdentifierExpression) {
        QString name =
            static_cast<QQmlJS::AST::IdentifierExpression *>(node)->name.toString();
        return QStringList() << name;
    } else if (node->kind == QQmlJS::AST::Node::Kind_FieldMemberExpression) {
        QQmlJS::AST::FieldMemberExpression *expr = static_cast<QQmlJS::AST::FieldMemberExpression *>(node);

        QStringList rv = astNodeToStringList(expr->base);
        if (rv.isEmpty())
            return rv;
        rv.append(expr->name.toString());
        return rv;
    }
    return QStringList();
}

bool IRBuilder::visit(QQmlJS::AST::UiEnumDeclaration *node)
{
    Enum *enumeration = New<Enum>();
    QString enumName = node->name.toString();
    enumeration->nameIndex = registerString(enumName);

    if (enumName.at(0).isLower())
        COMPILE_EXCEPTION(node->enumToken, tr("Scoped enum names must begin with an upper case letter"));

    enumeration->location.line = node->enumToken.startLine;
    enumeration->location.column = node->enumToken.startColumn;

    enumeration->enumValues = New<PoolList<EnumValue>>();

    QQmlJS::AST::UiEnumMemberList *e = node->members;
    while (e) {
        EnumValue *enumValue = New<EnumValue>();
        QString member = e->member.toString();
        enumValue->nameIndex = registerString(member);
        if (member.at(0).isLower())
            COMPILE_EXCEPTION(e->memberToken, tr("Enum names must begin with an upper case letter"));

        double part;
        if (std::modf(e->value, &part) != 0.0)
            COMPILE_EXCEPTION(e->valueToken, tr("Enum value must be an integer"));
        if (e->value > std::numeric_limits<qint32>::max() || e->value < std::numeric_limits<qint32>::min())
            COMPILE_EXCEPTION(e->valueToken, tr("Enum value out of range"));
        enumValue->value = e->value;

        enumValue->location.line = e->memberToken.startLine;
        enumValue->location.column = e->memberToken.startColumn;
        enumeration->enumValues->append(enumValue);

        e = e->next;
    }

    QString error = _object->appendEnum(enumeration);
    if (!error.isEmpty()) {
        recordError(node->enumToken, error);
        return false;
    }

    return false;
}


bool IRBuilder::visit(QQmlJS::AST::UiPublicMember *node)
{
    static const struct TypeNameToType {
        const char *name;
        size_t nameLength;
        QV4::CompiledData::Property::Type type;
    } propTypeNameToTypes[] = {
        { "int", strlen("int"), QV4::CompiledData::Property::Int },
        { "bool", strlen("bool"), QV4::CompiledData::Property::Bool },
        { "double", strlen("double"), QV4::CompiledData::Property::Real },
        { "real", strlen("real"), QV4::CompiledData::Property::Real },
        { "string", strlen("string"), QV4::CompiledData::Property::String },
        { "url", strlen("url"), QV4::CompiledData::Property::Url },
        { "color", strlen("color"), QV4::CompiledData::Property::Color },
        // Internally QTime, QDate and QDateTime are all supported.
        // To be more consistent with JavaScript we expose only
        // QDateTime as it matches closely with the Date JS type.
        // We also call it "date" to match.
        // { "time", strlen("time"), Property::Time },
        // { "date", strlen("date"), Property::Date },
        { "date", strlen("date"), QV4::CompiledData::Property::DateTime },
        { "rect", strlen("rect"), QV4::CompiledData::Property::Rect },
        { "point", strlen("point"), QV4::CompiledData::Property::Point },
        { "size", strlen("size"), QV4::CompiledData::Property::Size },
        { "font", strlen("font"), QV4::CompiledData::Property::Font },
        { "vector2d", strlen("vector2d"), QV4::CompiledData::Property::Vector2D },
        { "vector3d", strlen("vector3d"), QV4::CompiledData::Property::Vector3D },
        { "vector4d", strlen("vector4d"), QV4::CompiledData::Property::Vector4D },
        { "quaternion", strlen("quaternion"), QV4::CompiledData::Property::Quaternion },
        { "matrix4x4", strlen("matrix4x4"), QV4::CompiledData::Property::Matrix4x4 },
        { "variant", strlen("variant"), QV4::CompiledData::Property::Variant },
        { "var", strlen("var"), QV4::CompiledData::Property::Var }
    };
    static const int propTypeNameToTypesCount = sizeof(propTypeNameToTypes) /
                                                sizeof(propTypeNameToTypes[0]);

    if (node->type == QQmlJS::AST::UiPublicMember::Signal) {
        Signal *signal = New<Signal>();
        QString signalName = node->name.toString();
        signal->nameIndex = registerString(signalName);

        QQmlJS::AST::SourceLocation loc = node->typeToken;
        signal->location.line = loc.startLine;
        signal->location.column = loc.startColumn;

        signal->parameters = New<PoolList<SignalParameter> >();

        QQmlJS::AST::UiParameterList *p = node->parameters;
        while (p) {
            const QString memberType = asString(p->type);

            if (memberType.isEmpty()) {
                recordError(node->typeToken, QCoreApplication::translate("QQmlParser","Expected parameter type"));
                return false;
            }

            const TypeNameToType *type = nullptr;
            for (int typeIndex = 0; typeIndex < propTypeNameToTypesCount; ++typeIndex) {
                const TypeNameToType *t = propTypeNameToTypes + typeIndex;
                if (memberType == QLatin1String(t->name, static_cast<int>(t->nameLength))) {
                    type = t;
                    break;
                }
            }

            SignalParameter *param = New<SignalParameter>();

            if (!type) {
                if (memberType.at(0).isUpper()) {
                    // Must be a QML object type.
                    // Lazily determine type during compilation.
                    param->type = QV4::CompiledData::Property::Custom;
                    param->customTypeNameIndex = registerString(memberType);
                } else {
                    QString errStr = QCoreApplication::translate("QQmlParser","Invalid signal parameter type: ");
                    errStr.append(memberType);
                    recordError(node->typeToken, errStr);
                    return false;
                }
            } else {
                // the parameter is a known basic type
                param->type = type->type;
                param->customTypeNameIndex = emptyStringIndex;
            }

            param->nameIndex = registerString(p->name.toString());
            param->location.line = p->identifierToken.startLine;
            param->location.column = p->identifierToken.startColumn;
            signal->parameters->append(param);
            p = p->next;
        }

        if (signalName.at(0).isUpper())
            COMPILE_EXCEPTION(node->identifierToken, tr("Signal names cannot begin with an upper case letter"));

        if (illegalNames.contains(signalName))
            COMPILE_EXCEPTION(node->identifierToken, tr("Illegal signal name"));

        QString error = _object->appendSignal(signal);
        if (!error.isEmpty()) {
            recordError(node->identifierToken, error);
            return false;
        }
    } else {
        QString memberType = asString(node->memberType);
        if (memberType == QLatin1String("alias")) {
            return appendAlias(node);
        } else {
            const QStringRef &name = node->name;

            bool typeFound = false;
            QV4::CompiledData::Property::Type type = QV4::CompiledData::Property::Var;

            for (int ii = 0; !typeFound && ii < propTypeNameToTypesCount; ++ii) {
                const TypeNameToType *t = propTypeNameToTypes + ii;
                if (memberType == QLatin1String(t->name, static_cast<int>(t->nameLength))) {
                    type = t->type;
                    typeFound = true;
                }
            }

            if (!typeFound && memberType.at(0).isUpper()) {
                const QStringRef &typeModifier = node->typeModifier;

                if (typeModifier.isEmpty()) {
                    type = QV4::CompiledData::Property::Custom;
                } else if (typeModifier == QLatin1String("list")) {
                    type = QV4::CompiledData::Property::CustomList;
                } else {
                    recordError(node->typeModifierToken, QCoreApplication::translate("QQmlParser","Invalid property type modifier"));
                    return false;
                }
                typeFound = true;
            } else if (!node->typeModifier.isNull()) {
                recordError(node->typeModifierToken, QCoreApplication::translate("QQmlParser","Unexpected property type modifier"));
                return false;
            }

            if (!typeFound) {
                recordError(node->typeToken, QCoreApplication::translate("QQmlParser","Expected property type"));
                return false;
            }

            Property *property = New<Property>();
            property->flags = 0;
            if (node->isReadonlyMember)
                property->flags |= QV4::CompiledData::Property::IsReadOnly;
            property->type = type;
            if (type >= QV4::CompiledData::Property::Custom)
                property->customTypeNameIndex = registerString(memberType);
            else
                property->customTypeNameIndex = emptyStringIndex;

            const QString propName = name.toString();
            property->nameIndex = registerString(propName);

            QQmlJS::AST::SourceLocation loc = node->firstSourceLocation();
            property->location.line = loc.startLine;
            property->location.column = loc.startColumn;

            QQmlJS::AST::SourceLocation errorLocation;
            QString error;

            if (illegalNames.contains(propName))
                error = tr("Illegal property name");
            else
                error = _object->appendProperty(property, propName, node->isDefaultMember, node->defaultToken, &errorLocation);

            if (!error.isEmpty()) {
                if (errorLocation.startLine == 0)
                    errorLocation = node->identifierToken;

                recordError(errorLocation, error);
                return false;
            }

            qSwap(_propertyDeclaration, property);
            if (node->binding) {
                // process QML-like initializers (e.g. property Object o: Object {})
                QQmlJS::AST::Node::accept(node->binding, this);
            } else if (node->statement) {
                if (!isRedundantNullInitializerForPropertyDeclaration(_propertyDeclaration, node->statement))
                    appendBinding(node->identifierToken, node->identifierToken, _propertyDeclaration->nameIndex, node->statement, node);
            }
            qSwap(_propertyDeclaration, property);
        }
    }

    return false;
}

bool IRBuilder::visit(QQmlJS::AST::UiSourceElement *node)
{
    if (QQmlJS::AST::FunctionDeclaration *funDecl = QQmlJS::AST::cast<QQmlJS::AST::FunctionDeclaration *>(node->sourceElement)) {
        CompiledFunctionOrExpression *foe = New<CompiledFunctionOrExpression>();
        foe->node = funDecl;
        foe->parentNode = funDecl;
        foe->nameIndex = registerString(funDecl->name.toString());
        const int index = _object->functionsAndExpressions->append(foe);

        Function *f = New<Function>();
        QQmlJS::AST::SourceLocation loc = funDecl->identifierToken;
        f->location.line = loc.startLine;
        f->location.column = loc.startColumn;
        f->index = index;
        f->nameIndex = registerString(funDecl->name.toString());

        const QStringList formals = funDecl->formals ? funDecl->formals->formals() : QStringList();
        int formalsCount = formals.size();
        f->formals.allocate(pool, formalsCount);

        int i = 0;
        for (const QString &arg : formals) {
            f->formals[i] = registerString(arg);
            ++i;
        }

        _object->appendFunction(f);
    } else {
        recordError(node->firstSourceLocation(), QCoreApplication::translate("QQmlParser","JavaScript declaration outside Script element"));
    }
    return false;
}

QString IRBuilder::asString(QQmlJS::AST::UiQualifiedId *node)
{
    QString s;

    for (QQmlJS::AST::UiQualifiedId *it = node; it; it = it->next) {
        s.append(it->name);

        if (it->next)
            s.append(QLatin1Char('.'));
    }

    return s;
}

QStringRef IRBuilder::asStringRef(QQmlJS::AST::Node *node)
{
    if (!node)
        return QStringRef();

    return textRefAt(node->firstSourceLocation(), node->lastSourceLocation());
}

void IRBuilder::extractVersion(QStringRef string, int *maj, int *min)
{
    *maj = -1; *min = -1;

    if (!string.isEmpty()) {

        int dot = string.indexOf(QLatin1Char('.'));

        if (dot < 0) {
            *maj = string.toInt();
            *min = 0;
        } else {
            *maj = string.left(dot).toInt();
            *min = string.mid(dot + 1).toInt();
        }
    }
}

QStringRef IRBuilder::textRefAt(const QQmlJS::AST::SourceLocation &first, const QQmlJS::AST::SourceLocation &last) const
{
    return QStringRef(&sourceCode, first.offset, last.offset + last.length - first.offset);
}

void IRBuilder::setBindingValue(QV4::CompiledData::Binding *binding, QQmlJS::AST::Statement *statement, QQmlJS::AST::Node *parentNode)
{
    QQmlJS::AST::SourceLocation loc = statement->firstSourceLocation();
    binding->valueLocation.line = loc.startLine;
    binding->valueLocation.column = loc.startColumn;
    binding->type = QV4::CompiledData::Binding::Type_Invalid;
    if (_propertyDeclaration && (_propertyDeclaration->flags & QV4::CompiledData::Property::IsReadOnly))
        binding->flags |= QV4::CompiledData::Binding::InitializerForReadOnlyDeclaration;

    QQmlJS::AST::ExpressionStatement *exprStmt = QQmlJS::AST::cast<QQmlJS::AST::ExpressionStatement *>(statement);
    if (exprStmt) {
        QQmlJS::AST::ExpressionNode * const expr = exprStmt->expression;
        if (QQmlJS::AST::StringLiteral *lit = QQmlJS::AST::cast<QQmlJS::AST::StringLiteral *>(expr)) {
            binding->type = QV4::CompiledData::Binding::Type_String;
            binding->stringIndex = registerString(lit->value.toString());
        } else if (expr->kind == QQmlJS::AST::Node::Kind_TrueLiteral) {
            binding->type = QV4::CompiledData::Binding::Type_Boolean;
            binding->value.b = true;
        } else if (expr->kind == QQmlJS::AST::Node::Kind_FalseLiteral) {
            binding->type = QV4::CompiledData::Binding::Type_Boolean;
            binding->value.b = false;
        } else if (QQmlJS::AST::NumericLiteral *lit = QQmlJS::AST::cast<QQmlJS::AST::NumericLiteral *>(expr)) {
            binding->type = QV4::CompiledData::Binding::Type_Number;
            binding->value.constantValueIndex = jsGenerator->registerConstant(QV4::Encode(lit->value));
        } else if (QQmlJS::AST::CallExpression *call = QQmlJS::AST::cast<QQmlJS::AST::CallExpression *>(expr)) {
            if (QQmlJS::AST::IdentifierExpression *base = QQmlJS::AST::cast<QQmlJS::AST::IdentifierExpression *>(call->base)) {
                tryGeneratingTranslationBinding(base->name, call->arguments, binding);
                // If it wasn't a translation binding, a normal script binding will be generated
                // below.
            }
        } else if (QQmlJS::AST::cast<QQmlJS::AST::FunctionExpression *>(expr)) {
            binding->flags |= QV4::CompiledData::Binding::IsFunctionExpression;
        } else if (QQmlJS::AST::UnaryMinusExpression *unaryMinus = QQmlJS::AST::cast<QQmlJS::AST::UnaryMinusExpression *>(expr)) {
            if (QQmlJS::AST::NumericLiteral *lit = QQmlJS::AST::cast<QQmlJS::AST::NumericLiteral *>(unaryMinus->expression)) {
                binding->type = QV4::CompiledData::Binding::Type_Number;
                binding->value.constantValueIndex = jsGenerator->registerConstant(QV4::Encode(-lit->value));
            }
        }
    }

    // Do binding instead
    if (binding->type == QV4::CompiledData::Binding::Type_Invalid) {
        binding->type = QV4::CompiledData::Binding::Type_Script;

        CompiledFunctionOrExpression *expr = New<CompiledFunctionOrExpression>();
        expr->node = statement;
        expr->parentNode = parentNode;
        expr->nameIndex = registerString(QLatin1String("expression for ")
                                         + stringAt(binding->propertyNameIndex));
        const int index = bindingsTarget()->functionsAndExpressions->append(expr);
        binding->value.compiledScriptIndex = index;
        // We don't need to store the binding script as string, except for script strings
        // and types with custom parsers. Those will be added later in the compilation phase.
        binding->stringIndex = emptyStringIndex;
    }
}

void IRBuilder::tryGeneratingTranslationBinding(const QStringRef &base, AST::ArgumentList *args, QV4::CompiledData::Binding *binding)
{
    if (base == QLatin1String("qsTr")) {
        QV4::CompiledData::TranslationData translationData;
        translationData.number = -1;
        translationData.commentIndex = 0; // empty string
        translationData.padding = 0;

        if (!args || !args->expression)
            return; // no arguments, stop

        QStringRef translation;
        if (QQmlJS::AST::StringLiteral *arg1 = QQmlJS::AST::cast<QQmlJS::AST::StringLiteral *>(args->expression)) {
            translation = arg1->value;
        } else {
            return; // first argument is not a string, stop
        }
        translationData.stringIndex = jsGenerator->registerString(translation.toString());

        args = args->next;

        if (args) {
            QQmlJS::AST::StringLiteral *arg2 = QQmlJS::AST::cast<QQmlJS::AST::StringLiteral *>(args->expression);
            if (!arg2)
                return; // second argument is not a string, stop
            translationData.commentIndex = jsGenerator->registerString(arg2->value.toString());

            args = args->next;
            if (args) {
                if (QQmlJS::AST::NumericLiteral *arg3 = QQmlJS::AST::cast<QQmlJS::AST::NumericLiteral *>(args->expression)) {
                    translationData.number = int(arg3->value);
                    args = args->next;
                } else {
                    return; // third argument is not a translation number, stop
                }
            }
        }

        if (args)
            return; // too many arguments, stop

        binding->type = QV4::CompiledData::Binding::Type_Translation;
        binding->value.translationDataIndex = jsGenerator->registerTranslation(translationData);
    } else if (base == QLatin1String("qsTrId")) {
        QV4::CompiledData::TranslationData translationData;
        translationData.number = -1;
        translationData.commentIndex = 0; // empty string, but unused
        translationData.padding = 0;

        if (!args || !args->expression)
            return; // no arguments, stop

        QStringRef id;
        if (QQmlJS::AST::StringLiteral *arg1 = QQmlJS::AST::cast<QQmlJS::AST::StringLiteral *>(args->expression)) {
            id = arg1->value;
        } else {
            return; // first argument is not a string, stop
        }
        translationData.stringIndex = jsGenerator->registerString(id.toString());

        args = args->next;

        if (args) {
            if (QQmlJS::AST::NumericLiteral *arg3 = QQmlJS::AST::cast<QQmlJS::AST::NumericLiteral *>(args->expression)) {
                translationData.number = int(arg3->value);
                args = args->next;
            } else {
                return; // third argument is not a translation number, stop
            }
        }

        if (args)
            return; // too many arguments, stop

        binding->type = QV4::CompiledData::Binding::Type_TranslationById;
        binding->value.translationDataIndex = jsGenerator->registerTranslation(translationData);
    } else if (base == QLatin1String("QT_TR_NOOP") || base == QLatin1String("QT_TRID_NOOP")) {
        if (!args || !args->expression)
            return; // no arguments, stop

        QStringRef str;
        if (QQmlJS::AST::StringLiteral *arg1 = QQmlJS::AST::cast<QQmlJS::AST::StringLiteral *>(args->expression)) {
            str = arg1->value;
        } else {
            return; // first argument is not a string, stop
        }

        args = args->next;
        if (args)
            return; // too many arguments, stop

        binding->type = QV4::CompiledData::Binding::Type_String;
        binding->stringIndex = jsGenerator->registerString(str.toString());
    } else if (base == QLatin1String("QT_TRANSLATE_NOOP")) {
        if (!args || !args->expression)
            return; // no arguments, stop

        args = args->next;
        if (!args || !args->expression)
            return; // no second arguments, stop

        QStringRef str;
        if (QQmlJS::AST::StringLiteral *arg2 = QQmlJS::AST::cast<QQmlJS::AST::StringLiteral *>(args->expression)) {
            str = arg2->value;
        } else {
            return; // first argument is not a string, stop
        }

        args = args->next;
        if (args)
            return; // too many arguments, stop

        binding->type = QV4::CompiledData::Binding::Type_String;
        binding->stringIndex = jsGenerator->registerString(str.toString());
    }
}

void IRBuilder::appendBinding(QQmlJS::AST::UiQualifiedId *name, QQmlJS::AST::Statement *value, QQmlJS::AST::Node *parentNode)
{
    const QQmlJS::AST::SourceLocation qualifiedNameLocation = name->identifierToken;
    Object *object = nullptr;
    if (!resolveQualifiedId(&name, &object))
        return;
    if (_object == object && name->name == QLatin1String("id")) {
        setId(name->identifierToken, value);
        return;
    }
    qSwap(_object, object);
    appendBinding(qualifiedNameLocation, name->identifierToken, registerString(name->name.toString()), value, parentNode);
    qSwap(_object, object);
}

void IRBuilder::appendBinding(QQmlJS::AST::UiQualifiedId *name, int objectIndex, bool isOnAssignment)
{
    const QQmlJS::AST::SourceLocation qualifiedNameLocation = name->identifierToken;
    Object *object = nullptr;
    if (!resolveQualifiedId(&name, &object, isOnAssignment))
        return;
    qSwap(_object, object);
    appendBinding(qualifiedNameLocation, name->identifierToken, registerString(name->name.toString()), objectIndex, /*isListItem*/false, isOnAssignment);
    qSwap(_object, object);
}

void IRBuilder::appendBinding(const QQmlJS::AST::SourceLocation &qualifiedNameLocation, const QQmlJS::AST::SourceLocation &nameLocation, quint32 propertyNameIndex,
                              QQmlJS::AST::Statement *value, QQmlJS::AST::Node *parentNode)
{
    Binding *binding = New<Binding>();
    binding->propertyNameIndex = propertyNameIndex;
    binding->offset = nameLocation.offset;
    binding->location.line = nameLocation.startLine;
    binding->location.column = nameLocation.startColumn;
    binding->flags = 0;
    setBindingValue(binding, value, parentNode);
    QString error = bindingsTarget()->appendBinding(binding, /*isListBinding*/false);
    if (!error.isEmpty()) {
        recordError(qualifiedNameLocation, error);
    }
}

void IRBuilder::appendBinding(const QQmlJS::AST::SourceLocation &qualifiedNameLocation, const QQmlJS::AST::SourceLocation &nameLocation, quint32 propertyNameIndex, int objectIndex, bool isListItem, bool isOnAssignment)
{
    if (stringAt(propertyNameIndex) == QLatin1String("id")) {
        recordError(nameLocation, tr("Invalid component id specification"));
        return;
    }

    Binding *binding = New<Binding>();
    binding->propertyNameIndex = propertyNameIndex;
    binding->offset = nameLocation.offset;
    binding->location.line = nameLocation.startLine;
    binding->location.column = nameLocation.startColumn;

    const Object *obj = _objects.at(objectIndex);
    binding->valueLocation = obj->location;

    binding->flags = 0;

    if (_propertyDeclaration && (_propertyDeclaration->flags & QV4::CompiledData::Property::IsReadOnly))
        binding->flags |= QV4::CompiledData::Binding::InitializerForReadOnlyDeclaration;

    // No type name on the initializer means it must be a group property
    if (_objects.at(objectIndex)->inheritedTypeNameIndex == emptyStringIndex)
        binding->type = QV4::CompiledData::Binding::Type_GroupProperty;
    else
        binding->type = QV4::CompiledData::Binding::Type_Object;

    if (isOnAssignment)
        binding->flags |= QV4::CompiledData::Binding::IsOnAssignment;
    if (isListItem)
        binding->flags |= QV4::CompiledData::Binding::IsListItem;

    binding->value.objectIndex = objectIndex;
    QString error = bindingsTarget()->appendBinding(binding, isListItem);
    if (!error.isEmpty()) {
        recordError(qualifiedNameLocation, error);
    }
}

bool IRBuilder::appendAlias(QQmlJS::AST::UiPublicMember *node)
{
    Alias *alias = New<Alias>();
    alias->flags = 0;
    if (node->isReadonlyMember)
        alias->flags |= QV4::CompiledData::Alias::IsReadOnly;

    const QString propName = node->name.toString();
    alias->nameIndex = registerString(propName);

    QQmlJS::AST::SourceLocation loc = node->firstSourceLocation();
    alias->location.line = loc.startLine;
    alias->location.column = loc.startColumn;

    alias->propertyNameIndex = emptyStringIndex;

    if (!node->statement && !node->binding)
        COMPILE_EXCEPTION(loc, tr("No property alias location"));

    QQmlJS::AST::SourceLocation rhsLoc;
    if (node->binding)
        rhsLoc = node->binding->firstSourceLocation();
    else if (node->statement)
        rhsLoc = node->statement->firstSourceLocation();
    else
        rhsLoc = node->semicolonToken;
    alias->referenceLocation.line = rhsLoc.startLine;
    alias->referenceLocation.column = rhsLoc.startColumn;

    QStringList aliasReference;

    if (QQmlJS::AST::ExpressionStatement *stmt = QQmlJS::AST::cast<QQmlJS::AST::ExpressionStatement*>(node->statement)) {
        aliasReference = astNodeToStringList(stmt->expression);
        if (aliasReference.isEmpty()) {
            if (isStatementNodeScript(node->statement)) {
                COMPILE_EXCEPTION(rhsLoc, tr("Invalid alias reference. An alias reference must be specified as <id>, <id>.<property> or <id>.<value property>.<property>"));
            } else {
                COMPILE_EXCEPTION(rhsLoc, tr("Invalid alias location"));
            }
        }
    } else {
        COMPILE_EXCEPTION(rhsLoc, tr("Invalid alias reference. An alias reference must be specified as <id>, <id>.<property> or <id>.<value property>.<property>"));
    }

    if (aliasReference.count() < 1 || aliasReference.count() > 3)
        COMPILE_EXCEPTION(rhsLoc, tr("Invalid alias reference. An alias reference must be specified as <id>, <id>.<property> or <id>.<value property>.<property>"));

     alias->idIndex = registerString(aliasReference.first());

     QString propertyValue = aliasReference.value(1);
     if (aliasReference.count() == 3)
         propertyValue += QLatin1Char('.') + aliasReference.at(2);
     alias->propertyNameIndex = registerString(propertyValue);

     QQmlJS::AST::SourceLocation errorLocation;
     QString error;

     if (illegalNames.contains(propName))
         error = tr("Illegal property name");
     else
         error = _object->appendAlias(alias, propName, node->isDefaultMember, node->defaultToken, &errorLocation);

     if (!error.isEmpty()) {
         if (errorLocation.startLine == 0)
             errorLocation = node->identifierToken;

         recordError(errorLocation, error);
         return false;
     }

     return false;
}

Object *IRBuilder::bindingsTarget() const
{
    if (_propertyDeclaration && _object->declarationsOverride)
        return _object->declarationsOverride;
    return _object;
}

bool IRBuilder::setId(const QQmlJS::AST::SourceLocation &idLocation, QQmlJS::AST::Statement *value)
{
    QQmlJS::AST::SourceLocation loc = value->firstSourceLocation();
    QStringRef str;

    QQmlJS::AST::Node *node = value;
    if (QQmlJS::AST::ExpressionStatement *stmt = QQmlJS::AST::cast<QQmlJS::AST::ExpressionStatement *>(node)) {
        if (QQmlJS::AST::StringLiteral *lit = QQmlJS::AST::cast<QQmlJS::AST::StringLiteral *>(stmt->expression)) {
            str = lit->value;
            node = nullptr;
        } else
            node = stmt->expression;
    }

    if (node && str.isEmpty())
        str = asStringRef(node);

    if (str.isEmpty())
        COMPILE_EXCEPTION(loc, tr( "Invalid empty ID"));

    QChar ch = str.at(0);
    if (ch.isLetter() && !ch.isLower())
        COMPILE_EXCEPTION(loc, tr( "IDs cannot start with an uppercase letter"));

    QChar u(QLatin1Char('_'));
    if (!ch.isLetter() && ch != u)
        COMPILE_EXCEPTION(loc, tr( "IDs must start with a letter or underscore"));

    for (int ii = 1; ii < str.count(); ++ii) {
        ch = str.at(ii);
        if (!ch.isLetterOrNumber() && ch != u)
            COMPILE_EXCEPTION(loc, tr( "IDs must contain only letters, numbers, and underscores"));
    }

    QString idQString(str.toString());
    if (illegalNames.contains(idQString))
        COMPILE_EXCEPTION(loc, tr( "ID illegally masks global JavaScript property"));

    if (_object->idNameIndex != emptyStringIndex)
        COMPILE_EXCEPTION(idLocation, tr("Property value set multiple times"));

    _object->idNameIndex = registerString(idQString);
    _object->locationOfIdProperty.line = idLocation.startLine;
    _object->locationOfIdProperty.column = idLocation.startColumn;

    return true;
}

bool IRBuilder::resolveQualifiedId(QQmlJS::AST::UiQualifiedId **nameToResolve, Object **object, bool onAssignment)
{
    QQmlJS::AST::UiQualifiedId *qualifiedIdElement = *nameToResolve;

    if (qualifiedIdElement->name == QLatin1String("id") && qualifiedIdElement->next)
        COMPILE_EXCEPTION(qualifiedIdElement->identifierToken, tr( "Invalid use of id property"));

    // If it's a namespace, prepend the qualifier and we'll resolve it later to the correct type.
    QString currentName = qualifiedIdElement->name.toString();
    if (qualifiedIdElement->next) {
        for (const QV4::CompiledData::Import* import : qAsConst(_imports))
            if (import->qualifierIndex != emptyStringIndex
                && stringAt(import->qualifierIndex) == currentName) {
                qualifiedIdElement = qualifiedIdElement->next;
                currentName += QLatin1Char('.') + qualifiedIdElement->name;

                if (!qualifiedIdElement->name.unicode()->isUpper())
                    COMPILE_EXCEPTION(qualifiedIdElement->firstSourceLocation(), tr("Expected type name"));

                break;
            }
    }

    *object = _object;
    while (qualifiedIdElement->next) {
        const quint32 propertyNameIndex = registerString(currentName);
        const bool isAttachedProperty = qualifiedIdElement->name.unicode()->isUpper();

        Binding *binding = (*object)->findBinding(propertyNameIndex);
        if (binding) {
            if (isAttachedProperty) {
                if (!binding->isAttachedProperty())
                    binding = nullptr;
            } else if (!binding->isGroupProperty()) {
                binding = nullptr;
            }
        }
        if (!binding) {
            binding = New<Binding>();
            binding->propertyNameIndex = propertyNameIndex;
            binding->offset = qualifiedIdElement->identifierToken.offset;
            binding->location.line = qualifiedIdElement->identifierToken.startLine;
            binding->location.column = qualifiedIdElement->identifierToken.startColumn;
            binding->valueLocation.line = qualifiedIdElement->next->identifierToken.startLine;
            binding->valueLocation.column = qualifiedIdElement->next->identifierToken.startColumn;
            binding->flags = 0;

            if (onAssignment)
                binding->flags |= QV4::CompiledData::Binding::IsOnAssignment;

            if (isAttachedProperty)
                binding->type = QV4::CompiledData::Binding::Type_AttachedProperty;
            else
                binding->type = QV4::CompiledData::Binding::Type_GroupProperty;

            int objIndex = 0;
            if (!defineQMLObject(&objIndex, nullptr, QQmlJS::AST::SourceLocation(), nullptr, nullptr))
                return false;
            binding->value.objectIndex = objIndex;

            QString error = (*object)->appendBinding(binding, /*isListBinding*/false);
            if (!error.isEmpty()) {
                recordError(qualifiedIdElement->identifierToken, error);
                return false;
            }
            *object = _objects.at(objIndex);
        } else {
            Q_ASSERT(binding->isAttachedProperty() || binding->isGroupProperty());
            *object = _objects.at(binding->value.objectIndex);
        }

        qualifiedIdElement = qualifiedIdElement->next;
        if (qualifiedIdElement)
            currentName = qualifiedIdElement->name.toString();
    }
    *nameToResolve = qualifiedIdElement;
    return true;
}

void IRBuilder::recordError(const QQmlJS::AST::SourceLocation &location, const QString &description)
{
    QQmlJS::DiagnosticMessage error;
    error.loc = location;
    error.message = description;
    errors << error;
}

bool IRBuilder::isStatementNodeScript(QQmlJS::AST::Statement *statement)
{
    if (QQmlJS::AST::ExpressionStatement *stmt = QQmlJS::AST::cast<QQmlJS::AST::ExpressionStatement *>(statement)) {
        QQmlJS::AST::ExpressionNode *expr = stmt->expression;
        if (QQmlJS::AST::cast<QQmlJS::AST::StringLiteral *>(expr))
            return false;
        else if (expr->kind == QQmlJS::AST::Node::Kind_TrueLiteral)
            return false;
        else if (expr->kind == QQmlJS::AST::Node::Kind_FalseLiteral)
            return false;
        else if (QQmlJS::AST::cast<QQmlJS::AST::NumericLiteral *>(expr))
            return false;
        else {

            if (QQmlJS::AST::UnaryMinusExpression *unaryMinus = QQmlJS::AST::cast<QQmlJS::AST::UnaryMinusExpression *>(expr)) {
               if (QQmlJS::AST::cast<QQmlJS::AST::NumericLiteral *>(unaryMinus->expression)) {
                   return false;
               }
            }
        }
    }

    return true;
}

bool IRBuilder::isRedundantNullInitializerForPropertyDeclaration(Property *property, QQmlJS::AST::Statement *statement)
{
    if (property->type != QV4::CompiledData::Property::Custom)
        return false;
    QQmlJS::AST::ExpressionStatement *exprStmt = QQmlJS::AST::cast<QQmlJS::AST::ExpressionStatement *>(statement);
    if (!exprStmt)
        return false;
    QQmlJS::AST::ExpressionNode * const expr = exprStmt->expression;
    return QQmlJS::AST::cast<QQmlJS::AST::NullExpression *>(expr);
}

void QmlUnitGenerator::generate(Document &output, const QV4::CompiledData::DependentTypesHasher &dependencyHasher)
{
    output.jsGenerator.stringTable.registerString(output.jsModule.fileName);
    output.jsGenerator.stringTable.registerString(output.jsModule.finalUrl);

    QQmlRefPointer<QV4::CompiledData::CompilationUnit> compilationUnit = output.javaScriptCompilationUnit;

    QV4::CompiledData::Unit *jsUnit = nullptr;
    const bool finalize = !compilationUnit->data;

    // We may already have unit data if we're loading an ahead-of-time generated cache file.
    if (!finalize) {
        jsUnit = const_cast<QV4::CompiledData::Unit *>(compilationUnit->data);
#ifndef V4_BOOTSTRAP
        output.javaScriptCompilationUnit->dynamicStrings = output.jsGenerator.stringTable.allStrings();
#endif
    } else {
        QV4::CompiledData::Unit *createdUnit;
        jsUnit = createdUnit = output.jsGenerator.generateUnit();

        // enable flag if we encountered pragma Singleton
        for (Pragma *p : qAsConst(output.pragmas)) {
            if (p->type == Pragma::PragmaSingleton) {
                createdUnit->flags |= QV4::CompiledData::Unit::IsSingleton;
                break;
            }
        }
        // This unit's memory was allocated with malloc on the heap, so it's
        // definitely not suitable for StaticData access.
        createdUnit->flags &= ~QV4::CompiledData::Unit::StaticData;

#ifndef V4_BOOTSTRAP
        if (dependencyHasher) {
            QCryptographicHash hash(QCryptographicHash::Md5);
            if (dependencyHasher(&hash)) {
                QByteArray checksum = hash.result();
                Q_ASSERT(checksum.size() == sizeof(createdUnit->dependencyMD5Checksum));
                memcpy(createdUnit->dependencyMD5Checksum, checksum.constData(), sizeof(createdUnit->dependencyMD5Checksum));
            }
        }
#else
        Q_UNUSED(dependencyHasher);
#endif
        createdUnit->sourceFileIndex = output.jsGenerator.stringTable.getStringId(output.jsModule.fileName);
        createdUnit->finalUrlIndex = output.jsGenerator.stringTable.getStringId(output.jsModule.finalUrl);
    }

    // No more new strings after this point, we're calculating offsets.
    output.jsGenerator.stringTable.freeze();

    const uint importSize = sizeof(QV4::CompiledData::Import) * output.imports.count();
    const uint objectOffsetTableSize = output.objects.count() * sizeof(quint32);

    QHash<const Object*, quint32> objectOffsets;

    const unsigned int objectOffset = sizeof(QV4::CompiledData::QmlUnit) + importSize;
    uint nextOffset = objectOffset + objectOffsetTableSize;
    for (Object *o : qAsConst(output.objects)) {
        objectOffsets.insert(o, nextOffset);
        nextOffset += QV4::CompiledData::Object::calculateSizeExcludingSignalsAndEnums(o->functionCount(), o->propertyCount(), o->aliasCount(), o->enumCount(), o->signalCount(), o->bindingCount(), o->namedObjectsInComponent.count);

        int signalTableSize = 0;
        for (const Signal *s = o->firstSignal(); s; s = s->next)
            signalTableSize += QV4::CompiledData::Signal::calculateSize(s->parameters->count);

        nextOffset += signalTableSize;

        int enumTableSize = 0;
        for (const Enum *e = o->firstEnum(); e; e = e->next)
            enumTableSize += QV4::CompiledData::Enum::calculateSize(e->enumValues->count);

        nextOffset += enumTableSize;
    }

    const uint totalSize = nextOffset;
    char *data = (char*)malloc(totalSize);
    memset(data, 0, totalSize);
    QV4::CompiledData::QmlUnit *qmlUnit = reinterpret_cast<QV4::CompiledData::QmlUnit *>(data);
    qmlUnit->offsetToImports = sizeof(*qmlUnit);
    qmlUnit->nImports = output.imports.count();
    qmlUnit->offsetToObjects = objectOffset;
    qmlUnit->nObjects = output.objects.count();

    // write imports
    char *importPtr = data + qmlUnit->offsetToImports;
    for (const QV4::CompiledData::Import *imp : qAsConst(output.imports)) {
        QV4::CompiledData::Import *importToWrite = reinterpret_cast<QV4::CompiledData::Import*>(importPtr);
        *importToWrite = *imp;
        importPtr += sizeof(QV4::CompiledData::Import);
    }

    // write objects
    quint32_le *objectTable = reinterpret_cast<quint32_le*>(data + qmlUnit->offsetToObjects);
    for (int i = 0; i < output.objects.count(); ++i) {
        const Object *o = output.objects.at(i);
        char * const objectPtr = data + objectOffsets.value(o);
        *objectTable++ = objectOffsets.value(o);

        QV4::CompiledData::Object *objectToWrite = reinterpret_cast<QV4::CompiledData::Object*>(objectPtr);
        objectToWrite->inheritedTypeNameIndex = o->inheritedTypeNameIndex;
        objectToWrite->indexOfDefaultPropertyOrAlias = o->indexOfDefaultPropertyOrAlias;
        objectToWrite->defaultPropertyIsAlias = o->defaultPropertyIsAlias;
        objectToWrite->flags = o->flags;
        objectToWrite->idNameIndex = o->idNameIndex;
        objectToWrite->id = o->id;
        objectToWrite->location = o->location;
        objectToWrite->locationOfIdProperty = o->locationOfIdProperty;

        quint32 nextOffset = sizeof(QV4::CompiledData::Object);

        objectToWrite->nFunctions = o->functionCount();
        objectToWrite->offsetToFunctions = nextOffset;
        nextOffset += objectToWrite->nFunctions * sizeof(quint32);

        objectToWrite->nProperties = o->propertyCount();
        objectToWrite->offsetToProperties = nextOffset;
        nextOffset += objectToWrite->nProperties * sizeof(QV4::CompiledData::Property);

        objectToWrite->nAliases = o->aliasCount();
        objectToWrite->offsetToAliases = nextOffset;
        nextOffset += objectToWrite->nAliases * sizeof(QV4::CompiledData::Alias);

        objectToWrite->nEnums = o->enumCount();
        objectToWrite->offsetToEnums = nextOffset;
        nextOffset += objectToWrite->nEnums * sizeof(quint32);

        objectToWrite->nSignals = o->signalCount();
        objectToWrite->offsetToSignals = nextOffset;
        nextOffset += objectToWrite->nSignals * sizeof(quint32);

        objectToWrite->nBindings = o->bindingCount();
        objectToWrite->offsetToBindings = nextOffset;
        nextOffset += objectToWrite->nBindings * sizeof(QV4::CompiledData::Binding);

        objectToWrite->nNamedObjectsInComponent = o->namedObjectsInComponent.count;
        objectToWrite->offsetToNamedObjectsInComponent = nextOffset;
        nextOffset += objectToWrite->nNamedObjectsInComponent * sizeof(quint32);

        quint32_le *functionsTable = reinterpret_cast<quint32_le *>(objectPtr + objectToWrite->offsetToFunctions);
        for (const Function *f = o->firstFunction(); f; f = f->next)
            *functionsTable++ = o->runtimeFunctionIndices.at(f->index);

        char *propertiesPtr = objectPtr + objectToWrite->offsetToProperties;
        for (const Property *p = o->firstProperty(); p; p = p->next) {
            QV4::CompiledData::Property *propertyToWrite = reinterpret_cast<QV4::CompiledData::Property*>(propertiesPtr);
            *propertyToWrite = *p;
            propertiesPtr += sizeof(QV4::CompiledData::Property);
        }

        char *aliasesPtr = objectPtr + objectToWrite->offsetToAliases;
        for (const Alias *a = o->firstAlias(); a; a = a->next) {
            QV4::CompiledData::Alias *aliasToWrite = reinterpret_cast<QV4::CompiledData::Alias*>(aliasesPtr);
            *aliasToWrite = *a;
            aliasesPtr += sizeof(QV4::CompiledData::Alias);
        }

        char *bindingPtr = objectPtr + objectToWrite->offsetToBindings;
        bindingPtr = writeBindings(bindingPtr, o, &QV4::CompiledData::Binding::isValueBindingNoAlias);
        bindingPtr = writeBindings(bindingPtr, o, &QV4::CompiledData::Binding::isSignalHandler);
        bindingPtr = writeBindings(bindingPtr, o, &QV4::CompiledData::Binding::isAttachedProperty);
        bindingPtr = writeBindings(bindingPtr, o, &QV4::CompiledData::Binding::isGroupProperty);
        bindingPtr = writeBindings(bindingPtr, o, &QV4::CompiledData::Binding::isValueBindingToAlias);
        Q_ASSERT((bindingPtr - objectToWrite->offsetToBindings - objectPtr) / sizeof(QV4::CompiledData::Binding) == unsigned(o->bindingCount()));

        quint32_le *signalOffsetTable = reinterpret_cast<quint32_le *>(objectPtr + objectToWrite->offsetToSignals);
        quint32 signalTableSize = 0;
        char *signalPtr = objectPtr + nextOffset;
        for (const Signal *s = o->firstSignal(); s; s = s->next) {
            *signalOffsetTable++ = signalPtr - objectPtr;
            QV4::CompiledData::Signal *signalToWrite = reinterpret_cast<QV4::CompiledData::Signal*>(signalPtr);

            signalToWrite->nameIndex = s->nameIndex;
            signalToWrite->location = s->location;
            signalToWrite->nParameters = s->parameters->count;

            QV4::CompiledData::Parameter *parameterToWrite = reinterpret_cast<QV4::CompiledData::Parameter*>(signalPtr + sizeof(*signalToWrite));
            for (SignalParameter *param = s->parameters->first; param; param = param->next, ++parameterToWrite)
                *parameterToWrite = *param;

            int size = QV4::CompiledData::Signal::calculateSize(s->parameters->count);
            signalTableSize += size;
            signalPtr += size;
        }
        nextOffset += signalTableSize;

        quint32_le *enumOffsetTable = reinterpret_cast<quint32_le*>(objectPtr + objectToWrite->offsetToEnums);
        quint32 enumTableSize = 0;
        char *enumPtr = objectPtr + nextOffset;
        for (const Enum *e = o->firstEnum(); e; e = e->next) {
            *enumOffsetTable++ = enumPtr - objectPtr;
            QV4::CompiledData::Enum *enumToWrite = reinterpret_cast<QV4::CompiledData::Enum*>(enumPtr);

            enumToWrite->nameIndex = e->nameIndex;
            enumToWrite->location = e->location;
            enumToWrite->nEnumValues = e->enumValues->count;

            QV4::CompiledData::EnumValue *enumValueToWrite = reinterpret_cast<QV4::CompiledData::EnumValue*>(enumPtr + sizeof(*enumToWrite));
            for (EnumValue *enumValue = e->enumValues->first; enumValue; enumValue = enumValue->next, ++enumValueToWrite)
                *enumValueToWrite = *enumValue;

            int size = QV4::CompiledData::Enum::calculateSize(e->enumValues->count);
            enumTableSize += size;
            enumPtr += size;
        }

        quint32_le *namedObjectInComponentPtr = reinterpret_cast<quint32_le *>(objectPtr + objectToWrite->offsetToNamedObjectsInComponent);
        for (int i = 0; i < o->namedObjectsInComponent.count; ++i) {
            *namedObjectInComponentPtr++ = o->namedObjectsInComponent.at(i);
        }
    }

    if (finalize) {
        // Combine the qml data into the general unit data.
        jsUnit = static_cast<QV4::CompiledData::Unit *>(realloc(jsUnit, jsUnit->unitSize + totalSize));
        jsUnit->offsetToQmlUnit = jsUnit->unitSize;
        jsUnit->unitSize += totalSize;
        memcpy(jsUnit->qmlUnit(), qmlUnit, totalSize);
        free(qmlUnit);
        jsUnit->generateChecksum();
        qmlUnit = jsUnit->qmlUnit();
    }

    static const bool showStats = qEnvironmentVariableIsSet("QML_SHOW_UNIT_STATS");
    if (showStats) {
        qDebug() << "Generated QML unit that is" << totalSize << "bytes big contains:";
        qDebug() << "    " << jsUnit->functionTableSize << "functions";
        qDebug() << "    " << jsUnit->unitSize << "for JS unit";
        qDebug() << "    " << importSize << "for imports";
        qDebug() << "    " << nextOffset - objectOffset - objectOffsetTableSize << "for" << qmlUnit->nObjects << "objects";
        quint32 totalBindingCount = 0;
        for (quint32 i = 0; i < qmlUnit->nObjects; ++i)
            totalBindingCount += qmlUnit->objectAt(i)->nBindings;
        qDebug() << "    " << totalBindingCount << "bindings";
        quint32 totalCodeSize = 0;
        for (quint32 i = 0; i < jsUnit->functionTableSize; ++i)
            totalCodeSize += jsUnit->functionAt(i)->codeSize;
        qDebug() << "    " << totalCodeSize << "bytes total byte code";
        qDebug() << "    " << jsUnit->stringTableSize << "strings";
        quint32 totalStringSize = 0;
        for (quint32 i = 0; i < jsUnit->stringTableSize; ++i)
            totalStringSize += QV4::CompiledData::String::calculateSize(jsUnit->stringAtInternal(i));
        qDebug() << "    " << totalStringSize << "bytes total strings";
    }

    compilationUnit->setUnitData(jsUnit, qmlUnit, output.jsModule.fileName, output.jsModule.finalUrl);
}

char *QmlUnitGenerator::writeBindings(char *bindingPtr, const Object *o, BindingFilter filter) const
{
    for (const Binding *b = o->firstBinding(); b; b = b->next) {
        if (!(b->*(filter))())
            continue;
        QV4::CompiledData::Binding *bindingToWrite = reinterpret_cast<QV4::CompiledData::Binding*>(bindingPtr);
        *bindingToWrite = *b;
        if (b->type == QV4::CompiledData::Binding::Type_Script)
            bindingToWrite->value.compiledScriptIndex = o->runtimeFunctionIndices.at(b->value.compiledScriptIndex);
        bindingPtr += sizeof(QV4::CompiledData::Binding);
    }
    return bindingPtr;
}

JSCodeGen::JSCodeGen(const QString &sourceCode, QV4::Compiler::JSUnitGenerator *jsUnitGenerator,
                     QV4::Compiler::Module *jsModule, QQmlJS::Engine *jsEngine,
                     QQmlJS::AST::UiProgram *qmlRoot,
                     const QV4::Compiler::StringTableGenerator *stringPool, const QSet<QString> &globalNames)
    : QV4::Compiler::Codegen(jsUnitGenerator, /*strict mode*/false)
    , sourceCode(sourceCode)
    , jsEngine(jsEngine)
    , qmlRoot(qmlRoot)
    , stringPool(stringPool)
{
    m_globalNames = globalNames;

    _module = jsModule;
    _fileNameIsUrl = true;
}

QVector<int> JSCodeGen::generateJSCodeForFunctionsAndBindings(const QList<CompiledFunctionOrExpression> &functions)
{
    auto qmlName = [&](const CompiledFunctionOrExpression &c) {
        if (c.nameIndex != 0)
            return stringPool->stringForIndex(c.nameIndex);
        else
            return QStringLiteral("%qml-expression-entry");
    };
    QVector<int> runtimeFunctionIndices(functions.size());

    QV4::Compiler::ScanFunctions scan(this, sourceCode, QV4::Compiler::ContextType::Global);
    scan.enterGlobalEnvironment(QV4::Compiler::ContextType::Binding);
    for (const CompiledFunctionOrExpression &f : functions) {
        Q_ASSERT(f.node != qmlRoot);
        Q_ASSERT(f.parentNode && f.parentNode != qmlRoot);
        QQmlJS::AST::FunctionDeclaration *function = QQmlJS::AST::cast<QQmlJS::AST::FunctionDeclaration*>(f.node);

        if (function) {
            scan.enterQmlFunction(function);
        } else {
            Q_ASSERT(f.node != f.parentNode);
            scan.enterEnvironment(f.parentNode, QV4::Compiler::ContextType::Binding, qmlName(f));
        }

        scan(function ? function->body : f.node);
        scan.leaveEnvironment();
    }
    scan.leaveEnvironment();

    if (hasError)
        return QVector<int>();

    _context = nullptr;

    for (int i = 0; i < functions.count(); ++i) {
        const CompiledFunctionOrExpression &qmlFunction = functions.at(i);
        QQmlJS::AST::Node *node = qmlFunction.node;
        Q_ASSERT(node != qmlRoot);

        QQmlJS::AST::FunctionDeclaration *function = QQmlJS::AST::cast<QQmlJS::AST::FunctionDeclaration*>(node);

        QString name;
        if (function)
            name = function->name.toString();
        else
            name = qmlName(qmlFunction);

        QQmlJS::AST::StatementList *body;
        if (function) {
            body = function->body;
        } else {
            // Synthesize source elements.
            QQmlJS::MemoryPool *pool = jsEngine->pool();

            QQmlJS::AST::Statement *stmt = node->statementCast();
            if (!stmt) {
                Q_ASSERT(node->expressionCast());
                QQmlJS::AST::ExpressionNode *expr = node->expressionCast();
                stmt = new (pool) QQmlJS::AST::ExpressionStatement(expr);
            }
            body = new (pool) QQmlJS::AST::StatementList(stmt);
            body = body->finish();
        }

        int idx = defineFunction(name, function ? function : qmlFunction.parentNode,
                                 function ? function->formals : nullptr,
                                 body);
        runtimeFunctionIndices[i] = idx;
    }

    return runtimeFunctionIndices;
}

#ifndef V4_BOOTSTRAP

QQmlPropertyData *PropertyResolver::property(const QString &name, bool *notInRevision, RevisionCheck check) const
{
    if (notInRevision) *notInRevision = false;

    QQmlPropertyData *d = cache->property(name, nullptr, nullptr);

    // Find the first property
    while (d && d->isFunction())
        d = cache->overrideData(d);

    if (check != IgnoreRevision && d && !cache->isAllowedInRevision(d)) {
        if (notInRevision) *notInRevision = true;
        return nullptr;
    } else {
        return d;
    }
}


QQmlPropertyData *PropertyResolver::signal(const QString &name, bool *notInRevision) const
{
    if (notInRevision) *notInRevision = false;

    QQmlPropertyData *d = cache->property(name, nullptr, nullptr);
    if (notInRevision) *notInRevision = false;

    while (d && !(d->isFunction()))
        d = cache->overrideData(d);

    if (d && !cache->isAllowedInRevision(d)) {
        if (notInRevision) *notInRevision = true;
        return nullptr;
    } else if (d && d->isSignal()) {
        return d;
    }

    if (name.endsWith(QLatin1String("Changed"))) {
        QString propName = name.mid(0, name.length() - static_cast<int>(strlen("Changed")));

        d = property(propName, notInRevision);
        if (d)
            return cache->signal(d->notifyIndex());
    }

    return nullptr;
}

IRLoader::IRLoader(const QV4::CompiledData::Unit *qmlData, QmlIR::Document *output)
    : unit(qmlData)
    , output(output)
{
    pool = output->jsParserEngine.pool();
}

void IRLoader::load()
{
    output->jsGenerator.stringTable.initializeFromBackingUnit(unit);

    const QV4::CompiledData::QmlUnit *qmlUnit = unit->qmlUnit();

    for (quint32 i = 0; i < qmlUnit->nImports; ++i)
        output->imports << qmlUnit->importAt(i);

    if (unit->flags & QV4::CompiledData::Unit::IsSingleton) {
        QmlIR::Pragma *p = New<QmlIR::Pragma>();
        p->location = QV4::CompiledData::Location();
        p->type = QmlIR::Pragma::PragmaSingleton;
        output->pragmas << p;
    }

    for (uint i = 0; i < qmlUnit->nObjects; ++i) {
        const QV4::CompiledData::Object *serializedObject = qmlUnit->objectAt(i);
        QmlIR::Object *object = loadObject(serializedObject);
        output->objects.append(object);
    }
}

struct FakeExpression : public QQmlJS::AST::NullExpression
{
    FakeExpression(int start, int length)
        : location(start, length)
    {}

    virtual QQmlJS::AST::SourceLocation firstSourceLocation() const
    { return location; }

    virtual QQmlJS::AST::SourceLocation lastSourceLocation() const
    { return location; }

private:
    QQmlJS::AST::SourceLocation location;
};

QmlIR::Object *IRLoader::loadObject(const QV4::CompiledData::Object *serializedObject)
{
    QmlIR::Object *object = pool->New<QmlIR::Object>();
    object->init(pool, serializedObject->inheritedTypeNameIndex, serializedObject->idNameIndex);

    object->indexOfDefaultPropertyOrAlias = serializedObject->indexOfDefaultPropertyOrAlias;
    object->defaultPropertyIsAlias = serializedObject->defaultPropertyIsAlias;
    object->flags = serializedObject->flags;
    object->id = serializedObject->id;
    object->location = serializedObject->location;
    object->locationOfIdProperty = serializedObject->locationOfIdProperty;

    QVector<int> functionIndices;
    functionIndices.reserve(serializedObject->nFunctions + serializedObject->nBindings / 2);

    for (uint i = 0; i < serializedObject->nBindings; ++i) {
        QmlIR::Binding *b = pool->New<QmlIR::Binding>();
        *static_cast<QV4::CompiledData::Binding*>(b) = serializedObject->bindingTable()[i];
        object->bindings->append(b);
        if (b->type == QV4::CompiledData::Binding::Type_Script) {
            functionIndices.append(b->value.compiledScriptIndex);
            b->value.compiledScriptIndex = functionIndices.count() - 1;

            QmlIR::CompiledFunctionOrExpression *foe = pool->New<QmlIR::CompiledFunctionOrExpression>();
            foe->nameIndex = 0;

            QQmlJS::AST::ExpressionNode *expr;

            if (b->stringIndex != quint32(0)) {
                const int start = output->code.length();
                const QString script = output->stringAt(b->stringIndex);
                const int length = script.length();
                output->code.append(script);
                expr = new (pool) FakeExpression(start, length);
            } else
                expr = new (pool) QQmlJS::AST::NullExpression();
            foe->node = new (pool) QQmlJS::AST::ExpressionStatement(expr); // dummy
            object->functionsAndExpressions->append(foe);
        }
    }

    Q_ASSERT(object->functionsAndExpressions->count == functionIndices.count());

    for (uint i = 0; i < serializedObject->nSignals; ++i) {
        const QV4::CompiledData::Signal *serializedSignal = serializedObject->signalAt(i);
        QmlIR::Signal *s = pool->New<QmlIR::Signal>();
        s->nameIndex = serializedSignal->nameIndex;
        s->location = serializedSignal->location;
        s->parameters = pool->New<QmlIR::PoolList<QmlIR::SignalParameter> >();

        for (uint i = 0; i < serializedSignal->nParameters; ++i) {
            QmlIR::SignalParameter *p = pool->New<QmlIR::SignalParameter>();
            *static_cast<QV4::CompiledData::Parameter*>(p) = *serializedSignal->parameterAt(i);
            s->parameters->append(p);
        }

        object->qmlSignals->append(s);
    }

    for (uint i = 0; i < serializedObject->nEnums; ++i) {
        const QV4::CompiledData::Enum *serializedEnum = serializedObject->enumAt(i);
        QmlIR::Enum *e = pool->New<QmlIR::Enum>();
        e->nameIndex = serializedEnum->nameIndex;
        e->location = serializedEnum->location;
        e->enumValues = pool->New<QmlIR::PoolList<QmlIR::EnumValue> >();

        for (uint i = 0; i < serializedEnum->nEnumValues; ++i) {
            QmlIR::EnumValue *v = pool->New<QmlIR::EnumValue>();
            *static_cast<QV4::CompiledData::EnumValue*>(v) = *serializedEnum->enumValueAt(i);
            e->enumValues->append(v);
        }

        object->qmlEnums->append(e);
    }

    const QV4::CompiledData::Property *serializedProperty = serializedObject->propertyTable();
    for (uint i = 0; i < serializedObject->nProperties; ++i, ++serializedProperty) {
        QmlIR::Property *p = pool->New<QmlIR::Property>();
        *static_cast<QV4::CompiledData::Property*>(p) = *serializedProperty;
        object->properties->append(p);
    }

    {
        const QV4::CompiledData::Alias *serializedAlias = serializedObject->aliasTable();
        for (uint i = 0; i < serializedObject->nAliases; ++i, ++serializedAlias) {
            QmlIR::Alias *a = pool->New<QmlIR::Alias>();
            *static_cast<QV4::CompiledData::Alias*>(a) = *serializedAlias;
            object->aliases->append(a);
        }
    }

    const quint32_le *functionIdx = serializedObject->functionOffsetTable();
    for (uint i = 0; i < serializedObject->nFunctions; ++i, ++functionIdx) {
        QmlIR::Function *f = pool->New<QmlIR::Function>();
        const QV4::CompiledData::Function *compiledFunction = unit->functionAt(*functionIdx);

        functionIndices.append(*functionIdx);
        f->index = functionIndices.count() - 1;
        f->location = compiledFunction->location;
        f->nameIndex = compiledFunction->nameIndex;

        const QString name = unit->stringAtInternal(compiledFunction->nameIndex);

        f->formals.allocate(pool, int(compiledFunction->nFormals));
        const quint32_le *formalNameIdx = compiledFunction->formalsTable();
        for (uint i = 0; i < compiledFunction->nFormals; ++i, ++formalNameIdx)
            f->formals[i] = *formalNameIdx;

        object->functions->append(f);
    }

    object->runtimeFunctionIndices.allocate(pool, functionIndices);

    return object;
}

#endif // V4_BOOTSTRAP
