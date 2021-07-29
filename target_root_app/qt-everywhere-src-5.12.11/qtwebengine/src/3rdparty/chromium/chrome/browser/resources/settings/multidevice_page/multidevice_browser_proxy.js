// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

cr.define('settings', function() {
  /** @interface */
  class MultiDeviceBrowserProxy {
    showMultiDeviceSetupDialog() {}

    /** @return Promise<!MultiDevicePageContentData> */
    getPageContentData() {}
  }

  /**
   * @implements {settings.MultiDeviceBrowserProxy}
   */
  class MultiDeviceBrowserProxyImpl {
    /** @override */
    showMultiDeviceSetupDialog() {
      chrome.send('showMultiDeviceSetupDialog');
    }

    /** @override */
    getPageContentData() {
      // TODO(jordynass): change method content to
      //    return cr.sendWithPromise('getPageContentData');
      // once handler is built.
      return Promise.resolve({
        mode: settings.MultiDeviceSettingsMode.HOST_SET_VERIFIED,
        hostDevice: {
          name: 'Pixel XL',
        },
      });
    }
  }

  cr.addSingletonGetter(MultiDeviceBrowserProxyImpl);

  return {
    MultiDeviceBrowserProxy: MultiDeviceBrowserProxy,
    MultiDeviceBrowserProxyImpl: MultiDeviceBrowserProxyImpl,
  };
});
