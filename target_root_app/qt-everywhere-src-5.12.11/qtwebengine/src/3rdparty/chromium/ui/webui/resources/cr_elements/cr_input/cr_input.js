// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview 'cr-input' is a component similar to native input.
 *
 * Native input attributes that are currently supported by cr-inputs are:
 *   autofocus
 *   disabled
 *   incremental (only applicable when type="search")
 *   maxlength
 *   minlength
 *   pattern
 *   placeholder
 *   readonly
 *   required
 *   tabindex
 *   type (only 'text', 'password', and 'search' supported)
 *   value
 *
 * Additional attributes that you can use with cr-input:
 *   label
 *   auto-validate - triggers validation based on |pattern| and |required|,
 *                   whenever |value| changes.
 *   error-message - message displayed under the input when |invalid| is true.
 *   invalid
 *
 * You may pass an element into cr-input via [slot="suffix"] to be vertically
 * center-aligned with the input field, regardless of position of the label and
 * error-message. Example:
 *   <cr-input>
 *     <paper-button slot="suffix"></paper-button>
 *   </cr-input>
 */
Polymer({
  is: 'cr-input',

  properties: {
    ariaLabel: String,

    autofocus: {
      type: Boolean,
      value: false,
      reflectToAttribute: true,
    },

    autoValidate: Boolean,

    disabled: {
      type: Boolean,
      value: false,
      reflectToAttribute: true,
      observer: 'disabledChanged_',
    },

    errorMessage: {
      type: String,
      value: '',
    },

    /**
     * This is strictly used internally for styling, do not attempt to use
     * this to set focus.
     * @private
     */
    focused_: {
      type: Boolean,
      value: false,
      reflectToAttribute: true,
    },

    incremental: Boolean,

    invalid: {
      type: Boolean,
      value: false,
      reflectToAttribute: true,
    },

    maxlength: {
      type: Number,
      reflectToAttribute: true,
    },

    minlength: {
      type: Number,
      reflectToAttribute: true,
    },

    pattern: {
      type: String,
      reflectToAttribute: true,
    },

    label: {
      type: String,
      value: '',
    },

    placeholder: {
      type: String,
      observer: 'placeholderChanged_',
    },

    readonly: {
      type: Boolean,
      reflectToAttribute: true,
    },

    required: {
      type: Boolean,
      reflectToAttribute: true,
    },

    /** @type {?number} */
    tabindex: {
      type: Number,
      value: 0,
      reflectToAttribute: true,
    },

    type: {
      type: String,
      value: 'text',  // Only 'text', 'password', 'search' are supported.
    },

    value: {
      type: String,
      value: '',
      notify: true,
      observer: 'onValueChanged_',
    },
  },

  hostAttributes: {
    'aria-disabled': 'false',
  },

  listeners: {
    'input.focus': 'onInputFocus_',
    'input.blur': 'onInputBlur_',
    'input.change': 'onInputChange_',
    'input.keydown': 'onInputKeydown_',
    'focus': 'focusInput_',
    'pointerdown': 'onPointerDown_',
  },

  /** @private {?number} */
  originalTabIndex_: null,

  /** @override */
  attached: function() {
    const ariaLabel = this.ariaLabel || this.label || this.placeholder;
    if (ariaLabel)
      this.inputElement.setAttribute('aria-label', ariaLabel);

    // Run this for the first time in attached instead of in disabledChanged_
    // since this.tabindex might not be set yet then.
    if (this.disabled)
      this.reconcileTabindex_();
  },

  /** @return {!HTMLInputElement} */
  get inputElement() {
    return this.$.input;
  },

  /** @private */
  disabledChanged_: function(current, previous) {
    this.setAttribute('aria-disabled', this.disabled ? 'true' : 'false');
    // In case input was focused when disabled changes.
    this.focused_ = false;

    // Don't change tabindex until after finished attaching, since this.tabindex
    // might not be intialized yet.
    if (previous !== undefined)
      this.reconcileTabindex_();
  },

  /**
   * This helper function manipulates the tabindex based on disabled state. If
   * this element is disabled, this function will remember the tabindex and
   * unset it. If the element is enabled again, it will restore the tabindex
   * to it's previous value.
   * @private
   */
  reconcileTabindex_: function() {
    if (this.disabled) {
      this.recordAndUnsetTabIndex_();
    } else {
      this.restoreTabIndex_();
    }
  },

  /**
   * This is necessary instead of doing <input placeholder="[[placeholder]]">
   * because if this.placeholder is set to a truthy value then removed, it
   * would show "null" as placeholder.
   * @private
   */
  placeholderChanged_: function() {
    if (this.placeholder || this.placeholder == '')
      this.inputElement.setAttribute('placeholder', this.placeholder);
    else
      this.inputElement.removeAttribute('placeholder');
  },

  /** @private */
  focusInput_: function() {
    if (this.shadowRoot.activeElement != this.inputElement)
      this.inputElement.focus();
  },

  /** @private */
  recordAndUnsetTabIndex_: function() {
    // Don't change originalTabIndex_ if it just got changed.
    if (this.originalTabIndex_ === null)
      this.originalTabIndex_ = this.tabindex;

    this.tabindex = null;
  },

  /** @private */
  restoreTabIndex_: function() {
    this.tabindex = this.originalTabIndex_;
    this.originalTabIndex_ = null;
  },

  /**
   * Prevents clicking random spaces within cr-input but outside of <input>
   * from triggering focus.
   * @param {!Event} e
   * @private
   */
  onPointerDown_: function(e) {
    // Don't need to manipulate tabindex if cr-input is already disabled.
    if (this.disabled)
      return;

    // Should not mess with tabindex when <input> is clicked, otherwise <input>
    // will lose and regain focus, and replay the focus animation.
    if (e.path[0].tagName !== 'INPUT') {
      this.recordAndUnsetTabIndex_();
      setTimeout(() => {
        // Restore tabindex, unless disabled in the same cycle as pointerdown.
        if (!this.disabled)
          this.restoreTabIndex_();
      }, 0);
    }
  },

  /**
   * When shift-tab is pressed, first bring the focus to the host element.
   * This accomplishes 2 things:
   * 1) Host doesn't get focused when the browser moves the focus backward.
   * 2) focus now escaped the shadow-dom of this element, so that it'll
   *    correctly obey non-zero tabindex ordering of the containing document.
   * TODO(scottchen): check if we still need this after switching to Polymer 2.
   * @private
   */
  onInputKeydown_: function(e) {
    if (e.shiftKey && e.key === 'Tab')
      this.focus();
  },

  /** @private */
  onValueChanged_: function() {
    if (this.autoValidate)
      this.validate();
  },

  /**
   * 'change' event fires when <input> value changes and user presses 'Enter'.
   * This function helps propagate it to host since change events don't
   * propagate across Shadow DOM boundary by default.
   * @param {!Event} e
   * @private
   */
  onInputChange_: function(e) {
    this.fire('change', {sourceEvent: e});
  },

  /** @private */
  onInputFocus_: function() {
    this.focused_ = true;
  },

  /** @private */
  onInputBlur_: function() {
    this.focused_ = false;
  },

  /**
   * Selects the text within the input. If no parameters are passed, it will
   * select the entire string. Either no params or both params should be passed.
   * Publicly, this function should be used instead of inputElement.select() or
   * manipulating inputElement.selectionStart/selectionEnd because the order of
   * execution between focus() and select() is sensitive.
   * @param {number=} start
   * @param {number=} end
   */
  select: function(start, end) {
    this.focusInput_();
    if (start !== undefined && end !== undefined) {
      this.inputElement.setSelectionRange(start, end);
    } else {
      // Can't just pass one param.
      assert(start === undefined && end === undefined);
      this.inputElement.select();
    }
  },

  /** @return {boolean} */
  validate: function() {
    this.invalid = !this.inputElement.checkValidity();
    return !this.invalid;
  },
});