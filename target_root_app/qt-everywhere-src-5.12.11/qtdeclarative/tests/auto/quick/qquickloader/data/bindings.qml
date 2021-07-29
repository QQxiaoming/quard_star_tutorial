/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

import QtQuick 2.0

Item {
    id: root

    property alias game: theGame
    property alias loader: theLoader

    Item {
        id: theGame

        property bool isReady: false

        onStateChanged: {
            if (state == "invalid") {
                // The Loader's active property is bound to isReady, so none of its bindings
                // should be updated when isReady becomes false
                isReady = false;

                player.destroy();
                player = null;
            } else if (state == "running") {
                player = Qt.createQmlObject("import QtQuick 2.0; Item { property color color: 'black' }", root);

                isReady = true;
            }
        }

        property Item player
    }

    Loader {
        id: theLoader
        active: theGame.isReady
        sourceComponent: Rectangle {
            width: 400
            height: 400
            color: game.player.color

            property var game: theGame
        }
    }
}
