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

#include <QtVirtualKeyboard/qvirtualkeyboardabstractinputmethod.h>
#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QVirtualKeyboardAbstractInputMethodPrivate : public QObjectPrivate
{
public:
    QVirtualKeyboardAbstractInputMethodPrivate();

    QVirtualKeyboardInputEngine *inputEngine;
};

/*!
    \class AbstractInputMethodPrivate
    \internal
*/

QVirtualKeyboardAbstractInputMethodPrivate::QVirtualKeyboardAbstractInputMethodPrivate() :
    inputEngine(nullptr)
{
}

/*!
    \class QVirtualKeyboardAbstractInputMethod

    \inmodule QtVirtualKeyboard

    \brief The base class for input methods.

    Use this class if you want to implement a custom input
    method using C/C++ language.
*/

/*!
    Constructs an input method with \a parent.
*/
QVirtualKeyboardAbstractInputMethod::QVirtualKeyboardAbstractInputMethod(QObject *parent) :
    QObject(*new QVirtualKeyboardAbstractInputMethodPrivate(), parent)
{
}

/*!
    Destroys the input method and frees all allocated resources.
*/
QVirtualKeyboardAbstractInputMethod::~QVirtualKeyboardAbstractInputMethod()
{
}

/*!
    Returns the input context associated with the input method.
    This method returns \c NULL if the input method is not active.
*/
QVirtualKeyboardInputContext *QVirtualKeyboardAbstractInputMethod::inputContext() const
{
    Q_D(const QVirtualKeyboardAbstractInputMethod);
    return d->inputEngine ? d->inputEngine->inputContext() : nullptr;
}

/*!
    Returns the input engine associated with the input method.
    This method returns \c NULL if the input method is not active.
*/
QVirtualKeyboardInputEngine *QVirtualKeyboardAbstractInputMethod::inputEngine() const
{
    Q_D(const QVirtualKeyboardAbstractInputMethod);
    return d->inputEngine;
}

/*!
    This method is called by the input engine when the input method needs
    to be reset. The input method must reset its internal state only. The main
    difference to the update() method is that reset() modifies only
    the input method state, i.e. it must not modify the input context.
*/
void QVirtualKeyboardAbstractInputMethod::reset()
{
}

/*!
    This method is called by the input engine when the input method needs to be
    updated. The input method must close the current pre-edit text and
    restore its internal state to the default.
*/
void QVirtualKeyboardAbstractInputMethod::update()
{
}

/*!
    \internal
    Called by the input engine when the input method is activated and
    deactivated.
*/
void QVirtualKeyboardAbstractInputMethod::setInputEngine(QVirtualKeyboardInputEngine *inputEngine)
{
    Q_D(QVirtualKeyboardAbstractInputMethod);
    if (d->inputEngine) {
        d->inputEngine->disconnect(this, SLOT(reset()));
        d->inputEngine->disconnect(this, SLOT(update()));
    }
    d->inputEngine = inputEngine;
    if (d->inputEngine) {
        connect(d->inputEngine, SIGNAL(inputMethodReset()), SLOT(reset()));
        connect(d->inputEngine, SIGNAL(inputMethodUpdate()), SLOT(update()));
    }
}

QList<QVirtualKeyboardSelectionListModel::Type> QVirtualKeyboardAbstractInputMethod::selectionLists()
{
    return QList<QVirtualKeyboardSelectionListModel::Type>();
}

int QVirtualKeyboardAbstractInputMethod::selectionListItemCount(QVirtualKeyboardSelectionListModel::Type type)
{
    Q_UNUSED(type)
    return 0;
}

