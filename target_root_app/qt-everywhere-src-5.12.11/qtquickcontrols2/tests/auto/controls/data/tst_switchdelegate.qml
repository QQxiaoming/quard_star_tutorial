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
    name: "SwitchDelegate"

    Component {
        id: switchDelegate
        SwitchDelegate {}
    }

    Component {
        id: signalSequenceSpy
        SignalSequenceSpy {
            signals: ["pressed", "released", "canceled", "clicked", "toggled", "pressedChanged", "checkedChanged"]
        }
    }

    // TODO: data-fy tst_checkbox (rename to tst_check?) so we don't duplicate its tests here?

    function test_defaults() {
        var control = createTemporaryObject(switchDelegate, testCase);
        verify(control);
        verify(!control.checked);
    }

    function test_checked() {
        var control = createTemporaryObject(switchDelegate, testCase);
        verify(control);

        mouseClick(control);
        verify(control.checked);

        mouseClick(control);
        verify(!control.checked);
    }

    function test_baseline() {
        var control = createTemporaryObject(switchDelegate, testCase);
        verify(control);
        compare(control.baselineOffset, control.contentItem.y + control.contentItem.baselineOffset);
    }

    function test_pressed_data() {
        return [
            { tag: "indicator", x: 15 },
            { tag: "background", x: 5 }
        ]
    }

    function test_pressed(data) {
        var control = createTemporaryObject(switchDelegate, testCase, {padding: 10})
        verify(control)

        // stays pressed when dragged outside
        compare(control.pressed, false)
        mousePress(control, data.x, control.height / 2, Qt.LeftButton)
        compare(control.pressed, true)
        mouseMove(control, -1, control.height / 2)
        compare(control.pressed, true)
        mouseRelease(control, -1, control.height / 2, Qt.LeftButton)
        compare(control.pressed, false)
    }

    function test_mouse() {
        var control = createTemporaryObject(switchDelegate, testCase)
        verify(control)

        // check
        var spy = signalSequenceSpy.createObject(control, {target: control})
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": false }],
                                "pressed"]
        mousePress(control, control.width / 2, control.height / 2, Qt.LeftButton)
        compare(control.pressed, true)
        verify(spy.success)
        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": false }],
                                ["checkedChanged", { "pressed": false, "checked": true }],
                                "toggled",
                                "released",
                                "clicked"]
        mouseRelease(control, control.width / 2, control.height / 2, Qt.LeftButton)
        compare(control.checked, true)
        compare(control.pressed, false)
        verify(spy.success)

        // uncheck
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": true }],
                                "pressed"]
        mousePress(control, control.width / 2, control.height / 2, Qt.LeftButton)
        compare(control.pressed, true)
        verify(spy.success)
        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": true }],
                                ["checkedChanged", { "pressed": false, "checked": false }],
                                "toggled",
                                "released",
                                "clicked"]
        mouseRelease(control, control.width / 2, control.height / 2, Qt.LeftButton)
        compare(control.checked, false)
        compare(control.pressed, false)
        verify(spy.success)

        // release on the right
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": false }],
                                "pressed"]
        mousePress(control, control.width / 2, control.height / 2, Qt.LeftButton)
        compare(control.pressed, true)
        verify(spy.success)
        mouseMove(control, control.width * 2, control.height / 2, 0)
        compare(control.pressed, true)
        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": false }],
                                ["checkedChanged", { "pressed": false, "checked": true }],
                                "toggled",
                                "released",
                                "clicked"]
        mouseRelease(control, control.width * 2, control.height / 2, Qt.LeftButton)
        compare(control.checked, true)
        compare(control.pressed, false)
        verify(spy.success)

        // release on the left
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": true }],
                                "pressed"]
        mousePress(control, control.width / 2, control.height / 2, Qt.LeftButton)
        compare(control.pressed, true)
        verify(spy.success)
        mouseMove(control, -control.width, control.height / 2, 0)
        compare(control.pressed, true)
        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": true }],
                                ["checkedChanged", { "pressed": false, "checked": false }],
                                "toggled",
                                "released",
                                "clicked"]
        mouseRelease(control, -control.width, control.height / 2, Qt.LeftButton)
        compare(control.checked, false)
        compare(control.pressed, false)
        verify(spy.success)

        // release in the middle
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": false }],
                                "pressed"]
        mousePress(control, 0, 0, Qt.LeftButton)
        compare(control.pressed, true)
        verify(spy.success)
        mouseMove(control, control.width / 2, control.height / 2, 0, Qt.LeftButton)
        compare(control.pressed, true)
        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": false }],
                                "released",
                                "clicked"]
        mouseRelease(control, control.width / 2, control.height / 2, Qt.LeftButton)
        compare(control.checked, false)
        compare(control.pressed, false)
        tryCompare(control, "position", 0) // QTBUG-57944
        verify(spy.success)

        // right button
        spy.expectedSequence = []
        mousePress(control, control.width / 2, control.height / 2, Qt.RightButton)
        compare(control.pressed, false)
        verify(spy.success)
        mouseRelease(control, control.width / 2, control.height / 2, Qt.RightButton)
        compare(control.checked, false)
        compare(control.pressed, false)
        verify(spy.success)
    }

    function test_touch() {
        var control = createTemporaryObject(switchDelegate, testCase)
        verify(control)

        var touch = touchEvent(control)

        // check
        var spy = signalSequenceSpy.createObject(control, {target: control})
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": false }],
                                "pressed"]
        touch.press(0, control, control.width / 2, control.height / 2).commit()
        compare(control.pressed, true)
        verify(spy.success)
        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": false }],
                                ["checkedChanged", { "pressed": false, "checked": true }],
                                "toggled",
                                "released",
                                "clicked"]
        touch.release(0, control, control.width / 2, control.height / 2).commit()
        compare(control.checked, true)
        compare(control.pressed, false)
        verify(spy.success)

        // uncheck
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": true }],
                                "pressed"]
        touch.press(0, control, control.width / 2, control.height / 2).commit()
        compare(control.pressed, true)
        verify(spy.success)
        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": true }],
                                ["checkedChanged", { "pressed": false, "checked": false }],
                                "toggled",
                                "released",
                                "clicked"]
        touch.release(0, control, control.width / 2, control.height / 2).commit()
        compare(control.checked, false)
        compare(control.pressed, false)
        verify(spy.success)

        // release on the right
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": false }],
                                "pressed"]
        touch.press(0, control, control.width / 2, control.height / 2).commit()
        compare(control.pressed, true)
        verify(spy.success)
        touch.move(0, control, control.width * 2, control.height / 2).commit()
        compare(control.pressed, true)
        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": false }],
                                ["checkedChanged", { "pressed": false, "checked": true }],
                                "toggled",
                                "released",
                                "clicked"]
        touch.release(0, control, control.width * 2, control.height / 2).commit()
        compare(control.checked, true)
        compare(control.pressed, false)
        verify(spy.success)

        // release on the left
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": true }],
                                "pressed"]
        touch.press(0, control, control.width / 2, control.height / 2).commit()
        compare(control.pressed, true)
        verify(spy.success)
        touch.move(0, control, -control.width, control.height / 2).commit()
        compare(control.pressed, true)
        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": true }],
                                ["checkedChanged", { "pressed": false, "checked": false }],
                                "toggled",
                                "released",
                                "clicked"]
        touch.release(0, control, -control.width, control.height / 2).commit()
        compare(control.checked, false)
        compare(control.pressed, false)
        verify(spy.success)

        // release in the middle
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": false }],
                                "pressed"]
        touch.press(0, control, 0, 0).commit()
        compare(control.pressed, true)
        verify(spy.success)
        touch.move(0, control, control.width / 2, control.height / 2).commit()
        compare(control.pressed, true)
        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": false }],
                                "released",
                                "clicked"]
        touch.release(0, control, control.width / 2, control.height / 2).commit()
        compare(control.checked, false)
        compare(control.pressed, false)
        tryCompare(control, "position", 0) // QTBUG-57944
        verify(spy.success)
    }

    function test_mouseDrag() {
        var control = createTemporaryObject(switchDelegate, testCase, {leftPadding: 100, rightPadding: 100})
        verify(control)

        var spy = signalSequenceSpy.createObject(control, {target: control})
        compare(control.position, 0.0)
        compare(control.checked, false)
        compare(control.pressed, false)

        // press-drag-release inside the indicator
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": false }],
                                "pressed"]
        mousePress(control.indicator, 0)
        compare(control.position, 0.0)
        compare(control.checked, false)
        compare(control.pressed, true)
        verify(spy.success)

        mouseMove(control.indicator, control.width)
        compare(control.position, 1.0)
        compare(control.checked, false)
        compare(control.pressed, true)

        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": false }],
                                ["checkedChanged", { "pressed": false, "checked": true }],
                                "toggled",
                                "released",
                                "clicked"]
        mouseRelease(control.indicator, control.indicator.width)
        compare(control.position, 1.0)
        compare(control.checked, true)
        compare(control.pressed, false)
        verify(spy.success)

        // press-drag-release outside the indicator
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": true }],
                                "pressed"]
        mousePress(control, 0)
        compare(control.position, 1.0)
        compare(control.checked, true)
        compare(control.pressed, true)
        verify(spy.success)

        mouseMove(control, control.width - control.rightPadding)
        compare(control.position, 1.0)
        compare(control.checked, true)
        compare(control.pressed, true)

        mouseMove(control, control.width / 2)
        compare(control.position, 0.5)
        compare(control.checked, true)
        compare(control.pressed, true)

        mouseMove(control, control.leftPadding)
        compare(control.position, 0.0)
        compare(control.checked, true)
        compare(control.pressed, true)

        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": true }],
                                ["checkedChanged", { "pressed": false, "checked": false }],
                                "toggled",
                                "released",
                                "clicked"]
        mouseRelease(control, control.width)
        compare(control.position, 0.0)
        compare(control.checked, false)
        compare(control.pressed, false)
        verify(spy.success)

        // press-drag-release from and to outside the indicator
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": false }],
                                "pressed"]
        mousePress(control, control.width)
        compare(control.position, 0.0)
        compare(control.checked, false)
        compare(control.pressed, true)
        verify(spy.success)

        mouseMove(control, control.width - control.rightPadding)
        compare(control.position, 0.0)
        compare(control.checked, false)
        compare(control.pressed, true)

        mouseMove(control, control.width / 2)
        compare(control.position, 0.5)
        compare(control.checked, false)
        compare(control.pressed, true)

        mouseMove(control, control.width - control.rightPadding)
        compare(control.position, 1.0)
        compare(control.checked, false)
        compare(control.pressed, true)

        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": false }],
                                ["checkedChanged", { "pressed": false, "checked": true }],
                                "toggled",
                                "released",
                                "clicked"]
        mouseRelease(control, control.width)
        compare(control.position, 1.0)
        compare(control.checked, true)
        compare(control.pressed, false)
        verify(spy.success)
    }

    function test_touchDrag() {
        var control = createTemporaryObject(switchDelegate, testCase, {leftPadding: 100, rightPadding: 100})
        verify(control)

        var touch = touchEvent(control)

        var spy = signalSequenceSpy.createObject(control, {target: control})
        compare(control.position, 0.0)
        compare(control.checked, false)
        compare(control.pressed, false)

        // press-drag-release inside the indicator
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": false }],
                                "pressed"]
        touch.press(0, control.indicator, 0).commit()
        compare(control.position, 0.0)
        compare(control.checked, false)
        compare(control.pressed, true)
        verify(spy.success)

        touch.move(0, control.indicator, control.width).commit()
        compare(control.position, 1.0)
        compare(control.checked, false)
        compare(control.pressed, true)

        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": false }],
                                ["checkedChanged", { "pressed": false, "checked": true }],
                                "toggled",
                                "released",
                                "clicked"]
        touch.release(0, control.indicator, control.indicator.width).commit()
        compare(control.position, 1.0)
        compare(control.checked, true)
        compare(control.pressed, false)
        verify(spy.success)

        // press-drag-release outside the indicator
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": true }],
                                "pressed"]
        touch.press(0, control, 0).commit()
        compare(control.position, 1.0)
        compare(control.checked, true)
        compare(control.pressed, true)
        verify(spy.success)

        touch.move(0, control, control.width - control.rightPadding).commit()
        compare(control.position, 1.0)
        compare(control.checked, true)
        compare(control.pressed, true)

        touch.move(0, control, control.width / 2).commit()
        compare(control.position, 0.5)
        compare(control.checked, true)
        compare(control.pressed, true)

        touch.move(0, control, control.leftPadding).commit()
        compare(control.position, 0.0)
        compare(control.checked, true)
        compare(control.pressed, true)

        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": true }],
                                ["checkedChanged", { "pressed": false, "checked": false }],
                                "toggled",
                                "released",
                                "clicked"]
        touch.release(0, control, control.width).commit()
        compare(control.position, 0.0)
        compare(control.checked, false)
        compare(control.pressed, false)
        verify(spy.success)

        // press-drag-release from and to outside the indicator
        spy.expectedSequence = [["pressedChanged", { "pressed": true, "checked": false }],
                                "pressed"]
        touch.press(0, control, control.width).commit()
        compare(control.position, 0.0)
        compare(control.checked, false)
        compare(control.pressed, true)
        verify(spy.success)

        touch.move(0, control, control.width - control.rightPadding).commit()
        compare(control.position, 0.0)
        compare(control.checked, false)
        compare(control.pressed, true)

        touch.move(0, control, control.width / 2).commit()
        compare(control.position, 0.5)
        compare(control.checked, false)
        compare(control.pressed, true)

        touch.move(0, control, control.width - control.rightPadding).commit()
        compare(control.position, 1.0)
        compare(control.checked, false)
        compare(control.pressed, true)

        spy.expectedSequence = [["pressedChanged", { "pressed": false, "checked": false }],
                                ["checkedChanged", { "pressed": false, "checked": true }],
                                "toggled",
                                "released",
                                "clicked"]
        touch.release(0, control, control.width).commit()
        compare(control.position, 1.0)
        compare(control.checked, true)
        compare(control.pressed, false)
        verify(spy.success)
    }

    function test_spacing() {
        var control = createTemporaryObject(switchDelegate, testCase, { text: "Some long, long, long text" })
        verify(control)
        verify(control.contentItem.implicitWidth + control.leftPadding + control.rightPadding > control.background.implicitWidth)

        var textLabel = findChild(control.contentItem, "label")
        verify(textLabel)

        // The implicitWidth of the IconLabel that all buttons use as their contentItem should be
        // equal to the implicitWidth of the Text and the switch indicator + spacing while no icon is set.
        compare(control.contentItem.implicitWidth, textLabel.implicitWidth + control.indicator.width + control.spacing)

        control.spacing += 100
        compare(control.contentItem.implicitWidth, textLabel.implicitWidth + control.indicator.width + control.spacing)

        compare(control.implicitWidth, textLabel.implicitWidth + control.indicator.width + control.spacing + control.leftPadding + control.rightPadding)
    }

    function test_display_data() {
        return [
            { "tag": "IconOnly", display: SwitchDelegate.IconOnly },
            { "tag": "TextOnly", display: SwitchDelegate.TextOnly },
            { "tag": "TextUnderIcon", display: SwitchDelegate.TextUnderIcon },
            { "tag": "TextBesideIcon", display: SwitchDelegate.TextBesideIcon },
            { "tag": "IconOnly, mirrored", display: SwitchDelegate.IconOnly, mirrored: true },
            { "tag": "TextOnly, mirrored", display: SwitchDelegate.TextOnly, mirrored: true },
            { "tag": "TextUnderIcon, mirrored", display: SwitchDelegate.TextUnderIcon, mirrored: true },
            { "tag": "TextBesideIcon, mirrored", display: SwitchDelegate.TextBesideIcon, mirrored: true }
        ]
    }

    function test_display(data) {
        var control = createTemporaryObject(switchDelegate, testCase, {
            text: "SwitchDelegate",
            display: data.display,
            width: 400,
            "icon.source": "qrc:/qt-project.org/imports/QtQuick/Controls.2/images/check.png",
            "LayoutMirroring.enabled": !!data.mirrored
        })
        verify(control)
        compare(control.icon.source, "qrc:/qt-project.org/imports/QtQuick/Controls.2/images/check.png")

        var iconImage = findChild(control.contentItem, "image")
        var textLabel = findChild(control.contentItem, "label")

        var availableWidth = control.availableWidth - control.indicator.width - control.spacing
        var indicatorOffset = control.mirrored ? control.indicator.width + control.spacing : 0

        switch (control.display) {
        case SwitchDelegate.IconOnly:
            verify(iconImage)
            verify(!textLabel)
            compare(iconImage.x, indicatorOffset + (availableWidth - iconImage.width) / 2)
            compare(iconImage.y, (control.availableHeight - iconImage.height) / 2)
            break;
        case SwitchDelegate.TextOnly:
            verify(!iconImage)
            verify(textLabel)
            compare(textLabel.x, control.mirrored ? control.availableWidth - textLabel.width : 0)
            compare(textLabel.y, (control.availableHeight - textLabel.height) / 2)
            break;
        case SwitchDelegate.TextUnderIcon:
            verify(iconImage)
            verify(textLabel)
            compare(iconImage.x, indicatorOffset + (availableWidth - iconImage.width) / 2)
            compare(textLabel.x, indicatorOffset + (availableWidth - textLabel.width) / 2)
            verify(iconImage.y < textLabel.y)
            break;
        case SwitchDelegate.TextBesideIcon:
            verify(iconImage)
            verify(textLabel)
            if (control.mirrored)
                verify(textLabel.x < iconImage.x)
            else
                verify(iconImage.x < textLabel.x)
            compare(iconImage.y, (control.availableHeight - iconImage.height) / 2)
            compare(textLabel.y, (control.availableHeight - textLabel.height) / 2)
            break;
        }
    }
}
