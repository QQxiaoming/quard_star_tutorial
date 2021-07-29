/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef QGEOROUTINGMANAGERENGINE_TEST_H
#define QGEOROUTINGMANAGERENGINE_TEST_H

#include <qgeoserviceprovider.h>
#include <qgeoroutingmanagerengine.h>
#include <QLocale>
#include <qgeoaddress.h>
#include <qgeoroutereply.h>
#include <QtLocation/private/qgeoroute_p.h>

#include <QDebug>
#include <QTimer>
#include <QTimerEvent>

QT_USE_NAMESPACE


class QGeoRoutePrivateDefaultAlt : public QGeoRoutePrivateDefault
{
public:
    QGeoRoutePrivateDefaultAlt() : QGeoRoutePrivateDefault()
    {
        m_travelTime = 123456; // To identify this is actually a QGeoRoutePrivateDefaultAlt
    }
    QGeoRoutePrivateDefaultAlt(const QGeoRoutePrivateDefaultAlt &other)
        : QGeoRoutePrivateDefault(other) {}
    ~QGeoRoutePrivateDefaultAlt() {}

    void setTravelTime(int travelTime) override
    {
        Q_UNUSED(travelTime)
    }
};

class QGeoRouteAlt : public QGeoRoute
{
public:
    QGeoRouteAlt()
    : QGeoRoute(QExplicitlySharedDataPointer<QGeoRoutePrivate>(new QGeoRoutePrivateDefaultAlt()))
    {
    }
};

class RouteReplyTest :public QGeoRouteReply
{
    Q_OBJECT

public:
    RouteReplyTest(QObject *parent=0) :QGeoRouteReply (QGeoRouteRequest(), parent)
    {}
    void  callSetError ( Error error, const QString & errorString ) {setError(error, errorString);}
    void  callSetFinished ( bool finished ) {setFinished(finished);}
    void  callSetRoutes(const QList<QGeoRoute> &routes) {setRoutes(routes);}
};

class QGeoRoutingManagerEngineTest: public QGeoRoutingManagerEngine
{
    Q_OBJECT
    RouteReplyTest* routeReply_;
    bool finishRequestImmediately_;
    int timerId_;
    QGeoRouteReply::Error errorCode_;
    QString errorString_;
    bool alternateGeoRouteImplementation_;

public:
    QGeoRoutingManagerEngineTest(const QVariantMap &parameters,
        QGeoServiceProvider::Error *error, QString *errorString) :
        QGeoRoutingManagerEngine(parameters),
        routeReply_(0),
        finishRequestImmediately_(true),
        timerId_(0),
        errorCode_(QGeoRouteReply::NoError),
        alternateGeoRouteImplementation_(false)
    {
        Q_UNUSED(error)
        Q_UNUSED(errorString)

        if (parameters.contains("gc_finishRequestImmediately")) {
            finishRequestImmediately_ = qvariant_cast<bool>(parameters.value("gc_finishRequestImmediately"));
        }

        if (parameters.contains("gc_alternateGeoRoute")) {
            alternateGeoRouteImplementation_ = qvariant_cast<bool>(parameters.value("gc_alternateGeoRoute"));
        }

        setLocale(QLocale (QLocale::German, QLocale::Germany));
        setSupportedFeatureTypes (
                    QGeoRouteRequest::NoFeature | QGeoRouteRequest::TollFeature |
                    QGeoRouteRequest::HighwayFeature | QGeoRouteRequest::PublicTransitFeature |
                    QGeoRouteRequest::FerryFeature | QGeoRouteRequest::TunnelFeature |
                    QGeoRouteRequest::DirtRoadFeature | QGeoRouteRequest::ParksFeature |
                    QGeoRouteRequest::MotorPoolLaneFeature );
        setSupportedFeatureWeights (
                    QGeoRouteRequest::NeutralFeatureWeight | QGeoRouteRequest::PreferFeatureWeight |
                    QGeoRouteRequest::RequireFeatureWeight | QGeoRouteRequest::AvoidFeatureWeight |
                    QGeoRouteRequest::DisallowFeatureWeight );
        setSupportedManeuverDetails (
                    QGeoRouteRequest::NoManeuvers | QGeoRouteRequest::BasicManeuvers);
        setSupportedRouteOptimizations (
                    QGeoRouteRequest::ShortestRoute | QGeoRouteRequest::FastestRoute |
                    QGeoRouteRequest::MostEconomicRoute | QGeoRouteRequest::MostScenicRoute);
        setSupportedSegmentDetails (
                    QGeoRouteRequest::NoSegmentData | QGeoRouteRequest::BasicSegmentData );
        setSupportedTravelModes (
                    QGeoRouteRequest::CarTravel | QGeoRouteRequest::PedestrianTravel |
                    QGeoRouteRequest::BicycleTravel | QGeoRouteRequest::PublicTransitTravel |
                    QGeoRouteRequest::TruckTravel );
    }

