/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Virtual Keyboard module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtTest 1.0
import QtQuick 2.0
import QtQuick.VirtualKeyboard 2.3
import QtQuick.VirtualKeyboard.Settings 2.2
import "handwriting.js" as Handwriting
import "utils.js" as Utils

InputPanel {
    id: inputPanel
    z: 99
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    visible: active

    property var testcase
    property var virtualKeyPressPoint: null
    readonly property int cursorPosition: InputContext.cursorPosition
    readonly property string preeditText: InputContext.preeditText
    readonly property string surroundingText: InputContext.surroundingText
    readonly property bool autoCapitalizationEnabled: InputContext.priv.shiftHandler.autoCapitalizationEnabled
    readonly property bool toggleShiftEnabled: InputContext.priv.shiftHandler.toggleShiftEnabled
    readonly property string locale: keyboard.locale
    readonly property string defaultLocale: VirtualKeyboardSettings.locale
    readonly property var availableLocales: VirtualKeyboardSettings.availableLocales
    readonly property var activeLocales: VirtualKeyboardSettings.activeLocales
    readonly property int inputMode: InputContext.inputEngine.inputMode
    readonly property var inputMethod: InputContext.inputEngine.inputMethod
    readonly property var keyboard: Utils.findChildByProperty(inputPanel, "objectName", "keyboard", null)
    readonly property bool handwritingMode: keyboard.handwritingMode
    readonly property var keyboardLayoutLoader: Utils.findChildByProperty(keyboard, "objectName", "keyboardLayoutLoader", null)
    readonly property var keyboardInputArea: Utils.findChildByProperty(keyboard, "objectName", "keyboardInputArea", null)
    readonly property var characterPreviewBubble: Utils.findChildByProperty(keyboard, "objectName", "characterPreviewBubble", null)
    readonly property var alternativeKeys: Utils.findChildByProperty(keyboard, "objectName", "alternativeKeys", null)
    readonly property var naviationHighlight: Utils.findChildByProperty(keyboard, "objectName", "naviationHighlight", null)
    readonly property bool naviationHighlightAnimating: naviationHighlight.xAnimation.running ||
                                                        naviationHighlight.yAnimation.running ||
                                                        naviationHighlight.widthAnimation.running ||
                                                        naviationHighlight.heightAnimation.running
    readonly property var wordCandidateView: Utils.findChildByProperty(keyboard, "objectName", "wordCandidateView", null)
    readonly property var wordCandidateContextMenu: Utils.findChildByProperty(keyboard, "objectName", "wordCandidateContextMenu", null)
    readonly property var shadowInputControl: Utils.findChildByProperty(keyboard, "objectName", "shadowInputControl", null)
    readonly property var shadowInput: Utils.findChildByProperty(keyboard, "objectName", "shadowInput", null)
    readonly property var selectionControl: Utils.findChildByProperty(inputPanel, "objectName", "selectionControl", null)
    readonly property var anchorHandle: selectionControl.children[0]
    readonly property var cursorHandle: selectionControl.children[1]
    readonly property var fullScreenModeSelectionControl: Utils.findChildByProperty(inputPanel, "objectName", "fullScreenModeSelectionControl", null)
    readonly property var fullScreenModeAnchorHandle: fullScreenModeSelectionControl.children[0]
    readonly property var fullScreenModeCursorHandle: fullScreenModeSelectionControl.children[1]
    readonly property bool wordCandidateListVisibleHint: InputContext.inputEngine.wordCandidateListVisibleHint
    readonly property bool keyboardLayoutsAvailable: keyboard.availableLocaleIndices.length > 0 && keyboard.availableLocaleIndices.indexOf(-1) === -1
    property alias keyboardLayoutsAvailableSpy: keyboardLayoutsAvailableSpy
    property alias keyboardLayoutLoaderItemSpy: keyboardLayoutLoaderItemSpy
    property alias characterPreviewBubbleSpy: characterPreviewBubbleSpy
    property alias alternativeKeysSpy: alternativeKeysSpy
    property alias activeKeyChangedSpy: activeKeyChangedSpy
    property alias virtualKeyClickedSpy: virtualKeyClickedSpy
    property alias dragSymbolModeSpy: dragSymbolModeSpy
    property alias styleSpy: styleSpy
    property alias soundEffectSpy: soundEffectSpy
    property alias inputMethodResultSpy: inputMethodResultSpy
    property alias wordCandidateListChangedSpy: wordCandidateListChangedSpy
    property alias inputMethodSelectionListChangedSpy: inputMethodSelectionListChangedSpy
    property alias wordCandidateListVisibleSpy: wordCandidateListVisibleSpy
    property alias shiftStateSpy: shiftStateSpy
    property alias shadowInputControlVisibleSpy: shadowInputControlVisibleSpy
    property alias externalLanguageSwitchSpy: externalLanguageSwitchSpy

    signal inputMethodResult(var text)

    LayoutMirroring.childrenInherit: true

    Connections {
        target: InputContext
        onPreeditTextChanged: if (InputContext.preeditText.length > 0) inputMethodResult(InputContext.preeditText)
    }

    Connections {
        target: InputContext.inputEngine
        onVirtualKeyClicked: inputMethodResult(text)
    }

    SignalSpy {
        id: keyboardLayoutsAvailableSpy
        target: inputPanel
        signalName: "onKeyboardLayoutsAvailableChanged"
    }

    SignalSpy {
        id: keyboardLayoutLoaderItemSpy
        target: keyboardLayoutLoader
        signalName: "onItemChanged"
    }

    SignalSpy {
        id: characterPreviewBubbleSpy
        target: characterPreviewBubble
        signalName: "onVisibleChanged"
    }

    SignalSpy {
        id: alternativeKeysSpy
        target: alternativeKeys
        signalName: "onVisibleChanged"
    }

    SignalSpy {
        id: activeKeyChangedSpy
        target: InputContext.inputEngine
        signalName: "activeKeyChanged"
    }

    SignalSpy {
        id: virtualKeyClickedSpy
        target: InputContext.inputEngine
        signalName: "virtualKeyClicked"
    }

    SignalSpy {
        id: dragSymbolModeSpy
        target: keyboardInputArea
        signalName: "onDragSymbolModeChanged"
    }

    SignalSpy {
        id: styleSpy
        target: keyboard
        signalName: "onStyleChanged"
    }

    SignalSpy {
        id: soundEffectSpy
        target: keyboard.soundEffect
        signalName: "onPlayingChanged"
    }

    SignalSpy {
        id: inputMethodResultSpy
        target: inputPanel
        signalName: "inputMethodResult"
    }

    SignalSpy {
        id: wordCandidateListChangedSpy
        target: wordCandidateView.model
        signalName: "dataChanged"
    }

    SignalSpy {
        id: inputMethodSelectionListChangedSpy
        target: InputContext.inputEngine.inputMethod
        signalName: "selectionListChanged"
    }

    SignalSpy {
        id: wordCandidateListVisibleSpy
        target: wordCandidateView
        signalName: "onVisibleConditionChanged"
    }

    SignalSpy {
        id: wordCandidateContextMenuActiveSpy
        target: wordCandidateContextMenu
        signalName: "onActiveChanged"
    }

    SignalSpy {
        id: shiftStateSpy
        target: InputContext
        signalName: "onShiftActiveChanged"
    }

    SignalSpy {
        id: shadowInputControlVisibleSpy
        target: shadowInputControl
        signalName: "onVisibleChanged"
    }

    SignalSpy {
        id: externalLanguageSwitchSpy
        target: inputPanel
        signalName: "onExternalLanguageSwitch"
    }

    function findChildByProperty(parent, propertyName, propertyValue, compareCb) {
        var obj = null
        if (parent === null)
            return null
        var children = parent.children
        for (var i = 0; i < children.length; i++) {
            obj = children[i]
            if (obj.hasOwnProperty(propertyName)) {
                if (compareCb !== null) {
                    if (compareCb(obj[propertyName], propertyValue))
                        break
                } else if (obj[propertyName] === propertyValue) {
                    break
                }
            }
            obj = findChildByProperty(obj, propertyName, propertyValue, compareCb)
            if (obj)
                break
        }
        return obj
    }

    function isLocaleSupported(inputLocale) {
        var localeIndex = VirtualKeyboardSettings.availableLocales.indexOf(inputLocale)
        return localeIndex !== -1
    }

    function setLocale(inputLocale) {
        VirtualKeyboardSettings.locale = inputLocale
        if (["ar", "fa"].indexOf(inputLocale.substring(0, 2)) !== -1)
            return inputPanel.keyboard.locale.substring(0, 2) === inputLocale.substring(0, 2)
        return inputPanel.keyboard.locale === inputLocale
    }

    function setActiveLocales(activeLocales) {
        VirtualKeyboardSettings.activeLocales = activeLocales
    }

    function setWclAutoHideDelay(wclAutoHideDelay) {
        VirtualKeyboardSettings.wordCandidateList.autoHideDelay = wclAutoHideDelay
    }

    function setWclAlwaysVisible(wclAlwaysVisible) {
        VirtualKeyboardSettings.wordCandidateList.alwaysVisible = wclAlwaysVisible
    }

    function setWclAutoCommitWord(wclAutoCommitWord) {
        VirtualKeyboardSettings.wordCandidateList.autoCommitWord = wclAutoCommitWord
    }

    function setFullScreenMode(fullScreenMode) {
        VirtualKeyboardSettings.fullScreenMode = fullScreenMode
    }

    function mapInputMode(inputModeName) {
        if (inputModeName === "Latin")
            return InputEngine.InputMode.Latin
        else if (inputModeName === "Numeric")
            return InputEngine.InputMode.Numeric
        else if (inputModeName === "Dialable")
            return InputEngine.InputMode.Dialable
        else if (inputModeName === "Pinyin")
            return InputEngine.InputMode.Pinyin
        else if (inputModeName === "Cangjie")
            return InputEngine.InputMode.Cangjie
        else if (inputModeName === "Zhuyin")
            return InputEngine.InputMode.Zhuyin
        else if (inputModeName === "Hangul")
            return InputEngine.InputMode.Hangul
        else if (inputModeName === "Hiragana")
            return InputEngine.InputMode.Hiragana
        else if (inputModeName === "Katakana")
            return InputEngine.InputMode.Katakana
        else if (inputModeName === "FullwidthLatin")
            return InputEngine.InputMode.FullwidthLatin
        else if (inputModeName === "Greek")
            return InputEngine.InputMode.Greek
        else if (inputModeName === "Cyrillic")
            return InputEngine.InputMode.Cyrillic
        else if (inputModeName === "Arabic")
            return InputEngine.InputMode.Arabic
        else if (inputModeName === "Hebrew")
            return InputEngine.InputMode.Hebrew
        else if (inputModeName === "ChineseHandwriting")
            return InputEngine.InputMode.ChineseHandwriting
        else if (inputModeName === "JapaneseHandwriting")
            return InputEngine.InputMode.JapaneseHandwriting
        else if (inputModeName === "KoreanHandwriting")
            return InputEngine.InputMode.KoreanHandwriting
        else if (inputModeName === "Thai")
            return InputEngine.InputMode.Thai
        else
            return -1
    }

    function isInputModeSupported(inputMode) {
        return InputContext.inputEngine.inputModes.indexOf(inputMode) !== -1
    }

    function setInputMode(inputMode) {
        if (!isInputModeSupported(inputMode))
            return false
        if (InputContext.inputEngine.inputMode !== inputMode)
            InputContext.inputEngine.inputMode = inputMode
        return true
    }

    function testPublicEnums() {
        // Scoped
        testcase.verify(InputEngine.TextCase.Lower !== undefined)
        testcase.verify(InputEngine.TextCase.Upper !== undefined)
        testcase.verify(InputEngine.InputMode.Latin !== undefined)
        testcase.verify(InputEngine.InputMode.Numeric !== undefined)
        testcase.verify(InputEngine.InputMode.Dialable !== undefined)
        testcase.verify(InputEngine.InputMode.Pinyin !== undefined)
        testcase.verify(InputEngine.InputMode.Cangjie !== undefined)
        testcase.verify(InputEngine.InputMode.Zhuyin !== undefined)
        testcase.verify(InputEngine.InputMode.Hangul !== undefined)
        testcase.verify(InputEngine.InputMode.Hiragana !== undefined)
        testcase.verify(InputEngine.InputMode.Katakana !== undefined)
        testcase.verify(InputEngine.InputMode.FullwidthLatin !== undefined)
        testcase.verify(InputEngine.InputMode.Greek !== undefined)
        testcase.verify(InputEngine.InputMode.Cyrillic !== undefined)
        testcase.verify(InputEngine.InputMode.Arabic !== undefined)
        testcase.verify(InputEngine.InputMode.Hebrew !== undefined)
        testcase.verify(InputEngine.InputMode.ChineseHandwriting !== undefined)
        testcase.verify(InputEngine.InputMode.JapaneseHandwriting !== undefined)
        testcase.verify(InputEngine.InputMode.KoreanHandwriting !== undefined)
        testcase.verify(InputEngine.InputMode.Thai !== undefined)
        testcase.verify(InputEngine.PatternRecognitionMode.None !== undefined)
        testcase.verify(InputEngine.PatternRecognitionMode.PatternRecognitionDisabled !== undefined)
        testcase.verify(InputEngine.PatternRecognitionMode.Handwriting !== undefined)
        testcase.verify(InputEngine.PatternRecognitionMode.HandwritingRecoginition !== undefined)
        testcase.verify(InputEngine.ReselectFlag.WordBeforeCursor !== undefined)
        testcase.verify(InputEngine.ReselectFlag.WordAfterCursor !== undefined)
        testcase.verify(InputEngine.ReselectFlag.WordAtCursor !== undefined)
        testcase.verify(SelectionListModel.Type.WordCandidateList !== undefined)
        testcase.verify(SelectionListModel.Role.Display !== undefined)
        testcase.verify(SelectionListModel.Role.WordCompletionLength !== undefined)
        testcase.verify(SelectionListModel.Role.Dictionary !== undefined)
        testcase.verify(SelectionListModel.DictionaryType.Default !== undefined)
        testcase.verify(SelectionListModel.DictionaryType.User !== undefined)
        // Unscoped
        testcase.verify(InputEngine.Lower !== undefined)
        testcase.verify(InputEngine.Upper !== undefined)
        testcase.verify(InputEngine.Latin !== undefined)
        testcase.verify(InputEngine.Numeric !== undefined)
        testcase.verify(InputEngine.Dialable !== undefined)
        testcase.verify(InputEngine.Pinyin !== undefined)
        testcase.verify(InputEngine.Cangjie !== undefined)
        testcase.verify(InputEngine.Zhuyin !== undefined)
        testcase.verify(InputEngine.Hangul !== undefined)
        testcase.verify(InputEngine.Hiragana !== undefined)
        testcase.verify(InputEngine.Katakana !== undefined)
        testcase.verify(InputEngine.FullwidthLatin !== undefined)
        testcase.verify(InputEngine.Greek !== undefined)
        testcase.verify(InputEngine.Cyrillic !== undefined)
        testcase.verify(InputEngine.Arabic !== undefined)
        testcase.verify(InputEngine.Hebrew !== undefined)
        testcase.verify(InputEngine.ChineseHandwriting !== undefined)
        testcase.verify(InputEngine.JapaneseHandwriting !== undefined)
        testcase.verify(InputEngine.KoreanHandwriting !== undefined)
        testcase.verify(InputEngine.Thai !== undefined)
        testcase.verify(InputEngine.None !== undefined)
        testcase.verify(InputEngine.PatternRecognitionDisabled !== undefined)
        testcase.verify(InputEngine.Handwriting !== undefined)
        testcase.verify(InputEngine.HandwritingRecoginition !== undefined)
        testcase.verify(InputEngine.WordBeforeCursor !== undefined)
        testcase.verify(InputEngine.WordAfterCursor !== undefined)
        testcase.verify(InputEngine.WordAtCursor !== undefined)
        testcase.verify(SelectionListModel.WordCandidateList !== undefined)
        testcase.verify(SelectionListModel.DisplayRole !== undefined)
        testcase.verify(SelectionListModel.WordCompletionLengthRole !== undefined)
        testcase.verify(SelectionListModel.DictionaryType !== undefined)
        testcase.verify(SelectionListModel.Default !== undefined)
        testcase.verify(SelectionListModel.User !== undefined)
    }

    function setExternalLanguageSwitchEnabled(enabled) {
        externalLanguageSwitchEnabled = enabled
    }

    function setLayoutMirroring(enabled) {
        LayoutMirroring.enabled = enabled
    }

    function findVirtualKey(key) {
        return Utils.findChild(keyboardLayoutLoader, key, function(obj, param) {
            if (!obj.hasOwnProperty("key") || !obj.hasOwnProperty("text"))
                return false
            return (typeof param == "number") ? obj.key === param : obj.text.toUpperCase() === param.toUpperCase()
        })
    }

    function findVirtualKeyAlternative(key) {
        if (typeof key != "string")
            return null
        return Utils.findChildByProperty(keyboardLayoutLoader, "effectiveAlternativeKeys", key.toUpperCase(),
                 function(propertyValue, key) {
                     if (typeof propertyValue == "string")
                         return propertyValue.toUpperCase().indexOf(key) !== -1
                     return propertyValue.filter(function(value) {
                         return key === value.toUpperCase()
                     }).length > 0
                 })
    }

    function findObjectByName(objectName) {
        return Utils.findChildByProperty(keyboard, "objectName", objectName, null)
    }

    function virtualKeyPressOnCurrentLayout(key) {
        var keyObj = typeof key == "object" && key.hasOwnProperty("key") ? key : findVirtualKey(key)
        var alternativeKey = false
        if (!keyObj && typeof key == "string") {
            keyObj = findVirtualKeyAlternative(key)
            alternativeKey = keyObj !== null
            if (alternativeKey)
                alternativeKeysSpy.clear()
        }
        if (keyObj) {
            virtualKeyPressPoint = inputPanel.mapFromItem(keyObj, keyObj.width / 2, keyObj.height / 2)
            testcase.mousePress(inputPanel, virtualKeyPressPoint.x, virtualKeyPressPoint.y)
            testcase.wait(20)
            if (alternativeKey) {
                alternativeKeysSpy.wait()
                var keyIndex = keyObj.effectiveAlternativeKeys.indexOf(key.toLowerCase())
                var itemX = keyIndex * keyboard.style.alternateKeysListItemWidth + keyboard.style.alternateKeysListItemWidth / 2
                virtualKeyPressPoint.x = inputPanel.mapFromItem(alternativeKeys.listView, itemX, 0).x
                testcase.mouseMove(inputPanel, virtualKeyPressPoint.x, virtualKeyPressPoint.y)
                testcase.waitForRendering(inputPanel)
            }
            return true
        }
        return false
    }

    function multiLayoutKeyActionHelper(key, keyActionOnCurrentLayoutCb) {
        if (!keyboardLayoutLoader.item) {
            console.warn("Key not found \\u%1 (keyboard layout not loaded)".arg(key.charCodeAt(0).toString(16)))
            return false
        }
        var success = keyActionOnCurrentLayoutCb(key)
        for (var c = 0; !success && c < 2; c++) {
            // Check if the current layout contains multiple layouts
            if (keyboardLayoutLoader.item.hasOwnProperty("item")) {
                if (keyboardLayoutLoader.item.hasOwnProperty("secondPage")) {
                    keyboardLayoutLoader.item.secondPage = !keyboardLayoutLoader.item.secondPage
                    testcase.waitForRendering(inputPanel)
                    success = keyActionOnCurrentLayoutCb(key)
                } else if (keyboardLayoutLoader.item.hasOwnProperty("page") && keyboardLayoutLoader.item.hasOwnProperty("numPages")) {
                    for (var page = 0; !success && page < keyboardLayoutLoader.item.numPages; page++) {
                        keyboardLayoutLoader.item.page = page
                        testcase.waitForRendering(inputPanel)
                        success = keyActionOnCurrentLayoutCb(key)
                    }
                } else if (typeof key != "number" || key !== Qt.Key_Shift) {
                    // Some layouts (such as Arabic, Hindi) may have a second layout
                    virtualKeyClick(Qt.Key_Shift)
                    InputContext.priv.shiftHandler.clearToggleShiftTimer()
                    testcase.waitForRendering(inputPanel)
                    success = keyActionOnCurrentLayoutCb(key)
                    if (!success) {
                        virtualKeyClick(Qt.Key_Shift)
                        InputContext.priv.shiftHandler.clearToggleShiftTimer()
                        testcase.waitForRendering(inputPanel)
                    }
                }
                if (success)
                    break
            }

            // Symbol mode not allowed in handwriting mode
            if (inputPanel.handwritingMode)
                break

            // Toggle symbol mode
            keyboard.symbolMode = !keyboard.symbolMode
            testcase.waitForRendering(inputPanel)
            success = keyActionOnCurrentLayoutCb(key)
        }
        if (!success)
            console.warn("Key not found \\u%1".arg(key.charCodeAt(0).toString(16)))
        return success
    }

    function virtualKeyPress(key) {
        if (!key)
            return false
        return multiLayoutKeyActionHelper(key, virtualKeyPressOnCurrentLayout)
    }

    function virtualKeyDrag(key) {
        if (virtualKeyPressPoint !== null) {
            var keyObj = Utils.findChildByProperty(keyboardLayoutLoader, (typeof key == "number") ? "key" : "text", key, null)
            if (keyObj !== null) {
                virtualKeyPressPoint = inputPanel.mapFromItem(keyObj, keyObj.width / 2, keyObj.height / 2)
                testcase.mouseMove(inputPanel, virtualKeyPressPoint.x, virtualKeyPressPoint.y)
                testcase.waitForRendering(inputPanel)
                return true
            }
        }
        return false
    }

    function virtualKeyRelease() {
        if (virtualKeyPressPoint !== null) {
            testcase.mouseRelease(inputPanel, virtualKeyPressPoint.x, virtualKeyPressPoint.y)
            virtualKeyPressPoint = null
            return true
        }
        return false
    }

    function virtualKeyClick(key) {
        if (virtualKeyPress(key)) {
            virtualKeyRelease()
            testcase.waitForRendering(inputPanel)
            return true
        }
        return false
    }

    function emulateNavigationKeyClick(navigationKey) {
        testcase.keyClick(navigationKey)
        while (inputPanel.naviationHighlightAnimating)
            testcase.wait(inputPanel.naviationHighlight.moveDuration / 2)
    }

    function navigationHighlightContains(point) {
        var navigationPoint = inputPanel.mapToItem(inputPanel.naviationHighlight, point.x, point.y)
        return inputPanel.naviationHighlight.contains(Qt.point(navigationPoint.x, navigationPoint.y))
    }

    function navigateToKeyOnPoint(point) {
        activateNavigationKeyMode()
        if (inputPanel.naviationHighlight.visible) {
            while (true) {
                var navigationPoint = inputPanel.mapToItem(inputPanel.naviationHighlight, point.x, point.y)
                if (navigationHighlightContains(point))
                    return true
                if (inputPanel.naviationHighlight.y > point.y)
                    emulateNavigationKeyClick(Qt.Key_Up)
                else if (inputPanel.naviationHighlight.y + inputPanel.naviationHighlight.height < point.y)
                    emulateNavigationKeyClick(Qt.Key_Down)
                else if (inputPanel.naviationHighlight.x > point.x)
                    emulateNavigationKeyClick(Qt.Key_Left)
                else if (inputPanel.naviationHighlight.x + inputPanel.naviationHighlight.width < point.x)
                    emulateNavigationKeyClick(Qt.Key_Right)
            }
        }
        return false
    }

    function navigateToKeyOnCurrentLayout(key) {
        var keyObj = findVirtualKey(key)
        var alternativeKey = false
        if (!keyObj && typeof key == "string") {
            keyObj = findVirtualKeyAlternative(key)
            alternativeKey = keyObj !== null
        }
        if (keyObj) {
            var point = inputPanel.mapFromItem(keyObj, keyObj.width / 2, keyObj.height / 2)
            if (!navigateToKeyOnPoint(point))
                return false
            if (alternativeKey) {
                alternativeKeysSpy.clear()
                testcase.keyPress(Qt.Key_Return)
                alternativeKeysSpy.wait()
                testcase.keyRelease(Qt.Key_Return)
                var keyIndex = keyObj.effectiveAlternativeKeys.indexOf(key)
                while (inputPanel.alternativeKeys.listView.currentIndex !== keyIndex) {
                    testcase.verify(inputPanel.alternativeKeys.listView.currentIndex !== -1)
                    emulateNavigationKeyClick(inputPanel.alternativeKeys.listView.currentIndex < keyIndex ? Qt.Key_Right : Qt.Key_Left)
                }
            }
            return true
        }
        return false
    }

    function navigationKeyClickOnCurrentLayout(key) {
        if (navigateToKeyOnCurrentLayout(key)) {
            testcase.keyClick(Qt.Key_Return)
            return true
        }
        return false
    }

    function navigateToKey(key) {
        return multiLayoutKeyActionHelper(key, navigateToKeyOnCurrentLayout)
    }

    function navigationKeyClick(key) {
        return multiLayoutKeyActionHelper(key, navigationKeyClickOnCurrentLayout)
    }

    function activateNavigationKeyMode() {
        if (!inputPanel.naviationHighlight.visible) {
            inputPanel.naviationHighlight.moveDuration = 0
            inputPanel.naviationHighlight.resizeDuration = 0
            emulateNavigationKeyClick(Qt.Key_Right)
            if (inputPanel.naviationHighlight.visible) {
                while (inputPanel.naviationHighlightAnimating)
                    testcase.wait(inputPanel.naviationHighlight.moveDuration / 2)
            }
        }
        return inputPanel.naviationHighlight.visible
    }

    function toggleShift() {
        InputContext.priv.shiftHandler.toggleShift()
    }

    function setShiftActive(shiftActive) {
        InputContext.priv.shiftHandler.shiftActive = shiftActive
    }

    function setCapsLockActive(capsLockActive) {
        InputContext.priv.shiftHandler.capsLockActive = capsLockActive
    }

    function style() {
        return VirtualKeyboardSettings.styleName
    }

    function setStyle(style) {
        VirtualKeyboardSettings.styleName = style
    }

    function selectionListSearchSuggestion(suggestion) {
        if (wordCandidateListVisibleHint === false)
            return false

        if (inputPanel.wordCandidateView.count === 0)
            return false;

        var suggestionFound = false
        var origIndex = inputPanel.wordCandidateView.currentIndex
        if (origIndex === -1) {
            inputPanel.wordCandidateView.incrementCurrentIndex()
            origIndex = inputPanel.wordCandidateView.currentIndex
        }
        if (origIndex !== -1) {
            while (true) {
                if (inputPanel.wordCandidateView.model.dataAt(inputPanel.wordCandidateView.currentIndex) === suggestion) {
                    suggestionFound = true
                    break
                }
                if (inputPanel.wordCandidateView.currentIndex === inputPanel.wordCandidateView.count - 1)
                    break
                inputPanel.wordCandidateView.incrementCurrentIndex()
            }
            if (!suggestionFound) {
                while (inputPanel.wordCandidateView.currentIndex !== origIndex) {
                    inputPanel.wordCandidateView.decrementCurrentIndex()
                }
            }
            testcase.waitForRendering(inputPanel)
        }
        return suggestionFound
    }

    function selectionListSelectCurrentItem() {
        if (!inputPanel.wordCandidateView.currentItem)
            return false
        testcase.wait(200)
        testcase.verify(inputPanel.wordCandidateView.currentItem,
            "Expected wordCandidateView to have a currentItem, but it's null."
            + " Its property values at the time of failure are:"
            + " x=" + inputPanel.wordCandidateView.x
            + " y=" + inputPanel.wordCandidateView.y
            + " width=" + inputPanel.wordCandidateView.width
            + " height=" + inputPanel.wordCandidateView.height
            + " count=" + inputPanel.wordCandidateView.count
            + " flicking=" + inputPanel.wordCandidateView.flicking
            + " moving=" + inputPanel.wordCandidateView.moving
            + " visible=" + inputPanel.wordCandidateView.visible)
        var itemPos = inputPanel.mapFromItem(inputPanel.wordCandidateView.currentItem,
                                             inputPanel.wordCandidateView.currentItem.width / 2,
                                             inputPanel.wordCandidateView.currentItem.height / 2)
        testcase.mouseClick(inputPanel, itemPos.x, itemPos.y, Qt.LeftButton, 0, 20)
        testcase.waitForRendering(inputPanel)
        return true
    }

    function selectionListCurrentIndex() {
        return inputPanel.wordCandidateView.currentIndex
    }

    function selectionListSuggestionIsFromUserDictionary() {
        if (!inputPanel.wordCandidateView.currentItem)
            return false
        var dictionaryType = inputPanel.wordCandidateView.model.dataAt(inputPanel.wordCandidateView.currentIndex, SelectionListModel.Role.Dictionary)
        return dictionaryType !== undefined && dictionaryType === SelectionListModel.DictionaryType.User
    }

    function openWordCandidateContextMenu() {
        if (!inputPanel.wordCandidateView.currentItem)
            return false
        testcase.wait(200)
        wordCandidateContextMenuActiveSpy.clear()
        testcase.mousePress(inputPanel.wordCandidateView.currentItem)
        wordCandidateContextMenuActiveSpy.wait()
        testcase.mouseRelease(inputPanel.wordCandidateView.currentItem)
        return wordCandidateContextMenu.active
    }

    function selectItemFromWordCandidateContextMenu(index) {
        if (!inputPanel.wordCandidateView.currentItem)
            return false
        if (!wordCandidateContextMenu.active)
            return false
        var wordCandidateContextMenuList = Utils.findChildByProperty(keyboard, "objectName", "wordCandidateContextMenuList", null)
        if (wordCandidateContextMenuList.currentIndex !== index) {
            wordCandidateContextMenuList.currentIndex = index
            testcase.waitForRendering(inputPanel)
        }
        if (!wordCandidateContextMenuList.currentItem)
            return false
        var itemPos = inputPanel.mapFromItem(wordCandidateContextMenuList.currentItem,
                                             wordCandidateContextMenuList.currentItem.width / 2,
                                             wordCandidateContextMenuList.currentItem.height / 2)
        testcase.mouseClick(inputPanel, itemPos.x, itemPos.y, Qt.LeftButton, 0, 20)
        testcase.waitForRendering(inputPanel)
        return true
    }

    function setHandwritingMode(enabled) {
        if (inputPanel.keyboard.handwritingMode !== enabled) {
            if (!enabled || inputPanel.keyboard.isHandwritingAvailable())
                inputPanel.keyboard.setHandwritingMode(enabled, true)
        }
        return inputPanel.keyboard.handwritingMode === enabled
    }

    function emulateHandwriting(ch, instant) {
        if (!inputPanel.keyboard.handwritingMode)
            return false
        var hwrInputArea = Utils.findChildByProperty(keyboard, "objectName", "hwrInputArea", null)
        inputMethodResultSpy.clear()
        if (!Handwriting.emulate(testcase, hwrInputArea, ch, instant,
                                 VirtualKeyboardSettings.locale)) {
            if (virtualKeyClick(ch))
                return true
            console.warn("Cannot produce the symbol '%1' in handwriting mode".arg(ch))
            return false
        }
        if (isSuperimposedHandwriting())
            return true
        inputMethodResultSpy.wait(3000)
        return inputMethodResultSpy.count > 0
    }

    function isSuperimposedHandwriting() {
        if (!inputPanel.keyboard.handwritingMode)
            return false
        return inputMethod != null && inputMethod.hasOwnProperty("superimposed") && inputMethod.superimposed
    }
}
