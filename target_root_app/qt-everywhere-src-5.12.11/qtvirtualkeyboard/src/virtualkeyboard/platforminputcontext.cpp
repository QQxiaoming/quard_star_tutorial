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

#include <QtVirtualKeyboard/private/platforminputcontext_p.h>
#include <QtVirtualKeyboard/qvirtualkeyboardinputcontext.h>
#include <QtVirtualKeyboard/private/qvirtualkeyboardinputcontext_p.h>
#include <QtVirtualKeyboard/private/shadowinputcontext_p.h>
#include <QtVirtualKeyboard/private/abstractinputpanel_p.h>
#ifdef QT_VIRTUALKEYBOARD_DESKTOP
#include <QtVirtualKeyboard/private/desktopinputpanel_p.h>
#endif
#include <QtVirtualKeyboard/private/appinputpanel_p.h>
#include <QtVirtualKeyboard/private/virtualkeyboarddebug_p.h>

#include <QWindow>
#include <QGuiApplication>

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

Q_LOGGING_CATEGORY(qlcVirtualKeyboard, "qt.virtualkeyboard")

/*!
    \class QtVirtualKeyboard::PlatformInputContext
    \internal
*/

PlatformInputContext::PlatformInputContext() :
    m_inputContext(nullptr),
    m_inputPanel(nullptr),
    m_focusObject(nullptr),
    m_locale(),
    m_inputDirection(m_locale.textDirection()),
    m_filterEvent(nullptr),
    m_visible(false)
{
}

PlatformInputContext::~PlatformInputContext()
{
}

bool PlatformInputContext::isValid() const
{
    return true;
}

void PlatformInputContext::reset()
{
    VIRTUALKEYBOARD_DEBUG() << "PlatformInputContext::reset()";
    if (m_inputContext)
        m_inputContext->priv()->reset();
}

void PlatformInputContext::commit()
{
    VIRTUALKEYBOARD_DEBUG() << "PlatformInputContext::commit()";
    if (m_inputContext)
        m_inputContext->priv()->commit();
}

void PlatformInputContext::update(Qt::InputMethodQueries queries)
{
    VIRTUALKEYBOARD_DEBUG() << "PlatformInputContext::update():" << queries;
    bool enabled = inputMethodQuery(Qt::ImEnabled).toBool();
#ifdef QT_VIRTUALKEYBOARD_DESKTOP
    if (enabled && !m_inputPanel) {
        m_inputPanel = new DesktopInputPanel(this);
        m_inputPanel->createView();
        if (m_inputContext) {
            m_selectionControl = new DesktopInputSelectionControl(this, m_inputContext);
            m_selectionControl->createHandles();
        }
    }
#endif
    if (m_inputContext) {
        if (enabled) {
            m_inputContext->priv()->update(queries);
            if (m_visible)
                updateInputPanelVisible();
        } else {
            hideInputPanel();
        }
        m_inputContext->priv()->setFocus(enabled);
    }
}

void PlatformInputContext::invokeAction(QInputMethod::Action action, int cursorPosition)
{
    VIRTUALKEYBOARD_DEBUG() << "PlatformInputContext::invokeAction():" << action << cursorPosition;
    if (m_inputContext)
        m_inputContext->priv()->invokeAction(action, cursorPosition);
}

QRectF PlatformInputContext::keyboardRect() const
{
    return m_inputContext ? m_inputContext->priv()->keyboardRectangle() : QRectF();
}

bool PlatformInputContext::isAnimating() const
{
    return m_inputContext ? m_inputContext->isAnimating() : false;
}

void PlatformInputContext::showInputPanel()
{
    if (!m_visible) {
        VIRTUALKEYBOARD_DEBUG() << "PlatformInputContext::showInputPanel()";
        m_visible = true;
    }
    updateInputPanelVisible();
}

void PlatformInputContext::hideInputPanel()
{
    if (m_visible) {
        VIRTUALKEYBOARD_DEBUG() << "PlatformInputContext::hideInputPanel()";
        m_visible = false;
    }
    updateInputPanelVisible();
}

bool PlatformInputContext::isInputPanelVisible() const
{
    return m_inputPanel ? m_inputPanel->isVisible() : false;
}

