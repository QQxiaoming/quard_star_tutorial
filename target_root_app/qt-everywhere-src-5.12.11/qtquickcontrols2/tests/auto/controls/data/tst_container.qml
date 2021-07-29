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
import QtQuick.Templates 2.12 as T

TestCase {
    id: testCase
    width: 400
    height: 400
    visible: true
    when: windowShown
    name: "Container"

    Component {
        id: container
        Container { }
    }

    Component {
        id: rectangle
        Rectangle { }
    }

    function test_implicitSize() {
        var control = createTemporaryObject(container, testCase)
        verify(control)

        compare(control.implicitWidth, 0)
        compare(control.implicitHeight, 0)

        control.contentItem = rectangle.createObject(control, {implicitWidth: 10, implicitHeight: 20})
        compare(control.implicitWidth, 10)
        compare(control.implicitHeight, 20)

        control.background = rectangle.createObject(control, {implicitWidth: 20, implicitHeight: 30})
        compare(control.implicitWidth, 20)
        compare(control.implicitHeight, 30)

        control.padding = 100
        compare(control.implicitWidth, 210)
        compare(control.implicitHeight, 220)
    }

    function test_currentIndex() {
        var control1 = createTemporaryObject(container, testCase)
        verify(control1)

        var control2 = createTemporaryObject(container, testCase)
        verify(control2)

        compare(control1.currentIndex, -1)
        compare(control2.currentIndex, -1)

        for (var i = 0; i < 3; ++i) {
            control1.addItem(rectangle.createObject(control1))
            control2.addItem(rectangle.createObject(control2))
        }

        compare(control1.count, 3)
        compare(control2.count, 3)
        compare(control1.currentIndex, 0)
        compare(control2.currentIndex, 0)

        control1.currentIndex = Qt.binding(function() { return control2.currentIndex })
        control2.currentIndex = Qt.binding(function() { return control1.currentIndex })

        control1.setCurrentIndex(1)
        compare(control1.currentIndex, 1)
        compare(control2.currentIndex, 1)

        control1.incrementCurrentIndex()
        compare(control1.currentIndex, 2)
        compare(control2.currentIndex, 2)

        control2.decrementCurrentIndex()
        compare(control1.currentIndex, 1)
        compare(control2.currentIndex, 1)
    }

    Component {
        id: repeaterContainer1
        Container {
            id: container
            Item { objectName: "0" }
            Item { objectName: "1" }
            Item { objectName: "2" }
            Item { objectName: "3" }
            contentItem: Row {
                Repeater {
                    model: container.contentModel
                }
            }
        }
    }

    Component {
        id: repeaterContainer2
        Container {
            id: container
            contentItem: Item {
                Repeater {
                    model: container.contentModel
                }
                Rectangle { objectName: "extra" }
            }
            Rectangle { objectName: "0" }
            Rectangle { objectName: "1" }
            Rectangle { objectName: "2" }
            Rectangle { objectName: "3" }
        }
    }

    function test_repeater_data() {
        return [
            { tag: "1", component: repeaterContainer1 },
            { tag: "2", component: repeaterContainer2 }
        ]
    }

    // don't crash (QTBUG-61310)
    function test_repeater(data) {
        var control = createTemporaryObject(data.component)
        verify(control)

        compare(control.itemAt(0).objectName, "0")
        compare(control.itemAt(1).objectName, "1")
        compare(control.itemAt(2).objectName, "2")
        compare(control.itemAt(3).objectName, "3")
    }

    function test_removeTakeItem() {
        var control = createTemporaryObject(container, testCase)
        verify(control)

        var item1 = rectangle.createObject(control)
        var item2 = rectangle.createObject(control)
        var item3 = rectangle.createObject(control)

        item1.Component.onDestruction.connect(function() { item1 = null })
        item2.Component.onDestruction.connect(function() { item2 = null })
        item3.Component.onDestruction.connect(function() { item3 = null })

        control.addItem(item1)
        control.addItem(item2)
        control.addItem(item3)
        compare(control.count, 3)

        // takeItem(int) does not destroy
        compare(control.takeItem(1), item2)
        compare(control.count, 2)
        wait(1)
        verify(item2)

        // removeItem(Item) destroys
        control.removeItem(item1)
        compare(control.count, 1)
        wait(1)
        verify(!item1)

        // removeItem(null) must not call removeItem(0)
        control.removeItem(null)
        compare(control.count, 1)
        wait(1)
        verify(item3)

        // deprecated removeItem(int) does not destroy
        control.removeItem(0)
        compare(control.count, 0)
        wait(1)
        verify(item3)
    }
}
