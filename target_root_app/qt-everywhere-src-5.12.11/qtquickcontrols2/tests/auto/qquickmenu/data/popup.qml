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
import QtQuick.Controls 2.12

ApplicationWindow {
    width: 400
    height: 600

    property alias menu: menu
    property alias menuItem1: menuItem1
    property alias menuItem2: menuItem2
    property alias menuItem3: menuItem3
    property alias button: button

    function popupAtCursor() {
        menu.popup()
    }

    function popupAtPos(pos) {
        menu.popup(pos)
    }

    function popupAtCoord(x, y) {
        menu.popup(x, y)
    }

    function popupItemAtCursor(item) {
        menu.popup(item)
    }

    function popupItemAtPos(pos, item) {
        menu.popup(pos, item)
    }

    function popupItemAtCoord(x, y, item) {
        menu.popup(x, y, item)
    }

    function popupAtParentCursor(parent) {
        menu.popup(parent)
    }

    function popupAtParentPos(parent, pos) {
        menu.popup(parent, pos)
    }

    function popupAtParentCoord(parent, x, y) {
        menu.popup(parent, x, y)
    }

    function popupItemAtParentCursor(parent, item) {
        menu.popup(parent, item)
    }

    function popupItemAtParentPos(parent, pos, item) {
        menu.popup(parent, pos, item)
    }

    function popupItemAtParentCoord(parent, x, y, item) {
        menu.popup(parent, x, y, item)
    }

    Menu {
        id: menu
        MenuItem { id: menuItem1; text: "Foo" }
        MenuItem { id: menuItem2; text: "Bar" }
        MenuItem { id: menuItem3; text: "Baz" }
    }

    Button {
        id: button
        text: "Button"
        anchors.centerIn: parent
    }
}
