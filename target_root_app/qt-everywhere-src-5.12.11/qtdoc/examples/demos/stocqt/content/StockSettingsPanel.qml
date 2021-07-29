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
import QtQuick.Layouts 1.1
import "."

Rectangle {
    id: root
    color: "transparent"

    property bool drawOpenPrice: false
    property bool drawClosePrice: false
    property bool drawHighPrice: true
    property bool drawLowPrice: true

    property string openColor: "#face20"
    property string closeColor: "#14aaff"
    property string highColor: "#80c342"
    property string lowColor: "#f30000"
    property string volumeColor: "#14aaff"

    GridLayout {
        id: settingsGrid
        rows: 5
        columns: 3
        rowSpacing: 4
        anchors.fill: parent

        Item {
            Layout.fillHeight: true
            Layout.columnSpan: 3
        }

        Text {
            id: openText
            color: "#000000"
            font.family: Settings.fontFamily
            font.pointSize: 19
            text: "Open"
            Layout.leftMargin: 10
        }
        Rectangle {
            Layout.preferredHeight: 4
            Layout.preferredWidth: 114
            color: openColor
        }
        CheckBox {
            id: openButton
            buttonEnabled: false
            onButtonEnabledChanged: drawOpenPrice = buttonEnabled
            Layout.rightMargin: 10
        }

        Text {
            id: closeText
            Layout.leftMargin: 10
            color: "#000000"
            font.family: Settings.fontFamily
            font.pointSize: 19
            text: "Close"
        }
        Rectangle {
            Layout.preferredHeight: 4
            Layout.preferredWidth: 114
            color: closeColor
        }
        CheckBox {
            id: closeButton
            buttonEnabled: false
            onButtonEnabledChanged: drawClosePrice = buttonEnabled
            Layout.rightMargin: 10
        }

        Text {
            id: highText
            Layout.leftMargin: 10
            color: "#000000"
            font.family: Settings.fontFamily
            font.pointSize: 19
            text: "High"
        }
        Rectangle {
            Layout.preferredHeight: 4
            Layout.preferredWidth: 114
            color: highColor
        }
        CheckBox {
            id: highButton
            buttonEnabled: true
            onButtonEnabledChanged: drawHighPrice = buttonEnabled
            Layout.rightMargin: 10
        }

        Text {
            id: lowText
            Layout.leftMargin: 10
            color: "#000000"
            font.family: Settings.fontFamily
            font.pointSize: 19
            text: "Low"
        }
        Rectangle {
            Layout.preferredHeight: 4
            Layout.preferredWidth: 114
            color: lowColor
        }

        CheckBox {
            id: lowButton
            buttonEnabled: true
            onButtonEnabledChanged: drawLowPrice = buttonEnabled
            Layout.rightMargin: 10
        }
    }
}
