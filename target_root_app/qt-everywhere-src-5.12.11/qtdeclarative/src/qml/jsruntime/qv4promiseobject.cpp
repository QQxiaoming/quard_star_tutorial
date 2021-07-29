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
#include <QCoreApplication>

#include <private/qv4promiseobject_p.h>
#include <private/qv4symbol_p.h>
#include "qv4jscall_p.h"

using namespace QV4;
using namespace QV4::Promise;

DEFINE_OBJECT_VTABLE(PromiseReaction);
DEFINE_OBJECT_VTABLE(PromiseCtor);
DEFINE_OBJECT_VTABLE(PromiseObject);
DEFINE_OBJECT_VTABLE(PromiseCapability);
DEFINE_OBJECT_VTABLE(PromiseExecutionState);

DEFINE_OBJECT_VTABLE(CapabilitiesExecutorWrapper);
DEFINE_OBJECT_VTABLE(ResolveElementWrapper);
DEFINE_OBJECT_VTABLE(ResolveWrapper);
DEFINE_OBJECT_VTABLE(RejectWrapper);


namespace {

bool isPromise(const Value &object)
{
    return object.as<PromiseObject>() != nullptr;
}

bool isCallable(const Value &object)
{
    return object.as<FunctionObject>() != nullptr;
}

void insertIdLengthTag(Scope& scope, Heap::FunctionObject* function)
{
    ScopedFunctionObject scopedFunction(scope, function);
    scopedFunction->insertMember(scope.engine->id_length(), Primitive::fromInt32(1), Attr_NotWritable|Attr_NotEnumerable);
}

void dropException(QV4::ExecutionEngine* e)
{
    e->hasException = false;
}
}

QT_BEGIN_NAMESPACE
namespace QV4 {
namespace Promise {

const int PROMISE_REACTION_EVENT = QEvent::registerEventType();
const int PROMISE_RESOLVE_THENABLE_EVENT = QEvent::registerEventType();

struct ReactionEvent : public QEvent
{
    ReactionEvent(ExecutionEngine *e, const Value *reaction_, const Value *resolution_)
            : QEvent(QEvent::Type(PROMISE_REACTION_EVENT)),
              reaction{e, *reaction_},
              resolution{e, *resolution_}
    {}

    QV4::PersistentValue reaction;
    QV4::PersistentValue resolution;
};

struct ResolveThenableEvent : public QEvent
{
    ResolveThenableEvent(ExecutionEngine *e, const PromiseObject *promise_, const Object *thenable_, const FunctionObject *then_)
        : QEvent(QEvent::Type(PROMISE_RESOLVE_THENABLE_EVENT)), promise(e, *promise_), thenable(e, *thenable_), then(e, *then_)
    {}

    QV4::PersistentValue promise;
    QV4::PersistentValue thenable;
    QV4::PersistentValue then;
};


} // namespace Promise
} // namespace QV4
QT_END_NAMESPACE

ReactionHandler::ReactionHandler(QObject *parent)
    : QObject(parent)
{}

ReactionHandler::~ReactionHandler()
{}

void ReactionHandler::addReaction(ExecutionEngine *e, const Value *reaction, const Value *value)
{
    QCoreApplication::postEvent(this, new ReactionEvent(e, reaction, value));
}

void ReactionHandler::addResolveThenable(ExecutionEngine *e, const PromiseObject *promise, const Object *thenable, const FunctionObject *then)
{
    QCoreApplication::postEvent(this, new ResolveThenableEvent(e, promise, thenable, then));
}

void ReactionHandler::customEvent(QEvent *event)
{
    if (event)
    {
        const int type = event->type();
        if (type == PROMISE_REACTION_EVENT)
            executeReaction(static_cast<ReactionEvent*>(event));

        if (type == PROMISE_RESOLVE_THENABLE_EVENT)
            executeResolveThenable(static_cast<ResolveThenableEvent*>(event));
    }
}

void ReactionHandler::executeReaction(ReactionEvent *event)
{
    Scope scope(event->reaction.engine());

    Scoped<QV4::PromiseReaction> ro(scope, event->reaction.as<QV4::PromiseReaction>());
    Scoped<QV4::PromiseCapability> capability(scope, ro->d()->capability);

    ScopedValue resolution(scope, event->resolution.value());
    ScopedValue promise(scope, capability->d()->promise);

    if (ro->d()->type == Heap::PromiseReaction::Function) {
        ScopedFunctionObject handler(scope, ro->d()->handler.as<QV4::FunctionObject>());
        ScopedValue result(scope, handler->call(promise, resolution, 1));

        ScopedFunctionObject reaction(scope);
        if (scope.hasException()) {
            reaction = capability->d()->reject.as<QV4::FunctionObject>();
        } else {
            reaction = capability->d()->resolve.as<QV4::FunctionObject>();
        }

        reaction->call(promise, result, 1);
    } else {
        ScopedFunctionObject reaction(scope);
        if (ro->d()->type == Heap::PromiseReaction::Identity) {
            reaction = capability->d()->resolve.as<QV4::FunctionObject>();
        } else {
            reaction = capability->d()->reject.as<QV4::FunctionObject>();
        }

        reaction->call(promise, resolution, 1);
    }
}


