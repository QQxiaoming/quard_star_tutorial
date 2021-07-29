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
    name: "StackView"

    Item { id: item }
    Component { id: textField; TextField { } }
    Component { id: component; Item { } }

    Component {
        id: stackView
        StackView { }
    }

    Component {
        id: signalSpy
        SignalSpy { }
    }

    function test_initialItem() {
        var control1 = createTemporaryObject(stackView, testCase)
        verify(control1)
        compare(control1.currentItem, null)
        control1.destroy()

        var control2 = createTemporaryObject(stackView, testCase, {initialItem: item})
        verify(control2)
        compare(control2.currentItem, item)
        control2.destroy()

        var control3 = createTemporaryObject(stackView, testCase, {initialItem: component})
        verify(control3)
        verify(control3.currentItem)
        control3.destroy()
    }

    function test_currentItem() {
        var control = createTemporaryObject(stackView, testCase, {initialItem: item})
        verify(control)
        compare(control.currentItem, item)
        control.push(component)
        verify(control.currentItem !== item)
        control.pop(StackView.Immediate)
        compare(control.currentItem, item)
    }

    function test_busy() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)
        compare(control.busy, false)

        var busyCount = 0
        var busySpy = signalSpy.createObject(control, {target: control, signalName: "busyChanged"})
        verify(busySpy.valid)

        control.push(component)
        compare(control.busy, false)
        compare(busySpy.count, busyCount)

        control.push(component)
        compare(control.busy, true)
        compare(busySpy.count, ++busyCount)
        tryCompare(control, "busy", false)
        compare(busySpy.count, ++busyCount)

        control.replace(component)
        compare(control.busy, true)
        compare(busySpy.count, ++busyCount)
        tryCompare(control, "busy", false)
        compare(busySpy.count, ++busyCount)

        control.pop()
        compare(control.busy, true)
        compare(busySpy.count, ++busyCount)
        tryCompare(control, "busy", false)
        compare(busySpy.count, ++busyCount)

        control.pushEnter = null
        control.pushExit = null

        control.push(component)
        compare(control.busy, false)
        compare(busySpy.count, busyCount)

        control.replaceEnter = null
        control.replaceExit = null

        control.replace(component)
        compare(control.busy, false)
        compare(busySpy.count, busyCount)

        control.popEnter = null
        control.popExit = null

        control.pop()
        compare(control.busy, false)
        compare(busySpy.count, busyCount)
    }

    function test_status() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        var item1 = component.createObject(control)
        compare(item1.StackView.status, StackView.Inactive)
        control.push(item1)
        compare(item1.StackView.status, StackView.Active)

        var item2 = component.createObject(control)
        compare(item2.StackView.status, StackView.Inactive)
        control.push(item2)
        compare(item2.StackView.status, StackView.Activating)
        compare(item1.StackView.status, StackView.Deactivating)
        tryCompare(item2.StackView, "status", StackView.Active)
        tryCompare(item1.StackView, "status", StackView.Inactive)

        control.pop()
        compare(item2.StackView.status, StackView.Deactivating)
        compare(item1.StackView.status, StackView.Activating)
        tryCompare(item2.StackView, "status", StackView.Inactive)
        tryCompare(item1.StackView, "status", StackView.Active)
    }

    function test_index() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        var item1 = component.createObject(control)
        compare(item1.StackView.index, -1)
        control.push(item1, StackView.Immediate)
        compare(item1.StackView.index, 0)

        var item2 = component.createObject(control)
        compare(item2.StackView.index, -1)
        control.push(item2, StackView.Immediate)
        compare(item2.StackView.index, 1)
        compare(item1.StackView.index, 0)

        control.pop(StackView.Immediate)
        compare(item2.StackView.index, -1)
        compare(item1.StackView.index, 0)
    }

    function test_view() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        var item1 = component.createObject(control)
        compare(item1.StackView.view, null)
        control.push(item1, StackView.Immediate)
        compare(item1.StackView.view, control)

        var item2 = component.createObject(control)
        compare(item2.StackView.view, null)
        control.push(item2, StackView.Immediate)
        compare(item2.StackView.view, control)
        compare(item1.StackView.view, control)

        control.pop(StackView.Immediate)
        compare(item2.StackView.view, null)
        compare(item1.StackView.view, control)
    }

    function test_depth() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        var depthChanges = 0
        var emptyChanges = 0
        var depthSpy = signalSpy.createObject(control, {target: control, signalName: "depthChanged"})
        var emptySpy = signalSpy.createObject(control, {target: control, signalName: "emptyChanged"})
        verify(depthSpy.valid)
        verify(emptySpy.valid)
        compare(control.depth, 0)
        compare(control.empty, true)

        control.push(item, StackView.Immediate)
        compare(control.depth, 1)
        compare(depthSpy.count, ++depthChanges)
        compare(control.empty, false)
        compare(emptySpy.count, ++emptyChanges)

        control.clear()
        compare(control.depth, 0)
        compare(depthSpy.count, ++depthChanges)
        compare(control.empty, true)
        compare(emptySpy.count, ++emptyChanges)

        control.push(component, StackView.Immediate)
        compare(control.depth, 1)
        compare(depthSpy.count, ++depthChanges)
        compare(control.empty, false)
        compare(emptySpy.count, ++emptyChanges)

        control.push(component, StackView.Immediate)
        compare(control.depth, 2)
        compare(depthSpy.count, ++depthChanges)
        compare(control.empty, false)
        compare(emptySpy.count, emptyChanges)

        control.replace(component, StackView.Immediate)
        compare(control.depth, 2)
        compare(depthSpy.count, depthChanges)
        compare(control.empty, false)
        compare(emptySpy.count, emptyChanges)

        control.replace([component, component], StackView.Immediate)
        compare(control.depth, 3)
        compare(depthSpy.count, ++depthChanges)
        compare(control.empty, false)
        compare(emptySpy.count, emptyChanges)

        control.pop(null, StackView.Immediate)
        compare(control.depth, 1)
        compare(depthSpy.count, ++depthChanges)
        compare(control.empty, false)
        compare(emptySpy.count, emptyChanges)

        control.pop(StackView.Immediate) // ignored
        compare(control.depth, 1)
        compare(depthSpy.count, depthChanges)
        compare(control.empty, false)
        compare(emptySpy.count, emptyChanges)

        control.clear()
        compare(control.depth, 0)
        compare(depthSpy.count, ++depthChanges)
        compare(control.empty, true)
        compare(emptySpy.count, ++emptyChanges)

        control.clear()
        compare(control.depth, 0)
        compare(depthSpy.count, depthChanges)
        compare(control.empty, true)
        compare(emptySpy.count, emptyChanges)
    }

    function test_size() {
        var container = createTemporaryObject(component, testCase, {width: 200, height: 200})
        verify(container)
        var control = stackView.createObject(container, {width: 100, height: 100})
        verify(control)

        container.width += 10
        container.height += 20
        compare(control.width, 100)
        compare(control.height, 100)

        control.push(item, StackView.Immediate)
        compare(item.width, control.width)
        compare(item.height, control.height)

        control.width = 200
        control.height = 200
        compare(item.width, control.width)
        compare(item.height, control.height)

        control.clear()
        control.width += 10
        control.height += 20
        verify(item.width !== control.width)
        verify(item.height !== control.height)

        control.push(item, StackView.Immediate)
        compare(item.width, control.width)
        compare(item.height, control.height)
    }

    function test_focus_data() {
        return [
            { tag: "true", focus: true, forceActiveFocus: false },
            { tag: "false", focus: false, forceActiveFocus: false },
            { tag: "forceActiveFocus()", focus: false, forceActiveFocus: true },
        ]
    }

    function test_focus(data) {
        var control = createTemporaryObject(stackView, testCase, {initialItem: item, width: 200, height: 200})
        verify(control)

        if (data.focus)
            control.focus = true
        if (data.forceActiveFocus)
            control.forceActiveFocus()
        compare(control.activeFocus, data.focus || data.forceActiveFocus)

        var page = control.push(textField, StackView.Immediate)
        verify(page)
        compare(control.currentItem, page)
        compare(page.activeFocus, control.activeFocus)

        control.pop(StackView.Immediate)
        compare(control.currentItem, item)
        compare(item.activeFocus, data.focus || data.forceActiveFocus)
        verify(!page.activeFocus)
    }

    function test_find() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        var item1 = component.createObject(control, {objectName: "1"})
        var item2 = component.createObject(control, {objectName: "2"})
        var item3 = component.createObject(control, {objectName: "3"})

        control.push(item1, StackView.Immediate)
        control.push(item2, StackView.Immediate)
        control.push(item3, StackView.Immediate)

        compare(control.find(function(item, index) { return index === 0 }), item1)
        compare(control.find(function(item) { return item.objectName === "1" }), item1)

        compare(control.find(function(item, index) { return index === 1 }), item2)
        compare(control.find(function(item) { return item.objectName === "2" }), item2)

        compare(control.find(function(item, index) { return index === 2 }), item3)
        compare(control.find(function(item) { return item.objectName === "3" }), item3)

        compare(control.find(function() { return false }), null)
        compare(control.find(function() { return true }), item3)
    }

    function test_get() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        control.push([item, component, component], StackView.Immediate)

        verify(control.get(0, StackView.DontLoad))
        compare(control.get(0, StackView.ForceLoad), item)

        verify(!control.get(1, StackView.DontLoad))

        verify(control.get(2, StackView.DontLoad))
        verify(control.get(2, StackView.ForceLoad))
    }

    function test_push() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        // missing arguments
        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: push: missing arguments")
        compare(control.push(), null)

        // nothing to push
        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: push: nothing to push")
        compare(control.push(StackView.Immediate), null)

        // unsupported type
        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: push: QtObject is not supported. Must be Item or Component.")
        control.push(Qt.createQmlObject('import QtQml 2.0; QtObject { }', control))

        // push(item)
        var item1 = component.createObject(control, {objectName:"1"})
        compare(control.push(item1, StackView.Immediate), item1)
        compare(control.depth, 1)
        compare(control.currentItem, item1)

        // push([item])
        var item2 = component.createObject(control, {objectName:"2"})
        compare(control.push([item2], StackView.Immediate), item2)
        compare(control.depth, 2)
        compare(control.currentItem, item2)

        // push(item, {properties})
        var item3 = component.createObject(control)
        compare(control.push(item3, {objectName:"3"}, StackView.Immediate), item3)
        compare(item3.objectName, "3")
        compare(control.depth, 3)
        compare(control.currentItem, item3)

        // push([item, {properties}])
        var item4 = component.createObject(control)
        compare(control.push([item4, {objectName:"4"}], StackView.Immediate), item4)
        compare(item4.objectName, "4")
        compare(control.depth, 4)
        compare(control.currentItem, item4)

        // push(component, {properties})
        var item5 = control.push(component, {objectName:"5"}, StackView.Immediate)
        compare(item5.objectName, "5")
        compare(control.depth, 5)
        compare(control.currentItem, item5)

        // push([component, {properties}])
        var item6 = control.push([component, {objectName:"6"}], StackView.Immediate)
        compare(item6.objectName, "6")
        compare(control.depth, 6)
        compare(control.currentItem, item6)
    }

    function test_pop() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        var items = []
        for (var i = 0; i < 7; ++i)
            items.push(component.createObject(control, {objectName:i}))

        control.push(items, StackView.Immediate)

        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: pop: too many arguments")
        compare(control.pop(1, 2, 3), null)

        // pop the top most item
        compare(control.pop(StackView.Immediate), items[6])
        compare(control.depth, 6)
        compare(control.currentItem, items[5])

        // pop down to the current item
        compare(control.pop(control.currentItem, StackView.Immediate), null)
        compare(control.depth, 6)
        compare(control.currentItem, items[5])

        // pop down to (but not including) the Nth item
        compare(control.pop(items[3], StackView.Immediate), items[5])
        compare(control.depth, 4)
        compare(control.currentItem, items[3])

        // pop the top most item
        compare(control.pop(undefined, StackView.Immediate), items[3])
        compare(control.depth, 3)
        compare(control.currentItem, items[2])

        // don't pop non-existent item
        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: pop: unknown argument: " + testCase)
        compare(control.pop(testCase, StackView.Immediate), null)
        compare(control.depth, 3)
        compare(control.currentItem, items[2])

        // pop all items down to (but not including) the 1st item
        control.pop(null, StackView.Immediate)
        compare(control.depth, 1)
        compare(control.currentItem, items[0])
    }

    function test_replace() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        // missing arguments
        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: replace: missing arguments")
        compare(control.replace(), null)

        // nothing to push
        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: replace: nothing to push")
        compare(control.replace(StackView.Immediate), null)

        // unsupported type
        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: replace: QtObject is not supported. Must be Item or Component.")
        compare(control.replace(Qt.createQmlObject('import QtQml 2.0; QtObject { }', control)), null)

        // replace(item)
        var item1 = component.createObject(control, {objectName:"1"})
        compare(control.replace(item1, StackView.Immediate), item1)
        compare(control.depth, 1)
        compare(control.currentItem, item1)

        // replace([item])
        var item2 = component.createObject(control, {objectName:"2"})
        compare(control.replace([item2], StackView.Immediate), item2)
        compare(control.depth, 1)
        compare(control.currentItem, item2)

        // replace(item, {properties})
        var item3 = component.createObject(control)
        compare(control.replace(item3, {objectName:"3"}, StackView.Immediate), item3)
        compare(item3.objectName, "3")
        compare(control.depth, 1)
        compare(control.currentItem, item3)

        // replace([item, {properties}])
        var item4 = component.createObject(control)
        compare(control.replace([item4, {objectName:"4"}], StackView.Immediate), item4)
        compare(item4.objectName, "4")
        compare(control.depth, 1)
        compare(control.currentItem, item4)

        // replace(component, {properties})
        var item5 = control.replace(component, {objectName:"5"}, StackView.Immediate)
        compare(item5.objectName, "5")
        compare(control.depth, 1)
        compare(control.currentItem, item5)

        // replace([component, {properties}])
        var item6 = control.replace([component, {objectName:"6"}], StackView.Immediate)
        compare(item6.objectName, "6")
        compare(control.depth, 1)
        compare(control.currentItem, item6)

        // replace the topmost item
        control.push(component)
        compare(control.depth, 2)
        var item7 = control.replace(control.get(1), component, StackView.Immediate)
        compare(control.depth, 2)
        compare(control.currentItem, item7)

        // replace the item in the middle
        control.push(component)
        control.push(component)
        control.push(component)
        compare(control.depth, 5)
        var item8 = control.replace(control.get(2), component, StackView.Immediate)
        compare(control.depth, 3)
        compare(control.currentItem, item8)
    }

    function test_clear() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        control.push(component, StackView.Immediate)

        control.clear()
        compare(control.depth, 0)
        compare(control.busy, false)

        control.push(component, StackView.Immediate)

        control.clear(StackView.PopTransition)
        compare(control.depth, 0)
        compare(control.busy, true)
        tryCompare(control, "busy", false)
    }

    function test_visibility_data() {
        return [
            {tag:"default transitions", properties: {}},
            {tag:"null transitions", properties: {pushEnter: null, pushExit: null, popEnter: null, popExit: null}}
        ]
    }

    function test_visibility(data) {
        var control = createTemporaryObject(stackView, testCase, data.properties)
        verify(control)

        var item1 = component.createObject(control)
        control.push(item1, StackView.Immediate)
        verify(item1.visible)

        var item2 = component.createObject(control)
        control.push(item2)
        tryCompare(item1, "visible", false)
        verify(item2.visible)

        control.pop()
        verify(item1.visible)
        tryCompare(item2, "visible", false)
    }

    Component {
        id: transitionView
        StackView {
            property int popEnterRuns
            property int popExitRuns
            property int pushEnterRuns
            property int pushExitRuns
            property int replaceEnterRuns
            property int replaceExitRuns
            popEnter: Transition {
                PauseAnimation { duration: 1 }
                onRunningChanged: if (!running) ++popEnterRuns
            }
            popExit: Transition {
                PauseAnimation { duration: 1 }
                onRunningChanged: if (!running) ++popExitRuns
            }
            pushEnter: Transition {
                PauseAnimation { duration: 1 }
                onRunningChanged: if (!running) ++pushEnterRuns
            }
            pushExit: Transition {
                PauseAnimation { duration: 1 }
                onRunningChanged: if (!running) ++pushExitRuns
            }
            replaceEnter: Transition {
                PauseAnimation { duration: 1 }
                onRunningChanged: if (!running) ++replaceEnterRuns
            }
            replaceExit: Transition {
                PauseAnimation { duration: 1 }
                onRunningChanged: if (!running) ++replaceExitRuns
            }
        }
    }

    function test_transitions_data() {
        return [
            { tag: "undefined", operation: undefined,
              pushEnterRuns: [0,1,1,1], pushExitRuns: [0,1,1,1], replaceEnterRuns: [0,0,1,1], replaceExitRuns: [0,0,1,1], popEnterRuns: [0,0,0,1], popExitRuns: [0,0,0,1] },
            { tag: "immediate", operation: StackView.Immediate,
              pushEnterRuns: [0,0,0,0], pushExitRuns: [0,0,0,0], replaceEnterRuns: [0,0,0,0], replaceExitRuns: [0,0,0,0], popEnterRuns: [0,0,0,0], popExitRuns: [0,0,0,0] },
            { tag: "push", operation: StackView.PushTransition,
              pushEnterRuns: [1,2,3,4], pushExitRuns: [0,1,2,3], replaceEnterRuns: [0,0,0,0], replaceExitRuns: [0,0,0,0], popEnterRuns: [0,0,0,0], popExitRuns: [0,0,0,0] },
            { tag: "pop", operation: StackView.PopTransition,
              pushEnterRuns: [0,0,0,0], pushExitRuns: [0,0,0,0], replaceEnterRuns: [0,0,0,0], replaceExitRuns: [0,0,0,0], popEnterRuns: [1,2,3,4], popExitRuns: [0,1,2,3] },
            { tag: "replace", operation: StackView.ReplaceTransition,
              pushEnterRuns: [0,0,0,0], pushExitRuns: [0,0,0,0], replaceEnterRuns: [1,2,3,4], replaceExitRuns: [0,1,2,3], popEnterRuns: [0,0,0,0], popExitRuns: [0,0,0,0] },
        ]
    }

    function test_transitions(data) {
        var control = createTemporaryObject(transitionView, testCase)
        verify(control)

        control.push(component, data.operation)
        tryCompare(control, "busy", false)
        compare(control.pushEnterRuns, data.pushEnterRuns[0])
        compare(control.pushExitRuns, data.pushExitRuns[0])
        compare(control.replaceEnterRuns, data.replaceEnterRuns[0])
        compare(control.replaceExitRuns, data.replaceExitRuns[0])
        compare(control.popEnterRuns, data.popEnterRuns[0])
        compare(control.popExitRuns, data.popExitRuns[0])

        control.push(component, data.operation)
        tryCompare(control, "busy", false)
        compare(control.pushEnterRuns, data.pushEnterRuns[1])
        compare(control.pushExitRuns, data.pushExitRuns[1])
        compare(control.replaceEnterRuns, data.replaceEnterRuns[1])
        compare(control.replaceExitRuns, data.replaceExitRuns[1])
        compare(control.popEnterRuns, data.popEnterRuns[1])
        compare(control.popExitRuns, data.popExitRuns[1])

        control.replace(component, data.operation)
        tryCompare(control, "busy", false)
        compare(control.pushEnterRuns, data.pushEnterRuns[2])
        compare(control.pushExitRuns, data.pushExitRuns[2])
        compare(control.replaceEnterRuns, data.replaceEnterRuns[2])
        compare(control.replaceExitRuns, data.replaceExitRuns[2])
        compare(control.popEnterRuns, data.popEnterRuns[2])
        compare(control.popExitRuns, data.popExitRuns[2])

        control.pop(data.operation)
        tryCompare(control, "busy", false)
        compare(control.pushEnterRuns, data.pushEnterRuns[3])
        compare(control.pushExitRuns, data.pushExitRuns[3])
        compare(control.replaceEnterRuns, data.replaceEnterRuns[3])
        compare(control.replaceExitRuns, data.replaceExitRuns[3])
        compare(control.popEnterRuns, data.popEnterRuns[3])
        compare(control.popExitRuns, data.popExitRuns[3])
    }

    TestItem {
        id: indestructibleItem
    }

    Component {
        id: destructibleComponent
        TestItem { }
    }

    function test_ownership_data() {
        return [
            {tag:"item, transition", arg: indestructibleItem, operation: StackView.Transition, destroyed: false},
            {tag:"item, immediate", arg: indestructibleItem, operation: StackView.Immediate, destroyed: false},
            {tag:"component, transition", arg: destructibleComponent, operation: StackView.Transition, destroyed: true},
            {tag:"component, immediate", arg: destructibleComponent, operation: StackView.Immediate, destroyed: true},
            {tag:"url, transition", arg: Qt.resolvedUrl("TestItem.qml"), operation: StackView.Transition, destroyed: true},
            {tag:"url, immediate", arg: Qt.resolvedUrl("TestItem.qml"), operation: StackView.Immediate, destroyed: true}
        ]
    }

    function test_ownership(data) {
        var control = createTemporaryObject(transitionView, testCase, {initialItem: component})
        verify(control)

        // push-pop
        control.push(data.arg, StackView.Immediate)
        verify(control.currentItem)
        verify(control.currentItem.hasOwnProperty("destroyedCallback"))
        var destroyed = false
        control.currentItem.destroyedCallback = function() { destroyed = true }
        control.pop(data.operation)
        tryCompare(control, "busy", false)
        wait(0) // deferred delete
        compare(destroyed, data.destroyed)

        // push-replace
        control.push(data.arg, StackView.Immediate)
        verify(control.currentItem)
        verify(control.currentItem.hasOwnProperty("destroyedCallback"))
        destroyed = false
        control.currentItem.destroyedCallback = function() { destroyed = true }
        control.replace(component, data.operation)
        tryCompare(control, "busy", false)
        wait(0) // deferred delete
        compare(destroyed, data.destroyed)
    }

    Component {
        id: removeComponent

        Item {
            objectName: "removeItem"
            StackView.onRemoved: destroy()
        }
    }

    function test_destroyOnRemoved() {
        var control = createTemporaryObject(stackView, testCase, { initialItem: component })
        verify(control)

        var item = removeComponent.createObject(control)
        verify(item)

        var removedSpy = signalSpy.createObject(control, { target: item.StackView, signalName: "removed" })
        verify(removedSpy)
        verify(removedSpy.valid)

        var destructionSpy = signalSpy.createObject(control, { target: item.Component, signalName: "destruction" })
        verify(destructionSpy)
        verify(destructionSpy.valid)

        // push-pop
        control.push(item, StackView.Immediate)
        compare(control.currentItem, item)
        control.pop(StackView.Transition)
        item = null
        tryCompare(removedSpy, "count", 1)
        tryCompare(destructionSpy, "count", 1)
        compare(control.busy, false)

        item = removeComponent.createObject(control)
        verify(item)

        removedSpy.target = item.StackView
        verify(removedSpy.valid)

        destructionSpy.target = item.Component
        verify(destructionSpy.valid)

        // push-replace
        control.push(item, StackView.Immediate)
        compare(control.currentItem, item)
        control.replace(component, StackView.Transition)
        item = null
        tryCompare(removedSpy, "count", 2)
        tryCompare(destructionSpy, "count", 2)
        compare(control.busy, false)
    }

    function test_pushOnRemoved() {
        var control = createTemporaryObject(stackView, testCase, { initialItem: component })
        verify(control)

        var item = control.push(component, StackView.Immediate)
        verify(item)

        item.StackView.onRemoved.connect(function() {
            ignoreWarning(/.*QML StackView: cannot push while already in the process of completing a pop/)
            control.push(component, StackView.Immediate)
        })

        // don't crash (QTBUG-62153)
        control.pop(StackView.Immediate)
    }

    Component {
        id: attachedItem
        Item {
            property int index: StackView.index
            property StackView view: StackView.view
            property int status: StackView.status
        }
    }

    function test_attached() {
        var control = createTemporaryObject(stackView, testCase, {initialItem: attachedItem})

        compare(control.get(0).index, 0)
        compare(control.get(0).view, control)
        compare(control.get(0).status, StackView.Active)

        control.push(attachedItem, StackView.Immediate)

        compare(control.get(0).index, 0)
        compare(control.get(0).view, control)
        compare(control.get(0).status, StackView.Inactive)

        compare(control.get(1).index, 1)
        compare(control.get(1).view, control)
        compare(control.get(1).status, StackView.Active)

        control.pop(StackView.Immediate)

        compare(control.get(0).index, 0)
        compare(control.get(0).view, control)
        compare(control.get(0).status, StackView.Active)
    }

    Component {
        id: testButton
        Button {
            property int clicks: 0
            onClicked: ++clicks
        }
    }

    function test_interaction() {
        var control = createTemporaryObject(stackView, testCase, {initialItem: testButton, width: testCase.width, height: testCase.height})
        verify(control)

        var firstButton = control.currentItem
        verify(firstButton)

        var firstClicks = 0
        var secondClicks = 0
        var thirdClicks = 0

        // push - default transition
        var secondButton = control.push(testButton)
        compare(control.busy, true)
        mouseClick(firstButton) // filtered while busy
        mouseClick(secondButton) // filtered while busy
        compare(firstButton.clicks, firstClicks)
        compare(secondButton.clicks, secondClicks)
        tryCompare(control, "busy", false)
        mouseClick(secondButton)
        compare(secondButton.clicks, ++secondClicks)

        // replace - default transition
        var thirdButton = control.replace(testButton)
        compare(control.busy, true)
        mouseClick(secondButton) // filtered while busy
        mouseClick(thirdButton) // filtered while busy
        compare(secondButton.clicks, secondClicks)
        compare(thirdButton.clicks, thirdClicks)
        tryCompare(control, "busy", false)
        secondButton = null
        secondClicks = 0
        mouseClick(thirdButton)
        compare(thirdButton.clicks, ++thirdClicks)

        // pop - default transition
        control.pop()
        compare(control.busy, true)
        mouseClick(firstButton) // filtered while busy
        mouseClick(thirdButton) // filtered while busy
        compare(firstButton.clicks, firstClicks)
        compare(thirdButton.clicks, thirdClicks)
        tryCompare(control, "busy", false)
        thirdButton = null
        thirdClicks = 0
        mouseClick(firstButton)
        compare(firstButton.clicks, ++firstClicks)

        // push - immediate operation
        secondButton = control.push(testButton, StackView.Immediate)
        compare(control.busy, false)
        mouseClick(secondButton)
        compare(secondButton.clicks, ++secondClicks)

        // replace - immediate operation
        thirdButton = control.replace(testButton, StackView.Immediate)
        compare(control.busy, false)
        secondButton = null
        secondClicks = 0
        mouseClick(thirdButton)
        compare(thirdButton.clicks, ++thirdClicks)

        // pop - immediate operation
        control.pop(StackView.Immediate)
        compare(control.busy, false)
        thirdButton = null
        thirdClicks = 0
        mouseClick(firstButton)
        compare(firstButton.clicks, ++firstClicks)

        // push - null transition
        control.pushEnter = null
        control.pushExit = null
        secondButton = control.push(testButton)
        compare(control.busy, false)
        mouseClick(secondButton)
        compare(secondButton.clicks, ++secondClicks)

        // replace - null transition
        control.replaceEnter = null
        control.replaceExit = null
        thirdButton = control.replace(testButton)
        compare(control.busy, false)
        secondButton = null
        secondClicks = 0
        mouseClick(thirdButton)
        compare(thirdButton.clicks, ++thirdClicks)

        // pop - null transition
        control.popEnter = null
        control.popExit = null
        control.pop()
        compare(control.busy, false)
        thirdButton = null
        thirdClicks = 0
        mouseClick(firstButton)
        compare(firstButton.clicks, ++firstClicks)
    }

    Component {
        id: mouseArea
        MouseArea {
            property int presses: 0
            property int releases: 0
            property int clicks: 0
            property int doubleClicks: 0
            property int cancels: 0
            onPressed: ++presses
            onReleased: ++releases
            onClicked: ++clicks
            onDoubleClicked: ++doubleClicks
            onCanceled: ++cancels
        }
    }

    // QTBUG-50305
    function test_events() {
        var control = createTemporaryObject(stackView, testCase, {initialItem: mouseArea, width: testCase.width, height: testCase.height})
        verify(control)

        var testItem = control.currentItem
        verify(testItem)

        testItem.doubleClicked.connect(function() {
            control.push(mouseArea) // ungrab -> cancel
        })

        mouseDoubleClickSequence(testItem)
        compare(testItem.presses, 2)
        compare(testItem.releases, 2)
        compare(testItem.clicks, 1)
        compare(testItem.doubleClicks, 1)
        compare(testItem.pressed, false)
        compare(testItem.cancels, 0)
    }

    function test_ungrab() {
        var control = createTemporaryObject(stackView, testCase, {initialItem: mouseArea, width: testCase.width, height: testCase.height})
        verify(control)

        var testItem = control.currentItem
        verify(testItem)

        mousePress(testItem)
        control.push(mouseArea)
        tryCompare(control, "busy", false)
        mouseRelease(testItem)

        compare(testItem.presses, 1)
        compare(testItem.releases, 0)
        compare(testItem.clicks, 0)
        compare(testItem.doubleClicks, 0)
        compare(testItem.pressed, false)
        compare(testItem.cancels, 1)
    }

    function test_failures() {
        var control = createTemporaryObject(stackView, testCase, {initialItem: component})
        verify(control)

        var error = Qt.resolvedUrl("non-existent.qml") + ":-1 No such file or directory"

        ignoreWarning("QQmlComponent: Component is not ready")
        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: push: " + error)
        control.push(Qt.resolvedUrl("non-existent.qml"))

        ignoreWarning("QQmlComponent: Component is not ready")
        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: replace: " + error)
        control.replace(Qt.resolvedUrl("non-existent.qml"))

        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: push: invalid url: x://[v]")
        control.push("x://[v]")

        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: replace: invalid url: x://[v]")
        control.replace("x://[v]")

        control.pop()
    }

    Component {
        id: rectangle
        Rectangle {
            property color initialColor
            Component.onCompleted: initialColor = color
        }
    }

    function test_properties() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        var rect = control.push(rectangle, {color: "#ff0000"})
        compare(rect.color, "#ff0000")
        compare(rect.initialColor, "#ff0000")
    }

    Component {
        id: signalTest
        Control {
            id: ctrl
            property SignalSpy activatedSpy: SignalSpy { target: ctrl.StackView; signalName: "activated" }
            property SignalSpy activatingSpy: SignalSpy { target: ctrl.StackView; signalName: "activating" }
            property SignalSpy deactivatedSpy: SignalSpy { target: ctrl.StackView; signalName: "deactivated" }
            property SignalSpy deactivatingSpy: SignalSpy { target: ctrl.StackView; signalName: "deactivating" }
        }
    }

    function test_signals() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        var item1 = signalTest.createObject(control)
        compare(item1.StackView.status, StackView.Inactive)
        control.push(item1)
        compare(item1.StackView.status, StackView.Active)
        compare(item1.activatedSpy.count, 1)
        compare(item1.activatingSpy.count, 1)
        compare(item1.deactivatedSpy.count, 0)
        compare(item1.deactivatingSpy.count, 0)

        var item2 = signalTest.createObject(control)
        compare(item2.StackView.status, StackView.Inactive)
        control.push(item2)
        compare(item2.StackView.status, StackView.Activating)
        compare(item2.activatedSpy.count, 0)
        compare(item2.activatingSpy.count, 1)
        compare(item2.deactivatedSpy.count, 0)
        compare(item2.deactivatingSpy.count, 0)
        compare(item1.StackView.status, StackView.Deactivating)
        compare(item1.activatedSpy.count, 1)
        compare(item1.activatingSpy.count, 1)
        compare(item1.deactivatedSpy.count, 0)
        compare(item1.deactivatingSpy.count, 1)
        tryCompare(item2.activatedSpy, "count", 1)
        tryCompare(item1.deactivatedSpy, "count", 1)

        control.pop()
        compare(item2.StackView.status, StackView.Deactivating)
        compare(item2.activatedSpy.count, 1)
        compare(item2.activatingSpy.count, 1)
        compare(item2.deactivatedSpy.count, 0)
        compare(item2.deactivatingSpy.count, 1)
        compare(item1.StackView.status, StackView.Activating)
        compare(item1.activatedSpy.count, 1)
        compare(item1.activatingSpy.count, 2)
        compare(item1.deactivatedSpy.count, 1)
        compare(item1.deactivatingSpy.count, 1)
        tryCompare(item1.activatedSpy, "count", 2)
    }

    // QTBUG-56158
    function test_repeatedPop() {
        var control = createTemporaryObject(stackView, testCase, {initialItem: component, width: testCase.width, height: testCase.height})
        verify(control)

        for (var i = 0; i < 12; ++i)
            control.push(component)
        tryCompare(control, "busy", false)

        while (control.depth > 1) {
            control.pop()
            wait(50)
        }
        tryCompare(control, "busy", false)
    }

    function test_pushSameItem() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        control.push(item, StackView.Immediate)
        compare(control.currentItem, item)
        compare(control.depth, 1)

        // Pushing the same Item should do nothing.
        ignoreWarning(Qt.resolvedUrl("tst_stackview.qml") + ":69:9: QML StackView: push: nothing to push")
        control.push(item, StackView.Immediate)
        compare(control.currentItem, item)
        compare(control.depth, 1)

        // Push a component so that it becomes current.
        var current = control.push(component, StackView.Immediate)
        compare(control.currentItem, current)
        compare(control.depth, 2)

        // Push a bunch of stuff. "item" is already in the stack, so it should be ignored.
        current = control.push(component, item, StackView.Immediate)
        verify(current !== item)
        compare(control.currentItem, current)
        compare(control.depth, 3)
    }

    function test_visible() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        var item1 = component.createObject(control)
        control.push(item1, StackView.Immediate)
        compare(item1.visible, true)
        compare(item1.StackView.visible, item1.visible)

        var item2 = component.createObject(control)
        control.push(item2, StackView.Immediate)
        compare(item1.visible, false)
        compare(item2.visible, true)
        compare(item1.StackView.visible, false)
        compare(item2.StackView.visible, true)

        // keep explicitly visible
        item2.StackView.visible = true
        control.push(component, StackView.Immediate)
        compare(item2.visible, true)
        compare(item2.StackView.visible, true)

        // show underneath
        item1.StackView.visible = true
        compare(item1.visible, true)
        compare(item1.StackView.visible, true)

        control.pop(StackView.Immediate)
        compare(item2.visible, true)
        compare(item2.StackView.visible, true)

        // hide the top-most
        item2.StackView.visible = false
        compare(item2.visible, false)
        compare(item2.StackView.visible, false)

        // reset the top-most
        item2.StackView.visible = undefined
        compare(item2.visible, true)
        compare(item2.StackView.visible, true)

        // reset underneath
        item1.StackView.visible = undefined
        compare(item1.visible, false)
        compare(item1.StackView.visible, false)

        control.pop(StackView.Immediate)
        compare(item1.visible, true)
        compare(item1.StackView.visible, true)
    }

    function test_resolveInitialItem() {
        var control = createTemporaryObject(stackView, testCase, {initialItem: "TestItem.qml"})
        verify(control)
        verify(control.currentItem)
    }

    function test_resolve() {
        var control = createTemporaryObject(stackView, testCase)
        verify(control)

        var item = control.push("TestItem.qml")
        compare(control.depth, 1)
        verify(item)
    }

    // QTBUG-65084
    function test_mouseArea() {
        var ma = createTemporaryObject(mouseArea, testCase, {width: testCase.width, height: testCase.height})
        verify(ma)

        var control = stackView.createObject(ma, {width: testCase.width, height: testCase.height})
        verify(control)

        mousePress(control)
        verify(ma.pressed)

        mouseRelease(control)
        verify(!ma.pressed)

        var touch = touchEvent(control)
        touch.press(0, control).commit()
        verify(ma.pressed)

        touch.release(0, control).commit()
        verify(!ma.pressed)
    }

    // Separate function to ensure that the temporary value created to hold the return value of the Qt.createComponent()
    // call is out of scope when the caller calls gc().
    function stackViewFactory()
    {
        return createTemporaryObject(stackView, testCase, {initialItem: Qt.createComponent("TestItem.qml")})
    }

    function test_initalItemOwnership()
    {
        var control = stackViewFactory()
        verify(control)
        gc()
        verify(control.initialItem)
    }

    // Need to use this specific structure in order to reproduce the crash.
    Component {
        id: clearUponDestructionContainerComponent

        Item {
            id: container
            objectName: "container"

            property alias control: stackView
            property var onDestructionCallback

            property Component clearUponDestructionComponent: Component {
                id: clearUponDestructionComponent

                Item {
                    objectName: "clearUponDestructionItem"
                    Component.onDestruction: container.onDestructionCallback(stackView)
                }
            }

            StackView {
                id: stackView
                initialItem: Item {
                    objectName: "initialItem"
                }
            }
        }
    }

    // QTBUG-80353
    // Tests that calling clear() in Component.onDestruction in response to that
    // item being removed (e.g. via an earlier call to clear()) results in a warning and not a crash.
    function test_recursiveClearClear() {
        let container = createTemporaryObject(clearUponDestructionContainerComponent, testCase,
            { onDestructionCallback: function(stackView) { stackView.clear(StackView.Immediate) }})
        verify(container)

        let control = container.control
        control.push(container.clearUponDestructionComponent, StackView.Immediate)

        // Shouldn't crash.
        ignoreWarning(/.*cannot clear while already in the process of completing a clear/)
        control.clear(StackView.Immediate)
    }

    function test_recursivePopClear() {
        let container = createTemporaryObject(clearUponDestructionContainerComponent, testCase,
            { onDestructionCallback: function(stackView) { stackView.clear(StackView.Immediate) }})
        verify(container)

        let control = container.control
        control.push(container.clearUponDestructionComponent, StackView.Immediate)

        // Pop all items except the first, removing the second item we pushed in the process.
        // Shouldn't crash.
        ignoreWarning(/.*cannot clear while already in the process of completing a pop/)
        control.pop(null, StackView.Immediate)
    }

    function test_recursivePopPop() {
        let container = createTemporaryObject(clearUponDestructionContainerComponent, testCase,
            { onDestructionCallback: function(stackView) { stackView.pop(null, StackView.Immediate) }})
        verify(container)

        let control = container.control
        // Push an extra item so that we can call pop(null) and reproduce the conditions for the crash.
        control.push(component, StackView.Immediate)
        control.push(container.clearUponDestructionComponent, StackView.Immediate)

        // Pop the top item, then pop down to the first item in response.
        ignoreWarning(/.*cannot pop while already in the process of completing a pop/)
        control.pop(StackView.Immediate)
    }

    function test_recursiveReplaceClear() {
        let container = createTemporaryObject(clearUponDestructionContainerComponent, testCase,
            { onDestructionCallback: function(stackView) { stackView.clear(StackView.Immediate) }})
        verify(container)

        let control = container.control
        control.push(container.clearUponDestructionComponent, StackView.Immediate)

        // Replace the top item, then clear in response.
        ignoreWarning(/.*cannot clear while already in the process of completing a replace/)
        control.replace(component, StackView.Immediate)
    }

    function test_recursiveClearReplace() {
        let container = createTemporaryObject(clearUponDestructionContainerComponent, testCase,
            { onDestructionCallback: function(stackView) { stackView.replace(component, StackView.Immediate) }})
        verify(container)

        let control = container.control
        control.push(container.clearUponDestructionComponent, StackView.Immediate)

        // Replace the top item, then clear in response.
        ignoreWarning(/.*cannot replace while already in the process of completing a clear/)
        control.clear(StackView.Immediate)
    }

    // QTBUG-84381
    function test_clearAndPushAfterDepthChange() {
        var control = createTemporaryObject(stackView, testCase, {
            popEnter: null, popExit: null, pushEnter: null,
            pushExit: null, replaceEnter: null, replaceExit: null
        })
        verify(control)

        control.depthChanged.connect(function() {
            if (control.depth === 2) {
                // Shouldn't assert.
                ignoreWarning(/.*QML StackView: cannot clear while already in the process of completing a push/)
                control.clear()
                // Shouldn't crash.
                ignoreWarning(/.*QML StackView: cannot push while already in the process of completing a push/)
                control.push(component)
            }
        })

        control.push(component)
        control.push(component)
    }
}
