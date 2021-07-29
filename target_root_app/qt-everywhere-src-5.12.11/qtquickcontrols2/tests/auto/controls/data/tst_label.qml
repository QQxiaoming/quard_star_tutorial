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
    width: 400
    height: 400
    visible: true
    when: windowShown
    name: "Label"

    Component {
        id: label
        Label { }
    }

    Component {
        id: backgroundLabel
        Label {
            background: Rectangle { }
        }
    }

    Component {
        id: rectangle
        Rectangle { }
    }

    Component {
        id: signalSpy
        SignalSpy { }
    }

    function test_creation() {
        var control = createTemporaryObject(label, testCase)
        verify(control)
    }

    function test_font_explicit_attributes_data() {
        return [
            {tag: "bold", value: true},
            {tag: "capitalization", value: Font.Capitalize},
            {tag: "family", value: "Courier"},
            {tag: "italic", value: true},
            {tag: "strikeout", value: true},
            {tag: "underline", value: true},
            {tag: "weight", value: Font.Black},
            {tag: "wordSpacing", value: 55}
        ]
    }

    function test_font_explicit_attributes(data) {
        var control = createTemporaryObject(label, testCase)
        verify(control)

        var child = label.createObject(control)
        verify(child)

        var controlSpy = signalSpy.createObject(control, {target: control, signalName: "fontChanged"})
        verify(controlSpy.valid)

        var childSpy = signalSpy.createObject(child, {target: child, signalName: "fontChanged"})
        verify(childSpy.valid)

        var defaultValue = control.font[data.tag]
        child.font[data.tag] = defaultValue

        compare(child.font[data.tag], defaultValue)
        compare(childSpy.count, 0)

        control.font[data.tag] = data.value

        compare(control.font[data.tag], data.value)
        compare(controlSpy.count, 1)

        compare(child.font[data.tag], defaultValue)
        compare(childSpy.count, 0)
    }

    function test_background() {
        var control = createTemporaryObject(backgroundLabel, testCase, {text: "Label"})
        verify(control)

        compare(control.background.width, control.width)
        compare(control.background.height, control.height)

        control.background = rectangle.createObject(control)
        compare(control.background.width, control.width)
        compare(control.background.height, control.height)

        // change implicit size (QTBUG-66455)
        control.background.implicitWidth = 160
        control.background.implicitHeight = 120
        compare(control.background.width, control.width)
        compare(control.background.height, control.height)
    }

    function test_inset() {
        var control = createTemporaryObject(label, testCase, {background: rectangle.createObject(control)})
        verify(control)

        var topInsetSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "topInsetChanged"})
        verify(topInsetSpy.valid)

        var leftInsetSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "leftInsetChanged"})
        verify(leftInsetSpy.valid)

        var rightInsetSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "rightInsetChanged"})
        verify(rightInsetSpy.valid)

        var bottomInsetSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "bottomInsetChanged"})
        verify(bottomInsetSpy.valid)

        var topInsetChanges = 0
        var leftInsetChanges = 0
        var rightInsetChanges = 0
        var bottomInsetChanges = 0

        compare(control.topInset, 0)
        compare(control.leftInset, 0)
        compare(control.rightInset, 0)
        compare(control.bottomInset, 0)

        control.width = 100
        control.height = 100
        compare(control.background.x, 0)
        compare(control.background.y, 0)
        compare(control.background.width, 100)
        compare(control.background.height, 100)

        control.topInset = 10
        compare(control.topInset, 10)
        compare(control.leftInset, 0)
        compare(control.rightInset, 0)
        compare(control.bottomInset, 0)
        compare(topInsetSpy.count, ++topInsetChanges)
        compare(leftInsetSpy.count, leftInsetChanges)
        compare(rightInsetSpy.count, rightInsetChanges)
        compare(bottomInsetSpy.count, bottomInsetChanges)
        compare(control.background.x, 0)
        compare(control.background.y, 10)
        compare(control.background.width, 100)
        compare(control.background.height, 90)

        control.leftInset = 20
        compare(control.topInset, 10)
        compare(control.leftInset, 20)
        compare(control.rightInset, 0)
        compare(control.bottomInset, 0)
        compare(topInsetSpy.count, topInsetChanges)
        compare(leftInsetSpy.count, ++leftInsetChanges)
        compare(rightInsetSpy.count, rightInsetChanges)
        compare(bottomInsetSpy.count, bottomInsetChanges)
        compare(control.background.x, 20)
        compare(control.background.y, 10)
        compare(control.background.width, 80)
        compare(control.background.height, 90)

        control.rightInset = 30
        compare(control.topInset, 10)
        compare(control.leftInset, 20)
        compare(control.rightInset, 30)
        compare(control.bottomInset, 0)
        compare(topInsetSpy.count, topInsetChanges)
        compare(leftInsetSpy.count, leftInsetChanges)
        compare(rightInsetSpy.count, ++rightInsetChanges)
        compare(bottomInsetSpy.count, bottomInsetChanges)
        compare(control.background.x, 20)
        compare(control.background.y, 10)
        compare(control.background.width, 50)
        compare(control.background.height, 90)

        control.bottomInset = 40
        compare(control.topInset, 10)
        compare(control.leftInset, 20)
        compare(control.rightInset, 30)
        compare(control.bottomInset, 40)
        compare(topInsetSpy.count, topInsetChanges)
        compare(leftInsetSpy.count, leftInsetChanges)
        compare(rightInsetSpy.count, rightInsetChanges)
        compare(bottomInsetSpy.count, ++bottomInsetChanges)
        compare(control.background.x, 20)
        compare(control.background.y, 10)
        compare(control.background.width, 50)
        compare(control.background.height, 50)

        control.topInset = undefined
        compare(control.topInset, 0)
        compare(control.leftInset, 20)
        compare(control.rightInset, 30)
        compare(control.bottomInset, 40)
        compare(topInsetSpy.count, ++topInsetChanges)
        compare(leftInsetSpy.count, leftInsetChanges)
        compare(rightInsetSpy.count, rightInsetChanges)
        compare(bottomInsetSpy.count, bottomInsetChanges)
        compare(control.background.x, 20)
        compare(control.background.y, 0)
        compare(control.background.width, 50)
        compare(control.background.height, 60)

        control.leftInset = undefined
        compare(control.topInset, 0)
        compare(control.leftInset, 0)
        compare(control.rightInset, 30)
        compare(control.bottomInset, 40)
        compare(topInsetSpy.count, topInsetChanges)
        compare(leftInsetSpy.count, ++leftInsetChanges)
        compare(rightInsetSpy.count, rightInsetChanges)
        compare(bottomInsetSpy.count, bottomInsetChanges)
        compare(control.background.x, 0)
        compare(control.background.y, 0)
        compare(control.background.width, 70)
        compare(control.background.height, 60)

        control.rightInset = undefined
        compare(control.topInset, 0)
        compare(control.leftInset, 0)
        compare(control.rightInset, 0)
        compare(control.bottomInset, 40)
        compare(topInsetSpy.count, topInsetChanges)
        compare(leftInsetSpy.count, leftInsetChanges)
        compare(rightInsetSpy.count, ++rightInsetChanges)
        compare(bottomInsetSpy.count, bottomInsetChanges)
        compare(control.background.x, 0)
        compare(control.background.y, 0)
        compare(control.background.width, 100)
        compare(control.background.height, 60)

        control.bottomInset = undefined
        compare(control.topInset, 0)
        compare(control.leftInset, 0)
        compare(control.rightInset, 0)
        compare(control.bottomInset, 0)
        compare(topInsetSpy.count, topInsetChanges)
        compare(leftInsetSpy.count, leftInsetChanges)
        compare(rightInsetSpy.count, rightInsetChanges)
        compare(bottomInsetSpy.count, ++bottomInsetChanges)
        compare(control.background.x, 0)
        compare(control.background.y, 0)
        compare(control.background.width, 100)
        compare(control.background.height, 100)
    }
}