namespace {

class FunctionBuilder {
public:
    static Heap::FunctionObject *makeResolveFunction(ExecutionEngine* e, QV4::Heap::PromiseObject *promise) {
        Scope scope(e);
        Scoped<QV4::ResolveWrapper> resolveWrapper(scope, e->memoryManager->allocate<QV4::ResolveWrapper>());

        insertIdLengthTag(scope, resolveWrapper->d());
        resolveWrapper->d()->promise.set(e, promise);

        return resolveWrapper->d();
    }

    static Heap::FunctionObject *makeRejectFunction(ExecutionEngine* e, QV4::Heap::PromiseObject *promise) {
        Scope scope(e);
        Scoped<QV4::RejectWrapper> rejectWrapper(scope, e->memoryManager->allocate<QV4::RejectWrapper>());

        insertIdLengthTag(scope, rejectWrapper->d());
        rejectWrapper->d()->promise.set(e, promise);

        return rejectWrapper->d();
    }

    static Heap::FunctionObject *makeResolveElementFunction(ExecutionEngine* e, uint index, Heap::PromiseExecutionState *executionState)
    {
        Scope scope(e);
        Scoped<QV4::ResolveElementWrapper> resolveElementWrapper(scope, e->memoryManager->allocate<QV4::ResolveElementWrapper>());

        resolveElementWrapper->d()->index = index;
        resolveElementWrapper->d()->alreadyResolved = false;
        resolveElementWrapper->d()->state.set(e, executionState);

        insertIdLengthTag(scope, resolveElementWrapper->d());

        return resolveElementWrapper->d();
    }
};

}


void ReactionHandler::executeResolveThenable(ResolveThenableEvent *event)
{
    Scope scope(event->then.engine());
    JSCallData jsCallData(scope, 2);
    PromiseObject *promise = event->promise.as<PromiseObject>();
    ScopedFunctionObject resolve {scope, FunctionBuilder::makeResolveFunction(scope.engine, promise->d())};
    ScopedFunctionObject reject {scope, FunctionBuilder::makeRejectFunction(scope.engine, promise->d())};
    jsCallData->args[0] = resolve;
    jsCallData.args[1] = reject;
    jsCallData->thisObject = event->thenable.as<QV4::Object>();
    event->then.as<const FunctionObject>()->call(jsCallData);
    if (scope.engine->hasException) {
        JSCallData rejectCallData(scope, 1);
        rejectCallData->args[0] = scope.engine->catchException();
        Scoped<RejectWrapper> reject {scope, scope.engine->memoryManager->allocate<QV4::RejectWrapper>()};
        reject->call(rejectCallData);
    }
}

void Heap::PromiseObject::setState(PromiseObject::State state)
{
    this->state = state;
}

bool Heap::PromiseObject::isSettled() const
{
    return (state != Pending);
}

bool Heap::PromiseObject::isPending() const
{
    return (state == Pending);
}

bool Heap::PromiseObject::isFulfilled() const
{
    return (state == Fulfilled);
}

bool Heap::PromiseObject::isRejected() const
{
    return (state == Rejected);
}

void Heap::PromiseObject::triggerFullfillReactions(ExecutionEngine *e)
{
    Scope scope(e);
    ScopedArrayObject a(scope, fulfillReactions);
    if (a->arrayData()) {
        Scoped<QV4::ArrayData> ad(scope, a->arrayData());
        const uint sz = ad->length();
        ScopedValue value(scope, resolution);
        for (uint i = 0; i < sz; i++) {
            Scoped<QV4::PromiseReaction> r(scope, ad->get(i));
            r->d()->triggerWithValue(scope.engine, value);
        }
    }
}

void Heap::PromiseObject::triggerRejectReactions(ExecutionEngine *e)
{
    Scope scope(e);
    ScopedArrayObject a(scope, rejectReactions);
    if (a->arrayData()) {
        Scoped<QV4::ArrayData> ad(scope, a->arrayData());
        const uint sz = ad->d()->length();
        ScopedValue value(scope, resolution);
        for (uint i = 0; i < sz; i++) {
            Scoped<QV4::PromiseReaction> r(scope, ad->d()->get(i));
            r->d()->triggerWithValue(scope.engine, value);
        }
    }
}

Heap::PromiseReaction *Heap::PromiseReaction::createFulfillReaction(ExecutionEngine* e,
    const QV4::PromiseCapability *capability, const QV4::FunctionObject *onFulfilled)
{
    Scope scope(e);
    Scoped<QV4::PromiseReaction> fulfillReaction(scope, e->memoryManager->allocate<QV4::PromiseReaction>());
    fulfillReaction->d()->capability.set(e, capability->d());

    if (onFulfilled) {
        QV4::ScopedFunctionObject scopedFullfillReaction(scope, onFulfilled);
        if (!scopedFullfillReaction) {
            fulfillReaction->d()->type = PromiseReaction::Identity;
        } else {
            fulfillReaction->d()->type = PromiseReaction::Function;
            fulfillReaction->d()->handler.set(e, scopedFullfillReaction);
        }
    } else {
        fulfillReaction->d()->type = PromiseReaction::Identity;
    }

    return fulfillReaction->d();
}

Heap::PromiseReaction *Heap::PromiseReaction::createRejectReaction(ExecutionEngine* e,
    const QV4::PromiseCapability *capability, const QV4::FunctionObject *onRejected)
{
    Scope scope(e);
    Scoped<QV4::PromiseReaction> rejectReaction(scope, e->memoryManager->allocate<QV4::PromiseReaction>());
    rejectReaction->d()->capability.set(e, capability->d());

    if (onRejected) {
        ScopedFunctionObject scopedRejectReaction(scope, onRejected);
        if (!scopedRejectReaction) {
            rejectReaction->d()->type = PromiseReaction::Thrower;
        } else {
            rejectReaction->d()->type = PromiseReaction::Function;
            rejectReaction->d()->handler.set(e, scopedRejectReaction);
        }
    } else {
        rejectReaction->d()->type = PromiseReaction::Thrower;
    }

    return rejectReaction->d();
}

void Heap::PromiseReaction::triggerWithValue(ExecutionEngine *e, const Value *value)
{
    Scope scope(e);
    auto handler =  e->getPromiseReactionHandler();
    ScopedValue reaction(scope, Value::fromHeapObject(this));
    handler->addReaction(e, reaction, value);
}

void Heap::PromiseCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("Promise"));
}

void Heap::PromiseObject::init(ExecutionEngine *e)
{
    Heap::Object::init();

    {
        Heap::ArrayObject* a = e->newArrayObject();
        fulfillReactions.set(e, a);
    }

    {
        Heap::ArrayObject* a = e->newArrayObject();
        rejectReactions.set(e, a);
    }
}

void Heap::CapabilitiesExecutorWrapper::init()
{
    Heap::FunctionObject::init();
}

void Heap::CapabilitiesExecutorWrapper::destroy()
{
    Heap::FunctionObject::destroy();
}

void Heap::PromiseExecutionState::init()
{
    index = 0;
    remainingElementCount = 0;
}

void Heap::ResolveElementWrapper::init()
{
    index = 0;
    alreadyResolved = false;

    Heap::FunctionObject::init();
}

void Heap::ResolveWrapper::init()
{
    alreadyResolved = false;
    Heap::FunctionObject::init();
}

void Heap::RejectWrapper::init()
{
    alreadyResolved = false;
    Heap::FunctionObject::init();
}

ReturnedValue PromiseCtor::virtualCall(const FunctionObject *f, const Value *, const Value *, int)
{
    // 25.4.3.1 Promise ( executor )
    // 1. If NewTarget is undefined, throw a TypeError exception.
    Scope scope(f);
    THROW_TYPE_ERROR();
}

ReturnedValue PromiseCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    // 25.4.3.1 Promise ( executor )

    Scope scope(f);

    if (argc == 0) // If there are no arguments, argument 1 will be undefined ==> thus not callable ==> type error
        THROW_TYPE_ERROR();

    ScopedFunctionObject executor(scope, argv[0].as<const FunctionObject>());
    if (!executor) //2. If IsCallable(executor) is false
        THROW_TYPE_ERROR(); // throw a TypeError exception

    Scoped<PromiseObject> a(scope, scope.engine->newPromiseObject());
    if (scope.engine->hasException)
        return Encode::undefined();

    a->d()->state = Heap::PromiseObject::Pending;  //4. Set promise.[[PromiseState]] to "pending"
    // 5. Set promise.[[PromiseFulfillReactions]] to a new empty List.
    // 6. Set promise.[[PromiseRejectReactions]] to a new empty List.
    // 7. Set promise.[[PromiseIsHandled]] to false.
    // happens in constructor of a

    ScopedFunctionObject resolve(scope, FunctionBuilder::makeResolveFunction(scope.engine, a->d()));
    ScopedFunctionObject reject(scope, FunctionBuilder::makeRejectFunction(scope.engine, a->d()));

    JSCallData jsCallData(scope, 2);
    jsCallData->args[0] = resolve;
    jsCallData->args[1] = reject;
    //jsCallData->thisObject = a; VERIFY corretness, but this should be undefined (see below)

    executor->call(jsCallData); // 9. Let completion be Call(executor, undefined, « resolvingFunctions.[[Resolve]], resolvingFunctions.[[Reject]] »).

    if (scope.engine->hasException) {
        ScopedValue exception {scope, scope.engine->catchException()};
        JSCallData callData {scope, 1};
        callData.args[0] = exception;
        reject->call(callData);
    }

    if (newTarget)
        a->setProtoFromNewTarget(newTarget);

    return a->asReturnedValue();
}


ReturnedValue PromiseCtor::method_resolve(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    // 25.4.4.5Promise.resolve ( x )
    Scope scope(f);
    ExecutionEngine* e = scope.engine;
    if (!thisObject || !thisObject->isObject()) // 2. If Type(C) is not Object, throw a TypeError exception
        THROW_TYPE_ERROR();

    ScopedValue x(scope);
    if (argc < 1) {
        x = Encode::undefined();
    } else {
        x = argv[0];
    }

    // 3. If IsPromise(x) is true, then
    if (isPromise(x) && x->isObject()) {
        ScopedObject so(scope, thisObject);
        // Let xConstructor be ? Get(x, "constructor").
        ScopedObject constructor(scope, x->objectValue()->get(e->id_constructor()));
        if (so->d() == constructor->d()) // If SameValue(xConstructor, C) is true, return x.
            return x->asReturnedValue();
    }

    // Let promiseCapability be ? NewPromiseCapability(C).
    Scoped<PromiseCapability> capability(scope, e->memoryManager->allocate<QV4::PromiseCapability>());

    ScopedObject newPromise(scope, e->newPromiseObject(thisObject->as<const FunctionObject>(), capability));
    if (!newPromise || !isCallable(capability->d()->resolve) || !isCallable(capability->d()->reject))
        THROW_TYPE_ERROR();

    // Perform ? Call(promiseCapability.[[Resolve]], undefined, « x »).
    ScopedValue undefined(scope, Value::undefinedValue());
    ScopedFunctionObject resolve(scope, capability->d()->resolve);
    resolve->call(undefined, x, 1);

    return newPromise.asReturnedValue();
}

ReturnedValue PromiseCtor::method_reject(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(f);
    ExecutionEngine *e = scope.engine;

    // 2. If Type(C) is not Object, throw a TypeError exception.
    if (!thisObject || !thisObject->isObject())
        THROW_TYPE_ERROR();

    ScopedValue r(scope);
    if (argc < 1) {
        r = Encode::undefined();
    } else {
        r = argv[0];
    }

    // 3. Let promiseCapability be ? NewPromiseCapability(C).
    Scoped<PromiseCapability> capability(scope, e->memoryManager->allocate<QV4::PromiseCapability>());

    ScopedObject newPromise(scope, e->newPromiseObject(thisObject->as<const FunctionObject>(), capability));
    if (!newPromise || !isCallable(capability->d()->resolve) || !isCallable(capability->d()->reject))
        THROW_TYPE_ERROR();

    ScopedValue undefined(scope, Value::undefinedValue());
    ScopedFunctionObject reject(scope, capability->d()->reject.as<const FunctionObject>());
    // Perform ? Call(promiseCapability.[[Reject]], undefined, « r »).
    reject->call(undefined, r, 1);

    return newPromise.asReturnedValue();
}

