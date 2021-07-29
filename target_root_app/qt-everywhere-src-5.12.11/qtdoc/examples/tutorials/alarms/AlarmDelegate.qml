/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
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

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11

ItemDelegate {
    id: root
    width: parent.width
    checkable: true

    onClicked: ListView.view.currentIndex = index

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            ColumnLayout {
                id: dateColumn

                readonly property date alarmDate: new Date(
                    model.year, model.month - 1, model.day, model.hour, model.minute)

                Label {
                    id: timeLabel
                    font.pixelSize: Qt.application.font.pixelSize * 2
                    text: dateColumn.alarmDate.toLocaleTimeString(window.locale, Locale.ShortFormat)
                }
                RowLayout {
                    Label {
                        id: dateLabel
                        text: dateColumn.alarmDate.toLocaleDateString(window.locale, Locale.ShortFormat)
                    }
                    Label {
                        id: alarmAbout
                        text: "⸱ " + model.label
                        visible: model.label.length > 0 && !root.checked
                    }
                }
            }
            Item {
                Layout.fillWidth: true
            }
            Switch {
                checked: model.activated
                Layout.alignment: Qt.AlignTop
                onClicked: model.activated = checked
            }
        }
        CheckBox {
            id: alarmRepeat
            text: qsTr("Repeat")
            checked: model.repeat
            visible: root.checked
            onToggled: model.repeat = checked
        }
        Flow {
            visible: root.checked && model.repeat
            Layout.fillWidth: true

            Repeater {
                id: dayRepeater
                model: daysToRepeat
                delegate: RoundButton {
                    text: Qt.locale().dayName(model.dayOfWeek, Locale.NarrowFormat)
                    flat: true
                    checked: model.repeat
                    checkable: true
                    Material.background: checked ? Material.accent : "transparent"
                    onToggled: model.repeat = checked
                }
            }
        }

        TextField {
            id: alarmDescriptionTextField
            placeholderText: qsTr("Enter description here")
            cursorVisible: true
            visible: root.checked
            text: model.label
            onTextEdited: model.label = text
        }
        Button {
            id: deleteAlarmButton
            text: qsTr("Delete")
            width: 40
            height: 40
            visible: root.checked
            onClicked: root.ListView.view.model.remove(root.ListView.view.currentIndex, 1)
        }
    }
}
