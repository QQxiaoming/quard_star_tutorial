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

#ifndef QDECLARATIVENAVIGATOR_P_P_H
#define QDECLARATIVENAVIGATOR_P_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qlist.h>
#include <QtLocation/private/qlocationglobal_p.h>
#include <QtCore/qpointer.h>
#include <QtLocation/qgeoroute.h>

QT_BEGIN_NAMESPACE

class QDeclarativeGeoServiceProvider;
class QDeclarativeGeoMap;
class QNavigationManager;
class QDeclarativeGeoRoute;
class QDeclarativePositionSource;
class QGeoMapParameter;
class QDeclarativeGeoRouteSegment;
class QParameterizableObject;
class QAbstractNavigator;

class Q_LOCATION_PRIVATE_EXPORT QDeclarativeNavigatorParams
{
public:
    QPointer<QDeclarativeGeoMap> m_map;
    QPointer<QDeclarativeGeoRoute> m_route;
    QGeoRoute m_geoRoute;
    QPointer<QDeclarativePositionSource> m_positionSource;
    QList<QPointer<QGeoMapParameter>> m_parameters;
    bool m_trackPositionSource = true;
};

class QDeclarativeNavigatorPrivate
{
public:
    QDeclarativeNavigatorPrivate(QParameterizableObject *q_);

    void updateReadyState();
    QParameterizableObject *q = nullptr;
    QSharedPointer<QDeclarativeNavigatorParams> m_params;
    QScopedPointer<QAbstractNavigator> m_navigator;
    QDeclarativeGeoServiceProvider *m_plugin = nullptr;
    QPointer<QDeclarativeGeoRoute> m_currentRoute;
    int m_currentSegment = 0;
    bool m_active = false;
    bool m_completed = false;
    bool m_ready = false;
};

QT_END_NAMESPACE

#endif // QDECLARATIVENAVIGATOR_P_P_H
