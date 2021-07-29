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

#include "qscriptvalueproperty_p.h"

#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QScriptValuePropertyPrivate : public QSharedData
{
public:
    QScriptValuePropertyPrivate();
    ~QScriptValuePropertyPrivate();

    QString name;
    QScriptValue value;
    QScriptValue::PropertyFlags flags;
};

QScriptValuePropertyPrivate::QScriptValuePropertyPrivate()
{
}

QScriptValuePropertyPrivate::~QScriptValuePropertyPrivate()
{
}

/*!
  Constructs an invalid QScriptValueProperty.
*/
QScriptValueProperty::QScriptValueProperty()
    : d_ptr(0)
{
}

/*!
  Constructs a QScriptValueProperty with the given \a name,
  \a value and \a flags.
*/
QScriptValueProperty::QScriptValueProperty(const QString &name,
                                           const QScriptValue &value,
                                           QScriptValue::PropertyFlags flags)
    : d_ptr(new QScriptValuePropertyPrivate)
{
    d_ptr->name = name;
    d_ptr->value = value;
    d_ptr->flags = flags;
    d_ptr->ref.ref();
}

/*!
  Constructs a QScriptValueProperty that is a copy of the \a other property.
*/
QScriptValueProperty::QScriptValueProperty(const QScriptValueProperty &other)
    : d_ptr(other.d_ptr.data())
{
    if (d_ptr)
        d_ptr->ref.ref();
}

/*!
  Destroys this QScriptValueProperty.
*/
QScriptValueProperty::~QScriptValueProperty()
{
}

/*!
  Assigns the \a other property to this QScriptValueProperty.
*/
QScriptValueProperty &QScriptValueProperty::operator=(const QScriptValueProperty &other)
{
    d_ptr.assign(other.d_ptr.data());
    return *this;
}

/*!
  Returns the name of this QScriptValueProperty.
*/
QString QScriptValueProperty::name() const
{
    Q_D(const QScriptValueProperty);
    if (!d)
        return QString();
    return d->name;
}

/*!
  Returns the value of this QScriptValueProperty.
*/
QScriptValue QScriptValueProperty::value() const
{
    Q_D(const QScriptValueProperty);
    if (!d)
        return QScriptValue();
    return d->value;
}

/*!
  Returns the flags of this QScriptValueProperty.
*/
QScriptValue::PropertyFlags QScriptValueProperty::flags() const
{
    Q_D(const QScriptValueProperty);
    if (!d)
        return 0;
    return d->flags;
}

/*!
  Returns true if this QScriptValueProperty is valid, otherwise
  returns false.
*/
bool QScriptValueProperty::isValid() const
{
    Q_D(const QScriptValueProperty);
    return (d != 0);
}

QT_END_NAMESPACE
