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

#ifndef THAIINPUTMETHOD_H
#define THAIINPUTMETHOD_H

#ifdef QT_HUNSPELLINPUTMETHOD_LIB
#include <QtHunspellInputMethod/private/hunspellinputmethod_p.h>
#define ThaiInputMethodBase HunspellInputMethod
#else
#include <QtVirtualKeyboard/qvirtualkeyboardabstractinputmethod.h>
#define ThaiInputMethodBase QVirtualKeyboardAbstractInputMethod
#endif

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

class ThaiInputMethodPrivate;

class ThaiInputMethod : public ThaiInputMethodBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ThaiInputMethod)
public:
    explicit ThaiInputMethod(QObject *parent = nullptr);

#ifndef QT_HUNSPELLINPUTMETHOD_LIB
    QList<QVirtualKeyboardInputEngine::InputMode> inputModes(const QString &locale);
    bool setInputMode(const QString &locale, QVirtualKeyboardInputEngine::InputMode inputMode);
    bool setTextCase(QVirtualKeyboardInputEngine::TextCase textCase);
#endif

    bool keyEvent(Qt::Key key, const QString &text, Qt::KeyboardModifiers modifiers);
};

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE

#endif
