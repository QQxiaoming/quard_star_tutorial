/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Rectangle {
    id: root

    property int progress: 0
    property url iconUrl: ""
    property url pageUrl: ""

    signal accepted(url addressUrl)

    clip: true

    onActiveFocusChanged: {
        if (activeFocus)
            addressField.forceActiveFocus();
    }

    Rectangle {
        width: addressField.width / 100 * root.progress
        height: root.height

        visible: root.progress < 100

        color: "#b6dca6"
        radius: root.radius
    }

    TextField {
        id: addressField
        anchors.fill: parent

        Image {
            anchors.verticalCenter: addressField.verticalCenter
            x: 5; z: parent.z + 1
            width: 16; height: 16
            sourceSize: Qt.size(width, height)
            source: root.iconUrl
            visible: root.progress == 100
        }

        Text {
            text: root.progress < 0 ? "" : root.progress + "%"
            x: 5; z: parent.z + 1
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter

            visible: root.progress < 100
        }

        style: TextFieldStyle {
            padding.left: 30

            background: Rectangle {
                color: "transparent"
                border.color: "black"
                border.width: 1
                radius: root.radius
            }
        }

        onActiveFocusChanged: {
            if (activeFocus)
                selectAll();
            else
                deselect();
        }

        text: root.pageUrl
        onAccepted: root.accepted(utils.fromUserInput(text))
    }
}
