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

#include "qqmltypecompiler_p.h"

#include <private/qqmlirbuilder_p.h>
#include <private/qqmlobjectcreator_p.h>
#include <private/qqmlcustomparser_p.h>
#include <private/qqmlvmemetaobject_p.h>
#include <private/qqmlcomponent_p.h>
#include <private/qqmldelegatecomponent_p.h>

#define COMPILE_EXCEPTION(token, desc) \
    { \
        recordError((token)->location, desc); \
        return false; \
    }

QT_BEGIN_NAMESPACE

QQmlTypeCompiler::QQmlTypeCompiler(QQmlEnginePrivate *engine, QQmlTypeData *typeData,
                                   QmlIR::Document *parsedQML, const QQmlRefPointer<QQmlTypeNameCache> &typeNameCache,
                                   QV4::CompiledData::ResolvedTypeReferenceMap *resolvedTypeCache, const QV4::CompiledData::DependentTypesHasher &dependencyHasher)
    : resolvedTypes(resolvedTypeCache)
    , engine(engine)
    , typeData(typeData)
    , dependencyHasher(dependencyHasher)
    , typeNameCache(typeNameCache)
    , document(parsedQML)
{
}

QQmlRefPointer<QV4::CompiledData::CompilationUnit> QQmlTypeCompiler::compile()
{
    // Build property caches and VME meta object data

    for (auto it = resolvedTypes->constBegin(), end = resolvedTypes->constEnd();
         it != end; ++it) {
        QQmlCustomParser *customParser = (*it)->type.customParser();
        if (customParser)
            customParsers.insert(it.key(), customParser);
    }

    QQmlPendingGroupPropertyBindings pendingGroupPropertyBindings;


    {
        QQmlPropertyCacheCreator<QQmlTypeCompiler> propertyCacheBuilder(&m_propertyCaches, &pendingGroupPropertyBindings,
                                                                        engine, this, imports());
        QQmlCompileError error = propertyCacheBuilder.buildMetaObjects();
        if (error.isSet()) {
            recordError(error);
            return nullptr;
        }
    }

    {
        QQmlDefaultPropertyMerger merger(this);
        merger.mergeDefaultProperties();
    }

    {
        SignalHandlerConverter converter(this);
        if (!converter.convertSignalHandlerExpressionsToFunctionDeclarations())
            return nullptr;
    }

    {
        QQmlEnumTypeResolver enumResolver(this);
        if (!enumResolver.resolveEnumBindings())
            return nullptr;
    }

    {
        QQmlCustomParserScriptIndexer cpi(this);
        cpi.annotateBindingsWithScriptStrings();
    }

    {
        QQmlAliasAnnotator annotator(this);
        annotator.annotateBindingsToAliases();
    }

    // Resolve component boundaries and aliases

    {
        // Scan for components, determine their scopes and resolve aliases within the scope.
        QQmlComponentAndAliasResolver resolver(this);
        if (!resolver.resolve())
            return nullptr;

        pendingGroupPropertyBindings.resolveMissingPropertyCaches(engine, &m_propertyCaches);
    }

    {
        QQmlDeferredAndCustomParserBindingScanner deferredAndCustomParserBindingScanner(this);
        if (!deferredAndCustomParserBindingScanner.scanObject())
            return nullptr;
    }

    if (!document->javaScriptCompilationUnit) {
        // Compile JS binding expressions and signal handlers if necessary
        {
            // We can compile script strings ahead of time, but they must be compiled
            // without type optimizations as their scope is always entirely dynamic.
            QQmlScriptStringScanner sss(this);
            sss.scan();
        }

        document->jsModule.fileName = typeData->urlString();
        document->jsModule.finalUrl = typeData->finalUrlString();
        QmlIR::JSCodeGen v4CodeGenerator(document->code, &document->jsGenerator, &document->jsModule, &document->jsParserEngine,
                                         document->program, &document->jsGenerator.stringTable, engine->v8engine()->illegalNames());
        QQmlJSCodeGenerator jsCodeGen(this, &v4CodeGenerator);
        if (!jsCodeGen.generateCodeForComponents())
            return nullptr;

        document->javaScriptCompilationUnit = v4CodeGenerator.generateCompilationUnit(/*generated unit data*/false);
    }

    // Generate QML compiled type data structures

    QmlIR::QmlUnitGenerator qmlGenerator;
    qmlGenerator.generate(*document, dependencyHasher);

    QQmlRefPointer<QV4::CompiledData::CompilationUnit> compilationUnit = document->javaScriptCompilationUnit;
    compilationUnit->typeNameCache = typeNameCache;
    compilationUnit->resolvedTypes = *resolvedTypes;
    compilationUnit->propertyCaches = std::move(m_propertyCaches);
    Q_ASSERT(compilationUnit->propertyCaches.count() == static_cast<int>(compilationUnit->objectCount()));

    if (errors.isEmpty())
        return compilationUnit;
    else
        return nullptr;
}

void QQmlTypeCompiler::recordError(QQmlError error)
{
    error.setUrl(url());
    errors << error;
}

void QQmlTypeCompiler::recordError(const QV4::CompiledData::Location &location, const QString &description)
{
    QQmlError error;
    error.setLine(location.line);
    error.setColumn(location.column);
    error.setDescription(description);
    error.setUrl(url());
    errors << error;
}

void QQmlTypeCompiler::recordError(const QQmlCompileError &error)
{
    recordError(error.location, error.description);
}

QString QQmlTypeCompiler::stringAt(int idx) const
{
    return document->stringAt(idx);
}

int QQmlTypeCompiler::registerString(const QString &str)
{
    return document->jsGenerator.registerString(str);
}

int QQmlTypeCompiler::registerConstant(QV4::ReturnedValue v)
{
    return document->jsGenerator.registerConstant(v);
}

const QV4::CompiledData::Unit *QQmlTypeCompiler::qmlUnit() const
{
    return document->javaScriptCompilationUnit->unitData();
}

const QQmlImports *QQmlTypeCompiler::imports() const
{
    return &typeData->imports();
}

QVector<QmlIR::Object *> *QQmlTypeCompiler::qmlObjects() const
{
    return &document->objects;
}

void QQmlTypeCompiler::setPropertyCaches(QQmlPropertyCacheVector &&caches)
{
    m_propertyCaches = std::move(caches);
    Q_ASSERT(m_propertyCaches.count() > 0);
}

const QQmlPropertyCacheVector *QQmlTypeCompiler::propertyCaches() const
{
    return &m_propertyCaches;
}

QQmlPropertyCacheVector &&QQmlTypeCompiler::takePropertyCaches()
{
    return std::move(m_propertyCaches);
}

QQmlJS::MemoryPool *QQmlTypeCompiler::memoryPool()
{
    return document->jsParserEngine.pool();
}

QStringRef QQmlTypeCompiler::newStringRef(const QString &string)
{
    return document->jsParserEngine.newStringRef(string);
}

