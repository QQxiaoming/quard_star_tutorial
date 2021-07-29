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

#include <QtVirtualKeyboard/qvirtualkeyboardinputengine.h>
#include <QtVirtualKeyboard/qvirtualkeyboardinputcontext.h>
#include <QtVirtualKeyboard/private/qvirtualkeyboardinputcontext_p.h>
#include <QtVirtualKeyboard/private/shifthandler_p.h>
#include <QtVirtualKeyboard/private/fallbackinputmethod_p.h>
#include <QtVirtualKeyboard/qvirtualkeyboardtrace.h>
#include <QtVirtualKeyboard/private/virtualkeyboarddebug_p.h>

#include <QTimerEvent>
#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE
using namespace QtVirtualKeyboard;

class QVirtualKeyboardInputEnginePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QVirtualKeyboardInputEngine)

public:
    QVirtualKeyboardInputEnginePrivate(QVirtualKeyboardInputEngine *q_ptr) :
        QObjectPrivate(),
        q_ptr(q_ptr),
        inputContext(nullptr),
        fallbackInputMethod(nullptr),
        textCase(QVirtualKeyboardInputEngine::TextCase::Lower),
        inputMode(QVirtualKeyboardInputEngine::InputMode::Latin),
        activeKey(Qt::Key_unknown),
        activeKeyModifiers(Qt::NoModifier),
        previousKey(Qt::Key_unknown),
        repeatTimer(0),
        repeatCount(0),
        recursiveMethodLock(0)
    {
    }

    virtual ~QVirtualKeyboardInputEnginePrivate()
    {
    }

    bool virtualKeyClick(Qt::Key key, const QString &text, Qt::KeyboardModifiers modifiers, bool isAutoRepeat)
    {
        Q_Q(QVirtualKeyboardInputEngine);
        bool accept = false;
        if (inputMethod) {
            accept = inputMethod->keyEvent(key, text, modifiers);
            if (!accept) {
                accept = fallbackInputMethod->keyEvent(key, text, modifiers);
            }
            emit q->virtualKeyClicked(key, text, modifiers, isAutoRepeat);
        } else {
            qWarning() << "input method is not set";
        }
        return accept;
    }

    QVirtualKeyboardInputEngine* q_ptr;
    QVirtualKeyboardInputContext *inputContext;
    QPointer<QVirtualKeyboardAbstractInputMethod> inputMethod;
    QVirtualKeyboardAbstractInputMethod *fallbackInputMethod;
    QVirtualKeyboardInputEngine::TextCase textCase;
    QVirtualKeyboardInputEngine::InputMode inputMode;
    QList<int> inputModes;
    QMap<QVirtualKeyboardSelectionListModel::Type, QVirtualKeyboardSelectionListModel *> selectionListModels;
    Qt::Key activeKey;
    QString activeKeyText;
    Qt::KeyboardModifiers activeKeyModifiers;
    Qt::Key previousKey;
    int repeatTimer;
    int repeatCount;
    int recursiveMethodLock;
};

class RecursiveMethodGuard
{
public:
    explicit RecursiveMethodGuard(int &ref) : m_ref(ref)
    {
        m_ref++;
    }
    ~RecursiveMethodGuard()
    {
        m_ref--;
    }
    bool locked() const
    {
        return m_ref > 1;
    }
private:
    int &m_ref;
};

/*!
    \qmltype InputEngine
    \inqmlmodule QtQuick.VirtualKeyboard
    \ingroup qtvirtualkeyboard-qml
    \instantiates QVirtualKeyboardInputEngine
    \brief Maps the user input to the input methods.

    The input engine is responsible for routing input events to input
    methods. The actual input logic is implemented by the input methods.

    The input engine also includes the default input method, which takes
    care of default processing if the active input method does not handle
    the event.
*/

/*!
    \class QVirtualKeyboardInputEngine
    \inmodule QtVirtualKeyboard
    \brief The InputEngine class provides an input engine
    that supports C++ and QML integration.

    The input engine is responsible for routing input events to input
    methods. The actual input logic is implemented by the input methods.

    The input engine also includes the default input method, which takes
    care of default processing if the active input method does not handle
    the event.
*/

/*!
    \internal
    Constructs an input engine with input context as \a parent.
*/
QVirtualKeyboardInputEngine::QVirtualKeyboardInputEngine(QVirtualKeyboardInputContext *parent) :
    QObject(*new QVirtualKeyboardInputEnginePrivate(this), parent)
{
    Q_D(QVirtualKeyboardInputEngine);
    d->inputContext = parent;
}

