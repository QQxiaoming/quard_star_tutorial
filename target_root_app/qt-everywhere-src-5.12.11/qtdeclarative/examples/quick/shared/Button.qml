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
import QtQuick.Window 2.1

Item {
    id: container

    property alias text: buttonLabel.text
    property alias label: buttonLabel
    signal clicked
    property alias containsMouse: hoverHandler.hovered
    property alias pressed: tapHandler.pressed
    implicitHeight: Math.max(Screen.pixelDensity * 7, buttonLabel.implicitHeight * 1.2)
    implicitWidth: Math.max(Screen.pixelDensity * 11, buttonLabel.implicitWidth * 1.3)
    height: implicitHeight
    width: implicitWidth

    SystemPalette { id: palette }

    Rectangle {
        id: frame
        anchors.fill: parent
        color: palette.button
        gradient: Gradient {
            GradientStop { position: 0.0; color: tapHandler.pressed ? Qt.darker(palette.button, 1.3) : palette.button }
            GradientStop { position: 1.0; color: Qt.darker(palette.button, 1.3) }
        }
        antialiasing: true
        radius: height / 6
        border.color: Qt.darker(palette.button, 1.5)
        border.width: 1
    }

    TapHandler {
        id: tapHandler
        onTapped: container.clicked();
    }
    HoverHandler {
        id: hoverHandler
    }

    Text {
        id: buttonLabel
        text: container.text
        color: palette.buttonText
        anchors.centerIn: parent
    }
}
