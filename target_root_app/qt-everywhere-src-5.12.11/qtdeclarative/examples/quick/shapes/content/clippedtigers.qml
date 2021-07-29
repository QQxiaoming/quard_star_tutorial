/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
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

import QtQuick 2.9
import QtQuick.Shapes 1.0

Rectangle {
    id: root
    width: 1024
    height: 768

    property color col: "lightsteelblue"
    gradient: Gradient {
        GradientStop { position: 0.0; color: Qt.tint(root.col, "#20FFFFFF") }
        GradientStop { position: 0.1; color: Qt.tint(root.col, "#20AAAAAA") }
        GradientStop { position: 0.9; color: Qt.tint(root.col, "#20666666") }
        GradientStop { position: 1.0; color: Qt.tint(root.col, "#20000000") }
    }

    Rectangle {
        id: scissorRect
        width: 200
        height: 200
        x: 150
        property real centerY: parent.height / 2 - height / 2
        property real dy: 0
        y: centerY + dy
        clip: true

        Loader {
            id: loader1
            width: parent.width
            height: parent.height
            y: 25 - scissorRect.dy
            source: "tiger.qml"
            asynchronous: true
            visible: status === Loader.Ready
        }

        SequentialAnimation on dy {
            loops: Animation.Infinite
            running: loader1.status === Loader.Ready && loader1.item.status === Shape.Ready
            NumberAnimation {
                from: 0
                to: -scissorRect.centerY
                duration: 2000
            }
            NumberAnimation {
                from: -scissorRect.centerY
                to: scissorRect.centerY
                duration: 4000
            }
            NumberAnimation {
                from: scissorRect.centerY
                to: 0
                duration: 2000
            }
        }
    }

    // With a more complex transformation (like rotation), stenciling is used
    // instead of scissoring, this is more expensive. It may also trigger a
    // slower code path for Shapes, depending on the path rendering backend
    // in use, and may affect rendering quality as well.
    Rectangle {
        id: stencilRect
        width: 300
        height: 200
        anchors.right: parent.right
        anchors.rightMargin: 100
        anchors.verticalCenter: parent.verticalCenter
        clip: true // NB! still clips to bounding rect (not shape)

        Loader {
            id: loader2
            width: parent.width
            height: parent.height
            source: "tiger.qml"
            asynchronous: true
            visible: status === Loader.Ready
        }

        NumberAnimation on rotation {
            from: 0
            to: 360
            duration: 5000
            loops: Animation.Infinite
        }
    }
}