void QVirtualKeyboardInputEngine::init()
{
    Q_D(QVirtualKeyboardInputEngine);
    ShiftHandler *shiftHandler = d->inputContext->priv()->shiftHandler();
    QObject::connect(shiftHandler, &ShiftHandler::shiftActiveChanged, this, &QVirtualKeyboardInputEngine::shiftChanged);
    QObject::connect(d->inputContext, &QVirtualKeyboardInputContext::localeChanged, this, &QVirtualKeyboardInputEngine::update);
    QObject::connect(d->inputContext, &QVirtualKeyboardInputContext::inputMethodHintsChanged, this, &QVirtualKeyboardInputEngine::updateSelectionListModels);
    QObject::connect(d->inputContext, &QVirtualKeyboardInputContext::localeChanged, this, &QVirtualKeyboardInputEngine::updateInputModes);
    QObject::connect(this, &QVirtualKeyboardInputEngine::inputMethodChanged, this, &QVirtualKeyboardInputEngine::updateInputModes);
    d->fallbackInputMethod = new FallbackInputMethod(this);
    if (d->fallbackInputMethod)
        d->fallbackInputMethod->setInputEngine(this);
    d->selectionListModels[QVirtualKeyboardSelectionListModel::Type::WordCandidateList] = new QVirtualKeyboardSelectionListModel(this);
}

/*!
    \internal
    Destroys the input engine and frees all allocated resources.
*/
QVirtualKeyboardInputEngine::~QVirtualKeyboardInputEngine()
{
}

/*!
    \qmlmethod bool InputEngine::virtualKeyPress(int key, string text, int modifiers, bool repeat)

    Called by the keyboard layer to indicate that \a key was pressed, with
    the given \a text and \a modifiers.

    The \a key is set as an active key (down key). The actual key event is
    triggered when the key is released by the virtualKeyRelease() method. The
    key press event can be discarded by calling virtualKeyCancel().

    The key press also initiates the key repeat timer if \a repeat is \c true.

    Returns \c true if the key was accepted by this input engine.

    \sa virtualKeyCancel(), virtualKeyRelease()
*/
/*!
    Called by the keyboard layer to indicate that \a key was pressed, with
    the given \a text and \a modifiers.

    The \a key is set as an active key (down key). The actual key event is
    triggered when the key is released by the virtualKeyRelease() method. The
    key press event can be discarded by calling virtualKeyCancel().

    The key press also initiates the key repeat timer if \a repeat is \c true.

    Returns \c true if the key was accepted by this input engine.

    \sa virtualKeyCancel(), virtualKeyRelease()
*/
bool QVirtualKeyboardInputEngine::virtualKeyPress(Qt::Key key, const QString &text, Qt::KeyboardModifiers modifiers, bool repeat)
{
    Q_D(QVirtualKeyboardInputEngine);
    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputEngine::virtualKeyPress()"
#ifdef SENSITIVE_DEBUG
           << key << text << modifiers << repeat
#endif
        ;

    bool accept = false;
    if (d->activeKey == Qt::Key_unknown || d->activeKey == key) {
        d->activeKey = key;
        d->activeKeyText = text;
        d->activeKeyModifiers = modifiers;
        if (repeat) {
            d->repeatTimer = startTimer(600);
        }
        accept = true;
        emit activeKeyChanged(d->activeKey);
    } else {
        qWarning("key press ignored; key is already active");
    }
    return accept;
}

/*!
    \qmlmethod void InputEngine::virtualKeyCancel()

    Reverts the active key state without emitting the key event.
    This method is useful when the user discards the current key and
    the key state needs to be restored.
*/
/*!
    \fn void QVirtualKeyboardInputEngine::virtualKeyCancel()

    Reverts the active key state without emitting the key event.
    This method is useful when the user discards the current key and
    the key state needs to be restored.
*/
void QVirtualKeyboardInputEngine::virtualKeyCancel()
{
    Q_D(QVirtualKeyboardInputEngine);
    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputEngine::virtualKeyCancel()";
    if (d->activeKey != Qt::Key_unknown) {
        d->activeKey = Qt::Key_unknown;
        d->activeKeyText = QString();
        d->activeKeyModifiers = Qt::KeyboardModifiers();
        if (d->repeatTimer) {
            killTimer(d->repeatTimer);
            d->repeatTimer = 0;
            d->repeatCount = 0;
        }
        emit activeKeyChanged(d->activeKey);
    }
}

