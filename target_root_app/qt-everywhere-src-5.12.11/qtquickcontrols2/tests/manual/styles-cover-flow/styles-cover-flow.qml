/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.12
import QtQuick.Window 2.12

Window {
    // Different delegate positions and widths and window background colors
    // can cause some unwanted "rogue pixels", so an easy way to get it perfect
    // is to mess with the width.
    width: 814
    height: 512
    visible: true
    color: backgroundColor
    flags: Qt.FramelessWindowHint

    readonly property color backgroundColor: "#ffffff"

    Shortcut {
        sequence: "Ctrl+Q"
        onActivated: Qt.quit()
    }

    PathView {
        id: view
        anchors.fill: parent
        anchors.leftMargin: 130
        anchors.rightMargin: 130
        model: ListModel {
            ListElement { source: "qtquickcontrols2-default.png"; dark: false }
            ListElement { source: "qtquickcontrols2-fusion.png"; dark: false }
            ListElement { source: "qtquickcontrols2-universal-light.png"; dark: false }
            ListElement { source: "qtquickcontrols2-universal-dark.png"; dark: true }
            ListElement { source: "qtquickcontrols2-material-dark.png"; dark: true }
            ListElement { source: "qtquickcontrols2-imagine.png"; dark: false }
            ListElement { source: "qtquickcontrols2-material-light.png"; dark: false }
        }

        highlightRangeMode: PathView.StrictlyEnforceRange
        preferredHighlightBegin: 0.5
        preferredHighlightEnd: 0.5
        pathItemCount: 9

        property real centerX: width / 2
        property real centerY: height * 0.4
        property real delegateSize: 393 / 2

        path: CoverFlowPath {
            pathView: view
        }
        delegate: CoverFlowDelegate {}
    }
}