const QV4::Compiler::StringTableGenerator *QQmlTypeCompiler::stringPool() const
{
    return &document->jsGenerator.stringTable;
}

void QQmlTypeCompiler::setBindingPropertyDataPerObject(const QVector<QV4::CompiledData::BindingPropertyData> &propertyData)
{
    document->javaScriptCompilationUnit->bindingPropertyDataPerObject = propertyData;
}

QString QQmlTypeCompiler::bindingAsString(const QmlIR::Object *object, int scriptIndex) const
{
    return object->bindingAsString(document, scriptIndex);
}

void QQmlTypeCompiler::addImport(const QString &module, const QString &qualifier, int majorVersion, int minorVersion)
{
    const quint32 moduleIdx = registerString(module);
    const quint32 qualifierIdx = registerString(qualifier);

    for (int i = 0, count = document->imports.count(); i < count; ++i) {
        const QV4::CompiledData::Import *existingImport = document->imports.at(i);
        if (existingImport->type == QV4::CompiledData::Import::ImportLibrary
            && existingImport->uriIndex == moduleIdx
            && existingImport->qualifierIndex == qualifierIdx)
            return;
    }
    auto pool = memoryPool();
    QV4::CompiledData::Import *import = pool->New<QV4::CompiledData::Import>();
    import->type = QV4::CompiledData::Import::ImportLibrary;
    import->majorVersion = majorVersion;
    import->minorVersion = minorVersion;
    import->uriIndex = moduleIdx;
    import->qualifierIndex = qualifierIdx;
    document->imports.append(import);
}

QQmlCompilePass::QQmlCompilePass(QQmlTypeCompiler *typeCompiler)
    : compiler(typeCompiler)
{
}



SignalHandlerConverter::SignalHandlerConverter(QQmlTypeCompiler *typeCompiler)
    : QQmlCompilePass(typeCompiler)
    , enginePrivate(typeCompiler->enginePrivate())
    , qmlObjects(*typeCompiler->qmlObjects())
    , imports(typeCompiler->imports())
    , customParsers(typeCompiler->customParserCache())
    , illegalNames(typeCompiler->enginePrivate()->v8engine()->illegalNames())
    , propertyCaches(typeCompiler->propertyCaches())
{
}

bool SignalHandlerConverter::convertSignalHandlerExpressionsToFunctionDeclarations()
{
    for (int objectIndex = 0; objectIndex < qmlObjects.count(); ++objectIndex) {
        const QmlIR::Object * const obj = qmlObjects.at(objectIndex);
        QQmlPropertyCache *cache = propertyCaches->at(objectIndex);
        if (!cache)
            continue;
        if (QQmlCustomParser *customParser = customParsers.value(obj->inheritedTypeNameIndex)) {
            if (!(customParser->flags() & QQmlCustomParser::AcceptsSignalHandlers))
                continue;
        }
        const QString elementName = stringAt(obj->inheritedTypeNameIndex);
        if (!convertSignalHandlerExpressionsToFunctionDeclarations(obj, elementName, cache))
            return false;
    }
    return true;
}

