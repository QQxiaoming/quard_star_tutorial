// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * display-size-slider is used to change the value of a pref via a slider
 * control. This specific slider is used instead of the settings-slider due to
 * its implementation of the tool tip that displays the current slider value.
 * This component fires a |immediate-value-changed| event while the dragging is
 * active. This event includes the immediate value of the slider.
 *
 * TODO (crbug.com/858882): this control should be replaced with a common
 * control like settings-slider or cr-slider.
 */

/**
 * The |value| is the corresponding value that the current slider tick is
 * assocated with. The string |label| is shown in the ui as the label for the
 * current slider value. The |ariaValue| number is used for aria-valuemin,
 * aria-valuemax, and aria-valuenow, and is optional. If missing, |value| will
 * be used instead.
 * @typedef {{
 *   value: !number,
 *   label: !string,
 *   ariaValue: (number|undefined),
 * }}
 */
let SliderTick;

/**
 * @typedef {!Array<SliderTick>}
 */
let SliderTicks;

(function() {

Polymer({
  is: 'display-size-slider',

  behaviors: [
    CrPolicyPrefBehavior,
    Polymer.IronA11yKeysBehavior,
    Polymer.IronRangeBehavior,
    Polymer.IronResizableBehavior,
    Polymer.PaperInkyFocusBehavior,
    PrefsBehavior,
  ],

  properties: {
    /** The slider is disabled if true. */
    disabled: {type: Boolean, value: false, readOnly: true},

    /** True when the user is dragging the slider knob. */
    dragging: {type: Boolean, value: false, readOnly: true},

    /** @type {number} The index for the current slider value in |ticks|. */
    index: {
      type: Number,
      value: 0,
      readOnly: true,
      observer: 'onIndexChanged_',
    },

    /** @type {string} The label for the minimum slider value */
    minLabel: {type: String, value: ''},

    /** @type {string} The label for the maximum slider value */
    maxLabel: {type: String, value: ''},

    /**
     * Each item in the array represents a UI element corresponding to a tick
     * value.
     * @type {!Array<number>}
     */
    markers: {
      type: Array,
      readOnly: true,
      value: function() {
        return [];
      }
    },

    /** @type {!chrome.settingsPrivate.PrefObject} */
    pref: Object,

    /**
     * The data associated with each tick on the slider. Each element in the
     * array contains a value and the label corresponding to that value.
     * @type {SliderTicks}
     */
    ticks: {
      type: Array,
      value: () => [],
    },

    /** @private */
    holdDown_: {
      type: Boolean,
      value: false,
      reflectToAttribute: true,
    },
  },

  listeners: {
    'blur': 'onBlur_',
    'focus': 'onFocus_',
    'keydown': 'onKeyDown_',
    'pointerdown': 'onPointerDown_',
    'pointerup': 'onPointerUp_',
  },

  observers: [
    'updateIndex_(pref.value)',
    'updateSliderParams_(ticks)',
    'updateMarkers_(min, max)',
    'updateDisabled_(ticks, pref.*)',
  ],

  hostAttributes: {role: 'slider', tabindex: 0},

  keyBindings: {
    'left': 'leftKeyPress_',
    'right': 'rightKeyPress_',
    'up': 'increment_',
    'down': 'decrement_',
    'pagedown home': 'resetToMinIndex_',
    'pageup end': 'resetToMaxIndex_',
  },

  /** @private {boolean} */
  usedMouse_: false,

  get _isRTL() {
    if (this.__isRTL === undefined) {
      this.__isRTL = window.getComputedStyle(this)['direction'] === 'rtl';
    }
    return this.__isRTL;
  },

  /** @private */
  setRippleHoldDown_: function(holdDown) {
    this.ensureRipple();
    this._ripple.holdDown = holdDown;
    this.holdDown_ = holdDown;
  },

  /** @private */
  onFocus_: function() {
    this.setRippleHoldDown_(true);
  },

  /** @private */
  onBlur_: function() {
    this.setRippleHoldDown_(false);
  },

  /** @private */
  onChange_: function() {
    this.setRippleHoldDown_(!this.usedMouse_);
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
    this.setRippleHoldDown_(false);
    setTimeout(() => {
      this.setRippleHoldDown_(true);
    });
  },

  /**
   * @param {!MouseEvent} event
   * @private
   */
  onPointerUp_: function(event) {
    if (event.button != 0)
      return;
    this.setRippleHoldDown_(false);
  },

  /**
   * Clamps the value of |newIndex| to IronRangeBehavior's max/min bounds and
   * updates the value for |index|.
   * @param {number} newIndex The new value for index that needs to be set.
   * @private
   */
  clampAndSetIndex_: function(newIndex) {
    newIndex = this.clampToRange_(newIndex, this.min, this.max);
    if (newIndex != this.index) {
      this._setIndex(newIndex);
      this.setAttribute(
          'aria-valuenow', this.getAriaValueForIndex_(this.ticks, this.index));
      this.setAttribute(
          'aria-valuetext', this.getLabelForIndex_(this.ticks, this.index));
      if (this.dragging) {
        this.fire(
            'immediate-value-changed', {value: this.ticks[newIndex].value});
      }
    }
  },

  /**
   * Clamps the value of |newIndex| to IronRangeBehavior's max/min bounds and
   * updates the value of |pref| to this clamped value.
   * @param {number} newIndex The new value for index that needs to be set.
   * @private
   */
  clampIndexAndUpdatePref_: function(newIndex) {
    newIndex = this.clampToRange_(newIndex, this.min, this.max);
    if (this.ticks[newIndex].value != this.pref.value)
      this.set('pref.value', this.ticks[newIndex].value);
    this.onChange_();
  },

  /**
   * Clamps and returns the given |value| within the |min| and |max| range.
   * @param {number} value The number to clamp.
   * @param {number} min The minimum value in range. Any value below this will
   *     be clamped to |min|.
   * @param {number} max The maximum value in range. Any value above this will
   *     be clamped to |max|.
   * @return {number}
   * @private
   */
  clampToRange_: function(value, min, max) {
    return Math.max(Math.min(value, max), min);
  },

  /**
   * Overrides _createRipple from PaperInkyFocusBehavior to set the ripple
   * container as the slider knob before creating the ripple animation. Without
   * this the PaperInkyFocusBehavior does not know where to create the ripple
   * animation.
   * @protected
   */
  _createRipple: function() {
    this._rippleContainer = this.$.sliderKnob;
    return Polymer.PaperInkyFocusBehaviorImpl._createRipple.call(this);
  },

  /** @private Safely decrements the slider index value by 1 and updates pref */
  decrement_: function() {
    this.clampIndexAndUpdatePref_(this.index - 1);
  },

  /**
   * Returns a string concatenated list of class names based on whether the
   * corresponding properties are set.
   * @return {string}
   */
  getClassNames_: function() {
    return this.mergeClasses_({
      disabled: this.disabled,
      dragging: this.dragging,
    });
  },

  /**
   * Returns the current label for the selected slider index.
   * @param {SliderTicks} ticks Slider label and corresponding value for each
   *    tick.
   * @param {number} index Index of the slider tick with the desired label.
   * @return {string}
   */
  getLabelForIndex_: function(ticks, index) {
    if (!ticks || ticks.length == 0 || index >= ticks.length)
      return '';
    return ticks[index].label;
  },

  /**
   * Returns the aria value for a selected slider index. aria-valuenow,
   * aria-valuemin and aria-valuemax are expected to be a numbers, so sliders
   * which use strings for labels should populate the ariaValue with a number
   * as well.
   * @param {SliderTicks} ticks Slider label and corresponding value for each
   *    tick.
   * @param {number} index Index of the slider tick with the desired label.
   * @return {number|string} Returns the empty string if there is not tick at
   *    the given index.
   */
  getAriaValueForIndex_: function(ticks, index) {
    if (!ticks || ticks.length == 0 || index >= ticks.length)
      return '';
    // ariaValue factored out for closure compilation.
    let ariaValue = ticks[index].ariaValue;
    return ariaValue !== undefined ? ariaValue : ticks[index].value;
  },

  /** @private Safely increments the slider index value by 1 and updates pref */
  increment_: function() {
    this.clampIndexAndUpdatePref_(this.index + 1);
  },

  /**
   * Handles the mouse drag and drop event for slider knob from start to end.
   * @param {Event} event
   * @private
   */
  knobTrack_: function(event) {
    switch (event.detail.state) {
      case 'start':
        this.knobTrackStart_();
        break;
      case 'track':
        this.knobTrackDrag_(event);
        break;
      case 'end':
        this.knobTrackEnd_(event);
        break;
    }
  },

  /**
   * Handles the drag event for the slider knob.
   * @param {!Event} event
   * @private
   */
  knobTrackDrag_: function(event) {
    // Distance travelled during mouse drag.
    const dx = event.detail.dx;

    // Total width of the progress bar in CSS pixels.
    const totalWidth = this.$.sliderBar.offsetWidth;

    // Distance between 2 consecutive tick markers.
    const tickWidth = totalWidth / (this.ticks.length - 1);

    // Number of ticks covered by |dx|.
    let dxInTicks = Math.round(dx / tickWidth);

    if (this._isRTL)
      dxInTicks *= -1;

    let nextIndex = this.startIndex_ + dxInTicks;

    this.clampAndSetIndex_(nextIndex);
  },

  /**
   * Handles the end of slider knob drag event.
   * @param {!Event} event
   * @private
   */
  knobTrackEnd_: function(event) {
    this._setDragging(false);

    // Update the pref once user stops dragging and releases mouse.
    if (this.index != this.startIndex_)
      this.clampIndexAndUpdatePref_(this.index);
  },

  /**
   * Handles the start of the slider knob drag event.
   * @private
   */
  knobTrackStart_: function() {
    this.startIndex_ = this.index;
    this._setDragging(true);
  },

  /** @private Handles the 'left' key press. */
  leftKeyPress_: function() {
    this._isRTL ? this.increment_() : this.decrement_();
  },

  /**
   * Returns a concatenated string of classnames based on the boolean table
   * |classes|.
   * @param {!Object<string, boolean>} classes An object mapping between
   *     classnames and boolean. The boolean for the corresponding classname is
   *     true if that classname needs to be present in the returned string.
   * @return {string}
   * @private
   */
  mergeClasses_: function(classes) {
    return Object.keys(classes)
        .filter(function(className) {
          return classes[className];
        })
        .join(' ');
  },

  /**
   * Handles the event where the user clicks or taps on the slider bar directly.
   * @param {!Event} event
   * @private
   */
  onBarDown_: function(event) {
    const barWidth = this.$.sliderBar.offsetWidth;
    const barOriginX = this.$.sliderBar.getBoundingClientRect().left;
    let eventOffsetFromOriginX = event.detail.x - barOriginX;
    if (this._isRTL)
      eventOffsetFromOriginX = barWidth - eventOffsetFromOriginX;
    const tickWidth = barWidth / (this.ticks.length - 1);
    let newTickIndex = Math.round(eventOffsetFromOriginX / tickWidth);
    this._setDragging(true);
    this.startIndex_ = this.index;

    // Update the index but dont update the pref until mouse is released.
    this.clampAndSetIndex_(newTickIndex);
  },

  /**
   * Handles the event of mouse up from the slider bar.
   * @private
   */
  onBarUp_: function() {
    if (this.dragging)
      this._setDragging(false);
    if (this.startIndex_ != this.index)
      this.clampIndexAndUpdatePref_(this.index);
  },

  /** @private Handles the 'right' key press. */
  rightKeyPress_: function() {
    this._isRTL ? this.decrement_() : this.increment_();
  },

  /** @private Handles the 'end' and 'page up' key press. */
  resetToMaxIndex_: function() {
    this.clampIndexAndUpdatePref_(this.max);
  },

  /** @private Handles the 'home' and 'page down' key press. */
  resetToMinIndex_: function() {
    this.clampIndexAndUpdatePref_(this.min);
  },

  /**
   * Sets the disabled property if there are no tick values or if the pref
   * cannot be modified by the user.
   * @private
   */
  updateDisabled_: function() {
    let disabled = false;

    // Disabled if no tick values are set for the slider.
    if (!this.ticks || this.ticks.length <= 1)
      disabled |= true;


    // Disabled if the pref cannot be modified.
    disabled |= this.isPrefEnforced();

    this._setDisabled(!!disabled);
  },

  /**
   * Updates the value for |index| based on the current set value of |pref|.
   * @private
   */
  updateIndex_: function() {
    if (!this.ticks || this.ticks.length == 0)
      return;
    if (!this.pref)
      return;
    for (let i = 0; i < this.ticks.length; i++) {
      if (this.ticks[i].value == this.pref.value) {
        this._setIndex(i);
        this.setAttribute(
            'aria-valuenow',
            this.getAriaValueForIndex_(this.ticks, this.index));
        this.setAttribute(
            'aria-valuetext', this.getLabelForIndex_(this.ticks, this.index));
      }
    }
  },

  /**
   * Updates the knob position based on the the value of progress indicator.
   * @private
   */
  onIndexChanged_: function() {
    this._setRatio(this._calcRatio(this.index) * 100);

    this.$.sliderKnob.style.left = this.ratio + '%';
    this.$.label.style.left = this.ratio + '%';
  },

  /**
   * Initializes the |markers| array based on the number of ticks.
   * @private
   */
  updateMarkers_: function(min, max) {
    let steps = Math.round((max - min) / this.step);
    if (steps < 0 || !isFinite(steps))
      steps = 0;
    this._setMarkers(new Array(steps));
  },

  /**
   * Updates the min and max possible values for the slider.
   * @private
   */
  updateSliderParams_: function() {
    this.min = 0;
    if (this.ticks.length == 0) {
      this.max = 0;
      return;
    }
    this.max = this.ticks.length - 1;
    this.setAttribute(
        'aria-valuemin', this.getAriaValueForIndex_(this.ticks, this.min));
    this.setAttribute(
        'aria-valuemax', this.getAriaValueForIndex_(this.ticks, this.max));
    this.updateIndex_();
  },
});
})();
