/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
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
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TYPES_H
#define TYPES_H

#include <QObject>
#include <QQmlListProperty>

class Type : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int baseProperty MEMBER m_baseProperty)

public:
    Type(QObject *parent = nullptr)
        : QObject(parent) {}

private:
    int m_baseProperty;
};

class ExtendedType : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int extendedProperty MEMBER m_extendedProperty)
    Q_PROPERTY(QQmlListProperty<QObject> data READ data)
    Q_CLASSINFO("DefaultProperty", "data")

public:
    ExtendedType(QObject *parent = nullptr)
        : QObject(parent) {}
    QQmlListProperty<QObject> data() { return QQmlListProperty<QObject>(this, m_data); }

private:
    QList<QObject *> m_data;
    int m_extendedProperty;
};

class DerivedType1 : public Type
{
    Q_OBJECT
    Q_PROPERTY(int m_exendedProperty2 MEMBER m_extendedProperty2)

public:
    DerivedType1(QObject *parent = nullptr)
        : Type(parent) {}

private:
    int m_extendedProperty2;
};

class DerivedType2 : public DerivedType1
{
    Q_OBJECT
public:
    DerivedType2(QObject *parent = nullptr)
        : DerivedType1(parent) {}
};

#endif // TYPES_H
