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


#include "qv4module_p.h"

#include <private/qv4mm_p.h>
#include <private/qv4vme_moth_p.h>
#include <private/qv4context_p.h>
#include <private/qv4symbol_p.h>
#include <private/qv4identifiertable_p.h>

#include <QScopeGuard>

using namespace QV4;

DEFINE_OBJECT_VTABLE(Module);

void Heap::Module::init(ExecutionEngine *engine, CompiledData::CompilationUnit *moduleUnit)
{
    Object::init();

    // This is a back pointer and there is no need to call addref() on the unit, because the unit
    // owns this object instead.
    unit = moduleUnit;
    self.set(engine, this);

    Function *moduleFunction = unit->runtimeFunctions[unit->unitData()->indexOfRootFunction];

    const uint locals = moduleFunction->compiledFunction->nLocals;
    const size_t requiredMemory = sizeof(QV4::CallContext::Data) - sizeof(Value) + sizeof(Value) * locals;
    scope.set(engine, engine->memoryManager->allocManaged<QV4::CallContext>(requiredMemory, moduleFunction->internalClass));
    scope->init();
    scope->outer.set(engine, engine->rootContext()->d());
    scope->locals.size = locals;
    scope->locals.alloc = locals;
    scope->nArgs = 0;

    // Prepare the temporal dead zone
    scope->setupLocalTemporalDeadZone(moduleFunction->compiledFunction);

    Scope valueScope(engine);

    // It's possible for example to re-export an import, for example:
    //     import * as foo from "./bar.js"
    //     export { foo }
    // Since we don't add imports to the locals, it won't be found typically.
    // Except now we add imports at the end of the internal class in the index
    // space past the locals, so that resolveExport can find it.
    {
        Scoped<QV4::InternalClass> ic(valueScope, scope->internalClass);

        for (uint i = 0; i < unit->data->importEntryTableSize; ++i) {
            const CompiledData::ImportEntry &import = unit->data->importEntryTable()[i];
            ic = ic->addMember(engine->identifierTable->asPropertyKey(unit->runtimeStrings[import.localName]), Attr_NotConfigurable);
        }
        scope->internalClass.set(engine, ic->d());
    }


    Scoped<QV4::Module> This(valueScope, this);
    ScopedString name(valueScope, engine->newString(QStringLiteral("Module")));
    This->insertMember(engine->symbol_toStringTag(), name, Attr_ReadOnly);
    This->setPrototypeUnchecked(nullptr);
}

void Module::evaluate()
{
    if (d()->evaluated)
        return;
    d()->evaluated = true;

    CompiledData::CompilationUnit *unit = d()->unit;

    unit->evaluateModuleRequests();

    ExecutionEngine *v4 = engine();
    Function *moduleFunction = unit->runtimeFunctions[unit->data->indexOfRootFunction];
    CppStackFrame frame;
    frame.init(v4, moduleFunction, nullptr, 0);
    frame.setupJSFrame(v4->jsStackTop, Value::undefinedValue(), d()->scope,
                       Value::undefinedValue(), Value::undefinedValue());

    frame.push();
    v4->jsStackTop += frame.requiredJSStackFrameSize();
    auto frameCleanup = qScopeGuard([&frame]() {
        frame.pop();
    });
    Moth::VME::exec(&frame, v4);
}

const Value *Module::resolveExport(PropertyKey id) const
{
    if (d()->unit->isESModule()) {
        if (!id.isString())
            return nullptr;
        Scope scope(engine());
        ScopedString name(scope, id.asStringOrSymbol());
        return d()->unit->resolveExport(name);
    } else {
        InternalClassEntry entry = d()->scope->internalClass->find(id);
        if (entry.isValid())
            return &d()->scope->locals[entry.index];
        return nullptr;
    }
}

