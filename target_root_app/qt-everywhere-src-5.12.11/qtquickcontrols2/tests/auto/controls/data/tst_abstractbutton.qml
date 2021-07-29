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

TestCase {
    id: testCase
    width: 200
    height: 200
    visible: true
    when: windowShown
    name: "AbstractButton"

    Component {
        id: button
        AbstractButton {}
    }

    Component {
        id: item
        Item { }
    }

    Component {
        id: action
        Action { }
    }

    Component {
        id: signalSpy
        SignalSpy { }
    }

    function test_text() {
        var control = createTemporaryObject(button, testCase);
        verify(control);

        compare(control.text, "");
        control.text = "Button";
        compare(control.text, "Button");
        control.text = "";
        compare(control.text, "");
    }

    function test_baseline() {
        var control = createTemporaryObject(button, testCase, {padding: 6})
        verify(control)
        compare(control.baselineOffset, 0)
        control.contentItem = item.createObject(control, {baselineOffset: 12})
        compare(control.baselineOffset, 18)
    }

    function test_implicitSize() {
        var control = createTemporaryObject(button, testCase)
        verify(control)

        compare(control.implicitWidth, 0)
        compare(control.implicitHeight, 0)

        control.contentItem = item.createObject(control, {implicitWidth: 10, implicitHeight: 20})
        compare(control.implicitWidth, 10)
        compare(control.implicitHeight, 20)

        control.background = item.createObject(control, {implicitWidth: 20, implicitHeight: 30})
        compare(control.implicitWidth, 20)
        compare(control.implicitHeight, 30)

        control.padding = 100
        compare(control.implicitWidth, 210)
        compare(control.implicitHeight, 220)
    }

    function test_pressPoint_data() {
        return [
            { tag: "mouse", mouse: true },
            { tag: "touch", touch: true }
        ]
    }

    function test_pressPoint(data) {
        var control = createTemporaryObject(button, testCase, {width: 100, height: 40})
        verify(control)

        var pressXChanges = 0
        var pressYChanges = 0

        var pressXSpy = signalSpy.createObject(control, {target: control, signalName: "pressXChanged"})
        verify(pressXSpy.valid)

        var pressYSpy = signalSpy.createObject(control, {target: control, signalName: "pressYChanged"})
        verify(pressYSpy.valid)

        compare(control.pressX, 0)
        compare(control.pressY, 0)

        var touch = data.touch ? touchEvent(control) : null

        if (data.touch)
            touch.press(0, control, control.width / 2, control.height / 2).commit()
        else
            mousePress(control, control.width / 2, control.height / 2)
        compare(control.pressX, control.width / 2)
        compare(control.pressY, control.height / 2)
        compare(pressXSpy.count, ++pressXChanges)
        compare(pressYSpy.count, ++pressYChanges)

        if (data.touch)
            touch.move(0, control, control.width / 2, control.height / 2).commit()
        else
            mouseMove(control, control.width / 2, control.height / 2)
        compare(control.pressX, control.width / 2)
        compare(control.pressY, control.height / 2)
        compare(pressXSpy.count, pressXChanges)
        compare(pressYSpy.count, pressYChanges)

        if (data.touch)
            touch.move(0, control, control.width / 4, control.height / 4).commit()
        else
            mouseMove(control, control.width / 4, control.height / 4)
        compare(control.pressX, control.width / 4)
        compare(control.pressY, control.height / 4)
        compare(pressXSpy.count, ++pressXChanges)
        compare(pressYSpy.count, ++pressYChanges)

        if (data.touch)
            touch.move(0, control, 0, 0).commit()
        else
            mouseMove(control, 0, 0)
        compare(control.pressX, 0)
        compare(control.pressY, 0)
        compare(pressXSpy.count, ++pressXChanges)
        compare(pressYSpy.count, ++pressYChanges)

        if (data.touch)
            touch.move(0, control, -control.width / 2, -control.height / 2).commit()
        else
            mouseMove(control, -control.width / 2, -control.height / 2)
        compare(control.pressX, -control.width / 2)
        compare(control.pressY, -control.height / 2)
        compare(pressXSpy.count, ++pressXChanges)
        compare(pressYSpy.count, ++pressYChanges)

        if (data.touch)
            touch.release(0, control, -control.width / 2, -control.height / 2).commit()
        else
            mouseRelease(control, -control.width / 2, -control.height / 2)
        compare(control.pressX, -control.width / 2)
        compare(control.pressY, -control.height / 2)
        compare(pressXSpy.count, pressXChanges)
        compare(pressYSpy.count, pressYChanges)

        if (data.touch)
            touch.press(0, control, control.width - 1, control.height - 1).commit()
        else
            mousePress(control, control.width - 1, control.height - 1)
        compare(control.pressX, control.width - 1)
        compare(control.pressY, control.height - 1)
        compare(pressXSpy.count, ++pressXChanges)
        compare(pressYSpy.count, ++pressYChanges)

        if (data.touch)
            touch.move(0, control, control.width + 1, control.height + 1).commit()
        else
            mousePress(control, control.width + 1, control.height + 1)
        compare(control.pressX, control.width + 1)
        compare(control.pressY, control.height + 1)
        compare(pressXSpy.count, ++pressXChanges)
        compare(pressYSpy.count, ++pressYChanges)

        if (data.touch)
            touch.release(0, control, control.width + 2, control.height + 2).commit()
        else
            mouseRelease(control, control.width + 2, control.height + 2)
        compare(control.pressX, control.width + 2)
        compare(control.pressY, control.height + 2)
        compare(pressXSpy.count, ++pressXChanges)
        compare(pressYSpy.count, ++pressYChanges)
    }

    function test_pressAndHold() {
        var control = createTemporaryObject(button, testCase, {checkable: true})
        verify(control)

        var pressAndHoldSpy = signalSpy.createObject(control, {target: control, signalName: "pressAndHold"})
        verify(pressAndHoldSpy.valid)

        mousePress(control)
        pressAndHoldSpy.wait()
        compare(control.checked, false)

        mouseRelease(control)
        compare(control.checked, false)
    }

    Component {
        id: keyCatcher
        Item {
            property int lastKeyPress: -1
            property int lastKeyRelease: -1
            Keys.onPressed: lastKeyPress = event.key
            Keys.onReleased: lastKeyRelease = event.key
        }
    }

    function test_keyEvents_data() {
        return [
            { tag: "space", key: Qt.Key_Space, result: -1 },
            { tag: "backspace", key: Qt.Key_Backspace, result: Qt.Key_Backspace }
        ]
    }

    function test_keyEvents(data) {
        var container = createTemporaryObject(keyCatcher, testCase)
        verify(container)

        var control = button.createObject(container)
        verify(control)

        control.forceActiveFocus()
        verify(control.activeFocus)

        keyPress(data.key)
        compare(container.lastKeyPress, data.result)

        keyRelease(data.key)
        compare(container.lastKeyRelease, data.result)
    }

    function test_icon() {
        var control = createTemporaryObject(button, testCase)
        verify(control)
        compare(control.icon.name, "")
        compare(control.icon.source, "")
        compare(control.icon.width, 0)
        compare(control.icon.height, 0)
        compare(control.icon.color, "#00000000")

        var iconSpy = signalSpy.createObject(control, { target: control, signalName: "iconChanged"} )
        verify(iconSpy.valid)

        control.icon.name = "test-name"
        compare(control.icon.name, "test-name")
        compare(iconSpy.count, 1)

        control.icon.source = "qrc:/test-source"
        compare(control.icon.source, "qrc:/test-source")
        compare(iconSpy.count, 2)

        control.icon.width = 32
        compare(control.icon.width, 32)
        compare(iconSpy.count, 3)

        control.icon.height = 32
        compare(control.icon.height, 32)
        compare(iconSpy.count, 4)

        control.icon.color = "#ff0000"
        compare(control.icon.color, "#ff0000")
        compare(iconSpy.count, 5)
    }

    function test_action_data() {
        return [
            { tag: "implicit text", property: "text",
                                    initButton: undefined, initAction: "Action",
                                    assignExpected: "Action", assignChanged: true,
                                    resetExpected: "", resetChanged: true },
            { tag: "explicit text", property: "text",
                                    initButton: "Button", initAction: "Action",
                                    assignExpected: "Button", assignChanged: false,
                                    resetExpected: "Button", resetChanged: false },
            { tag: "empty button text", property: "text",
                                    initButton: "", initAction: "Action",
                                    assignExpected: "", assignChanged: false,
                                    resetExpected: "", resetChanged: false },
            { tag: "empty action text", property: "text",
                                    initButton: "Button", initAction: "",
                                    assignExpected: "Button", assignChanged: false,
                                    resetExpected: "Button", resetChanged: false },
            { tag: "empty both text", property: "text",
                                    initButton: undefined, initAction: "",
                                    assignExpected: "", assignChanged: false,
                                    resetExpected: "", resetChanged: false },

            { tag: "modify button text", property: "text",
                                    initButton: undefined, initAction: "Action",
                                    assignExpected: "Action", assignChanged: true,
                                    modifyButton: "Button2",
                                    modifyButtonExpected: "Button2", modifyButtonChanged: true,
                                    resetExpected: "Button2", resetChanged: false },
            { tag: "modify implicit action text", property: "text",
                                    initButton: undefined, initAction: "Action",
                                    assignExpected: "Action", assignChanged: true,
                                    modifyAction: "Action2",
                                    modifyActionExpected: "Action2", modifyActionChanged: true,
                                    resetExpected: "", resetChanged: true },
            { tag: "modify explicit action text", property: "text",
                                    initButton: "Button", initAction: "Action",
                                    assignExpected: "Button", assignChanged: false,
                                    modifyAction: "Action2",
                                    modifyActionExpected: "Button", modifyActionChanged: false,
                                    resetExpected: "Button", resetChanged: false },
        ]
    }

    function test_action(data) {
        var control = createTemporaryObject(button, testCase)
        verify(control)
        control[data.property] = data.initButton

        var act = action.createObject(control)
        act[data.property] = data.initAction

        var spy = signalSpy.createObject(control, {target: control, signalName: data.property + "Changed"})
        verify(spy.valid)

        // assign action
        spy.clear()
        control.action = act
        compare(control[data.property], data.assignExpected)
        compare(spy.count, data.assignChanged ? 1 : 0)

        // modify button
        if (data.hasOwnProperty("modifyButton")) {
            spy.clear()
            control[data.property] = data.modifyButton
            compare(control[data.property], data.modifyButtonExpected)
            compare(spy.count, data.modifyButtonChanged ? 1 : 0)
        }

        // modify action
        if (data.hasOwnProperty("modifyAction")) {
            spy.clear()
            act[data.property] = data.modifyAction
            compare(control[data.property], data.modifyActionExpected)
            compare(spy.count, data.modifyActionChanged ? 1 : 0)
        }

        // reset action
        spy.clear()
        control.action = null
        compare(control[data.property], data.resetExpected)
        compare(spy.count, data.resetChanged ? 1 : 0)
    }

    function test_actionIcon_data() {
        var data = []

        // Save duplicating the rows by reusing them with different properties of the same type.
        // This means that the first loop will test icon.name and the second one will test icon.source.
        var stringPropertyValueSuffixes = [
            { propertyName: "name", valueSuffix: "IconName" },
            { propertyName: "source", valueSuffix: "IconSource" }
        ]

        for (var i = 0; i < stringPropertyValueSuffixes.length; ++i) {
            var propertyName = stringPropertyValueSuffixes[i].propertyName
            var valueSuffix = stringPropertyValueSuffixes[i].valueSuffix

            var buttonPropertyValue = "Button" + valueSuffix
            var buttonPropertyValue2 = "Button" + valueSuffix + "2"
            var actionPropertyValue = "Action" + valueSuffix
            var actionPropertyValue2 = "Action" + valueSuffix + "2"

            data.push({ tag: "implicit " + propertyName, property: propertyName,
                initButton: undefined, initAction: actionPropertyValue,
                assignExpected: actionPropertyValue, assignChanged: true,
                resetExpected: "", resetChanged: true })
            data.push({ tag: "explicit " + propertyName, property: propertyName,
                initButton: buttonPropertyValue, initAction: actionPropertyValue,
                assignExpected: buttonPropertyValue, assignChanged: false,
                resetExpected: buttonPropertyValue, resetChanged: false })
            data.push({ tag: "empty button " + propertyName, property: propertyName,
                initButton: "", initAction: actionPropertyValue,
                assignExpected: "", assignChanged: false,
                resetExpected: "", resetChanged: false })
            data.push({ tag: "empty action " + propertyName, property: propertyName,
                initButton: buttonPropertyValue, initAction: "",
                assignExpected: buttonPropertyValue, assignChanged: false,
                resetExpected: buttonPropertyValue, resetChanged: false })
            data.push({ tag: "empty both " + propertyName, property: propertyName,
                initButton: undefined, initAction: "",
                assignExpected: "", assignChanged: false,
                resetExpected: "", resetChanged: false })
            data.push({ tag: "modify button " + propertyName, property: propertyName,
                initButton: undefined, initAction: actionPropertyValue,
                assignExpected: actionPropertyValue, assignChanged: true,
                modifyButton: buttonPropertyValue2,
                modifyButtonExpected: buttonPropertyValue2, modifyButtonChanged: true,
                resetExpected: buttonPropertyValue2, resetChanged: false })
            data.push({ tag: "modify implicit action " + propertyName, property: propertyName,
                initButton: undefined, initAction: actionPropertyValue,
                assignExpected: actionPropertyValue, assignChanged: true,
                modifyAction: actionPropertyValue2,
                modifyActionExpected: actionPropertyValue2, modifyActionChanged: true,
                resetExpected: "", resetChanged: true })
            data.push({ tag: "modify explicit action " + propertyName, property: propertyName,
                initButton: buttonPropertyValue, initAction: actionPropertyValue,
                assignExpected: buttonPropertyValue, assignChanged: false,
                modifyAction: actionPropertyValue2,
                modifyActionExpected: buttonPropertyValue, modifyActionChanged: false,
                resetExpected: buttonPropertyValue, resetChanged: false })
        }

        var intPropertyNames = [
            "width",
            "height",
        ]

        for (i = 0; i < intPropertyNames.length; ++i) {
            propertyName = intPropertyNames[i]

            buttonPropertyValue = 20
            buttonPropertyValue2 = 21
            actionPropertyValue = 40
            actionPropertyValue2 = 41
            var defaultValue = 0

            data.push({ tag: "implicit " + propertyName, property: propertyName,
                initButton: undefined, initAction: actionPropertyValue,
                assignExpected: actionPropertyValue, assignChanged: true,
                resetExpected: defaultValue, resetChanged: true })
            data.push({ tag: "explicit " + propertyName, property: propertyName,
                initButton: buttonPropertyValue, initAction: actionPropertyValue,
                assignExpected: buttonPropertyValue, assignChanged: false,
                resetExpected: buttonPropertyValue, resetChanged: false })
            data.push({ tag: "default button " + propertyName, property: propertyName,
                initButton: defaultValue, initAction: actionPropertyValue,
                assignExpected: defaultValue, assignChanged: false,
                resetExpected: defaultValue, resetChanged: false })
            data.push({ tag: "default action " + propertyName, property: propertyName,
                initButton: buttonPropertyValue, initAction: defaultValue,
                assignExpected: buttonPropertyValue, assignChanged: false,
                resetExpected: buttonPropertyValue, resetChanged: false })
            data.push({ tag: "default both " + propertyName, property: propertyName,
                initButton: undefined, initAction: defaultValue,
                assignExpected: defaultValue, assignChanged: false,
                resetExpected: defaultValue, resetChanged: false })
            data.push({ tag: "modify button " + propertyName, property: propertyName,
                initButton: undefined, initAction: actionPropertyValue,
                assignExpected: actionPropertyValue, assignChanged: true,
                modifyButton: buttonPropertyValue2,
                modifyButtonExpected: buttonPropertyValue2, modifyButtonChanged: true,
                resetExpected: buttonPropertyValue2, resetChanged: false })
            data.push({ tag: "modify implicit action " + propertyName, property: propertyName,
                initButton: undefined, initAction: actionPropertyValue,
                assignExpected: actionPropertyValue, assignChanged: true,
                modifyAction: actionPropertyValue2,
                modifyActionExpected: actionPropertyValue2, modifyActionChanged: true,
                resetExpected: defaultValue, resetChanged: true })
            data.push({ tag: "modify explicit action " + propertyName, property: propertyName,
                initButton: buttonPropertyValue, initAction: actionPropertyValue,
                assignExpected: buttonPropertyValue, assignChanged: false,
                modifyAction: actionPropertyValue2,
                modifyActionExpected: buttonPropertyValue, modifyActionChanged: false,
                resetExpected: buttonPropertyValue, resetChanged: false })
        }

        propertyName = "color"
        buttonPropertyValue = "#aa0000"
        buttonPropertyValue2 = "#ff0000"
        actionPropertyValue = "#0000aa"
        actionPropertyValue2 = "#0000ff"
        defaultValue = "#00000000"

        data.push({ tag: "implicit " + propertyName, property: propertyName,
            initButton: undefined, initAction: actionPropertyValue,
            assignExpected: actionPropertyValue, assignChanged: true,
            resetExpected: defaultValue, resetChanged: true })
        data.push({ tag: "explicit " + propertyName, property: propertyName,
            initButton: buttonPropertyValue, initAction: actionPropertyValue,
            assignExpected: buttonPropertyValue, assignChanged: false,
            resetExpected: buttonPropertyValue, resetChanged: false })
        data.push({ tag: "default button " + propertyName, property: propertyName,
            initButton: defaultValue, initAction: actionPropertyValue,
            assignExpected: defaultValue, assignChanged: false,
            resetExpected: defaultValue, resetChanged: false })
        data.push({ tag: "default action " + propertyName, property: propertyName,
            initButton: buttonPropertyValue, initAction: defaultValue,
            assignExpected: buttonPropertyValue, assignChanged: false,
            resetExpected: buttonPropertyValue, resetChanged: false })
        data.push({ tag: "default both " + propertyName, property: propertyName,
            initButton: undefined, initAction: defaultValue,
            assignExpected: defaultValue, assignChanged: false,
            resetExpected: defaultValue, resetChanged: false })
        data.push({ tag: "modify button " + propertyName, property: propertyName,
            initButton: undefined, initAction: actionPropertyValue,
            assignExpected: actionPropertyValue, assignChanged: true,
            modifyButton: buttonPropertyValue2,
            modifyButtonExpected: buttonPropertyValue2, modifyButtonChanged: true,
            resetExpected: buttonPropertyValue2, resetChanged: false })
        data.push({ tag: "modify implicit action " + propertyName, property: propertyName,
            initButton: undefined, initAction: actionPropertyValue,
            assignExpected: actionPropertyValue, assignChanged: true,
            modifyAction: actionPropertyValue2,
            modifyActionExpected: actionPropertyValue2, modifyActionChanged: true,
            resetExpected: defaultValue, resetChanged: true })
        data.push({ tag: "modify explicit action " + propertyName, property: propertyName,
            initButton: buttonPropertyValue, initAction: actionPropertyValue,
            assignExpected: buttonPropertyValue, assignChanged: false,
            modifyAction: actionPropertyValue2,
            modifyActionExpected: buttonPropertyValue, modifyActionChanged: false,
            resetExpected: buttonPropertyValue, resetChanged: false })

        return data;
    }

    function test_actionIcon(data) {
        var control = createTemporaryObject(button, testCase)
        verify(control)
        control.icon[data.property] = data.initButton

        var act = action.createObject(control)
        act.icon[data.property] = data.initAction

        var spy = signalSpy.createObject(control, {target: control, signalName: "iconChanged"})
        verify(spy.valid)

        // assign action
        spy.clear()
        control.action = act
        compare(control.icon[data.property], data.assignExpected)
        compare(spy.count, data.assignChanged ? 1 : 0)

        // modify button
        if (data.hasOwnProperty("modifyButton")) {
            spy.clear()
            control.icon[data.property] = data.modifyButton
            compare(control.icon[data.property], data.modifyButtonExpected)
            compare(spy.count, data.modifyButtonChanged ? 1 : 0)
        }

        // modify action
        if (data.hasOwnProperty("modifyAction")) {
            spy.clear()
            act.icon[data.property] = data.modifyAction
            compare(control.icon[data.property], data.modifyActionExpected)
            compare(spy.count, data.modifyActionChanged ? 1 : 0)
        }

        // reset action
        spy.clear()
        control.action = null
        compare(control.icon[data.property], data.resetExpected)
        compare(spy.count, data.resetChanged ? 1 : 0)
    }

    Component {
        id: actionButton
        AbstractButton {
            action: Action {
                text: "Default"
                icon.name: checked ? "checked" : "unchecked"
                icon.source: "qrc:/icons/default.png"
                checkable: true
                checked: true
                enabled: false
            }
        }
    }

    function test_actionButton() {
        var control = createTemporaryObject(actionButton, testCase)
        verify(control)

        // initial values
        compare(control.text, "Default")
        compare(control.checkable, true)
        compare(control.checked, true)
        compare(control.enabled, false)
        compare(control.icon.name, "checked")

        var textSpy = signalSpy.createObject(control, { target: control, signalName: "textChanged" })
        verify(textSpy.valid)

        // changes via action
        control.action.text = "Action"
        control.action.checkable = false
        control.action.checked = false
        control.action.enabled = true
        compare(control.text, "Action") // propagates
        compare(control.checkable, false) // propagates
        compare(control.checked, false) // propagates
        compare(control.enabled, true) // propagates
        compare(control.icon.name, "unchecked") // propagates
        compare(textSpy.count, 1)

        // changes via button
        control.text = "Button"
        control.checkable = true
        control.checked = true
        control.enabled = false
        control.icon.name = "default"
        compare(control.text, "Button")
        compare(control.checkable, true)
        compare(control.checked, true)
        compare(control.enabled, false)
        compare(control.icon.name, "default")
        compare(control.action.text, "Action") // does NOT propagate
        compare(control.action.checkable, true) // propagates
        compare(control.action.checked, true) // propagates
        compare(control.action.enabled, true) // does NOT propagate
        compare(control.action.icon.name, control.action.checked ? "checked" : "unchecked") // does NOT propagate
        compare(textSpy.count, 2)

        // remove the action so that only the button's properties are left
        control.action = null
        compare(control.text, "Button")
        compare(control.icon.name, "default")
        compare(textSpy.count, 2)

        // setting an action while button has a particular property set
        // shouldn't cause a change in the button's effective property value
        var secondAction = createTemporaryObject(action, testCase)
        verify(secondAction)
        secondAction.text = "SecondAction"
        control.action = secondAction
        compare(control.text, "Button")
        compare(textSpy.count, 2)

        // test setting an action whose properties aren't set
        var thirdAction = createTemporaryObject(action, testCase)
        verify(thirdAction)
        control.action = thirdAction
        compare(control.text, "Button")
        compare(textSpy.count, 2)
    }

    Component {
        id: checkableButton
        AbstractButton {
            checkable: true
            action: Action {}
        }
    }

    function test_checkable_button() {
        var control = createTemporaryObject(checkableButton, testCase)
        verify(control)
        control.checked = false
        control.forceActiveFocus()
        verify(control.activeFocus)
        verify(!control.checked)
        verify(!control.action.checked)

        keyPress(Qt.Key_Space)
        keyRelease(Qt.Key_Space)

        compare(control.action.checked, true)
        compare(control.checked, true)

        keyPress(Qt.Key_Space)

        compare(control.action.checked, true)
        compare(control.checked, true)

        keyRelease(Qt.Key_Space)

        compare(control.action.checked, false)
        compare(control.checked, false)

        var checkedSpy = signalSpy.createObject(control, {target: control.action, signalName: "checkedChanged"})
        var toggledSpy = signalSpy.createObject(control, {target: control, signalName: "toggled"})
        var actionToggledSpy = signalSpy.createObject(control, {target: control.action, signalName: "toggled"})

        verify(checkedSpy.valid)
        verify(toggledSpy.valid)
        verify(actionToggledSpy.valid)

        mousePress(control)

        compare(control.action.checked, false)
        compare(control.checked, false)

        mouseRelease(control)

        checkedSpy.wait()
        compare(checkedSpy.count, 1)
        compare(actionToggledSpy.count, 1)
        compare(toggledSpy.count, 1)

        compare(control.action.checked, true)
        compare(control.checked, true)

        mousePress(control)
        mouseRelease(control)

        compare(control.checked, false)
        compare(control.action.checked, false)
    }

    function test_trigger_data() {
        return [
            {tag: "click", click: true, button: true, action: true, clicked: true, triggered: true},
            {tag: "click disabled button", click: true, button: false, action: true, clicked: false, triggered: false},
            {tag: "click disabled action", click: true, button: true, action: false, clicked: true, triggered: false},
            {tag: "trigger", trigger: true, button: true, action: true, clicked: true, triggered: true},
            {tag: "trigger disabled button", trigger: true, button: false, action: true, clicked: false, triggered: true},
            {tag: "trigger disabled action", trigger: true, button: true, action: false, clicked: false, triggered: false}
        ]
    }

    function test_trigger(data) {
        var control = createTemporaryObject(actionButton, testCase, {"action.enabled": data.action, "enabled": data.button})
        verify(control)

        compare(control.enabled, data.button)
        compare(control.action.enabled, data.action)

        var buttonSpy = signalSpy.createObject(control, {target: control, signalName: "clicked"})
        verify(buttonSpy.valid)

        var actionSpy = signalSpy.createObject(control, {target: control.action, signalName: "triggered"})
        verify(actionSpy.valid)

        if (data.click)
            mouseClick(control)
        else if (data.trigger)
            control.action.trigger()

        compare(buttonSpy.count, data.clicked ? 1 : 0)
        compare(actionSpy.count, data.triggered ? 1 : 0)
    }

    function test_mnemonic() {
        if (Qt.platform.os === "osx" || Qt.platform.os === "macos")
            skip("Mnemonics are not used on macOS")

        var control = createTemporaryObject(button, testCase)
        verify(control)

        control.text = "&Hello"
        compare(control.text, "&Hello")

        var clickSpy = signalSpy.createObject(control, {target: control, signalName: "clicked"})
        verify(clickSpy.valid)

        keyClick(Qt.Key_H, Qt.AltModifier)
        compare(clickSpy.count, 1)

        control.visible = false
        keyClick(Qt.Key_H, Qt.AltModifier)
        compare(clickSpy.count, 1)

        control.visible = true
        keyClick(Qt.Key_H, Qt.AltModifier)
        compare(clickSpy.count, 2)

        control.text = "Te&st"
        compare(control.text, "Te&st")

        keyClick(Qt.Key_H, Qt.AltModifier)
        compare(clickSpy.count, 2)

        keyClick(Qt.Key_S, Qt.AltModifier)
        compare(clickSpy.count, 3)

        control.visible = false
        control.text = "&Hidden"
        keyClick(Qt.Key_H, Qt.AltModifier)
        compare(clickSpy.count, 3)

        control.visible = true
        keyClick(Qt.Key_H, Qt.AltModifier)
        compare(clickSpy.count, 4)

        control.text = undefined
        control.action = action.createObject(control, {text: "&Action"})

        var actionSpy = signalSpy.createObject(control, {target: control.action, signalName: "triggered"})
        verify(actionSpy.valid)

        keyClick(Qt.Key_A, Qt.AltModifier)
        compare(actionSpy.count, 1)
        compare(clickSpy.count, 5)

        // ungrab on destruction (don't crash)
        control.Component.onDestruction.connect(function() { control = null })
        control.destroy()
        wait(0)
        verify(!control)
        keyClick(Qt.Key_H, Qt.AltModifier)
    }

    Component {
        id: actionGroup
        ActionGroup {
            Action { id: action1; checkable: true; checked: true }
            Action { id: action2; checkable: true }
            Action { id: action3; checkable: true }
        }
    }

    function test_actionGroup() {
        var group = createTemporaryObject(actionGroup, testCase)
        verify(group)

        var button1 = createTemporaryObject(button, testCase, {action: group.actions[0], width: 10, height: 10})
        var button2 = createTemporaryObject(button, testCase, {action: group.actions[1], width: 10, height: 10, y: 10})
        var button3 = createTemporaryObject(button, testCase, {action: group.actions[2], width: 10, height: 10, y: 20})

        verify(button1)
        compare(button1.checked, true)
        compare(button1.action.checked, true)

        verify(button2)
        compare(button2.checked, false)
        compare(button2.action.checked, false)

        verify(button3)
        compare(button3.checked, false)
        compare(button3.action.checked, false)

        mouseClick(button2)

        compare(button1.checked, false)
        compare(button1.action.checked, false)

        compare(button2.checked, true)
        compare(button2.action.checked, true)

        compare(button3.checked, false)
        compare(button3.action.checked, false)
    }

    function test_clickedAfterLongPress() {
        var control = createTemporaryObject(button, testCase, { text: "Hello" })
        verify(control)

        var clickedSpy = signalSpy.createObject(control, { target: control, signalName: "clicked" })
        verify(clickedSpy.valid)

        mousePress(control)
        // Ensure that clicked is emitted when no handler is defined for the pressAndHold() signal.
        // Note that even though signal spies aren't considered in QObject::isSignalConnected(),
        // we can't use one here to check for pressAndHold(), because otherwise clicked() won't be emitted.
        wait(Qt.styleHints.mousePressAndHoldInterval + 100)
        mouseRelease(control)
        compare(clickedSpy.count, 1)
    }
}
