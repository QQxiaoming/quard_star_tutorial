/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

import QtQuick 2.0

PathView {
    id: view
    width: 400; height: 240
    highlight: Rectangle { width: 80; height: 80; color: "lightsteelblue" }
    preferredHighlightBegin: 0.5
    preferredHighlightEnd: 0.5
    model: ListModel {
        id: appModel
        ListElement { name: "Music" }
        ListElement { name: "Movies" }
        ListElement { name: "Camera" }
        ListElement { name: "Calendar" }
        ListElement { name: "Messaging" }
        ListElement { name: "Todo List" }
        ListElement { name: "Contacts" }
    }
    delegate: Rectangle {
        width: 100; height: 100
        scale: PathView.iconScale
        border.color: "lightgrey"
        color: "transparent"
        Text {
            anchors { horizontalCenter: parent.horizontalCenter }
            text: name
            smooth: true
            color: ma.pressed ? "red" : "black"
        }

        MouseArea {
            id: ma
            anchors.fill: parent
            onClicked: view.currentIndex = index
        }
    }
    path: Path {
        startX: 10
        startY: 50
        PathAttribute { name: "iconScale"; value: 0.5 }
        PathQuad { x: 200; y: 150; controlX: 50; controlY: 200 }
        PathAttribute { name: "iconScale"; value: 1.0 }
        PathQuad { x: 390; y: 50; controlX: 350; controlY: 200 }
        PathAttribute { name: "iconScale"; value: 0.5 }
    }
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 20
        text: view.currentIndex + " @ " + offset.toFixed(2)
    }
}
