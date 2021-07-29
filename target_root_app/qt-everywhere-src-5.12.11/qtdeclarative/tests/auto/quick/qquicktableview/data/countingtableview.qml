/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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
import QtQuick.Window 2.3

Item {
    id: root
    width: 640
    height: 450

    property alias tableView: tableView

    // currentDelegateCount is the number of currently visible items
    property int currentDelegateCount: 0
    // maxDelegateCount is the largest number of items that has ever been visible at the same time
    property int maxDelegateCount: 0
    // delegatesCreatedCount is the number of items created during the lifetime of the test
    property int delegatesCreatedCount: 0

    property real delegateWidth: 100
    property real delegateHeight: 50

    TableView {
        id: tableView
        width: 600
        height: 400
        anchors.margins: 1
        clip: true
        delegate: tableViewDelegate
    }

    Component {
        id: tableViewDelegate
        Rectangle {
            objectName: "tableViewDelegate"
            implicitWidth: delegateWidth
            implicitHeight: delegateHeight
            color: "lightgray"
            border.width: 1

            property int pooledCount: 0
            property int reusedCount: 0
            TableView.onPooled: pooledCount++;
            TableView.onReused: reusedCount++;

            Text {
                anchors.centerIn: parent
                text: column
            }
            Component.onCompleted: {
                delegatesCreatedCount++;
                currentDelegateCount++;
                maxDelegateCount = Math.max(maxDelegateCount, currentDelegateCount);
            }
            Component.onDestruction: {
                currentDelegateCount--;
            }
        }
    }

}