ReturnedValue PromiseCtor::method_all(const FunctionObject *f, const Value *thisObject, const Value *argv, int)
{
    Scope scope(f);
    ExecutionEngine* e = scope.engine;

    // 2. If Type(C) is not Object, throw a TypeError exception.
    if (!thisObject || !thisObject->isObject())
        THROW_TYPE_ERROR();

    ScopedString resolveName(scope, e->newIdentifier(QStringLiteral("resolve")));
    ScopedString thenName(scope, e->newIdentifier(QStringLiteral("then")));

    Scoped<PromiseCapability> capability(scope, e->memoryManager->allocate<QV4::PromiseCapability>());

    ScopedObject newPromise(scope, e->newPromiseObject(thisObject->as<FunctionObject>(), capability));
    if (!newPromise || !isCallable(capability->d()->resolve) || !isCallable(capability->d()->reject)) {
        if (scope.hasException()) {
            return e->exceptionValue->asReturnedValue();
        } else {
            THROW_TYPE_ERROR();
        }
    }
    capability->d()->promise.set(e, newPromise);

    ScopedFunctionObject reject(scope, capability->d()->reject);

    ScopedObject itemsObject(scope, argv);
    ScopedObject iteratorObject(scope, Runtime::method_getIterator(e, itemsObject, true));
    if (!iteratorObject || scope.hasException()) {
        ScopedObject error(scope);
        if (scope.hasException()) {
            error = e->exceptionValue;
            dropException(e);
        } else {
            error = e->newTypeErrorObject(QStringLiteral("Type error"));
        }
        reject->call(newPromise, error, 1);
        return newPromise.asReturnedValue();
    }

    Scoped<QV4::PromiseExecutionState> executionState(scope, e->memoryManager->allocate<QV4::PromiseExecutionState>());
    executionState->d()->remainingElementCount = 1;
    executionState->d()->capability.set(e, capability);

    Scoped<QV4::ArrayObject> results(scope, e->newArrayObject(0));
    executionState->d()->values.set(e, results);

    ScopedValue doneValue(scope);
    uint index = 0;
    for (;;) {
        Scope scope(e);
        ScopedValue nextValue(scope);
        doneValue = Value::fromReturnedValue(Runtime::method_iteratorNext(e, iteratorObject, nextValue));

        if (doneValue->toBoolean())
            break;

        ScopedObject nextObject(scope);
        if (nextValue->isObject()) {
            nextObject = *nextValue;
        } else if (nextValue->isBoolean()) {
            nextObject = e->newBooleanObject(nextValue->toBoolean());
        } else if (nextValue->isInteger() || nextValue->isDouble()) {
            nextObject = e->newNumberObject(nextValue->toInteger());
        } else if (nextValue->isString()) {
            ScopedString scopedString(scope, nextValue->toString(scope.engine));
            nextObject = e->newStringObject(scopedString);
        }

        ScopedFunctionObject resolve(scope, thisObject->as<Object>()->get(resolveName));
        if (!resolve || scope.hasException()) {
            ScopedValue completion(scope);
            if (!scope.hasException()) {
                completion = e->newTypeErrorObject(QStringLiteral("Type error"));
            } else {
                completion = e->exceptionValue->asReturnedValue();
                dropException(e);
            }

            if (!doneValue->toBoolean())
                completion = Runtime::method_iteratorClose(e, iteratorObject, doneValue);

            reject->call(newPromise, completion, 1);
            return newPromise.asReturnedValue();
        }

        ScopedObject nextPromise(scope, Value::fromReturnedValue(resolve->call(thisObject, nextValue, 1)));
        if (scope.hasException() || !nextPromise) {
            ScopedValue completion(scope, Runtime::method_iteratorClose(e, iteratorObject, doneValue));
            if (scope.hasException()) {
                completion = e->exceptionValue->asReturnedValue();
                dropException(e);
            }
            reject->call(newPromise, completion, 1);
            return newPromise.asReturnedValue();
        }

        executionState->d()->remainingElementCount++;

        ScopedFunctionObject then(scope, nextPromise->get(thenName));
        if (!then || scope.hasException()) {
            ScopedValue completion(scope);
            if (!scope.hasException()) {
                completion = e->newTypeErrorObject(QStringLiteral("Type error"));
            } else {
                completion = e->exceptionValue->asReturnedValue();
                dropException(e);
            }

            if (!doneValue->toBoolean())
                completion = Runtime::method_iteratorClose(scope.engine, iteratorObject, doneValue);

            reject->call(newPromise, completion, 1);
            return newPromise.asReturnedValue();
        }

        ScopedFunctionObject resolveElement(scope, FunctionBuilder::makeResolveElementFunction(e, index, executionState->d()));

        JSCallData jsCallData(scope, 2);
        jsCallData->args[0] = resolveElement;
        jsCallData->args[1] = reject;
        jsCallData->thisObject = nextPromise;

        then->call(jsCallData);
        if (scope.hasException()) {
            ScopedValue completion(scope, e->exceptionValue->asReturnedValue());
            dropException(e);

            if (!doneValue->toBoolean())
                completion = Runtime::method_iteratorClose(scope.engine, iteratorObject, doneValue);

            reject->call(newPromise, completion, 1);
            return newPromise.asReturnedValue();
        }

        index++;
    }

    // empty list
    executionState->d()->remainingElementCount--;
    if (executionState->d()->remainingElementCount == 0) {
        const FunctionObject *resolve = capability->d()->resolve.as<FunctionObject>();
        if (!resolve)
            THROW_TYPE_ERROR();

        ScopedValue values(scope, executionState->d()->values);
        resolve->call(newPromise, values, 1);
        if (scope.hasException()) {
            dropException(e);
            reject->call(newPromise, scope.engine->exceptionValue, 1);
        }
    }

    return newPromise.asReturnedValue();
}

