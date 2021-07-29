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

import QtQuick 2.10
import QtQuick.VirtualKeyboard 2.3

KeyboardLayoutLoader {
    sharedLayouts: ['symbols']
    sourceComponent: InputContext.inputEngine.inputMode === InputEngine.InputMode.Cyrillic ? cyrillicLayout : latinLayout
    Component {
        id: cyrillicLayout
        KeyboardLayout {
            keyWeight: 160
            KeyboardRow {
                Key {
                    key: 0x0419
                    text: "й"
                }
                Key {
                    key: 0x0426
                    text: "ц"
                }
                Key {
                    key: 0x0423
                    text: "у"
                }
                Key {
                    key: 0x041A
                    text: "к"
                }
                Key {
                    key: 0x0415
                    text: "е"
                    alternativeKeys: "её"
                }
                Key {
                    key: 0x041D
                    text: "н"
                }
                Key {
                    key: 0x0413
                    text: "г"
                }
                Key {
                    key: 0x0428
                    text: "ш"
                }
                Key {
                    key: 0x0429
                    text: "щ"
                }
                Key {
                    key: 0x0417
                    text: "з"
                }
                Key {
                    key: 0x0425
                    text: "х"
                }
                BackspaceKey {
                    weight: 180
                }
            }
            KeyboardRow {
                FillerKey {
                    weight: 40
                }
                Key {
                    key: 0x0424
                    text: "ф"
                }
                Key {
                    key: 0x042B
                    text: "ы"
                }
                Key {
                    key: 0x0412
                    text: "в"
                }
                Key {
                    key: 0x0410
                    text: "а"
                }
                Key {
                    key: 0x041F
                    text: "п"
                }
                Key {
                    key: 0x0420
                    text: "р"
                }
                Key {
                    key: 0x041E
                    text: "о"
                }
                Key {
                    key: 0x041B
                    text: "л"
                }
                Key {
                    key: 0x0414
                    text: "д"
                }
                Key {
                    key: 0x0416
                    text: "ж"
                }
                Key {
                    key: 0x042D
                    text: "э"
                }
                EnterKey {
                    weight: 280
                }
            }
            KeyboardRow {
                InputModeKey {
                    inputModes: [InputEngine.InputMode.Cyrillic, InputEngine.InputMode.Latin]
                }
                Key {
                    key: 0x042F
                    text: "я"
                }
                Key {
                    key: 0x0427
                    text: "ч"
                }
                Key {
                    key: 0x0421
                    text: "с"
                }
                Key {
                    key: 0x041C
                    text: "м"
                }
                Key {
                    key: 0x0418
                    text: "и"
                }
                Key {
                    key: 0x0422
                    text: "т"
                }
                Key {
                    key: 0x042C
                    text: "ь"
                    alternativeKeys: "ьъ"
                }
                Key {
                    key: 0x0411
                    text: "б"
                }
                Key {
                    key: 0x042E
                    text: "ю"
                }
                Key {
                    key: Qt.Key_Minus
                    text: "-"
                    alternativeKeys: "—"
                }
                ShiftKey {
                    weight: 272
                }
            }
            KeyboardRow {
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
                    alternativeKeys: ";:!?,."
                }
                Key {
                    key: 0xE000
                    text: ":-)"
                    alternativeKeys: [ ";-)", ":-)", ":-D", ":-(", "<3" ]
                }
                HideKeyboardKey {
                    weight: 204
                }
            }
        }
    }
    Component {
        id: latinLayout
        KeyboardLayout {
            keyWeight: 160
            KeyboardRow {
                Key {
                    key: Qt.Key_Q
                    text: "q"
                }
                Key {
                    key: Qt.Key_W
                    text: "w"
                }
                Key {
                    key: Qt.Key_E
                    text: "e"
                }
                Key {
                    key: Qt.Key_R
                    text: "r"
                }
                Key {
                    key: Qt.Key_T
                    text: "t"
                }
                Key {
                    key: Qt.Key_Z
                    text: "z"
                    alternativeKeys: "zž"
                }
                Key {
                    key: Qt.Key_U
                    text: "u"
                }
                Key {
                    key: Qt.Key_I
                    text: "i"
                }
                Key {
                    key: Qt.Key_O
                    text: "o"
                    alternativeKeys: "oö"
                }
                Key {
                    key: Qt.Key_P
                    text: "p"
                }
                BackspaceKey {}
            }
            KeyboardRow {
                FillerKey {
                    weight: 56
                }
                Key {
                    key: Qt.Key_A
                    text: "a"
                    alternativeKeys: "aåä"
                }
                Key {
                    key: Qt.Key_S
                    text: "s"
                    alternativeKeys: "sš"
                }
                Key {
                    key: Qt.Key_D
                    text: "d"
                    alternativeKeys: "dđ"
                }
                Key {
                    key: Qt.Key_F
                    text: "f"
                }
                Key {
                    key: Qt.Key_G
                    text: "g"
                }
                Key {
                    key: Qt.Key_H
                    text: "h"
                }
                Key {
                    key: Qt.Key_J
                    text: "j"
                }
                Key {
                    key: Qt.Key_K
                    text: "k"
                }
                Key {
                    key: Qt.Key_L
                    text: "l"
                }
                EnterKey {
                    weight: 283
                }
            }
            KeyboardRow {
                keyWeight: 156
                InputModeKey {
                    inputModes: [InputEngine.InputMode.Cyrillic, InputEngine.InputMode.Latin]
                }
                Key {
                    key: Qt.Key_Y
                    text: "y"
                }
                Key {
                    key: Qt.Key_X
                    text: "x"
                }
                Key {
                    key: Qt.Key_C
                    text: "c"
                    alternativeKeys: "ćcč"
                }
                Key {
                    key: Qt.Key_V
                    text: "v"
                }
                Key {
                    key: Qt.Key_B
                    text: "b"
                }
                Key {
                    key: Qt.Key_N
                    text: "n"
                }
                Key {
                    key: Qt.Key_M
                    text: "m"
                }
                Key {
                    key: Qt.Key_Comma
                    text: ","
                }
                Key {
                    key: Qt.Key_Period
                    text: "."
                }
                ShiftKey {
                    weight: 204
                }
            }
            KeyboardRow {
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
                    key: Qt.Key_Minus
                    text: "-"
                    alternativeKeys: "-\"'"
                }
                Key {
                    key: 0xE000
                    text: ":-)"
                    alternativeKeys: [ ";-)", ":-)", ":-D", ":-(", "<3" ]
                }
                HideKeyboardKey {
                    weight: 204
                }
            }
        }
    }
}
