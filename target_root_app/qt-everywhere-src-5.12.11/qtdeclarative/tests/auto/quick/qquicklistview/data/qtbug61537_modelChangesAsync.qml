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
import QtQuick 2.0

Item {
    visible: true
    width: 640
    height: 480

    property ListView listView

    Loader {
        id: loader
        anchors.fill: parent
        asynchronous: true
        sourceComponent: comp

        onStatusChanged: {
            if (status == Loader.Ready) {
                // Assign the listview to the root prop late, so
                // that the c++ part doesn't start before everything is ready.
                listView = item.listView
            }
        }
    }

    Component {
        id: comp
        Item {
            property alias listView: listView

            ListView {
                id: listView

                model: ListModel {
                    id: listModel
                    ListElement { title: "one" }
                    ListElement { title: "two" }
                }

                anchors.fill: parent
                orientation: ListView.Horizontal

                delegate: Item {
                    id: delegateRoot
                    objectName: "delegate"

                    width: 200
                    height: 200

                    Component.onCompleted: {
                        if (index === listModel.count - 1) {
                            // Add a new item while the outer Loader is still incubating async. If the new model item
                            // incubates using e.g QQmlIncubator::AsynchronousIfNested, it will also be loaded async, which
                            // is not currently supported (the item will not be added to the listview, or end up the wrong
                            // position, depending on its index and the current state of the refill/layout logic in
                            // QQuickListView).
                            // We add the new item here at the last delegates Component.onCompleted to hit the point in time
                            // when the listview is not expecting any more async items. In that case, the item will only be
                            // added to the list of visible items if incubated synchronously, which gives us something we
                            // can test for in the auto-test.
                            listModel.insert(0, {title: "zero"});
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: index
                        }
                    }
                }
            }
        }
    }
}
