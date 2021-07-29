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
        width: 100
        height: 100
        anchors.centerIn: parent
        ShapePath {
            id: star
            strokeColor: "blue"
            fillColor: "magenta"
            strokeWidth: 2
            PathMove { x: 90; y: 50 }
            PathLine { x: 50 + 40 * Math.cos(0.8 * 1 * Math.PI); y: 50 + 40 * Math.sin(0.8 * 1 * Math.PI) }
            PathLine { x: 50 + 40 * Math.cos(0.8 * 2 * Math.PI); y: 50 + 40 * Math.sin(0.8 * 2 * Math.PI) }
            PathLine { x: 50 + 40 * Math.cos(0.8 * 3 * Math.PI); y: 50 + 40 * Math.sin(0.8 * 3 * Math.PI) }
            PathLine { x: 50 + 40 * Math.cos(0.8 * 4 * Math.PI); y: 50 + 40 * Math.sin(0.8 * 4 * Math.PI) }
            PathLine { x: 90; y: 50 }
        }
        NumberAnimation on rotation {
            from: 0
            to: 360
            duration: 5000
            loops: Animation.Infinite
        }
    }
    Timer {
        interval: 2000
        onTriggered: star.fillRule = (star.fillRule === ShapePath.OddEvenFill ? ShapePath.WindingFill : ShapePath.OddEvenFill)
        repeat: true
        running: true
    }
    Text {
        anchors.right: parent.right
        text: star.fillRule === ShapePath.OddEvenFill ? "OddEvenFill" : "WindingFill"
    }
}
