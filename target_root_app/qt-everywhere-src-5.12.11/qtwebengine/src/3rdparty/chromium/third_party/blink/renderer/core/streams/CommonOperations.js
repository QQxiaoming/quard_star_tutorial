// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Implementation of functions that are shared between ReadableStream and
// WritableStream.

(function(global, binding, v8) {
  'use strict';

  // Common private symbols. These correspond directly to internal slots in the
  // standard. "[[X]]" in the standard is spelt _X here.
  const _queue = v8.createPrivateSymbol('[[queue]]');
  const _queueTotalSize = v8.createPrivateSymbol('[[queueTotalSize]]');

  // A symbol to protect against double-resolution of promises. This
  // functionality is not explicit in the standard, but is implied in the way
  // the operations are defined.
  const _isSettled = v8.createPrivateSymbol('isSettled');

  // Javascript functions. It is important to use these copies for security and
  // robustness. See "V8 Extras Design Doc", section "Security Considerations".
  // https://docs.google.com/document/d/1AT5-T0aHGp7Lt29vPWFr2-qG8r3l9CByyvKwEuA8Ec0/edit#heading=h.9yixony1a18r
  const Boolean = global.Boolean;
  const Number = global.Number;
  const Number_isFinite = Number.isFinite;
  const Number_isNaN = Number.isNaN;

  const RangeError = global.RangeError;
  const TypeError = global.TypeError;

  const hasOwnProperty = v8.uncurryThis(global.Object.hasOwnProperty);

  function hasOwnPropertyNoThrow(x, property) {
    // The cast of |x| to Boolean will eliminate undefined and null, which would
    // cause hasOwnProperty to throw a TypeError, as well as some other values
    // that can't be objects and so will fail the check anyway.
    return Boolean(x) && hasOwnProperty(x, property);
  }

  //
  // Assert is not normally enabled, to avoid the space and time overhead. To
  // enable, uncomment this definition and then in the file you wish to enable
  // asserts for, uncomment the assert statements and add this definition:
  // const assert = pred => binding.SimpleAssert(pred);
  //
  // binding.SimpleAssert = pred => {
  //   if (pred) {
  //     return;
  //   }
  //   v8.log('\n\n\n  *** ASSERTION FAILURE ***\n\n');
  //   v8.logStackTrace();
  //   v8.log('**************************************************\n\n');
  //   class StreamsAssertionError extends Error {}
  //   throw new StreamsAssertionError('Streams Assertion Failure');
  // };

  //
  // Promise-manipulation functions
  //

  // Not exported.
  function streamInternalError() {
    throw new RangeError('Stream API Internal Error');
  }

  // For safety, this must always be used in place of calling v8.createPromise()
  // directly.
  function createPromise() {
    const p = v8.createPromise();
    p[_isSettled] = false;
    return p;
  }

  // Calling v8.rejectPromise() directly is very dangerous. Always use this
  // wrapper.
  function rejectPromise(p, reason) {
    if (!v8.isPromise(p)) {
      streamInternalError();
    }

    // assert(typeof p[_isSettled] === 'boolean',
    //        'Type(p.[[isSettled]]) is `"boolean"`');

    // Note that this makes the function a no-op for promises that were not
    // created via createPromise(). This is critical for security.
    if (p[_isSettled] !== false) {
      return;
    }
    p[_isSettled] = true;

    v8.rejectPromise(p, reason);
  }

  // This must always be used instead of Promise.reject().
  function createRejectedPromise(reason) {
    const p = createPromise();
    rejectPromise(p, reason);
    return p;
  }

  // Calling v8.resolvePromise() directly is very dangerous. Always use this
  // wrapper. If |value| is an object this will look up Object.prototype.then
  // and so may be re-entrant.
  function resolvePromise(p, value) {
    if (!v8.isPromise(p)) {
      streamInternalError();
    }

    // assert(typeof p[_isSettled] === 'boolean',
    //        'Type(p.[[isSettled]]) is `"boolean"`');

    // Note that this makes the function a no-op for promises that were not
    // created via createPromise(). This is critical for security.
    if (p[_isSettled] !== false) {
      return;
    }
    p[_isSettled] = true;

    v8.resolvePromise(p, value);
  }

  // This must always be used instead of Promise.resolve(). If |value| is an
  // object this will look up Object.prototype.then and so may be re-entrant.
  function createResolvedPromise(value) {
    if (v8.isPromise(value)) {
      // This case applies when an underlying method returns a promise. Promises
      // that are passed through in this way are not used with resolvePromise()
      // or rejectPromise().
      return value;
    }
    const p = createPromise();
    resolvePromise(p, value);
    return p;
  }

  function markPromiseAsHandled(p) {
    if (!v8.isPromise(p)) {
      streamInternalError();
    }
    v8.markPromiseAsHandled(p);
  }

  function promiseState(p) {
    if (!v8.isPromise(p)) {
      streamInternalError();
    }
    return v8.promiseState(p);
  }

  //
  // Queue-with-Sizes Operations
  //
  function DequeueValue(container) {
    // assert(
    //     hasOwnProperty(container, _queue) &&
    //         hasOwnProperty(container, _queueTotalSize),
    //     '_container_ has [[queue]] and [[queueTotalSize]] internal slots.');
    // assert(container[_queue].length !== 0,
    //        '_container_.[[queue]] is not empty.');
    const pair = container[_queue].shift();
    container[_queueTotalSize] -= pair.size;
    if (container[_queueTotalSize] < 0) {
      container[_queueTotalSize] = 0;
    }
    return pair.value;
  }

  function EnqueueValueWithSize(container, value, size) {
    // assert(
    //     hasOwnProperty(container, _queue) &&
    //         hasOwnProperty(container, _queueTotalSize),
    //     '_container_ has [[queue]] and [[queueTotalSize]] internal 'slots.');
    size = Number(size);
    if (!IsFiniteNonNegativeNumber(size)) {
      throw new RangeError(binding.streamErrors.invalidSize);
    }

    container[_queue].push({value, size});
    container[_queueTotalSize] += size;
  }

  function PeekQueueValue(container) {
    // assert(
    //     hasOwnProperty(container, _queue) &&
    //         hasOwnProperty(container, _queueTotalSize),
    //     '_container_ has [[queue]] and [[queueTotalSize]] internal slots.');
    // assert(container[_queue].length !== 0,
    //        '_container_.[[queue]] is not empty.');
    const pair = container[_queue].peek();
    return pair.value;
  }

  function ResetQueue(container) {
    // assert(
    //     hasOwnProperty(container, _queue) &&
    //         hasOwnProperty(container, _queueTotalSize),
    //     '_container_ has [[queue]] and [[queueTotalSize]] internal slots.');
    container[_queue] = new binding.SimpleQueue();
    container[_queueTotalSize] = 0;
  }

  // Not exported.
  function IsFiniteNonNegativeNumber(v) {
    return Number_isFinite(v) && v >= 0;
  }

  function ValidateAndNormalizeHighWaterMark(highWaterMark) {
    highWaterMark = Number(highWaterMark);
    if (Number_isNaN(highWaterMark)) {
      throw new RangeError(binding.streamErrors.invalidHWM);
    }
    if (highWaterMark < 0) {
      throw new RangeError(binding.streamErrors.invalidHWM);
    }
    return highWaterMark;
  }

  // Unlike the version in the standard, this implementation returns the
  // original function as-is if it is set. This means users of the return value
  // need to be careful to explicitly set |this| when calling it.
  function MakeSizeAlgorithmFromSizeFunction(size) {
    if (size === undefined) {
      return () => 1;
    }

    if (typeof size !== 'function') {
      throw new TypeError(binding.streamErrors.sizeNotAFunction);
    }

    return size;
  }

  //
  // Invoking functions.
  // These differ from the Invoke versions in the spec in that they take a fixed
  // number of arguments rather than a list, and also take a name to be used for
  // the function on error.
  //

  // Internal utility functions. Not exported.
  const callFunction = v8.uncurryThis(global.Function.prototype.call);
  const errTmplMustBeFunctionOrUndefined = name =>
      `${name} must be a function or undefined`;
  const Function_bind = v8.uncurryThis(global.Function.prototype.bind);

  function resolveMethod(O, P, nameForError) {
    const method = O[P];

    if (typeof method !== 'function' && typeof method !== 'undefined') {
      throw new TypeError(errTmplMustBeFunctionOrUndefined(nameForError));
    }

    return method;
  }

  function CreateAlgorithmFromUnderlyingMethod(
      underlyingObject, methodName, algoArgCount, methodNameForError) {
    // assert(underlyingObject !== undefined,
    //        'underlyingObject is not undefined.');
    // assert(IsPropertyKey(methodName),
    // '! IsPropertyKey(methodName) is true.');
    // assert(algoArgCount === 0 || algoArgCount === 1,
    // 'algoArgCount is 0 or 1.');
    // assert(
    //     typeof methodNameForError === 'string',
    //     'methodNameForError is a string');
    const method =
        resolveMethod(underlyingObject, methodName, methodNameForError);
    // The implementation uses bound functions rather than lambdas where
    // possible to give the compiler the maximum opportunity to optimise.
    if (method === undefined) {
      return () => createResolvedPromise();
    }

    if (algoArgCount === 0) {
      return Function_bind(PromiseCall0, undefined, method, underlyingObject);
    }

    return Function_bind(PromiseCall1, undefined, method, underlyingObject);
  }

  function CreateAlgorithmFromUnderlyingMethodPassingController(
      underlyingObject, methodName, algoArgCount, controller,
      methodNameForError) {
    // assert(underlyingObject !== undefined,
    //        'underlyingObject is not undefined.');
    // assert(IsPropertyKey(methodName),
    // '! IsPropertyKey(methodName) is true.');
    // assert(algoArgCount === 0 || algoArgCount === 1,
    // 'algoArgCount is 0 or 1.');
    // assert(typeof controller === 'object', 'controller is an object');
    // assert(
    //     typeof methodNameForError === 'string',
    //     'methodNameForError is a string');
    const method =
        resolveMethod(underlyingObject, methodName, methodNameForError);
    if (method === undefined) {
      return () => createResolvedPromise();
    }

    if (algoArgCount === 0) {
      return Function_bind(
          PromiseCall1, undefined, method, underlyingObject, controller);
    }

    return arg => PromiseCall2(method, underlyingObject, arg, controller);
  }

  // Modified from InvokeOrNoop in spec. Takes 1 argument.
  function CallOrNoop1(O, P, arg0, nameForError) {
    const method = resolveMethod(O, P, nameForError);
    if (method === undefined) {
      return undefined;
    }

    return callFunction(method, O, arg0);
  }

  function PromiseCall0(F, V) {
    // assert(typeof F === 'function', 'IsCallable(F) is true.');
    // assert(V !== undefined, 'V is not undefined.');
    try {
      return createResolvedPromise(callFunction(F, V));
    } catch (e) {
      return createRejectedPromise(e);
    }
  }

  function PromiseCall1(F, V, arg0) {
    // assert(typeof F === 'function', 'IsCallable(F) is true.');
    // assert(V !== undefined, 'V is not undefined.');
    try {
      return createResolvedPromise(callFunction(F, V, arg0));
    } catch (e) {
      return createRejectedPromise(e);
    }
  }

  function PromiseCall2(F, V, arg0, arg1) {
    // assert(typeof F === 'function', 'IsCallable(F) is true.');
    // assert(V !== undefined, 'V is not undefined.');
    try {
      return createResolvedPromise(callFunction(F, V, arg0, arg1));
    } catch (e) {
      return createRejectedPromise(e);
    }
  }

  binding.streamOperations = {
    _queue,
    _queueTotalSize,
    createPromise,
    createRejectedPromise,
    createResolvedPromise,
    hasOwnPropertyNoThrow,
    rejectPromise,
    resolvePromise,
    markPromiseAsHandled,
    promiseState,
    CreateAlgorithmFromUnderlyingMethod,
    CreateAlgorithmFromUnderlyingMethodPassingController,
    DequeueValue,
    EnqueueValueWithSize,
    PeekQueueValue,
    ResetQueue,
    ValidateAndNormalizeHighWaterMark,
    MakeSizeAlgorithmFromSizeFunction,
    CallOrNoop1,
    PromiseCall2
  };
});
