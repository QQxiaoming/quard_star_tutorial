/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

import QtQuick 2.12

Rectangle {
    id: root
    property alias label: label.text
    property alias pressed: tap.pressed
    property bool checked: false
    property alias gesturePolicy: tap.gesturePolicy
    property alias margin: tap.margin
    signal tapped

    width: label.implicitWidth * 1.5; height: label.implicitHeight * 2.0
    border.color: "#9f9d9a"; border.width: 1; radius: height / 4; antialiasing: true

    gradient: Gradient {
        GradientStop { position: 0.0; color: tap.pressed ? "#b8b5b2" : "#efebe7" }
        GradientStop { position: 1.0; color: "#b8b5b2" }
    }

    TapHandler {
        id: tap
        objectName: label.text
        onTapped: {
            tapFlash.start()
            root.tapped()
        }
    }

    Text {
        id: label
        font.pointSize: 14
        text: "Button"
        anchors.centerIn: parent
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: 2; radius: root.radius; antialiasing: true
        opacity: tapFlash.running ? 1 : 0
        FlashAnimation on visible {
            id: tapFlash
        }
    }
}
