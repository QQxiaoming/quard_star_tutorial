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

#include "qscriptdebuggerconsolecommandgroupdata_p.h"

#include <QtCore/qstring.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

/*!
  \since 4.5
  \class QScriptDebuggerConsoleCommandGroupData
  \internal

  \brief The QScriptDebuggerConsoleCommandGroupData class holds data associated with a console command group.
*/

class QScriptDebuggerConsoleCommandGroupDataPrivate : public QSharedData
{
public:
    QScriptDebuggerConsoleCommandGroupDataPrivate();
    ~QScriptDebuggerConsoleCommandGroupDataPrivate();

    QString shortDescription;
    QString longDescription;
};

QScriptDebuggerConsoleCommandGroupDataPrivate::QScriptDebuggerConsoleCommandGroupDataPrivate()
{
}

QScriptDebuggerConsoleCommandGroupDataPrivate::~QScriptDebuggerConsoleCommandGroupDataPrivate()
{
}

QScriptDebuggerConsoleCommandGroupData::QScriptDebuggerConsoleCommandGroupData()
    : d_ptr(0)
{
}

QScriptDebuggerConsoleCommandGroupData::QScriptDebuggerConsoleCommandGroupData(
    const QString &shortDescription, const QString &longDescription)
    : d_ptr(new QScriptDebuggerConsoleCommandGroupDataPrivate)
{
    d_ptr->shortDescription = shortDescription;
    d_ptr->longDescription = longDescription;
    d_ptr->ref.ref();
}

QScriptDebuggerConsoleCommandGroupData::QScriptDebuggerConsoleCommandGroupData(
    const QScriptDebuggerConsoleCommandGroupData &other)
    : d_ptr(other.d_ptr.data())
{
    if (d_ptr)
        d_ptr->ref.ref();
}

QScriptDebuggerConsoleCommandGroupData::~QScriptDebuggerConsoleCommandGroupData()
{
}

QScriptDebuggerConsoleCommandGroupData &QScriptDebuggerConsoleCommandGroupData::operator=(
    const QScriptDebuggerConsoleCommandGroupData &other)
{
    d_ptr.assign(other.d_ptr.data());
    return *this;
}

QString QScriptDebuggerConsoleCommandGroupData::shortDescription() const
{
    Q_D(const QScriptDebuggerConsoleCommandGroupData);
    return d->shortDescription;
}

QString QScriptDebuggerConsoleCommandGroupData::longDescription() const
{
    Q_D(const QScriptDebuggerConsoleCommandGroupData);
    return d->longDescription;
}

bool QScriptDebuggerConsoleCommandGroupData::isValid() const
{
    Q_D(const QScriptDebuggerConsoleCommandGroupData);
    return (d != 0);
}

QT_END_NAMESPACE