ReturnedValue Module::virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty)
{
    if (id.isSymbol())
        return Object::virtualGet(m, id, receiver, hasProperty);

    const Module *module = static_cast<const Module *>(m);
    const Value *v = module->resolveExport(id);
    if (hasProperty)
        *hasProperty = v != nullptr;
    if (!v)
        return Encode::undefined();
    if (v->isEmpty()) {
        Scope scope(m->engine());
        ScopedValue propName(scope, id.toStringOrSymbol(scope.engine));
        return scope.engine->throwReferenceError(propName);
    }
    return v->asReturnedValue();
}

PropertyAttributes Module::virtualGetOwnProperty(const Managed *m, PropertyKey id, Property *p)
{
    if (id.isSymbol())
        return Object::virtualGetOwnProperty(m, id, p);

    const Module *module = static_cast<const Module *>(m);
    const Value *v = module->resolveExport(id);
    if (!v) {
        if (p)
            p->value = Encode::undefined();
        return Attr_Invalid;
    }
    if (p)
        p->value = v->isEmpty() ? Encode::undefined() : v->asReturnedValue();
    if (v->isEmpty()) {
        Scope scope(m->engine());
        ScopedValue propName(scope, id.toStringOrSymbol(scope.engine));
        scope.engine->throwReferenceError(propName);
    }
    return Attr_Data | Attr_NotConfigurable;
}

bool Module::virtualHasProperty(const Managed *m, PropertyKey id)
{
    if (id.isSymbol())
        return Object::virtualHasProperty(m, id);

    const Module *module = static_cast<const Module *>(m);
    const Value *v = module->resolveExport(id);
    return v != nullptr;
}

bool Module::virtualPreventExtensions(Managed *)
{
    return true;
}

bool Module::virtualDefineOwnProperty(Managed *, PropertyKey, const Property *, PropertyAttributes)
{
    return false;
}

bool Module::virtualPut(Managed *, PropertyKey, const Value &, Value *)
{
    return false;
}

bool Module::virtualDeleteProperty(Managed *m, PropertyKey id)
{
    if (id.isSymbol())
        return Object::virtualDeleteProperty(m, id);
    const Module *module = static_cast<const Module *>(m);
    const Value *v = module->resolveExport(id);
    if (v)
        return false;
    return true;
}

struct ModuleNamespaceIterator : ObjectOwnPropertyKeyIterator
{
    QStringList exportedNames;
    int exportIndex = 0;
    ModuleNamespaceIterator(const QStringList &names) : exportedNames(names) {}
    ~ModuleNamespaceIterator() override = default;
    PropertyKey next(const Object *o, Property *pd = nullptr, PropertyAttributes *attrs = nullptr) override;

};

PropertyKey ModuleNamespaceIterator::next(const Object *o, Property *pd, PropertyAttributes *attrs)
{
    const Module *module = static_cast<const Module *>(o);
    if (exportIndex < exportedNames.count()) {
        if (attrs)
            *attrs = Attr_Data;
        Scope scope(module->engine());
        ScopedString exportName(scope, scope.engine->newString(exportedNames.at(exportIndex)));
        exportIndex++;
        const Value *v = module->resolveExport(exportName->toPropertyKey());
        if (pd) {
            if (v->isEmpty())
                scope.engine->throwReferenceError(exportName);
            else
                pd->value = *v;
        }
        return exportName->toPropertyKey();
    }
    return ObjectOwnPropertyKeyIterator::next(o, pd, attrs);
}

OwnPropertyKeyIterator *Module::virtualOwnPropertyKeys(const Object *o, Value *target)
{
    const Module *module = static_cast<const Module *>(o);
    *target = *o;

    QStringList names;
    if (module->d()->unit->isESModule()) {
        names = module->d()->unit->exportedNames();
    } else {
        Heap::InternalClass *scopeClass = module->d()->scope->internalClass;
        for (uint i = 0; i < scopeClass->size; ++i)
            names << scopeClass->keyAt(i);
    }

    return new ModuleNamespaceIterator(names);
}

Heap::Object *Module::virtualGetPrototypeOf(const Managed *)
{
    return nullptr;
}

bool Module::virtualSetPrototypeOf(Managed *, const Object *proto)
{
    return proto == nullptr;
}

bool Module::virtualIsExtensible(const Managed *)
{
    return false;
}
