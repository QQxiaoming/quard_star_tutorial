// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * 'settings-account-manager' is the settings subpage containing controls to
 * list, add and delete Secondary Google Accounts.
 */

Polymer({
  is: 'settings-account-manager',

  behaviors: [
    I18nBehavior,
    WebUIListenerBehavior,
  ],

  properties: {
    /**
     * List of Accounts.
     * @type {!Array<settings.Account>}
     */
    accounts_: {
      type: Array,
      value: function() {
        return [];
      },
    },
  },

  /** @private {?settings.AccountManagerBrowserProxy} */
  browserProxy_: null,

  /** @override */
  attached: function() {
    this.addWebUIListener('accounts-changed', this.refreshAccounts_.bind(this));
  },

  /** @override */
  ready: function() {
    this.browserProxy_ = settings.AccountManagerBrowserProxyImpl.getInstance();
    this.refreshAccounts_();
  },

  /**
   * @param {string} iconUrl
   * @return {string} A CSS image-set for multiple scale factors.
   * @private
   */
  getIconImageSet_: function(iconUrl) {
    return cr.icon.getImage(iconUrl);
  },

  /**
   * @param {!Event} event
   * @private
   */
  addAccount_: function(event) {
    this.browserProxy_.addAccount();
  },

  /**
   * @private
   */
  refreshAccounts_: function() {
    this.browserProxy_.getAccounts().then(accounts => {
      this.set('accounts_', accounts);
    });
  },
});
