// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @const
 */
var nux_google_apps = nux_google_apps || {};

/**
 * @typedef {{
 *   item: !{
 *     name: string,
 *     icon: string,
 *     selected: boolean,
 *   },
 *   set: function(string, boolean):void
 * }}
 */
nux_google_apps.AppsArrayModel;

Polymer({
  is: 'apps-chooser',
  properties: {
    // TODO(hcarmona): Get this list dynamically.
    appList: {
      type: Array,
      value: function() {
        return [
          {
            name: 'Gmail',
            icon: 'gmail',
            selected: true,
          },
          {
            name: 'YouTube',
            icon: 'youtube',
            selected: true,
          },
          {
            name: 'Maps',
            icon: 'maps',
            selected: true,
          },
          {
            name: 'Translate',
            icon: 'translate',
            selected: true,
          },
          {
            name: 'News',
            icon: 'news',
            selected: true,
          },
          {
            name: 'Chrome Web Store',
            icon: 'chrome_store',
            selected: true,
          },
        ];
      },
    },

    hasAppsSelected: {
      type: Boolean,
      notify: true,
      value: true,
    }
  },

  /**
   * Returns an array of booleans for each selected app.
   * @return {Array<boolean>}
   */
  getSelectedAppList() {
    return this.appList.map(a => a.selected)
  },

  /**
   * Handle toggling the apps selected.
   * @param {!{model: !nux_google_apps.AppsArrayModel}} e
   * @private
   */
  onAppClick_: function(e) {
    e.model.set('item.selected', !e.model.item.selected);
    this.hasAppsSelected = this.computeHasAppsSelected_();
  },

  /** @private {boolean} */
  computeHasAppsSelected_: function() {
    return this.appList.some(a => a.selected);
  },
});
