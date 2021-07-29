/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

ApplicationWindow {
    id: window
    width: 1200
    height: 800
    title: "Buttons"
    visible: true

    property alias visualizeDialogButtonBoxContentItem: visualizeDialogButtonBoxContentItemMenuItem.checked
    property alias visualizeDialogButtonBox: visualizeDialogButtonBoxMenuItem.checked

    property int dialogSpacing: 60

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            Item {
                Layout.fillWidth: true
            }

            ToolButton {
                text: "Settings"
                onClicked: settingsMenu.open()

                Menu {
                    id: settingsMenu
                    width: 400

                    MenuItem {
                        id: visualizeDialogButtonBoxContentItemMenuItem
                        text: "Visualize DialogButtonBox contentItem"
                        checkable: true
                    }

                    MenuItem {
                        id: visualizeDialogButtonBoxMenuItem
                        text: "Visualize DialogButtonBox"
                        checkable: true
                    }
                }
            }
        }
    }


    DialogLabel {
        text: "implicit width"
        dialog: dialogImplicitWidthNoButtons
        width: 100
    }
    CustomDialog {
        id: dialogImplicitWidthNoButtons
        x: dialogSpacing
        y: dialogSpacing
        space: 200
    }

    DialogLabel {
        text: "title, implicit width"
        dialog: dialogImplicitWidthTitleNoButtons
        width: 150
    }
    CustomDialog {
        id: dialogImplicitWidthTitleNoButtons
        y: dialogSpacing
        title: "Test"
        previousDialog: dialogImplicitWidthNoButtons
        space: 200
    }

    DialogLabel {
        text: "title, fixed width"
        dialog: dialogFixedWidthTitleNoButtons
    }
    CustomDialog {
        id: dialogFixedWidthTitleNoButtons
        y: dialogSpacing
        width: 300
        title: "Test"
        previousDialog: dialogImplicitWidthTitleNoButtons
        space: 200
    }


    DialogLabel {
        text: "one standard button, implicit width"
        dialog: dialogImplicitWidthOneButton
    }
    CustomDialog {
        id: dialogImplicitWidthOneButton
        x: dialogSpacing
        y: dialogFixedWidthTitleNoButtons.y + dialogFixedWidthTitleNoButtons.height + dialogSpacing
        standardButtons: Dialog.Ok
    }

    DialogLabel {
        text: "two standard buttons, implicit width"
        dialog: dialogImplicitWidthTwoButtons
    }
    CustomDialog {
        id: dialogImplicitWidthTwoButtons
        standardButtons: Dialog.Ok | Dialog.Cancel
        previousDialog: dialogImplicitWidthOneButton
    }

    DialogLabel {
        text: "three standard buttons, implicit width"
        dialog: dialogImplicitWidthThreeButtons
    }
    CustomDialog {
        id: dialogImplicitWidthThreeButtons
        standardButtons: Dialog.Apply | Dialog.RestoreDefaults | Dialog.Cancel
        previousDialog: dialogImplicitWidthTwoButtons
    }


    DialogLabel {
        text: "text, one standard button, implicit width"
        dialog: dialogTextImplicitWidthOneButton
    }
    CustomDialog {
        id: dialogTextImplicitWidthOneButton
        x: dialogSpacing
        y: dialogImplicitWidthThreeButtons.y + dialogImplicitWidthThreeButtons.height + dialogSpacing
        standardButtons: Dialog.Ok

        Label {
            text: "A Label"
        }
    }

    DialogLabel {
        text: "text, two standard buttons, implicit width"
        dialog: dialogTextImplicitWidthTwoButtons
    }
    CustomDialog {
        id: dialogTextImplicitWidthTwoButtons
        standardButtons: Dialog.Ok | Dialog.Cancel
        previousDialog: dialogTextImplicitWidthOneButton

        Label {
            text: "A Label"
        }
    }

    DialogLabel {
        text: "text, three standard buttons, implicit width"
        dialog: dialogTextImplicitWidthThreeButtons
    }
    CustomDialog {
        id: dialogTextImplicitWidthThreeButtons
        standardButtons: Dialog.Apply | Dialog.RestoreDefaults | Dialog.Cancel
        previousDialog: dialogTextImplicitWidthTwoButtons

        Label {
            text: "A Label"
        }
    }


    DialogLabel {
        text: "one standard button, fixed width (300)"
        dialog: dialogFixedWidthOneButton
    }
    CustomDialog {
        id: dialogFixedWidthOneButton
        x: dialogSpacing
        y: dialogTextImplicitWidthThreeButtons.y + dialogTextImplicitWidthThreeButtons.height + dialogSpacing
        width: 300
        standardButtons: Dialog.Ok
    }

    DialogLabel {
        text: "two standard buttons, fixed width (300)"
        dialog: dialogFixedWidthTwoButtons
    }
    CustomDialog {
        id: dialogFixedWidthTwoButtons
        width: 300
        standardButtons: Dialog.Ok | Dialog.Cancel
        previousDialog: dialogFixedWidthOneButton
    }

    DialogLabel {
        text: "three standard buttons, fixed width (300)"
        dialog: dialogFixedWidthThreeButtons
    }
    CustomDialog {
        id: dialogFixedWidthThreeButtons
        width: 300
        standardButtons: Dialog.Apply | Dialog.RestoreDefaults | Dialog.Cancel
        previousDialog: dialogFixedWidthTwoButtons
    }
}
