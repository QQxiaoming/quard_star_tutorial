// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview A helper object used from the "Google accounts" subsection of
 * the "People" section of Settings, to interact with the browser. Chrome OS
 * only.
 */
cr.exportPath('settings');

/**
 * Information for an account managed by Chrome OS AccountManager.
 * @typedef {{
 *   fullName: string,
 *   email: string,
 *   pic: string,
 * }}
 */
settings.Account;

cr.define('settings', function() {
  /** @interface */
  class AccountManagerBrowserProxy {
    /**
     * Returns a Promise for the list of GAIA accounts held in AccountManager.
     * @return {!Promise<!Array<settings.Account>>}
     */
    getAccounts() {}

    /**
     * Triggers the 'Add account' flow.
     */
    addAccount() {}
  }

  /**
   * @implements {settings.AccountManagerBrowserProxy}
   */
  class AccountManagerBrowserProxyImpl {
    /** @override */
    getAccounts() {
      return cr.sendWithPromise('getAccounts');
    }

    /** @override */
    addAccount() {
      chrome.send('addAccount');
    }
  }

  cr.addSingletonGetter(AccountManagerBrowserProxyImpl);

  return {
    AccountManagerBrowserProxy: AccountManagerBrowserProxy,
    AccountManagerBrowserProxyImpl: AccountManagerBrowserProxyImpl,
  };
});
