// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Container element that interfaces with the
 * MultiDeviceBrowserProxy to ensure that if there is not a host device or
 * potential host device
 *     (a) the entire multidevice settings-section is hidden and
 *     (b) the settings-multidevice-page is not attched to the DOM.
 * It also provides the settings-multidevice-page with the data it needs to
 * provide the user with the correct infomation and call(s) to action based on
 * the data retrieved from the browser proxy (i.e. the mode_ property).
 */

cr.exportPath('settings');

Polymer({
  is: 'settings-multidevice-page-container',

  behaviors: [WebUIListenerBehavior],

  properties: {
    /** SettingsPrefsElement 'prefs' Object reference. See prefs.js. */
    prefs: {
      type: Object,
      notify: true,
    },

    /**
     * Whether a phone was found on the account that is either connected to the
     * Chromebook or has the potential to be.
     * @type {boolean}
     */
    doesPotentialConnectedPhoneExist: {
      type: Boolean,
      computed: 'computeDoesPotentialConnectedPhoneExist(pageContentData_)',
      notify: true,
    },

    /** @private {MultiDevicePageContentData} */
    pageContentData_: Object,
  },

  /** @private {?settings.MultiDeviceBrowserProxy} */
  browserProxy_: null,

  /** @override */
  ready: function() {
    this.browserProxy_ = settings.MultiDeviceBrowserProxyImpl.getInstance();

    this.addWebUIListener(
        'settings.updateMultidevicePageContentData',
        this.onPageContentDataChanged_.bind(this));

    this.browserProxy_.getPageContentData().then(
        this.onPageContentDataChanged_.bind(this));
  },

  /**
   * @param {!MultiDevicePageContentData} newData
   * @private
   */
  onPageContentDataChanged_: function(newData) {
    if (!this.isStatusChangeValid_(newData)) {
      console.error('Invalid status change');
      return;
    }
    this.pageContentData_ = newData;
  },

  /**
   * If the new mode corresponds to no eligible host or unset potential hosts
   * (i.e. NO_ELIGIBLE_HOSTS or NO_HOST_SET), then newHostDevice should be null
   * or undefined. Otherwise it should be defined and non-null.
   * @param {!MultiDevicePageContentData} newData
   * @private
   */
  isStatusChangeValid_: function(newData) {
    const noHostModes = [
      settings.MultiDeviceSettingsMode.NO_ELIGIBLE_HOSTS,
      settings.MultiDeviceSettingsMode.NO_HOST_SET,
    ];
    return !newData.hostDevice == noHostModes.includes(newData.mode);
  },

  /**
   * @return {boolean}
   * @private
   */
  computeDoesPotentialConnectedPhoneExist: function() {
    return !!this.pageContentData_ &&
        this.pageContentData_.mode !=
        settings.MultiDeviceSettingsMode.NO_ELIGIBLE_HOSTS;
  },
});