/*!
    \qmlmethod bool InputEngine::virtualKeyRelease(int key, string text, int modifiers)

    Releases the key at \a key. The method emits a key event for the input
    method if the event has not been generated by a repeating timer.
    The \a text and \a modifiers are passed to the input method.

    Returns \c true if the key was accepted by the input engine.
*/
/*!
    Releases the key at \a key. The method emits a key event for the input
    method if the event has not been generated by a repeating timer.
    The \a text and \a modifiers are passed to the input method.

    Returns \c true if the key was accepted by the input engine.
*/
bool QVirtualKeyboardInputEngine::virtualKeyRelease(Qt::Key key, const QString &text, Qt::KeyboardModifiers modifiers)
{
    Q_D(QVirtualKeyboardInputEngine);
    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputEngine::virtualKeyRelease()"
#ifdef SENSITIVE_DEBUG
           << key << text << modifiers
#endif
        ;

    bool accept = false;
    if (d->activeKey == key) {
        if (!d->repeatCount) {
            accept = d->virtualKeyClick(key, text, modifiers, false);
        } else {
            accept = true;
        }
    } else {
        qWarning("key release ignored; key is not pressed");
    }
    if (d->activeKey != Qt::Key_unknown) {
        d->previousKey = d->activeKey;
        emit previousKeyChanged(d->previousKey);
        d->activeKey = Qt::Key_unknown;
        d->activeKeyText = QString();
        d->activeKeyModifiers = Qt::KeyboardModifiers();
        if (d->repeatTimer) {
            killTimer(d->repeatTimer);
            d->repeatTimer = 0;
            d->repeatCount = 0;
        }
        emit activeKeyChanged(d->activeKey);
    }
    return accept;
}

/*!
    \qmlmethod bool InputEngine::virtualKeyClick(int key, string text, int modifiers)

    Emits a key click event for the given \a key, \a text and \a modifiers.
    Returns \c true if the key event was accepted by the input engine.
*/
/*!
    Emits a key click event for the given \a key, \a text and \a modifiers.
    Returns \c true if the key event was accepted by the input engine.
*/
bool QVirtualKeyboardInputEngine::virtualKeyClick(Qt::Key key, const QString &text, Qt::KeyboardModifiers modifiers)
{
    Q_D(QVirtualKeyboardInputEngine);
    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputEngine::virtualKeyClick()"
#ifdef SENSITIVE_DEBUG
           << key << text << modifiers
#endif
        ;
    return d->virtualKeyClick(key, text, modifiers, false);
}

/*!
    Returns the \c InputContext instance associated with the input
    engine.
*/
QVirtualKeyboardInputContext *QVirtualKeyboardInputEngine::inputContext() const
{
    Q_D(const QVirtualKeyboardInputEngine);
    return d->inputContext;
}

/*!
    Returns the currently active key, or Qt::Key_unknown if no key is active.
*/
Qt::Key QVirtualKeyboardInputEngine::activeKey() const
{
    Q_D(const QVirtualKeyboardInputEngine);
    return d->activeKey;
}

/*!
    Returns the previously active key, or Qt::Key_unknown if no key has been
    active.
*/
Qt::Key QVirtualKeyboardInputEngine::previousKey() const
{
    Q_D(const QVirtualKeyboardInputEngine);
    return d->previousKey;
}

/*!
    Returns the active input method.
*/
QVirtualKeyboardAbstractInputMethod *QVirtualKeyboardInputEngine::inputMethod() const
{
    Q_D(const QVirtualKeyboardInputEngine);
    return d->inputMethod;
}

/*!
    Sets \a inputMethod as the active input method.
*/
void QVirtualKeyboardInputEngine::setInputMethod(QVirtualKeyboardAbstractInputMethod *inputMethod)
{
    Q_D(QVirtualKeyboardInputEngine);
    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputEngine::setInputMethod():" << inputMethod;
    if (d->inputMethod != inputMethod) {
        update();
        if (d->inputMethod) {
            QObject::disconnect(d->inputMethod.data(), &QVirtualKeyboardAbstractInputMethod::selectionListsChanged, this, &QVirtualKeyboardInputEngine::updateSelectionListModels);
            d->inputMethod->setInputEngine(nullptr);
        }
        d->inputMethod = inputMethod;
        if (d->inputMethod) {
            d->inputMethod->setInputEngine(this);
            QObject::connect(d->inputMethod.data(), &QVirtualKeyboardAbstractInputMethod::selectionListsChanged, this, &QVirtualKeyboardInputEngine::updateSelectionListModels);

            // Set current text case
            d->inputMethod->setTextCase(d->textCase);
        }
        updateSelectionListModels();
        emit inputMethodChanged();
        emit patternRecognitionModesChanged();
    }
}

/*!
    Returns the list of available input modes.
*/
QList<int> QVirtualKeyboardInputEngine::inputModes() const
{
    Q_D(const QVirtualKeyboardInputEngine);
    return d->inputModes;
}

QVirtualKeyboardInputEngine::InputMode QVirtualKeyboardInputEngine::inputMode() const
{
    Q_D(const QVirtualKeyboardInputEngine);
    return d->inputMode;
}

void QVirtualKeyboardInputEngine::setInputMode(QVirtualKeyboardInputEngine::InputMode inputMode)
{
    Q_D(QVirtualKeyboardInputEngine);
    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputEngine::setInputMode():" << inputMode;
    if (d->inputMethod) {
#ifdef QT_DEBUG
        // Cached input modes should be in sync with the input method
        // If the assert below fails, we have missed an update somewhere
        QList<int> cachedInputModes(d->inputModes);
        updateInputModes();
        Q_ASSERT(cachedInputModes == d->inputModes);
#endif
        if (d->inputModes.contains(static_cast<const int>(inputMode))) {
            d->inputMethod->setInputMode(d->inputContext->locale(), inputMode);
            if (d->inputMode != inputMode) {
                d->inputMode = inputMode;
                emit inputModeChanged();
            }
        } else {
            qWarning() << "Input mode" << inputMode <<
                          "is not in the list of available input modes" << d->inputModes;
        }
    }
}

QVirtualKeyboardSelectionListModel *QVirtualKeyboardInputEngine::wordCandidateListModel() const
{
    Q_D(const QVirtualKeyboardInputEngine);
    return d->selectionListModels[QVirtualKeyboardSelectionListModel::Type::WordCandidateList];
}

bool QVirtualKeyboardInputEngine::wordCandidateListVisibleHint() const
{
    Q_D(const QVirtualKeyboardInputEngine);
    const auto it = d->selectionListModels.constFind(QVirtualKeyboardSelectionListModel::Type::WordCandidateList);
    if (it == d->selectionListModels.cend())
        return false;
    return it.value()->dataSource() != nullptr;
}

/*!
   Returns list of supported pattern recognition modes.
*/
QList<int> QVirtualKeyboardInputEngine::patternRecognitionModes() const
{
    Q_D(const QVirtualKeyboardInputEngine);
    QList<PatternRecognitionMode> patterRecognitionModeList;
    if (d->inputMethod)
        patterRecognitionModeList = d->inputMethod->patternRecognitionModes();
    QList<int> resultList;
    if (patterRecognitionModeList.isEmpty())
        return resultList;
    resultList.reserve(patterRecognitionModeList.size());
    for (const PatternRecognitionMode &patternRecognitionMode : qAsConst(patterRecognitionModeList))
        resultList.append(static_cast<int>(patternRecognitionMode));
    return resultList;
}

/*!
    \qmlmethod QVirtualKeyboardTrace InputEngine::traceBegin(int traceId, int patternRecognitionMode, var traceCaptureDeviceInfo, var traceScreenInfo)
    \since QtQuick.VirtualKeyboard 2.0

    Starts a trace interaction with the input engine.

    The trace is uniquely identified by the \a traceId. The input engine will assign
    the id to the Trace object if the input method accepts the event.

    The \a patternRecognitionMode specifies the recognition mode used for the pattern.

    If the current input method accepts the event it returns a Trace object associated with this interaction.
    If the input method discards the event, it returns a null value.

    The \a traceCaptureDeviceInfo provides information about the source device and the \a traceScreenInfo
    provides information about the screen context.

    By definition, the Trace object remains valid until the traceEnd() method is called.

    The trace interaction is ended by calling the \l {InputEngine::traceEnd()} {InputEngine.traceEnd()} method.
*/
/*!
    \since QtQuick.VirtualKeyboard 2.0

    Starts a trace interaction with the input engine.

    The trace is uniquely identified by the \a traceId. The input engine will assign
    the id to the QVirtualKeyboardTrace object if the input method accepts the event.

    The \a patternRecognitionMode specifies the recognition mode used for the pattern.

    If the current input method accepts the event it returns a QVirtualKeyboardTrace object associated with this interaction.
    If the input method discards the event, it returns a NULL value.

    The \a traceCaptureDeviceInfo provides information about the source device and the \a traceScreenInfo
    provides information about the screen context.

    By definition, the QVirtualKeyboardTrace object remains valid until the traceEnd() method is called.

    The trace interaction is ended by calling the traceEnd() method.
*/
QVirtualKeyboardTrace *QVirtualKeyboardInputEngine::traceBegin(
        int traceId, PatternRecognitionMode patternRecognitionMode,
        const QVariantMap &traceCaptureDeviceInfo, const QVariantMap &traceScreenInfo)
{
    Q_D(QVirtualKeyboardInputEngine);
    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputEngine::traceBegin():"
                            << "traceId:" << traceId
                            << "patternRecognitionMode:" << patternRecognitionMode
                            << "traceCaptureDeviceInfo:" << traceCaptureDeviceInfo
                            << "traceScreenInfo:" << traceScreenInfo;
    if (!d->inputMethod)
        return nullptr;
    if (patternRecognitionMode == PatternRecognitionMode::None)
        return nullptr;
    if (!d->inputMethod->patternRecognitionModes().contains(patternRecognitionMode))
        return nullptr;
    QVirtualKeyboardTrace *trace = d->inputMethod->traceBegin(traceId, patternRecognitionMode, traceCaptureDeviceInfo, traceScreenInfo);
    if (trace)
        trace->setTraceId(traceId);
    return trace;
}