QLocale PlatformInputContext::locale() const
{
    return m_locale;
}

void PlatformInputContext::setLocale(QLocale locale)
{
    if (m_locale != locale) {
        VIRTUALKEYBOARD_DEBUG() << "PlatformInputContext::setLocale():" << locale;
        m_locale = locale;
        emitLocaleChanged();
    }
}

Qt::LayoutDirection PlatformInputContext::inputDirection() const
{
    return m_inputDirection;
}

void PlatformInputContext::setInputDirection(Qt::LayoutDirection direction)
{
    if (m_inputDirection != direction) {
        VIRTUALKEYBOARD_DEBUG() << "PlatformInputContext::setInputDirection():" << direction;
        m_inputDirection = direction;
        emitInputDirectionChanged(m_inputDirection);
    }
}

QObject *PlatformInputContext::focusObject()
{
    return m_focusObject;
}

void PlatformInputContext::setFocusObject(QObject *object)
{
    VIRTUALKEYBOARD_DEBUG() << "PlatformInputContext::setFocusObject():" << object;
    Q_ASSERT(m_inputContext == nullptr ||
             m_inputContext->priv()->shadow()->inputItem() == nullptr ||
             m_inputContext->priv()->shadow()->inputItem() != object);
    if (m_focusObject != object) {
        if (m_focusObject)
            m_focusObject->removeEventFilter(this);
        m_focusObject = object;
        if (m_focusObject)
            m_focusObject->installEventFilter(this);
        emit focusObjectChanged();
    }
    update(Qt::ImQueryAll);
}

QVirtualKeyboardInputContext *PlatformInputContext::inputContext() const
{
    return m_inputContext;
}

bool PlatformInputContext::eventFilter(QObject *object, QEvent *event)
{
    if (event != m_filterEvent && object == m_focusObject && m_inputContext)
        return m_inputContext->priv()->filterEvent(event);
    return false;
}

void PlatformInputContext::setInputMethods(const QStringList &inputMethods)
{
    m_inputMethods = inputMethods;
}

QStringList PlatformInputContext::inputMethods() const
{
    return m_inputMethods;
}

void PlatformInputContext::sendEvent(QEvent *event)
{
    if (m_focusObject) {
        m_filterEvent = event;
        QGuiApplication::sendEvent(m_focusObject, event);
        m_filterEvent = nullptr;
    }
}

void PlatformInputContext::sendKeyEvent(QKeyEvent *event)
{
    const QGuiApplication *app = qApp;
    QWindow *focusWindow = app ? app->focusWindow() : nullptr;
    if (focusWindow) {
        m_filterEvent = event;
        QGuiApplication::sendEvent(focusWindow, event);
        m_filterEvent = nullptr;
    }
}

QVariant PlatformInputContext::inputMethodQuery(Qt::InputMethodQuery query)
{
    QInputMethodQueryEvent event(query);
    sendEvent(&event);
    return event.value(query);
}

void PlatformInputContext::setInputContext(QVirtualKeyboardInputContext *context)
{
    if (m_inputContext) {
        disconnect(this, SLOT(keyboardRectangleChanged()));
    }
    m_inputContext = context;
    if (m_inputContext) {
        if (!m_inputPanel)
            m_inputPanel = new AppInputPanel(this);
        QObject::connect(m_inputContext->priv(), &QVirtualKeyboardInputContextPrivate::keyboardRectangleChanged, this, &PlatformInputContext::keyboardRectangleChanged);
    } else if (m_inputPanel) {
        m_inputPanel = nullptr;
    }
}

void PlatformInputContext::keyboardRectangleChanged()
{
    m_inputPanel->setInputRect(m_inputContext->priv()->keyboardRectangle().toRect());
}

void PlatformInputContext::updateInputPanelVisible()
{
    if (!m_inputPanel)
        return;

    if (m_visible != m_inputPanel->isVisible()) {
        if (m_visible)
            m_inputPanel->show();
        else
            m_inputPanel->hide();
        if (m_selectionControl)
            m_selectionControl->setEnabled(m_visible);
        emitInputPanelVisibleChanged();
    }
}

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE
