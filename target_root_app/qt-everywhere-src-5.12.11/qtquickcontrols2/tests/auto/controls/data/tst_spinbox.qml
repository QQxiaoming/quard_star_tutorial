/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.12
import QtTest 1.0
import QtQuick.Controls 2.12
import QtQuick.Window 2.12

TestCase {
    id: testCase
    width: 400
    height: 400
    visible: true
    when: windowShown
    name: "SpinBox"

    Component {
        id: signalSpy
        SignalSpy { }
    }

    Component {
        id: spinBox
        SpinBox { }
    }

    Component {
        id: mouseArea
        MouseArea { }
    }

    function test_defaults() {
        var control = createTemporaryObject(spinBox, testCase)
        verify(control)

        compare(control.from, 0)
        compare(control.to, 99)
        compare(control.value, 0)
        compare(control.stepSize, 1)
        compare(control.editable, false)
        compare(control.up.pressed, false)
        compare(control.up.indicator.enabled, true)
        compare(control.down.pressed, false)
        compare(control.down.indicator.enabled, false)
    }

    function test_value() {
        var control = createTemporaryObject(spinBox, testCase)
        verify(control)

        compare(control.value, 0)
        control.value = 50
        compare(control.value, 50)
        control.value = 99
        compare(control.value, 99)
        control.value = -99
        compare(control.value, 0)
        control.value = 100
        compare(control.value, 99)
    }

    function test_range() {
        var control = createTemporaryObject(spinBox, testCase, {from: 0, to: 100, value: 50})
        verify(control)

        compare(control.from, 0)
        compare(control.to, 100)
        compare(control.value, 50)
        compare(control.up.indicator.enabled, true)
        compare(control.down.indicator.enabled, true)

        control.value = 1000
        compare(control.value, 100)
        compare(control.up.indicator.enabled, false)
        compare(control.down.indicator.enabled, true)

        control.wrap = true
        compare(control.up.indicator.enabled, true)
        compare(control.down.indicator.enabled, true)

        control.value = -1
        compare(control.value, 0)
        compare(control.up.indicator.enabled, true)
        compare(control.down.indicator.enabled, true)

        control.from = 25
        compare(control.from, 25)
        compare(control.value, 25)
        compare(control.up.indicator.enabled, true)
        compare(control.down.indicator.enabled, true)

        control.wrap = false
        compare(control.up.indicator.enabled, true)
        compare(control.down.indicator.enabled, false)

        control.value = 30
        compare(control.from, 25)
        compare(control.value, 30)
        compare(control.up.indicator.enabled, true)
        compare(control.down.indicator.enabled, true)

        control.from = 30
        compare(control.from, 30)
        compare(control.value, 30)
        compare(control.up.indicator.enabled, true)
        compare(control.down.indicator.enabled, false)

        control.to = 75
        compare(control.to, 75)
        compare(control.value, 30)
        compare(control.up.indicator.enabled, true)
        compare(control.down.indicator.enabled, false)

        control.value = 50
        compare(control.value, 50)
        compare(control.up.indicator.enabled, true)
        compare(control.down.indicator.enabled, true)

        control.to = 50
        compare(control.to, 50)
        compare(control.value, 50)
        compare(control.up.indicator.enabled, false)
        compare(control.down.indicator.enabled, true)

        control.to = 40
        compare(control.to, 40)
        compare(control.value, 40)
        compare(control.up.indicator.enabled, false)
        compare(control.down.indicator.enabled, true)
    }

    function test_inverted() {
        var control = createTemporaryObject(spinBox, testCase, {from: 100, to: -100})
        verify(control)

        compare(control.from, 100)
        compare(control.to, -100)
        compare(control.value, 0)
        compare(control.up.indicator.enabled, true)
        compare(control.down.indicator.enabled, true)

        control.value = 200
        compare(control.value, 100)
        compare(control.up.indicator.enabled, true)
        compare(control.down.indicator.enabled, false)

        control.value = -200
        compare(control.value, -100)
        compare(control.up.indicator.enabled, false)
        compare(control.down.indicator.enabled, true)

        control.value = 0
        compare(control.value, 0)
        compare(control.up.indicator.enabled, true)
        compare(control.down.indicator.enabled, true)
    }

    function test_mouse_data() {
        return [
            { tag: "up", button: "up", value: 50, enabled: true, hold: false, modified: 1, expected: 51 },
            { tag: "down", button: "down", value: 50, enabled: true, hold: false, modified: 1, expected: 49 },
            { tag: "up:disabled", button: "up", value: 99, enabled: false, hold: false, modified: 0, expected: 99 },
            { tag: "down:disabled", button: "down", value: 0, enabled: false, hold: false, modified: 0, expected: 0 },
            { tag: "up:hold", button: "up", value: 95, enabled: true, hold: true, modified: 4, expected: 99 },
            { tag: "down:hold", button: "down", value: 5, enabled: true, hold: true, modified: 5, expected: 0 }
        ]
    }

    function test_mouse(data) {
        var control = createTemporaryObject(spinBox, testCase, {value: data.value})
        verify(control)

        var button = control[data.button]
        verify(button)

        var pressedSpy = signalSpy.createObject(control, {target: button, signalName: "pressedChanged"})
        verify(pressedSpy.valid)

        var valueModifiedSpy = signalSpy.createObject(control, {target: control, signalName: "valueModified"})
        verify(valueModifiedSpy.valid)

        mousePress(button.indicator)
        compare(pressedSpy.count, data.enabled ? 1 : 0)
        compare(button.pressed, data.enabled)
        compare(control.value, data.value)
        compare(valueModifiedSpy.count, 0)

        if (data.hold)
            tryCompare(control, "value", data.expected)

        mouseRelease(button.indicator)
        compare(pressedSpy.count, data.enabled ? 2 : 0)
        compare(button.pressed, false)
        compare(control.value, data.expected)
        compare(valueModifiedSpy.count, data.modified)
    }

    function test_keys_data() {
        return [
            { tag: "1", from: 1, to: 10, value: 1, stepSize: 1, upSteps: [2,3,4], downSteps: [3,2,1,1] },
            { tag: "2", from: 1, to: 10, value: 10, stepSize: 2, upSteps: [10,10], downSteps: [8,6,4] },
            { tag: "25", from: 0, to: 100, value: 50, stepSize: 25, upSteps: [75,100,100], downSteps: [75,50,25,0,0] },
            { tag: "wrap1", wrap: true, from: 1, to: 10, value: 1, stepSize: 1, upSteps: [2,3], downSteps: [2,1,10,9] },
            { tag: "wrap2", wrap: true, from: 1, to: 10, value: 10, stepSize: 2, upSteps: [1,3,5], downSteps: [3,1,10,8,6] },
            { tag: "wrap25", wrap: true, from: 0, to: 100, value: 50, stepSize: 25, upSteps: [75,100,0,25], downSteps: [0,100,75] }
        ]
    }

    function test_keys(data) {
        var control = createTemporaryObject(spinBox, testCase, {wrap: data.wrap, from: data.from, to: data.to, value: data.value, stepSize: data.stepSize})
        verify(control)

        var upPressedCount = 0
        var downPressedCount = 0
        var valueModifiedCount = 0

        var upPressedSpy = signalSpy.createObject(control, {target: control.up, signalName: "pressedChanged"})
        verify(upPressedSpy.valid)

        var downPressedSpy = signalSpy.createObject(control, {target: control.down, signalName: "pressedChanged"})
        verify(downPressedSpy.valid)

        var valueModifiedSpy = signalSpy.createObject(control, {target: control, signalName: "valueModified"})
        verify(valueModifiedSpy.valid)

        control.forceActiveFocus()
        verify(control.activeFocus)

        for (var u = 0; u < data.upSteps.length; ++u) {
            var wasUpEnabled = control.wrap || control.value < control.to
            keyPress(Qt.Key_Up)
            compare(control.up.pressed, wasUpEnabled)
            compare(control.down.pressed, false)
            if (wasUpEnabled) {
                ++upPressedCount
                ++valueModifiedCount
            }
            compare(upPressedSpy.count, upPressedCount)
            compare(valueModifiedSpy.count, valueModifiedCount)

            compare(control.value, data.upSteps[u])

            keyRelease(Qt.Key_Up)
            compare(control.down.pressed, false)
            compare(control.up.pressed, false)
            if (wasUpEnabled)
                ++upPressedCount
            compare(upPressedSpy.count, upPressedCount)
            compare(valueModifiedSpy.count, valueModifiedCount)
        }

        for (var d = 0; d < data.downSteps.length; ++d) {
            var wasDownEnabled = control.wrap || control.value > control.from
            keyPress(Qt.Key_Down)
            compare(control.down.pressed, wasDownEnabled)
            compare(control.up.pressed, false)
            if (wasDownEnabled) {
                ++downPressedCount
                ++valueModifiedCount
            }
            compare(downPressedSpy.count, downPressedCount)
            compare(valueModifiedSpy.count, valueModifiedCount)

            compare(control.value, data.downSteps[d])

            keyRelease(Qt.Key_Down)
            compare(control.down.pressed, false)
            compare(control.up.pressed, false)
            if (wasDownEnabled)
                ++downPressedCount
            compare(downPressedSpy.count, downPressedCount)
            compare(valueModifiedSpy.count, valueModifiedCount)
        }
    }

    function test_locale() {
        var control = createTemporaryObject(spinBox, testCase)
        verify(control)

        control.locale = Qt.locale("ar_EG") // Arabic, Egypt

        var numbers = ["٠", "١", "٢", "٣", "٤", "٥", "٦", "٧", "٨", "٩"]
        for (var i = 0; i < 10; ++i) {
            control.value = i
            compare(control.contentItem.text, numbers[i])
        }
    }

    function test_baseline() {
        var control = createTemporaryObject(spinBox, testCase)
        verify(control)
        compare(control.baselineOffset, control.contentItem.y + control.contentItem.baselineOffset)
    }

    function test_focus() {
        var control = createTemporaryObject(spinBox, testCase, {from: 10, to: 1000, value: 100, focus: true})
        verify(control)

        control.forceActiveFocus()
        compare(control.activeFocus, true)

        compare(control.from, 10)
        compare(control.to, 1000)
        compare(control.value, 100)

        control.focus = false
        compare(control.activeFocus, false)

        compare(control.from, 10)
        compare(control.to, 1000)
        compare(control.value, 100)
    }

    function test_initialFocus() {
        var window = testCase.Window.window
        verify(window)
        compare(window.activeFocusItem, window.contentItem)

        var control = createTemporaryObject(spinBox, testCase, { editable: true, focus: true })
        verify(control)
        tryCompare(control.contentItem, "activeFocus", true)
    }

    function test_editable() {
        var control = createTemporaryObject(spinBox, testCase)
        verify(control)

        var valueModifiedSpy = signalSpy.createObject(control, {target: control, signalName: "valueModified"})
        verify(valueModifiedSpy.valid)

        control.contentItem.forceActiveFocus()
        compare(control.contentItem.activeFocus, true)

        compare(control.editable, false)
        control.contentItem.selectAll()
        keyClick(Qt.Key_5)
        keyClick(Qt.Key_Return)
        compare(control.value, 0)
        compare(valueModifiedSpy.count, 0)

        control.editable = true
        compare(control.editable, true)
        control.contentItem.selectAll()
        keyClick(Qt.Key_5)
        keyClick(Qt.Key_Return)
        compare(control.value, 5)
        compare(valueModifiedSpy.count, 1)
    }

    function test_wheel_data() {
        return [
            { tag: "1", from: 1, to: 10, value: 1, stepSize: 1, upSteps: [2,3,4], downSteps: [3,2,1,1] },
            { tag: "2", from: 1, to: 10, value: 10, stepSize: 2, upSteps: [10,10], downSteps: [8,6,4] },
            { tag: "25", from: 0, to: 100, value: 50, stepSize: 25, upSteps: [75,100,100], downSteps: [75,50,25,0,0] },
            { tag: "wrap1", wrap: true, from: 1, to: 10, value: 1, stepSize: 1, upSteps: [2,3], downSteps: [2,1,10,9] },
            { tag: "wrap2", wrap: true, from: 1, to: 10, value: 10, stepSize: 2, upSteps: [1,3,5], downSteps: [3,1,10,8,6] },
            { tag: "wrap25", wrap: true, from: 0, to: 100, value: 50, stepSize: 25, upSteps: [75,100,0,25], downSteps: [0,100,75] }
        ]
    }

    function test_wheel(data) {
        var ma = createTemporaryObject(mouseArea, testCase, {width: 100, height: 100})
        verify(ma)

        var control = spinBox.createObject(ma, {wrap: data.wrap, from: data.from, to: data.to, value: data.value, stepSize: data.stepSize, wheelEnabled: true})
        verify(control)

        var valueModifiedCount = 0
        var valueModifiedSpy = signalSpy.createObject(control, {target: control, signalName: "valueModified"})
        verify(valueModifiedSpy.valid)

        var delta = 120

        var spy = signalSpy.createObject(ma, {target: ma, signalName: "wheel"})
        verify(spy.valid)

        for (var u = 0; u < data.upSteps.length; ++u) {
            var wasUpEnabled = control.wrap || control.value < control.to
            mouseWheel(control, control.width / 2, control.height / 2, delta, delta)
            if (wasUpEnabled)
                ++valueModifiedCount
            compare(valueModifiedSpy.count, valueModifiedCount)
            compare(spy.count, 0) // no propagation
            compare(control.value, data.upSteps[u])
        }

        for (var d = 0; d < data.downSteps.length; ++d) {
            var wasDownEnabled = control.wrap || control.value > control.from
            mouseWheel(control, control.width / 2, control.height / 2, -delta, -delta)
            if (wasDownEnabled)
                ++valueModifiedCount
            compare(valueModifiedSpy.count, valueModifiedCount)
            compare(spy.count, 0) // no propagation
            compare(control.value, data.downSteps[d])
        }
    }

    function test_initiallyDisabledIndicators_data() {
        return [
            { tag: "down disabled", from: 0, value: 0, to: 99, upEnabled: true, downEnabled: false },
            { tag: "up disabled", from: 0, value: 99, to: 99, upEnabled: false, downEnabled: true },
            { tag: "inverted, down disabled", from: 99, value: 99, to: 0, upEnabled: true, downEnabled: false },
            { tag: "inverted, up disabled", from: 99, value: 0, to: 0, upEnabled: false, downEnabled: true }
        ]
    }

    function test_initiallyDisabledIndicators(data) {
        var control = createTemporaryObject(spinBox, testCase, { from: data.from, value: data.value, to: data.to })
        verify(control)

        compare(control.up.indicator.enabled, data.upEnabled)
        compare(control.down.indicator.enabled, data.downEnabled)
    }

    function test_hover_data() {
        return [
            { tag: "up:true", button: "up", hoverEnabled: true, value: 50 },
            { tag: "up:false", button: "up", hoverEnabled: false, value: 50 },
            { tag: "up:max", button: "up", hoverEnabled: true, value: 99 },
            { tag: "down:true", button: "down", hoverEnabled: true, value: 50 },
            { tag: "down:false", button: "down", hoverEnabled: false, value: 50 },
            { tag: "down:min", button: "down", hoverEnabled: true, value: 0 }
        ]
    }

    function test_hover(data) {
        var control = createTemporaryObject(spinBox, testCase, {hoverEnabled: data.hoverEnabled, value: data.value})
        verify(control)

        var button = control[data.button]
        compare(control.hovered, false)
        compare(button.hovered, false)

        mouseMove(control, button.indicator.x + button.indicator.width / 2, button.indicator.y + button.indicator.height / 2)
        compare(control.hovered, data.hoverEnabled)
        compare(button.hovered, data.hoverEnabled && button.indicator.enabled)

        mouseMove(control, button.indicator.x - 1, button.indicator.y - 1)
        compare(button.hovered, false)
    }

    function test_hoverWhilePressed_data() {
        return [
            { tag: "up" },
            { tag: "down" },
        ]
    }

    // QTBUG-74688
    function test_hoverWhilePressed(data) {
        var control = createTemporaryObject(spinBox, testCase, { hoverEnabled: true, value: 50 })
        verify(control)

        var button = control[data.tag]
        compare(control.hovered, false)
        compare(button.hovered, false)

        // Hover over the indicator. It should be hovered.
        var buttonXCenter = button.indicator.x + button.indicator.width / 2
        var buttonYCenter = button.indicator.y + button.indicator.height / 2
        mouseMove(control, buttonXCenter, buttonYCenter)
        compare(button.hovered, true)

        // Press on the indicator and then move the mouse outside of it.
        mousePress(control, buttonXCenter, buttonYCenter)
        compare(button.hovered, true)
        mouseMove(control, buttonXCenter - button.indicator.width, buttonYCenter - button.indicator.height)
        // It should not be pressed or hovered.
        compare(button.pressed, false)
        compare(button.hovered, false)

        mouseRelease(control)
    }

    function test_valueFromText_data() {
        return [
            { tag: "editable", editable: true },
            { tag: "non-editable", editable: false }
        ]
    }

    function test_valueFromText(data) {
        var control = createTemporaryObject(spinBox, testCase, {editable: data.editable})
        verify(control)

        control.forceActiveFocus()
        verify(control.activeFocus)

        var valueFromTextCalls = 0
        control.valueFromText = function(text, locale) {
            ++valueFromTextCalls
            return Number.fromLocaleString(locale, text);
        }

        keyClick(Qt.Key_Enter)
        compare(valueFromTextCalls, data.editable ? 1 : 0)

        keyClick(Qt.Key_Return)
        compare(valueFromTextCalls, data.editable ? 2 : 0)

        control.focus = false
        compare(valueFromTextCalls, data.editable ? 3 : 0)
    }

    function test_autoRepeat() {
        var control = createTemporaryObject(spinBox, testCase)
        verify(control)

        compare(control.value, 0)

        var valueSpy = signalSpy.createObject(control, {target: control, signalName: "valueChanged"})
        verify(valueSpy.valid)

        var countBefore = 0

        // repeat up
        mousePress(control.up.indicator)
        verify(control.up.pressed)
        compare(valueSpy.count, 0)
        valueSpy.wait()
        valueSpy.wait()
        countBefore = valueSpy.count
        mouseRelease(control.up.indicator)
        verify(!control.up.pressed)
        compare(valueSpy.count, countBefore)

        valueSpy.clear()

        // repeat down
        mousePress(control.down.indicator)
        verify(control.down.pressed)
        compare(valueSpy.count, 0)
        valueSpy.wait()
        valueSpy.wait()
        countBefore = valueSpy.count
        mouseRelease(control.down.indicator)
        verify(!control.down.pressed)
        compare(valueSpy.count, countBefore)

        mousePress(control.up.indicator)
        verify(control.up.pressed)
        valueSpy.wait()

        // move inside during repeat -> continue repeat (QTBUG-57085)
        mouseMove(control.up.indicator, control.up.indicator.width / 4, control.up.indicator.height / 4)
        verify(control.up.pressed)
        valueSpy.wait()

        valueSpy.clear()

        // move outside during repeat -> stop repeat
        mouseMove(control.up.indicator, -1, -1)
        verify(!control.up.pressed)
        // NOTE: The following wait() is NOT a reliable way to test that the
        // auto-repeat timer is not running, but there's no way dig into the
        // private APIs from QML. If this test ever fails in the future, it
        // indicates that the auto-repeat timer logic is broken.
        wait(125)
        compare(valueSpy.count, 0)

        mouseRelease(control.up.indicator, -1, -1)
        verify(!control.up.pressed)
    }

    function test_initialValue() {
        var control = createTemporaryObject(spinBox, testCase, {from: 1000, to: 10000})
        verify(control)
        compare(control.value, 1000)
    }

    Component {
        id: sizeBox
        SpinBox {
            from: 0
            to: items.length - 1

            property var items: ["Small", "Medium", "Large"]

            validator: RegExpValidator {
                regExp: new RegExp("(Small|Medium|Large)", "i")
            }

            textFromValue: function(value) {
                return items[value];
            }

            valueFromText: function(text) {
                for (var i = 0; i < items.length; ++i) {
                    if (items[i].toLowerCase().indexOf(text.toLowerCase()) === 0)
                        return i
                }
                return sb.value
            }
        }
    }

    function test_textFromValue_data() {
        return [
            { tag: "default", component: spinBox, values: [0, 10, 99], displayTexts: ["0", "10", "99"] },
            { tag: "custom", component: sizeBox, values: [0, 1, 2], displayTexts: ["Small", "Medium", "Large"] }
        ]
    }

    function test_textFromValue(data) {
        var control = createTemporaryObject(data.component, testCase)
        verify(control)

        for (var i = 0; i < data.values.length; ++i) {
            control.value = data.values[i]
            compare(control.value, data.values[i])
            compare(control.displayText, data.displayTexts[i])
        }
    }
}
