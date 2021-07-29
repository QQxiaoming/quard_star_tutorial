/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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


import QtQuick 2.7

Rectangle {
    id: root
    width: 320
    height: 240
    color: "black"

    property int current: list.currentIndex
    property int horizontalVelocityZeroCount: 0

    ListView {
        id: list
        objectName: "list"
        anchors.fill: parent

        focus: true

        orientation: ListView.Horizontal

        snapMode: ListView.SnapToItem
        flickableDirection: Flickable.HorizontalFlick

        model: 10
        delegate: Item {
            width: root.width / 3
            height: root.height
            Rectangle {
                anchors.centerIn: parent
                width: 50
                height: 50
                color: list.currentIndex === index ? "red" : "white"
            }
        }

        onHorizontalVelocityChanged:  {
            if (list.horizontalVelocity === 0.0)
                root.horizontalVelocityZeroCount++
        }

    }

    Rectangle {
        color: "red"
        width: 50
        height: 50
        anchors.left: parent.left
        anchors.bottom: parent.bottom

        MouseArea {
            anchors.fill: parent
            onClicked: {
                list.currentIndex--;
            }
        }
    }

    Rectangle {
        color: "red"
        width: 50
        height: 50
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        MouseArea {
            anchors.fill: parent
            onClicked: {
                list.currentIndex++;
            }
        }
    }
}

