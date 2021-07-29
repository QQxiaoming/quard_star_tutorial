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

#include "qtquickvirtualkeyboardplugin.h"

#include <QtVirtualKeyboard/qvirtualkeyboardinputcontext.h>
#include <QtVirtualKeyboard/qvirtualkeyboardinputengine.h>
#include <QtVirtualKeyboard/private/qvirtualkeyboardinputcontext_p.h>
#include <QtVirtualKeyboard/private/shifthandler_p.h>
#include <QtVirtualKeyboard/private/inputmethod_p.h>
#include <QtVirtualKeyboard/qvirtualkeyboardselectionlistmodel.h>
#include <QtVirtualKeyboard/private/enterkeyaction_p.h>
#include <QtVirtualKeyboard/private/enterkeyactionattachedtype_p.h>
#include <QtVirtualKeyboard/qvirtualkeyboardtrace.h>
#include <QtVirtualKeyboard/private/shadowinputcontext_p.h>
#include <QtVirtualKeyboard/private/qvirtualkeyboard_staticplugin_p.h>

QT_BEGIN_NAMESPACE

using namespace QtVirtualKeyboard;

static QObject *createInputContextModule(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new QVirtualKeyboardInputContext();
}

void QtQuickVirtualKeyboardPlugin::registerTypes(const char *uri)
{
#if defined(QT_STATICPLUGIN)
    Q_VKB_IMPORT_PLUGIN(QtQuick2Plugin)
    Q_VKB_IMPORT_PLUGIN(QtQuick2WindowPlugin)
    Q_VKB_IMPORT_PLUGIN(QtQuickLayoutsPlugin)
    Q_VKB_IMPORT_PLUGIN(QmlFolderListModelPlugin)
    Q_VKB_IMPORT_PLUGIN(QtQuickVirtualKeyboardSettingsPlugin)
    Q_VKB_IMPORT_PLUGIN(QtQuickVirtualKeyboardStylesPlugin)
#endif

    qmlRegisterSingletonType<QVirtualKeyboardInputContext>(uri, 1, 0, "InputContext", createInputContextModule);
    qmlRegisterSingletonType<QVirtualKeyboardInputContext>(uri, 2, 0, "InputContext", createInputContextModule);
    qmlRegisterUncreatableType<QVirtualKeyboardInputContextPrivate>(uri, 1, 0, "InputContextPrivate", QLatin1String("Cannot create input context private"));
    qmlRegisterUncreatableType<QVirtualKeyboardInputEngine>(uri, 1, 0, "InputEngine", QLatin1String("Cannot create input method engine"));
    qmlRegisterUncreatableType<QVirtualKeyboardInputEngine>(uri, 2, 0, "InputEngine", QLatin1String("Cannot create input method engine"));
    qRegisterMetaType<ShiftHandler *>("ShiftHandler*");
    qmlRegisterUncreatableType<ShiftHandler>(uri, 1, 0, "ShiftHandler", QLatin1String("Cannot create shift handler"));
    qmlRegisterUncreatableType<ShiftHandler>(uri, 2, 0, "ShiftHandler", QLatin1String("Cannot create shift handler"));
    qmlRegisterUncreatableType<QVirtualKeyboardSelectionListModel>(uri, 1, 0, "SelectionListModel", QLatin1String("Cannot create selection list model"));
    qmlRegisterUncreatableType<QVirtualKeyboardSelectionListModel>(uri, 2, 0, "SelectionListModel", QLatin1String("Cannot create selection list model"));
    qmlRegisterUncreatableType<QVirtualKeyboardAbstractInputMethod>(uri, 1, 0, "AbstractInputMethod", QLatin1String("Cannot create abstract input method"));
    qmlRegisterUncreatableType<QVirtualKeyboardAbstractInputMethod>(uri, 2, 0, "AbstractInputMethod", QLatin1String("Cannot create abstract input method"));
    qmlRegisterType<InputMethod>(uri, 1, 0, "InputMethod");
    qmlRegisterType<InputMethod>(uri, 2, 0, "InputMethod");
    qmlRegisterType<EnterKeyActionAttachedType>();
    qmlRegisterType<EnterKeyAction>(uri, 1, 0, "EnterKeyAction");
    qmlRegisterType<EnterKeyAction>(uri, 2, 0, "EnterKeyAction");
    qmlRegisterType<QVirtualKeyboardTrace>(uri, 2, 0, "Trace");
    qmlRegisterType<QVirtualKeyboardTrace>(uri, 2, 4, "Trace");
    qRegisterMetaType<ShadowInputContext *>("ShadowInputContext*");
    qmlRegisterUncreatableType<ShadowInputContext>(uri, 2, 2, "ShadowInputContext", QLatin1String("Cannot create shadow input context"));

    const QString path(QStringLiteral("qrc:///QtQuick/VirtualKeyboard/content/"));
    qmlRegisterType(QUrl(path + QLatin1String("InputPanel.qml")), uri, 1, 0, "InputPanel");
    qmlRegisterType(QUrl(path + QLatin1String("InputPanel.qml")), uri, 1, 2, "InputPanel");
    qmlRegisterType(QUrl(path + QLatin1String("InputPanel.qml")), uri, 1, 3, "InputPanel");
    qmlRegisterType(QUrl(path + QLatin1String("InputPanel.qml")), uri, 2, 0, "InputPanel");
    qmlRegisterType(QUrl(path + QLatin1String("InputPanel.qml")), uri, 2, 1, "InputPanel");
    qmlRegisterType(QUrl(path + QLatin1String("InputPanel.qml")), uri, 2, 2, "InputPanel");
    qmlRegisterType(QUrl(path + QLatin1String("InputPanel.qml")), uri, 2, 3, "InputPanel");
    qmlRegisterType(QUrl(path + QLatin1String("InputPanel.qml")), uri, 2, 4, "InputPanel");
    qmlRegisterType(QUrl(path + QLatin1String("HandwritingInputPanel.qml")), uri, 2, 0, "HandwritingInputPanel");
    const QString componentsPath = path + QStringLiteral("components/");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("AlternativeKeys.qml")), uri, 1, 0, "AlternativeKeys");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("AlternativeKeys.qml")), uri, 2, 0, "AlternativeKeys");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("BackspaceKey.qml")), uri, 1, 0, "BackspaceKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("BackspaceKey.qml")), uri, 2, 0, "BackspaceKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("BaseKey.qml")), uri, 1, 0, "BaseKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("BaseKey.qml")), uri, 2, 0, "BaseKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("ChangeLanguageKey.qml")), uri, 1, 0, "ChangeLanguageKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("ChangeLanguageKey.qml")), uri, 2, 0, "ChangeLanguageKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("CharacterPreviewBubble.qml")), uri, 1, 0, "CharacterPreviewBubble");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("CharacterPreviewBubble.qml")), uri, 2, 0, "CharacterPreviewBubble");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("EnterKey.qml")), uri, 1, 0, "EnterKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("EnterKey.qml")), uri, 2, 0, "EnterKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("FillerKey.qml")), uri, 1, 0, "FillerKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("FillerKey.qml")), uri, 2, 0, "FillerKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("HideKeyboardKey.qml")), uri, 1, 0, "HideKeyboardKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("HideKeyboardKey.qml")), uri, 2, 0, "HideKeyboardKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("KeyboardColumn.qml")), uri, 1, 0, "KeyboardColumn");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("KeyboardColumn.qml")), uri, 2, 0, "KeyboardColumn");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("KeyboardLayout.qml")), uri, 1, 0, "KeyboardLayout");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("KeyboardLayout.qml")), uri, 2, 0, "KeyboardLayout");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("KeyboardLayoutLoader.qml")), uri, 1, 1, "KeyboardLayoutLoader");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("KeyboardLayoutLoader.qml")), uri, 2, 0, "KeyboardLayoutLoader");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("Keyboard.qml")), uri, 1, 0, "Keyboard");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("Keyboard.qml")), uri, 2, 0, "Keyboard");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("KeyboardRow.qml")), uri, 1, 0, "KeyboardRow");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("KeyboardRow.qml")), uri, 2, 0, "KeyboardRow");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("Key.qml")), uri, 1, 0, "Key");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("Key.qml")), uri, 2, 0, "Key");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("ModeKey.qml")), uri, 2, 0, "ModeKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("MultiSoundEffect.qml")), uri, 1, 1, "MultiSoundEffect");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("MultiSoundEffect.qml")), uri, 2, 0, "MultiSoundEffect");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("MultitapInputMethod.qml")), uri, 1, 0, "MultitapInputMethod");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("MultitapInputMethod.qml")), uri, 2, 0, "MultitapInputMethod");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("NumberKey.qml")), uri, 1, 0, "NumberKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("NumberKey.qml")), uri, 2, 0, "NumberKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("ShiftKey.qml")), uri, 1, 0, "ShiftKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("ShiftKey.qml")), uri, 2, 0, "ShiftKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("SpaceKey.qml")), uri, 1, 0, "SpaceKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("SpaceKey.qml")), uri, 2, 0, "SpaceKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("SymbolModeKey.qml")), uri, 1, 0, "SymbolModeKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("SymbolModeKey.qml")), uri, 2, 0, "SymbolModeKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("HandwritingModeKey.qml")), uri, 2, 0, "HandwritingModeKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("TraceInputArea.qml")), uri, 2, 0, "TraceInputArea");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("TraceInputKey.qml")), uri, 2, 0, "TraceInputKey");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("WordCandidatePopupList.qml")), uri, 2, 0, "WordCandidatePopupList");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("PopupList.qml")), uri, 2, 3, "PopupList");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("SelectionControl.qml")), uri, 2, 1, "SelectionControl");
    qmlRegisterType(QUrl(componentsPath + QLatin1String("InputModeKey.qml")), uri, 2, 3, "InputModeKey");

    // New revisions in 5.12.
    qmlRegisterSingletonType<QVirtualKeyboardInputContext>(uri, 2, 4, "InputContext", createInputContextModule);
}

QT_END_NAMESPACE