bool SignalHandlerConverter::convertSignalHandlerExpressionsToFunctionDeclarations(const QmlIR::Object *obj, const QString &typeName, QQmlPropertyCache *propertyCache)
{
    // map from signal name defined in qml itself to list of parameters
    QHash<QString, QStringList> customSignals;

    for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
        QString propertyName = stringAt(binding->propertyNameIndex);
        // Attached property?
        if (binding->type == QV4::CompiledData::Binding::Type_AttachedProperty) {
            const QmlIR::Object *attachedObj = qmlObjects.at(binding->value.objectIndex);
            auto *typeRef = resolvedType(binding->propertyNameIndex);
            QQmlType type = typeRef ? typeRef->type : QQmlType();
            if (!type.isValid()) {
                if (imports->resolveType(propertyName, &type, nullptr, nullptr, nullptr)) {
                    if (type.isComposite()) {
                        QQmlRefPointer<QQmlTypeData> tdata = enginePrivate->typeLoader.getType(type.sourceUrl());
                        Q_ASSERT(tdata);
                        Q_ASSERT(tdata->isComplete());

                        auto compilationUnit = tdata->compilationUnit();
                        type = QQmlMetaType::qmlType(compilationUnit->metaTypeId);
                    }
                }
            }

            const QMetaObject *attachedType = type.attachedPropertiesType(enginePrivate);
            if (!attachedType)
                COMPILE_EXCEPTION(binding, tr("Non-existent attached object"));
            QQmlPropertyCache *cache = compiler->enginePrivate()->cache(attachedType);
            if (!convertSignalHandlerExpressionsToFunctionDeclarations(attachedObj, propertyName, cache))
                return false;
            continue;
        }

        if (!QmlIR::IRBuilder::isSignalPropertyName(propertyName))
            continue;

        QmlIR::PropertyResolver resolver(propertyCache);

        Q_ASSERT(propertyName.startsWith(QLatin1String("on")));
        propertyName.remove(0, 2);

        // Note that the property name could start with any alpha or '_' or '$' character,
        // so we need to do the lower-casing of the first alpha character.
        for (int firstAlphaIndex = 0; firstAlphaIndex < propertyName.size(); ++firstAlphaIndex) {
            if (propertyName.at(firstAlphaIndex).isUpper()) {
                propertyName[firstAlphaIndex] = propertyName.at(firstAlphaIndex).toLower();
                break;
            }
        }

        QList<QString> parameters;

        bool notInRevision = false;
        QQmlPropertyData *signal = resolver.signal(propertyName, &notInRevision);
        if (signal) {
            int sigIndex = propertyCache->methodIndexToSignalIndex(signal->coreIndex());
            sigIndex = propertyCache->originalClone(sigIndex);

            bool unnamedParameter = false;

            QList<QByteArray> parameterNames = propertyCache->signalParameterNames(sigIndex);
            for (int i = 0; i < parameterNames.count(); ++i) {
                const QString param = QString::fromUtf8(parameterNames.at(i));
                if (param.isEmpty())
                    unnamedParameter = true;
                else if (unnamedParameter) {
                    COMPILE_EXCEPTION(binding, tr("Signal uses unnamed parameter followed by named parameter."));
                } else if (illegalNames.contains(param)) {
                    COMPILE_EXCEPTION(binding, tr("Signal parameter \"%1\" hides global variable.").arg(param));
                }
                parameters += param;
            }
        } else {
            if (notInRevision) {
                // Try assinging it as a property later
                if (resolver.property(propertyName, /*notInRevision ptr*/nullptr))
                    continue;

                const QString &originalPropertyName = stringAt(binding->propertyNameIndex);

                auto *typeRef = resolvedType(obj->inheritedTypeNameIndex);
                const QQmlType type = typeRef ? typeRef->type : QQmlType();
                if (type.isValid()) {
                    COMPILE_EXCEPTION(binding, tr("\"%1.%2\" is not available in %3 %4.%5.").arg(typeName).arg(originalPropertyName).arg(type.module()).arg(type.majorVersion()).arg(type.minorVersion()));
                } else {
                    COMPILE_EXCEPTION(binding, tr("\"%1.%2\" is not available due to component versioning.").arg(typeName).arg(originalPropertyName));
                }
            }

            // Try to look up the signal parameter names in the object itself

            // build cache if necessary
            if (customSignals.isEmpty()) {
                for (const QmlIR::Signal *signal = obj->firstSignal(); signal; signal = signal->next) {
                    const QString &signalName = stringAt(signal->nameIndex);
                    customSignals.insert(signalName, signal->parameterStringList(compiler->stringPool()));
                }

                for (const QmlIR::Property *property = obj->firstProperty(); property; property = property->next) {
                    const QString propName = stringAt(property->nameIndex);
                    customSignals.insert(propName, QStringList());
                }
            }

            QHash<QString, QStringList>::ConstIterator entry = customSignals.constFind(propertyName);
            if (entry == customSignals.constEnd() && propertyName.endsWith(QLatin1String("Changed"))) {
                QString alternateName = propertyName.mid(0, propertyName.length() - static_cast<int>(strlen("Changed")));
                entry = customSignals.constFind(alternateName);
            }

            if (entry == customSignals.constEnd()) {
                // Can't find even a custom signal, then just don't do anything and try
                // keeping the binding as a regular property assignment.
                continue;
            }

            parameters = entry.value();
        }

        // Binding object to signal means connect the signal to the object's default method.
        if (binding->type == QV4::CompiledData::Binding::Type_Object) {
            binding->flags |= QV4::CompiledData::Binding::IsSignalHandlerObject;
            continue;
        }

        if (binding->type != QV4::CompiledData::Binding::Type_Script) {
            if (binding->type < QV4::CompiledData::Binding::Type_Script) {
                COMPILE_EXCEPTION(binding, tr("Cannot assign a value to a signal (expecting a script to be run)"));
            } else {
                COMPILE_EXCEPTION(binding, tr("Incorrectly specified signal assignment"));
            }
        }

        QQmlJS::MemoryPool *pool = compiler->memoryPool();

        QQmlJS::AST::FormalParameterList *paramList = nullptr;
        for (const QString &param : qAsConst(parameters)) {
            QStringRef paramNameRef = compiler->newStringRef(param);

            QQmlJS::AST::PatternElement *b = new (pool) QQmlJS::AST::PatternElement(paramNameRef, nullptr);
            paramList = new (pool) QQmlJS::AST::FormalParameterList(paramList, b);
        }

        if (paramList)
            paramList = paramList->finish(pool);

        QmlIR::CompiledFunctionOrExpression *foe = obj->functionsAndExpressions->slowAt(binding->value.compiledScriptIndex);
        QQmlJS::AST::FunctionDeclaration *functionDeclaration = nullptr;
        if (QQmlJS::AST::ExpressionStatement *es = QQmlJS::AST::cast<QQmlJS::AST::ExpressionStatement*>(foe->node)) {
            if (QQmlJS::AST::FunctionExpression *fe = QQmlJS::AST::cast<QQmlJS::AST::FunctionExpression*>(es->expression)) {
                functionDeclaration = new (pool) QQmlJS::AST::FunctionDeclaration(fe->name, fe->formals, fe->body);
                functionDeclaration->functionToken = fe->functionToken;
                functionDeclaration->identifierToken = fe->identifierToken;
                functionDeclaration->lparenToken = fe->lparenToken;
                functionDeclaration->rparenToken = fe->rparenToken;
                functionDeclaration->lbraceToken = fe->lbraceToken;
                functionDeclaration->rbraceToken = fe->rbraceToken;
            }
        }
        if (!functionDeclaration) {
            QQmlJS::AST::Statement *statement = static_cast<QQmlJS::AST::Statement*>(foe->node);
            QQmlJS::AST::StatementList *body = new (pool) QQmlJS::AST::StatementList(statement);
            body = body->finish();

            functionDeclaration = new (pool) QQmlJS::AST::FunctionDeclaration(compiler->newStringRef(stringAt(binding->propertyNameIndex)), paramList, body);
            functionDeclaration->lbraceToken = functionDeclaration->functionToken
                    = foe->node->firstSourceLocation();
            functionDeclaration->rbraceToken = foe->node->lastSourceLocation();
        }
        foe->node = functionDeclaration;
        binding->propertyNameIndex = compiler->registerString(propertyName);
        binding->flags |= QV4::CompiledData::Binding::IsSignalHandlerExpression;
    }
    return true;
}

QQmlEnumTypeResolver::QQmlEnumTypeResolver(QQmlTypeCompiler *typeCompiler)
    : QQmlCompilePass(typeCompiler)
    , qmlObjects(*typeCompiler->qmlObjects())
    , propertyCaches(typeCompiler->propertyCaches())
    , imports(typeCompiler->imports())
{
}

bool QQmlEnumTypeResolver::resolveEnumBindings()
{
    for (int i = 0; i < qmlObjects.count(); ++i) {
        QQmlPropertyCache *propertyCache = propertyCaches->at(i);
        if (!propertyCache)
            continue;
        const QmlIR::Object *obj = qmlObjects.at(i);

        QmlIR::PropertyResolver resolver(propertyCache);

        for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
            if (binding->flags & QV4::CompiledData::Binding::IsSignalHandlerExpression
                || binding->flags & QV4::CompiledData::Binding::IsSignalHandlerObject)
                continue;

            if (binding->type != QV4::CompiledData::Binding::Type_Script)
                continue;

            const QString propertyName = stringAt(binding->propertyNameIndex);
            bool notInRevision = false;
            QQmlPropertyData *pd = resolver.property(propertyName, &notInRevision);
            if (!pd)
                continue;

            if (!pd->isEnum() && pd->propType() != QMetaType::Int)
                continue;

            if (!tryQualifiedEnumAssignment(obj, propertyCache, pd, binding))
                return false;
        }
    }

    return true;
}

struct StaticQtMetaObject : public QObject
{
    static const QMetaObject *get()
        { return &staticQtMetaObject; }
};

bool QQmlEnumTypeResolver::assignEnumToBinding(QmlIR::Binding *binding, const QStringRef &enumName, int enumValue, bool isQtObject)
{
    if (enumName.length() > 0 && enumName[0].isLower() && !isQtObject) {
        COMPILE_EXCEPTION(binding, tr("Invalid property assignment: Enum value \"%1\" cannot start with a lowercase letter").arg(enumName.toString()));
    }
    binding->type = QV4::CompiledData::Binding::Type_Number;
    binding->value.constantValueIndex = compiler->registerConstant(QV4::Encode((double)enumValue));
//    binding->setNumberValueInternal((double)enumValue);
    binding->flags |= QV4::CompiledData::Binding::IsResolvedEnum;
    return true;
}

