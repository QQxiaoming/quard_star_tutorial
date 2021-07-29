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

#include <QtVirtualKeyboard/qvirtualkeyboardinputcontext.h>
#include <QtVirtualKeyboard/private/qvirtualkeyboardinputcontext_p.h>
#include <QtVirtualKeyboard/private/shifthandler_p.h>
#include <QtVirtualKeyboard/private/platforminputcontext_p.h>
#include <QtVirtualKeyboard/private/virtualkeyboarddebug_p.h>

#include <QTextFormat>
#include <QGuiApplication>

QT_BEGIN_NAMESPACE
using namespace QtVirtualKeyboard;

/*!
    \qmltype InputContext
    \instantiates QVirtualKeyboardInputContext
    \inqmlmodule QtQuick.VirtualKeyboard
    \ingroup qtvirtualkeyboard-qml
    \brief Provides access to an input context.

    The InputContext can be accessed as singleton instance.
*/

/*!
    \class QVirtualKeyboardInputContext
    \inmodule QtVirtualKeyboard
    \brief Provides access to an input context.
*/

/*!
    \internal
    Constructs an input context with \a parent as the platform input
    context.
*/
QVirtualKeyboardInputContext::QVirtualKeyboardInputContext(QObject *parent) :
    QObject(parent),
    d_ptr(new QVirtualKeyboardInputContextPrivate(this))
{
    Q_D(QVirtualKeyboardInputContext);
    d->init();
    QObject::connect(d->_shiftHandler, &ShiftHandler::shiftActiveChanged, this, &QVirtualKeyboardInputContext::shiftActiveChanged);
    QObject::connect(d->_shiftHandler, &ShiftHandler::capsLockActiveChanged, this, &QVirtualKeyboardInputContext::capsLockActiveChanged);
    QObject::connect(d->_shiftHandler, &ShiftHandler::uppercaseChanged, this, &QVirtualKeyboardInputContext::uppercaseChanged);
    QObject::connect(d, &QVirtualKeyboardInputContextPrivate::localeChanged, this, &QVirtualKeyboardInputContext::localeChanged);
    QObject::connect(d, &QVirtualKeyboardInputContextPrivate::inputItemChanged, this, &QVirtualKeyboardInputContext::inputItemChanged);
}

/*!
    \internal
    Destroys the input context and frees all allocated resources.
*/
QVirtualKeyboardInputContext::~QVirtualKeyboardInputContext()
{
}

bool QVirtualKeyboardInputContext::isShiftActive() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->_shiftHandler->isShiftActive();
}

bool QVirtualKeyboardInputContext::isCapsLockActive() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->_shiftHandler->isCapsLockActive();
}

bool QVirtualKeyboardInputContext::isUppercase() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->_shiftHandler->isUppercase();
}

int QVirtualKeyboardInputContext::anchorPosition() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->anchorPosition;
}

int QVirtualKeyboardInputContext::cursorPosition() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->cursorPosition;
}

Qt::InputMethodHints QVirtualKeyboardInputContext::inputMethodHints() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->inputMethodHints;
}

QString QVirtualKeyboardInputContext::preeditText() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->preeditText;
}

void QVirtualKeyboardInputContext::setPreeditText(const QString &text, QList<QInputMethodEvent::Attribute> attributes, int replaceFrom, int replaceLength)
{
    Q_D(QVirtualKeyboardInputContext);
    // Add default attributes
    if (!text.isEmpty()) {
        if (!d->testAttribute(attributes, QInputMethodEvent::TextFormat)) {
            QTextCharFormat textFormat;
            textFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, text.length(), textFormat));
        }
    } else if (d->_forceCursorPosition != -1) {
        d->addSelectionAttribute(attributes);
    }

    d->sendPreedit(text, attributes, replaceFrom, replaceLength);
}

QList<QInputMethodEvent::Attribute> QVirtualKeyboardInputContext::preeditTextAttributes() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->preeditTextAttributes;
}

QString QVirtualKeyboardInputContext::surroundingText() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->surroundingText;
}

QString QVirtualKeyboardInputContext::selectedText() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->selectedText;
}

QRectF QVirtualKeyboardInputContext::anchorRectangle() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->anchorRectangle;
}

