/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSCriptTools module of the Qt Toolkit.
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

#include "qscriptdebuggerconsolecommand_p.h"
#include "qscriptdebuggerconsolecommand_p_p.h"

#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

/*!
  \since 4.5
  \class QScriptDebuggerConsoleCommand
  \internal

  \brief The QScriptDebuggerConsoleCommand class is the base class of console commands.

  \sa QScriptDebuggerConsoleCommandManager
*/

QScriptDebuggerConsoleCommandPrivate::QScriptDebuggerConsoleCommandPrivate()
{
}

QScriptDebuggerConsoleCommandPrivate::~QScriptDebuggerConsoleCommandPrivate()
{
}

QScriptDebuggerConsoleCommand::QScriptDebuggerConsoleCommand()
    : d_ptr(new QScriptDebuggerConsoleCommandPrivate)
{
    d_ptr->q_ptr = this;
}

QScriptDebuggerConsoleCommand::~QScriptDebuggerConsoleCommand()
{
}

QScriptDebuggerConsoleCommand::QScriptDebuggerConsoleCommand(QScriptDebuggerConsoleCommandPrivate &dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

/*!
  \fn QString QScriptDebuggerConsoleCommand::name() const

  Returns the name of this console command.
*/

/*!
  \fn QString QScriptDebuggerConsoleCommand::group() const

  Returns the group that this console command belongs to.
*/

/*!
  \fn QString QScriptDebuggerConsoleCommand::shortDescription() const

  Returns a short (one line) description of the command.
*/

/*!
  \fn QString QScriptDebuggerConsoleCommand::longDescription() const

  Returns a detailed description of how to use the command.
*/

/*!
  \fn QScriptDebuggerConsoleCommandJob *QScriptDebuggerConsoleCommand::createJob(
        const QStringList &arguments,
        QScriptDebuggerConsole *console,
        QScriptMessageHandlerInterface *messageHandler,
        QScriptDebuggerCommandSchedulerInterface *scheduler) = 0

  Creates a job that will execute this command with the given \a
  arguments. If the command cannot be executed (e.g. because one or
  more arguments are invalid), a suitable error message should be
  output to the \a messageHandler, and 0 should be returned.
*/

/*!
  Returns a list of names of commands that may also be of interest to
  users of this command.
*/
QStringList QScriptDebuggerConsoleCommand::seeAlso() const
{
    return QStringList();
}

/*!
  Returns a list of aliases for this command.
*/
QStringList QScriptDebuggerConsoleCommand::aliases() const
{
    return QStringList();
}

QStringList QScriptDebuggerConsoleCommand::argumentTypes() const
{
    return QStringList();
}

QStringList QScriptDebuggerConsoleCommand::subCommands() const
{
    return QStringList();
}

QT_END_NAMESPACE
