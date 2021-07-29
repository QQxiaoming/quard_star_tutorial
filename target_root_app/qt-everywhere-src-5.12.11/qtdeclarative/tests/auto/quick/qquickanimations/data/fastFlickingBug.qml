/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
import QtQuick 2.11
import QtQuick.Window 2.11

Window {
    id: root
    property alias timer : timer
    property alias listView : listView
    property alias theModel: theModel
    property variant ops: [{'op': 'add', 'count': 30}, {'op': 'add', 'count': 60}, {'op': 'rem', 'count': 40}, {'op': 'rem', 'count': 10}, {'op': 'rem', 'count': 39},
    {'op': 'add', 'count': 30}, {'op': 'add', 'count': 60}, {'op': 'rem', 'count': 40}, {'op': 'rem', 'count': 10}, {'op': 'rem', 'count': 39},
    {'op': 'add', 'count': 30}, {'op': 'add', 'count': 60}, {'op': 'rem', 'count': 40}, {'op': 'rem', 'count': 10}, {'op': 'rem', 'count': 39},
    {'op': 'add', 'count': 30}, {'op': 'add', 'count': 60}, {'op': 'rem', 'count': 40}, {'op': 'rem', 'count': 10}, {'op': 'rem', 'count': 39},
    {'op': 'add', 'count': 30}, {'op': 'add', 'count': 60}, {'op': 'rem', 'count': 40}, {'op': 'rem', 'count': 10}, {'op': 'rem', 'count': 39},
    {'op': 'add', 'count': 30}, {'op': 'add', 'count': 60}, {'op': 'rem', 'count': 40}, {'op': 'rem', 'count': 10}, {'op': 'rem', 'count': 39},
    {'op': 'add', 'count': 30}, {'op': 'add', 'count': 60}, {'op': 'rem', 'count': 40}, {'op': 'rem', 'count': 10}, {'op': 'rem', 'count': 39}]
    property int opIndex : 0
    width: 400
    height: 600

    ListModel {
        id: theModel
    }

    Timer {
        id: timer
        interval: 100
        running: false
        repeat: true
        onTriggered: {
            if (opIndex >= ops.length) {
                timer.stop()
                return
            }
            let op = ops[opIndex]
            for (var i = 0; i < op.count; ++i) {
                if (op.op === "add")
                    theModel.append({"name": "opIndex " + opIndex})
                else
                    theModel.remove(0, 1);
            }
            opIndex = opIndex + 1
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        spacing: 4
        model: theModel
        header: Text {
            text: "YAnimator"
        }
        add: Transition {
            NumberAnimation { property: "scale";   from: 0; to: 1; duration: 200 }
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
        }
        displaced: Transition {
            YAnimator { duration: 500 }
            NumberAnimation { property: "opacity"; to: 1.0; duration: 1000 }
            NumberAnimation { property: "scale";   to: 1.0; duration: 1000 }
        }
        remove: Transition {
            NumberAnimation { property: "opacity"; to: 0; duration: 200 }
            NumberAnimation { property: "scale";   to: 0; duration: 200 }
        }
        delegate: Rectangle {
            width: 200
            height: 20
            color:"red"
            Text {
                anchors.centerIn: parent
                text: name
            }
        }
    }
}
