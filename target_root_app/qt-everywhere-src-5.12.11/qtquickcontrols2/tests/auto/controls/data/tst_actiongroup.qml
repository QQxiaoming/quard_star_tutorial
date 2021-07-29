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
    name: "ActionGroup"

    Component {
        id: actionGroup
        ActionGroup { }
    }

    Component {
        id: nonExclusiveGroup
        ActionGroup { exclusive: false }
    }

    Component {
        id: declarativeGroup
        ActionGroup {
            Action { text: "First" }
            Action { text: "Second" }
            Action { text: "Third" }
        }
    }

    Component {
        id: signalSpy
        SignalSpy { }
    }

    function test_null() {
        var group = createTemporaryObject(actionGroup, testCase)
        verify(group)

        group.addAction(null)
        group.removeAction(null)
    }

    Component {
        id: action
        Action { }
    }

    function test_defaults() {
        var group = createTemporaryObject(actionGroup, testCase)
        verify(group)
        compare(group.actions.length, 0)
        compare(group.checkedAction, null)
        compare(group.exclusive, true)
    }

    function test_current() {
        var group = createTemporaryObject(actionGroup, testCase)
        verify(group)

        var checkedActionSpy = createTemporaryObject(signalSpy, testCase, {target: group, signalName: "checkedActionChanged"})
        verify(checkedActionSpy.valid)
        verify(!group.checkedAction)

        var action1 = createTemporaryObject(action, testCase, {checked: true})
        var action2 = createTemporaryObject(action, testCase, {checked: false})
        var action3 = createTemporaryObject(action, testCase, {checked: true, objectName: "3"})

        // add checked
        group.addAction(action1)
        compare(group.checkedAction, action1)
        compare(action1.checked, true)
        compare(action2.checked, false)
        compare(action3.checked, true)
        compare(checkedActionSpy.count, 1)

        // add non-checked
        group.addAction(action2)
        compare(group.checkedAction, action1)
        compare(action1.checked, true)
        compare(action2.checked, false)
        compare(action3.checked, true)
        compare(checkedActionSpy.count, 1)

        // add checked
        group.addAction(action3)
        compare(group.checkedAction, action3)
        compare(action1.checked, false)
        compare(action2.checked, false)
        compare(action3.checked, true)
        compare(checkedActionSpy.count, 2)

        // change current
        group.checkedAction = action2
        compare(group.checkedAction, action2)
        compare(action1.checked, false)
        compare(action2.checked, true)
        compare(action3.checked, false)
        compare(checkedActionSpy.count, 3)

        // check
        action1.checked = true
        compare(group.checkedAction, action1)
        compare(action1.checked, true)
        compare(action2.checked, false)
        compare(action3.checked, false)
        compare(checkedActionSpy.count, 4)

        // remove non-checked
        group.removeAction(action2)
        compare(group.checkedAction, action1)
        compare(action1.checked, true)
        compare(action2.checked, false)
        compare(action3.checked, false)
        compare(checkedActionSpy.count, 4)

        // remove checked
        group.removeAction(action1)
        verify(!group.checkedAction)
        compare(action1.checked, false)
        compare(action2.checked, false)
        compare(action3.checked, false)
        compare(checkedActionSpy.count, 5)
    }

    function test_actions() {
        var group = createTemporaryObject(actionGroup, testCase)
        verify(group)

        var actionsSpy = createTemporaryObject(signalSpy, testCase, {target: group, signalName: "actionsChanged"})
        verify(actionsSpy.valid)

        compare(group.actions.length, 0)
        compare(group.checkedAction, null)

        var action1 = createTemporaryObject(action, testCase, {checked: true})
        var action2 = createTemporaryObject(action, testCase, {checked: false})

        group.actions = [action1, action2]
        compare(group.actions.length, 2)
        compare(group.actions[0], action1)
        compare(group.actions[1], action2)
        compare(group.checkedAction, action1)
        compare(actionsSpy.count, 2)

        var action3 = createTemporaryObject(action, testCase, {checked: true})

        group.addAction(action3)
        compare(group.actions.length, 3)
        compare(group.actions[0], action1)
        compare(group.actions[1], action2)
        compare(group.actions[2], action3)
        compare(group.checkedAction, action3)
        compare(actionsSpy.count, 3)

        group.removeAction(action1)
        compare(group.actions.length, 2)
        compare(group.actions[0], action2)
        compare(group.actions[1], action3)
        compare(group.checkedAction, action3)
        compare(actionsSpy.count, 4)

        group.actions = []
        compare(group.actions.length, 0)
        tryCompare(group, "checkedAction", null)
        compare(actionsSpy.count, 5)
    }

    function test_declarative() {
        var group = createTemporaryObject(declarativeGroup, testCase)
        verify(group)

        compare(group.actions.length, 3)
        compare(group.actions[0].text, "First")
        compare(group.actions[1].text, "Second")
        compare(group.actions[2].text, "Third")
    }

    function test_triggered_data() {
        return [
            {tag: "exclusive", exclusive: true},
            {tag: "non-exclusive", exclusive: false}
        ]
    }

    function test_triggered(data) {
        var group = createTemporaryObject(actionGroup, testCase, {exclusive: data.exclusive})
        verify(group)

        var triggeredSpy = createTemporaryObject(signalSpy, testCase, {target: group, signalName: "triggered"})
        verify(triggeredSpy.valid)

        var action1 = createTemporaryObject(action, testCase)
        var action2 = createTemporaryObject(action, testCase)

        group.addAction(action1)
        group.addAction(action2)

        action1.triggered()
        compare(triggeredSpy.count, 1)
        compare(triggeredSpy.signalArguments[0][0], action1)

        action2.triggered()
        compare(triggeredSpy.count, 2)
        compare(triggeredSpy.signalArguments[1][0], action2)
    }

    Component {
        id: attachedGroup
        Item {
            property ActionGroup group: ActionGroup { id: group }
            property Action action1: Action { ActionGroup.group: group }
            property Action action2: Action { ActionGroup.group: group }
            property Action action3: Action { ActionGroup.group: group }
        }
    }

    function test_attached() {
        var container = createTemporaryObject(attachedGroup, testCase)
        verify(container)

        verify(!container.group.checkedAction)

        container.action1.checked = true
        compare(container.group.checkedAction, container.action1)
        compare(container.action1.checked, true)
        compare(container.action2.checked, false)
        compare(container.action3.checked, false)

        container.action2.checked = true
        compare(container.group.checkedAction, container.action2)
        compare(container.action1.checked, false)
        compare(container.action2.checked, true)
        compare(container.action3.checked, false)

        container.action3.checked = true
        compare(container.group.checkedAction, container.action3)
        compare(container.action1.checked, false)
        compare(container.action2.checked, false)
        compare(container.action3.checked, true)
    }

    function test_actionDestroyed() {
        var group = createTemporaryObject(actionGroup, testCase)
        verify(group)

        var actionsSpy = createTemporaryObject(signalSpy, testCase, {target: group, signalName: "actionsChanged"})
        verify(actionsSpy.valid)

        var action1 = createTemporaryObject(action, testCase, {objectName: "action1", checked: true})

        group.addAction(action1)
        compare(group.actions.length, 1)
        compare(group.actions[0], action1)
        compare(group.checkedAction, action1)
        compare(actionsSpy.count, 1)

        action1.destroy()
        wait(0)
        compare(group.actions.length, 0)
        compare(group.checkedAction, null)
        compare(actionsSpy.count, 2)
    }

    function test_nonExclusive() {
        var group = createTemporaryObject(nonExclusiveGroup, testCase)
        verify(group)

        var action1 = createTemporaryObject(action, testCase, {checked: true})
        group.addAction(action1)
        compare(action1.checked, true)
        compare(group.checkedAction, null)

        var action2 = createTemporaryObject(action, testCase, {checked: true})
        group.addAction(action2)
        compare(action1.checked, true)
        compare(action2.checked, true)
        compare(group.checkedAction, null)

        action1.checked = false
        compare(action1.checked, false)
        compare(action2.checked, true)
        compare(group.checkedAction, null)

        action2.checked = false
        compare(action1.checked, false)
        compare(action2.checked, false)
        compare(group.checkedAction, null)

        action1.checked = true
        compare(action1.checked, true)
        compare(action2.checked, false)
        compare(group.checkedAction, null)

        action2.checked = true
        compare(action1.checked, true)
        compare(action2.checked, true)
        compare(group.checkedAction, null)
    }

    function test_enabled() {
        var group = createTemporaryObject(actionGroup, testCase)
        verify(group)

        compare(group.enabled, true)

        var action1 = createTemporaryObject(action, testCase)
        var action2 = createTemporaryObject(action, testCase)
        compare(action1.enabled, true)
        compare(action2.enabled, true)

        var action1Spy = createTemporaryObject(signalSpy, testCase, {target: action1, signalName: "enabledChanged"})
        var action2Spy = createTemporaryObject(signalSpy, testCase, {target: action2, signalName: "enabledChanged"})
        verify(action1Spy.valid && action2Spy.valid)

        group.addAction(action1)
        compare(action1.enabled, true)
        compare(action2.enabled, true)
        compare(action1Spy.count, 0)
        compare(action2Spy.count, 0)

        group.enabled = false
        compare(action1.enabled, false)
        compare(action2.enabled, true)
        compare(action1Spy.count, 1)
        compare(action1Spy.signalArguments[0][0], false)
        compare(action2Spy.count, 0)

        group.addAction(action2)
        compare(action1.enabled, false)
        compare(action2.enabled, false)
        compare(action1Spy.count, 1)
        compare(action2Spy.count, 1)
        compare(action2Spy.signalArguments[0][0], false)

        action1.enabled = false
        compare(action1.enabled, false)
        compare(action1Spy.count, 2)
        compare(action1Spy.signalArguments[1][0], false)
        compare(action2Spy.count, 1)

        group.enabled = true
        compare(action1.enabled, false)
        compare(action2.enabled, true)
        compare(action1Spy.count, 2)
        compare(action2Spy.count, 2)
        compare(action2Spy.signalArguments[1][0], true)
    }
}
