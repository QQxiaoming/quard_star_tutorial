/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

import QtQuick 2.12

Rectangle {
    id: whiteRect
    property real scale: -1.0
    property int activeCount : 0
    property int deactiveCount : 0
    width: 320; height: 320
    color: "white"
    Rectangle {
        id: blackRect
        objectName: "blackrect"
        color: "black"
        y: 50
        x: 50
        width: 100
        height: 100
        opacity: (whiteRect.width-blackRect.x+whiteRect.height-blackRect.y-199)/200
        Text { color: "white"; text: "opacity: " + blackRect.opacity + "\nscale: " + blackRect.scale}
        Rectangle {
            color: "red"
            width: 6; height: 6; radius: 3
            visible: pincharea.active
            x: pincharea.centroid.position.x - radius
            y: pincharea.centroid.position.y - radius
        }

        PinchHandler {
            id: pincharea
            objectName: "pinchHandler"
            minimumScale: 1.0
            maximumScale: 4.0
            minimumRotation: 0.0
            maximumRotation: 90.0
            xAxis.maximum: 140
            yAxis.maximum: 170
            onActiveChanged: {
                whiteRect.scale = pincharea.scale
                if (active) ++activeCount
                else ++deactiveCount;
            }

            onUpdated: {
                whiteRect.scale = pincharea.scale
                //whiteRect.pointCount = pincharea.pointCount
            }
         }
     }
 }