QRectF QVirtualKeyboardInputContext::cursorRectangle() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->cursorRectangle;
}

bool QVirtualKeyboardInputContext::isAnimating() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->animating;
}

void QVirtualKeyboardInputContext::setAnimating(bool animating)
{
    Q_D(QVirtualKeyboardInputContext);
    if (d->animating != animating) {
        VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputContext::setAnimating():" << animating;
        d->animating = animating;
        emit animatingChanged();
        d->platformInputContext->emitAnimatingChanged();
    }
}

QString QVirtualKeyboardInputContext::locale() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->locale();
}

QObject *QVirtualKeyboardInputContext::inputItem() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->inputItem();
}

QVirtualKeyboardInputEngine *QVirtualKeyboardInputContext::inputEngine() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->inputEngine;
}

/*!
    \qmlmethod void InputContext::sendKeyClick(int key, string text, int modifiers = 0)

    Sends a key click event with the given \a key, \a text and \a modifiers to
    the input item that currently has focus.
*/
/*!
    Sends a key click event with the given \a key, \a text and \a modifiers to
    the input item that currently has focus.
*/
void QVirtualKeyboardInputContext::sendKeyClick(int key, const QString &text, int modifiers)
{
    Q_D(QVirtualKeyboardInputContext);
    if (d->_focus && d->platformInputContext) {
        QKeyEvent pressEvent(QEvent::KeyPress, key, Qt::KeyboardModifiers(modifiers), text);
        QKeyEvent releaseEvent(QEvent::KeyRelease, key, Qt::KeyboardModifiers(modifiers), text);
        VIRTUALKEYBOARD_DEBUG().nospace() << "InputContext::sendKeyClick()"
#ifdef SENSITIVE_DEBUG
            << ": " << key
#endif
        ;


        d->setState(QVirtualKeyboardInputContextPrivate::State::KeyEvent);
        d->platformInputContext->sendKeyEvent(&pressEvent);
        d->platformInputContext->sendKeyEvent(&releaseEvent);
        if (d->activeKeys.isEmpty())
            d->clearState(QVirtualKeyboardInputContextPrivate::State::KeyEvent);
    } else {
        VIRTUALKEYBOARD_WARN() << "InputContext::sendKeyClick(): no focus to send key click"
#ifdef SENSITIVE_DEBUG
            << key << text
#endif
            << "- QGuiApplication::focusWindow() is:" << QGuiApplication::focusWindow();
    }
}

/*!
    \qmlmethod void InputContext::commit()

    Commits the current pre-edit text.
*/
/*!
    \fn void QVirtualKeyboardInputContext::commit()

    Commits the current pre-edit text.
*/
void QVirtualKeyboardInputContext::commit()
{
    Q_D(QVirtualKeyboardInputContext);
    QString text = d->preeditText;
    commit(text);
}

/*!
    \qmlmethod void InputContext::commit(string text, int replaceFrom = 0, int replaceLength = 0)

    Commits the final \a text to the input item and optionally
    modifies the text relative to the start of the pre-edit text.
    If \a replaceFrom is non-zero, the \a text replaces the
    contents relative to \a replaceFrom with a length of
    \a replaceLength.
*/
/*!
    Commits the final \a text to the input item and optionally
    modifies the text relative to the start of the pre-edit text.
    If \a replaceFrom is non-zero, the \a text replaces the
    contents relative to \a replaceFrom with a length of
    \a replaceLength.
*/
void QVirtualKeyboardInputContext::commit(const QString &text, int replaceFrom, int replaceLength)
{
    Q_D(QVirtualKeyboardInputContext);

    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputContext::commit()"
#ifdef SENSITIVE_DEBUG
           << text << replaceFrom << replaceLength
#endif
        ;
    bool preeditChanged = !d->preeditText.isEmpty();

    if (d->platformInputContext) {
        QList<QInputMethodEvent::Attribute> attributes;
        d->addSelectionAttribute(attributes);
        d->preeditText.clear();
        d->preeditTextAttributes.clear();
        QInputMethodEvent inputEvent(QString(), attributes);
        inputEvent.setCommitString(text, replaceFrom, replaceLength);
        d->sendInputMethodEvent(&inputEvent);
    } else {
        d->preeditText.clear();
        d->preeditTextAttributes.clear();
    }

    if (preeditChanged)
        emit preeditTextChanged();
}

