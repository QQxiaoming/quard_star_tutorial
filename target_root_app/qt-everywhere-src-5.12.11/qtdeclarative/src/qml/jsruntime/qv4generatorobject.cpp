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

#include <qv4generatorobject_p.h>
#include <qv4symbol_p.h>
#include <qv4iterator_p.h>
#include <qv4jscall_p.h>
#include <qv4vme_moth_p.h>

using namespace QV4;

DEFINE_OBJECT_VTABLE(GeneratorFunctionCtor);
DEFINE_OBJECT_VTABLE(GeneratorFunction);
DEFINE_OBJECT_VTABLE(GeneratorObject);

void Heap::GeneratorFunctionCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("GeneratorFunction"));
}

ReturnedValue GeneratorFunctionCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    ExecutionEngine *engine = f->engine();

    QQmlRefPointer<CompiledData::CompilationUnit> compilationUnit = parse(engine, argv, argc, Type_Generator);
    if (engine->hasException)
        return Encode::undefined();

    Function *vmf = compilationUnit->linkToEngine(engine);
    ExecutionContext *global = engine->scriptContext();
    ReturnedValue o = Encode(GeneratorFunction::create(global, vmf));

    if (!newTarget)
        return o;
    Scope scope(engine);
    ScopedObject obj(scope, o);
    obj->setProtoFromNewTarget(newTarget);
    return obj->asReturnedValue();
}

// 15.3.1: This is equivalent to new Function(...)
ReturnedValue GeneratorFunctionCtor::virtualCall(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    return virtualCallAsConstructor(f, argv, argc, f);
}

Heap::FunctionObject *GeneratorFunction::create(ExecutionContext *context, Function *function)
{
    Scope scope(context);
    Scoped<GeneratorFunction> g(scope, context->engine()->memoryManager->allocate<GeneratorFunction>(context, function));
    ScopedObject proto(scope, scope.engine->newObject());
    proto->setPrototypeOf(scope.engine->generatorPrototype());
    g->defineDefaultProperty(scope.engine->id_prototype(), proto, Attr_NotConfigurable|Attr_NotEnumerable);
    g->setPrototypeOf(ScopedObject(scope, scope.engine->generatorFunctionCtor()->get(scope.engine->id_prototype())));
    return g->d();
}

ReturnedValue GeneratorFunction::virtualCall(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    const GeneratorFunction *gf = static_cast<const GeneratorFunction *>(f);
    Function *function = gf->function();
    ExecutionEngine *engine = gf->engine();

    // We need to set up a separate stack for the generator, as it's being re-entered
    uint stackSize = argc // space for the original arguments
                   + CppStackFrame::requiredJSStackFrameSize(function); // space for the JS stack frame

    size_t requiredMemory = sizeof(GeneratorObject::Data) - sizeof(Value) + sizeof(Value) * stackSize;

    Scope scope(gf);
    Scoped<GeneratorObject> g(scope, scope.engine->memoryManager->allocManaged<GeneratorObject>(requiredMemory, scope.engine->classes[EngineBase::Class_GeneratorObject]));
    g->setPrototypeOf(ScopedObject(scope, gf->get(scope.engine->id_prototype())));

    Heap::GeneratorObject *gp = g->d();
    gp->stack.size = stackSize;
    gp->stack.alloc = stackSize;

    // copy original arguments
    memcpy(gp->stack.values, argv, argc*sizeof(Value));
    gp->cppFrame.init(engine, function, gp->stack.values, argc);
    gp->cppFrame.setupJSFrame(&gp->stack.values[argc], *gf, gf->scope(),
                              thisObject ? *thisObject : Value::undefinedValue(),
                              Value::undefinedValue());

    gp->cppFrame.push();

    Moth::VME::interpret(&gp->cppFrame, engine, function->codeData);
    gp->state = GeneratorState::SuspendedStart;

    gp->cppFrame.pop();
    return g->asReturnedValue();
}


void Heap::GeneratorPrototype::init()
{
    Heap::FunctionObject::init();
}


void GeneratorPrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedValue v(scope);

    Scoped<InternalClass> ic(scope, engine->newInternalClass(
                                            Object::staticVTable(), engine->functionPrototype()));
    ScopedObject ctorProto(scope, engine->newObject(ic->d()));

    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(1));
    ctor->defineReadonlyProperty(engine->id_prototype(), ctorProto);

    ctorProto->defineDefaultProperty(QStringLiteral("constructor"), (v = ctor), Attr_ReadOnly_ButConfigurable);
    ctorProto->defineDefaultProperty(engine->symbol_toStringTag(), (v = engine->newIdentifier(QStringLiteral("GeneratorFunction"))), Attr_ReadOnly_ButConfigurable);
    ctorProto->defineDefaultProperty(engine->id_prototype(), (v = this), Attr_ReadOnly_ButConfigurable);

    setPrototypeOf(engine->iteratorPrototype());
    defineDefaultProperty(QStringLiteral("constructor"), ctorProto, Attr_ReadOnly_ButConfigurable);
    defineDefaultProperty(QStringLiteral("next"), method_next, 1);
    defineDefaultProperty(QStringLiteral("return"), method_return, 1);
    defineDefaultProperty(QStringLiteral("throw"), method_throw, 1);
    defineDefaultProperty(engine->symbol_toStringTag(), (v = engine->newString(QStringLiteral("Generator"))), Attr_ReadOnly_ButConfigurable);
}