QVariant QVirtualKeyboardAbstractInputMethod::selectionListData(QVirtualKeyboardSelectionListModel::Type type, int index, QVirtualKeyboardSelectionListModel::Role role)
{
    Q_UNUSED(type)
    Q_UNUSED(index)
    switch (role) {
    case QVirtualKeyboardSelectionListModel::Role::Display:
        return QVariant(QString());
    case QVirtualKeyboardSelectionListModel::Role::WordCompletionLength:
        return QVariant(0);
    case QVirtualKeyboardSelectionListModel::Role::Dictionary:
        return QVariant(static_cast<int>(QVirtualKeyboardSelectionListModel::DictionaryType::Default));
    case QVirtualKeyboardSelectionListModel::Role::CanRemoveSuggestion:
        return QVariant(false);
    }
    return QVariant();
}

void QVirtualKeyboardAbstractInputMethod::selectionListItemSelected(QVirtualKeyboardSelectionListModel::Type type, int index)
{
    Q_UNUSED(type)
    Q_UNUSED(index)
}

bool QVirtualKeyboardAbstractInputMethod::selectionListRemoveItem(QVirtualKeyboardSelectionListModel::Type type, int index)
{
    Q_UNUSED(type)
    Q_UNUSED(index)
    return false;
}

/*!
    \since QtQuick.VirtualKeyboard 2.0

    Returns list of supported pattern recognition modes.

    This method is called by the input engine to query the list of
    supported pattern recognition modes.
*/
QList<QVirtualKeyboardInputEngine::PatternRecognitionMode> QVirtualKeyboardAbstractInputMethod::patternRecognitionModes() const
{
    return QList<QVirtualKeyboardInputEngine::PatternRecognitionMode>();
}

/*!
    \since QtQuick.VirtualKeyboard 2.0

    This method is called when a trace interaction starts with the specified \a patternRecognitionMode.
    The trace is uniquely identified by the \a traceId.
    The \a traceCaptureDeviceInfo provides information about the source device and the
    \a traceScreenInfo provides information about the screen context.

    If the input method accepts the event and wants to capture the trace input, it must return
    a new QVirtualKeyboardTrace object. This object must remain valid until the traceEnd() method is called. If the
    QVirtualKeyboardTrace is rendered on screen, it remains there until the QVirtualKeyboardTrace object is destroyed.
*/
QVirtualKeyboardTrace *QVirtualKeyboardAbstractInputMethod::traceBegin(
        int traceId, QVirtualKeyboardInputEngine::PatternRecognitionMode patternRecognitionMode,
        const QVariantMap &traceCaptureDeviceInfo, const QVariantMap &traceScreenInfo)
{
    Q_UNUSED(traceId)
    Q_UNUSED(patternRecognitionMode)
    Q_UNUSED(traceCaptureDeviceInfo)
    Q_UNUSED(traceScreenInfo)
    return nullptr;
}

/*!
    \since QtQuick.VirtualKeyboard 2.0

    This method is called when the trace interaction ends. The input method should destroy the \a trace object
    at some point after this function is called. See the \l {Trace API for Input Methods} how to access the gathered
    data.

    The method returns \c true if the trace interaction is accepted.
*/
bool QVirtualKeyboardAbstractInputMethod::traceEnd(QVirtualKeyboardTrace *trace)
{
    Q_UNUSED(trace)
    return false;
}

/*!
    \since QtQuick.VirtualKeyboard 2.0

    This function attempts to reselect a word located at the \a cursorPosition.
    The \a reselectFlags define the rules for how the word should be selected in
    relation to the cursor position.

    The function returns \c true if the word was successfully reselected.
*/
bool QVirtualKeyboardAbstractInputMethod::reselect(int cursorPosition, const QVirtualKeyboardInputEngine::ReselectFlags &reselectFlags)
{
    Q_UNUSED(cursorPosition)
    Q_UNUSED(reselectFlags)
    return false;
}

/*!
    \since QtQuick.VirtualKeyboard 2.4

    Called when the user clicks on pre-edit text at \a cursorPosition.

    The function should return \c true if it handles the event. Otherwise the input
    falls back to \l reselect() for further processing.
*/
bool QVirtualKeyboardAbstractInputMethod::clickPreeditText(int cursorPosition)
{
    Q_UNUSED(cursorPosition)
    return false;
}

