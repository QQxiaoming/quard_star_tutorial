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

#include "qquickbuttongroup_p.h"

#include <QtCore/private/qobject_p.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qvariant.h>
#include <QtQml/qqmlinfo.h>

#include "qquickabstractbutton_p_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype ButtonGroup
    \inherits QtObject
    \instantiates QQuickButtonGroup
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup utilities
    \brief Mutually-exclusive group of checkable buttons.

    ButtonGroup is a non-visual, mutually exclusive group of buttons.
    It is used with controls such as RadioButton, where only one of the options
    can be selected at a time.

    The most straight-forward way to use ButtonGroup is to assign
    a list of buttons. For example, the list of children of a
    \l{Item Positioners}{positioner} or a \l{Qt Quick Layouts}{layout}
    that manages a group of mutually exclusive buttons.

    \code
    ButtonGroup {
        buttons: column.children
    }

    Column {
        id: column

        RadioButton {
            checked: true
            text: qsTr("DAB")
        }

        RadioButton {
            text: qsTr("FM")
        }

        RadioButton {
            text: qsTr("AM")
        }
    }
    \endcode

    Mutually exclusive buttons do not always share the same parent item,
    or the parent layout may sometimes contain items that should not be
    included in the button group. Such cases are best handled using
    the \l group attached property.

    \code
    ButtonGroup { id: radioGroup }

    Column {
        Label {
            text: qsTr("Radio:")
        }

        RadioButton {
            checked: true
            text: qsTr("DAB")
            ButtonGroup.group: radioGroup
        }

        RadioButton {
            text: qsTr("FM")
            ButtonGroup.group: radioGroup
        }

        RadioButton {
            text: qsTr("AM")
            ButtonGroup.group: radioGroup
        }
    }
    \endcode

    More advanced use cases can be handled using the \c addButton() and
    \c removeButton() methods.

    \sa RadioButton, {Button Controls}
*/

/*!
    \qmlsignal QtQuick.Controls::ButtonGroup::clicked(AbstractButton button)
    \since QtQuick.Controls 2.1 (Qt 5.8)

    This signal is emitted when a \a button in the group has been clicked.

    This signal is convenient for implementing a common signal handler for
    all buttons in the same group.

    \code
    ButtonGroup {
        buttons: column.children
        onClicked: console.log("clicked:", button.text)
    }

    Column {
        id: column
        Button { text: "First" }
        Button { text: "Second" }
        Button { text: "Third" }
    }
    \endcode

    \sa AbstractButton::clicked()
*/

class QQuickButtonGroupPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QQuickButtonGroup)

public:
    void clear();
    void buttonClicked();
    void _q_updateCurrent();
    void updateCheckState();
    void setCheckState(Qt::CheckState state);

    static void buttons_append(QQmlListProperty<QQuickAbstractButton> *prop, QQuickAbstractButton *obj);
    static int buttons_count(QQmlListProperty<QQuickAbstractButton> *prop);
    static QQuickAbstractButton *buttons_at(QQmlListProperty<QQuickAbstractButton> *prop, int index);
    static void buttons_clear(QQmlListProperty<QQuickAbstractButton> *prop);

    bool complete = true;
    bool exclusive = true;
    bool settingCheckState = false;
    Qt::CheckState checkState = Qt::Unchecked;
    QPointer<QQuickAbstractButton> checkedButton;
    QVector<QQuickAbstractButton*> buttons;
};

void QQuickButtonGroupPrivate::clear()
{
    for (QQuickAbstractButton *button : qAsConst(buttons)) {
        QQuickAbstractButtonPrivate::get(button)->group = nullptr;
        QObjectPrivate::disconnect(button, &QQuickAbstractButton::clicked, this, &QQuickButtonGroupPrivate::buttonClicked);
        QObjectPrivate::disconnect(button, &QQuickAbstractButton::checkedChanged, this, &QQuickButtonGroupPrivate::_q_updateCurrent);
    }
    buttons.clear();
}

void QQuickButtonGroupPrivate::buttonClicked()
{
    Q_Q(QQuickButtonGroup);
    QQuickAbstractButton *button = qobject_cast<QQuickAbstractButton*>(q->sender());
    if (button)
        emit q->clicked(button);
}

void QQuickButtonGroupPrivate::_q_updateCurrent()
{
    Q_Q(QQuickButtonGroup);
    if (exclusive) {
        QQuickAbstractButton *button = qobject_cast<QQuickAbstractButton*>(q->sender());
        if (button && button->isChecked())
            q->setCheckedButton(button);
        else if (!buttons.contains(checkedButton))
            q->setCheckedButton(nullptr);
    }
    updateCheckState();
}

void QQuickButtonGroupPrivate::updateCheckState()
{
    if (!complete || settingCheckState)
        return;

    bool anyChecked = false;
    bool allChecked = !buttons.isEmpty();
    for (QQuickAbstractButton *button : qAsConst(buttons)) {
        const bool isChecked = button->isChecked();
        anyChecked |= isChecked;
        allChecked &= isChecked;
    }
    setCheckState(Qt::CheckState(anyChecked + allChecked));
}