bool QQmlEnumTypeResolver::tryQualifiedEnumAssignment(const QmlIR::Object *obj, const QQmlPropertyCache *propertyCache, const QQmlPropertyData *prop, QmlIR::Binding *binding)
{
    bool isIntProp = (prop->propType() == QMetaType::Int) && !prop->isEnum();
    if (!prop->isEnum() && !isIntProp)
        return true;

    if (!prop->isWritable() && !(binding->flags & QV4::CompiledData::Binding::InitializerForReadOnlyDeclaration))
        COMPILE_EXCEPTION(binding, tr("Invalid property assignment: \"%1\" is a read-only property").arg(stringAt(binding->propertyNameIndex)));

    Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_Script);
    const QString string = compiler->bindingAsString(obj, binding->value.compiledScriptIndex);
    if (!string.constData()->isUpper())
        return true;

    // we support one or two '.' in the enum phrase:
    // * <TypeName>.<EnumValue>
    // * <TypeName>.<ScopedEnumName>.<EnumValue>

    int dot = string.indexOf(QLatin1Char('.'));
    if (dot == -1 || dot == string.length()-1)
        return true;

    int dot2 = string.indexOf(QLatin1Char('.'), dot+1);
    if (dot2 != -1 && dot2 != string.length()-1) {
        if (!string.at(dot+1).isUpper())
            return true;
        if (string.indexOf(QLatin1Char('.'), dot2+1) != -1)
            return true;
    }

    QHashedStringRef typeName(string.constData(), dot);
    const bool isQtObject = (typeName == QLatin1String("Qt"));
    const QStringRef scopedEnumName = (dot2 != -1 ? string.midRef(dot + 1, dot2 - dot - 1) : QStringRef());
    // ### consider supporting scoped enums in Qt namespace
    const QStringRef enumValue = string.midRef(!isQtObject && dot2 != -1 ? dot2 + 1 : dot + 1);

    if (isIntProp) { // ### C++11 allows enums to be other integral types. Should we support other integral types here?
        // Allow enum assignment to ints.
        bool ok;
        int enumval = evaluateEnum(typeName.toString(), scopedEnumName, enumValue, &ok);
        if (ok) {
            if (!assignEnumToBinding(binding, enumValue, enumval, isQtObject))
                return false;
        }
        return true;
    }
    QQmlType type;
    imports->resolveType(typeName, &type, nullptr, nullptr, nullptr);

    if (!type.isValid() && !isQtObject)
        return true;

    int value = 0;
    bool ok = false;

    auto *tr = resolvedType(obj->inheritedTypeNameIndex);
    if (type.isValid() && tr && tr->type == type) {
        // When these two match, we can short cut the search
        QMetaProperty mprop = propertyCache->firstCppMetaObject()->property(prop->coreIndex());
        QMetaEnum menum = mprop.enumerator();
        QByteArray enumName = enumValue.toUtf8();
        if (menum.isScoped() && !scopedEnumName.isEmpty() && enumName != scopedEnumName.toUtf8())
            return true;

        if (mprop.isFlagType()) {
            value = menum.keysToValue(enumName.constData(), &ok);
        } else {
            value = menum.keyToValue(enumName.constData(), &ok);
        }
    } else {
        // Otherwise we have to search the whole type
        if (type.isValid()) {
            if (!scopedEnumName.isEmpty())
                value = type.scopedEnumValue(compiler->enginePrivate(), scopedEnumName, enumValue, &ok);
            else
                value = type.enumValue(compiler->enginePrivate(), QHashedStringRef(enumValue), &ok);
        } else {
            QByteArray enumName = enumValue.toUtf8();
            const QMetaObject *metaObject = StaticQtMetaObject::get();
            for (int ii = metaObject->enumeratorCount() - 1; !ok && ii >= 0; --ii) {
                QMetaEnum e = metaObject->enumerator(ii);
                value = e.keyToValue(enumName.constData(), &ok);
            }
        }
    }

    if (!ok)
        return true;

    return assignEnumToBinding(binding, enumValue, value, isQtObject);
}

int QQmlEnumTypeResolver::evaluateEnum(const QString &scope, const QStringRef &enumName, const QStringRef &enumValue, bool *ok) const
{
    Q_ASSERT_X(ok, "QQmlEnumTypeResolver::evaluateEnum", "ok must not be a null pointer");
    *ok = false;

    if (scope != QLatin1String("Qt")) {
        QQmlType type;
        imports->resolveType(scope, &type, nullptr, nullptr, nullptr);
        if (!type.isValid())
            return -1;
        if (!enumName.isEmpty())
            return type.scopedEnumValue(compiler->enginePrivate(), enumName, enumValue, ok);
        return type.enumValue(compiler->enginePrivate(), QHashedStringRef(enumValue.constData(), enumValue.length()), ok);
    }

    const QMetaObject *mo = StaticQtMetaObject::get();
    int i = mo->enumeratorCount();
    const QByteArray ba = enumValue.toUtf8();
    while (i--) {
        int v = mo->enumerator(i).keyToValue(ba.constData(), ok);
        if (*ok)
            return v;
    }
    return -1;
}

QQmlCustomParserScriptIndexer::QQmlCustomParserScriptIndexer(QQmlTypeCompiler *typeCompiler)
    : QQmlCompilePass(typeCompiler)
    , qmlObjects(*typeCompiler->qmlObjects())
    , customParsers(typeCompiler->customParserCache())
{
}

void QQmlCustomParserScriptIndexer::annotateBindingsWithScriptStrings()
{
    scanObjectRecursively(/*root object*/0);
}

void QQmlCustomParserScriptIndexer::scanObjectRecursively(int objectIndex, bool annotateScriptBindings)
{
    const QmlIR::Object * const obj = qmlObjects.at(objectIndex);
    if (!annotateScriptBindings)
        annotateScriptBindings = customParsers.contains(obj->inheritedTypeNameIndex);
    for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
        if (binding->type >= QV4::CompiledData::Binding::Type_Object) {
            scanObjectRecursively(binding->value.objectIndex, annotateScriptBindings);
            continue;
        } else if (binding->type != QV4::CompiledData::Binding::Type_Script)
            continue;
        if (!annotateScriptBindings)
            continue;
        const QString script = compiler->bindingAsString(obj, binding->value.compiledScriptIndex);
        binding->stringIndex = compiler->registerString(script);
    }
}

QQmlAliasAnnotator::QQmlAliasAnnotator(QQmlTypeCompiler *typeCompiler)
    : QQmlCompilePass(typeCompiler)
    , qmlObjects(*typeCompiler->qmlObjects())
    , propertyCaches(typeCompiler->propertyCaches())
{
}

void QQmlAliasAnnotator::annotateBindingsToAliases()
{
    for (int i = 0; i < qmlObjects.count(); ++i) {
        QQmlPropertyCache *propertyCache = propertyCaches->at(i);
        if (!propertyCache)
            continue;

        const QmlIR::Object *obj = qmlObjects.at(i);

        QmlIR::PropertyResolver resolver(propertyCache);
        QQmlPropertyData *defaultProperty = obj->indexOfDefaultPropertyOrAlias != -1 ? propertyCache->parent()->defaultProperty() : propertyCache->defaultProperty();

        for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
            if (!binding->isValueBinding())
                continue;
            bool notInRevision = false;
            QQmlPropertyData *pd = binding->propertyNameIndex != quint32(0) ? resolver.property(stringAt(binding->propertyNameIndex), &notInRevision) : defaultProperty;
            if (pd && pd->isAlias())
                binding->flags |= QV4::CompiledData::Binding::IsBindingToAlias;
        }
    }
}