ReturnedValue PromiseCtor::method_race(const FunctionObject *f, const Value *thisObject, const Value *argv, int)
{
    Scope scope(f);
    ExecutionEngine* e = scope.engine;

    if (!thisObject || !thisObject->isObject())
        THROW_TYPE_ERROR();

    ScopedString resolveName(scope, e->newIdentifier(QStringLiteral("resolve")));
    ScopedString thenName(scope, e->newIdentifier(QStringLiteral("then")));

    Scoped<PromiseCapability> capability(scope, scope.engine->memoryManager->allocate<QV4::PromiseCapability>());

    ScopedObject newPromise(scope, e->newPromiseObject(thisObject->as<FunctionObject>(), capability));
    if (!newPromise || !isCallable(capability->d()->resolve) || !isCallable(capability->d()->reject))
        THROW_TYPE_ERROR();
    capability->d()->promise.set(scope.engine, newPromise);

    ScopedFunctionObject reject(scope, capability->d()->reject);

    ScopedObject itemsObject(scope, argv);
    ScopedObject iteratorObject(scope, Runtime::method_getIterator(e, itemsObject, true));
    if (!iteratorObject) {
        ScopedObject error(scope, e->newTypeErrorObject(QStringLiteral("Type error")));
        reject->call(newPromise, error, 1);
        return newPromise.asReturnedValue();
    }

    ScopedValue doneValue(scope);
    for (;;) {
        Scope scope(e);
        ScopedValue nextValue(scope);
        doneValue = Value::fromReturnedValue(Runtime::method_iteratorNext(e, iteratorObject, nextValue));

        if (scope.hasException()) {
            ScopedValue completion(scope, Runtime::method_iteratorClose(e, iteratorObject, doneValue));
            if (scope.hasException()) {
                completion = e->exceptionValue->asReturnedValue();
                dropException(e);
            }
            reject->call(newPromise, completion, 1);
            return newPromise.asReturnedValue();
        }

        if (doneValue->toBoolean())
            break;

        ScopedObject nextObject(scope);
        if (nextValue->isObject()) {
            nextObject = *nextValue;
        } else if (nextValue->isBoolean()) {
            nextObject = scope.engine->newBooleanObject(nextValue->toBoolean());
        } else if (nextValue->isInteger() || nextValue->isDouble()) {
            nextObject = scope.engine->newNumberObject(nextValue->toInteger());
        } else if (nextValue->isString()) {
            ScopedString scopedString(scope, nextValue->toString(scope.engine));
            nextObject = scope.engine->newStringObject(scopedString);
        }

       ScopedFunctionObject resolve(scope, thisObject->as<FunctionObject>()->get(resolveName));
        if (!resolve || scope.hasException()) {
            ScopedValue completion(scope);
            if (!scope.hasException()) {
                completion = e->newTypeErrorObject(QStringLiteral("Type error"));
            } else {
                completion = e->exceptionValue->asReturnedValue();
                dropException(e);
            }

            if (!doneValue->toBoolean())
                completion = Runtime::method_iteratorClose(e, iteratorObject, doneValue);

            reject->call(newPromise, completion, 1);
            return newPromise.asReturnedValue();
        }

        ScopedObject nextPromise(scope, Value::fromReturnedValue(resolve->call(thisObject, nextValue, 1)));
        if (scope.hasException() || !nextPromise) {
            ScopedValue completion(scope, Runtime::method_iteratorClose(e, iteratorObject, doneValue));
            if (scope.hasException()) {
                completion = e->exceptionValue->asReturnedValue();
                dropException(e);
            }
            reject->call(newPromise, completion, 1);
            return newPromise.asReturnedValue();
        }

        ScopedFunctionObject then(scope, nextPromise->get(thenName));
        if (!then || scope.hasException()) {
            ScopedValue completion(scope);
            if (!scope.hasException()) {
                completion = e->newTypeErrorObject(QStringLiteral("Type error"));
            } else {
                completion = e->exceptionValue->asReturnedValue();
                dropException(e);
            }

            if (!doneValue->toBoolean())
                completion = Runtime::method_iteratorClose(e, iteratorObject, doneValue);

            reject->call(newPromise, completion, 1);
            return newPromise.asReturnedValue();
        }

        ScopedFunctionObject resolveOriginalPromise(scope, capability->d()->resolve);

        JSCallData jsCallData(scope, 2);
        jsCallData->args[0] = resolveOriginalPromise;
        jsCallData->args[1] = reject;
        jsCallData->thisObject = nextPromise;

        then->call(jsCallData);
        if (scope.hasException()) {
            ScopedValue completion(scope, e->exceptionValue->asReturnedValue());
            dropException(e);

            if (!doneValue->toBoolean())
                completion = Runtime::method_iteratorClose(e, iteratorObject, doneValue);

            reject->call(newPromise, completion, 1);
            return newPromise.asReturnedValue();
        }
    }

    return newPromise.asReturnedValue();
}

void PromisePrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);

    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Primitive::fromInt32(1));
    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));

    ctor->defineDefaultProperty(QStringLiteral("resolve"), PromiseCtor::method_resolve, 1);
    ctor->defineDefaultProperty(QStringLiteral("reject"), PromiseCtor::method_reject, 1);
    ctor->defineDefaultProperty(QStringLiteral("all"), PromiseCtor::method_all, 1);
    ctor->defineDefaultProperty(QStringLiteral("race"), PromiseCtor::method_race, 1);
    ctor->addSymbolSpecies();

    defineDefaultProperty(engine->id_constructor(), (o = ctor));

    ScopedString val(scope, engine->newString(QLatin1String("Promise")));
    defineReadonlyConfigurableProperty(engine->symbol_toStringTag(), val);

    defineDefaultProperty(QStringLiteral("then"), method_then, 2);
    defineDefaultProperty(QStringLiteral("catch"), method_catch, 1);
}

