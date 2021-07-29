// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Polymer behavior for alerting specified child elements of
 * changes to the devices network data.
 */

/** @polymerBehavior */
const NetworkListenerBehavior = {
  properties: {
    /**
     * Array of selectors specifying all children to alert of changes to the
     * network list.
     * @private {!Array<string>}
     */
    networkListChangeSubscriberSelectors_: Array,

    /**
     * Array of selectors specifying all children to alert of important changes
     * to the specific networks.
     * @private {!Array<string>}
     */
    networksChangeSubscriberSelectors_: Array,

    /** @type {!NetworkingPrivate} */
    networkingPrivate: Object,
  },

  /** @private {?function(!Array<string>)} */
  networkListChangedListener_: null,

  /** @private {?function(!Array<string>)} */
  networksChangedListener_: null,

  /** @override */
  attached: function() {
    this.networkListChangedListener_ = this.networkListChangedListener_ ||
        this.onNetworkListChanged_.bind(this);
    this.networkingPrivate.onNetworkListChanged.addListener(
        this.networkListChangedListener_);

    this.networksChangedListener_ =
        this.networksChangedListener_ || this.onNetworksChanged_.bind(this);
    this.networkingPrivate.onNetworksChanged.addListener(
        this.networksChangedListener_);
  },

  /** @override */
  detached: function() {
    this.networkingPrivate.onNetworkListChanged.removeListener(
        assert(this.networkListChangedListener_));
    this.networkingPrivate.onNetworksChanged.removeListener(
        assert(this.networksChangedListener_));
  },

  /**
   * This event is triggered when the list of networks changes.
   * |networkIds| contains the ids for all visible or configured networks.
   * networkingPrivate.onNetworkListChanged event callback.
   * @param {!Array<string>} networkIds
   * @private
   */
  onNetworkListChanged_: function(networkIds) {
    const event = new CustomEvent('network-list-changed', {detail: networkIds});
    this.networkListChangeSubscriberSelectors_.forEach(
        selector => this.maybeDispatchEvent_(selector, event));
  },

  /**
   * This event is triggered when interesting properties of a network change.
   * |networkIds| contains the ids for networks whose properties have changed.
   * networkingPrivate.onNetworksChanged event callback.
   * @param {!Array<string>} networkIds
   * @private
   */
  onNetworksChanged_: function(networkIds) {
    const event = new CustomEvent('networks-changed', {detail: networkIds});
    this.networksChangeSubscriberSelectors_.forEach(
        selector => this.maybeDispatchEvent_(selector, event));
  },

  /**
   * @param {!Event} event
   * @private
   */
  maybeDispatchEvent_: function(selectors, event) {
    const element = this.$$(selectors);
    if (!element)
      return;
    element.dispatchEvent(event);
  },
};
