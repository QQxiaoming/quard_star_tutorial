/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

KeyboardLayoutLoader {
    inputMode: InputEngine.InputMode.Latin
    function createInputMethod() {
        return Qt.createQmlObject('import QtQuick 2.0; import QtQuick.VirtualKeyboard.Plugins 2.3; ThaiInputMethod {}', parent, "thaiInputMethod")
    }
    sharedLayouts: [ "symbols" ]
    sourceComponent: InputContext.shiftActive ? page2 : page1
    Component {
        id: page1
        KeyboardLayout {
            keyWeight: 160
            KeyboardRow {
                Layout.preferredHeight: 3
                smallTextVisible: true
                KeyboardColumn {
                    Layout.preferredWidth: bottomRow.width - hideKeyboardKey.width
                    KeyboardRow {
                        Key {
                            text: "ๆ"
                            alternativeKeys: "ๆ๑"
                        }
                        Key {
                            text: "ๅ"
                            alternativeKeys: "ๅ๒"
                        }
                        Key {
                            text: "ภ"
                            alternativeKeys: "ภ๓"
                        }
                        Key {
                            text: "ถ"
                            alternativeKeys: "ถ๔"
                        }
                        Key {
                            text: "\u0E38"
                            displayText: " \u0E38"
                            alternativeKeys: [ "\u0E38", " \u0E39" ]
                        }
                        Key {
                            text: "\u0E36"
                            displayText: " \u0E36"
                        }
                        Key {
                            text: "ค"
                            alternativeKeys: "ค๕"
                        }
                        Key {
                            text: "ต"
                            alternativeKeys: "ต๖"
                        }
                        Key {
                            text: "จ"
                            alternativeKeys: "จ๗"
                        }
                        Key {
                            text: "ข"
                            alternativeKeys: "ข๘"
                        }
                        Key {
                            text: "ช"
                            alternativeKeys: "ช๙"
                        }
                    }
                    KeyboardRow {
                        Key {
                            text: "ไ"
                            alternativeKeys: "ไ฿"
                        }
                        Key {
                            text: " ำ"
                            alternativeKeys: " ำฎ"
                        }
                        Key {
                            text: "พ"
                            alternativeKeys: "พฑ"
                        }
                        Key {
                            text: "ะ"
                            alternativeKeys: "ะธ"
                        }
                        Key {
                            text: "\u0E31"
                            displayText: " \u0E31"
                            alternativeKeys: "\u0E31。"
                        }
                        Key {
                            text: "\u0E35"
                            displayText: " \u0E35"
                            alternativeKeys: [ "\u0E35", " \u0E4A" ]
                        }
                        Key {
                            text: "ร"
                            alternativeKeys: "รณ"
                        }
                        Key {
                            text: "น"
                            alternativeKeys: "นฯ"
                        }
                        Key {
                            text: "ย"
                            alternativeKeys: "ยญ"
                        }
                        Key {
                            text: "บ"
                            alternativeKeys: "บฐ"
                        }
                        Key {
                            text: "ล"
                        }
                    }
                    KeyboardRow {
                        Key {
                            text: "ฟ"
                            alternativeKeys: "ฟฤ"
                        }
                        Key {
                            text: "ห"
                            alternativeKeys: "หฆ"
                        }
                        Key {
                            text: "ก"
                            alternativeKeys: "กฏ"
                        }
                        Key {
                            text: "ด"
                            alternativeKeys: "ดโ"
                        }
                        Key {
                            text: "เ"
                            alternativeKeys: "เฌ"
                        }
                        Key {
                            text: "\u0E49"
                            displayText: " \u0E49"
                            alternativeKeys: [ "\u0E49", " \u0E47" ]
                        }
                        Key {
                            text: "\u0E48"
                            displayText: " \u0E48"
                            alternativeKeys: [ "\u0E48", " \u0E4B" ]
                        }
                        Key {
                            text: "า"
                            alternativeKeys: "าษ"
                        }
                        Key {
                            text: "ส"
                            alternativeKeys: "สศ"
                        }
                        Key {
                            text: "ว"
                            alternativeKeys: "วซ"
                        }
                        Key {
                            text: "ง"
                        }
                    }
                    KeyboardRow {
                        FillerKey {
                            weight: 80
                        }
                        Key {
                            text: "ผ"
                        }
                        Key {
                            text: "ป"
                        }
                        Key {
                            text: "แ"
                            alternativeKeys: "แฉ"
                        }
                        Key {
                            text: "อ"
                            alternativeKeys: "อฮ"
                        }
                        Key {
                            text: "\u0E34"
                            displayText: " \u0E34"
                            alternativeKeys: [ "\u0E34", " \u0E3A" ]
                        }
                        Key {
                            text: "\u0E37"
                            displayText: " \u0E37"
                            alternativeKeys: [ "\u0E37", " \u0E4C" ]
                        }
                        Key {
                            text: "ท"
                        }
                        Key {
                            text: "ม"
                            alternativeKeys: "มฒ"
                        }
                        Key {
                            text: "ใ"
                            alternativeKeys: "ใฬ"
                        }
                        Key {
                            text: "ฝ"
                            alternativeKeys: "ฝฦ"
                        }
                        FillerKey {
                            weight: 80
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
                keyWeight: 154
                SymbolModeKey {
                    weight: 217
                }
                ChangeLanguageKey {
                    weight: 154
                }
                HandwritingModeKey {
                    weight: 154
                }
                SpaceKey {
                    weight: 864
                }
                Key {
                    key: Qt.Key_Period
                    text: "."
                    alternativeKeys: "!?.,"
                    smallTextVisible: true
                }
                Key {
                    key: 0xE000
                    text: ":-)"
                    alternativeKeys: [ ";-)", ":-)", ":-D", ":-(", "<3" ]
                }
                HideKeyboardKey {
                    id: hideKeyboardKey
                    weight: 204
                }
            }
        }
    }
    Component {
        id: page2
        KeyboardLayout {
            keyWeight: 160
            KeyboardRow {
                Layout.preferredHeight: 3
                smallTextVisible: true
                KeyboardColumn {
                    Layout.preferredWidth: bottomRow.width - hideKeyboardKey.width
                    KeyboardRow {
                        Key {
                            text: "๑"
                        }
                        Key {
                            text: "๒"
                        }
                        Key {
                            text: "๓"
                        }
                        Key {
                            text: "๔"
                        }
                        Key {
                            text: "\u0E39"
                            displayText: " \u0E39"
                        }
                        Key {
                            enabled: false
                        }
                        Key {
                            text: "๕"
                        }
                        Key {
                            text: "๖"
                        }
                        Key {
                            text: "๗"
                        }
                        Key {
                            text: "๘"
                        }
                        Key {
                            text: "๙"
                        }
                    }
                    KeyboardRow {
                        Key {
                            text: "฿"
                        }
                        Key {
                            text: "ฎ"
                        }
                        Key {
                            text: "ฑ"
                        }
                        Key {
                            text: "ธ"
                        }
                        Key {
                            text: "。"
                        }
                        Key {
                            text: "\u0E4A"
                            displayText: " \u0E4A"
                        }
                        Key {
                            text: "ณ"
                        }
                        Key {
                            text: "ฯ"
                        }
                        Key {
                            text: "ญ"
                        }
                        Key {
                            text: "ฐ"
                        }
                        Key {
                            enabled: false
                        }
                    }
                    KeyboardRow {
                        Key {
                            text: "ฤ"
                        }
                        Key {
                            text: "ฆ"
                        }
                        Key {
                            text: "ฏ"
                        }
                        Key {
                            text: "โ"
                        }
                        Key {
                            text: "ฌ"
                        }
                        Key {
                            text: "\u0E47"
                            displayText: " \u0E47"
                        }
                        Key {
                            text: "\u0E4B"
                            displayText: " \u0E4B"
                        }
                        Key {
                            text: "ษ"
                        }
                        Key {
                            text: "ศ"
                        }
                        Key {
                            text: "ซ"
                        }
                        Key {
                            enabled: false
                        }
                    }
                    KeyboardRow {
                        FillerKey {
                            weight: 80
                        }
                        Key {
                            enabled: false
                        }
                        Key {
                            enabled: false
                        }
                        Key {
                            text: "ฉ"
                        }
                        Key {
                            text: "ฮ"
                        }
                        Key {
                            text: "\u0E3A"
                            displayText: " \u0E3A"
                        }
                        Key {
                            text: "\u0E4C"
                            displayText: " \u0E4C"
                        }
                        Key {
                            enabled: false
                        }
                        Key {
                            text: "ฒ"
                        }
                        Key {
                            text: "ฬ"
                        }
                        Key {
                            text: "ฦ"
                        }
                        FillerKey {
                            weight: 80
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
                keyWeight: 154
                SymbolModeKey {
                    weight: 217
                }
                ChangeLanguageKey {
                    weight: 154
                }
                HandwritingModeKey {
                    weight: 154
                }
                SpaceKey {
                    weight: 864
                }
                Key {
                    key: Qt.Key_Comma
                    text: ","
                    alternativeKeys: "!?,"
                }
                Key {
                    key: 0xE000
                    text: ":-)"
                    alternativeKeys: [ ";-)", ":-)", ":-D", ":-(", "<3" ]
                }
                HideKeyboardKey {
                    id: hideKeyboardKey
                    weight: 204
                }
            }
        }
    }
}
