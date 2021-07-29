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

#include <QtVirtualKeyboard/private/shifthandler_p.h>
#include <QtVirtualKeyboard/private/qvirtualkeyboardinputcontext_p.h>
#include <QtVirtualKeyboard/qvirtualkeyboardinputcontext.h>
#include <QtVirtualKeyboard/qvirtualkeyboardinputengine.h>
#include <QtCore/private/qobject_p.h>
#include <QSet>
#include <QGuiApplication>
#include <QTime>
#include <QStyleHints>

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

class ShiftHandlerPrivate : public QObjectPrivate
{
public:
    ShiftHandlerPrivate() :
        QObjectPrivate(),
        inputContext(nullptr),
        sentenceEndingCharacters(QLatin1String(".!?") + QChar(Qt::Key_exclamdown) + QChar(Qt::Key_questiondown)),
        autoCapitalizationEnabled(false),
        toggleShiftEnabled(false),
        shift(false),
        shiftChanged(false),
        capsLock(false),
        resetWhenVisible(false),
        manualShiftLanguageFilter(QSet<QLocale::Language>() << QLocale::Arabic << QLocale::Persian << QLocale::Hindi << QLocale::Korean << QLocale::Thai),
        manualCapsInputModeFilter(QSet<QVirtualKeyboardInputEngine::InputMode>() << QVirtualKeyboardInputEngine::InputMode::Cangjie << QVirtualKeyboardInputEngine::InputMode::Zhuyin << QVirtualKeyboardInputEngine::InputMode::Hebrew),
        noAutoUppercaseInputModeFilter(QSet<QVirtualKeyboardInputEngine::InputMode>() << QVirtualKeyboardInputEngine::InputMode::FullwidthLatin << QVirtualKeyboardInputEngine::InputMode::Pinyin << QVirtualKeyboardInputEngine::InputMode::Cangjie << QVirtualKeyboardInputEngine::InputMode::Zhuyin << QVirtualKeyboardInputEngine::InputMode::ChineseHandwriting << QVirtualKeyboardInputEngine::InputMode::JapaneseHandwriting << QVirtualKeyboardInputEngine::InputMode::KoreanHandwriting),
        allCapsInputModeFilter(QSet<QVirtualKeyboardInputEngine::InputMode>() << QVirtualKeyboardInputEngine::InputMode::Hiragana << QVirtualKeyboardInputEngine::InputMode::Katakana)
    {
    }

    QVirtualKeyboardInputContext *inputContext;
    QString sentenceEndingCharacters;
    bool autoCapitalizationEnabled;
    bool toggleShiftEnabled;
    bool shift;
    bool shiftChanged;
    bool capsLock;
    bool resetWhenVisible;
    QLocale locale;
    QTime timer;
    const QSet<QLocale::Language> manualShiftLanguageFilter;
    const QSet<QVirtualKeyboardInputEngine::InputMode> manualCapsInputModeFilter;
    const QSet<QVirtualKeyboardInputEngine::InputMode> noAutoUppercaseInputModeFilter;
    const QSet<QVirtualKeyboardInputEngine::InputMode> allCapsInputModeFilter;
};

/*!
    \qmltype ShiftHandler
    \inqmlmodule QtQuick.VirtualKeyboard
    \ingroup qtvirtualkeyboard-qml
    \instantiates QtVirtualKeyboard::ShiftHandler
    \brief Manages the shift state.
*/

/*!
    \class QtVirtualKeyboard::ShiftHandler
    \internal
    \inmodule QtVirtualKeyboard
    \brief Manages the shift state.
*/

ShiftHandler::ShiftHandler(QVirtualKeyboardInputContext *parent) :
    QObject(*new ShiftHandlerPrivate(), parent)
{
    Q_D(ShiftHandler);
    d->inputContext = parent;
}

void ShiftHandler::init()
{
    Q_D(ShiftHandler);
    connect(d->inputContext, SIGNAL(inputMethodHintsChanged()), SLOT(restart()));
    connect(d->inputContext->priv(), SIGNAL(inputItemChanged()), SLOT(restart()));
    connect(d->inputContext->inputEngine(), SIGNAL(inputModeChanged()), SLOT(restart()));
    connect(d->inputContext, SIGNAL(preeditTextChanged()), SLOT(autoCapitalize()));
    connect(d->inputContext, SIGNAL(surroundingTextChanged()), SLOT(autoCapitalize()));
    connect(d->inputContext, SIGNAL(cursorPositionChanged()), SLOT(autoCapitalize()));
    connect(d->inputContext, SIGNAL(localeChanged()), SLOT(localeChanged()));
    connect(qGuiApp->inputMethod(), SIGNAL(visibleChanged()), SLOT(inputMethodVisibleChanged()));
    d->locale = QLocale(d->inputContext->locale());
}

