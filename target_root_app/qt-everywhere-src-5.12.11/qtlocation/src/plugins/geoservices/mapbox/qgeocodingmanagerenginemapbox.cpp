/****************************************************************************
**
** Copyright (C) 2017 Mapbox, Inc.
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

#include "qgeocodingmanagerenginemapbox.h"
#include "qgeocodereplymapbox.h"
#include "qmapboxcommon.h"

#include <QtCore/QVariantMap>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QLocale>
#include <QtCore/QStringList>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoShape>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoRectangle>

QT_BEGIN_NAMESPACE

namespace {
    static const QString allAddressTypes = QStringLiteral("address,district,locality,neighborhood,place,postcode,region,country");
}

QGeoCodingManagerEngineMapbox::QGeoCodingManagerEngineMapbox(const QVariantMap &parameters,
                                                       QGeoServiceProvider::Error *error,
                                                       QString *errorString)
:   QGeoCodingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("mapbox.useragent")))
        m_userAgent = parameters.value(QStringLiteral("mapbox.useragent")).toString().toLatin1();
    else
        m_userAgent = QByteArrayLiteral("Qt Location based application");

    m_accessToken = parameters.value(QStringLiteral("mapbox.access_token")).toString();

    m_isEnterprise = parameters.value(QStringLiteral("mapbox.enterprise")).toBool();
    m_urlPrefix = m_isEnterprise ? mapboxGeocodingEnterpriseApiPath : mapboxGeocodingApiPath;

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoCodingManagerEngineMapbox::~QGeoCodingManagerEngineMapbox()
{
}

QGeoCodeReply *QGeoCodingManagerEngineMapbox::geocode(const QGeoAddress &address, const QGeoShape &bounds)
{
    QUrlQuery queryItems;

    // If address text() is not generated: a manual setText() has been made.
    if (!address.isTextGenerated()) {
        queryItems.addQueryItem(QStringLiteral("type"), allAddressTypes);
        return doSearch(address.text().simplified(), queryItems, bounds);
    }

    QStringList addressString;
    QStringList typeString;

    if (!address.street().isEmpty()) {
        addressString.append(address.street());
        typeString.append(QStringLiteral("address"));
    }

    if (!address.district().isEmpty()) {
        addressString.append(address.district());
        typeString.append(QStringLiteral("district"));
        typeString.append(QStringLiteral("locality"));
        typeString.append(QStringLiteral("neighborhood"));
    }

    if (!address.city().isEmpty()) {
        addressString.append(address.city());
        typeString.append(QStringLiteral("place"));
    }

    if (!address.postalCode().isEmpty()) {
        addressString.append(address.postalCode());
        typeString.append(QStringLiteral("postcode"));
    }

    if (!address.state().isEmpty()) {
        addressString.append(address.state());
        typeString.append(QStringLiteral("region"));
    }

    if (!address.country().isEmpty()) {
        addressString.append(address.country());
        typeString.append(QStringLiteral("country"));
    }

    queryItems.addQueryItem(QStringLiteral("type"), typeString.join(QLatin1Char(',')));
    queryItems.addQueryItem(QStringLiteral("limit"), QString::number(1));

    return doSearch(addressString.join(QStringLiteral(", ")), queryItems, bounds);
}

QGeoCodeReply *QGeoCodingManagerEngineMapbox::geocode(const QString &address, int limit, int offset, const QGeoShape &bounds)
{
    Q_UNUSED(offset)

    QUrlQuery queryItems;
    queryItems.addQueryItem(QStringLiteral("type"), allAddressTypes);
    queryItems.addQueryItem(QStringLiteral("limit"), QString::number(limit));

    return doSearch(address, queryItems, bounds);
}

QGeoCodeReply *QGeoCodingManagerEngineMapbox::reverseGeocode(const QGeoCoordinate &coordinate, const QGeoShape &bounds)
{
    const QString coordinateString = QString::number(coordinate.longitude()) + QLatin1Char(',') + QString::number(coordinate.latitude());

    QUrlQuery queryItems;
    queryItems.addQueryItem(QStringLiteral("limit"), QString::number(1));

    return doSearch(coordinateString, queryItems, bounds);
}

QGeoCodeReply *QGeoCodingManagerEngineMapbox::doSearch(const QString &request, QUrlQuery &queryItems, const QGeoShape &bounds)
{
    queryItems.addQueryItem(QStringLiteral("access_token"), m_accessToken);

    const QString &languageCode = QLocale::system().name().section(QLatin1Char('_'), 0, 0);
    queryItems.addQueryItem(QStringLiteral("language"), languageCode);

    QGeoRectangle boundingBox = bounds.boundingGeoRectangle();
    if (!boundingBox.isEmpty()) {
        queryItems.addQueryItem(QStringLiteral("bbox"),
                QString::number(boundingBox.topLeft().longitude()) + QLatin1Char(',') +
                QString::number(boundingBox.bottomRight().latitude()) + QLatin1Char(',') +
                QString::number(boundingBox.bottomRight().longitude()) + QLatin1Char(',') +
                QString::number(boundingBox.topLeft().latitude()));
    }

    QUrl requestUrl(m_urlPrefix + request + QStringLiteral(".json"));
    requestUrl.setQuery(queryItems);

    QNetworkRequest networkRequest(requestUrl);
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    QNetworkReply *networkReply = m_networkManager->get(networkRequest);
    QGeoCodeReplyMapbox *reply = new QGeoCodeReplyMapbox(networkReply, this);

    connect(reply, &QGeoCodeReplyMapbox::finished, this, &QGeoCodingManagerEngineMapbox::onReplyFinished);
    connect(reply, QOverload<QGeoCodeReply::Error, const QString &>::of(&QGeoCodeReply::error),
            this, &QGeoCodingManagerEngineMapbox::onReplyError);

    return reply;
}

void QGeoCodingManagerEngineMapbox::onReplyFinished()
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QGeoCodingManagerEngineMapbox::onReplyError(QGeoCodeReply::Error errorCode, const QString &errorString)
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit error(reply, errorCode, errorString);
}

QT_END_NAMESPACE
