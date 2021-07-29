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

import QtQuick 2.0

Rectangle {
    width: 100
    height: 100
    color: "blue"

    RotationAnimation on rotation {
        duration: 3600
        loops: Animation.Infinite
        from: 0
        to: 360
    }

    Timer {
        interval: 300
        repeat: true
        running: true
        property int prevHit: -1
        property int prevRotation: -1
        onTriggered: {
            var date = new Date;
            var millis = date.getMilliseconds()

            if (prevHit < 0) {
                prevHit = millis;
                prevRotation = parent.rotation
                return;
            }

            var milliDelta = millis - prevHit;
            if (milliDelta <= 0)
                milliDelta += 1000;
            prevHit = millis;

            var delta = parent.rotation - prevRotation;
            if (delta < 0)
                delta += 360
            prevRotation = parent.rotation
            console.log(milliDelta, delta, "ms/degrees ");
        }
    }
}