/*!
    \qmlmethod void InputContext::clear()

    Clears the pre-edit text.
*/
/*!
    \fn void QVirtualKeyboardInputContext::clear()

    Clears the pre-edit text.
*/
void QVirtualKeyboardInputContext::clear()
{
    Q_D(QVirtualKeyboardInputContext);
    bool preeditChanged = !d->preeditText.isEmpty();
    d->preeditText.clear();
    d->preeditTextAttributes.clear();

    if (d->platformInputContext) {
        QList<QInputMethodEvent::Attribute> attributes;
        d->addSelectionAttribute(attributes);
        QInputMethodEvent event(QString(), attributes);
        d->sendInputMethodEvent(&event);
    }

    if (preeditChanged)
        emit preeditTextChanged();
}

/*!
    \internal
*/
void QVirtualKeyboardInputContext::setSelectionOnFocusObject(const QPointF &anchorPos, const QPointF &cursorPos)
{
    QPlatformInputContext::setSelectionOnFocusObject(anchorPos, cursorPos);
}

/*!
    \property QVirtualKeyboardInputContext::anchorRectIntersectsClipRect
    \brief Holds \c true if the bounding rectangle of the selection anchor
    intersects the exposed input item rectangle.

    \sa Qt::ImAnchorRectangle, Qt::ImInputItemClipRectangle
*/
/*!
    \qmlproperty bool InputContext::anchorRectIntersectsClipRect
    \readonly
    \brief Holds \c true if the bounding rectangle of the selection anchor
    intersects the exposed input item rectangle.

    \sa Qt::ImAnchorRectangle, Qt::ImInputItemClipRectangle
*/
bool QVirtualKeyboardInputContext::anchorRectIntersectsClipRect() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->anchorRectIntersectsClipRect;
}

/*!
    \property QVirtualKeyboardInputContext::cursorRectIntersectsClipRect
    \brief Holds \c true if the bounding rectangle of the input cursor
    intersects the exposed input item rectangle.

    \sa Qt::ImCursorRectangle, Qt::ImInputItemClipRectangle
*/
/*!
    \qmlproperty bool InputContext::cursorRectIntersectsClipRect
    \readonly
    \brief Holds \c true if the bounding rectangle of the input cursor
    intersects the exposed input item rectangle.

    \sa Qt::ImCursorRectangle, Qt::ImInputItemClipRectangle
*/
bool QVirtualKeyboardInputContext::cursorRectIntersectsClipRect() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->cursorRectIntersectsClipRect;
}

/*!
    \property QVirtualKeyboardInputContext::selectionControlVisible
    \brief Holds \c true if the selection control is currently visible.
*/
/*!
    \qmlproperty bool InputContext::selectionControlVisible
    \readonly
    \brief Holds \c true if the selection control is currently visible.
*/
bool QVirtualKeyboardInputContext::isSelectionControlVisible() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return d->selectionControlVisible;
}

/*!
    \internal
*/
QVirtualKeyboardInputContextPrivate *QVirtualKeyboardInputContext::priv() const
{
    Q_D(const QVirtualKeyboardInputContext);
    return const_cast<QVirtualKeyboardInputContextPrivate *>(d);
}

/*!
    \qmlproperty bool InputContext::shift
    \deprecated

    Use \l shiftActive instead.

    This property is changed when the shift status changes.
*/

/*!
    \property QVirtualKeyboardInputContext::shift
    \brief the shift status.
    \deprecated

    Use \l shiftActive instead.

    This property is changed when the shift status changes.
*/

/*!
    \qmlproperty bool InputContext::shiftActive
    \since QtQuick.VirtualKeyboard 2.4

    This property is changed when the shift status changes.
*/

/*!
    \property QVirtualKeyboardInputContext::shiftActive
    \brief the shift status.

    This property is changed when the shift status changes.
*/

/*!
    \qmlproperty bool InputContext::capsLock
    \deprecated

    Use \l capsLockActive instead.

    This property is changed when the caps lock status changes.
*/

/*!
    \property QVirtualKeyboardInputContext::capsLock
    \brief the caps lock status.
    \deprecated

    Use \l capsLockActive instead.

    This property is changed when the caps lock status changes.
*/

