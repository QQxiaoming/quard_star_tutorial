/****************************************************************************
**
** Copyright (C) 2016 Vlad Seryakov <vseryakov@gmail.com>
** Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
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

#include "qgeoroutingmanagerenginemapbox.h"
#include "qgeoroutereplymapbox.h"
#include "qmapboxcommon.h"
#include <QtLocation/private/qgeorouteparserosrmv5_p.h>
#include <QtLocation/qgeoroutesegment.h>
#include <QtLocation/qgeomaneuver.h>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QUrlQuery>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

class QGeoRouteParserOsrmV5ExtensionMapbox: public QGeoRouteParserOsrmV5Extension
{
public:
    QGeoRouteParserOsrmV5ExtensionMapbox(const QString &accessToken, bool useMapboxTextInstructions);
    void updateQuery(QUrlQuery &query) const override;
    void updateSegment(QGeoRouteSegment &segment, const QJsonObject &step, const QJsonObject &maneuver) const override;

    QString m_accessToken;
    bool m_useMapboxTextInstructions = false;
};

QGeoRouteParserOsrmV5ExtensionMapbox::QGeoRouteParserOsrmV5ExtensionMapbox(const QString &accessToken, bool useMapboxTextInstructions)
    : QGeoRouteParserOsrmV5Extension(), m_accessToken(accessToken), m_useMapboxTextInstructions(useMapboxTextInstructions)
{

}

void QGeoRouteParserOsrmV5ExtensionMapbox::updateQuery(QUrlQuery &query) const
{
    if (!m_accessToken.isEmpty())
        query.addQueryItem(QLatin1String("access_token"), m_accessToken);

    query.addQueryItem(QLatin1String("annotations"), QLatin1String("duration,distance,speed,congestion"));

    query.addQueryItem(QLatin1String("voice_instructions"), QLatin1String("true"));
    query.addQueryItem(QLatin1String("banner_instructions"), QLatin1String("true"));
    query.addQueryItem(QLatin1String("roundabout_exits"), QLatin1String("true"));

    QLocale::MeasurementSystem unit = QLocale::system().measurementSystem();
    query.addQueryItem(QLatin1String("voice_units"), unit == QLocale::MetricSystem ? QLatin1String("metric") : QLatin1String("imperial"));
}

static QVariantMap parseMapboxVoiceInstruction(const QJsonObject &voiceInstruction)
{
    QVariantMap map;

    if (voiceInstruction.value(QLatin1String("distanceAlongGeometry")).isDouble())
        map.insert(QLatin1String("distance_along_geometry"), voiceInstruction.value(QLatin1String("distanceAlongGeometry")).toDouble());

    if (voiceInstruction.value(QLatin1String("announcement")).isString())
        map.insert(QLatin1String("announcement"), voiceInstruction.value(QLatin1String("announcement")).toString());

    if (voiceInstruction.value(QLatin1String("ssmlAnnouncement")).isString())
        map.insert(QLatin1String("ssml_announcement"), voiceInstruction.value(QLatin1String("ssmlAnnouncement")).toString());

    return map;
}

static QVariantList parseMapboxVoiceInstructions(const QJsonArray &voiceInstructions)
{
    QVariantList list;
    for (const QJsonValue &voiceInstructionValue : voiceInstructions) {
        if (voiceInstructionValue.isObject())
            list << parseMapboxVoiceInstruction(voiceInstructionValue.toObject());
    }
    return list;
}

static QVariantMap parseMapboxBannerComponent(const QJsonObject &bannerComponent)
{
    QVariantMap map;

    if (bannerComponent.value(QLatin1String("type")).isString())
        map.insert(QLatin1String("type"), bannerComponent.value(QLatin1String("type")).toString());

    if (bannerComponent.value(QLatin1String("text")).isString())
        map.insert(QLatin1String("text"), bannerComponent.value(QLatin1String("text")).toString());

    if (bannerComponent.value(QLatin1String("abbr")).isString())
        map.insert(QLatin1String("abbr"), bannerComponent.value(QLatin1String("abbr")).toString());

    if (bannerComponent.value(QLatin1String("abbr_priority")).isDouble())
        map.insert(QLatin1String("abbr_priority"), bannerComponent.value(QLatin1String("abbr_priority")).toInt());

    return map;
}

static QVariantList parseMapboxBannerComponents(const QJsonArray &bannerComponents)
{
    QVariantList list;
    for (const QJsonValue &bannerComponentValue : bannerComponents) {
        if (bannerComponentValue.isObject())
            list << parseMapboxBannerComponent(bannerComponentValue.toObject());
    }
    return list;
}

static QVariantMap parseMapboxBanner(const QJsonObject &banner)
{
    QVariantMap map;

    if (banner.value(QLatin1String("text")).isString())
        map.insert(QLatin1String("text"), banner.value(QLatin1String("text")).toString());

    if (banner.value(QLatin1String("components")).isArray())
        map.insert(QLatin1String("components"), parseMapboxBannerComponents(banner.value(QLatin1String("components")).toArray()));

    if (banner.value(QLatin1String("type")).isString())
        map.insert(QLatin1String("type"), banner.value(QLatin1String("type")).toString());

    if (banner.value(QLatin1String("modifier")).isString())
        map.insert(QLatin1String("modifier"), banner.value(QLatin1String("modifier")).toString());

    if (banner.value(QLatin1String("degrees")).isDouble())
        map.insert(QLatin1String("degrees"), banner.value(QLatin1String("degrees")).toDouble());

    if (banner.value(QLatin1String("driving_side")).isString())
        map.insert(QLatin1String("driving_side"), banner.value(QLatin1String("driving_side")).toString());

    return map;
}

static QVariantMap parseMapboxBannerInstruction(const QJsonObject &bannerInstruction)
{
    QVariantMap map;

    if (bannerInstruction.value(QLatin1String("distanceAlongGeometry")).isDouble())
        map.insert(QLatin1String("distance_along_geometry"), bannerInstruction.value(QLatin1String("distanceAlongGeometry")).toDouble());

    if (bannerInstruction.value(QLatin1String("primary")).isObject())
        map.insert(QLatin1String("primary"), parseMapboxBanner(bannerInstruction.value(QLatin1String("primary")).toObject()));

    if (bannerInstruction.value(QLatin1String("secondary")).isObject())
        map.insert(QLatin1String("secondary"), parseMapboxBanner(bannerInstruction.value(QLatin1String("secondary")).toObject()));

    if (bannerInstruction.value(QLatin1String("then")).isObject())
        map.insert(QLatin1String("then"), parseMapboxBanner(bannerInstruction.value(QLatin1String("then")).toObject()));

    return map;
}

static QVariantList parseMapboxBannerInstructions(const QJsonArray &bannerInstructions)
{
    QVariantList list;
    for (const QJsonValue &bannerInstructionValue : bannerInstructions) {
        if (bannerInstructionValue.isObject())
            list << parseMapboxBannerInstruction(bannerInstructionValue.toObject());
    }
    return list;
}

void QGeoRouteParserOsrmV5ExtensionMapbox::updateSegment(QGeoRouteSegment &segment, const QJsonObject &step, const QJsonObject &maneuver) const
{
    QGeoManeuver m = segment.maneuver();
    QVariantMap extendedAttributes = m.extendedAttributes();
    if (m_useMapboxTextInstructions && maneuver.value(QLatin1String("instruction")).isString()) {
        QString maneuverInstructionText = maneuver.value(QLatin1String("instruction")).toString();
        if (!maneuverInstructionText.isEmpty())
            m.setInstructionText(maneuverInstructionText);
    }

    if (step.value(QLatin1String("voiceInstructions")).isArray())
        extendedAttributes.insert(QLatin1String("mapbox.voice_instructions"),
                                  parseMapboxVoiceInstructions(step.value(QLatin1String("voiceInstructions")).toArray()));
    if (step.value(QLatin1String("bannerInstructions")).isArray())
        extendedAttributes.insert(QLatin1String("mapbox.banner_instructions"),
                                  parseMapboxBannerInstructions(step.value(QLatin1String("bannerInstructions")).toArray()));

    m.setExtendedAttributes(extendedAttributes);
    segment.setManeuver(m);
}


QGeoRoutingManagerEngineMapbox::QGeoRoutingManagerEngineMapbox(const QVariantMap &parameters,
                                                         QGeoServiceProvider::Error *error,
                                                         QString *errorString)
    : QGeoRoutingManagerEngine(parameters),
      m_networkManager(new QNetworkAccessManager(this)),
      m_userAgent(mapboxDefaultUserAgent)
{
    if (parameters.contains(QStringLiteral("mapbox.useragent"))) {
        m_userAgent = parameters.value(QStringLiteral("mapbox.useragent")).toString().toLatin1();
    }

    if (parameters.contains(QStringLiteral("mapbox.access_token"))) {
        m_accessToken = parameters.value(QStringLiteral("mapbox.access_token")).toString();
    }

    bool use_mapbox_text_instructions = true;
    if (parameters.contains(QStringLiteral("mapbox.routing.use_mapbox_text_instructions"))) {
        use_mapbox_text_instructions = parameters.value(QStringLiteral("mapbox.routing.use_mapbox_text_instructions")).toBool();
    }

    QGeoRouteParserOsrmV5 *parser = new QGeoRouteParserOsrmV5(this);
    parser->setExtension(new QGeoRouteParserOsrmV5ExtensionMapbox(m_accessToken, use_mapbox_text_instructions));
    if (parameters.contains(QStringLiteral("mapbox.routing.traffic_side"))) {
        QString trafficSide = parameters.value(QStringLiteral("mapbox.routing.traffic_side")).toString();
        if (trafficSide == QStringLiteral("right"))
            parser->setTrafficSide(QGeoRouteParser::RightHandTraffic);
        else if (trafficSide == QStringLiteral("left"))
            parser->setTrafficSide(QGeoRouteParser::LeftHandTraffic);
    }
    m_routeParser = parser;

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoRoutingManagerEngineMapbox::~QGeoRoutingManagerEngineMapbox()
{
}

QGeoRouteReply* QGeoRoutingManagerEngineMapbox::calculateRoute(const QGeoRouteRequest &request)
{
    QNetworkRequest networkRequest;
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    QString url = mapboxDirectionsApiPath;

    QGeoRouteRequest::TravelModes travelModes = request.travelModes();
    if (travelModes.testFlag(QGeoRouteRequest::PedestrianTravel)) {
        url += QStringLiteral("walking/");
    } else if (travelModes.testFlag(QGeoRouteRequest::BicycleTravel)) {
        url += QStringLiteral("cycling/");
    } else if (travelModes.testFlag(QGeoRouteRequest::CarTravel)) {
        const QList<QGeoRouteRequest::FeatureType> &featureTypes = request.featureTypes();
        int trafficFeatureIdx = featureTypes.indexOf(QGeoRouteRequest::TrafficFeature);
        QGeoRouteRequest::FeatureWeight trafficWeight = request.featureWeight(QGeoRouteRequest::TrafficFeature);
        if (trafficFeatureIdx >= 0 &&
           (trafficWeight == QGeoRouteRequest::AvoidFeatureWeight || trafficWeight == QGeoRouteRequest::DisallowFeatureWeight)) {
            url += QStringLiteral("driving-traffic/");
        } else {
            url += QStringLiteral("driving/");
        }
    }

    networkRequest.setUrl(m_routeParser->requestUrl(request, url));

    QNetworkReply *reply = m_networkManager->get(networkRequest);

    QGeoRouteReplyMapbox *routeReply = new QGeoRouteReplyMapbox(reply, request, this);

    connect(routeReply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(routeReply, SIGNAL(error(QGeoRouteReply::Error,QString)),
            this, SLOT(replyError(QGeoRouteReply::Error,QString)));

    return routeReply;
}

const QGeoRouteParser *QGeoRoutingManagerEngineMapbox::routeParser() const
{
    return m_routeParser;
}

void QGeoRoutingManagerEngineMapbox::replyFinished()
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QGeoRoutingManagerEngineMapbox::replyError(QGeoRouteReply::Error errorCode,
                                             const QString &errorString)
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        emit error(reply, errorCode, errorString);
}

QT_END_NAMESPACE
