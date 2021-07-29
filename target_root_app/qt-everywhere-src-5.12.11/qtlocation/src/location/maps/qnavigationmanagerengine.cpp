/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
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

#include "qgeoroute.h"
#include "qnavigationmanagerengine_p.h"

QT_BEGIN_NAMESPACE

class QNavigationManagerEnginePrivate
{
public:
    QString managerName;
    int managerVersion;
    QLocale locale;
    QLocale::MeasurementSystem measurementSystem;
    bool initialized = false;
};

class QAbstractNavigatorPrivate
{
public:
    QLocale locale;
    QLocale::MeasurementSystem measurementSystem;
};

QAbstractNavigator::QAbstractNavigator(QObject *parent)
    : QObject(parent)
    , d(new QAbstractNavigatorPrivate)
{
}

QAbstractNavigator::~QAbstractNavigator()
{
}

void QAbstractNavigator::setLocale(const QLocale &locale)
{
    d->locale = locale;
}

QLocale QAbstractNavigator::locale() const
{
    return  d->locale;
}

void QAbstractNavigator::setMeasurementSystem(QLocale::MeasurementSystem system)
{
    d->measurementSystem = system;
}

QLocale::MeasurementSystem QAbstractNavigator::measurementSystem() const
{
    return d->measurementSystem;
}

QNavigationManagerEngine::QNavigationManagerEngine(const QVariantMap &parameters, QObject *parent)
    : QObject(parent)
    , d(new QNavigationManagerEnginePrivate)
{
    Q_UNUSED(parameters)
}

QNavigationManagerEngine::~QNavigationManagerEngine()
{
}

void QNavigationManagerEngine::setManagerName(const QString &name)
{
    d->managerName = name;
}

QString QNavigationManagerEngine::managerName() const
{
    return d->managerName;
}

void QNavigationManagerEngine::setManagerVersion(int version)
{
    d->managerVersion = version;
}

int QNavigationManagerEngine::managerVersion() const
{
    return d->managerVersion;
}

void QNavigationManagerEngine::setLocale(const QLocale &locale)
{
    d->locale = locale;
}

QLocale QNavigationManagerEngine::locale() const
{
    return d->locale;
}

void QNavigationManagerEngine::setMeasurementSystem(QLocale::MeasurementSystem system)
{
    d->measurementSystem = system;
}

QLocale::MeasurementSystem QNavigationManagerEngine::measurementSystem() const
{
    return d->measurementSystem;
}

bool QNavigationManagerEngine::isInitialized() const
{
    return d->initialized;
}

void QNavigationManagerEngine::engineInitialized()
{
    d->initialized = true;
    emit initialized();
}

QT_END_NAMESPACE
