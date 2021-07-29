// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

cr.define('settings', function() {
  /**
   * The possible statuses of hosts on the logged in account that determine the
   * page content. Note that this is based on (and must include an analog of
   * all values in) the HostStatus enum in
   * services/multidevice_setup/public/mojom/multidevice_setup.mojom.
   * @enum {number}
   */
  MultiDeviceSettingsMode = {
    NO_ELIGIBLE_HOSTS: 0,
    NO_HOST_SET: 1,
    HOST_SET_WAITING_FOR_SERVER: 2,
    HOST_SET_WAITING_FOR_VERIFICATION: 3,
    HOST_SET_VERIFIED: 4,
  };

  /**
   * MultiDevice software features. Note that this is copied from (and must
   * include an analog of all values in) the enum of the same name in
   * //components/cryptauth/proto/cryptauth_api.proto.
   * @enum {number}
   */
  MultiDeviceSoftwareFeature = {
    UNKNOWN_FEATURE: 0,
    BETTER_TOGETHER_HOST: 1,
    BETTER_TOGETHER_CLIENT: 2,
    EASY_UNLOCK_HOST: 3,
    EASY_UNLOCK_CLIENT: 4,
    MAGIC_TETHER_HOST: 5,
    MAGIC_TETHER_CLIENT: 6,
    SMS_CONNECT_HOST: 7,
    SMS_CONNECT_CLIENT: 8,
  };

  /**
   * Possible states of MultiDevice software features. Note that this is based
   * on (and must include an analog of all values in) the enum of the same name
   * in //components/cryptauth/software_feature_state.h.
   * @enum {number}
   */
  MultiDeviceSoftwareFeatureState = {
    NOT_SUPPORTED: 0,
    SUPPORTED: 1,
    ENABLED: 2,
  };

  return {
    MultiDeviceSettingsMode: MultiDeviceSettingsMode,
    MultiDeviceSoftwareFeature: MultiDeviceSoftwareFeature,
    MultiDeviceSoftwareFeatureState: MultiDeviceSoftwareFeatureState,
  };
});

/**
 * Represents a multidevice host, i.e. a phone set by the user to connect to
 * their Chromebook(s). The type is a subset of the RemoteDevice structure
 * defined by CryptAuth (components/cryptauth/remote_device.h). It contains the
 * host device's name (e.g. Pixel, Nexus 5) and the map softwareFeatures
 * sending each MultiDevice feature to the host device's state with regards to
 * that feature.
 *
 * @typedef {{
 *   name: string,
 *   softwareFeatures:
 *       !Object<settings.MultiDeviceSoftwareFeature,
 *           settings.MultiDeviceSoftwareFeatureState>
 * }}
 */
let RemoteDevice;

/**
 * Container for the initial data that the page requires in order to display
 * the correct content. It is also used for receiving status updates during
 * use. Note that the host may be verified (enabled or disabled), awaiting
 * verification, or it may have failed setup because it was not able to connect
 * to the server. If the property is null or undefined, then no host has been
 * set up, although there may be potential hosts on the account.
 *
 * @typedef {{
 *   mode: !settings.MultiDeviceSettingsMode,
 *   hostDevice: (?RemoteDevice|undefined)
 * }}
 */
let MultiDevicePageContentData;