QQmlScriptStringScanner::QQmlScriptStringScanner(QQmlTypeCompiler *typeCompiler)
    : QQmlCompilePass(typeCompiler)
    , qmlObjects(*typeCompiler->qmlObjects())
    , propertyCaches(typeCompiler->propertyCaches())
{

}

void QQmlScriptStringScanner::scan()
{
    const int scriptStringMetaType = qMetaTypeId<QQmlScriptString>();
    for (int i = 0; i < qmlObjects.count(); ++i) {
        QQmlPropertyCache *propertyCache = propertyCaches->at(i);
        if (!propertyCache)
            continue;

        const QmlIR::Object *obj = qmlObjects.at(i);

        QmlIR::PropertyResolver resolver(propertyCache);
        QQmlPropertyData *defaultProperty = obj->indexOfDefaultPropertyOrAlias != -1 ? propertyCache->parent()->defaultProperty() : propertyCache->defaultProperty();

        for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
            if (binding->type != QV4::CompiledData::Binding::Type_Script)
                continue;
            bool notInRevision = false;
            QQmlPropertyData *pd = binding->propertyNameIndex != quint32(0) ? resolver.property(stringAt(binding->propertyNameIndex), &notInRevision) : defaultProperty;
            if (!pd || pd->propType() != scriptStringMetaType)
                continue;

            QString script = compiler->bindingAsString(obj, binding->value.compiledScriptIndex);
            binding->stringIndex = compiler->registerString(script);
        }
    }
}

QQmlComponentAndAliasResolver::QQmlComponentAndAliasResolver(QQmlTypeCompiler *typeCompiler)
    : QQmlCompilePass(typeCompiler)
    , enginePrivate(typeCompiler->enginePrivate())
    , pool(typeCompiler->memoryPool())
    , qmlObjects(typeCompiler->qmlObjects())
    , propertyCaches(std::move(typeCompiler->takePropertyCaches()))
{
}

void QQmlComponentAndAliasResolver::findAndRegisterImplicitComponents(const QmlIR::Object *obj, QQmlPropertyCache *propertyCache)
{
    QmlIR::PropertyResolver propertyResolver(propertyCache);

    QQmlPropertyData *defaultProperty = obj->indexOfDefaultPropertyOrAlias != -1 ? propertyCache->parent()->defaultProperty() : propertyCache->defaultProperty();

    for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
        if (binding->type != QV4::CompiledData::Binding::Type_Object)
            continue;
        if (binding->flags & QV4::CompiledData::Binding::IsSignalHandlerObject)
            continue;

        const QmlIR::Object *targetObject = qmlObjects->at(binding->value.objectIndex);
        auto *tr = resolvedType(targetObject->inheritedTypeNameIndex);
        Q_ASSERT(tr);

        const QMetaObject *firstMetaObject = nullptr;
        if (tr->type.isValid())
            firstMetaObject = tr->type.metaObject();
        else if (tr->compilationUnit)
            firstMetaObject = tr->compilationUnit->rootPropertyCache()->firstCppMetaObject();
        // 1: test for QQmlComponent
        if (firstMetaObject && firstMetaObject == &QQmlComponent::staticMetaObject)
            continue;
        // 2: test for QQmlAbstractDelegateComponent
        while (firstMetaObject && firstMetaObject != &QQmlAbstractDelegateComponent::staticMetaObject)
            firstMetaObject = firstMetaObject->superClass();
        if (firstMetaObject)
            continue;
        // if here, not a QQmlComponent or a QQmlAbstractDelegateComponent, so needs wrapping

        QQmlPropertyData *pd = nullptr;
        if (binding->propertyNameIndex != quint32(0)) {
            bool notInRevision = false;
            pd = propertyResolver.property(stringAt(binding->propertyNameIndex), &notInRevision);
        } else {
            pd = defaultProperty;
        }
        if (!pd || !pd->isQObject())
            continue;

        QQmlPropertyCache *pc = enginePrivate->rawPropertyCacheForType(pd->propType(), pd->typeMinorVersion());
        const QMetaObject *mo = pc ? pc->firstCppMetaObject() : nullptr;
        while (mo) {
            if (mo == &QQmlComponent::staticMetaObject)
                break;
            mo = mo->superClass();
        }

        if (!mo)
            continue;

        // emulate "import Qml 2.0 as QmlInternals" and then wrap the component in "QmlInternals.Component {}"
        QQmlType componentType = QQmlMetaType::qmlType(&QQmlComponent::staticMetaObject);
        Q_ASSERT(componentType.isValid());
        const QString qualifier = QStringLiteral("QmlInternals");

        compiler->addImport(componentType.module(), qualifier, componentType.majorVersion(), componentType.minorVersion());

        QmlIR::Object *syntheticComponent = pool->New<QmlIR::Object>();
        syntheticComponent->init(pool, compiler->registerString(qualifier + QLatin1Char('.') + componentType.elementName()), compiler->registerString(QString()));
        syntheticComponent->location = binding->valueLocation;
        syntheticComponent->flags |= QV4::CompiledData::Object::IsComponent;

        if (!containsResolvedType(syntheticComponent->inheritedTypeNameIndex)) {
            auto typeRef = new QV4::CompiledData::ResolvedTypeReference;
            typeRef->type = componentType;
            typeRef->majorVersion = componentType.majorVersion();
            typeRef->minorVersion = componentType.minorVersion();
            insertResolvedType(syntheticComponent->inheritedTypeNameIndex, typeRef);
        }

        qmlObjects->append(syntheticComponent);
        const int componentIndex = qmlObjects->count() - 1;
        // Keep property caches symmetric
        QQmlPropertyCache *componentCache = enginePrivate->cache(&QQmlComponent::staticMetaObject);
        propertyCaches.append(componentCache);

        QmlIR::Binding *syntheticBinding = pool->New<QmlIR::Binding>();
        *syntheticBinding = *binding;
        syntheticBinding->type = QV4::CompiledData::Binding::Type_Object;
        QString error = syntheticComponent->appendBinding(syntheticBinding, /*isListBinding*/false);
        Q_ASSERT(error.isEmpty());
        Q_UNUSED(error);

        binding->value.objectIndex = componentIndex;

        componentRoots.append(componentIndex);
    }
}

