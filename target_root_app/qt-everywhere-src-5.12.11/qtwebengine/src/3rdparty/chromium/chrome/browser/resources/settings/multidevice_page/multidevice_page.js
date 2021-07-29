// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * Settings page for managing MultiDevice features.
 */
cr.exportPath('settings');

Polymer({
  is: 'settings-multidevice-page',

  behaviors: [I18nBehavior],

  properties: {
    /** SettingsPrefsElement 'prefs' Object reference. See prefs.js. */
    prefs: {
      type: Object,
      notify: true,
    },

    /** @type {MultiDevicePageContentData} */
    pageContentData: Object,

    // TODO(jordynass): Set this variable once the information is retrieved from
    // prefs.
    /**
     * True if the multidevice setup is complete and the paired phone has been
     * verified; otherwise, false.
     * @private {boolean}
     */
    hostEnabled_: {
      type: Boolean,
      value: true,
    },

    /**
     * A Map specifying which element should be focused when exiting a subpage.
     * The key of the map holds a settings.Route path, and the value holds a
     * query selector that identifies the desired element.
     * @private {!Map<string, string>}
     */
    focusConfig_: {
      type: Object,
      value: function() {
        const map = new Map();
        if (settings.routes.MULTIDEVICE_FEATURES)
          map.set(
              settings.routes.MULTIDEVICE_FEATURES.path,
              '#multidevice-item .subpage-arrow');
        return map;
      },
    },
  },

  /** @private {?settings.MultiDeviceBrowserProxy} */
  browserProxy_: null,

  /** @override */
  created: function() {
    this.browserProxy_ = settings.MultiDeviceBrowserProxyImpl.getInstance();
  },

  /**
   * @return {string} Translated item label.
   * @private
   */
  getLabelText_: function() {
    return !!this.pageContentData.hostDevice ?
        this.pageContentData.hostDevice.name :
        this.i18n('multideviceSetupItemHeading');
  },

  /**
   * @return {string} Translated sublabel with a "learn more" link.
   * @private
   */
  getSubLabelInnerHtml_: function() {
    switch (this.pageContentData.mode) {
      case settings.MultiDeviceSettingsMode.NO_HOST_SET:
        return this.i18nAdvanced('multideviceSetupSummary');
      case settings.MultiDeviceSettingsMode.HOST_SET_WAITING_FOR_SERVER:
        return this.i18nAdvanced('multideviceCouldNotConnect');
      case settings.MultiDeviceSettingsMode.HOST_SET_WAITING_FOR_VERIFICATION:
        return this.i18nAdvanced('multideviceVerificationText');
      default:
        return this.hostEnabled_ ? this.i18n('multideviceEnabled') :
                                   this.i18n('multideviceDisabled');
    }
  },

  /**
   * @return {string} Translated button text.
   * @private
   */
  getButtonText_: function() {
    switch (this.pageContentData.mode) {
      case settings.MultiDeviceSettingsMode.NO_HOST_SET:
        return this.i18n('multideviceSetupButton');
      case settings.MultiDeviceSettingsMode.HOST_SET_WAITING_FOR_SERVER:
        return this.i18n('retry');
      case settings.MultiDeviceSettingsMode.HOST_SET_WAITING_FOR_VERIFICATION:
        return this.i18n('multideviceVerifyButton');
      default:
        return '';
    }
  },

  /**
   * @return {boolean}
   * @private
   */
  showButton_: function() {
    return this.pageContentData.mode !=
        settings.MultiDeviceSettingsMode.HOST_SET_VERIFIED;
  },

  /**
   * @return {boolean}
   * @private
   */
  showToggle_: function() {
    return this.pageContentData.mode ==
        settings.MultiDeviceSettingsMode.HOST_SET_VERIFIED;
  },

  /**
   * @return {boolean}
   * @private
   */
  doesClickOpenSubpage_: function() {
    return [
      settings.MultiDeviceSettingsMode.HOST_SET_WAITING_FOR_SERVER,
      settings.MultiDeviceSettingsMode.HOST_SET_WAITING_FOR_VERIFICATION,
      settings.MultiDeviceSettingsMode.HOST_SET_VERIFIED,
    ].includes(this.pageContentData.mode);
  },

  /** @private */
  handleItemClick_: function() {
    if (!this.doesClickOpenSubpage_())
      return;
    settings.navigateTo(settings.routes.MULTIDEVICE_FEATURES);
  },

  /** @private */
  handleButtonClick_: function(event) {
    event.stopPropagation();
    switch (this.pageContentData.mode) {
      case settings.MultiDeviceSettingsMode.NO_HOST_SET:
        this.browserProxy_.showMultiDeviceSetupDialog();
        return;
      case settings.MultiDeviceSettingsMode.HOST_SET_WAITING_FOR_SERVER:
        // TODO(jordynass): Implement this when API is ready.
        console.log('Trying to connect to server again.');
        return;
      case settings.MultiDeviceSettingsMode.HOST_SET_WAITING_FOR_VERIFICATION:
        // TODO(jordynass): Implement this when API is ready.
        console.log('Trying to verify multidevice connection.');
        return;
    }
  },
});
