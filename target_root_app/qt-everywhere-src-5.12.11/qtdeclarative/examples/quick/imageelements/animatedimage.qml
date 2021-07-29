/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
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
import "../shared" as Examples

Column {
    width: 320
    height: 480
    spacing: 6
    y: 12

//! [image]
    AnimatedImage {
        id: animation
        source: "content/Uniflow_steam_engine.gif"
        anchors.horizontalCenter: parent.horizontalCenter
        speed: speedSlider.value
        TapHandler {
            onTapped: animation.playing = !animation.playing
        }
    }
//! [image]

    Rectangle {
        id: timeline
        color: "steelblue"
        width: animation.width
        height: 1
        x: animation.x
        y: animation.height + 12
        visible: animation.playing

        Rectangle {
            property int frames: animation.frameCount
            width: 4; height: 8
            x: (animation.width - width) * animation.currentFrame / frames
            y: -4
            color: "red"
        }
    }

    Examples.Slider {
        id: speedSlider
        name: "Speed"
        min: 0
        max: 5
        init: 1
        width: 240
        x: animation.x
        Text {
            font.pointSize: 12
            text: Math.round(animation.speed * 100) + "%"
            x: animation.width - width
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 6
        }
    }

    Examples.Button {
        text: "Reset"
        enabled: speedSlider.value !== 1
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            speedSlider.setValue(1)
            animation.playing = true
        }
    }
}
