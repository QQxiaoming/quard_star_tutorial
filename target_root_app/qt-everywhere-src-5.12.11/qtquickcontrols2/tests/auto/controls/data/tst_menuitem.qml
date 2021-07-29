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
    name: "MenuItem"

    Component {
        id: menuItem
        MenuItem { }
    }

    Component {
        id: menu
        Menu { }
    }

    function test_baseline() {
        var control = createTemporaryObject(menuItem, testCase)
        verify(control)
        compare(control.baselineOffset, control.contentItem.y + control.contentItem.baselineOffset)
    }

    function test_checkable() {
        var control = createTemporaryObject(menuItem, testCase)
        verify(control)
        verify(control.hasOwnProperty("checkable"))
        verify(!control.checkable)

        mouseClick(control)
        verify(!control.checked)

        control.checkable = true
        mouseClick(control)
        verify(control.checked)

        mouseClick(control)
        verify(!control.checked)
    }

    function test_highlighted() {
        var control = createTemporaryObject(menuItem, testCase)
        verify(control)
        verify(!control.highlighted)

        control.highlighted = true
        verify(control.highlighted)
    }

    function test_display_data() {
        return [
            { "tag": "IconOnly", display: MenuItem.IconOnly },
            { "tag": "TextOnly", display: MenuItem.TextOnly },
            { "tag": "TextUnderIcon", display: MenuItem.TextUnderIcon },
            { "tag": "TextBesideIcon", display: MenuItem.TextBesideIcon },
            { "tag": "IconOnly, mirrored", display: MenuItem.IconOnly, mirrored: true },
            { "tag": "TextOnly, mirrored", display: MenuItem.TextOnly, mirrored: true },
            { "tag": "TextUnderIcon, mirrored", display: MenuItem.TextUnderIcon, mirrored: true },
            { "tag": "TextBesideIcon, mirrored", display: MenuItem.TextBesideIcon, mirrored: true }
        ]
    }

    function test_display(data) {
        var control = createTemporaryObject(menuItem, testCase, {
            text: "MenuItem",
            display: data.display,
            "icon.source": "qrc:/qt-project.org/imports/QtQuick/Controls.2/images/check.png",
            "LayoutMirroring.enabled": !!data.mirrored
        })
        verify(control)
        compare(control.icon.source, "qrc:/qt-project.org/imports/QtQuick/Controls.2/images/check.png")

        var padding = data.mirrored ? control.contentItem.rightPadding : control.contentItem.leftPadding
        var iconImage = findChild(control.contentItem, "image")
        var textLabel = findChild(control.contentItem, "label")

        switch (control.display) {
        case MenuItem.IconOnly:
            verify(iconImage)
            verify(!textLabel)
            compare(iconImage.x, control.mirrored ? control.availableWidth - iconImage.width - padding : padding)
            compare(iconImage.y, (control.availableHeight - iconImage.height) / 2)
            break;
        case MenuItem.TextOnly:
            verify(!iconImage)
            verify(textLabel)
            compare(textLabel.x, control.mirrored ? control.availableWidth - textLabel.width - padding : padding)
            compare(textLabel.y, (control.availableHeight - textLabel.height) / 2)
            break;
        case MenuItem.TextUnderIcon:
            verify(iconImage)
            verify(textLabel)
            compare(iconImage.x, control.mirrored ? control.availableWidth - iconImage.width - (textLabel.width - iconImage.width) / 2 - padding : (textLabel.width - iconImage.width) / 2 + padding)
            compare(textLabel.x, control.mirrored ? control.availableWidth - textLabel.width - padding : padding)
            verify(iconImage.y < textLabel.y)
            break;
        case MenuItem.TextBesideIcon:
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

    function test_menu() {
        var control = createTemporaryObject(menu, testCase)
        verify(control)

        var item1 = createTemporaryObject(menuItem, testCase)
        verify(item1)
        compare(item1.menu, null)

        var item2 = createTemporaryObject(menuItem, testCase)
        verify(item2)
        compare(item2.menu, null)

        control.addItem(item1)
        compare(item1.menu, control)
        compare(item2.menu, null)

        control.insertItem(1, item2)
        compare(item1.menu, control)
        compare(item2.menu, control)

        control.removeItem(1)
        compare(item1.menu, control)
        compare(item2.menu, null)

        control.removeItem(0)
        compare(item1.menu, null)
        compare(item2.menu, null)
    }
}
