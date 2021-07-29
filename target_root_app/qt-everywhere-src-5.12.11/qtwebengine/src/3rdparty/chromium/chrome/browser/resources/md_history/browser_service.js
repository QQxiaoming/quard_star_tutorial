// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Defines a singleton object, md_history.BrowserService, which
 * provides access to chrome.send APIs.
 */

cr.define('md_history', function() {
  class BrowserService {
    constructor() {
      /** @private {Array<!HistoryEntry>} */
      this.pendingDeleteItems_ = null;
      /** @private {PromiseResolver} */
      this.pendingDeletePromise_ = null;
    }

    /**
     * @param {!Array<!HistoryEntry>} items
     * @return {Promise<!Array<!HistoryEntry>>}
     */
    deleteItems(items) {
      if (this.pendingDeleteItems_ != null) {
        // There's already a deletion in progress, reject immediately.
        return new Promise(function(resolve, reject) {
          reject(items);
        });
      }

      const removalList = items.map(function(item) {
        return {
          url: item.url,
          timestamps: item.allTimestamps,
        };
      });

      this.pendingDeleteItems_ = items;
      this.pendingDeletePromise_ = new PromiseResolver();

      chrome.send('removeVisits', removalList);

      return this.pendingDeletePromise_.promise;
    }

    /**
     * @param {!string} url
     */
    removeBookmark(url) {
      chrome.send('removeBookmark', [url]);
    }

    /**
     * @param {string} sessionTag
     */
    openForeignSessionAllTabs(sessionTag) {
      chrome.send('openForeignSession', [sessionTag]);
    }

    /**
     * @param {string} sessionTag
     * @param {number} windowId
     * @param {number} tabId
     * @param {MouseEvent} e
     */
    openForeignSessionTab(sessionTag, windowId, tabId, e) {
      chrome.send('openForeignSession', [
        sessionTag, String(windowId), String(tabId), e.button || 0, e.altKey,
        e.ctrlKey, e.metaKey, e.shiftKey
      ]);
    }

    /**
     * @param {string} sessionTag
     */
    deleteForeignSession(sessionTag) {
      chrome.send('deleteForeignSession', [sessionTag]);
    }

    openClearBrowsingData() {
      chrome.send('clearBrowsingData');
    }

    /**
     * @param {string} histogram
     * @param {number} value
     * @param {number} max
     */
    recordHistogram(histogram, value, max) {
      chrome.send('metricsHandler:recordInHistogram', [histogram, value, max]);
    }

    /**
     * Record an action in UMA.
     * @param {string} action The name of the action to be logged.
     */
    recordAction(action) {
      if (action.indexOf('_') == -1)
        action = `HistoryPage_${action}`;
      chrome.send('metricsHandler:recordAction', [action]);
    }

    /**
     * @param {boolean} successful
     * @private
     */
    resolveDelete_(successful) {
      if (this.pendingDeleteItems_ == null ||
          this.pendingDeletePromise_ == null) {
        return;
      }

      if (successful)
        this.pendingDeletePromise_.resolve(this.pendingDeleteItems_);
      else
        this.pendingDeletePromise_.reject(this.pendingDeleteItems_);

      this.pendingDeleteItems_ = null;
      this.pendingDeletePromise_ = null;
    }

    menuPromoShown() {
      chrome.send('menuPromoShown');
    }
  }

  cr.addSingletonGetter(BrowserService);

  return {BrowserService: BrowserService};
});

/**
 * Called by the history backend when deletion was succesful.
 */
function deleteComplete() {
  md_history.BrowserService.getInstance().resolveDelete_(true);
}

/**
 * Called by the history backend when the deletion failed.
 */
function deleteFailed() {
  md_history.BrowserService.getInstance().resolveDelete_(false);
}
