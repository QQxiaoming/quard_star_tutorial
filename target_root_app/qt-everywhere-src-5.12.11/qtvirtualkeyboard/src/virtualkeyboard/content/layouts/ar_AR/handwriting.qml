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
import QtQuick.Layouts 1.0
import QtQuick.VirtualKeyboard 2.3

KeyboardLayout {
    function createInputMethod() {
        return Qt.createQmlObject('import QtQuick 2.0; import QtQuick.VirtualKeyboard.Plugins 2.3; HandwritingInputMethod {}', parent)
    }
    sharedLayouts: ['symbols']
    inputMode: InputEngine.InputMode.Arabic
    readonly property bool alphabeticInputMode: [InputEngine.InputMode.Latin, InputEngine.InputMode.Dialable].indexOf(InputContext.inputEngine.inputMode) !== -1

    KeyboardRow {
        Layout.preferredHeight: 3
        KeyboardColumn {
            Layout.preferredWidth: bottomRow.width - hideKeyboardKey.width
            KeyboardRow {
                TraceInputKey {
                    objectName: "hwrInputArea"
                    patternRecognitionMode: InputEngine.PatternRecognitionMode.Handwriting
                    horizontalRulers:
                        alphabeticInputMode ? [] :
                            [Math.round(boundingBox.height / 4), Math.round(boundingBox.height / 4) * 2, Math.round(boundingBox.height / 4) * 3]
                }
            }
        }
        KeyboardColumn {
            Layout.preferredWidth: hideKeyboardKey.width
            KeyboardRow {
                BackspaceKey {}
            }
            KeyboardRow {
                EnterKey {}
            }
            KeyboardRow {
                ShiftKey { }
            }
        }
    }
    KeyboardRow {
        id: bottomRow
        Layout.preferredHeight: 1
        keyWeight: 154
        InputModeKey {
            weight: 217
            inputModeNameList: [
                "ABC",  // InputEngine.InputMode.Latin
                "\u0660\u0661\u0662",  // InputEngine.InputMode.Numeric
                "123",  // InputEngine.InputMode.Dialable
                "",  // InputEngine.InputMode.Pinyin
                "",  // InputEngine.InputMode.Cangjie
                "",  // InputEngine.InputMode.Zhuyin
                "",  // InputEngine.InputMode.Hangul
                "",  // InputEngine.InputMode.Hiragana
                "",  // InputEngine.InputMode.Katakana
                "",  // InputEngine.InputMode.FullwidthLatin
                "",  // InputEngine.InputMode.Greek
                "",  // InputEngine.InputMode.Cyrillic
                "\u0623\u200C\u0628\u200C\u062C",  // InputEngine.InputMode.Arabic
            ]
        }
        ChangeLanguageKey {
            weight: 154
            customLayoutsOnly: true
        }
        HandwritingModeKey {
            weight: 154
        }
        SpaceKey {
            weight: 864
        }
        Key {
            key: alphabeticInputMode ? Qt.Key_Apostrophe : 0x060C
            text: alphabeticInputMode ? "'" : "\u060C"
            alternativeKeys: alphabeticInputMode ? "<>()#%&*/\\\"'=+-_" : "\"\u060C\u066B\u066C\u066D"
        }
        Key {
            key: Qt.Key_Period
            text: "."
            alternativeKeys: alphabeticInputMode ? ":;,.?!" : ":\u061B.\u061F!"
        }
        HideKeyboardKey {
            id: hideKeyboardKey
            weight: 204
        }
    }
}
