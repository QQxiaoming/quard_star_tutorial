/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

import QtQuick 2.4
import QtTest 1.1

Item {
    id: root
    width: 500
    height: 500

    Timer {
        id: timer
        interval: 1
        running: true
        repeat: true
        onTriggered: {
            if (myCanvas.parent == root) {
                myCanvas.parent = null
            } else {
                myCanvas.parent = root
            }
        }
    }

    Canvas {
        id: myCanvas
        anchors.fill: parent
        property var paintContext: null

        function paint() {
            paintContext.fillStyle = Qt.rgba(1, 0, 0, 1);
            paintContext.fillRect(0, 0, width, height);
            requestAnimationFrame(paint);
        }

        onAvailableChanged: {
            if (available) {
                paintContext = getContext("2d")
                requestAnimationFrame(paint);
            }
        }
    }

    TestCase {
        name: "invalidContext"
        when: myCanvas.parent === null && myCanvas.paintContext !== null

        function test_paintContextInvalid() {
            verify(myCanvas.paintContext);
            var caught = false;
            try {
                console.log(myCanvas.paintContext.fillStyle);
            } catch(e) {
                caught = true;
            }
            verify(caught);
            timer.running = false
        }
    }
}