bool QQmlComponentAndAliasResolver::resolve()
{
    // Detect real Component {} objects as well as implicitly defined components, such as
    //     someItemDelegate: Item {}
    // In the implicit case Item is surrounded by a synthetic Component {} because the property
    // on the left hand side is of QQmlComponent type.
    const int objCountWithoutSynthesizedComponents = qmlObjects->count();
    for (int i = 0; i < objCountWithoutSynthesizedComponents; ++i) {
        QmlIR::Object *obj = qmlObjects->at(i);
        QQmlPropertyCache *cache = propertyCaches.at(i);
        if (obj->inheritedTypeNameIndex == 0 && !cache)
            continue;

        bool isExplicitComponent = false;

        if (obj->inheritedTypeNameIndex) {
            auto *tref = resolvedType(obj->inheritedTypeNameIndex);
            Q_ASSERT(tref);
            if (tref->type.metaObject() == &QQmlComponent::staticMetaObject)
                isExplicitComponent = true;
        }
        if (!isExplicitComponent) {
            if (cache)
                findAndRegisterImplicitComponents(obj, cache);
            continue;
        }

        obj->flags |= QV4::CompiledData::Object::IsComponent;

        if (obj->functionCount() > 0)
            COMPILE_EXCEPTION(obj, tr("Component objects cannot declare new functions."));
        if (obj->propertyCount() > 0 || obj->aliasCount() > 0)
            COMPILE_EXCEPTION(obj, tr("Component objects cannot declare new properties."));
        if (obj->signalCount() > 0)
            COMPILE_EXCEPTION(obj, tr("Component objects cannot declare new signals."));

        if (obj->bindingCount() == 0)
            COMPILE_EXCEPTION(obj, tr("Cannot create empty component specification"));

        const QmlIR::Binding *rootBinding = obj->firstBinding();

        for (const QmlIR::Binding *b = rootBinding; b; b = b->next) {
            if (b->propertyNameIndex != 0)
                COMPILE_EXCEPTION(rootBinding, tr("Component elements may not contain properties other than id"));
        }

        if (rootBinding->next || rootBinding->type != QV4::CompiledData::Binding::Type_Object)
            COMPILE_EXCEPTION(obj, tr("Invalid component body specification"));

        // For the root object, we are going to collect ids/aliases and resolve them for as a separate
        // last pass.
        if (i != 0)
            componentRoots.append(i);

    }

    for (int i = 0; i < componentRoots.count(); ++i) {
        QmlIR::Object *component  = qmlObjects->at(componentRoots.at(i));
        const QmlIR::Binding *rootBinding = component->firstBinding();

        _idToObjectIndex.clear();

        _objectsWithAliases.clear();

        if (!collectIdsAndAliases(rootBinding->value.objectIndex))
            return false;

        component->namedObjectsInComponent.allocate(pool, _idToObjectIndex);

        if (!resolveAliases(componentRoots.at(i)))
            return false;
    }

    // Collect ids and aliases for root
    _idToObjectIndex.clear();
    _objectsWithAliases.clear();

    collectIdsAndAliases(/*root object*/0);

    QmlIR::Object *rootComponent = qmlObjects->at(/*root object*/0);
    rootComponent->namedObjectsInComponent.allocate(pool, _idToObjectIndex);

    if (!resolveAliases(/*root object*/0))
        return false;

    // Implicit component insertion may have added objects and thus we also need
    // to extend the symmetric propertyCaches.
    compiler->setPropertyCaches(std::move(propertyCaches));
    compiler->setComponentRoots(componentRoots);

    return true;
}

bool QQmlComponentAndAliasResolver::collectIdsAndAliases(int objectIndex)
{
    QmlIR::Object *obj = qmlObjects->at(objectIndex);

    if (obj->idNameIndex != 0) {
        if (_idToObjectIndex.contains(obj->idNameIndex)) {
            recordError(obj->locationOfIdProperty, tr("id is not unique"));
            return false;
        }
        obj->id = _idToObjectIndex.count();
        _idToObjectIndex.insert(obj->idNameIndex, objectIndex);
    }

    if (obj->aliasCount() > 0)
        _objectsWithAliases.append(objectIndex);

    // Stop at Component boundary
    if (obj->flags & QV4::CompiledData::Object::IsComponent && objectIndex != /*root object*/0)
        return true;

    for (const QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
        if (binding->type != QV4::CompiledData::Binding::Type_Object
            && binding->type != QV4::CompiledData::Binding::Type_AttachedProperty
            && binding->type != QV4::CompiledData::Binding::Type_GroupProperty)
            continue;

        if (!collectIdsAndAliases(binding->value.objectIndex))
            return false;
    }

    return true;
}

bool QQmlComponentAndAliasResolver::resolveAliases(int componentIndex)
{
    if (_objectsWithAliases.isEmpty())
        return true;

    QQmlPropertyCacheAliasCreator<QQmlTypeCompiler> aliasCacheCreator(&propertyCaches, compiler);

    bool atLeastOneAliasResolved;
    do {
        atLeastOneAliasResolved = false;
        QVector<int> pendingObjects;

        for (int objectIndex: qAsConst(_objectsWithAliases)) {

            QQmlCompileError error;
            const auto result = resolveAliasesInObject(objectIndex, &error);

            if (error.isSet()) {
                recordError(error);
                return false;
            }

            if (result == AllAliasesResolved) {
                QQmlCompileError error = aliasCacheCreator.appendAliasesToPropertyCache(*qmlObjects->at(componentIndex), objectIndex);
                if (error.isSet()) {
                    recordError(error);
                    return false;
                }
                atLeastOneAliasResolved = true;
            } else if (result == SomeAliasesResolved) {
                atLeastOneAliasResolved = true;
                pendingObjects.append(objectIndex);
            } else {
                pendingObjects.append(objectIndex);
            }
        }
        qSwap(_objectsWithAliases, pendingObjects);
    } while (!_objectsWithAliases.isEmpty() && atLeastOneAliasResolved);

    if (!atLeastOneAliasResolved && !_objectsWithAliases.isEmpty()) {
        const QmlIR::Object *obj = qmlObjects->at(_objectsWithAliases.first());
        for (auto alias = obj->aliasesBegin(), end = obj->aliasesEnd(); alias != end; ++alias) {
            if (!(alias->flags & QV4::CompiledData::Alias::Resolved)) {
                recordError(alias->location, tr("Circular alias reference detected"));
                return false;
            }
        }
    }

    return true;
}

