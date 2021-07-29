/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tests of the QtQuick module of the Qt Toolkit.
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

import QtQuick.Window 2.2
import QtQuick 2.6
import QtQml.Models 2.11
import example 1.0

Window {
    visible: true
    property bool running: rebuildTimer.running
    ListView {
        anchors.fill: parent
        model: delegateModel
    }

    DelegateModel {
        id: delegateModel
        model: objectsProvider.objects
        delegate: Item {}
    }

    Timer {
        id: rebuildTimer
        running: true
        repeat: true
        interval: 1

        property int count: 0
        onTriggered: {
            objectsProvider.rebuild();
            if (++count === 10)
                running = false;
        }
    }

    ObjectsProvider {
        id: objectsProvider
    }
}
