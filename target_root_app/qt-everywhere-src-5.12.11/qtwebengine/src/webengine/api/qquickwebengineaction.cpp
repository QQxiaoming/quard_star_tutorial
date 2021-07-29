/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qquickwebengineaction_p.h"
#include "qquickwebengineaction_p_p.h"
#include "qquickwebengineview_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype WebEngineAction
    \instantiates QQuickWebEngineAction
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.8

    \brief An action that represents a \l WebEngineView::WebAction.

    A WebEngineAction is returned by the \l WebEngineView::action()
    method. It provides information about the action, such as
    whether it is \l enabled.

    The following code uses the \l WebEngineView::action() method to check if the
    the copy action is enabled:

    \code
    var copyAction = webEngineView.action(WebEngineView.Copy);
    if (copyAction.enabled)
        console.log("Copy is enabled.");
    else
        console.log("Copy is disabled.");
    \endcode
*/

QQuickWebEngineActionPrivate::QQuickWebEngineActionPrivate(const QVariant &data, const QString &text, const QString &iconName, bool enabled)
    : m_data(data)
    , m_text(text)
    , m_iconName(iconName)
    , m_enabled(enabled)
{
}

QQuickWebEngineActionPrivate::~QQuickWebEngineActionPrivate()
{
}

void QQuickWebEngineActionPrivate::setEnabled(bool enabled)
{
    Q_Q(QQuickWebEngineAction);
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    emit q->enabledChanged();
}

QVariant QQuickWebEngineActionPrivate::data() const
{
    return m_data;
}

void QQuickWebEngineActionPrivate::trigger()
{
    Q_Q(QQuickWebEngineAction);
    if (QQuickWebEngineView *view = static_cast<QQuickWebEngineView*>(q->parent())) {
        view->triggerWebAction(static_cast<QQuickWebEngineView::WebAction>(data().toInt()));
    }
}

QQuickWebEngineAction::QQuickWebEngineAction(const QVariant &data, const QString &text, const QString &iconName, bool enabled, QObject *parent)
    : QObject(parent)
    , d_ptr(new QQuickWebEngineActionPrivate(data, text, iconName, enabled))
{
    d_ptr->q_ptr = this;
}

QQuickWebEngineAction::QQuickWebEngineAction(QObject *parent)
    : QObject(parent)
    , d_ptr(new QQuickWebEngineActionPrivate(-1, QStringLiteral(""), QStringLiteral(""), false))
{
    d_ptr->q_ptr = this;
}

QQuickWebEngineAction::~QQuickWebEngineAction()
{
}

/*!
    \qmlproperty int WebEngineAction::text

    This property holds a textual description of the action.
*/
QString QQuickWebEngineAction::text() const
{
    Q_D(const QQuickWebEngineAction);
    return d->m_text;
}

/*!
    \qmlproperty string WebEngineAction::iconName

    This property holds the name of the icon for the action. This name
    can be used to pick the icon from a theme.
*/
QString QQuickWebEngineAction::iconName() const
{
    Q_D(const QQuickWebEngineAction);
    return d->m_iconName;
}

/*!
    \qmlproperty bool WebEngineAction::enabled

    This property holds whether the action is enabled.
*/
bool QQuickWebEngineAction::isEnabled() const
{
    Q_D(const QQuickWebEngineAction);
    return d->m_enabled;
}

/*!
    \qmlmethod void WebEngineAction::trigger()

    Triggers the action.
*/
void QQuickWebEngineAction::trigger()
{
    Q_D(QQuickWebEngineAction);
    if (!isEnabled())
        return;

    d->trigger();
    emit triggered();
}

QT_END_NAMESPACE

