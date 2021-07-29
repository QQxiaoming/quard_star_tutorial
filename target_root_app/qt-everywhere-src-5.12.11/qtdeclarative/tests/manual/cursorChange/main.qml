/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the manual tests of the Qt Toolkit.
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

import QtQuick 2.0
import QtQuick.Window 2.0

Window {
    id: tw
    visible: true
    width: 800
    height: 500
    color: "green"
    Text {
        id: txt
        font.pointSize: 16
        anchors.top: parent.top
        text: "Move to the blue item.\nCheck the mouse cursor is a PointingHand.\nClick on the blue item."
    }

    Rectangle {
        anchors.centerIn: parent
        width: 100
        height: 50
        color: "blue"
        MouseArea {
            id: testHand
            anchors.fill: parent
            onClicked: {
                tw1.show()
            }
            cursorShape: Qt.PointingHandCursor
        }
    }

    Window {
        Text {
            font.pointSize: 16
            anchors.top: parent.top
            text: "Move the cursor to near one of the edges.\nClick the mouse button."
        }
        id: tw1
        visible: false
        width: 800
        height: 500
        color: "yellow"
        MouseArea {
            anchors.fill: parent
            onClicked: {
                tw1.close()
                txt.text = "Mouse cursor should now be back to an Arrow cursor"
            }
        }
    }
}