void QQuickButtonGroupPrivate::setCheckState(Qt::CheckState state)
{
    Q_Q(QQuickButtonGroup);
    if (checkState == state)
        return;

    checkState = state;
    emit q->checkStateChanged();
}

void QQuickButtonGroupPrivate::buttons_append(QQmlListProperty<QQuickAbstractButton> *prop, QQuickAbstractButton *obj)
{
    QQuickButtonGroup *q = static_cast<QQuickButtonGroup *>(prop->object);
    q->addButton(obj);
}

int QQuickButtonGroupPrivate::buttons_count(QQmlListProperty<QQuickAbstractButton> *prop)
{
    QQuickButtonGroupPrivate *p = static_cast<QQuickButtonGroupPrivate *>(prop->data);
    return p->buttons.count();
}

QQuickAbstractButton *QQuickButtonGroupPrivate::buttons_at(QQmlListProperty<QQuickAbstractButton> *prop, int index)
{
    QQuickButtonGroupPrivate *p = static_cast<QQuickButtonGroupPrivate *>(prop->data);
    return p->buttons.value(index);
}

void QQuickButtonGroupPrivate::buttons_clear(QQmlListProperty<QQuickAbstractButton> *prop)
{
    QQuickButtonGroupPrivate *p = static_cast<QQuickButtonGroupPrivate *>(prop->data);
    if (!p->buttons.isEmpty()) {
        p->clear();
        QQuickButtonGroup *q = static_cast<QQuickButtonGroup *>(prop->object);
        // QTBUG-52358: don't clear the checked button immediately
        QMetaObject::invokeMethod(q, "_q_updateCurrent", Qt::QueuedConnection);
        emit q->buttonsChanged();
    }
}

QQuickButtonGroup::QQuickButtonGroup(QObject *parent)
    : QObject(*(new QQuickButtonGroupPrivate), parent)
{
}

QQuickButtonGroup::~QQuickButtonGroup()
{
    Q_D(QQuickButtonGroup);
    d->clear();
}

QQuickButtonGroupAttached *QQuickButtonGroup::qmlAttachedProperties(QObject *object)
{
    return new QQuickButtonGroupAttached(object);
}

/*!
    \qmlproperty AbstractButton QtQuick.Controls::ButtonGroup::checkedButton

    This property holds the currently selected button in an exclusive group,
    or \c null if there is none or the group is non-exclusive.

    By default, it is the first checked button added to an exclusive button group.

    \sa exclusive
*/
QQuickAbstractButton *QQuickButtonGroup::checkedButton() const
{
    Q_D(const QQuickButtonGroup);
    return d->checkedButton;
}

void QQuickButtonGroup::setCheckedButton(QQuickAbstractButton *checkedButton)
{
    Q_D(QQuickButtonGroup);
    if (d->checkedButton == checkedButton)
        return;

    if (d->checkedButton)
        d->checkedButton->setChecked(false);
    d->checkedButton = checkedButton;
    if (checkedButton)
        checkedButton->setChecked(true);
    emit checkedButtonChanged();
}

