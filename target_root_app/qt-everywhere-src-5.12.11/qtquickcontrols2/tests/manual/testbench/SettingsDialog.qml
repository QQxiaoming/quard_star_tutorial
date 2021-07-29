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
import QtQuick.Controls 2.12
import QtQuick.Controls.Imagine 2.12
import QtQuick.Layouts 1.12

import App 1.0

Dialog {
    id: settingsDialog
    title: "Settings"
    width: 500
    height: 400
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    Imagine.path: defaultImaginePath

    property alias imaginePathTextField: imaginePathTextField
    property alias customImagineStyleCheckBox: customImagineStyleCheckBox
    property alias autoFixImagineAssetsCheckBox: autoFixImagineAssetsCheckBox

    onAboutToShow: {
        styleComboBox.currentIndex = styleComboBox.styleIndex
        customImagineStyleCheckBox.checked = settings.useCustomImaginePath
        imaginePathTextField.text = settings.imaginePath
        autoFixImagineAssetsCheckBox.checked = settings.autoFixImagineAssets

        updatePaletteSettingsControls(paletteSettings)
    }

    onAccepted: {
        settings.style = styleComboBox.displayText
        settings.useCustomImaginePath = customImagineStyleCheckBox.checked
        settings.imaginePath = imaginePathTextField.text
        settings.autoFixImagineAssets = autoFixImagineAssetsCheckBox.checked

        paletteSettings.useCustomPalette = useCustomPaletteCheckBox.checked
        paletteSettings.window = windowColorEditor.color
        paletteSettings.windowText = windowTextColorEditor.color
        paletteSettings.base = baseColorEditor.color
        paletteSettings.text = textColorEditor.color
        paletteSettings.button = buttonColorEditor.color
        paletteSettings.buttonText = buttonTextColorEditor.color
        paletteSettings.brightText = brightTextColorEditor.color
        paletteSettings.toolTipBase = toolTipBaseColorEditor.color
        paletteSettings.toolTipText = toolTipTextColorEditor.color
        paletteSettings.light = lightColorEditor.color
        paletteSettings.midlight = midlightColorEditor.color
        paletteSettings.dark = darkColorEditor.color
        paletteSettings.mid = midColorEditor.color
        paletteSettings.shadow = shadowColorEditor.color
        paletteSettings.highlight = highlightColorEditor.color
        paletteSettings.highlightedText = highlightedTextColorEditor.color
        paletteSettings.link = linkColorEditor.color
    }

    function updatePaletteSettingsControls(source) {
        useCustomPaletteCheckBox.checked = paletteSettings.useCustomPalette
        windowColorEditor.color = source.window
        windowTextColorEditor.color = source.windowText
        baseColorEditor.color = source.base
        textColorEditor.color = source.text
        buttonColorEditor.color = source.button
        buttonTextColorEditor.color = source.buttonText
        brightTextColorEditor.color = source.brightText
        toolTipBaseColorEditor.color = source.toolTipBase
        toolTipTextColorEditor.color = source.toolTipText
        lightColorEditor.color = source.light
        midlightColorEditor.color = source.midlight
        darkColorEditor.color = source.dark
        midColorEditor.color = source.mid
        shadowColorEditor.color = source.shadow
        highlightColorEditor.color = source.highlight
        highlightedTextColorEditor.color = source.highlightedText
        linkColorEditor.color = source.link
    }

    function paletteSettingsMap() {
        var map = ({});
        map.window = windowColorEditor.color
        map.windowText = windowTextColorEditor.color
        map.base = baseColorEditor.color
        map.text = textColorEditor.color
        map.button = buttonColorEditor.color
        map.buttonText = buttonTextColorEditor.color
        map.brightText = brightTextColorEditor.color
        map.toolTipBase = toolTipBaseColorEditor.color
        map.toolTipText = toolTipTextColorEditor.color
        map.light = lightColorEditor.color
        map.midlight = midlightColorEditor.color
        map.dark = darkColorEditor.color
        map.mid = midColorEditor.color
        map.shadow = shadowColorEditor.color
        map.highlight = highlightColorEditor.color
        map.highlightedText = highlightedTextColorEditor.color
        map.link = linkColorEditor.color
        return map;
    }

    Clipboard {
        id: clipboard
    }

    DirectoryValidator {
        id: directoryValidator
        path: imaginePathTextField.text
    }

    contentItem: Flickable {
        contentWidth: settingsDialog.availableWidth
        contentHeight: contentLayout.implicitHeight
        flickableDirection: Qt.Vertical
        clip: true

        ColumnLayout {
            id: contentLayout
            anchors.fill: parent

            GroupBox {
                title: qsTr("General")
                Layout.fillWidth: true

                GridLayout {
                    columns: 2

                    Label {
                        text: "Style:"
                    }

                    ComboBox {
                        id: styleComboBox
                        model: availableStyles

                        property int styleIndex: -1

                        Component.onCompleted: {
                            styleIndex = find(settings.style, Qt.MatchFixedString)
                            if (styleIndex !== -1)
                                currentIndex = styleIndex
                        }

                        Layout.fillWidth: true
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: "Restart required"
                        color: "#e41e25"
                        opacity: styleComboBox.currentIndex !== styleComboBox.styleIndex ? 1.0 : 0.0
                        Layout.topMargin: 12
                        Layout.maximumHeight: styleComboBox.currentIndex !== styleComboBox.styleIndex ? implicitHeight : 0
                    }
                }
            }

            GroupBox {
                title: qsTr("Imagine Style")
                visible: usingImagineStyle
                Layout.fillWidth: true
                Layout.columnSpan: 2

                GridLayout {
                    columns: 2
                    anchors.fill: parent

                    CheckBox {
                        id: customImagineStyleCheckBox
                        text: qsTr("Use Custom Assets")
                        enabled: usingImagineStyle

                        Layout.columnSpan: 2
                    }

                    Label {
                        text: "Asset Path"
                        enabled: usingImagineStyle && customImagineStyleCheckBox.checked
                    }

                    TextField {
                        id: imaginePathTextField
                        text: settings.imaginePath
                        enabled: usingImagineStyle && customImagineStyleCheckBox.checked

                        Layout.preferredWidth: 200
                        Layout.fillWidth: true

                        ToolTip {
                            text: "Path to a folder that contains Imagine style image assets"
                            visible: imaginePathTextField.hovered
                            delay: 500
                            parent: imaginePathTextField

                            Imagine.path: defaultImaginePath
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: directoryValidator.errorMessage
                        color: "#e41e25"
                        opacity: !directoryValidator.valid ? 1.0 : 0.0
                        elide: Label.ElideMiddle
                        Layout.topMargin: !directoryValidator.valid ? 12 : 0
                        Layout.maximumHeight: !directoryValidator.valid ? implicitHeight : 0

                        Layout.fillWidth: true
                    }

                    CheckBox {
                        id: autoFixImagineAssetsCheckBox
                        text: "Automatically Fix Custom Assets"
                        enabled: usingImagineStyle && customImagineStyleCheckBox.checked

                        Layout.columnSpan: 2

                        ToolTip {
                            text: "If set, custom Imagine style assets will be modified to be compliant whenever they have changed on disk"
                            visible: autoFixImagineAssetsCheckBox.hovered
                            delay: 500
                            parent: imaginePathTextField

                            Imagine.path: defaultImaginePath
                        }
                    }
                }
            }

            GroupBox {
                title: qsTr("Palette")
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    anchors.fill: parent

                    CheckBox {
                        id: useCustomPaletteCheckBox
                        text: qsTr("Use Custom Palette")
                    }

                    ColorEditor {
                        id: windowColorEditor
                        labelText: qsTr("Window")
                        toolTipText: qsTr("A general background color.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: windowTextColorEditor
                        labelText: qsTr("WindowText")
                        toolTipText: qsTr("A general foreground color.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: baseColorEditor
                        labelText: qsTr("Base")
                        toolTipText: qsTr("Used mostly as the background color for text editor controls and items views. It is usually white or another light color.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: textColorEditor
                        labelText: qsTr("Text")
                        toolTipText: qsTr("The foreground color used with Base. This is usually the same as the WindowText, in which case it must provide good contrast with Window and Base.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: buttonColorEditor
                        labelText: qsTr("Button")
                        toolTipText: qsTr("The general button background color. This background can be different from Window as some styles require a different background color for buttons.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: buttonTextColorEditor
                        labelText: qsTr("ButtonText")
                        toolTipText: qsTr("A foreground color used with the Button color.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: brightTextColorEditor
                        labelText: qsTr("BrightText")
                        toolTipText: qsTr("A text color that is very different from WindowText, and contrasts well with e.g. Dark. Typically used for text that needs to be drawn where Text, WindowText or ButtonText would give poor contrast, such as on highlighted buttons.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: toolTipBaseColorEditor
                        labelText: qsTr("ToolTipBase")
                        toolTipText: qsTr("Used as the background color for tooltips.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: toolTipTextColorEditor
                        labelText: qsTr("ToolTipText")
                        toolTipText: qsTr("Used as the foreground color for tooltips.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: lightColorEditor
                        labelText: qsTr("Light")
                        toolTipText: qsTr("Lighter than Button.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: midlightColorEditor
                        labelText: qsTr("Midlight")
                        toolTipText: qsTr("Between Button and Light.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: darkColorEditor
                        labelText: qsTr("Dark")
                        toolTipText: qsTr("Darker than Button.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: midColorEditor
                        labelText: qsTr("Mid")
                        toolTipText: qsTr("Between Button and Dark.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: shadowColorEditor
                        labelText: qsTr("Shadow")
                        toolTipText: qsTr("A very dark color.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: highlightColorEditor
                        labelText: qsTr("Highlight")
                        toolTipText: qsTr("A color to indicate a selected item or the current item.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: linkColorEditor
                        labelText: qsTr("Link")
                        toolTipText: qsTr("A text color used for hyperlinks.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    ColorEditor {
                        id: highlightedTextColorEditor
                        labelText: qsTr("HighlightedText")
                        toolTipText: qsTr("A text color that contrasts with Highlight.")
                        enabled: useCustomPaletteCheckBox.checked
                    }

                    Button {
                        text: qsTr("Copy Palette Settings To Clipboard")
                        enabled: useCustomPaletteCheckBox.checked
                        onClicked: clipboard.copy(paletteSettingsMap())

                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignRight
                    }

                    Button {
                        text: qsTr("Import Palette Settings From Clipboard")
                        enabled: useCustomPaletteCheckBox.checked
                        onClicked: updatePaletteSettingsControls(clipboard.paste())

                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignRight
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }

    footer: DialogButtonBox {
        Button {
            text: qsTr("OK")
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            enabled: !imaginePathTextField.enabled || (imaginePathTextField.enabled && directoryValidator.valid)
        }
        Button {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }
}
