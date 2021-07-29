/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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
        width: image.width
        height: image.height
        transform: Rotation { id: tilt; origin.x: width / 2; origin.y: height / 2; axis { x: 1; y: 0; z: 0 } }

        Image {
            id: image
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
            source: "resources/map.svgz"
            Component.onCompleted: { width = implicitWidth; height = implicitHeight }
        }

        Text {
            anchors.centerIn: parent
            text: image.sourceSize.width + " x " + image.sourceSize.height +
                  " scale " + map.scale.toFixed(2) + " active scale " + pinch.activeScale.toFixed(2)
        }
    }

    DragHandler {
        objectName: "single-point drag"
        target: map
    }

    DragHandler {
        id: tiltHandler
        objectName: "two-point tilt"
        minimumPointCount: 2
        maximumPointCount: 2
        xAxis.enabled: false
        target: null
        onTranslationChanged: tilt.angle = translation.y / -2
    }

    PinchHandler {
        id: pinch
        objectName: "two-point pinch"
        target: map
        minimumScale: 0.1
        maximumScale: 10
        xAxis.enabled: false
        yAxis.enabled: false
        onActiveChanged: if (!active) reRenderIfNecessary()
        grabPermissions: PinchHandler.TakeOverForbidden // don't allow takeover if pinch has started
    }

    function reRenderIfNecessary() {
        var newSourceWidth = image.sourceSize.width * pinch.scale
        var ratio = newSourceWidth / image.sourceSize.width
        if (ratio > 1.1 || ratio < 0.9)
            image.sourceSize.width = newSourceWidth
    }
}