ReturnedValue GeneratorPrototype::method_next(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *engine = f->engine();
    const GeneratorObject *g = thisObject->as<GeneratorObject>();
    if (!g || g->d()->state == GeneratorState::Executing)
        return engine->throwTypeError();
    Heap::GeneratorObject *gp = g->d();

    if (gp->state == GeneratorState::Completed)
        return IteratorPrototype::createIterResultObject(engine, Value::undefinedValue(), true);

    return g->resume(engine, argc ? argv[0] : Value::undefinedValue());
}

ReturnedValue GeneratorPrototype::method_return(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *engine = f->engine();
    const GeneratorObject *g = thisObject->as<GeneratorObject>();
    if (!g || g->d()->state == GeneratorState::Executing)
        return engine->throwTypeError();

    Heap::GeneratorObject *gp = g->d();

    if (gp->state == GeneratorState::SuspendedStart)
        gp->state = GeneratorState::Completed;

    if (gp->state == GeneratorState::Completed)
        return IteratorPrototype::createIterResultObject(engine, argc ? argv[0] : Value::undefinedValue(), true);

    // the bytecode interpreter interprets an exception with empty value as
    // a yield called with return()
    engine->throwError(Value::emptyValue());

    return g->resume(engine, argc ? argv[0]: Value::undefinedValue());
}

ReturnedValue GeneratorPrototype::method_throw(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *engine = f->engine();
    const GeneratorObject *g = thisObject->as<GeneratorObject>();
    if (!g || g->d()->state == GeneratorState::Executing)
        return engine->throwTypeError();

    Heap::GeneratorObject *gp = g->d();

    engine->throwError(argc ? argv[0]: Value::undefinedValue());

    if (gp->state == GeneratorState::SuspendedStart || gp->state == GeneratorState::Completed) {
        gp->state = GeneratorState::Completed;
        return Encode::undefined();
    }

    return g->resume(engine, Value::undefinedValue());
}

ReturnedValue GeneratorObject::resume(ExecutionEngine *engine, const Value &arg) const
{
    Heap::GeneratorObject *gp = d();
    gp->state = GeneratorState::Executing;
    gp->cppFrame.parent = engine->currentStackFrame;
    engine->currentStackFrame = &gp->cppFrame;

    Q_ASSERT(gp->cppFrame.yield != nullptr);
    const char *code = gp->cppFrame.yield;
    gp->cppFrame.yield = nullptr;
    gp->cppFrame.jsFrame->accumulator = arg;
    gp->cppFrame.yieldIsIterator = false;

    Scope scope(engine);
    ScopedValue result(scope, Moth::VME::interpret(&gp->cppFrame, engine, code));

    engine->currentStackFrame = gp->cppFrame.parent;

    bool done = (gp->cppFrame.yield == nullptr);
    gp->state = done ? GeneratorState::Completed : GeneratorState::SuspendedYield;
    if (engine->hasException)
        return Encode::undefined();
    if (gp->cppFrame.yieldIsIterator)
        return result->asReturnedValue();
    return IteratorPrototype::createIterResultObject(engine, result, done);
}

DEFINE_OBJECT_VTABLE(MemberGeneratorFunction);

Heap::FunctionObject *MemberGeneratorFunction::create(ExecutionContext *context, Function *function, Object *homeObject, String *name)
{
    Scope scope(context);
    Scoped<MemberGeneratorFunction> g(scope, context->engine()->memoryManager->allocate<MemberGeneratorFunction>(context, function, name));
    g->d()->homeObject.set(scope.engine, homeObject->d());
    ScopedObject proto(scope, scope.engine->newObject());
    proto->setPrototypeOf(scope.engine->generatorPrototype());
    g->defineDefaultProperty(scope.engine->id_prototype(), proto, Attr_NotConfigurable|Attr_NotEnumerable);
    g->setPrototypeOf(ScopedObject(scope, scope.engine->generatorFunctionCtor()->get(scope.engine->id_prototype())));
    return g->d();
}

ReturnedValue MemberGeneratorFunction::virtualCall(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    return GeneratorFunction::virtualCall(f, thisObject, argv, argc);
}
