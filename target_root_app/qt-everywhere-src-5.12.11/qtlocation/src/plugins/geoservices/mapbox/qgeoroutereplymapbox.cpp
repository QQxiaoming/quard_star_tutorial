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

#include "qgeoroutereplymapbox.h"
#include "qgeoroutingmanagerenginemapbox.h"
#include <QtLocation/private/qgeorouteparser_p.h>
#include <QtLocation/private/qgeoroute_p.h>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtLocation/QGeoRouteSegment>
#include <QtLocation/QGeoManeuver>

namespace {

class QGeoRouteMapbox : public QGeoRoute
{
public:
    QGeoRouteMapbox(const QGeoRoute &other, const QVariantMap &metadata);
};

class QGeoRoutePrivateMapbox : public QGeoRoutePrivateDefault
{
public:
    QGeoRoutePrivateMapbox(const QGeoRoutePrivateDefault &other, const QVariantMap &metadata);

    virtual QString engineName() const override;
    virtual QVariantMap metadata() const override;

    QVariantMap m_metadata;
};

QGeoRouteMapbox::QGeoRouteMapbox(const QGeoRoute &other, const QVariantMap &metadata)
    : QGeoRoute(QExplicitlySharedDataPointer<QGeoRoutePrivateMapbox>(new QGeoRoutePrivateMapbox(*static_cast<const QGeoRoutePrivateDefault *>(QGeoRoutePrivate::routePrivateData(other)), metadata)))
{
}

QGeoRoutePrivateMapbox::QGeoRoutePrivateMapbox(const QGeoRoutePrivateDefault &other, const QVariantMap &metadata)
    : QGeoRoutePrivateDefault(other)
    , m_metadata(metadata)
{
}

QString QGeoRoutePrivateMapbox::engineName() const
{
    return QStringLiteral("mapbox");
}

QVariantMap QGeoRoutePrivateMapbox::metadata() const
{
    return m_metadata;
}

} // namespace

QT_BEGIN_NAMESPACE

QGeoRouteReplyMapbox::QGeoRouteReplyMapbox(QNetworkReply *reply, const QGeoRouteRequest &request,
                                     QObject *parent)
:   QGeoRouteReply(request, parent)
{
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    connect(reply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(networkReplyError(QNetworkReply::NetworkError)));
    connect(this, &QGeoRouteReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

QGeoRouteReplyMapbox::~QGeoRouteReplyMapbox()
{
}

void QGeoRouteReplyMapbox::networkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QGeoRoutingManagerEngineMapbox *engine = qobject_cast<QGeoRoutingManagerEngineMapbox *>(parent());
    const QGeoRouteParser *parser = engine->routeParser();

    QList<QGeoRoute> routes;
    QString errorString;

    QByteArray routeReply = reply->readAll();
    QGeoRouteReply::Error error = parser->parseReply(routes, errorString, routeReply);
    // Setting the request into the result
    for (QGeoRoute &route : routes) {
        route.setRequest(request());
        for (QGeoRoute &leg: route.routeLegs()) {
            leg.setRequest(request());
        }
    }

    QVariantMap metadata;
    metadata["osrm.reply-json"] = routeReply;

    QList<QGeoRoute> mapboxRoutes;
    for (const QGeoRoute &route : routes.mid(0, request().numberAlternativeRoutes() + 1)) {
        QGeoRouteMapbox mapboxRoute(route, metadata);
        mapboxRoutes.append(mapboxRoute);
    }

    if (error == QGeoRouteReply::NoError) {
        setRoutes(mapboxRoutes);
        // setError(QGeoRouteReply::NoError, status);  // can't do this, or NoError is emitted and does damages
        setFinished(true);
    } else {
        setError(error, errorString);
    }
}

void QGeoRouteReplyMapbox::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(QGeoRouteReply::CommunicationError, reply->errorString());
}

QT_END_NAMESPACE
