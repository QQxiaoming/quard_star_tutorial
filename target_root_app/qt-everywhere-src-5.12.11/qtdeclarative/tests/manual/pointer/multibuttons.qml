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

import QtQuick 2.12
import QtQuick.Particles 2.0
import QtQuick.Layouts 1.0
import "content"

Item {
    width: 800
    height: 800
    ColumnLayout {
        anchors.right: parent.right
        spacing: 20
        Text { text: "protagonist"; font.pointSize: 12 }
        MultiButton {
            id: balloonsButton
            label: "Launch Balloons"
            Layout.fillWidth: true
            gesturePolicy: TapHandler.WithinBounds
            margin: 10
        }
        Text { text: "the goons"; font.pointSize: 12 }
        MultiButton {
            id: missilesButton
            label: "Launch Missile"
            Layout.fillWidth: true
            gesturePolicy: TapHandler.ReleaseWithinBounds
            margin: 10
            onTapped: missileEmitter.burst(1)
        }
        MultiButton {
            id: fightersButton
            label: "Launch Fighters"
            Layout.fillWidth: true
            gesturePolicy: TapHandler.DragThreshold
            margin: 10
        }
    }
    ParticleSystem {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 150
        ImageParticle { source: "resources/balloon.png" }
        Emitter { anchors.bottom: parent.bottom; enabled: balloonsButton.pressed; lifeSpan: 5000; size: 64
            maximumEmitted: 99
            emitRate: 50; velocity: PointDirection { x: 10; y: -150; yVariation: 30; xVariation: 50 } } }
    ParticleSystem {
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        ImageParticle { source: "resources/fighter.png" }
        Emitter { anchors.bottom: parent.bottom; enabled: fightersButton.pressed; lifeSpan: 15000; size: 204
            emitRate: 3; velocity: PointDirection { x: -1000; y: -250; yVariation: 150; xVariation: 50 } } }
    ParticleSystem {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 100
        ImageParticle { source: "resources/missile.png"; autoRotation: true; rotation: 90 }
        Emitter { id: missileEmitter; anchors.bottom: parent.bottom; lifeSpan: 5000; size: 128;
            emitRate: 0; velocity: PointDirection { x: -200; y: -350; yVariation: 200; xVariation: 100 } } }
}
