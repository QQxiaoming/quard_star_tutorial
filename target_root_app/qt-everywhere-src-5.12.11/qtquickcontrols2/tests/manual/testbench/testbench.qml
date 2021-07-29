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
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Imagine 2.12
import Qt.labs.folderlistmodel 2.2
import Qt.labs.settings 1.0

import App 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 1000
    height: 750
    title: "Style Testbench - " + settings.style + " Style" + (usingImagineStyle ? imagineTitleText : "")

    Imagine.path: defaultImaginePath

    readonly property bool usingImagineStyle: settings.style.toLowerCase() === "imagine"
    // Some controls should be visible regardless of whether or not custom assets are lacking for it,
    // so we use the default assets in some cases.
    readonly property string defaultImaginePath: "qrc:/qt-project.org/imports/QtQuick/Controls.2/Imagine/images/"
    property bool settingsLoaded: false
    readonly property string imagineTitleText: " - " + (settings.useCustomImaginePath ? settings.imaginePath : "Default Assets")

    LoggingCategory {
        id: brief
        name: "qt.quick.controls.tools.testbench.assetfixer.brief"
    }

    Shortcut {
        sequence: "Ctrl+F"
        onActivated: searchTextField.forceActiveFocus()
    }

    Shortcut {
        sequence: "Ctrl+Q"
        onActivated: Qt.quit()
    }

    Action {
        id: fixAssetsAction
        text: qsTr("Fix Custom Assets")
        shortcut: "Ctrl+Shift+X"
        enabled: usingImagineStyle
        onTriggered: assetFixer.manualFix()
    }

    Action {
        id: useCustomAssetsAction
        text: qsTr("Use Custom Assets")
        shortcut: "Ctrl+Shift+C"
        enabled: usingImagineStyle
        checkable: true
        checked: settings.useCustomImaginePath
        onTriggered: settings.useCustomImaginePath = !settings.useCustomImaginePath
    }

    Action {
        id: reloadAssetsAction
        text: qsTr("Reload Assets")
        shortcut: "Ctrl+R"
        enabled: usingImagineStyle
        onTriggered: assetFixer.reloadAssets()
    }

    FontMetrics {
        id: fontMetrics
    }

    Settings {
        id: settings

        property alias windowX: window.x
        property alias windowY: window.y
        property alias windowWidth: window.width
        property alias windowHeight: window.height

        property string style: "Imagine"

        property bool useCustomImaginePath
        property string imaginePath
        property bool autoFixImagineAssets
        property alias imagineDirLastModified: assetFixer.assetDirectoryLastModified

        Component.onCompleted: settingsLoaded = true
    }

    Settings {
        id: paletteSettings

        category: "Palette"

        property bool useCustomPalette
        property string window
        property string windowText
        property string base
        property string text
        property string button
        property string buttonText
        property string brightText
        property string toolTipBase
        property string toolTipText
        property string light
        property string midlight
        property string dark
        property string mid
        property string shadow
        property string highlight
        property string highlightedText
        property string link
    }

    header: ToolBar {
        // Seems to be necessary to get the default assets to be used here,
        // though it should inherit the window's path
        Imagine.path: defaultImaginePath

        RowLayout {
            anchors.fill: parent

            ToolButton {
                text: "\uf0c9"
                font.family: "fontawesome"
                font.pixelSize: Qt.application.font.pixelSize * 1.6
                onClicked: drawer.open()
            }

            ToolSeparator {}

            TextField {
                id: searchTextField
                placeholderText: "Search"
            }

            Item {
                Layout.fillWidth: true
            }

            ToolButton {
                id: optionsMenuButton
                text: "\ue800"
                font.family: "FontAwesome"
                font.pixelSize: Qt.application.font.pixelSize * 1.6
                checked: optionsMenu.visible
                checkable: true

                onClicked: optionsMenu.open()

                Menu {
                    id: optionsMenu
                    x: 1
                    y: 1 + parent.height
                    visible: optionsMenuButton.checked
                    closePolicy: Popup.CloseOnPressOutsideParent

                    Imagine.path: defaultImaginePath

                    MenuItem {
                        text: qsTr("Open Asset Directory")
                        onClicked: Qt.openUrlExternally(assetFixer.assetDirectoryUrl)
                        enabled: usingImagineStyle
                    }

                    MenuItem {
                        action: reloadAssetsAction
                    }

                    MenuItem {
                        action: useCustomAssetsAction
                    }

                    MenuItem {
                        action: fixAssetsAction
                    }

                    MenuSeparator {}

                    MenuItem {
                        id: settingsMenuItem
                        text: qsTr("Settings")
                        onTriggered: settingsDialog.open()
                    }

                    MenuSeparator {}

                    MenuItem {
                        text: qsTr("Quit")
                        onTriggered: Qt.quit()
                    }
                }
            }
        }
    }

    SettingsDialog {
        id: settingsDialog

        Imagine.path: defaultImaginePath
    }

    Drawer {
        id: drawer
        width: parent.width * 0.33
        height: window.height
        focus: false
        modal: false

        Label {
            text: "Drawer contents go here"
            anchors.centerIn: parent
        }
    }

    AssetFixer {
        id: assetFixer
        assetDirectory: settings.imaginePath
        // Don't start watching until the settings have loaded, as AssetFixer can be completed before it.
        // AssetFixer needs the settings in order to check the last modified time of the asset directory.
        // Also, wait until the UI has been rendered for the first time so that we can show our busy indicators, etc.
        shouldWatch: usingImagineStyle && settings.useCustomImaginePath && settingsLoaded && initialUiRenderDelayTimer.hasRun
        shouldFix: (shouldWatch && settings.autoFixImagineAssets) || manuallyFixing

        onFixSuggested: autoFix()
        onDelayedFixSuggested: assetFixerFileSystemDelayTimer.restart()
        onReloadSuggested: reloadAssets()

        property bool manuallyFixing: false

        function reloadAssets() {
            console.log(brief, "Reloading assets...")
            // Clear the model, otherwise ListView will keep the old items around
            // with the old assets, even after clearing the pixmap cache
            listView.resettingModel = true
            listView.model = null
            window.Imagine.path = ""
            assetReloadNextFrameTimer.start()
        }

        function autoFix() {
            // This is a bit of a hack, but I can't think of a nice way to solve it.
            // The problem is that shouldWatch becomes true, causing startWatching() to be called.
            // If a fix is suggested as a result of that, this function is called.
            // However, the shouldFix binding hasn't been updated yet, so even though shouldWatch
            // and settings.autoFixImagineAssets are both true (the properties that make up its binding),
            // the if check below fails. So, we check for that case with effectiveShouldFix.
            var effectiveShouldFix = shouldWatch && settings.autoFixImagineAssets;
            if (shouldWatch && effectiveShouldFix && assetDirectory.length > 0) {
                fixEmUp();
            }
        }

        function manualFix() {
            fixEmUp(true);
        }

        function fixEmUp(manually) {
            assetFixer.manuallyFixing = !!manually

            // Disable image caching if it hasn't already been done.
            assetFixer.clearImageCache()

            busyIndicatorRow.visible = true
            assetFixerAnimationDelayTimer.start()
        }
    }

    // The controls' assets don't always "reload" if the path is cleared and then set in the same frame,
    // so we delay the setting to the next frame.
    Timer {
        id: assetReloadNextFrameTimer
        interval: 0
        onTriggered: {
            window.Imagine.path = Qt.binding(function() {
                return settings.useCustomImaginePath && settings.imaginePath.length > 0 ? settings.imaginePath : undefined
            })

            infoToolTip.text = "Reloaded assets"
            infoToolTip.timeout = 1500
            infoToolTip.open()

            listView.model = controlFolderListModel
            listView.resettingModel = false

            console.log(brief, "... reloaded assets.")
        }
    }

    // When exporting or deleting a large amount of assets (not uncommon),
    // the filesystem watcher seems to emit directoryChanged() every second or so,
    // so rather than process hundreds of assets every time we get notified, delay
    // it until we haven't been notified for a while.
    Timer {
        id: assetFixerFileSystemDelayTimer
        interval: 2000
        onRunningChanged: {
            if (running) {
                infoToolTip.text = "Assets changed on disk - reloading in 2 seconds if no further changes are detected"
                infoToolTip.timeout = 2000
                infoToolTip.open()
            }
        }
        onTriggered: assetFixer.autoFix()
    }

    // Gives the BusyIndicator animation a chance to start.
    Timer {
        id: assetFixerAnimationDelayTimer
        interval: 100
        onTriggered: {
            assetFixer.fixAssets()
            busyIndicatorRow.visible = false
            assetFixer.manuallyFixing = false
        }
    }

    // Gives the UI a chance to render before the initial fixup.
    Timer {
        id: initialUiRenderDelayTimer
        interval: 300
        running: true
        onTriggered: hasRun = true

        property bool hasRun: false
    }

    function getControlElements(control) {
        var props = [];
        for (var p in control) {
            if (p !== "component" && typeof control[p] === 'object')
                props.push(p);
        }
        return props;
    }

    Pane {
        id: contentPane
        anchors.fill: parent

        Imagine.path: settings.useCustomImaginePath && settings.imaginePath.length > 0 ? settings.imaginePath : undefined

        palette.window: effectiveColor(paletteSettings.window)
        palette.windowText: effectiveColor(paletteSettings.windowText)
        palette.base: effectiveColor(paletteSettings.base)
        palette.text: effectiveColor(paletteSettings.text)
        palette.button: effectiveColor(paletteSettings.button)
        palette.buttonText: effectiveColor(paletteSettings.buttonText)
        palette.brightText: effectiveColor(paletteSettings.brightText)
        palette.toolTipBase: effectiveColor(paletteSettings.toolTipBase)
        palette.toolTipText: effectiveColor(paletteSettings.toolTipText)
        palette.light: effectiveColor(paletteSettings.light)
        palette.midlight: effectiveColor(paletteSettings.midlight)
        palette.dark: effectiveColor(paletteSettings.dark)
        palette.mid: effectiveColor(paletteSettings.mid)
        palette.shadow: effectiveColor(paletteSettings.shadow)
        palette.highlight: effectiveColor(paletteSettings.highlight)
        palette.highlightedText: effectiveColor(paletteSettings.highlightedText)
        palette.link: effectiveColor(paletteSettings.link)

        function effectiveColor(paletteColorString) {
            return paletteSettings.useCustomPalette && paletteColorString.length > 0 ? paletteColorString : undefined
        }

        FolderListModel {
            id: controlFolderListModel
            folder: "qrc:/controls"
            showDirs: false
            nameFilters: searchTextField.text.length > 0 ? ["*" + searchTextField.text + "*.qml"] : []
            caseSensitive: false
        }

        ListView {
            id: listView
            anchors.fill: parent
            spacing: 30
            visible: !busyIndicatorRow.visible && !resettingModel

            property bool resettingModel: false

            ScrollBar.vertical: ScrollBar {
                parent: contentPane
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
            }

            model: controlFolderListModel
            delegate: ColumnLayout {
                id: rootDelegate
                width: parent.width

                MenuSeparator {
                    Layout.fillWidth: true
                    visible: index !== 0
                }

                Label {
                    text: customControlName.length === 0 ? model.fileBaseName : customControlName
                    font.pixelSize: Qt.application.font.pixelSize * 2
                }

                readonly property var controlName: model.fileBaseName
                readonly property var controlMetaObject: controlMetaObjectLoader.item
                readonly property string customControlName: controlMetaObject && controlMetaObject.hasOwnProperty("customControlName")
                    ? controlMetaObject.customControlName : ""
                readonly property var supportedStates: rootDelegate.controlMetaObject.supportedStates
                readonly property int maxStateCombinations: {
                    var largest = 0;
                    for (var i = 0; i < supportedStates.length; ++i) {
                        var combinations = supportedStates[i];
                        if (combinations.length > largest)
                            largest = combinations.length;
                    }
                    return largest;
                }

                Loader {
                    id: controlMetaObjectLoader
                    source: "qrc" + model.filePath
                }

                Flow {
                    spacing: 10

                    Layout.fillWidth: true

                    Repeater {
                        id: stateRepeater
                        model: rootDelegate.supportedStates

                        ColumnLayout {
                            id: labelWithDelegatesColumn
                            spacing: 4

                            readonly property var states: modelData
                            readonly property string statesAsString: states.join("\n")

                            Label {
                                text: statesAsString.length > 0 ? statesAsString : "normal"

                                // 4 is the most states for any element (Button)
                                Layout.preferredHeight: (fontMetrics.lineSpacing) * (rootDelegate.maxStateCombinations + 1)
                            }

                            ControlContainer {
                                id: controlContainer
                                objectName: controlName + "ControlContainer"
                                controlMetaObject: rootDelegate.controlMetaObject
                                states: labelWithDelegatesColumn.states

                                Layout.alignment: Qt.AlignHCenter
                            }
                        }
                    }
                }

                ExampleContainer {
                    id: exampleContainer
                    controlMetaObject: rootDelegate.controlMetaObject
                    visible: !!controlMetaObject.exampleComponent

                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: visible ? 14 : 0
                    Layout.fillWidth: true
                    Layout.preferredHeight: visible ? implicitHeight : 0
                }
            }
        }
    }

    RowLayout {
        id: busyIndicatorRow
        anchors.centerIn: parent
        visible: false

        BusyIndicator {
            id: busyIndicator
            running: visible
        }

        Label {
            text: qsTr("Fixing assets...")
            font.pixelSize: Qt.application.font.pixelSize * 2
        }
    }

    ToolTip {
        id: infoToolTip
        x: (parent.width - width) / 2
        y: parent.height - height - 40
        parent: window.contentItem
    }
}

