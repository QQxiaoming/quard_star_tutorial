// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview 'cr-slider' is a wrapper around paper-slider to alter the
 * styling. The bahavior of the slider remains the same.
 */
Polymer({
  is: 'cr-slider',

  properties: {
    min: Number,

    max: Number,

    snaps: {
      type: Boolean,
      value: true,
    },

    disabled: {
      type: Boolean,
      observer: 'onDisabledChanged_',
    },

    value: Number,
    maxMarkers: Number,

    immediateValue: {
      type: Number,
      observer: 'onImmediateValueChanged_',
    },

    dragging: Boolean,
  },

  listeners: {
    'focus': 'onFocus_',
    'blur': 'onBlur_',
    'keydown': 'onKeyDown_',
    'pointerdown': 'onPointerDown_',
    'pointerup': 'onPointerUp_',
  },

  /** @private {boolean} */
  usedMouse_: false,

  /** @override */
  attached: function() {
    this.onDisabledChanged_();
  },

  /** @private */
  onFocus_: function() {
    this.$.slider.getRipple().holdDown = true;
    this.$.slider._expandKnob();
  },

  /** @private */
  onBlur_: function() {
    this.$.slider.getRipple().holdDown = false;
    this.$.slider._resetKnob();
  },

  /** @private */
  onChange_: function() {
    this.$.slider._setExpand(!this.usedMouse_);
    this.$.slider.getRipple().holdDown = !this.usedMouse_;
    this.usedMouse_ = false;
  },

  /** @private */
  onKeyDown_: function() {
    this.usedMouse_ = false;
    if (!this.disabled)
      this.onFocus_();
  },

  /**
   * @param {!MouseEvent} event
   * @private
   */
  onPointerDown_: function(event) {
    if (this.disabled || event.button != 0) {
      event.preventDefault();
      return;
    }
    this.usedMouse_ = true;
    setTimeout(() => {
      this.$.slider.getRipple().holdDown = true;
    });
  },

  /**
   * @param {!MouseEvent} event
   * @private
   */
  onPointerUp_: function(event) {
    if (event.button != 0)
      return;
    this.$.slider.getRipple().holdDown = false;
  },

  /**
   * The style is being set in this way to keep the knob size the same
   * regardless of the state or properties set in the paper-slider. paper-slider
   * styles alter the size in multiple places making it difficult to introduce
   * one or two mixins to override the existing paper-slider knob styling.
   * @private
   */
  onDisabledChanged_: function() {
    const knob = this.$.slider.$$('.slider-knob-inner');
    if (this.disabled) {
      knob.style.transform = 'scale(1.2)';
      knob.style.border = '2px solid white';
    } else {
      knob.style.margin = '11px';
      knob.style.height = '10px';
      knob.style.width = '10px';
      knob.style.transform = 'unset';
    }
  },

  /** @private */
  onImmediateValueChanged_: function() {
    // TODO(dpapad): Need to catch and refire the property changed event in
    // Polymer 2 only, since it does not bubble by default. Remove the
    // condition when migration to Polymer 2 is completed.
    if (Polymer.DomIf)
      this.fire('immediate-value-changed', this.immediateValue);
  },

  /**
   * TODO(scottchen): temporary fix until polymer gesture bug resolved. See:
   * https://github.com/PolymerElements/paper-slider/issues/186
   * @private
   */
  resetTrackLock_: function() {
    Polymer.Gestures.gestures.tap.reset();
  },
});
