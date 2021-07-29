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

#ifndef QDECLARATIVENAVIGATOR_P_H
#define QDECLARATIVENAVIGATOR_P_H

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

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtQml/qqml.h>
#include <QSharedPointer>
#include <QtLocation/private/qparameterizableobject_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeGeoServiceProvider;
class QDeclarativeGeoMap;
class QNavigationManager;
class QDeclarativeGeoRoute;
class QDeclarativePositionSource;
class QDeclarativeGeoWaypoint;
class QGeoRoute;
class QGeoRouteSegment;
class QDeclarativeNavigatorPrivate;
class QDeclarativeGeoRouteSegment;

class Q_LOCATION_PRIVATE_EXPORT QDeclarativeNavigator : public QParameterizableObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QDeclarativeGeoServiceProvider *plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(QDeclarativeGeoMap *map READ map WRITE setMap NOTIFY mapChanged)
    Q_PROPERTY(QDeclarativeGeoRoute *route READ route WRITE setRoute NOTIFY routeChanged)
    Q_PROPERTY(QDeclarativePositionSource *positionSource READ positionSource WRITE setPositionSource NOTIFY positionSourceChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool navigatorReady READ navigatorReady NOTIFY navigatorReadyChanged)
    Q_PROPERTY(bool trackPositionSource READ trackPositionSource WRITE setTrackPositionSource NOTIFY trackPositionSourceChanged)
    Q_PROPERTY(QDeclarativeGeoRoute *currentRoute READ currentRoute NOTIFY currentRouteChanged)
    Q_PROPERTY(int currentSegment READ currentSegment NOTIFY currentSegmentChanged)
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit QDeclarativeNavigator(QObject *parent = nullptr);
    ~QDeclarativeNavigator();

    // QQmlParserStatus interface
    void classBegin() override;
    void componentComplete() override;

    // QDeclarativeNavigator
    void start();
    void stop();

    void setActive(bool active);
    bool active() const;

    void setPlugin(QDeclarativeGeoServiceProvider * plugin);
    QDeclarativeGeoServiceProvider *plugin() const;

    void setMap(QDeclarativeGeoMap *map);
    QDeclarativeGeoMap * map() const;

    void setRoute(QDeclarativeGeoRoute *route);
    QDeclarativeGeoRoute *route() const;

    void setPositionSource(QDeclarativePositionSource *positionSource);
    QDeclarativePositionSource *positionSource() const;

    bool navigatorReady() const;

    void setTrackPositionSource(bool trackPositionSource);
    bool trackPositionSource() const;

    QDeclarativeGeoRoute *currentRoute() const;
    int currentSegment() const;

signals:
    void navigatorReadyChanged(bool ready);
    void trackPositionSourceChanged(bool trackPositionSource);
    void activeChanged(bool active);
    void waypointReached(const QDeclarativeGeoWaypoint *pos);
    void destinationReached();

    void pluginChanged();
    void mapChanged();
    void routeChanged();
    void positionSourceChanged();
    void currentRouteChanged();
    void currentSegmentChanged();

private:
    void pluginReady();
    bool ensureEngine();
    void updateReadyState();

private slots:
    void onCurrentRouteChanged(const QGeoRoute &route);
    void onCurrentSegmentChanged(int segment);

private:
    QScopedPointer<QDeclarativeNavigatorPrivate> d_ptr;

    friend class QDeclarativeNavigatorPrivate;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeNavigator)


#endif // QDECLARATIVENAVIGATOR_P_H
