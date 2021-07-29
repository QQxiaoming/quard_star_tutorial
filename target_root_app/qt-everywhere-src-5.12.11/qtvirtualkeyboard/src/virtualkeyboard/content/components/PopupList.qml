/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Virtual Keyboard module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtQuick.VirtualKeyboard 2.3

ListView {
    property int maxVisibleItems: 5
    readonly property int preferredVisibleItems: count < maxVisibleItems ? count : maxVisibleItems
    readonly property real contentWidth: contentItem.childrenRect.width
    property alias background: popupListBackground.sourceComponent
    property alias defaultHighlight: defaultHighlight

    clip: true
    visible: enabled && count > 0
    width: contentWidth
    height: currentItem ? currentItem.height * preferredVisibleItems + (spacing * preferredVisibleItems - 1) : 0
    orientation: ListView.Vertical
    snapMode: ListView.SnapToItem
    delegate: keyboard.style.popupListDelegate
    highlight: keyboard.style.popupListHighlight ? keyboard.style.popupListHighlight : defaultHighlight
    highlightMoveDuration: 0
    highlightResizeDuration: 0
    add: keyboard.style.popupListAdd
    remove: keyboard.style.popupListRemove
    keyNavigationWraps: true

    onCurrentItemChanged: if (currentItem) keyboard.soundEffect.register(currentItem.soundEffect)

    Component {
        id: defaultHighlight
        Item {}
    }

    Loader {
        id: popupListBackground
        sourceComponent: keyboard.style.popupListBackground
        anchors.fill: parent
        z: -1
    }
}
