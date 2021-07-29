/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
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

import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import QtQuick.LocalStorage 2.0
import "Database.js" as JS

Window {
    visible: true
    width: Screen.width / 2
    height: Screen.height / 1.8
    color: "#161616"

    property bool creatingNewEntry: false
    property bool editingEntry: false

    Rectangle {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent

            Header {
                id: input
                Layout.fillWidth: true
            }
            RowLayout {
                MyButton {
                    text: "New"
                    onClicked: {
                        input.initrec_new()
                        creatingNewEntry = true
                        listView.model.setProperty(listView.currentIndex, "id", 0)
                    }
                }
                MyButton {
                    id: saveButton
                    enabled: (creatingNewEntry || editingEntry) && listView.currentIndex != -1
                    text: "Save"
                    onClicked: {
                        var insertedRow = false;
                        if (listView.model.get(listView.currentIndex).id < 1) {
                            //insert mode
                            if (input.insertrec()) {
                                // Successfully inserted a row.
                                input.setlistview()
                                insertedRow = true
                            } else {
                                // Failed to insert a row; display an error message.
                                statustext.text = "Failed to insert row"
                            }
                        } else {
                            // edit mode
                            input.setlistview()
                            JS.dbUpdate(listView.model.get(listView.currentIndex).date,
                                        listView.model.get(listView.currentIndex).trip_desc,
                                        listView.model.get(listView.currentIndex).distance,
                                        listView.model.get(listView.currentIndex).id)
                        }

                        if (insertedRow) {
                            input.initrec()
                            creatingNewEntry = false
                            editingEntry = false
                            listView.forceLayout()
                        }
                    }
                }
                MyButton {
                    id: editButton
                    text: "Edit"
                    enabled: !creatingNewEntry && !editingEntry && listView.currentIndex != -1
                    onClicked: {
                        input.editrec(listView.model.get(listView.currentIndex).date,
                                      listView.model.get(listView.currentIndex).trip_desc,
                                      listView.model.get(listView.currentIndex).distance,
                                      listView.model.get(listView.currentIndex).id)

                        editingEntry = true
                    }
                }
                MyButton {
                    id: deleteButton
                    text: "Delete"
                    enabled: !creatingNewEntry && listView.currentIndex != -1
                    onClicked: {
                        JS.dbDeleteRow(listView.model.get(listView.currentIndex).id)
                        listView.model.remove(listView.currentIndex, 1)
                        if (listView.count == 0) {
                            // ListView doesn't automatically set its currentIndex to -1
                            // when the count becomes 0.
                            listView.currentIndex = -1
                        }
                    }
                }
                MyButton {
                    id: cancelButton
                    text: "Cancel"
                    enabled: (creatingNewEntry || editingEntry) && listView.currentIndex != -1
                    onClicked: {
                        if (listView.model.get(listView.currentIndex).id === 0) {
                            // This entry had an id of 0, which means it was being created and hadn't
                            // been saved to the database yet, so we can safely remove it from the model.
                            listView.model.remove(listView.currentIndex, 1)
                        }
                        listView.forceLayout()
                        creatingNewEntry = false
                        editingEntry = false
                        input.initrec()
                    }
                }
                MyButton {
                    text: "Exit"
                    onClicked: Qt.quit()
                }
            }
            Component {
                id: highlightBar
                Rectangle {
                    width: listView.currentItem.width
                    height: listView.currentItem.height
                    color: "lightgreen"
                }
            }
            ListView {
                id: listView
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: MyModel {}
                delegate: MyDelegate {}
                // Don't allow changing the currentIndex while the user is creating/editing values.
                enabled: !creatingNewEntry && !editingEntry

                highlight: highlightBar
                highlightFollowsCurrentItem: true
                focus: true

                header: Component {
                    Text {
                        text: "Saved activities"
                    }
                }
            }
            Text {
                id: statustext
                color: "red"
                Layout.fillWidth: true
                font.bold: true
                font.pointSize: 20

            }
        }
    }
    Component.onCompleted: {
        JS.dbInit()
    }
}
