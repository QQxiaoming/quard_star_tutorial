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

#include <QtVirtualKeyboard/private/qvirtualkeyboardinputcontext_p.h>
#include <QtVirtualKeyboard/private/platforminputcontext_p.h>
#include <QtVirtualKeyboard/private/settings_p.h>
#include <QtVirtualKeyboard/private/shifthandler_p.h>
#include <QtVirtualKeyboard/private/virtualkeyboarddebug_p.h>
#include <QtVirtualKeyboard/private/enterkeyaction_p.h>
#include <QtVirtualKeyboard/qvirtualkeyboardinputengine.h>

#include <QGuiApplication>
#include <QtGui/qpa/qplatformintegration.h>
#include <QtGui/private/qguiapplication_p.h>

QT_BEGIN_NAMESPACE

bool operator==(const QInputMethodEvent::Attribute &attribute1, const QInputMethodEvent::Attribute &attribute2)
{
    return attribute1.start == attribute2.start &&
           attribute1.length == attribute2.length &&
           attribute1.type == attribute2.type &&
           attribute1.value == attribute2.value;
}

using namespace QtVirtualKeyboard;

QVirtualKeyboardInputContextPrivate::QVirtualKeyboardInputContextPrivate(QVirtualKeyboardInputContext *q_ptr) :
    QObject(nullptr),
    q_ptr(q_ptr),
    platformInputContext(nullptr),
    inputEngine(nullptr),
    _shiftHandler(nullptr),
    keyboardRect(),
    previewRect(),
    _previewVisible(false),
    animating(false),
    _focus(false),
    cursorPosition(0),
    anchorPosition(0),
    forceAnchorPosition(-1),
    _forceCursorPosition(-1),
    inputMethodHints(Qt::ImhNone),
    preeditText(),
    preeditTextAttributes(),
    surroundingText(),
    selectedText(),
    anchorRectangle(),
    cursorRectangle(),
    selectionControlVisible(false),
    anchorRectIntersectsClipRect(false),
    cursorRectIntersectsClipRect(false)
#ifdef QT_VIRTUALKEYBOARD_ARROW_KEY_NAVIGATION
    , activeNavigationKeys()
#endif
{
}

void QVirtualKeyboardInputContextPrivate::init()
{
    Q_Q(QVirtualKeyboardInputContext);
    QGuiApplicationPrivate *guiApplicationPrivate = QGuiApplicationPrivate::instance();
    QPlatformIntegration *platformIntegration = guiApplicationPrivate->platformIntegration();
    QPlatformInputContext *unknownPlatformInputContext = platformIntegration->inputContext();
    platformInputContext = qobject_cast<PlatformInputContext *>(unknownPlatformInputContext);
    inputEngine = new QVirtualKeyboardInputEngine(q);
    _shiftHandler = new ShiftHandler(q);
    inputEngine->init();
    _shiftHandler->init();
    _shadow.setInputContext(q);
    if (platformInputContext) {
        platformInputContext->setInputContext(q);
        QObject::connect(platformInputContext, &PlatformInputContext::focusObjectChanged, this, &QVirtualKeyboardInputContextPrivate::onInputItemChanged);
        QObject::connect(platformInputContext, &PlatformInputContext::focusObjectChanged, this, &QVirtualKeyboardInputContextPrivate::inputItemChanged);
    }
}

QVirtualKeyboardInputContextPrivate::~QVirtualKeyboardInputContextPrivate()
{
}

bool QVirtualKeyboardInputContextPrivate::focus() const
{
    return _focus;
}

void QVirtualKeyboardInputContextPrivate::setFocus(bool focus)
{
    if (_focus != focus) {
        VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputContextPrivate::setFocus():" << focus;
        _focus = focus;
        emit focusChanged();
    }
}

QRectF QVirtualKeyboardInputContextPrivate::keyboardRectangle() const
{
    return keyboardRect;
}

void QVirtualKeyboardInputContextPrivate::setKeyboardRectangle(QRectF rectangle)
{
    if (keyboardRect != rectangle) {
        keyboardRect = rectangle;
        emit keyboardRectangleChanged();
        platformInputContext->emitKeyboardRectChanged();
    }
}

