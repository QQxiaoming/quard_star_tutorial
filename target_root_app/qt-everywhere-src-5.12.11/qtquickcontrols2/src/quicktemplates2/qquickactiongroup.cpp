/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickactiongroup_p.h"

#include <QtCore/private/qobject_p.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qvariant.h>
#include <QtQml/qqmlinfo.h>

#include "qquickaction_p.h"
#include "qquickaction_p_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype ActionGroup
    \inherits QtObject
    \instantiates QQuickActionGroup
    \inqmlmodule QtQuick.Controls
    \since 5.10
    \ingroup utilities
    \brief Groups actions together.

    ActionGroup is a non-visual group of actions. A mutually \l exclusive
    action group is used with actions where only one of the options can be
    selected at a time.

    The most straight-forward way to use ActionGroup is to declare actions
    as children of the group.

    \code
    ActionGroup {
        id: alignmentGroup

        Action {
            checked: true
            checkable: true
            text: qsTr("Left")
        }

        Action {
            checkable: true
            text: qsTr("Center")
        }

        Action {
            checkable: true
            text: qsTr("Right")
        }
    }
    \endcode

    Alternatively, the \l group attached property allows declaring the actions
    elsewhere and assigning them to a specific group.

    \code
    ActionGroup { id: alignmentGroup }

    Action {
        checked: true
        checkable: true
        text: qsTr("Left")
        ActionGroup.group: alignmentGroup
    }

    Action {
        checkable: true
        text: qsTr("Center")
        ActionGroup.group: alignmentGroup
    }

    Action {
        checkable: true
        text: qsTr("Right")
        ActionGroup.group: alignmentGroup
    }
    \endcode

    More advanced use cases can be handled using the \c addAction() and
    \c removeAction() methods.

    \sa Action, ButtonGroup
*/

/*!
    \qmlsignal QtQuick.Controls::ActionGroup::triggered(Action action)

    This signal is emitted when an \a action in the group has been triggered.

    This signal is convenient for implementing a common signal handler for
    all actions in the same group.

    \code
    ActionGroup {
        onTriggered: console.log("triggered:", action.text)

        Action { text: "First" }
        Action { text: "Second" }
        Action { text: "Third" }
    }
    \endcode

    \sa Action::triggered()
*/

class QQuickActionGroupPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QQuickActionGroup)

public:
    void clear();
    void actionTriggered();
    void _q_updateCurrent();

    static bool changeEnabled(QQuickAction *action, bool enabled);

    static void actions_append(QQmlListProperty<QQuickAction> *prop, QQuickAction *obj);
    static int actions_count(QQmlListProperty<QQuickAction> *prop);
    static QQuickAction *actions_at(QQmlListProperty<QQuickAction> *prop, int index);
    static void actions_clear(QQmlListProperty<QQuickAction> *prop);

    bool enabled = true;
    bool exclusive = true;
    QPointer<QQuickAction> checkedAction;
    QVector<QQuickAction*> actions;
};

void QQuickActionGroupPrivate::clear()
{
    for (QQuickAction *action : qAsConst(actions)) {
        QQuickActionPrivate::get(action)->group = nullptr;
        QObjectPrivate::disconnect(action, &QQuickAction::triggered, this, &QQuickActionGroupPrivate::actionTriggered);
        QObjectPrivate::disconnect(action, &QQuickAction::checkedChanged, this, &QQuickActionGroupPrivate::_q_updateCurrent);
    }
    actions.clear();
}

void QQuickActionGroupPrivate::actionTriggered()
{
    Q_Q(QQuickActionGroup);
    QQuickAction *action = qobject_cast<QQuickAction*>(q->sender());
    if (action)
        emit q->triggered(action);
}

void QQuickActionGroupPrivate::_q_updateCurrent()
{
    Q_Q(QQuickActionGroup);
    if (!exclusive)
        return;
    QQuickAction *action = qobject_cast<QQuickAction*>(q->sender());
    if (action && action->isChecked())
        q->setCheckedAction(action);
    else if (!actions.contains(checkedAction))
        q->setCheckedAction(nullptr);
}

bool QQuickActionGroupPrivate::changeEnabled(QQuickAction *action, bool enabled)
{
    return action->isEnabled() != enabled && (!enabled || !QQuickActionPrivate::get(action)->explicitEnabled);
}

void QQuickActionGroupPrivate::actions_append(QQmlListProperty<QQuickAction> *prop, QQuickAction *obj)
{
    QQuickActionGroup *q = static_cast<QQuickActionGroup *>(prop->object);
    q->addAction(obj);
}

int QQuickActionGroupPrivate::actions_count(QQmlListProperty<QQuickAction> *prop)
{
    QQuickActionGroupPrivate *p = static_cast<QQuickActionGroupPrivate *>(prop->data);
    return p->actions.count();
}

QQuickAction *QQuickActionGroupPrivate::actions_at(QQmlListProperty<QQuickAction> *prop, int index)
{
    QQuickActionGroupPrivate *p = static_cast<QQuickActionGroupPrivate *>(prop->data);
    return p->actions.value(index);
}

void QQuickActionGroupPrivate::actions_clear(QQmlListProperty<QQuickAction> *prop)
{
    QQuickActionGroupPrivate *p = static_cast<QQuickActionGroupPrivate *>(prop->data);
    if (!p->actions.isEmpty()) {
        p->clear();
        QQuickActionGroup *q = static_cast<QQuickActionGroup *>(prop->object);
        // QTBUG-52358: don't clear the checked action immediately
        QMetaObject::invokeMethod(q, "_q_updateCurrent", Qt::QueuedConnection);
        emit q->actionsChanged();
    }
}

QQuickActionGroup::QQuickActionGroup(QObject *parent)
    : QObject(*(new QQuickActionGroupPrivate), parent)
{
}

QQuickActionGroup::~QQuickActionGroup()
{
    Q_D(QQuickActionGroup);
    d->clear();
}

QQuickActionGroupAttached *QQuickActionGroup::qmlAttachedProperties(QObject *object)
{
    return new QQuickActionGroupAttached(object);
}

/*!
    \qmlproperty Action QtQuick.Controls::ActionGroup::checkedAction

    This property holds the currently selected action in an exclusive group,
    or \c null if there is none or the group is non-exclusive.

    By default, it is the first checked action added to an exclusive action group.

    \sa exclusive
*/
QQuickAction *QQuickActionGroup::checkedAction() const
{
    Q_D(const QQuickActionGroup);
    return d->checkedAction;
}

void QQuickActionGroup::setCheckedAction(QQuickAction *checkedAction)
{
    Q_D(QQuickActionGroup);
    if (d->checkedAction == checkedAction)
        return;

    if (d->checkedAction)
        d->checkedAction->setChecked(false);
    d->checkedAction = checkedAction;
    if (checkedAction)
        checkedAction->setChecked(true);
    emit checkedActionChanged();
}

/*!
    \qmlproperty list<Action> QtQuick.Controls::ActionGroup::actions
    \default

    This property holds the list of actions in the group.

    \sa group
*/
QQmlListProperty<QQuickAction> QQuickActionGroup::actions()
{
    Q_D(QQuickActionGroup);
    return QQmlListProperty<QQuickAction>(this, d,
        QQuickActionGroupPrivate::actions_append,
        QQuickActionGroupPrivate::actions_count,
        QQuickActionGroupPrivate::actions_at,
        QQuickActionGroupPrivate::actions_clear);
}

/*!
    \qmlproperty bool QtQuick.Controls::ActionGroup::exclusive

    This property holds whether the action group is exclusive. The default value is \c true.

    If this property is \c true, then only one action in the group can be checked at any given time.
    The user can trigger any action to check it, and that action will replace the existing one as
    the checked action in the group.

    In an exclusive group, the user cannot uncheck the currently checked action by triggering it;
    instead, another action in the group must be triggered to set the new checked action for that
    group.

    In a non-exclusive group, checking and unchecking actions does not affect the other actions in
    the group. Furthermore, the value of the \l checkedAction property is \c null.
*/
bool QQuickActionGroup::isExclusive() const
{
    Q_D(const QQuickActionGroup);
    return d->exclusive;
}

void QQuickActionGroup::setExclusive(bool exclusive)
{
    Q_D(QQuickActionGroup);
    if (d->exclusive == exclusive)
        return;

    d->exclusive = exclusive;
    emit exclusiveChanged();
}

/*!
    \qmlproperty bool QtQuick.Controls::ActionGroup::enabled

    This property holds whether the action group is enabled. The default value is \c true.

    If this property is \c false, then all actions in the group are disabled. If this property
    is \c true, all actions in the group are enabled, unless explicitly disabled.
*/
bool QQuickActionGroup::isEnabled() const
{
    Q_D(const QQuickActionGroup);
    return d->enabled;
}

void QQuickActionGroup::setEnabled(bool enabled)
{
    Q_D(QQuickActionGroup);
    if (d->enabled == enabled)
        return;

    for (QQuickAction *action : qAsConst(d->actions)) {
        if (d->changeEnabled(action, enabled))
            emit action->enabledChanged(enabled);
    }

    d->enabled = enabled;
    emit enabledChanged();
}

/*!
    \qmlmethod void QtQuick.Controls::ActionGroup::addAction(Action action)

    Adds an \a action to the action group.

    \note Manually adding objects to a action group is typically unnecessary.
          The \l actions property and the \l group attached property provide a
          convenient and declarative syntax.

    \sa actions, group
*/
void QQuickActionGroup::addAction(QQuickAction *action)
{
    Q_D(QQuickActionGroup);
    if (!action || d->actions.contains(action))
        return;

    const bool enabledChange = d->changeEnabled(action, d->enabled);

    QQuickActionPrivate::get(action)->group = this;
    QObjectPrivate::connect(action, &QQuickAction::triggered, d, &QQuickActionGroupPrivate::actionTriggered);
    QObjectPrivate::connect(action, &QQuickAction::checkedChanged, d, &QQuickActionGroupPrivate::_q_updateCurrent);

    if (d->exclusive && action->isChecked())
        setCheckedAction(action);
    if (enabledChange)
        emit action->enabledChanged(action->isEnabled());

    d->actions.append(action);
    emit actionsChanged();
}

/*!
    \qmlmethod void QtQuick.Controls::ActionGroup::removeAction(Action action)

    Removes an \a action from the action group.

    \note Manually removing objects from a action group is typically unnecessary.
          The \l actions property and the \l group attached property provide a
          convenient and declarative syntax.

    \sa actions, group
*/
void QQuickActionGroup::removeAction(QQuickAction *action)
{
    Q_D(QQuickActionGroup);
    if (!action || !d->actions.contains(action))
        return;

    const bool enabledChange = d->changeEnabled(action, d->enabled);

    QQuickActionPrivate::get(action)->group = nullptr;
    QObjectPrivate::disconnect(action, &QQuickAction::triggered, d, &QQuickActionGroupPrivate::actionTriggered);
    QObjectPrivate::disconnect(action, &QQuickAction::checkedChanged, d, &QQuickActionGroupPrivate::_q_updateCurrent);

    if (d->checkedAction == action)
        setCheckedAction(nullptr);
    if (enabledChange)
        emit action->enabledChanged(action->isEnabled());

    d->actions.removeOne(action);
    emit actionsChanged();
}

class QQuickActionGroupAttachedPrivate : public QObjectPrivate
{
public:
    QQuickActionGroup *group = nullptr;
};

QQuickActionGroupAttached::QQuickActionGroupAttached(QObject *parent)
    : QObject(*(new QQuickActionGroupAttachedPrivate), parent)
{
}

/*!
    \qmlattachedproperty ActionGroup QtQuick.Controls::ActionGroup::group

    This property attaches an action to an action group.

    \code
    ActionGroup { id: group }

    Action {
        checked: true
        text: qsTr("Option A")
        ActionGroup.group: group
    }

    Action {
        text: qsTr("Option B")
        ActionGroup.group: group
    }
    \endcode

    \sa actions
*/
QQuickActionGroup *QQuickActionGroupAttached::group() const
{
    Q_D(const QQuickActionGroupAttached);
    return d->group;
}

void QQuickActionGroupAttached::setGroup(QQuickActionGroup *group)
{
    Q_D(QQuickActionGroupAttached);
    if (d->group == group)
        return;

    if (d->group)
        d->group->removeAction(qobject_cast<QQuickAction*>(parent()));
    d->group = group;
    if (group)
        group->addAction(qobject_cast<QQuickAction*>(parent()));
    emit groupChanged();
}

QT_END_NAMESPACE

#include "moc_qquickactiongroup_p.cpp"
