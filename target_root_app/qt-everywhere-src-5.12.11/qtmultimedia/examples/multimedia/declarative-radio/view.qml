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

import QtQuick 2.0
import QtMultimedia 5.0

Rectangle {
    anchors.fill: parent
    id: root
    property int margins: 5
    property int spacing: 10

    QtObject {
        id: d
        property int buttonHeight: root.height > root.width ? root.width / 3 : root.height / 3
        property int buttonWidth: (root.width - spacing * 4) / 4
    }

    Radio {
        id: radio
        band: Radio.FM
    }

    Column {
        anchors.fill: parent
        anchors.margins: root.margins

        Rectangle {
            color: root.color
            width: parent.width
            height: root.height - d.buttonHeight - 40

            Text {
                id: freq
                verticalAlignment: Text.AlignVCenter
                text: "" + radio.frequency / 1000 + " kHz"
                anchors {
                    bottom: parent.bottom
                    margins: root.margins
                }
            }
            Text {
                id: sig
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    margins: root.margins

                }

                verticalAlignment: Text.AlignVCenter
                text: (radio.availability == Radio.Available ? "No Signal " : "No Radio Found")
            }
        }

        Row {
            spacing: 5

            Rectangle {
                width: root.width - 10
                height: 20
                color: "black"

                Rectangle {
                    width: 5
                    height: 20
                    color: "red"

                    y: 0
                    x: (parent.width - 5) * ((radio.frequency - radio.minimumFrequency) / (radio.maximumFrequency -
                    radio.minimumFrequency))
                }
            }
        }
        Row {
            spacing: root.spacing
            Button {
                text: "Scan Down"
                onClicked: radio.scanDown()
            }
            Button {
                text: "Freq Down"
                onClicked: radio.tuneDown()
            }
            Button {
                text: "Freq Up"
                onClicked: radio.tuneUp()
            }
            Button {
                text: "Scan Up"
                onClicked: radio.scanUp()
            }
        }
    }
}

