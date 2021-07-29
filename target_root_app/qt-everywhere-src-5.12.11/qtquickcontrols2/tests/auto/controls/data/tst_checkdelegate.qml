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
    name: "CheckDelegate"

    Component {
        id: checkDelegate
        CheckDelegate {}
    }

    // TODO: data-fy tst_checkbox (rename to tst_check?) so we don't duplicate its tests here?

    function test_defaults() {
        var control = createTemporaryObject(checkDelegate, testCase);
        verify(control);
        verify(!control.checked);
    }

    function test_checked() {
        var control = createTemporaryObject(checkDelegate, testCase);
        verify(control);

        mouseClick(control);
        verify(control.checked);

        mouseClick(control);
        verify(!control.checked);
    }

    function test_baseline() {
        var control = createTemporaryObject(checkDelegate, testCase);
        verify(control);
        compare(control.baselineOffset, control.contentItem.y + control.contentItem.baselineOffset);
    }

    function test_spacing() {
        var control = createTemporaryObject(checkDelegate, testCase, { text: "Some long, long, long text" })
        verify(control)
        verify(control.contentItem.implicitWidth + control.leftPadding + control.rightPadding > control.background.implicitWidth)

        var textLabel = findChild(control.contentItem, "label")
        verify(textLabel)

        // The implicitWidth of the IconLabel that all buttons use as their contentItem should be
        // equal to the implicitWidth of the Text and the check indicator + spacing while no icon is set.
        compare(control.contentItem.implicitWidth, textLabel.implicitWidth + control.indicator.width + control.spacing)

        control.spacing += 100
        compare(control.contentItem.implicitWidth, textLabel.implicitWidth + control.indicator.width + control.spacing)

        compare(control.implicitWidth, textLabel.implicitWidth + control.indicator.width + control.spacing + control.leftPadding + control.rightPadding)
    }

    function test_display_data() {
        return [
            { "tag": "IconOnly", display: CheckDelegate.IconOnly },
            { "tag": "TextOnly", display: CheckDelegate.TextOnly },
            { "tag": "TextUnderIcon", display: CheckDelegate.TextUnderIcon },
            { "tag": "TextBesideIcon", display: CheckDelegate.TextBesideIcon },
            { "tag": "IconOnly, mirrored", display: CheckDelegate.IconOnly, mirrored: true },
            { "tag": "TextOnly, mirrored", display: CheckDelegate.TextOnly, mirrored: true },
            { "tag": "TextUnderIcon, mirrored", display: CheckDelegate.TextUnderIcon, mirrored: true },
            { "tag": "TextBesideIcon, mirrored", display: CheckDelegate.TextBesideIcon, mirrored: true }
        ]
    }

    function test_display(data) {
        var control = createTemporaryObject(checkDelegate, testCase, {
            text: "CheckDelegate",
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
        case CheckDelegate.IconOnly:
            verify(iconImage)
            verify(!textLabel)
            compare(iconImage.x, indicatorOffset + (availableWidth - iconImage.width) / 2)
            compare(iconImage.y, (control.availableHeight - iconImage.height) / 2)
            break;
        case CheckDelegate.TextOnly:
            verify(!iconImage)
            verify(textLabel)
            compare(textLabel.x, control.mirrored ? control.availableWidth - textLabel.width : 0)
            compare(textLabel.y, (control.availableHeight - textLabel.height) / 2)
            break;
        case CheckDelegate.TextUnderIcon:
            verify(iconImage)
            verify(textLabel)
            compare(iconImage.x, indicatorOffset + (availableWidth - iconImage.width) / 2)
            compare(textLabel.x, indicatorOffset + (availableWidth - textLabel.width) / 2)
            verify(iconImage.y < textLabel.y)
            break;
        case CheckDelegate.TextBesideIcon:
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

    Component {
        id: nextCheckStateDelegate
        CheckDelegate {
            tristate: true
            nextCheckState: function() {
                if (checkState === Qt.Checked)
                    return Qt.Unchecked
                else
                    return Qt.Checked
            }
        }
    }

    function test_nextCheckState_data() {
        return [
            { tag: "unchecked", checkState: Qt.Unchecked, expectedState: Qt.Checked },
            { tag: "partially-checked", checkState: Qt.PartiallyChecked, expectedState: Qt.Checked },
            { tag: "checked", checkState: Qt.Checked, expectedState: Qt.Unchecked }
        ]
    }

    function test_nextCheckState(data) {
        var control = createTemporaryObject(nextCheckStateDelegate, testCase)
        verify(control)

        // mouse
        control.checkState = data.checkState
        compare(control.checkState, data.checkState)
        mouseClick(control)
        compare(control.checkState, data.expectedState)

        // touch
        control.checkState = data.checkState
        compare(control.checkState, data.checkState)
        var touch = touchEvent(control)
        touch.press(0, control).commit().release(0, control).commit()
        compare(control.checkState, data.expectedState)

        // keyboard
        control.forceActiveFocus()
        tryCompare(control, "activeFocus", true)
        control.checkState = data.checkState
        compare(control.checkState, data.checkState)
        keyClick(Qt.Key_Space)
        compare(control.checkState, data.expectedState)
    }
}
