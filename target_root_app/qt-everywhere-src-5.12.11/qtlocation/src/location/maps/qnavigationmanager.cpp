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

#include "qnavigationmanager_p.h"
#include "qnavigationmanagerengine_p.h"

QT_BEGIN_NAMESPACE

class QNavigationManagerPrivate
{
public:
    QNavigationManagerPrivate();
    ~QNavigationManagerPrivate();

    QNavigationManagerEngine *engine = nullptr;

private:
    Q_DISABLE_COPY(QNavigationManagerPrivate)
};

QNavigationManagerPrivate::QNavigationManagerPrivate()
{

}

QNavigationManagerPrivate::~QNavigationManagerPrivate()
{
    delete engine;
    engine = nullptr;
}

QNavigationManager::~QNavigationManager()
{
    delete d_ptr;
}

QString QNavigationManager::managerName() const
{
    return d_ptr->engine->managerName();
}

int QNavigationManager::managerVersion() const
{
    return d_ptr->engine->managerVersion();
}

QNavigationManagerEngine *QNavigationManager::engine()
{
    return d_ptr->engine;
}

bool QNavigationManager::isInitialized() const
{
    return d_ptr->engine->isInitialized();
}

void QNavigationManager::setLocale(const QLocale &locale)
{
    d_ptr->engine->setLocale(locale);
}

QLocale QNavigationManager::locale() const
{
    return d_ptr->engine->locale();
}

void QNavigationManager::setMeasurementSystem(QLocale::MeasurementSystem system)
{
    d_ptr->engine->setMeasurementSystem(system);
}

QLocale::MeasurementSystem QNavigationManager::measurementSystem() const
{
    return d_ptr->engine->measurementSystem();
}

QAbstractNavigator *QNavigationManager::createNavigator(const QSharedPointer<QDeclarativeNavigatorParams> &navigator)
{
    return d_ptr->engine->createNavigator(navigator);
}

QNavigationManager::QNavigationManager(QNavigationManagerEngine *engine, QObject *parent) : QObject(parent),
    d_ptr(new QNavigationManagerPrivate)
{
    d_ptr->engine = engine;
    if (!d_ptr->engine) {
        qFatal("The navigation manager engine that was set for this mapping manager was NULL.");
    }

    connect(d_ptr->engine,
            SIGNAL(initialized()),
            this,
            SIGNAL(initialized()),
            Qt::QueuedConnection);
}

QT_END_NAMESPACE
