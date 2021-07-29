// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

Polymer({
  is: 'welcome-app',

  /** @private */
  onAccept_: function() {
    chrome.send('handleActivateSignIn');
  },

  /** @private */
  onDecline_: function() {
    chrome.send('handleUserDecline');
  },

  /** @private */
  onLogoTap_: function() {
    this.$$('.logo-icon')
        .animate(
            {
              transform: ['none', 'rotate(-10turn)'],
            },
            /** @type {!KeyframeEffectOptions} */ ({
              duration: 500,
              easing: 'cubic-bezier(1, 0, 0, 1)',
            }));
  },
});