/*!
    \fn QList<QVirtualKeyboardInputEngine::InputMode> QVirtualKeyboardAbstractInputMethod::inputModes(const QString& locale)

    Returns the list of input modes for \a locale.
*/

/*!
    \fn bool QVirtualKeyboardAbstractInputMethod::setInputMode(const QString& locale, QVirtualKeyboardInputEngine::InputMode inputMode)

    Sets the \a inputMode and \a locale for this input method. Returns \c true
    if successful.
*/

/*!
    \fn bool QVirtualKeyboardAbstractInputMethod::setTextCase(QVirtualKeyboardInputEngine::TextCase textCase)

    Updates the \a textCase for this input method. The method returns \c true
    if successful.
*/

/*!
    \fn bool QVirtualKeyboardAbstractInputMethod::keyEvent(Qt::Key key, const QString& text, Qt::KeyboardModifiers modifiers)

    The purpose of this method is to handle the key events generated by the the
    input engine.

    The \a key parameter specifies the code of the key to handle. The key code
    does not distinguish between capital and non-capital letters. The \a
    text parameter contains the Unicode text for the key. The \a modifiers
    parameter contains the key modifiers that apply to key.

    This method returns \c true if the key event was successfully handled.
    If the return value is \c false, the key event is redirected to the default
    input method for further processing.
*/

/*!
    \fn QList<QVirtualKeyboardSelectionListModel::Type> QVirtualKeyboardAbstractInputMethod::selectionLists()

    Returns the list of selection lists used by this input method.

    This method is called by input engine when the input method is being
    activated and every time the input method hints are updated. The input method
    can reserve selection lists by returning the desired selection list types.

    The input method may request the input engine to update the selection lists
    at any time by emitting selectionListsChanged() signal. This signal will
    trigger a call to this method, allowing the input method to update the selection
    list types.
*/

/*!
    \fn int QVirtualKeyboardAbstractInputMethod::selectionListItemCount(QVirtualKeyboardSelectionListModel::Type type)

    Returns the number of items in the selection list identified by \a type.
*/

/*!
    \fn QVariant QVirtualKeyboardAbstractInputMethod::selectionListData(QVirtualKeyboardSelectionListModel::Type type, int index, QVirtualKeyboardSelectionListModel::Role role)

    Returns item data for the selection list identified by \a type. The \a role
    parameter specifies which data is requested. The \a index parameter is a
    zero based index into the list.
*/

/*!
    \fn void QVirtualKeyboardAbstractInputMethod::selectionListItemSelected(QVirtualKeyboardSelectionListModel::Type type, int index)

    This method is called when an item at \a index has been selected by the
    user. The selection list is identified by the \a type parameter.
*/

/*!
    \fn bool QVirtualKeyboardAbstractInputMethod::selectionListRemoveItem(QVirtualKeyboardSelectionListModel::Type type, int index)

    This method is called when an item at \a index must be removed from dictionary.
    The selection list is identified by the \a type parameter.
    The function returns \c true if the word was successfully removed.
*/

/*!
    \fn void QVirtualKeyboardAbstractInputMethod::selectionListChanged(QVirtualKeyboardSelectionListModel::Type type)

    The input method emits this signal when the contents of the selection list
    has changed. The \a type parameter specifies which selection list has
    changed.
*/

/*!
    \fn void QVirtualKeyboardAbstractInputMethod::selectionListActiveItemChanged(QVirtualKeyboardSelectionListModel::Type type, int index)

    The input method emits this signal when the current \a index has changed
    in the selection list identified by \a type.
*/

/*!
    \fn void QVirtualKeyboardAbstractInputMethod::selectionListsChanged()
    \since QtQuick.VirtualKeyboard 2.2

    The input method emits this signal when the selection list types have
    changed. This signal will trigger a call to selectionLists() method,
    allowing the input method to update the selection list types.
*/

QT_END_NAMESPACE