    virtual QGeoRouteReply* calculateRoute(const QGeoRouteRequest& request)
    {
        routeReply_ = new RouteReplyTest();
        connect(routeReply_, SIGNAL(aborted()), this, SLOT(requestAborted()));

        if (request.numberAlternativeRoutes() > 70) {
            errorCode_ = (QGeoRouteReply::Error)(request.numberAlternativeRoutes() - 70);
            errorString_ = "error";
        } else {
            errorCode_ = QGeoRouteReply::NoError;
            errorString_ = "";
        }
        setRoutes(request, routeReply_);
        if (finishRequestImmediately_) {
            if (errorCode_) {
                routeReply_->callSetError(errorCode_, errorString_);
            } else {
                routeReply_->callSetError(QGeoRouteReply::NoError, "no error");
                routeReply_->callSetFinished(true);
            }
        } else {
            // we only allow serialized requests in QML - previous must have been aborted or finished
            Q_ASSERT(timerId_ == 0);
            timerId_ = startTimer(200);
        }
        return static_cast<QGeoRouteReply*>(routeReply_);
    }

    void setRoutes(const QGeoRouteRequest& request, RouteReplyTest* reply)
    {
        QList<QGeoRoute> routes;
        int travelTime = 0;
        if (request.extraParameters().contains("test-traveltime"))
            travelTime = request.extraParameters().value("test-traveltime").toMap().value("requestedTime").toInt();

        for (int i = 0; i < request.numberAlternativeRoutes(); ++i) {
            QGeoRoute route;
            if (alternateGeoRouteImplementation_)
                route = QGeoRouteAlt();
            route.setPath(request.waypoints());
            route.setTravelTime(travelTime);

            const QList<QVariantMap> metadata = request.waypointsMetadata();
            for (const auto &meta: metadata) {
                if (meta.contains("extra")) {
                    QVariantMap extra = meta.value("extra").toMap();
                    if (extra.contains("user_distance"))
                        route.setDistance(meta.value("extra").toMap().value("user_distance").toMap().value("distance").toDouble());
                }
            }

            routes.append(route);
        }
        reply->callSetRoutes(routes);
    }

public Q_SLOTS:
    void requestAborted()
    {
        if (timerId_) {
            killTimer(timerId_);
            timerId_ = 0;
        }
        errorCode_ = QGeoRouteReply::NoError;
        errorString_ = "";
    }

protected:
     void timerEvent(QTimerEvent *event)
     {
         Q_UNUSED(event);
         Q_ASSERT(timerId_ == event->timerId());
         Q_ASSERT(routeReply_);
         killTimer(timerId_);
         timerId_ = 0;
         if (errorCode_) {
             routeReply_->callSetError(errorCode_, errorString_);
             emit error(routeReply_, errorCode_, errorString_);
         } else {
             routeReply_->callSetError(QGeoRouteReply::NoError, "no error");
             routeReply_->callSetFinished(true);
             emit finished(routeReply_);
         }
     }
};

#endif
