/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#include "qqmldelegatecomponent_p.h"
#include <QtQml/private/qqmladaptormodel_p.h>

QT_BEGIN_NAMESPACE

QQmlAbstractDelegateComponent::QQmlAbstractDelegateComponent(QObject *parent)
    : QQmlComponent(parent)
{
}

QQmlAbstractDelegateComponent::~QQmlAbstractDelegateComponent()
{
}

QVariant QQmlAbstractDelegateComponent::value(QQmlAdaptorModel *adaptorModel, int row, int column, const QString &role) const
{
    if (!adaptorModel)
        return QVariant();
    return adaptorModel->value(adaptorModel->indexAt(row, column), role);
}

/*!
    \qmlmodule Qt.labs.qmlmodels 1.0
    \title Qt Labs QML Models - QML Types
    \ingroup qmlmodules
    \brief The Qt Labs QML Models module provides various model-related types for use with views.

    To use this module, import the module with the following line:

    \qml
    import Qt.labs.qmlmodels 1.0
    \endqml
*/

/*!
    \qmltype DelegateChoice
    \instantiates QQmlDelegateChoice
    \inqmlmodule Qt.labs.qmlmodels
    \brief Encapsulates a delegate and when to use it.

    The DelegateChoice type wraps a delegate and defines the circumstances
    in which it should be chosen.

    DelegateChoices can be nested inside a DelegateChooser.

    \sa DelegateChooser
*/

/*!
    \qmlproperty string QtQml.Models::DelegateChoice::roleValue
    This property holds the value used to match the role data for the role provided by \l DelegateChooser::role.
*/
QVariant QQmlDelegateChoice::roleValue() const
{
    return m_value;
}

void QQmlDelegateChoice::setRoleValue(const QVariant &value)
{
    if (m_value == value)
        return;
    m_value = value;
    emit roleValueChanged();
    emit changed();
}

/*!
    \qmlproperty index QtQml.Models::DelegateChoice::row
    This property holds the value used to match the row value of model elements.
    With models that have only the index property (and thus only one column), this property
    should be intended as an index, and set to the desired index value.

    \note Setting both row and index has undefined behavior. The two are equivalent and only
    one should be used.

    \sa index
*/

/*!
    \qmlproperty index QtQml.Models::DelegateChoice::index
    This property holds the value used to match the index value of model elements.
    This is effectively an alias for \l row.

    \sa row
*/
int QQmlDelegateChoice::row() const
{
    return m_row;
}

void QQmlDelegateChoice::setRow(int r)
{
    if (m_row == r)
        return;
    m_row = r;
    emit rowChanged();
    emit indexChanged();
    emit changed();
}

/*!
    \qmlproperty index QtQml.Models::DelegateChoice::column
    This property holds the value used to match the column value of model elements.
*/
int QQmlDelegateChoice::column() const
{
    return m_column;
}

void QQmlDelegateChoice::setColumn(int c)
{
    if (m_column == c)
        return;
    m_column = c;
    emit columnChanged();
    emit changed();
}

QQmlComponent *QQmlDelegateChoice::delegate() const
{
    return m_delegate;
}

/*!
    \qmlproperty Component QtQml.Models::DelegateChoice::delegate
    This property holds the delegate to use if this choice matches the model item.
*/
void QQmlDelegateChoice::setDelegate(QQmlComponent *delegate)
{
    if (m_delegate == delegate)
        return;
    QQmlAbstractDelegateComponent *adc = static_cast<QQmlAbstractDelegateComponent *>(m_delegate);
    if (adc)
        disconnect(adc, &QQmlAbstractDelegateComponent::delegateChanged, this, &QQmlDelegateChoice::delegateChanged);
    m_delegate = delegate;
    adc = static_cast<QQmlAbstractDelegateComponent *>(delegate);
    if (adc)
        connect(adc, &QQmlAbstractDelegateComponent::delegateChanged, this, &QQmlDelegateChoice::delegateChanged);
    emit delegateChanged();
    emit changed();
}

bool QQmlDelegateChoice::match(int row, int column, const QVariant &value) const
{
    if (!m_value.isValid() && m_row < 0 && m_column < 0)
        return true;

    const bool roleMatched = (m_value.isValid()) ? value == m_value : true;
    const bool rowMatched = (m_row < 0 ) ? true : m_row == row;
    const bool columnMatched = (m_column < 0 ) ? true : m_column == column;
    return roleMatched && rowMatched && columnMatched;
}

