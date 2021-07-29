/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include "qnodecommand.h"
#include "qnodecommand_p.h"
#include <Qt3DCore/qnode.h>
#include <Qt3DCore/private/qnode_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DCore {

QNodeCommandPrivate::QNodeCommandPrivate()
    : QSceneChangePrivate()
    , m_commandId(createId())
    , m_replyToCommandId()
{
}

QNodeCommandPrivate::~QNodeCommandPrivate()
{
}

QNodeCommand::CommandId QNodeCommandPrivate::createId()
{
    static QBasicAtomicInteger<QNodeCommand::CommandId> next = Q_BASIC_ATOMIC_INITIALIZER(0);
    return next.fetchAndAddRelaxed(1) + 1;
}

/*!
 * \class Qt3DCore::QNodeCommand
 * \inheaderfile Qt3DCore/QNodeCommand
 * \inherits Qt3DCore::QSceneChange
 * \inmodule Qt3DCore
 * \since 5.10
 * \brief The QNodeCommand class is the base class for all CommandRequested QSceneChange events.
 *
 * The QNodeCommand class is the base class for all QSceneChange events that
 * have the changeType() CommandRequested.
 *
 * You can subclass this to create your own node update types for communication between
 * your QNode and QBackendNode subclasses when writing your own aspects.
 */

/*!
 * \typedef Qt3DCore::QNodeCommandPtr
 * \relates Qt3DCore::QNodeCommand
 *
 * A shared pointer for QNodeCommand.
 */

/*!
 * \typedef QNodeCommand::CommandId
 *
 * Type of the command id, defined either as quint64 or quint32
 * depending on the platform support.
 */

/*!
 * Constructs a new QNodeCommand with \a id.
 */
QNodeCommand::QNodeCommand(QNodeId id)
    : QSceneChange(*new QNodeCommandPrivate(), CommandRequested, id)
{
}

QNodeCommand::QNodeCommand(QNodeCommandPrivate &dd, QNodeId id)
    : QSceneChange(dd, CommandRequested, id)
{
}

QNodeCommand::~QNodeCommand()
{
}

/*!
 * \return commandId.
 */
QNodeCommand::CommandId QNodeCommand::commandId() const
{
    Q_D(const QNodeCommand);
    return d->m_commandId;
}

/*!
 * \return name.
 */
QString QNodeCommand::name() const
{
    Q_D(const QNodeCommand);
    return d->m_name;
}

void QNodeCommand::setName(const QString &name)
{
    Q_D(QNodeCommand);
    d->m_name = name;
}

/*!
 * \return data.
 */
QVariant QNodeCommand::data() const
{
    Q_D(const QNodeCommand);
    return d->m_data;
}

void QNodeCommand::setData(const QVariant &data)
{
    Q_D(QNodeCommand);
    d->m_data = data;
}

QNodeCommand::CommandId QNodeCommand::inReplyTo() const
{
    Q_D(const QNodeCommand);
    return d->m_replyToCommandId;
}

void QNodeCommand::setReplyToCommandId(QNodeCommand::CommandId id)
{
    Q_D(QNodeCommand);
    d->m_replyToCommandId = id;
}

} // namespace Qt3DCore

QT_END_NAMESPACE