/*!
    \qmlmethod bool InputEngine::traceEnd(Trace trace)

    Ends the trace interaction with the input engine.

    The \a trace object may be discarded at any point after calling this function.

    The function returns true if the trace interaction was accepted (i.e. the touch
    events should not be used for anything else).
*/
/*!
    Ends the trace interaction with the input engine.

    The \a trace object may be discarded at any point after calling this function.

    The function returns true if the trace interaction was accepted (i.e. the touch
    events should not be used for anything else).
*/
bool QVirtualKeyboardInputEngine::traceEnd(QVirtualKeyboardTrace *trace)
{
    Q_D(QVirtualKeyboardInputEngine);
    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputEngine::traceEnd():" << trace;
    Q_ASSERT(trace);
    if (!d->inputMethod)
        return false;
    return d->inputMethod->traceEnd(trace);
}

/*!
    \since QtQuick.VirtualKeyboard 2.0

    This function attempts to reselect a word located at the \a cursorPosition.
    The \a reselectFlags define the rules for how the word should be selected in
    relation to the cursor position.

    The function returns \c true if the word was successfully reselected.
*/
bool QVirtualKeyboardInputEngine::reselect(int cursorPosition, const ReselectFlags &reselectFlags)
{
    Q_D(QVirtualKeyboardInputEngine);
    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputEngine::reselect():" << cursorPosition << reselectFlags;
    if (!d->inputMethod || !wordCandidateListVisibleHint())
        return false;
    return d->inputMethod->reselect(cursorPosition, reselectFlags);
}

/*!
    \internal
    This method is called when the current preedit text is clicked.
*/
bool QVirtualKeyboardInputEngine::clickPreeditText(int cursorPosition)
{
    Q_D(QVirtualKeyboardInputEngine);
    if (!d->inputMethod)
        return false;
    return d->inputMethod->clickPreeditText(cursorPosition);
}

/*!
    \internal
    Resets the input method.
*/
void QVirtualKeyboardInputEngine::reset()
{
    Q_D(QVirtualKeyboardInputEngine);
    if (d->inputMethod) {
        RecursiveMethodGuard guard(d->recursiveMethodLock);
        if (!guard.locked()) {
            emit inputMethodReset();
            updateInputModes();
        }
    } else {
        updateInputModes();
    }
}

/*!
    \internal
    Updates the input method's state. This method is called whenever the input
    context is changed.
*/
void QVirtualKeyboardInputEngine::update()
{
    Q_D(QVirtualKeyboardInputEngine);
    if (d->inputMethod) {
        RecursiveMethodGuard guard(d->recursiveMethodLock);
        if (!guard.locked()) {
            emit inputMethodUpdate();
        }
    }
}

/*!
    \internal
    Updates the text case for the input method.
*/
void QVirtualKeyboardInputEngine::shiftChanged()
{
    Q_D(QVirtualKeyboardInputEngine);
    TextCase newCase = d->inputContext->priv()->shiftHandler()->isShiftActive() ? TextCase::Upper : TextCase::Lower;
    if (d->textCase != newCase) {
        d->textCase = newCase;
        if (d->inputMethod) {
            d->inputMethod->setTextCase(d->textCase);
        }
    }
}

