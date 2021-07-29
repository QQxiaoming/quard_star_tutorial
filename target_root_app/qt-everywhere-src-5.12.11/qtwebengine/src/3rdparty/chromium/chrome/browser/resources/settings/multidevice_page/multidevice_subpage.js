// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * Subpage of settings-multidevice-page for managing multidevice features
 * individually and for forgetting a host.
 */
cr.exportPath('settings');

Polymer({
  is: 'settings-multidevice-subpage',

  behaviors: [I18nBehavior],

  properties: {
    /** SettingsPrefsElement 'prefs' Object reference. See prefs.js. */
    prefs: {
      type: Object,
      notify: true,
    },

    // TODO(jordynass): Set this based on data in this.prefs.
    /**
     * If a host has been verified, this is true if that host is and enabled and
     * false if it is disabled. Otherwise it is undefined.
     * @type {boolean|undefined}
     */
    hostEnabled: {
      type: Boolean,
      notify: true,
    },

    /** @type {?SettingsRoutes} */
    routes: {
      type: Object,
      value: settings.routes,
    },

    /** @type {MultiDevicePageContentData} */
    pageContentData: Object,
  },

  /**
   * @return {boolean}
   * @private
   */
  shouldShowIndividualFeatures_: function() {
    return this.pageContentData.mode ===
        settings.MultiDeviceSettingsMode.HOST_SET_VERIFIED;
  },

  /**
   * @return {string}
   * @private
   */
  getStatusText_: function() {
    return this.hostEnabled ? this.i18n('multideviceEnabled') :
                              this.i18n('multideviceDisabled');
  },
});
