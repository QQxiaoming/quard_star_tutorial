// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * 'site-data-entry' handles showing the local storage summary for a site.
 */

Polymer({
  is: 'site-data-entry',

  behaviors: [
    FocusRowBehavior,
    I18nBehavior,
  ],

  properties: {
    /** @type {!CookieDataSummaryItem} */
    model: Object,

    /**
     * Icon to use for a given site.
     * @private
     */
    favicon_: {
      type: String,
      computed: 'computeFavicon_(model)',
    },
  },

  /** @private {settings.LocalDataBrowserProxy} */
  browserProxy_: null,

  /** @override */
  ready: function() {
    this.browserProxy_ = settings.LocalDataBrowserProxyImpl.getInstance();
  },

  /**
   * @return {string}
   * @private
   */
  computeFavicon_: function() {
    const url = this.model.site;
    // If the url doesn't have a scheme, inject HTTP as the scheme. Otherwise,
    // the URL isn't valid and no icon will be returned.
    const urlWithScheme = url.includes('://') ? url : 'http://' + url;
    return cr.icon.getFavicon(urlWithScheme);
  },

  /**
   * Deletes all site data for this site.
   * @param {!Event} e
   * @private
   */
  onRemove_: function(e) {
    e.stopPropagation();
    this.browserProxy_.removeItem(this.model.site);
  },
});