ReturnedValue PromisePrototype::method_then(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    // 25.4.5.3 Promise.prototype.then
    Scope scope(f);
    ExecutionEngine* e = scope.engine;

    Scoped<QV4::PromiseObject> promise(scope, thisObject);
    if (!promise)
        THROW_TYPE_ERROR();

    ScopedFunctionObject onFulfilled(scope);
    if (argc >= 1) {
        onFulfilled = argv[0];
    } else {
        onFulfilled = Encode::undefined();
    }

    ScopedFunctionObject onRejected(scope);
    if (argc >= 2) {
        onRejected = argv[1];
    } else {
        onRejected = Encode::undefined();
    }

    Scoped<PromiseCapability> capability(scope, e->memoryManager->allocate<PromiseCapability>());

    ScopedFunctionObject constructor(scope, promise->get(e->id_constructor()));
    if (!constructor || scope.hasException())
        THROW_TYPE_ERROR();

    // 4. Let resultCapability be ? NewPromiseCapability(C).
    ScopedObject nextPromise(scope, e->newPromiseObject(constructor, capability));
    capability->d()->promise.set(scope.engine, nextPromise);

    Scoped<PromiseReaction> fulfillReaction(scope, Heap::PromiseReaction::createFulfillReaction(scope.engine, capability, onFulfilled));
    Scoped<PromiseReaction> rejectReaction(scope, Heap::PromiseReaction::createRejectReaction(scope.engine, capability, onRejected));

    ScopedValue resolution(scope, promise->d()->resolution);
    if (promise->d()->isPending()) { // 7. If promise.[[PromiseState]] is "pending"
        {
            // Append fulfillReaction as the last element of the List that is promise.[[PromiseFulfillReactions]].
            ScopedArrayObject a(scope, promise->d()->fulfillReactions);
            ScopedValue newValue(scope, fulfillReaction->d());
            a->push_back(newValue);
        }

        {
            // Append rejectReaction as the last element of the List that is promise.[[PromiseRejectReactions]].
            ScopedArrayObject a(scope, promise->d()->rejectReactions);
            ScopedValue newValue(scope, rejectReaction->d());
            a->push_back(newValue);
        }
    } else if (promise->d()->isFulfilled()) { // 8. Else if promise.[[PromiseState]] is "fulfilled", then
        // Perform EnqueueJob("PromiseJobs", PromiseReactionJob, « fulfillReaction, value »).
        fulfillReaction->as<QV4::PromiseReaction>()->d()->triggerWithValue(e, resolution); // Perform EnqueueJob("PromiseJobs", PromiseReactionJob, « fulfillReaction, value »).
    } else if (promise->d()->isRejected()) { // 9. Else
        // Perform EnqueueJob("PromiseJobs", PromiseReactionJob, « rejectReaction, reason »).
        rejectReaction->as<QV4::PromiseReaction>()->d()->triggerWithValue(e, resolution);
    } else {
        Q_ASSERT(false);
        THROW_GENERIC_ERROR("Should never be thrown. Unknown promise state");
    }

    return nextPromise->asReturnedValue();
}

ReturnedValue PromisePrototype::method_catch(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(f);
    Scoped<Object> promise(scope);
    if (thisObject->isObject()) {
        promise.setPointer(thisObject->as<Object>());
    } else if (thisObject->isBoolean()) {
        promise = scope.engine->newBooleanObject(thisObject->toBoolean());
    } else if (thisObject->isInteger() || thisObject->isDouble()) {
        promise = scope.engine->newNumberObject(thisObject->toInteger());
    } else if (thisObject->isString()) {
        ScopedString scopedString(scope, thisObject->toString(scope.engine));
        promise = scope.engine->newStringObject(scopedString);
    } else {
        THROW_TYPE_ERROR();
    }

    ScopedValue onRejected(scope);
    if (argc < 1) {
        onRejected = Encode::undefined();
    } else {
        onRejected = argv[0];
    }

    JSCallData jsCallData(scope, 2);
    jsCallData->args[0] = Encode::undefined();
    jsCallData->args[1] = onRejected;
    jsCallData->thisObject = promise;

    ScopedString thenName(scope, scope.engine->newIdentifier(QStringLiteral("then")));
    ScopedFunctionObject then(scope, promise->get(thenName));
    if (!then || scope.hasException())
        THROW_TYPE_ERROR();

    return then->call(jsCallData);
}

ReturnedValue CapabilitiesExecutorWrapper::virtualCall(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    Q_UNUSED(thisObject);

    Scope scope(f);
    const CapabilitiesExecutorWrapper* self = static_cast<const CapabilitiesExecutorWrapper*>(f);
    Heap::PromiseCapability *capabilities = self->d()->capabilities;

    if (!capabilities->resolve.isUndefined() || !capabilities->reject.isUndefined())
        THROW_TYPE_ERROR();

    if (argc >= 1 && !argv[0].isUndefined())
        capabilities->resolve.set(scope.engine, argv[0]);

    if (argc >= 2 && !argv[1].isUndefined())
        capabilities->reject.set(scope.engine, argv[1]);

    return Encode::undefined();
}

