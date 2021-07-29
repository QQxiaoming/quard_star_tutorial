/****************************************************************************
**
** Copyright (C) 2017 Mapbox, Inc.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtFoo module of the Qt Toolkit.
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

#include "qplacemanagerenginemapbox.h"
#include "qplacesearchreplymapbox.h"
#include "qplacesearchsuggestionreplymapbox.h"
#include "qplacecategoriesreplymapbox.h"
#include "qmapboxcommon.h"

#include <QtCore/QUrlQuery>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QRegularExpression>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoCircle>
#include <QtLocation/private/unsupportedreplies_p.h>

#include <QtCore/QElapsedTimer>

namespace {

// https://www.mapbox.com/api-documentation/#poi-categories
static const QStringList categories = QStringList()
    << QStringLiteral("bakery")
    << QStringLiteral("bank")
    << QStringLiteral("bar")
    << QStringLiteral("cafe")
    << QStringLiteral("church")
    << QStringLiteral("cinema")
    << QStringLiteral("coffee")
    << QStringLiteral("concert")
    << QStringLiteral("fast food")
    << QStringLiteral("finance")
    << QStringLiteral("gallery")
    << QStringLiteral("historic")
    << QStringLiteral("hotel")
    << QStringLiteral("landmark")
    << QStringLiteral("museum")
    << QStringLiteral("music")
    << QStringLiteral("park")
    << QStringLiteral("pizza")
    << QStringLiteral("restaurant")
    << QStringLiteral("retail")
    << QStringLiteral("school")
    << QStringLiteral("shop")
    << QStringLiteral("tea")
    << QStringLiteral("theater")
    << QStringLiteral("university");

} // namespace

// Mapbox API does not provide support for paginated place queries. This
// implementation is a wrapper around its Geocoding service:
// https://www.mapbox.com/api-documentation/#geocoding
QPlaceManagerEngineMapbox::QPlaceManagerEngineMapbox(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString)
    : QPlaceManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("mapbox.useragent")))
        m_userAgent = parameters.value(QStringLiteral("mapbox.useragent")).toString().toLatin1();
    else
        m_userAgent = mapboxDefaultUserAgent;

    m_accessToken = parameters.value(QStringLiteral("mapbox.access_token")).toString();

    m_isEnterprise = parameters.value(QStringLiteral("mapbox.enterprise")).toBool();
    m_urlPrefix = m_isEnterprise ? mapboxGeocodingEnterpriseApiPath : mapboxGeocodingApiPath;

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QPlaceManagerEngineMapbox::~QPlaceManagerEngineMapbox()
{
}

QPlaceSearchReply *QPlaceManagerEngineMapbox::search(const QPlaceSearchRequest &request)
{
    return qobject_cast<QPlaceSearchReply *>(doSearch(request, PlaceSearchType::CompleteSearch));
}

QPlaceSearchSuggestionReply *QPlaceManagerEngineMapbox::searchSuggestions(const QPlaceSearchRequest &request)
{
    return qobject_cast<QPlaceSearchSuggestionReply *>(doSearch(request, PlaceSearchType::SuggestionSearch));
}

QPlaceReply *QPlaceManagerEngineMapbox::doSearch(const QPlaceSearchRequest &request, PlaceSearchType searchType)
{
    const QGeoShape searchArea = request.searchArea();
    const QString searchTerm = request.searchTerm();
    const QString recommendationId = request.recommendationId();
    const QList<QPlaceCategory> placeCategories = request.categories();

    bool invalidRequest = false;

    // QLocation::DeviceVisibility is not allowed for non-enterprise accounts.
    if (!m_isEnterprise)
        invalidRequest |= request.visibilityScope().testFlag(QLocation::DeviceVisibility);

    // Must provide either a search term, categories or recommendation.
    invalidRequest |= searchTerm.isEmpty() && placeCategories.isEmpty() && recommendationId.isEmpty();

    // Category search must not provide recommendation, and vice-versa.
    invalidRequest |= searchTerm.isEmpty() && !placeCategories.isEmpty() && !recommendationId.isEmpty();

    if (invalidRequest) {
        QPlaceReply *reply;
        if (searchType == PlaceSearchType::CompleteSearch)
            reply = new QPlaceSearchReplyMapbox(request, 0, this);
        else
            reply = new QPlaceSearchSuggestionReplyMapbox(0, this);

        connect(reply, &QPlaceReply::finished, this, &QPlaceManagerEngineMapbox::onReplyFinished);
        connect(reply, QOverload<QPlaceReply::Error, const QString &>::of(&QPlaceReply::error),
                this, &QPlaceManagerEngineMapbox::onReplyError);

        QMetaObject::invokeMethod(reply, "setError", Qt::QueuedConnection,
                Q_ARG(QPlaceReply::Error, QPlaceReply::BadArgumentError),
                Q_ARG(QString, "Invalid request."));

        return reply;
    }

    QString queryString;
    if (!searchTerm.isEmpty()) {
        queryString = searchTerm;
    } else if (!recommendationId.isEmpty()) {
        queryString = recommendationId;
    } else {
        QStringList similarIds;
        for (const QPlaceCategory &placeCategory : placeCategories)
            similarIds.append(placeCategory.categoryId());
        queryString = similarIds.join(QLatin1Char(','));
    }
    queryString.append(QStringLiteral(".json"));

    // https://www.mapbox.com/api-documentation/#request-format
    QUrl requestUrl(m_urlPrefix + queryString);

    QUrlQuery queryItems;
    queryItems.addQueryItem(QStringLiteral("access_token"), m_accessToken);

    // XXX: Investigate situations where we need to filter by 'country'.

    QStringList languageCodes;
    for (const QLocale& locale: qAsConst(m_locales)) {
        // Returns the language and country of this locale as a string of the
        // form "language_country", where language is a lowercase, two-letter
        // ISO 639 language code, and country is an uppercase, two- or
        // three-letter ISO 3166 country code.

        if (locale.language() == QLocale::C)
            continue;

        const QString languageCode = locale.name().section(QLatin1Char('_'), 0, 0);
        if (!languageCodes.contains(languageCode))
            languageCodes.append(languageCode);
    }

    if (!languageCodes.isEmpty())
        queryItems.addQueryItem(QStringLiteral("language"), languageCodes.join(QLatin1Char(',')));

    if (searchArea.type() != QGeoShape::UnknownType) {
        const QGeoCoordinate center = searchArea.center();
        queryItems.addQueryItem(QStringLiteral("proximity"),
                                QString::number(center.longitude()) + QLatin1Char(',') + QString::number(center.latitude()));
    }

    queryItems.addQueryItem(QStringLiteral("type"), QStringLiteral("poi"));

    // XXX: Investigate situations where 'autocomplete' should be disabled.

    QGeoRectangle boundingBox = searchArea.boundingGeoRectangle();
    if (!boundingBox.isEmpty()) {
        queryItems.addQueryItem(QStringLiteral("bbox"),
                QString::number(boundingBox.topLeft().longitude()) + QLatin1Char(',') +
                QString::number(boundingBox.bottomRight().latitude()) + QLatin1Char(',') +
                QString::number(boundingBox.bottomRight().longitude()) + QLatin1Char(',') +
                QString::number(boundingBox.topLeft().latitude()));
    }

    if (request.limit() > 0)
        queryItems.addQueryItem(QStringLiteral("limit"), QString::number(request.limit()));

    // XXX: Investigate searchContext() use cases.

    requestUrl.setQuery(queryItems);

    QNetworkRequest networkRequest(requestUrl);
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    QNetworkReply *networkReply = m_networkManager->get(networkRequest);
    QPlaceReply *reply;
    if (searchType == PlaceSearchType::CompleteSearch)
        reply = new QPlaceSearchReplyMapbox(request, networkReply, this);
    else
        reply = new QPlaceSearchSuggestionReplyMapbox(networkReply, this);

    connect(reply, &QPlaceReply::finished, this, &QPlaceManagerEngineMapbox::onReplyFinished);
    connect(reply, QOverload<QPlaceReply::Error, const QString &>::of(&QPlaceReply::error),
            this, &QPlaceManagerEngineMapbox::onReplyError);

    return reply;
}

QPlaceReply *QPlaceManagerEngineMapbox::initializeCategories()
{
    if (m_categories.isEmpty()) {
        for (const QString &categoryId : categories) {
            QPlaceCategory category;
            category.setName(QMapboxCommon::mapboxNameForCategory(categoryId));
            category.setCategoryId(categoryId);
            category.setVisibility(QLocation::PublicVisibility);
            m_categories[categoryId] = category;
        }
    }

    QPlaceCategoriesReplyMapbox *reply = new QPlaceCategoriesReplyMapbox(this);
    connect(reply, &QPlaceReply::finished, this, &QPlaceManagerEngineMapbox::onReplyFinished);
    connect(reply, QOverload<QPlaceReply::Error, const QString &>::of(&QPlaceReply::error),
            this, &QPlaceManagerEngineMapbox::onReplyError);

    // Queue a future finished() emission from the reply.
    QMetaObject::invokeMethod(reply, "finish", Qt::QueuedConnection);

    return reply;
}

QString QPlaceManagerEngineMapbox::parentCategoryId(const QString &categoryId) const
{
    Q_UNUSED(categoryId)

    // Only a single category level.
    return QString();
}

QStringList QPlaceManagerEngineMapbox::childCategoryIds(const QString &categoryId) const
{
    // Only a single category level.
    if (categoryId.isEmpty())
        return m_categories.keys();

    return QStringList();
}

QPlaceCategory QPlaceManagerEngineMapbox::category(const QString &categoryId) const
{
    return m_categories.value(categoryId);
}

QList<QPlaceCategory> QPlaceManagerEngineMapbox::childCategories(const QString &parentId) const
{
    // Only a single category level.
    if (parentId.isEmpty())
        return m_categories.values();

    return QList<QPlaceCategory>();
}

QList<QLocale> QPlaceManagerEngineMapbox::locales() const
{
    return m_locales;
}

void QPlaceManagerEngineMapbox::setLocales(const QList<QLocale> &locales)
{
    m_locales = locales;
}

void QPlaceManagerEngineMapbox::onReplyFinished()
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QPlaceManagerEngineMapbox::onReplyError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit error(reply, errorCode, errorString);
}
