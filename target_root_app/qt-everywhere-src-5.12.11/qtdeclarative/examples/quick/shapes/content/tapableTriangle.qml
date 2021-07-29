/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

import QtQuick 2.12
import QtQuick.Shapes 1.12

Rectangle {
    width: 120
    height: 120
    color: th.pressed ? "steelBlue" : "lightGray"
    containmentMask: ctr

    TapHandler { id: th }

    Shape {
        id: ctr
        anchors.fill: parent
        containsMode: Shape.FillContains

        ShapePath {
            strokeColor: "red"
            fillColor: "blue"

            SequentialAnimation on strokeWidth {
                loops: Animation.Infinite
                NumberAnimation { from: 1; to: 30; duration: 5000 }
                NumberAnimation { from: 30; to: 1; duration: 5000 }
                PauseAnimation { duration: 2000 }
            }

            startX: 30; startY: 30
            PathLine { x: ctr.width - 30; y: ctr.height - 30 }
            PathLine { x: 30; y: ctr.height - 30 }
            PathLine { x: 30; y: 30 }
        }

        // Besides ShapePath, Shape supports visual and non-visual objects too, allowing
        // free mixing without going through extra hoops:
        Rectangle {
            id: testRect
            color: "green"
            opacity: 0.3
            width: 20
            height: 20
            anchors.right: parent.right
        }
        Timer {
            interval: 100
            repeat: true
            onTriggered: testRect.width = testRect.width > 1 ? testRect.width - 1 : 20
            running: true
        }
    }
}