/*!
    \internal
*/
void QVirtualKeyboardInputEngine::updateSelectionListModels()
{
    Q_D(QVirtualKeyboardInputEngine);
    QList<QVirtualKeyboardSelectionListModel::Type> inactiveSelectionLists = d->selectionListModels.keys();
    if (d->inputMethod) {
        // Allocate selection lists for the input method
        const QList<QVirtualKeyboardSelectionListModel::Type> activeSelectionLists = d->inputMethod->selectionLists();
        for (const QVirtualKeyboardSelectionListModel::Type &selectionListType : activeSelectionLists) {
            auto it = d->selectionListModels.find(selectionListType);
            if (it == d->selectionListModels.end()) {
                it = d->selectionListModels.insert(selectionListType, new QVirtualKeyboardSelectionListModel(this));
                if (selectionListType == QVirtualKeyboardSelectionListModel::Type::WordCandidateList)
                    emit wordCandidateListModelChanged();
            }
            it.value()->setDataSource(d->inputMethod, selectionListType);
            if (selectionListType == QVirtualKeyboardSelectionListModel::Type::WordCandidateList)
                emit wordCandidateListVisibleHintChanged();
            inactiveSelectionLists.removeAll(selectionListType);
        }
    }

    // Deallocate inactive selection lists
    for (const QVirtualKeyboardSelectionListModel::Type &selectionListType : qAsConst(inactiveSelectionLists)) {
        const auto it = d->selectionListModels.constFind(selectionListType);
        if (it != d->selectionListModels.cend()) {
            it.value()->setDataSource(nullptr, selectionListType);
            if (selectionListType == QVirtualKeyboardSelectionListModel::Type::WordCandidateList)
                emit wordCandidateListVisibleHintChanged();
        }
    }
}

/*!
    \internal
*/
void QVirtualKeyboardInputEngine::updateInputModes()
{
    Q_D(QVirtualKeyboardInputEngine);
    QList<int> newInputModes;
    if (d->inputMethod) {
        QList<InputMode> tmpList(d->inputMethod->inputModes(d->inputContext->locale()));
        if (!tmpList.isEmpty()) {
            std::transform(tmpList.constBegin(), tmpList.constEnd(),
                           std::back_inserter(newInputModes),
                           [tmpList] (InputMode inputMode) {
                               return static_cast<int>(inputMode);
                           });
        }
    }
    if (d->inputModes != newInputModes) {
        d->inputModes = newInputModes;
        emit inputModesChanged();
    }
}

/*!
    \internal
*/
void QVirtualKeyboardInputEngine::timerEvent(QTimerEvent *timerEvent)
{
    Q_D(QVirtualKeyboardInputEngine);
    if (timerEvent->timerId() == d->repeatTimer) {
        d->repeatTimer = 0;
        d->virtualKeyClick(d->activeKey, d->activeKeyText, d->activeKeyModifiers, true);
        d->repeatTimer = startTimer(50);
        d->repeatCount++;
    }
}

/*!
    \qmlproperty int InputEngine::activeKey

    Currently pressed key.
*/

/*!
    \property QVirtualKeyboardInputEngine::activeKey
    \brief the active key.

    Currently pressed key.
*/

/*!
    \qmlproperty int InputEngine::previousKey

    Previously pressed key.
*/
/*!
    \property QVirtualKeyboardInputEngine::previousKey
    \brief the previous active key.

    Previously pressed key.
*/

/*!
    \qmlproperty InputMethod InputEngine::inputMethod

    Use this property to set the active input method, or to monitor when the
    active input method changes.
*/

/*!
    \property QVirtualKeyboardInputEngine::inputMethod
    \brief the active input method.

    Use this property to set active the input method, or to monitor when the
    active input method changes.
*/

/*!
    \qmlproperty list<int> InputEngine::inputModes

    The list of available input modes is dependent on the input method and
    locale. This property is updated when either of the dependencies change.
*/

/*!
    \property QVirtualKeyboardInputEngine::inputModes
    \brief the available input modes for active input method.

    The list of available input modes is dependent on the input method and
    locale. This property is updated when either of the dependencies changes.
*/

