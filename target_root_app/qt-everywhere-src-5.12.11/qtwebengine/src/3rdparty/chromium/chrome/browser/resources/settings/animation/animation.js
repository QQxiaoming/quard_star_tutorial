// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Simplified API wrapping native Web Animations with some sugar.
 * A compromise between the draft spec and Chrome's evolving support. This API
 * will be changed (or removed) as Chrome support evolves.
 */
cr.define('settings.animation', function() {
  'use strict';

  /** Default timing constants. */
  const Timing = {
    DURATION: 250,
    EASING: 'cubic-bezier(0.4, 0, 0.2, 1)',  // Fast out, slow in.
  };

  /**
   * Offers a small subset of the v1 Animation interface. The underlying
   * animation can be reversed, canceled or immediately finished.
   * @see https://www.w3.org/TR/web-animations-1/#animation
   */
  class Animation extends cr.EventTarget {
    /**
     * @param {!Element} el The element to animate.
     * @param {!Array<!Object>|!Object<!Array>|!Object<string>} keyframes
     *     Keyframes, as in Element.prototype.animate.
     * @param {number|!KeyframeEffectOptions=} opt_options Duration or options
     *     object, as in Element.prototype.animate.
     */
    constructor(el, keyframes, opt_options) {
      super();

      // Disallow direct usage of the underlying animation.
      this.animation_ = el.animate(keyframes, opt_options);

      /** @type {!Promise} */
      this.finished = new Promise((resolve, reject) => {
        // If we were implementing the full spec, we'd have to support
        // removing or resetting these listeners.
        this.animation_.addEventListener('finish', e => {
          resolve();
          // According to the spec, queue a task to fire the event after
          // resolving the promise.
          this.queueDispatch_(e);
        });
        this.animation_.addEventListener('cancel', e => {
          // clang-format off
          reject(new
              /**
               * @see https://heycam.github.io/webidl/#es-DOMException-call
               * @type {function (new:DOMException, string, string)}
               */(
                  DOMException
              )('', 'AbortError'));
          // clang-format on
          this.queueDispatch_(e);
        });
      });
    }

    finish() {
      assert(this.animation_);
      this.animation_.finish();
    }

    cancel() {
      assert(this.animation_);
      this.animation_.cancel();
    }

    /**
     * @param {!Event} e
     * @private
     */
    queueDispatch_(e) {
      setTimeout(() => {
        this.dispatchEvent(e);
        this.animation_ = undefined;
      });
    }
  }

  return {
    Animation: Animation,
    Timing: Timing,
  };
});