/*!
    \qmltype DelegateChooser
    \instantiates QQmlDelegateChooser
    \inqmlmodule Qt.labs.qmlmodels
    \brief Allows a view to use different delegates for different types of items in the model.

    The DelegateChooser is a special \l Component type intended for those scenarios where a Component is required
    by a view and used as a delegate.
    DelegateChooser encapsulates a set of \l {DelegateChoice}s.
    These choices are used to determine the delegate that will be instantiated for each
    item in the model.
    The selection of the choice is performed based on the value that a model item has for \l role,
    and also based on index.

    DelegateChooser is commonly used when a view needs to display a set of delegates that are significantly
    different from each other. For example, a typical phone settings view might include toggle switches,
    sliders, radio buttons, and other visualizations based on the type of each setting. In this case, DelegateChooser
    could provide an easy way to associate a different type of delegate with each setting:

    \qml \QtMinorVersion
    import QtQuick 2.\1
    import QtQuick.Controls 2.\1
    import Qt.labs.qmlmodels 1.0

    ListView {
        width: 200; height: 400

        ListModel {
            id: listModel
            ListElement { type: "info"; ... }
            ListElement { type: "switch"; ... }
            ListElement { type: "swipe"; ... }
            ListElement { type: "switch"; ... }
        }

        DelegateChooser {
            id: chooser
            role: "type"
            DelegateChoice { roleValue: "info"; ItemDelegate { ... } }
            DelegateChoice { roleValue: "switch"; SwitchDelegate { ... } }
            DelegateChoice { roleValue: "swipe"; SwipeDelegate { ... } }
        }

        model: listModel
        delegate: chooser
    }
    \endqml

    \note This type is intended to transparently work only with TableView and any DelegateModel-based view.
    Views (including user-defined views) that aren't internally based on a DelegateModel need to explicitly support
    this type of component to make it function as described.

    \sa DelegateChoice
*/

/*!
    \qmlproperty string QtQml.Models::DelegateChooser::role
    This property holds the role used to determine the delegate for a given model item.

    \sa DelegateChoice
*/
void QQmlDelegateChooser::setRole(const QString &role)
{
    if (m_role == role)
        return;
    m_role = role;
    emit roleChanged();
}

/*!
    \qmlproperty list<DelegateChoice> QtQml.Models::DelegateChooser::choices
    \default

    The list of DelegateChoices for the chooser.

    The list is treated as an ordered list, where the first DelegateChoice to match
    will be used be a view.

    It should not generally be necessary to refer to the \c choices property,
    as it is the default property for DelegateChooser and thus all child items are
    automatically assigned to this property.
*/

QQmlListProperty<QQmlDelegateChoice> QQmlDelegateChooser::choices()
{
    return QQmlListProperty<QQmlDelegateChoice>(this, nullptr,
                                                QQmlDelegateChooser::choices_append,
                                                QQmlDelegateChooser::choices_count,
                                                QQmlDelegateChooser::choices_at,
                                                QQmlDelegateChooser::choices_clear);
}

void QQmlDelegateChooser::choices_append(QQmlListProperty<QQmlDelegateChoice> *prop, QQmlDelegateChoice *choice)
{
    QQmlDelegateChooser *q = static_cast<QQmlDelegateChooser *>(prop->object);
    q->m_choices.append(choice);
    connect(choice, &QQmlDelegateChoice::changed, q, &QQmlAbstractDelegateComponent::delegateChanged);
    q->delegateChanged();
}

int QQmlDelegateChooser::choices_count(QQmlListProperty<QQmlDelegateChoice> *prop)
{
    QQmlDelegateChooser *q = static_cast<QQmlDelegateChooser*>(prop->object);
    return q->m_choices.count();
}

QQmlDelegateChoice *QQmlDelegateChooser::choices_at(QQmlListProperty<QQmlDelegateChoice> *prop, int index)
{
    QQmlDelegateChooser *q = static_cast<QQmlDelegateChooser*>(prop->object);
    return q->m_choices.at(index);
}

void QQmlDelegateChooser::choices_clear(QQmlListProperty<QQmlDelegateChoice> *prop)
{
    QQmlDelegateChooser *q = static_cast<QQmlDelegateChooser *>(prop->object);
    for (QQmlDelegateChoice *choice : q->m_choices)
        disconnect(choice, &QQmlDelegateChoice::changed, q, &QQmlAbstractDelegateComponent::delegateChanged);
    q->m_choices.clear();
    q->delegateChanged();
}

QQmlComponent *QQmlDelegateChooser::delegate(QQmlAdaptorModel *adaptorModel, int row, int column) const
{
    QVariant v;
    if (!m_role.isNull())
        v = value(adaptorModel, row, column, m_role);
    if (!v.isValid()) { // check if the row only has modelData, for example if the row is a QVariantMap
        v = value(adaptorModel, row, column, QStringLiteral("modelData"));
        if (v.isValid())
            v = v.toMap().value(m_role);
    }
    // loop through choices, finding first one that fits
    for (int i = 0; i < m_choices.count(); ++i) {
        const QQmlDelegateChoice *choice = m_choices.at(i);
        if (choice->match(row, column, v))
            return choice->delegate();
    }

    return nullptr;
}

QT_END_NAMESPACE
