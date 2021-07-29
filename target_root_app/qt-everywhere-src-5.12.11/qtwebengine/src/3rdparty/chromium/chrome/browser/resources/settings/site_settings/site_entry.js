// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * 'site-entry' is an element representing a single eTLD+1 site entity.
 */
Polymer({
  is: 'site-entry',

  behaviors: [SiteSettingsBehavior],

  properties: {
    /**
     * An object representing a group of sites with the same eTLD+1.
     * @type {!SiteGroup}
     */
    siteGroup: {
      type: Object,
      observer: 'onSiteGroupChanged_',
    },

    /**
     * The name to display beside the icon. If grouped_() is true, it will be
     * the eTLD+1 for all the origins, otherwise, it will return the host.
     * @private
     */
    displayName_: String,

    /**
     * The string to display when there is a non-zero number of cookies.
     * @private
     */
    cookieString_: {
      type: String,
      value: '',
    },
  },

  /** @private {?settings.LocalDataBrowserProxy} */
  localDataBrowserProxy_: null,

  /** @override */
  created: function() {
    this.localDataBrowserProxy_ =
        settings.LocalDataBrowserProxyImpl.getInstance();
  },

  /**
   * Whether the list of origins displayed in this site-entry is a group of
   * eTLD+1 origins or not.
   * @param {SiteGroup} siteGroup The eTLD+1 group of origins.
   * @return {boolean}
   * @private
   */
  grouped_: function(siteGroup) {
    if (siteGroup)
      return siteGroup.origins.length != 1;
    return false;
  },

  /**
   * Returns a user-friendly name for the origin corresponding to |originIndex|.
   * If grouped_() is true and |originIndex| is not provided, returns the eTLD+1
   * for all the origins, otherwise, return the host for that origin.
   * @param {SiteGroup} siteGroup The eTLD+1 group of origins.
   * @param {number} originIndex Index of the origin to get a user-friendly name
   *     for. If -1, returns the eTLD+1 name if any, otherwise defaults to the
   *     first origin.
   * @return {string} The user-friendly name.
   * @private
   */
  siteRepresentation_: function(siteGroup, originIndex) {
    if (!siteGroup)
      return '';
    if (this.grouped_(siteGroup) && originIndex == -1) {
      if (siteGroup.etldPlus1 != '')
        return siteGroup.etldPlus1;
      // Fall back onto using the host of the first origin, if no eTLD+1 name
      // was computed.
    }
    originIndex = this.getIndexBoundToOriginList_(siteGroup, originIndex);
    const url = this.toUrl(siteGroup.origins[originIndex]);
    return url.host;
  },

  /**
   * @param {SiteGroup} siteGroup The eTLD+1 group of origins.
   * @private
   */
  onSiteGroupChanged_: function(siteGroup) {
    this.displayName_ = this.siteRepresentation_(siteGroup, -1);

    if (!this.grouped_(SiteGroup)) {
      // Ensure ungrouped |siteGroup|s do not get stuck in an opened state.
      if (this.$.collapseChild.opened)
        this.toggleCollapsible_();
      // Ungrouped site-entries should not show cookies.
      if (this.cookieString_) {
        this.cookieString_ = '';
        this.fire('site-entry-resized');
      }
    }
    if (!siteGroup || !this.grouped_(siteGroup))
      return;

    this.localDataBrowserProxy_.getNumCookiesString(this.displayName_)
        .then(string => {
          // If there was no cookie string previously and now there is, or vice
          // versa, the height of this site-entry will have changed.
          if ((this.cookieString_ == '') != (string == ''))
            this.fire('site-entry-resized');

          this.cookieString_ = string;
        });
  },

  /**
   * Returns any non-HTTPS scheme/protocol for the origin corresponding to
   * |originIndex|. Otherwise, returns a empty string.
   * @param {SiteGroup} siteGroup The eTLD+1 group of origins.
   * @param {number} originIndex Index of the origin to get the non-HTTPS scheme
   *     for. If -1, returns an empty string for the grouped |siteGroup|s but
   *     defaults to 0 for non-grouped.
   * @return {string} The scheme if non-HTTPS, or empty string if HTTPS.
   * @private
   */
  scheme_: function(siteGroup, originIndex) {
    if (!siteGroup || (this.grouped_(siteGroup) && originIndex == -1))
      return '';
    originIndex = this.getIndexBoundToOriginList_(siteGroup, originIndex);

    const url = this.toUrl(siteGroup.origins[originIndex]);
    const scheme = url.protocol.replace(new RegExp(':*$'), '');
    /** @type{string} */ const HTTPS_SCHEME = 'https';
    if (scheme == HTTPS_SCHEME)
      return '';
    return scheme;
  },

  /**
   * Get an appropriate favicon that represents this group of eTLD+1 sites as a
   * whole.
   * @param {SiteGroup} siteGroup The eTLD+1 group of origins.
   * @return {string} CSS to apply to show the appropriate favicon.
   * @private
   */
  getSiteGroupIcon_: function(siteGroup) {
    // TODO(https://crbug.com/835712): Implement heuristic for finding a good
    // favicon.
    return this.computeSiteIcon(siteGroup.origins[0]);
  },

  /**
   * Navigates to the corresponding Site Details page for the given origin.
   * @param {string} origin The origin to navigate to the Site Details page for
   * it.
   * @private
   */
  navigateToSiteDetails_: function(origin) {
    settings.navigateTo(
        settings.routes.SITE_SETTINGS_SITE_DETAILS,
        new URLSearchParams('site=' + origin));
  },

  /**
   * A handler for selecting a site (by clicking on the origin).
   * @param {!{model: !{index: !number}}} e
   * @private
   */
  onOriginTap_: function(e) {
    this.navigateToSiteDetails_(this.siteGroup.origins[e.model.index]);
  },

  /**
   * A handler for clicking on a site-entry heading. This will either show a
   * list of origins or directly navigates to Site Details if there is only one.
   * @private
   */
  onSiteEntryTap_: function() {
    // Individual origins don't expand - just go straight to Site Details.
    if (!this.grouped_(this.siteGroup)) {
      this.navigateToSiteDetails_(this.siteGroup.origins[0]);
      return;
    }
    this.toggleCollapsible_();

    // Make sure the expanded origins can be viewed without further scrolling
    // (in case |this| is already at the bottom of the viewport).
    this.scrollIntoViewIfNeeded();
  },

  /**
   * Toggles open and closed the list of origins if there is more than one.
   * @private
   */
  toggleCollapsible_: function() {
    let collapseChild =
        /** @type {IronCollapseElement} */ (this.$.collapseChild);
    collapseChild.toggle();
    this.$.toggleButton.setAttribute('aria-expanded', collapseChild.opened);
    this.$.expandIcon.toggleClass('icon-expand-more');
    this.$.expandIcon.toggleClass('icon-expand-less');
    this.fire('iron-resize');
  },

  /**
   * Opens the overflow menu at event target.
   * @param {!{target: !Element}} e
   * @private
   */
  showOverflowMenu_: function(e) {
    this.$.menu.get().showAt(e.target);
  },

  /** @private */
  onCloseDialog_: function(e) {
    e.target.closest('cr-dialog').close();
    this.$.menu.get().close();
  },

  /**
   * Confirms the resetting of all content settings for an origin.
   * @param {!{target: !Element}} e
   * @private
   */
  onConfirmResetSettings_: function(e) {
    e.preventDefault();
    this.$.confirmResetSettings.showModal();
  },

  /**
   * Resets all permissions for all origins listed in |siteGroup.origins|.
   * @param {!Event} e
   * @private
   */
  onResetSettings_: function(e) {
    const contentSettingsTypes = this.getCategoryList();
    for (let i = 0; i < this.siteGroup.origins.length; ++i) {
      const origin = this.siteGroup.origins[i];
      this.browserProxy.setOriginPermissions(
          origin, contentSettingsTypes, settings.ContentSetting.DEFAULT);
      if (contentSettingsTypes.includes(settings.ContentSettingsTypes.PLUGINS))
        this.browserProxy.clearFlashPref(origin);
    }
    this.onCloseDialog_(e);
  },

  /**
   * Formats the |label| string with |name|, using $<num> as markers.
   * @param {string} label
   * @param {string} name
   * @return {string}
   * @private
   */
  getFormatString_: function(label, name) {
    return loadTimeData.substituteString(label, name);
  },

  /**
   * Returns a valid index for an origin contained in |siteGroup.origins| by
   * clamping the given |index|. This also replaces undefined |index|es with 0.
   * Use this to prevent being given out-of-bounds indexes by dom-repeat when
   * scrolling an iron-list storing these site-entries too quickly.
   * @param {!number=} index
   * @return {number}
   * @private
   */
  getIndexBoundToOriginList_: function(siteGroup, index) {
    return Math.max(0, Math.min(index, siteGroup.origins.length - 1));
  },

  /**
   * Returns the correct class to apply depending on this site-entry's position
   * in a list.
   * @param {number} index
   * @private
   */
  getClassForIndex_: function(index) {
    if (index == 0)
      return 'first';
    return '';
  },
});