QRectF QVirtualKeyboardInputContextPrivate::previewRectangle() const
{
    return previewRect;
}

void QVirtualKeyboardInputContextPrivate::setPreviewRectangle(QRectF rectangle)
{
    if (previewRect != rectangle) {
        previewRect = rectangle;
        emit previewRectangleChanged();
    }
}

bool QVirtualKeyboardInputContextPrivate::previewVisible() const
{
    return _previewVisible;
}

void QVirtualKeyboardInputContextPrivate::setPreviewVisible(bool visible)
{
    if (_previewVisible != visible) {
        _previewVisible = visible;
        emit previewVisibleChanged();
    }
}

QString QVirtualKeyboardInputContextPrivate::locale() const
{
    return platformInputContext ? platformInputContext->locale().name() : QString();
}

void QVirtualKeyboardInputContextPrivate::setLocale(const QString &locale)
{
    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputContextPrivate::setLocale():" << locale;
    QLocale newLocale(locale);
    if (newLocale != platformInputContext->locale()) {
        platformInputContext->setLocale(newLocale);
        platformInputContext->setInputDirection(newLocale.textDirection());
        emit localeChanged();
    }
}

QObject *QVirtualKeyboardInputContextPrivate::inputItem() const
{
    return platformInputContext ? platformInputContext->focusObject() : nullptr;
}

ShiftHandler *QVirtualKeyboardInputContextPrivate::shiftHandler() const
{
    return _shiftHandler;
}

ShadowInputContext *QVirtualKeyboardInputContextPrivate::shadow() const
{
    return const_cast<ShadowInputContext *>(&_shadow);
}

QStringList QVirtualKeyboardInputContextPrivate::inputMethods() const
{
    return platformInputContext ? platformInputContext->inputMethods() : QStringList();
}

bool QVirtualKeyboardInputContextPrivate::fileExists(const QUrl &fileUrl)
{
    QString fileName;
    if (fileUrl.scheme() == QLatin1String("qrc")) {
        fileName = QLatin1Char(':') + fileUrl.path();
    } else {
        fileName = fileUrl.toLocalFile();
    }
    return !fileName.isEmpty() && QFile::exists(fileName);
}

bool QVirtualKeyboardInputContextPrivate::hasEnterKeyAction(QObject *item) const
{
    return item != nullptr && qmlAttachedPropertiesObject<EnterKeyAction>(item, false);
}

void QVirtualKeyboardInputContextPrivate::hideInputPanel()
{
    platformInputContext->hideInputPanel();
}

void QVirtualKeyboardInputContextPrivate::updateAvailableLocales(const QStringList &availableLocales)
{
    Settings *settings = Settings::instance();
    if (settings)
        settings->setAvailableLocales(availableLocales);
}

void QVirtualKeyboardInputContextPrivate::forceCursorPosition(int anchorPosition, int cursorPosition)
{
    if (!_shadow.inputItem())
        return;
    if (!platformInputContext->m_visible)
        return;
    if (testState(State::Reselect))
        return;
    if (testState(State::SyncShadowInput))
        return;

    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputContextPrivate::forceCursorPosition():" << cursorPosition << "anchorPosition:" << anchorPosition;
    if (!preeditText.isEmpty()) {
        forceAnchorPosition = -1;
        _forceCursorPosition = cursorPosition;
        if (cursorPosition > this->cursorPosition)
            _forceCursorPosition += preeditText.length();
        commit();
    } else {
        forceAnchorPosition = anchorPosition;
        _forceCursorPosition = cursorPosition;
        Q_Q(QVirtualKeyboardInputContext);
        q->setPreeditText(QString());
        if (!inputMethodHints.testFlag(Qt::ImhNoPredictiveText) &&
                   cursorPosition > 0 && selectedText.isEmpty()) {
            QVirtualKeyboardScopedState reselectState(this, State::Reselect);
            if (inputEngine->reselect(cursorPosition, QVirtualKeyboardInputEngine::ReselectFlag::WordAtCursor))
                setState(State::InputMethodClick);
        }
    }
}