ReturnedValue ResolveElementWrapper::virtualCall(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    Q_UNUSED(thisObject);

    Scope scope(f);
    const ResolveElementWrapper* self = static_cast<const ResolveElementWrapper*>(f);

    if (self->d()->alreadyResolved)
        return Encode::undefined();

    ScopedValue value(scope);
    if (argc == 1) {
        value = argv[0];
    } else {
        value = Encode::undefined();
    }

    Scoped<PromiseExecutionState> so(scope, self->d()->state);
    self->d()->alreadyResolved = true;

    ScopedObject values(scope, so->d()->values);
    values->arraySet(self->d()->index, value);

    so->d()->remainingElementCount--;
    if (so->d()->remainingElementCount == 0) {
        Scoped<PromiseCapability> capability(scope, so->d()->capability);
        ScopedValue promise(scope, capability->d()->promise);
        ScopedFunctionObject resolve(scope, capability->d()->resolve.as<QV4::FunctionObject>());
        resolve->call(promise, values, 1);
    }

    return Encode::undefined();
}

ReturnedValue ResolveWrapper::virtualCall(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    // 25.4.1.3.2 (ecmase-262/8.0)

    Q_UNUSED(thisObject);

    Scope scope(f);
    const ResolveWrapper *self = static_cast<const ResolveWrapper*>(f);

    Scoped<PromiseObject> promise(scope, self->d()->promise);
    // 4. If alreadyRseolved.[[Value]] is true, return undefined
    if (self->d()->alreadyResolved || !promise->d()->isPending()) // Why check for pending?
        return Encode::undefined();

    // 5. Set alreadyResolved.[[Value]] to true
    self->d()->alreadyResolved = true;

    ScopedValue resolution(scope);
    if (argc == 1) {
        resolution = argv[0];
    } else {
        resolution = Encode::undefined();
    }

    if (!resolution->isObject()) { // 7 If Type(resolution) is not Object
        // then Return FullFillPromise(promise, resolution)
        // (FullFillPromise will return undefined, so we share the return with the other path which also returns undefined
        promise->d()->setState(Heap::PromiseObject::Fulfilled);
        promise->d()->resolution.set(scope.engine, resolution);
        promise->d()->triggerFullfillReactions(scope.engine);
    } else {
        //PromiseObject *promise = resolution->as<PromiseObject>();
        auto resolutionObject = resolution->as<Object>();
        ScopedString thenName(scope, scope.engine->newIdentifier(QStringLiteral("then")));

        // 8. Let then be Get(resolution, then)
        ScopedFunctionObject thenAction { scope, resolutionObject->get(thenName)};
        // 9. If then is an abrupt completion, then
        if (scope.engine->hasException) {
            // Return RecjectPromise(promise, then.[[Value]]
            ScopedValue thenValue {scope, scope.engine->catchException()};
            promise->d()->setState(Heap::PromiseObject::Rejected);
            promise->d()->resolution.set(scope.engine, thenValue);
            promise->d()->triggerRejectReactions(scope.engine);
        } else {
            // 10. Let thenAction be then.[[Value]]
            if (!thenAction) { // 11. If IsCallable(thenAction) is false
                promise->d()->setState(Heap::PromiseObject::Fulfilled);
                promise->d()->resolution.set(scope.engine, resolution);
                promise->d()->triggerFullfillReactions(scope.engine);
            } else {
                // 12. Perform EnqueueJob("PromiseJobs", PromiseResolveThenableJob, « promise, resolution, thenAction »).
                scope.engine->getPromiseReactionHandler()->addResolveThenable(scope.engine, promise.getPointer(), resolutionObject, thenAction);
            }
        }
    }

    return Encode::undefined();
}

ReturnedValue RejectWrapper::virtualCall(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    Q_UNUSED(thisObject);

    Scope scope(f);
    const RejectWrapper *self = static_cast<const RejectWrapper*>(f);

    Scoped<PromiseObject> promise(scope, self->d()->promise);
    if (self->d()->alreadyResolved || !promise->d()->isPending())
        return Encode::undefined();

    ScopedValue value(scope);
    if (argc == 1) {
        value = argv[0];
    } else {
        value = Encode::undefined();
    }

    if (!isPromise(value)) {
        self->d()->alreadyResolved = true;
        promise->d()->setState(Heap::PromiseObject::Rejected);
        promise->d()->resolution.set(scope.engine, value);

        promise->d()->triggerRejectReactions(scope.engine);

    } else {
        PromiseObject *promise = value->as<PromiseObject>();
        ScopedString thenName(scope, scope.engine->newIdentifier(QStringLiteral("catch")));

        ScopedFunctionObject then(scope, promise->get(thenName));
        JSCallData jsCallData(scope, 2);
        jsCallData->args[0] = *f;
        jsCallData->args[1] = Encode::undefined();
        jsCallData->thisObject = value;

        then->call(jsCallData);
    }

    return Encode::undefined();
}
