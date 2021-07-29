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

import QtQuick 2.12

Rectangle {
    id: root
    width: 900
    height: 850
    objectName: "root"
    color: "#222222"

    Grid {
        objectName: "grid"
        anchors.fill: parent
        spacing: 10
        columns: 6
        Repeater {
            id: top
            objectName: "top"
            model: 6

            delegate: Slider {
                objectName: label
                label: "Drag Knob " + index
                width: 140
            }
        }
        Repeater {
            id: bottom
            objectName: "bottom"
            model: 6

            delegate: DragAnywhereSlider {
                objectName: label
                label: "Drag Anywhere " + index
                width: 140
            }
        }
    }
}