void QVirtualKeyboardInputContextPrivate::onInputItemChanged()
{
    if (!inputItem() && !activeKeys.isEmpty()) {
        // After losing keyboard focus it is impossible to track pressed keys
        activeKeys.clear();
        clearState(State::KeyEvent);
    }
    clearState(State::InputMethodClick);
}

void QVirtualKeyboardInputContextPrivate::sendPreedit(const QString &text, const QList<QInputMethodEvent::Attribute> &attributes, int replaceFrom, int replaceLength)
{
    VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputContextPrivate::sendPreedit()"
#ifdef SENSITIVE_DEBUG
           << text << replaceFrom << replaceLength
#endif
        ;

    bool textChanged = preeditText != text;
    bool attributesChanged = preeditTextAttributes != attributes;

    if (textChanged || attributesChanged) {
        preeditText = text;
        preeditTextAttributes = attributes;

        if (platformInputContext) {
            QInputMethodEvent event(text, attributes);
            const bool replace = replaceFrom != 0 || replaceLength > 0;
            if (replace)
                event.setCommitString(QString(), replaceFrom, replaceLength);

            sendInputMethodEvent(&event);

            // Send also to shadow input if only attributes changed.
            // In this case the update() may not be called, so the shadow
            // input may be out of sync.
            if (_shadow.inputItem() && !replace && !text.isEmpty() &&
                    !textChanged && attributesChanged) {
                VIRTUALKEYBOARD_DEBUG() << "QVirtualKeyboardInputContextPrivate::sendPreedit(shadow)"
#ifdef SENSITIVE_DEBUG
                       << text << replaceFrom << replaceLength
#endif
                    ;
                event.setAccepted(true);
                QGuiApplication::sendEvent(_shadow.inputItem(), &event);
            }
        }

        if (textChanged) {
            Q_Q(QVirtualKeyboardInputContext);
            emit q->preeditTextChanged();
        }
    }

    if (preeditText.isEmpty())
        preeditTextAttributes.clear();
}

void QVirtualKeyboardInputContextPrivate::sendInputMethodEvent(QInputMethodEvent *event)
{
    QVirtualKeyboardScopedState inputMethodEventState(this, State::InputMethodEvent);
    platformInputContext->sendEvent(event);
}

void QVirtualKeyboardInputContextPrivate::reset()
{
    inputEngine->reset();
}

void QVirtualKeyboardInputContextPrivate::commit()
{
    inputEngine->update();
}

void QVirtualKeyboardInputContextPrivate::update(Qt::InputMethodQueries queries)
{
    Q_Q(QVirtualKeyboardInputContext);

    // No need to fetch input clip rectangle during animation
    if (!(queries & ~Qt::ImInputItemClipRectangle) && animating)
        return;

    // fetch
    QInputMethodQueryEvent imQueryEvent(Qt::InputMethodQueries(Qt::ImHints |
                    Qt::ImQueryInput | Qt::ImInputItemClipRectangle));
    platformInputContext->sendEvent(&imQueryEvent);
    Qt::InputMethodHints inputMethodHints = Qt::InputMethodHints(imQueryEvent.value(Qt::ImHints).toInt());
    const int cursorPosition = imQueryEvent.value(Qt::ImCursorPosition).toInt();
    const int anchorPosition = imQueryEvent.value(Qt::ImAnchorPosition).toInt();
    QRectF anchorRectangle;
    QRectF cursorRectangle;
    if (const QGuiApplication *app = qApp) {
        anchorRectangle = app->inputMethod()->anchorRectangle();
        cursorRectangle = app->inputMethod()->cursorRectangle();
    } else {
        anchorRectangle = this->anchorRectangle;
        cursorRectangle = this->cursorRectangle;
    }
    QString surroundingText = imQueryEvent.value(Qt::ImSurroundingText).toString();
    QString selectedText = imQueryEvent.value(Qt::ImCurrentSelection).toString();

    // check against changes
    bool newInputMethodHints = inputMethodHints != this->inputMethodHints;
    bool newSurroundingText = surroundingText != this->surroundingText;
    bool newSelectedText = selectedText != this->selectedText;
    bool newAnchorPosition = anchorPosition != this->anchorPosition;
    bool newCursorPosition = cursorPosition != this->cursorPosition;
    bool newAnchorRectangle = anchorRectangle != this->anchorRectangle;
    bool newCursorRectangle = cursorRectangle != this->cursorRectangle;
    bool selectionControlVisible = platformInputContext->isInputPanelVisible() && (cursorPosition != anchorPosition) && !inputMethodHints.testFlag(Qt::ImhNoTextHandles);
    bool newSelectionControlVisible = selectionControlVisible != this->selectionControlVisible;

    QRectF inputItemClipRect = imQueryEvent.value(Qt::ImInputItemClipRectangle).toRectF();
    QRectF anchorRect = imQueryEvent.value(Qt::ImAnchorRectangle).toRectF();
    QRectF cursorRect = imQueryEvent.value(Qt::ImCursorRectangle).toRectF();

    bool anchorRectIntersectsClipRect = inputItemClipRect.intersects(anchorRect);
    bool newAnchorRectIntersectsClipRect = anchorRectIntersectsClipRect != this->anchorRectIntersectsClipRect;

    bool cursorRectIntersectsClipRect = inputItemClipRect.intersects(cursorRect);
    bool newCursorRectIntersectsClipRect = cursorRectIntersectsClipRect != this->cursorRectIntersectsClipRect;

    // update
    this->inputMethodHints = inputMethodHints;
    this->surroundingText = surroundingText;
    this->selectedText = selectedText;
    this->anchorPosition = anchorPosition;
    this->cursorPosition = cursorPosition;
    this->anchorRectangle = anchorRectangle;
    this->cursorRectangle = cursorRectangle;
    this->selectionControlVisible = selectionControlVisible;
    this->anchorRectIntersectsClipRect = anchorRectIntersectsClipRect;
    this->cursorRectIntersectsClipRect = cursorRectIntersectsClipRect;

    // update input engine
    if ((newSurroundingText || newCursorPosition) &&
            !testState(State::InputMethodEvent)) {
        commit();
    }
    if (newInputMethodHints) {
        reset();
    }

    // notify
    if (newInputMethodHints) {
        emit q->inputMethodHintsChanged();
    }
    if (newSurroundingText) {
        emit q->surroundingTextChanged();
    }
    if (newSelectedText) {
        emit q->selectedTextChanged();
    }
    if (newAnchorPosition) {
        emit q->anchorPositionChanged();
    }
    if (newCursorPosition) {
        emit q->cursorPositionChanged();
    }
    if (newAnchorRectangle) {
        emit q->anchorRectangleChanged();
    }
    if (newCursorRectangle) {
        emit q->cursorRectangleChanged();
    }
    if (newSelectionControlVisible) {
        emit q->selectionControlVisibleChanged();
    }
    if (newAnchorRectIntersectsClipRect) {
        emit q->anchorRectIntersectsClipRectChanged();
    }
    if (newCursorRectIntersectsClipRect) {
        emit q->cursorRectIntersectsClipRectChanged();
    }

    // word reselection
    if (newInputMethodHints || newSurroundingText || newSelectedText)
        clearState(State::InputMethodClick);
    if ((newSurroundingText || newCursorPosition) && !newSelectedText && isEmptyState() &&
            !inputMethodHints.testFlag(Qt::ImhNoPredictiveText) &&
            cursorPosition > 0 && this->selectedText.isEmpty()) {
        QVirtualKeyboardScopedState reselectState(this, State::Reselect);
        if (inputEngine->reselect(cursorPosition, QVirtualKeyboardInputEngine::ReselectFlag::WordAtCursor))
            setState(State::InputMethodClick);
    }

    if (!testState(State::SyncShadowInput)) {
        QVirtualKeyboardScopedState syncShadowInputState(this, State::SyncShadowInput);
        _shadow.update(queries);
    }
}

void QVirtualKeyboardInputContextPrivate::invokeAction(QInputMethod::Action action, int cursorPosition)
{
    switch (action) {
    case QInputMethod::Click:
        if (isEmptyState()) {
            if (inputEngine->clickPreeditText(cursorPosition))
                break;

            bool reselect = !inputMethodHints.testFlag(Qt::ImhNoPredictiveText) && selectedText.isEmpty() && cursorPosition < preeditText.length();
            if (reselect) {
                QVirtualKeyboardScopedState reselectState(this, State::Reselect);
                _forceCursorPosition = this->cursorPosition + cursorPosition;
                commit();
                inputEngine->reselect(this->cursorPosition, QVirtualKeyboardInputEngine::ReselectFlag::WordBeforeCursor);
            } else if (!preeditText.isEmpty() && cursorPosition == preeditText.length()) {
                commit();
            }
        }
        clearState(State::InputMethodClick);
        break;

    case QInputMethod::ContextMenu:
        break;
    }
}

bool QVirtualKeyboardInputContextPrivate::filterEvent(const QEvent *event)
{
    QEvent::Type type = event->type();
    if (type == QEvent::KeyPress || type == QEvent::KeyRelease) {
        const QKeyEvent *keyEvent = static_cast<const QKeyEvent *>(event);

        // Keep track of pressed keys update key event state
        if (type == QEvent::KeyPress)
            activeKeys += keyEvent->nativeScanCode();
        else if (type == QEvent::KeyRelease)
            activeKeys -= keyEvent->nativeScanCode();

        if (activeKeys.isEmpty())
            clearState(State::KeyEvent);
        else
            setState(State::KeyEvent);

#ifdef QT_VIRTUALKEYBOARD_ARROW_KEY_NAVIGATION
        int key = keyEvent->key();
        if ((key >= Qt::Key_Left && key <= Qt::Key_Down) || key == Qt::Key_Return) {
            if (type == QEvent::KeyPress && platformInputContext->isInputPanelVisible()) {
                activeNavigationKeys += key;
                emit navigationKeyPressed(key, keyEvent->isAutoRepeat());
                return true;
            } else if (type == QEvent::KeyRelease && activeNavigationKeys.contains(key)) {
                activeNavigationKeys -= key;
                emit navigationKeyReleased(key, keyEvent->isAutoRepeat());
                return true;
            }
        }
#endif

        // Break composing text since the virtual keyboard does not support hard keyboard events
        if (!preeditText.isEmpty())
            commit();
    }
    return false;
}

void QVirtualKeyboardInputContextPrivate::addSelectionAttribute(QList<QInputMethodEvent::Attribute> &attributes)
{
    if (!testAttribute(attributes, QInputMethodEvent::Selection)) {
        // Convert Cursor attribute to Selection attribute.
        // In this case the cursor is set in pre-edit text, but
        // the cursor is not being forced to specific location.
        if (_forceCursorPosition == -1) {
            int cursorAttributeIndex = findAttribute(preeditTextAttributes, QInputMethodEvent::Cursor);
            if (cursorAttributeIndex != -1 && preeditTextAttributes[cursorAttributeIndex].length > 0)
                _forceCursorPosition = cursorPosition + preeditTextAttributes[cursorAttributeIndex].start;
            forceAnchorPosition = -1;
        }

        if (_forceCursorPosition != -1) {
            if (forceAnchorPosition != -1)
                attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::Selection, forceAnchorPosition, _forceCursorPosition - forceAnchorPosition, QVariant()));
            else
                attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::Selection, _forceCursorPosition, 0, QVariant()));
        }
    }
    forceAnchorPosition = -1;
    _forceCursorPosition = -1;
}

bool QVirtualKeyboardInputContextPrivate::testAttribute(const QList<QInputMethodEvent::Attribute> &attributes, QInputMethodEvent::AttributeType attributeType) const
{
    for (const QInputMethodEvent::Attribute &attribute : qAsConst(attributes)) {
        if (attribute.type == attributeType)
            return true;
    }
    return false;
}

int QVirtualKeyboardInputContextPrivate::findAttribute(const QList<QInputMethodEvent::Attribute> &attributes, QInputMethodEvent::AttributeType attributeType) const
{
    const int count = attributes.count();
    for (int i = 0; i < count; ++i) {
        if (attributes.at(i).type == attributeType)
            return i;
    }
    return -1;
}

QT_END_NAMESPACE
