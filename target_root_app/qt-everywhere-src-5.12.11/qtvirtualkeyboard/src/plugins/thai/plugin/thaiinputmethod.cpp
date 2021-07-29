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

#include "thaiinputmethod_p.h"
#include <QtVirtualKeyboard/qvirtualkeyboardinputengine.h>
#include <QtVirtualKeyboard/qvirtualkeyboardinputcontext.h>

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

/*!
    \class QtVirtualKeyboard::ThaiInputMethod
    \internal
*/

ThaiInputMethod::ThaiInputMethod(QObject *parent) :
    ThaiInputMethodBase(parent)
{
}

#ifndef QT_HUNSPELLINPUTMETHOD_LIB
QList<QVirtualKeyboardInputEngine::InputMode> ThaiInputMethod::inputModes(
        const QString &locale)
{
    Q_UNUSED(locale)
    return QList<QVirtualKeyboardInputEngine::InputMode>() << QVirtualKeyboardInputEngine::InputMode::Latin;
}

bool ThaiInputMethod::setInputMode(const QString &locale,
                                         QVirtualKeyboardInputEngine::InputMode inputMode)
{
    Q_UNUSED(locale)
    Q_UNUSED(inputMode)
    return true;
}

bool ThaiInputMethod::setTextCase(QVirtualKeyboardInputEngine::TextCase textCase)
{
    Q_UNUSED(textCase)
    return true;
}
#endif

bool ThaiInputMethod::keyEvent(Qt::Key key,
                               const QString &text,
                               Qt::KeyboardModifiers modifiers)
{
    const bool isMark = text.length() == 2 && text.at(0) == QLatin1Char(' ');
#ifdef QT_HUNSPELLINPUTMETHOD_LIB
    if (isMark) {
        const QString mark(text.right(1));
        return ThaiInputMethodBase::keyEvent(static_cast<Qt::Key>(mark.at(0).unicode()),
                                             mark, modifiers);
    }
    return ThaiInputMethodBase::keyEvent(key, text, modifiers);
#else
    Q_UNUSED(key)
    if (isMark) {
        const QString mark(text.right(1));
        inputContext()->sendKeyClick(static_cast<Qt::Key>(mark.at(0).unicode()), mark, modifiers);
        return true;
    }
    return false;
#endif
}

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE
