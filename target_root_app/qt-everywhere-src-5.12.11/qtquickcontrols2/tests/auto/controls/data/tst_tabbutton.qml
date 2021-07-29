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
    name: "TabButton"

    Component {
        id: tabButton
        TabButton { }
    }

    Component {
        id: repeater
        Column {
            Repeater {
                model: 3
                delegate: TabButton { }
            }
        }
    }

    function test_autoExclusive() {
        var container = createTemporaryObject(repeater, testCase)

        for (var i = 0; i < 3; ++i) {
            container.children[i].checked = true
            compare(container.children[i].checked, true)

            // check that all other buttons are unchecked
            for (var j = 0; j < 3; ++j) {
                if (j !== i)
                    compare(container.children[j].checked, false)
            }
        }
    }

    function test_baseline() {
        var control = createTemporaryObject(tabButton, testCase)
        verify(control)
        compare(control.baselineOffset, control.contentItem.y + control.contentItem.baselineOffset)
    }

    function test_spacing() {
        var control = createTemporaryObject(tabButton, testCase, { text: "Some long, long, long text" })
        verify(control)
        if (control.background)
            verify(control.contentItem.implicitWidth + control.leftPadding + control.rightPadding > control.background.implicitWidth)

        var textLabel = findChild(control.contentItem, "label")
        verify(textLabel)

        // The implicitWidth of the IconLabel that all buttons use as their contentItem
        // should be equal to the implicitWidth of the Text while no icon is set.
        compare(control.contentItem.implicitWidth, textLabel.implicitWidth)

        // That means that spacing shouldn't affect it.
        control.spacing += 100
        compare(control.contentItem.implicitWidth, textLabel.implicitWidth)

        // The implicitWidth of the TabButton itself should, therefore, also never include spacing while no icon is set.
        compare(control.implicitWidth, textLabel.implicitWidth + control.leftPadding + control.rightPadding)
    }

    function test_display_data() {
        return [
            { "tag": "IconOnly", display: TabButton.IconOnly },
            { "tag": "TextOnly", display: TabButton.TextOnly },
            { "tag": "TextUnderIcon", display: TabButton.TextUnderIcon },
            { "tag": "TextBesideIcon", display: TabButton.TextBesideIcon },
            { "tag": "IconOnly, mirrored", display: TabButton.IconOnly, mirrored: true },
            { "tag": "TextOnly, mirrored", display: TabButton.TextOnly, mirrored: true },
            { "tag": "TextUnderIcon, mirrored", display: TabButton.TextUnderIcon, mirrored: true },
            { "tag": "TextBesideIcon, mirrored", display: TabButton.TextBesideIcon, mirrored: true }
        ]
    }

    function test_display(data) {
        var control = createTemporaryObject(tabButton, testCase, {
            text: "TabButton",
            display: data.display,
            "icon.source": "qrc:/qt-project.org/imports/QtQuick/Controls.2/images/check.png",
            "LayoutMirroring.enabled": !!data.mirrored
        })
        verify(control)
        compare(control.icon.source, "qrc:/qt-project.org/imports/QtQuick/Controls.2/images/check.png")

        var iconImage = findChild(control.contentItem, "image")
        var textLabel = findChild(control.contentItem, "label")

        switch (control.display) {
        case TabButton.IconOnly:
            verify(iconImage)
            verify(!textLabel)
            compare(iconImage.x, (control.availableWidth - iconImage.width) / 2)
            compare(iconImage.y, (control.availableHeight - iconImage.height) / 2)
            break;
        case TabButton.TextOnly:
            verify(!iconImage)
            verify(textLabel)
            compare(textLabel.x, (control.availableWidth - textLabel.width) / 2)
            compare(textLabel.y, (control.availableHeight - textLabel.height) / 2)
            break;
        case TabButton.TextUnderIcon:
            verify(iconImage)
            verify(textLabel)
            compare(iconImage.x, (control.availableWidth - iconImage.width) / 2)
            compare(textLabel.x, (control.availableWidth - textLabel.width) / 2)
            verify(iconImage.y < textLabel.y)
            break;
        case TabButton.TextBesideIcon:
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
