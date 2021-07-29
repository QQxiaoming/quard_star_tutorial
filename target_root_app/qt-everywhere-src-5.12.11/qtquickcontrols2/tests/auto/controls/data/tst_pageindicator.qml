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
    name: "PageIndicator"

    Component {
        id: pageIndicator
        PageIndicator { }
    }

    Component {
        id: mouseArea
        MouseArea { }
    }

    function test_count() {
        var control = createTemporaryObject(pageIndicator, testCase)
        verify(control)

        compare(control.count, 0)
        control.count = 3
        compare(control.count, 3)
    }

    function test_currentIndex() {
        var control = createTemporaryObject(pageIndicator, testCase)
        verify(control)

        compare(control.currentIndex, 0)
        control.currentIndex = 5
        compare(control.currentIndex, 5)
    }

    function test_interactive_data() {
        return [
            { tag: "mouse", touch: false },
            { tag: "touch", touch: true }
        ]
    }

    function test_interactive(data) {
        var control = createTemporaryObject(pageIndicator, testCase, {count: 5, spacing: 10, topPadding: 10, leftPadding: 10, rightPadding: 10, bottomPadding: 10})
        verify(control)

        verify(!control.interactive)
        compare(control.currentIndex, 0)

        var touch = touchEvent(control)

        if (data.touch)
            touch.press(0, control).commit().release(0, control).commit()
        else
            mouseClick(control, control.width / 2, control.height / 2, Qt.LeftButton)
        compare(control.currentIndex, 0)

        control.interactive = true
        verify(control.interactive)

        if (data.touch)
            touch.press(0, control).commit().release(0, control).commit()
        else
            mouseClick(control, control.width / 2, control.height / 2, Qt.LeftButton)
        compare(control.currentIndex, 2)

        // test also clicking outside delegates => the nearest should be selected
        for (var i = 0; i < control.count; ++i) {
            var child = control.contentItem.children[i]

            var points = [
                Qt.point(child.width / 2, -2), // top
                Qt.point(-2, child.height / 2), // left
                Qt.point(child.width + 2, child.height / 2), // right
                Qt.point(child.width / 2, child.height + 2), // bottom

                Qt.point(-2, -2), // top-left
                Qt.point(child.width + 2, -2), // top-right
                Qt.point(-2, child.height + 2), // bottom-left
                Qt.point(child.width + 2, child.height + 2), // bottom-right
            ]

            for (var j = 0; j < points.length; ++j) {
                control.currentIndex = -1
                compare(control.currentIndex, -1)

                var point = points[j]
                var pos = control.mapFromItem(child, x, y)
                if (data.touch)
                    touch.press(0, control, pos.x, pos.y).commit().release(0, control, pos.x, pos.y).commit()
                else
                    mouseClick(control, pos.x, pos.y, Qt.LeftButton)
                compare(control.currentIndex, i)
            }
        }
    }

    function test_mouseArea_data() {
        return [
            { tag: "interactive", interactive: true },
            { tag: "non-interactive", interactive: false }
        ]
    }

    // QTBUG-61785
    function test_mouseArea(data) {
        var ma = createTemporaryObject(mouseArea, testCase, {width: testCase.width, height: testCase.height})
        verify(ma)

        var control = pageIndicator.createObject(ma, {count: 5, interactive: data.interactive, width: testCase.width, height: testCase.height})
        verify(control)

        compare(control.interactive, data.interactive)

        mousePress(control)
        compare(ma.pressed, !data.interactive)

        mouseRelease(control)
        verify(!ma.pressed)

        var touch = touchEvent(control)
        touch.press(0, control).commit()
        compare(ma.pressed, !data.interactive)

        touch.release(0, control).commit()
        verify(!ma.pressed)
    }
}
