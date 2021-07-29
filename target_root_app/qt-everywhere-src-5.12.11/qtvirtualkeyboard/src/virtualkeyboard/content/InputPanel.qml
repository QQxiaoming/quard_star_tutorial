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
import QtQuick.VirtualKeyboard 2.1

/*!
    \qmltype InputPanel
    \inqmlmodule QtQuick.VirtualKeyboard

    \brief Provides the virtual keyboard UI.
    \ingroup qtvirtualkeyboard-qml

    The keyboard size is automatically calculated from the available
    width; that is, the keyboard maintains the aspect ratio specified by the current
    style. Therefore the application should only set the \l {Item::}{width} and \l {Item::}{y}
    coordinates of the InputPanel, and not the \l {Item::}{height}.

    As with \l {Qt Virtual Keyboard QML Types}{all other QML types} provided by
    the module, the \c QT_IM_MODULE environment variable must be set to
    \c qtvirtualkeyboard before using InputPanel. For more information, see
    \l {Loading the Plugin}.
*/

Item {
    id: inputPanel

    /*!
        \qmlproperty bool InputPanel::active
        \since QtQuick.VirtualKeyboard 2.0

        This property reflects the active status of the input panel.
        The keyboard should be made visible to the user when this property is
        \c true.
    */
    property alias active: keyboard.active

    /*!
        \qmlproperty bool InputPanel::externalLanguageSwitchEnabled
        \since QtQuick.VirtualKeyboard 2.4

        This property enables the external language switch mechanism.
        When this property is \c true, the virtual keyboard will not show
        the built-in language popup, but will emit the \l externalLanguageSwitch
        signal instead. The application can handle this signal and show a
        custom language selection dialog instead.
    */
    property bool externalLanguageSwitchEnabled

    /*!
        \qmlsignal InputPanel::externalLanguageSwitch(var localeList, int currentIndex)
        \since QtQuick.VirtualKeyboard 2.4

        This signal is emitted when \l externalLanguageSwitchEnabled is \c true
        and the \l {user-guide-language}{language switch key} is pressed by the user.

        It serves as a hook to display a custom language dialog instead of
        the built-in language popup in the virtual keyboard.

        The \a localeList parameter contains a list of locale names to choose
        from. To get more information about a particular language, use the
        \l {QtQml::Qt::locale()}{Qt.locale()} function. The \a currentIndex
        is the index of current locale in the \a localeList. This item should
        be highlighted as the current item in the UI.

        To select a new language, use the \l {VirtualKeyboardSettings::locale}
        {VirtualKeyboardSettings.locale} property.

        Below is an example that demonstrates a custom language dialog implementation:

        \snippet qtvirtualkeyboard-custom-language-popup.qml popup

        The dialog would then be declared:

        \snippet qtvirtualkeyboard-custom-language-popup.qml declaring

        In the application's InputPanel, add the following code:

        \snippet qtvirtualkeyboard-custom-language-popup.qml using

        The custom dialog will now be shown when the language switch key is pressed.
    */
    signal externalLanguageSwitch(var localeList, int currentIndex)

    /*! \internal */
    property alias keyboard: keyboard

    /*! \internal */
    readonly property bool __isRootItem: inputPanel.parent != null && inputPanel.parent.parent == null

    SelectionControl {
        objectName: "selectionControl"
        x: -parent.x
        y: -parent.y
        enabled: active && !keyboard.fullScreenMode && !__isRootItem
    }

    implicitHeight: keyboard.height
    Keyboard {
        id: keyboard
        readonly property real yOffset: keyboard.wordCandidateView.currentYOffset - (keyboard.shadowInputControl.visible ? keyboard.shadowInputControl.height : 0)
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
    MouseArea {
        z: -1
        anchors.fill: keyboard
        enabled: active
    }

    Binding {
        target: InputContext.priv
        property: "keyboardRectangle"
        value: keyboardRectangle()
        when: !InputContext.animating
    }

    /*! \internal */
    function keyboardRectangle() {
        var rect = Qt.rect(0, keyboard.yOffset, keyboard.width, keyboard.height - keyboard.yOffset)
        if (__isRootItem) {
            rect.x += keyboard.x
            rect.y += keyboard.y
        }
        // Read the inputPanel position.
        // This ensures that the Binding works.
        var unusedX = inputPanel.x
        var unusedY = inputPanel.y
        return mapToItem(null, rect.x, rect.y, rect.width, rect.height)
    }
}
