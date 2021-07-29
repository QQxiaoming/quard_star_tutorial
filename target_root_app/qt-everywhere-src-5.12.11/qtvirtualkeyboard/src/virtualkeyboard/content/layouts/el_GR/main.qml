/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
import QtQuick.VirtualKeyboard 2.3

KeyboardLayoutLoader {
    sharedLayouts: ['symbols']
    sourceComponent: InputContext.inputEngine.inputMode === InputEngine.InputMode.Greek ? greekLayout : latinLayout
    Component {
        id: greekLayout
        KeyboardLayout {
            keyWeight: 160
            KeyboardRow {
                Key {
                    text: "ς"
                }
                Key {
                    text: "ε"
                    alternativeKeys: "εέ"
                }
                Key {
                    text: "ρ"
                }
                Key {
                    text: "τ"
                }
                Key {
                    text: "ψ"
                }
                Key {
                    text: "υ"
                    alternativeKeys: "υύϋΰ"
                }
                Key {
                    text: "θ"
                }
                Key {
                    text: "ι"
                    alternativeKeys: "ιίϊΐ"
                }
                Key {
                    text: "ο"
                    alternativeKeys: "οό"
                }
                Key {
                    text: "π"
                }
                BackspaceKey {}
            }
            KeyboardRow {
                FillerKey {
                    weight: 56
                }
                Key {
                    text: "α"
                    alternativeKeys: "αά"
                }
                Key {
                    text: "σ"
                }
                Key {
                    text: "δ"
                }
                Key {
                    text: "φ"
                }
                Key {
                    text: "γ"
                }
                Key {
                    text: "η"
                    alternativeKeys: "ηή"
                }
                Key {
                    text: "ξ"
                }
                Key {
                    text: "κ"
                }
                Key {
                    text: "λ"
                }
                EnterKey {
                    weight: 283
                }
            }
            KeyboardRow {
                keyWeight: 156
                InputModeKey {
                    inputModes: [InputEngine.InputMode.Greek, InputEngine.InputMode.Latin]
                }
                Key {
                    text: "ζ"
                }
                Key {
                    text: "χ"
                }
                Key {
                    text: "ψ"
                }
                Key {
                    text: "ω"
                    alternativeKeys: "ωώ"
                }
                Key {
                    text: "β"
                }
                Key {
                    text: "ν"
                }
                Key {
                    text: "μ"
                }
                Key {
                    key: Qt.Key_Comma
                    text: ","
                }
                Key {
                    key: Qt.Key_Period
                    text: "."
                    alternativeKeys: "!.;?"
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
                    key: Qt.Key_Apostrophe
                    text: "'"
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
                    alternativeKeys: "êeëèé"
                }
                Key {
                    key: Qt.Key_R
                    text: "r"
                    alternativeKeys: "ŕrř"
                }
                Key {
                    key: Qt.Key_T
                    text: "t"
                    alternativeKeys: "ţtŧť"
                }
                Key {
                    key: Qt.Key_Y
                    text: "y"
                    alternativeKeys: "ÿyýŷ"
                }
                Key {
                    key: Qt.Key_U
                    text: "u"
                    alternativeKeys: "űūũûüuùú"
                }
                Key {
                    key: Qt.Key_I
                    text: "i"
                    alternativeKeys: "îïīĩiìí"
                }
                Key {
                    key: Qt.Key_O
                    text: "o"
                    alternativeKeys: "œøõôöòóo"
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
                    alternativeKeys: (InputContext.inputMethodHints & (Qt.ImhEmailCharactersOnly | Qt.ImhUrlCharactersOnly)) ? "a@äåãâàá" : "aäåãâàá"
                    smallTextVisible: (InputContext.inputMethodHints & (Qt.ImhEmailCharactersOnly | Qt.ImhUrlCharactersOnly))
                }
                Key {
                    key: Qt.Key_S
                    text: "s"
                    alternativeKeys: "šsşś"
                }
                Key {
                    key: Qt.Key_D
                    text: "d"
                    alternativeKeys: "dđď"
                }
                Key {
                    key: Qt.Key_F
                    text: "f"
                }
                Key {
                    key: Qt.Key_G
                    text: "g"
                    alternativeKeys: "ġgģĝğ"
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
                    alternativeKeys: "ĺŀłļľl"
                }
                EnterKey {
                    weight: 283
                }
            }
            KeyboardRow {
                keyWeight: 156
                InputModeKey {
                    inputModes: [InputEngine.InputMode.Greek, InputEngine.InputMode.Latin]
                }
                Key {
                    key: Qt.Key_Z
                    text: "z"
                    alternativeKeys: "zžż"
                }
                Key {
                    key: Qt.Key_X
                    text: "x"
                }
                Key {
                    key: Qt.Key_C
                    text: "c"
                    alternativeKeys: "çcċčć"
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
                    alternativeKeys: "ņńnň"
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
                    key: Qt.Key_Apostrophe
                    text: "'"
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
