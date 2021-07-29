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

Item {
    id: root
    width: 400; height: 400

    PointHandler {
        id: ph1
        objectName: "ph1"

        target: Rectangle {
            parent: root
            visible: ph1.active
            x: ph1.point.position.x - width / 2
            y: ph1.point.position.y - height / 2
            width: 140
            height: width
            radius: width / 2
            color: "orange"
            opacity: 0.3
        }
    }

    PointHandler {
        id: ph2
        objectName: "ph2"

        target: Rectangle {
            parent: root
            visible: ph2.active
            x: ph2.point.position.x - width / 2
            y: ph2.point.position.y - height / 2
            width: 140
            height: width
            radius: width / 2
            color: "orange"
            opacity: 0.3
        }
    }

    PointHandler {
        id: ph3
        objectName: "ph3"

        target: Rectangle {
            parent: root
            visible: ph3.active
            x: ph3.point.position.x - width / 2
            y: ph3.point.position.y - height / 2
            width: 140
            height: width
            radius: width / 2
            color: "orange"
            opacity: 0.3
        }
    }
}