/*!
    \internal
*/
ShiftHandler::~ShiftHandler()
{

}

QString ShiftHandler::sentenceEndingCharacters() const
{
    Q_D(const ShiftHandler);
    return d->sentenceEndingCharacters;
}

void ShiftHandler::setSentenceEndingCharacters(const QString &value)
{
    Q_D(ShiftHandler);
    if (d->sentenceEndingCharacters != value) {
        d->sentenceEndingCharacters = value;
        autoCapitalize();
        emit sentenceEndingCharactersChanged();
    }
}

bool ShiftHandler::isAutoCapitalizationEnabled() const
{
    Q_D(const ShiftHandler);
    return d->autoCapitalizationEnabled;
}

bool ShiftHandler::isToggleShiftEnabled() const
{
    Q_D(const ShiftHandler);
    return d->toggleShiftEnabled;
}

bool ShiftHandler::isShiftActive() const
{
    Q_D(const ShiftHandler);
    return d->shift;
}

void ShiftHandler::setShiftActive(bool active)
{
    Q_D(ShiftHandler);
    if (d->shift != active) {
        d->shift = active;
        d->shiftChanged = true;
        emit shiftActiveChanged();
        if (!d->capsLock)
            emit uppercaseChanged();
    }
}

bool ShiftHandler::isCapsLockActive() const
{
    Q_D(const ShiftHandler);
    return d->capsLock;
}

void ShiftHandler::setCapsLockActive(bool active)
{
    Q_D(ShiftHandler);
    if (d->capsLock != active) {
        d->capsLock = active;
        emit capsLockActiveChanged();
        if (!d->shift)
            emit uppercaseChanged();
    }
}

bool ShiftHandler::isUppercase() const
{
    Q_D(const ShiftHandler);
    return d->shift || d->capsLock;
}

/*!
    \since 1.2

    \qmlmethod void ShiftHandler::toggleShift()

    Toggles the current shift state.

    This method provides the functionality of the shift key.

    \sa toggleShiftEnabled
*/
/*!
    \since 1.2

    \fn void QtVirtualKeyboard::ShiftHandler::toggleShift()

    Toggles the current shift state.

    This method provides the functionality of the shift key.

    \sa toggleShiftEnabled
*/
void ShiftHandler::toggleShift()
{
    Q_D(ShiftHandler);
    if (!d->toggleShiftEnabled)
        return;
    if (d->manualShiftLanguageFilter.contains(d->locale.language())) {
        setCapsLockActive(false);
        setShiftActive(!d->shift);
    } else if (d->inputContext->inputMethodHints() & Qt::ImhNoAutoUppercase ||
               d->manualCapsInputModeFilter.contains(d->inputContext->inputEngine()->inputMode())) {
        bool capsLock = d->capsLock;
        setCapsLockActive(!capsLock);
        setShiftActive(!capsLock);
    } else {
        if (d->capsLock) {
            setCapsLockActive(!d->capsLock && d->shift && !d->shiftChanged);
        }

        QStyleHints *style = QGuiApplication::styleHints();

        if (d->timer.isNull() || d->timer.elapsed() > style->mouseDoubleClickInterval()) {
            d->timer.restart();
        } else if (d->timer.elapsed() < style->mouseDoubleClickInterval() && !d->capsLock) {
            setCapsLockActive(!d->capsLock && d->shift && !d->shiftChanged);
        }

        setShiftActive(d->capsLock || !d->shift);
        d->shiftChanged = false;
    }
}

/*! Clears the toggle shift timer.
    \internal

*/
void ShiftHandler::clearToggleShiftTimer()
{
    Q_D(ShiftHandler);
    d->timer = QTime();
}

