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

import QtQuick 2.12

Item {
    id: root
    width: 400
    height: 400

    property bool finishedUsableInQml: false

    property alias simpleTopLevelAnimation: simpleTopLevelAnimation
    property real foo: 0

    property alias transitionRect: transitionRect
    property alias transition: transition
    property alias animationWithinTransition: animationWithinTransition

    property real bar: 0
    property alias animationWithinBehavior: animationWithinBehavior
    property alias behavior: behavior

    NumberAnimation {
        id: simpleTopLevelAnimation
        target: root
        property: "foo"
        from: 0
        to: 1
        duration: 10

        onFinished: finishedUsableInQml = true
    }

    Rectangle {
        id: transitionRect
        color: "green"
        width: 50
        height: 50
        anchors.centerIn: parent

        states: State {
            name: "go"
        }
        transitions: Transition {
            id: transition
            to: "go"
            SequentialAnimation {
                NumberAnimation {
                    id: animationWithinTransition
                    duration: 10
                    property: "foo"
                    from: 1
                    to: 2
                }
            }
        }
    }

    Behavior on bar {
        id: behavior
        NumberAnimation {
            id: animationWithinBehavior
            duration: 10
            property: "bar"
            from: 0
            to: 1
        }
    }
}
