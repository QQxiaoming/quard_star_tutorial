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

import QtQuick 2.11
import QtQuick.Shapes 1.0

Rectangle {
    color: "lightGray"
    Shape {
        id: shape
        width: 220
        height: 200
        anchors.centerIn: parent

        ShapePath {
            fillGradient: LinearGradient {
                y2: shape.height
                GradientStop { position: 0; color: "yellow" }
                GradientStop { position: 1; color: "green" }
            }

            startX: 10; startY: 100
            PathArc {
                relativeX: 50; y: 100
                radiusX: 25; radiusY: 25
            }
            PathArc {
                relativeX: 50; y: 100
                radiusX: 25; radiusY: 35
            }
            PathArc {
                relativeX: 50; y: 100
                radiusX: 25; radiusY: 60
            }
            PathArc {
                relativeX: 50; y: 100
                radiusX: 50; radiusY: 120
            }
        }
    }

    Shape {
        width: 120
        height: 130
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        scale: 0.5

        ShapePath {
            fillColor: "transparent"
            strokeColor: "darkBlue"
            strokeWidth: 20
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: 65; centerY: 95
                radiusX: 45; radiusY: 45
                startAngle: -180
                SequentialAnimation on sweepAngle {
                    loops: Animation.Infinite
                    NumberAnimation { to: 360; duration: 2000 }
                    NumberAnimation { to: 0; duration: 2000 }
                }
            }
        }
    }
}