QQmlComponentAndAliasResolver::AliasResolutionResult QQmlComponentAndAliasResolver::resolveAliasesInObject(int objectIndex, QQmlCompileError *error)
{
    const QmlIR::Object * const obj = qmlObjects->at(objectIndex);
    if (!obj->aliasCount())
        return AllAliasesResolved;

    int numResolvedAliases = 0;
    bool seenUnresolvedAlias = false;

    for (QmlIR::Alias *alias = obj->firstAlias(); alias; alias = alias->next) {
        if (alias->flags & QV4::CompiledData::Alias::Resolved)
            continue;

        seenUnresolvedAlias = true;

        const int idIndex = alias->idIndex;
        const int targetObjectIndex = _idToObjectIndex.value(idIndex, -1);
        if (targetObjectIndex == -1) {
            *error = QQmlCompileError(alias->referenceLocation, tr("Invalid alias reference. Unable to find id \"%1\"").arg(stringAt(idIndex)));
            break;
        }

        const QmlIR::Object *targetObject = qmlObjects->at(targetObjectIndex);
        Q_ASSERT(targetObject->id >= 0);
        alias->targetObjectId = targetObject->id;
        alias->aliasToLocalAlias = false;

        const QString aliasPropertyValue = stringAt(alias->propertyNameIndex);

        QStringRef property;
        QStringRef subProperty;

        const int propertySeparator = aliasPropertyValue.indexOf(QLatin1Char('.'));
        if (propertySeparator != -1) {
            property = aliasPropertyValue.leftRef(propertySeparator);
            subProperty = aliasPropertyValue.midRef(propertySeparator + 1);
        } else
            property = QStringRef(&aliasPropertyValue, 0, aliasPropertyValue.length());

        QQmlPropertyIndex propIdx;

        if (property.isEmpty()) {
            alias->flags |= QV4::CompiledData::Alias::AliasPointsToPointerObject;
        } else {
            QQmlPropertyCache *targetCache = propertyCaches.at(targetObjectIndex);
            if (!targetCache) {
                *error = QQmlCompileError(alias->referenceLocation, tr("Invalid alias target location: %1").arg(property.toString()));
                break;
            }

            QmlIR::PropertyResolver resolver(targetCache);

            QQmlPropertyData *targetProperty = resolver.property(property.toString());

            // If it's an alias that we haven't resolved yet, try again later.
            if (!targetProperty) {
                bool aliasPointsToOtherAlias = false;
                int localAliasIndex = 0;
                for (auto targetAlias = targetObject->aliasesBegin(), end = targetObject->aliasesEnd(); targetAlias != end; ++targetAlias, ++localAliasIndex) {
                    if (stringAt(targetAlias->nameIndex) == property) {
                        aliasPointsToOtherAlias = true;
                        break;
                    }
                }
                if (aliasPointsToOtherAlias) {
                    if (targetObjectIndex == objectIndex) {
                        alias->localAliasIndex = localAliasIndex;
                        alias->aliasToLocalAlias = true;
                        alias->flags |= QV4::CompiledData::Alias::Resolved;
                        ++numResolvedAliases;
                        continue;
                    }

                    // restore
                    alias->idIndex = idIndex;
                    // Try again later and resolve the target alias first.
                    break;
                }
            }

            if (!targetProperty || targetProperty->coreIndex() > 0x0000FFFF) {
                *error = QQmlCompileError(alias->referenceLocation, tr("Invalid alias target location: %1").arg(property.toString()));
                break;
            }

            propIdx = QQmlPropertyIndex(targetProperty->coreIndex());

            if (!subProperty.isEmpty()) {
                const QMetaObject *valueTypeMetaObject = QQmlValueTypeFactory::metaObjectForMetaType(targetProperty->propType());
                if (!valueTypeMetaObject) {
                    *error = QQmlCompileError(alias->referenceLocation, tr("Invalid alias target location: %1").arg(subProperty.toString()));
                    break;
                }

                int valueTypeIndex =
                        valueTypeMetaObject->indexOfProperty(subProperty.toString().toUtf8().constData());
                if (valueTypeIndex == -1) {
                    *error = QQmlCompileError(alias->referenceLocation, tr("Invalid alias target location: %1").arg(subProperty.toString()));
                    break;
                }
                Q_ASSERT(valueTypeIndex <= 0x0000FFFF);

                propIdx = QQmlPropertyIndex(propIdx.coreIndex(), valueTypeIndex);
            } else {
                if (targetProperty->isQObject())
                    alias->flags |= QV4::CompiledData::Alias::AliasPointsToPointerObject;
            }
        }

        alias->encodedMetaPropertyIndex = propIdx.toEncoded();
        alias->flags |= QV4::CompiledData::Alias::Resolved;
        numResolvedAliases++;
    }

    if (numResolvedAliases == 0)
        return seenUnresolvedAlias ? NoAliasResolved : AllAliasesResolved;

    return SomeAliasesResolved;
}

QQmlDeferredAndCustomParserBindingScanner::QQmlDeferredAndCustomParserBindingScanner(QQmlTypeCompiler *typeCompiler)
    : QQmlCompilePass(typeCompiler)
    , qmlObjects(typeCompiler->qmlObjects())
    , propertyCaches(typeCompiler->propertyCaches())
    , customParsers(typeCompiler->customParserCache())
    , _seenObjectWithId(false)
{
}

bool QQmlDeferredAndCustomParserBindingScanner::scanObject()
{
    return scanObject(/*root object*/0);
}

bool QQmlDeferredAndCustomParserBindingScanner::scanObject(int objectIndex)
{
    QmlIR::Object *obj = qmlObjects->at(objectIndex);
    if (obj->idNameIndex != 0)
        _seenObjectWithId = true;

    if (obj->flags & QV4::CompiledData::Object::IsComponent) {
        Q_ASSERT(obj->bindingCount() == 1);
        const QV4::CompiledData::Binding *componentBinding = obj->firstBinding();
        Q_ASSERT(componentBinding->type == QV4::CompiledData::Binding::Type_Object);
        return scanObject(componentBinding->value.objectIndex);
    }

    QQmlPropertyCache *propertyCache = propertyCaches->at(objectIndex);
    if (!propertyCache)
        return true;

    QString defaultPropertyName;
    QQmlPropertyData *defaultProperty = nullptr;
    if (obj->indexOfDefaultPropertyOrAlias != -1) {
        QQmlPropertyCache *cache = propertyCache->parent();
        defaultPropertyName = cache->defaultPropertyName();
        defaultProperty = cache->defaultProperty();
    } else {
        defaultPropertyName = propertyCache->defaultPropertyName();
        defaultProperty = propertyCache->defaultProperty();
    }

    QQmlCustomParser *customParser = customParsers.value(obj->inheritedTypeNameIndex);

    QmlIR::PropertyResolver propertyResolver(propertyCache);

    QStringList deferredPropertyNames;
    {
        const QMetaObject *mo = propertyCache->firstCppMetaObject();
        const int namesIndex = mo->indexOfClassInfo("DeferredPropertyNames");
        if (namesIndex != -1) {
            QMetaClassInfo classInfo = mo->classInfo(namesIndex);
            deferredPropertyNames = QString::fromUtf8(classInfo.value()).split(QLatin1Char(','));
        }
    }

    for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
        QQmlPropertyData *pd = nullptr;
        QString name = stringAt(binding->propertyNameIndex);

        if (customParser) {
            if (binding->type == QV4::CompiledData::Binding::Type_AttachedProperty) {
                if (customParser->flags() & QQmlCustomParser::AcceptsAttachedProperties) {
                    binding->flags |= QV4::CompiledData::Binding::IsCustomParserBinding;
                    obj->flags |= QV4::CompiledData::Object::HasCustomParserBindings;
                    continue;
                }
            } else if (QmlIR::IRBuilder::isSignalPropertyName(name)
                       && !(customParser->flags() & QQmlCustomParser::AcceptsSignalHandlers)) {
                obj->flags |= QV4::CompiledData::Object::HasCustomParserBindings;
                binding->flags |= QV4::CompiledData::Binding::IsCustomParserBinding;
                continue;
            }
        }

        if (name.isEmpty()) {
            pd = defaultProperty;
            name = defaultPropertyName;
        } else {
            if (name.constData()->isUpper())
                continue;

            bool notInRevision = false;
            pd = propertyResolver.property(name, &notInRevision, QmlIR::PropertyResolver::CheckRevision);
        }

        bool seenSubObjectWithId = false;

        if (binding->type >= QV4::CompiledData::Binding::Type_Object && (pd || binding->isAttachedProperty())) {
            qSwap(_seenObjectWithId, seenSubObjectWithId);
            const bool subObjectValid = scanObject(binding->value.objectIndex);
            qSwap(_seenObjectWithId, seenSubObjectWithId);
            if (!subObjectValid)
                return false;
            _seenObjectWithId |= seenSubObjectWithId;
        }

        if (!seenSubObjectWithId && binding->type != QV4::CompiledData::Binding::Type_GroupProperty
            && !deferredPropertyNames.isEmpty() && deferredPropertyNames.contains(name)) {

            binding->flags |= QV4::CompiledData::Binding::IsDeferredBinding;
            obj->flags |= QV4::CompiledData::Object::HasDeferredBindings;
        }

        if (binding->flags & QV4::CompiledData::Binding::IsSignalHandlerExpression
            || binding->flags & QV4::CompiledData::Binding::IsSignalHandlerObject)
            continue;

        if (!pd) {
            if (customParser) {
                obj->flags |= QV4::CompiledData::Object::HasCustomParserBindings;
                binding->flags |= QV4::CompiledData::Binding::IsCustomParserBinding;
            }
        }
    }

    return true;
}

