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
    id: root
    default property alias data: __contentItem.data
    property alias velocity: anim.velocity
    property alias contentX: __contentItem.x // sign is reversed compared to Flickable.contentX
    property alias contentY: __contentItem.y // sign is reversed compared to Flickable.contentY
    property alias contentWidth: __contentItem.width
    property alias contentHeight: __contentItem.height
    signal flickStarted
    signal flickEnded

    Item {
        id: __contentItem
        objectName: "__contentItem"
        width: childrenRect.width
        height: childrenRect.height

        property real xlimit: root.width - __contentItem.width
        property real ylimit: root.height - __contentItem.height

        function returnToBounds() {
            if (x > 0) {
                returnXAnim.to = 0
                returnXAnim.start()
            } else if (x < xlimit) {
                returnXAnim.to = xlimit
                returnXAnim.start()
            }
            if (y > 0) {
                returnYAnim.to = 0
                returnYAnim.start()
            } else if (y < ylimit) {
                returnYAnim.to = ylimit
                returnYAnim.start()
            }
        }

        DragHandler {
            id: dragHandler
            onActiveChanged: if (!active) anim.restart(centroid.velocity)
        }
        MomentumAnimation {
            id: anim
            target: __contentItem
            onStarted: root.flickStarted()
            onStopped: {
                __contentItem.returnToBounds()
                root.flickEnded()
            }
        }
        NumberAnimation {
            id: returnXAnim
            target: __contentItem
            property: "x"
            duration: 200
            easing.type: Easing.OutQuad
        }
        NumberAnimation {
            id: returnYAnim
            target: __contentItem
            property: "y"
            duration: 200
            easing.type: Easing.OutQuad
        }
    }
}
