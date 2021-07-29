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

ParallelAnimation {
    id: root
    property Item target: null
    property int duration: 500
    property vector2d velocity: Qt.vector2d(0,0)

    function restart(vel) {
        stop()
        velocity = vel
        start()
    }

    NumberAnimation {
        id: xAnim
        target: root.target
        property: "x"
        to: target.x + velocity.x / duration * 100000
        duration: root.duration
        easing.type: Easing.OutQuad
    }
    NumberAnimation {
        id: yAnim
        target: root.target
        property: "y"
        to: target.y + velocity.y / duration * 100000
        duration: root.duration
        easing.type: Easing.OutQuad
    }
}
