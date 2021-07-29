// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview A helper object used by the sync confirmation dialog to
 * interact with the browser.
 */

cr.define('sync.confirmation', function() {

  /**
   * ConsentBumpAction enum.
   * These values are persisted to logs and should not be renumbered or re-used.
   * See tools/metrics/histograms/enums.xml.
   * @enum {number}
   */
  const ConsentBumpAction = {
    DEFAULT_OPT_IN: 0,
    MORE_OPTIONS_OPT_IN: 1,
    MORE_OPTIONS_REVIEW_SETTINGS: 2,
    MORE_OPTIONS_MAKE_NO_CHANGES: 3,
    ABORT: 4,  // Not actually used in JS, but here to match the C++ type.
  };

  /**
   * The number of enum values in ConsentBumpAction. This should
   * be kept in sync with the enum count in tools/metrics/histograms/enums.xml.
   * @type {number}
   */
  const CONSENT_BUMP_ACTION_COUNT = Object.keys(ConsentBumpAction).length;

  /**
   * The metrics name corresponding to ConsentBumpAction.
   */
  const CONSENT_BUMP_ACTION_METRIC_NAME = 'UnifiedConsent.ConsentBump.Action';

  /** @interface */
  class SyncConfirmationBrowserProxy {
    /**
     * Called when the user confirms the Sync Confirmation dialog.
     * @param {!Array<string>} description Strings that the user was presented
     *     with in the UI.
     * @param {string} confirmation Text of the element that the user
     *     clicked on.
     * @param {boolean} isConsentBump Boolean indicating whether the
     *     confirmation dialog was a consent bump.
     * @param {boolean} moreOptionsPage Boolean indicating whether the user
     *     selected confirm from the moreOptions page in the consent bump.
     */
    confirm(description, confirmation, isConsentBump, moreOptionsPage) {}

    /** Called when the user undoes the Sync confirmation.
     * @param {boolean} isConsentBump Boolean indicating whether the
     *     confirmation dialog was a consent bump.
     */
    undo(isConsentBump) {}

    /**
     * Called when the user clicks on the Settings link in
     *     the Sync Confirmation dialog.
     * @param {!Array<string>} description Strings that the user was presented
     *     with in the UI.
     * @param {string} confirmation Text of the element that the user
     *     clicked on.
     * @param {boolean} isConsentBump Boolean indicating whether the
     *     confirmation dialog was a consent bump.
     */
    goToSettings(description, confirmation, isConsentBump) {}

    /** @param {!Array<number>} height */
    initializedWithSize(height) {}
  }

  /** @implements {sync.confirmation.SyncConfirmationBrowserProxy} */
  class SyncConfirmationBrowserProxyImpl {
    /** @override */
    confirm(description, confirmation, isConsentBump, moreOptionsPage) {
      if (isConsentBump) {
        this.recordConsentBumpAction_(
            moreOptionsPage ? ConsentBumpAction.MORE_OPTIONS_OPT_IN :
                              ConsentBumpAction.DEFAULT_OPT_IN);
      }
      chrome.send('confirm', [description, confirmation]);
    }

    /** @override */
    undo(isConsentBump) {
      if (isConsentBump) {
        this.recordConsentBumpAction_(
            ConsentBumpAction.MORE_OPTIONS_MAKE_NO_CHANGES);
      }
      chrome.send('undo');
    }

    /** @override */
    goToSettings(description, confirmation, isConsentBump) {
      if (isConsentBump) {
        this.recordConsentBumpAction_(
            ConsentBumpAction.MORE_OPTIONS_REVIEW_SETTINGS);
      }
      chrome.send('goToSettings', [description, confirmation]);
    }

    /** @override */
    initializedWithSize(height) {
      chrome.send('initializedWithSize', height);
    }

    /** @private */
    recordConsentBumpAction_(action) {
      chrome.metricsPrivate.recordEnumerationValue(
          CONSENT_BUMP_ACTION_METRIC_NAME, action, CONSENT_BUMP_ACTION_COUNT);
    }
  }

  cr.addSingletonGetter(SyncConfirmationBrowserProxyImpl);

  return {
    SyncConfirmationBrowserProxy: SyncConfirmationBrowserProxy,
    SyncConfirmationBrowserProxyImpl: SyncConfirmationBrowserProxyImpl,
  };
});
