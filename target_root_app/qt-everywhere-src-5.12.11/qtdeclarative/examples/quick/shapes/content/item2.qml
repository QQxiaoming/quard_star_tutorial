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
    color: "lightGray"

    Shape {
        id: circ1
        anchors.fill: parent

        ShapePath {
            id: p1
            fillColor: "transparent" // stroke only
            strokeWidth: 4

            SequentialAnimation on strokeColor {
                loops: Animation.Infinite
                ColorAnimation {
                    from: "black"
                    to: "yellow"
                    duration: 5000
                }
                ColorAnimation {
                    from: "yellow"
                    to: "green"
                    duration: 5000
                }
                ColorAnimation {
                    from: "green"
                    to: "black"
                    duration: 5000
                }
            }

            property real r: 60
            startX: circ1.width / 2 - r
            startY: circ1.height / 2 - r
            PathArc {
                x: circ1.width / 2 + p1.r
                y: circ1.height / 2 + p1.r
                radiusX: p1.r; radiusY: p1.r
                useLargeArc: true
            }
            PathArc {
                x: circ1.width / 2 - p1.r
                y: circ1.height / 2 - p1.r
                radiusX: p1.r; radiusY: p1.r
                useLargeArc: true
            }
        }
    }

    Shape {
        id: circ2
        anchors.fill: parent

        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 1.0; to: 0.0; duration: 5000 }
            NumberAnimation { from: 0.0; to: 1.0; duration: 5000 }
        }

        ShapePath {
            id: p2
            strokeWidth: -1 // or strokeColor: "transparent"

            SequentialAnimation on fillColor {
                loops: Animation.Infinite
                ColorAnimation {
                    from: "gray"
                    to: "purple"
                    duration: 3000
                }
                ColorAnimation {
                    from: "purple"
                    to: "red"
                    duration: 3000
                }
                ColorAnimation {
                    from: "red"
                    to: "gray"
                    duration: 3000
                }
            }

            property real r: 40
            startX: circ2.width / 2 - r
            startY: circ2.height / 2 - r
            PathArc {
                x: circ2.width / 2 + p2.r
                y: circ2.height / 2 + p2.r
                radiusX: p2.r; radiusY: p2.r
                useLargeArc: true
            }
            PathArc {
                x: circ2.width / 2 - p2.r
                y: circ2.height / 2 - p2.r
                radiusX: p2.r; radiusY: p2.r
                useLargeArc: true
            }
        }
    }
}
