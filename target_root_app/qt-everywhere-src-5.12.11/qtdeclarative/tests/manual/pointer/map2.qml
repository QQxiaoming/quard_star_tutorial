/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

Item {
    width: 640
    height: 480

    Rectangle {
        id: map
        color: "aqua"
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: image.implicitWidth
        height: image.implicitHeight
        property point center : Qt.point(x + map.width/2, y + map.height/2)

        function setCenter(xx, yy) {
            map.x = xx - map.width/2
            map.y = yy - map.height/2
        }


        Image {
            id: image
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
            source: "resources/map.svgz"
        }
    }

    PinchHandler {
        id: pinch
        target: map
        minimumScale: 0.1
        maximumScale: 10
    }

    DragHandler {
        property point startDrag
        target: null
        onActiveChanged: {
            if (active)
                startDrag = map.center
        }

        onTranslationChanged: {
            if (!target)
                map.setCenter(startDrag.x + translation.x, startDrag.y + translation.y)
        }
    }
}
