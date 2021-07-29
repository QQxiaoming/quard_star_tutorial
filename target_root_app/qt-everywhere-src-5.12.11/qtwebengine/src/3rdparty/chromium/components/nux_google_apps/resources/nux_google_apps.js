// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

Polymer({
  is: 'nux-google-apps',

  properties: {
    /** @private */
    hasAppsSelected_: Boolean,
  },

  /** @private */
  onNoThanksClicked_: function() {
    chrome.send('rejectGoogleApps');
    window.location.replace('chrome://newtab');
  },

  /** @private */
  onGetStartedClicked_: function() {
    let selectedApps = this.$.appChooser.getSelectedAppList();
    nux.NuxGoogleAppsProxyImpl.getInstance().addGoogleApps(selectedApps);
    window.location.replace('chrome://newtab');
  },
});