/*!
    \qmlproperty list<AbstractButton> QtQuick.Controls::ButtonGroup::buttons
    \default

    This property holds the list of buttons.

    \code
    ButtonGroup {
        buttons: column.children
    }

    Column {
        id: column

        RadioButton {
            checked: true
            text: qsTr("Option A")
        }

        RadioButton {
            text: qsTr("Option B")
        }
    }
    \endcode

    \sa group
*/
QQmlListProperty<QQuickAbstractButton> QQuickButtonGroup::buttons()
{
    Q_D(QQuickButtonGroup);
    return QQmlListProperty<QQuickAbstractButton>(this, d,
        QQuickButtonGroupPrivate::buttons_append,
        QQuickButtonGroupPrivate::buttons_count,
        QQuickButtonGroupPrivate::buttons_at,
        QQuickButtonGroupPrivate::buttons_clear);
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty bool QtQuick.Controls::ButtonGroup::exclusive

    This property holds whether the button group is exclusive. The default value is \c true.

    If this property is \c true, then only one button in the group can be checked at any given time.
    The user can click on any button to check it, and that button will replace the existing one as
    the checked button in the group.

    In an exclusive group, the user cannot uncheck the currently checked button by clicking on it;
    instead, another button in the group must be clicked to set the new checked button for that group.

    In a non-exclusive group, checking and unchecking buttons does not affect the other buttons in
    the group. Furthermore, the value of the \l checkedButton property is \c null.
*/
bool QQuickButtonGroup::isExclusive() const
{
    Q_D(const QQuickButtonGroup);
    return d->exclusive;
}

void QQuickButtonGroup::setExclusive(bool exclusive)
{
    Q_D(QQuickButtonGroup);
    if (d->exclusive == exclusive)
        return;

    d->exclusive = exclusive;
    emit exclusiveChanged();
}

/*!
    \since QtQuick.Controls 2.4 (Qt 5.11)
    \qmlproperty enumeration QtQuick.Controls::ButtonGroup::checkState

    This property holds the combined check state of the button group.

    Available states:
    \value Qt.Unchecked None of the buttons are checked.
    \value Qt.PartiallyChecked Some of the buttons are checked.
    \value Qt.Checked All of the buttons are checked.

    Setting the check state of a non-exclusive button group to \c Qt.Unchecked
    or \c Qt.Checked unchecks or checks all buttons in the group, respectively.
    \c Qt.PartiallyChecked is ignored.

    Setting the check state of an exclusive button group to \c Qt.Unchecked
    unchecks the \l checkedButton. \c Qt.Checked and \c Qt.PartiallyChecked
    are ignored.
*/
Qt::CheckState QQuickButtonGroup::checkState() const
{
    Q_D(const QQuickButtonGroup);
    return d->checkState;
}

void QQuickButtonGroup::setCheckState(Qt::CheckState state)
{
    Q_D(QQuickButtonGroup);
    if (d->checkState == state || state == Qt::PartiallyChecked)
        return;

    d->settingCheckState = true;
    if (d->exclusive) {
        if (d->checkedButton && state == Qt::Unchecked)
            setCheckedButton(nullptr);
    } else {
        for (QQuickAbstractButton *button : qAsConst(d->buttons))
            button->setChecked(state == Qt::Checked);
    }
    d->settingCheckState = false;
    d->setCheckState(state);
}

/*!
    \qmlmethod void QtQuick.Controls::ButtonGroup::addButton(AbstractButton button)

    Adds a \a button to the button group.

    \note Manually adding objects to a button group is typically unnecessary.
          The \l buttons property and the \l group attached property provide a
          convenient and declarative syntax.

    \sa buttons, group
*/
void QQuickButtonGroup::addButton(QQuickAbstractButton *button)
{
    Q_D(QQuickButtonGroup);
    if (!button || d->buttons.contains(button))
        return;

    QQuickAbstractButtonPrivate::get(button)->group = this;
    QObjectPrivate::connect(button, &QQuickAbstractButton::clicked, d, &QQuickButtonGroupPrivate::buttonClicked);
    QObjectPrivate::connect(button, &QQuickAbstractButton::checkedChanged, d, &QQuickButtonGroupPrivate::_q_updateCurrent);

    if (d->exclusive && button->isChecked())
        setCheckedButton(button);

    d->buttons.append(button);
    d->updateCheckState();
    emit buttonsChanged();
}

/*!
    \qmlmethod void QtQuick.Controls::ButtonGroup::removeButton(AbstractButton button)

    Removes a \a button from the button group.

    \note Manually removing objects from a button group is typically unnecessary.
          The \l buttons property and the \l group attached property provide a
          convenient and declarative syntax.

    \sa buttons, group
*/
void QQuickButtonGroup::removeButton(QQuickAbstractButton *button)
{
    Q_D(QQuickButtonGroup);
    if (!button || !d->buttons.contains(button))
        return;

    QQuickAbstractButtonPrivate::get(button)->group = nullptr;
    QObjectPrivate::disconnect(button, &QQuickAbstractButton::clicked, d, &QQuickButtonGroupPrivate::buttonClicked);
    QObjectPrivate::disconnect(button, &QQuickAbstractButton::checkedChanged, d, &QQuickButtonGroupPrivate::_q_updateCurrent);

    if (d->checkedButton == button)
        setCheckedButton(nullptr);

    d->buttons.removeOne(button);
    d->updateCheckState();
    emit buttonsChanged();
}

void QQuickButtonGroup::classBegin()
{
    Q_D(QQuickButtonGroup);
    d->complete = false;
}

void QQuickButtonGroup::componentComplete()
{
    Q_D(QQuickButtonGroup);
    d->complete = true;
    if (!d->buttons.isEmpty())
        d->updateCheckState();
}

class QQuickButtonGroupAttachedPrivate : public QObjectPrivate
{
public:
    QQuickButtonGroup *group = nullptr;
};

QQuickButtonGroupAttached::QQuickButtonGroupAttached(QObject *parent)
    : QObject(*(new QQuickButtonGroupAttachedPrivate), parent)
{
}

/*!
    \qmlattachedproperty ButtonGroup QtQuick.Controls::ButtonGroup::group

    This property attaches a button to a button group.

    \code
    ButtonGroup { id: group }

    RadioButton {
        checked: true
        text: qsTr("Option A")
        ButtonGroup.group: group
    }

    RadioButton {
        text: qsTr("Option B")
        ButtonGroup.group: group
    }
    \endcode

    \sa buttons
*/
QQuickButtonGroup *QQuickButtonGroupAttached::group() const
{
    Q_D(const QQuickButtonGroupAttached);
    return d->group;
}

void QQuickButtonGroupAttached::setGroup(QQuickButtonGroup *group)
{
    Q_D(QQuickButtonGroupAttached);
    if (d->group == group)
        return;

    if (d->group)
        d->group->removeButton(qobject_cast<QQuickAbstractButton*>(parent()));
    d->group = group;
    if (group)
        group->addButton(qobject_cast<QQuickAbstractButton*>(parent()));
    emit groupChanged();
}

QT_END_NAMESPACE

#include "moc_qquickbuttongroup_p.cpp"