/*!
    \qmlproperty int InputEngine::inputMode

    Use this property to get or set the current input mode. The
    InputEngine::inputModes property provides the list of valid input modes
    for the current input method and locale.

    The predefined input modes are:

    \list
        \li \c InputEngine.InputMode.Latin The default input mode for latin text.
        \li \c InputEngine.InputMode.Numeric Only numeric input is allowed.
        \li \c InputEngine.InputMode.Dialable Only dialable input is allowed.
        \li \c InputEngine.InputMode.Pinyin Pinyin input mode for Chinese.
        \li \c InputEngine.InputMode.Cangjie Cangjie input mode for Chinese.
        \li \c InputEngine.InputMode.Zhuyin Zhuyin input mode for Chinese.
        \li \c InputEngine.InputMode.Hangul Hangul input mode for Korean.
        \li \c InputEngine.InputMode.Hiragana Hiragana input mode for Japanese.
        \li \c InputEngine.InputMode.Katakana Katakana input mode for Japanese.
        \li \c InputEngine.InputMode.FullwidthLatin Fullwidth latin input mode for East Asian languages.
        \li \c InputEngine.InputMode.Greek Greek input mode.
        \li \c InputEngine.InputMode.Cyrillic Cyrillic input mode.
        \li \c InputEngine.InputMode.Arabic Arabic input mode.
        \li \c InputEngine.InputMode.Hebrew Hebrew input mode.
        \li \c InputEngine.InputMode.ChineseHandwriting Chinese handwriting.
        \li \c InputEngine.InputMode.JapaneseHandwriting Japanese handwriting.
        \li \c InputEngine.InputMode.KoreanHandwriting Korean handwriting.
        \li \c InputEngine.InputMode.Thai Thai input mode.
    \endlist
*/

/*!
    \property QVirtualKeyboardInputEngine::inputMode
    \brief the current input mode.

    Use this property to get or set the current input mode. The
    InputEngine::inputModes provides list of valid input modes
    for current input method and locale.
*/

/*!
    \qmlproperty QVirtualKeyboardSelectionListModel InputEngine::wordCandidateListModel

    Use this property to access the list model for the word candidate
    list.
*/

/*!
    \property QVirtualKeyboardInputEngine::wordCandidateListModel
    \brief list model for the word candidate list.

    Use this property to access the list model for the word candidate
    list.
*/

/*!
    \qmlproperty bool InputEngine::wordCandidateListVisibleHint

    Use this property to check if the word candidate list should be visible
    in the UI.
*/

/*!
    \property QVirtualKeyboardInputEngine::wordCandidateListVisibleHint
    \brief visible hint for the word candidate list.

    Use this property to check if the word candidate list should be visible
    in the UI.
*/

/*!
    \enum QVirtualKeyboardInputEngine::InputMode

    This enum specifies the input mode for the input method.

    \value Latin
           The default input mode for latin text.
    \value Numeric
           Only numeric input is allowed.
    \value Dialable
           Only dialable input is allowed.
    \value Pinyin
           Pinyin input mode for Chinese.
    \value Cangjie
           Cangjie input mode for Chinese.
    \value Zhuyin
           Zhuyin input mode for Chinese.
    \value Hangul
           Hangul input mode for Korean.
    \value Hiragana
           Hiragana input mode for Japanese.
    \value Katakana
           Katakana input mode for Japanese.
    \value FullwidthLatin
           Fullwidth latin input mode for East Asian languages.
    \value Greek
           Greek input mode.
    \value Cyrillic
           Cyrillic input mode.
    \value Arabic
           Arabic input mode.
    \value Hebrew
           Hebrew input mode.
    \value ChineseHandwriting
           Chinese handwriting input mode.
    \value JapaneseHandwriting
           Japanese handwriting input mode.
    \value KoreanHandwriting
           Korean handwriting input mode.
    \value Thai
           Thai input mode.
*/

/*!
    \enum QVirtualKeyboardInputEngine::TextCase

    This enum specifies the text case for the input method.

    \value Lower
           Lower case text.
    \value Upper
           Upper case text.
*/

/*!
    \enum QVirtualKeyboardInputEngine::PatternRecognitionMode

    This enum specifies the input mode for the input method.

    \value None
           Pattern recognition is not available.
    \value PatternRecognitionDisabled
           \c obsolete Use PatternRecognitionMode::None
    \value Handwriting
           Pattern recognition mode for handwriting recognition.
    \value HandwritingRecoginition
           \c obsolete Use PatternRecognitionMode::Handwriting
*/

/*!
    \enum QVirtualKeyboardInputEngine::ReselectFlag

    This enum specifies the rules for word reselection.

    \value WordBeforeCursor
           Activate the word before the cursor. When this flag is used exclusively, the word must end exactly at the cursor.
    \value WordAfterCursor
           Activate the word after the cursor. When this flag is used exclusively, the word must start exactly at the cursor.
    \value WordAtCursor
           Activate the word at the cursor. This flag is a combination of the above flags with the exception that the word cannot start or stop at the cursor.
*/

/*!
    \qmlsignal void InputEngine::virtualKeyClicked(int key, string text, int modifiers)

    Indicates that the virtual \a key was clicked with the given \a text and
    \a modifiers.
    This signal is emitted after the input method has processed the key event.
*/

/*!
    \fn void QVirtualKeyboardInputEngine::virtualKeyClicked(Qt::Key key, const QString &text, Qt::KeyboardModifiers modifiers, bool isAutoRepeat)

    Indicates that the virtual \a key was clicked with the given \a text and
    \a modifiers. The \a isAutoRepeat indicates if the event is automatically
    repeated while the key is being pressed.
    This signal is emitted after the input method has processed the key event.
*/

/*!
    \qmlproperty list<int> InputEngine::patternRecognitionModes
    \since QtQuick.VirtualKeyboard 2.0

    The list of available pattern recognition modes.

    Possible values:

    \value InputEngine.PatternRecognitionMode.None
           Pattern recognition is not available.
    \value InputEngine.PatternRecognitionMode.PatternRecognitionDisabled
           \c obsolete - Use \c None instead.
    \value InputEngine.PatternRecognitionMode.Handwriting
           Pattern recognition mode for handwriting recognition.
    \value InputEngine.PatternRecognitionMode.HandwritingRecoginition
           \c obsolete - Use \c Handwriting instead.

*/

/*!
    \property QVirtualKeyboardInputEngine::patternRecognitionModes
    \since QtQuick.VirtualKeyboard 2.0
    \brief the list of available pattern recognition modes.

    The list of available pattern recognition modes.
*/

/*!
    \qmlsignal void InputEngine::activeKeyChanged(int key)

    Indicates that the active \a key has changed.
*/

/*!
    \fn void QVirtualKeyboardInputEngine::activeKeyChanged(Qt::Key key)

    Indicates that the active \a key has changed.
*/

/*!
    \qmlsignal void InputEngine::previousKeyChanged(int key)

    Indicates that the previous \a key has changed.
*/

/*!
    \fn void QVirtualKeyboardInputEngine::previousKeyChanged(Qt::Key key)

    Indicates that the previous \a key has changed.
*/

/*!
    \qmlsignal void InputEngine::inputMethodChanged()

    Indicates that the input method has changed.
*/

/*!
    \fn void QVirtualKeyboardInputEngine::inputMethodChanged()

    Indicates that the input method has changed.
*/

/*!
    \qmlsignal void InputEngine::inputMethodReset()

    Emitted when the input method needs to be reset.

    \note This signal is automatically connected to QVirtualKeyboardAbstractInputMethod::reset()
    and InputMethod::reset() when the input method is activated.
*/

/*!
    \fn void QVirtualKeyboardInputEngine::inputMethodReset()

    Emitted when the input method needs to be reset.

    \note This signal is automatically connected to QVirtualKeyboardAbstractInputMethod::reset()
    and InputMethod::reset() when the input method is activated.
*/

/*!
    \qmlsignal void InputEngine::inputMethodUpdate()

    \note This signal is automatically connected to QVirtualKeyboardAbstractInputMethod::update()
    and InputMethod::update() when the input method is activated.
*/

/*!
    \fn void QVirtualKeyboardInputEngine::inputMethodUpdate()

    \note This signal is automatically connected to QVirtualKeyboardAbstractInputMethod::update()
    and InputMethod::update() when the input method is activated.
*/

/*!
    \qmlsignal void InputEngine::inputModesChanged()

    Indicates that the available input modes have changed.
*/

/*!
    \fn void QVirtualKeyboardInputEngine::inputModesChanged()

    Indicates that the available input modes have changed.
*/

/*!
    \qmlsignal void InputEngine::inputModeChanged()

    Indicates that the input mode has changed.
*/

/*!
    \fn void QVirtualKeyboardInputEngine::inputModeChanged()

    Indicates that the input mode has changed.
*/

/*!
    \qmlsignal void InputEngine::patternRecognitionModesChanged()
    \since QtQuick.VirtualKeyboard 2.0

    Indicates that the available pattern recognition modes have changed.

    The predefined pattern recognition modes are:

    \list
        \li \c InputEngine.PatternRecognitionMode.None Pattern recognition is not available.
        \li \c InputEngine.PatternRecognitionMode.PatternRecognitionDisabled \c obsolete Use InputEngine.PatternRecognitionMode.None
        \li \c InputEngine.PatternRecognitionMode.Handwriting Pattern recognition mode for handwriting recognition.
        \li \c InputEngine.PatternRecognitionMode.HandwritingRecoginition \c obsolete Use InputEngine.PatternRecognitionMode.Handwriting
    \endlist
*/

/*!
    \fn void QVirtualKeyboardInputEngine::patternRecognitionModesChanged()
    \since QtQuick.VirtualKeyboard 2.0

    Indicates that the available pattern recognition modes have changed.
*/

QT_END_NAMESPACE
