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
import QtQuick.Window 2.12

Item {
    id: root
    width: 640
    height: 480

    property alias tableView: tableView

    property int row: 42
    property int column: 42

    property int resolvedDelegateRow: 0
    property int resolvedDelegateColumn: 0

    TableView {
        id: tableView
        // Dummy tableView, to let the auto test follow the
        // same pattern for loading qml files as other tests.
    }

    Item {
        width: 100
        height: parent.height;
        Repeater {
            model: 1
            delegate: Component {
                Rectangle {
                    color: "blue"
                    height: 100
                    width: 100
                    Component.onCompleted: {
                        // row and column should be resolved to be the ones
                        // found in the root item, and not in the delegate
                        // items context. The context properties are revisioned,
                        // and require that the QQmlDelegateModel has an import
                        // version set (which is not the case when using a
                        // Repeater, only when using a TableView).
                        resolvedDelegateRow = row
                        resolvedDelegateColumn = column
                    }
                }
            }
        }
    }
}