void ShiftHandler::reset()
{
    Q_D(ShiftHandler);
    if (d->inputContext->priv()->inputItem()) {
        Qt::InputMethodHints inputMethodHints = d->inputContext->inputMethodHints();
        QVirtualKeyboardInputEngine::InputMode inputMode = d->inputContext->inputEngine()->inputMode();
        bool preferUpperCase = (inputMethodHints & (Qt::ImhPreferUppercase | Qt::ImhUppercaseOnly));
        bool autoCapitalizationEnabled = !(d->inputContext->inputMethodHints() & (Qt::ImhNoAutoUppercase |
              Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly | Qt::ImhEmailCharactersOnly |
              Qt::ImhUrlCharactersOnly | Qt::ImhDialableCharactersOnly | Qt::ImhFormattedNumbersOnly |
              Qt::ImhDigitsOnly)) && !d->noAutoUppercaseInputModeFilter.contains(inputMode);
        bool toggleShiftEnabled = !(inputMethodHints & (Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly));
        // For filtered languages reset the initial shift status to lower case
        // and allow manual shift change
        if (d->manualShiftLanguageFilter.contains(d->locale.language()) ||
                d->manualCapsInputModeFilter.contains(inputMode)) {
            preferUpperCase = false;
            autoCapitalizationEnabled = false;
            toggleShiftEnabled = true;
        } else if (d->allCapsInputModeFilter.contains(inputMode)) {
            preferUpperCase = true;
            autoCapitalizationEnabled = false;
            toggleShiftEnabled = false;
        }
        setToggleShiftEnabled(toggleShiftEnabled);
        setAutoCapitalizationEnabled(autoCapitalizationEnabled);
        setCapsLockActive(preferUpperCase);
        if (preferUpperCase)
            setShiftActive(preferUpperCase);
        else
            autoCapitalize();
    }
}

void ShiftHandler::autoCapitalize()
{
    Q_D(ShiftHandler);
    if (d->capsLock)
        return;
    if (!d->autoCapitalizationEnabled || !d->inputContext->preeditText().isEmpty()) {
        setShiftActive(false);
    } else {
        int cursorPosition = d->inputContext->cursorPosition();
        bool preferLowerCase = d->inputContext->inputMethodHints() & Qt::ImhPreferLowercase;
        if (cursorPosition == 0) {
            setShiftActive(!preferLowerCase);
        } else {
            QString text = d->inputContext->surroundingText();
            text.truncate(cursorPosition);
            text = text.trimmed();
            if (text.length() == 0)
                setShiftActive(!preferLowerCase);
            else if (text.length() > 0 && d->sentenceEndingCharacters.indexOf(text[text.length() - 1]) >= 0)
                setShiftActive(!preferLowerCase);
            else
                setShiftActive(false);
        }
    }
}

void ShiftHandler::restart()
{
    Q_D(ShiftHandler);
    const QGuiApplication *app = qGuiApp;
    if (!app || !app->inputMethod()->isVisible()) {
        d->resetWhenVisible = true;
        return;
    }
    reset();
}

void ShiftHandler::localeChanged()
{
    Q_D(ShiftHandler);
    d->locale = QLocale(d->inputContext->locale());
    restart();
}

void ShiftHandler::inputMethodVisibleChanged()
{
    Q_D(ShiftHandler);
    if (!d->resetWhenVisible)
        return;

    const QGuiApplication *app = qGuiApp;
    if (app && app->inputMethod()->isVisible()) {
        d->resetWhenVisible = false;
        reset();
    }
}

void ShiftHandler::setAutoCapitalizationEnabled(bool enabled)
{
    Q_D(ShiftHandler);
    if (d->autoCapitalizationEnabled != enabled) {
        d->autoCapitalizationEnabled = enabled;
        emit autoCapitalizationEnabledChanged();
    }
}

void ShiftHandler::setToggleShiftEnabled(bool enabled)
{
    Q_D(ShiftHandler);
    if (d->toggleShiftEnabled != enabled) {
        d->toggleShiftEnabled = enabled;
        emit toggleShiftEnabledChanged();
    }
}

/*!
    \property QtVirtualKeyboard::ShiftHandler::sentenceEndingCharacters

    This property specifies the sentence ending characters which
    will cause shift state change.

    By default, the property is initialized to sentence
    ending characters found in the ASCII range (i.e. ".!?").
*/

/*!
    \qmlproperty string ShiftHandler::sentenceEndingCharacters

    This property specifies the sentence ending characters which
    will cause shift state change.

    By default, the property is initialized to sentence
    ending characters found in the ASCII range (i.e. ".!?").
*/

/*!
    \since 1.2

    \property QtVirtualKeyboard::ShiftHandler::autoCapitalizationEnabled

    This property provides the current state of the automatic
    capitalization feature.
*/

/*!
    \since 1.2

    \qmlproperty bool ShiftHandler::autoCapitalizationEnabled

    This property provides the current state of the automatic
    capitalization feature.
*/

/*!
    \since 1.2

    \property QtVirtualKeyboard::ShiftHandler::toggleShiftEnabled

    This property provides the current state of the toggleShift()
    method. When true, the current shift state can be changed by
    calling the toggleShift() method.
*/

/*!
    \since 1.2

    \qmlproperty bool ShiftHandler::toggleShiftEnabled

    This property provides the current state of the toggleShift()
    method. When true, the current shift state can be changed by
    calling the toggleShift() method.
*/

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE
