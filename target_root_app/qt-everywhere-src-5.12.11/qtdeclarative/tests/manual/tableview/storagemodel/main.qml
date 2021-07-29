/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
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

import QtQuick 2.12
import QtQuick.Window 2.12
import Qt.labs.qmlmodels 1.0
import StorageModel 0.1

Window {
    id: window
    width: 480
    height: 300
    visible: true
    color: "darkgray"
    title: "Storage Volumes"

    TableView {
        id: table
        anchors.fill: parent
        anchors.margins: 10
        clip: true
        model: StorageModel { }
        columnSpacing: 1
        rowSpacing: 1
        delegate: DelegateChooser {
            role: "type"
            DelegateChoice {
                roleValue: "Value"
                delegate: Rectangle {
                    color: "tomato"
                    implicitWidth: Math.max(100, label.implicitWidth + 8)
                    implicitHeight: label.implicitHeight + 4

                    Rectangle {
                        x: parent.width - width
                        width: value * parent.width / valueMax
                        height: parent.height
                        color: "white"
                    }

                    Text {
                        id: label
                        anchors.baseline: parent.bottom
                        anchors.baselineOffset: -4
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        text: valueDisplay + " of " + valueMaxDisplay + " " + heading
                    }
                }
            }
            DelegateChoice {
                roleValue: "Flag"
                // We could use a checkbox here but that would be another component (e.g. from Controls)
                delegate: Rectangle {
                    implicitWidth: checkBox.implicitWidth + 8
                    implicitHeight: checkBox.implicitHeight + 4
                    Text {
                        id: checkBox
                        anchors.baseline: parent.bottom
                        anchors.baselineOffset: -4
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        text: (checkState ? "☑ " : "☐ ") + heading
                    }
                }
            }
            DelegateChoice {
                // roleValue: "String" // default delegate
                delegate: Rectangle {
                    implicitWidth: stringLabel.implicitWidth + 8
                    implicitHeight: stringLabel.implicitHeight + 4
                    Text {
                        id: stringLabel
                        anchors.baseline: parent.bottom
                        anchors.baselineOffset: -4
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        text: display
                    }
                }
            }
        }
    }
}
