/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
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
import QtQuick.Scene3D 2.0

import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Item {
    id: root

    Scene3D {
        anchors.fill: parent
        focus: true
        aspects: ["input", "logic", "render"]
        cameraAspectRatioMode: Scene3D.AutomaticAspectRatio

        Scene {
            id: scene
            width : root.lineWidth
        }
    }

    property real wMin : 0.5
    property real wMax : 64.0
    property real lineWidth : wMin * Math.exp(Math.log(wMax / wMin) * slider.value)

    Text {
        id: sliderLabel
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 20
        font.pixelSize: 20
        color: "white"
        text: "width = " + lineWidth.toFixed(2)
    }

    Slider {
        id: slider
        anchors.top: sliderLabel.bottom
        anchors.right: parent.right
        anchors.margins: 20
        width: 200
        value: 0.0
    }

    Text {
        anchors.top: slider.bottom
        anchors.right: parent.right
        anchors.margins: 20
        color: scene.smooth ? "white" : "lightgray"
        text: scene.smooth ? "Line Smoothing enabled" : "Line Smoothing disabled"
        font.pixelSize: 20
        MouseArea {
            anchors.fill: parent
            onClicked: scene.smooth = !scene.smooth
        }
    }
}