/*!
    \qmlproperty bool InputContext::capsLockActive
    \since QtQuick.VirtualKeyboard 2.4

    This property is changed when the caps lock status changes.
*/

/*!
    \property QVirtualKeyboardInputContext::capsLockActive
    \brief the caps lock status.

    This property is changed when the caps lock status changes.
*/

/*!
    \qmlproperty bool InputContext::uppercase
    \since QtQuick.VirtualKeyboard 2.2

    This property is \c true when either \l shiftActive or \l capsLockActive is \c true.
*/

/*!
    \property QVirtualKeyboardInputContext::uppercase
    \brief the uppercase status.

    This property is \c true when either \l shiftActive or \l capsLockActive is \c true.
*/

/*!
    \qmlproperty int InputContext::anchorPosition
    \since QtQuick.VirtualKeyboard 2.2

    This property is changed when the anchor position changes.
*/

/*!
    \property QVirtualKeyboardInputContext::anchorPosition
    \brief the anchor position.

    This property is changed when the anchor position changes.
*/

/*!
    \qmlproperty int InputContext::cursorPosition

    This property is changed when the cursor position changes.
*/

/*!
    \property QVirtualKeyboardInputContext::cursorPosition
    \brief the cursor position.

    This property is changed when the cursor position changes.
*/

/*!
    \qmlproperty int InputContext::inputMethodHints

    This property is changed when the input method hints changes.
*/

/*!
    \property QVirtualKeyboardInputContext::inputMethodHints
    \brief the input method hints.

    This property is changed when the input method hints changes.
*/

/*!
    \qmlproperty string InputContext::preeditText

    This property sets the pre-edit text.
*/

/*!
    \property QVirtualKeyboardInputContext::preeditText
    \brief the pre-edit text.

    This property sets the pre-edit text.
*/

/*!
    \qmlproperty string InputContext::surroundingText

    This property is changed when the surrounding text around the cursor changes.
*/

/*!
    \property QVirtualKeyboardInputContext::surroundingText
    \brief the surrounding text around cursor.

    This property is changed when the surrounding text around the cursor changes.
*/

/*!
    \qmlproperty string InputContext::selectedText

    This property is changed when the selected text changes.
*/

/*!
    \property QVirtualKeyboardInputContext::selectedText
    \brief the selected text.

    This property is changed when the selected text changes.
*/

/*!
    \qmlproperty rect InputContext::anchorRectangle
    \since QtQuick.VirtualKeyboard 2.1

    This property is changed when the anchor rectangle changes.
*/

/*!
    \property QVirtualKeyboardInputContext::anchorRectangle
    \brief the anchor rectangle.

    This property is changed when the anchor rectangle changes.
*/

/*!
    \qmlproperty rect InputContext::cursorRectangle

    This property is changed when the cursor rectangle changes.
*/

/*!
    \property QVirtualKeyboardInputContext::cursorRectangle
    \brief the cursor rectangle.

    This property is changed when the cursor rectangle changes.
*/

/*!
    \qmlproperty bool InputContext::animating

    Use this property to set the animating status, for example
    during UI transitioning states.
*/

/*!
    \property QVirtualKeyboardInputContext::animating
    \brief the animating status.

    Use this property to set the animating status, for example
    during UI transitioning states.
*/

/*!
    \qmlproperty string InputContext::locale

    This property is changed when the input locale changes.
*/

/*!
    \property QVirtualKeyboardInputContext::locale
    \brief the locale.

    This property is changed when the input locale changes.
*/

/*!
    \qmlproperty QtObject InputContext::inputItem
    \deprecated

    This property is changed when the focused input item changes.
*/

/*!
    \property QVirtualKeyboardInputContext::inputItem
    \brief the focused input item.
    \deprecated

    This property is changed when the focused input item changes.
*/

/*!
    \qmlproperty InputEngine InputContext::inputEngine

    This property stores the input engine.
*/

/*!
    \property QVirtualKeyboardInputContext::inputEngine
    \brief the input engine.

    This property stores the input engine.
*/

/*!
    \property QVirtualKeyboardInputContext::priv
    \internal
*/
QT_END_NAMESPACE