QQmlJSCodeGenerator::QQmlJSCodeGenerator(QQmlTypeCompiler *typeCompiler, QmlIR::JSCodeGen *v4CodeGen)
    : QQmlCompilePass(typeCompiler)
    , customParsers(typeCompiler->customParserCache())
    , qmlObjects(*typeCompiler->qmlObjects())
    , propertyCaches(typeCompiler->propertyCaches())
    , v4CodeGen(v4CodeGen)
{
}

bool QQmlJSCodeGenerator::generateCodeForComponents()
{
    const QVector<quint32> &componentRoots = compiler->componentRoots();
    for (int i = 0; i < componentRoots.count(); ++i) {
        if (!compileComponent(componentRoots.at(i)))
            return false;
    }

    return compileComponent(/*root object*/0);
}

bool QQmlJSCodeGenerator::compileComponent(int contextObject)
{
    const QmlIR::Object *obj = qmlObjects.at(contextObject);
    if (obj->flags & QV4::CompiledData::Object::IsComponent) {
        Q_ASSERT(obj->bindingCount() == 1);
        const QV4::CompiledData::Binding *componentBinding = obj->firstBinding();
        Q_ASSERT(componentBinding->type == QV4::CompiledData::Binding::Type_Object);
        contextObject = componentBinding->value.objectIndex;
    }

    if (!compileJavaScriptCodeInObjectsRecursively(contextObject, contextObject))
        return false;

    return true;
}

bool QQmlJSCodeGenerator::compileJavaScriptCodeInObjectsRecursively(int objectIndex, int scopeObjectIndex)
{
    QmlIR::Object *object = qmlObjects.at(objectIndex);
    if (object->flags & QV4::CompiledData::Object::IsComponent)
        return true;

    if (object->functionsAndExpressions->count > 0) {
        QList<QmlIR::CompiledFunctionOrExpression> functionsToCompile;
        for (QmlIR::CompiledFunctionOrExpression *foe = object->functionsAndExpressions->first; foe; foe = foe->next)
            functionsToCompile << *foe;
        const QVector<int> runtimeFunctionIndices = v4CodeGen->generateJSCodeForFunctionsAndBindings(functionsToCompile);
        const QList<QQmlError> jsErrors = v4CodeGen->qmlErrors();
        if (!jsErrors.isEmpty()) {
            for (const QQmlError &e : jsErrors)
                compiler->recordError(e);
            return false;
        }

        QQmlJS::MemoryPool *pool = compiler->memoryPool();
        object->runtimeFunctionIndices.allocate(pool, runtimeFunctionIndices);
    }

    for (const QmlIR::Binding *binding = object->firstBinding(); binding; binding = binding->next) {
        if (binding->type < QV4::CompiledData::Binding::Type_Object)
            continue;

        int target = binding->value.objectIndex;
        int scope = binding->type == QV4::CompiledData::Binding::Type_Object ? target : scopeObjectIndex;

        if (!compileJavaScriptCodeInObjectsRecursively(binding->value.objectIndex, scope))
            return false;
    }

    return true;
}

QQmlDefaultPropertyMerger::QQmlDefaultPropertyMerger(QQmlTypeCompiler *typeCompiler)
    : QQmlCompilePass(typeCompiler)
    , qmlObjects(*typeCompiler->qmlObjects())
    , propertyCaches(typeCompiler->propertyCaches())
{

}

void QQmlDefaultPropertyMerger::mergeDefaultProperties()
{
    for (int i = 0; i < qmlObjects.count(); ++i)
        mergeDefaultProperties(i);
}

void QQmlDefaultPropertyMerger::mergeDefaultProperties(int objectIndex)
{
    QQmlPropertyCache *propertyCache = propertyCaches->at(objectIndex);
    if (!propertyCache)
        return;

    QmlIR::Object *object = qmlObjects.at(objectIndex);

    QString defaultProperty = object->indexOfDefaultPropertyOrAlias != -1 ? propertyCache->parent()->defaultPropertyName() : propertyCache->defaultPropertyName();
    QmlIR::Binding *bindingsToReinsert = nullptr;
    QmlIR::Binding *tail = nullptr;

    QmlIR::Binding *previousBinding = nullptr;
    QmlIR::Binding *binding = object->firstBinding();
    while (binding) {
        if (binding->propertyNameIndex == quint32(0) || stringAt(binding->propertyNameIndex) != defaultProperty) {
            previousBinding = binding;
            binding = binding->next;
            continue;
        }

        QmlIR::Binding *toReinsert = binding;
        binding = object->unlinkBinding(previousBinding, binding);

        if (!tail) {
            bindingsToReinsert = toReinsert;
            tail = toReinsert;
        } else {
            tail->next = toReinsert;
            tail = tail->next;
        }
        tail->next = nullptr;
    }

    binding = bindingsToReinsert;
    while (binding) {
        QmlIR::Binding *toReinsert = binding;
        binding = binding->next;
        object->insertSorted(toReinsert);
    }
}

QT_END_NAMESPACE
