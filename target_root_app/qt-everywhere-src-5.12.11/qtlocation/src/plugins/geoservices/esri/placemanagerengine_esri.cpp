/****************************************************************************
**
** Copyright (C) 2013-2018 Esri <contracts@esri.com>
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

#include "placemanagerengine_esri.h"
#include "placesearchreply_esri.h"
#include "placecategoriesreply_esri.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QtCore/QUrlQuery>

QT_BEGIN_NAMESPACE

// https://developers.arcgis.com/rest/geocode/api-reference/geocoding-find-address-candidates.htm
// https://developers.arcgis.com/rest/geocode/api-reference/geocoding-category-filtering.htm
// https://developers.arcgis.com/rest/geocode/api-reference/geocoding-service-output.htm

static const QString kCategoriesKey(QStringLiteral("categories"));
static const QString kSingleLineKey(QStringLiteral("singleLine"));
static const QString kLocationKey(QStringLiteral("location"));
static const QString kNameKey(QStringLiteral("name"));
static const QString kOutFieldsKey(QStringLiteral("outFields"));
static const QString kCandidateFieldsKey(QStringLiteral("candidateFields"));
static const QString kCountriesKey(QStringLiteral("detailedCountries"));
static const QString kLocalizedNamesKey(QStringLiteral("localizedNames"));
static const QString kMaxLocationsKey(QStringLiteral("maxLocations"));

static const QUrl kUrlGeocodeServer("http://geocode.arcgis.com/arcgis/rest/services/World/GeocodeServer?f=pjson");
static const QUrl kUrlFindAddressCandidates("http://geocode.arcgis.com/arcgis/rest/services/World/GeocodeServer/findAddressCandidates");

PlaceManagerEngineEsri::PlaceManagerEngineEsri(const QVariantMap &parameters, QGeoServiceProvider::Error *error,
                                               QString *errorString) :
    QPlaceManagerEngine(parameters),
    m_networkManager(new QNetworkAccessManager(this))
{
    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

PlaceManagerEngineEsri::~PlaceManagerEngineEsri()
{
}

QList<QLocale> PlaceManagerEngineEsri::locales() const
{
    return m_locales;
}

void PlaceManagerEngineEsri::setLocales(const QList<QLocale> &locales)
{
    m_locales = locales;
}

/***** Search *****/

QPlaceSearchReply *PlaceManagerEngineEsri::search(const QPlaceSearchRequest &request)
{
    bool unsupported = false;

    // Only public visibility supported
    unsupported |= request.visibilityScope() != QLocation::UnspecifiedVisibility &&
            request.visibilityScope() != QLocation::PublicVisibility;
    unsupported |= request.searchTerm().isEmpty() && request.categories().isEmpty();

    if (unsupported)
        return QPlaceManagerEngine::search(request);

    QUrlQuery queryItems;
    queryItems.addQueryItem(QStringLiteral("f"), QStringLiteral("json"));

    const QGeoCoordinate center = request.searchArea().center();
    if (center.isValid())
    {
        const QString location = QString("%1,%2").arg(center.longitude()).arg(center.latitude());
        queryItems.addQueryItem(kLocationKey, location);
    }

    const QGeoRectangle boundingBox = request.searchArea().boundingGeoRectangle();
    if (!boundingBox.isEmpty())
    {
        const QString searchExtent = QString("%1,%2,%3,%4")
                .arg(boundingBox.topLeft().longitude())
                .arg(boundingBox.topLeft().latitude())
                .arg(boundingBox.bottomRight().longitude())
                .arg(boundingBox.bottomRight().latitude());
        queryItems.addQueryItem(QStringLiteral("searchExtent"), searchExtent);
    }

    if (!request.searchTerm().isEmpty())
        queryItems.addQueryItem(kSingleLineKey, request.searchTerm());

    QStringList categories;
    if (!request.categories().isEmpty())
    {
        foreach (const QPlaceCategory &placeCategory, request.categories())
            categories.append(placeCategory.categoryId());
        queryItems.addQueryItem("category", categories.join(","));
    }

    if (request.limit() > 0)
        queryItems.addQueryItem(kMaxLocationsKey, QString::number(request.limit()));

    queryItems.addQueryItem(kOutFieldsKey, QStringLiteral("*"));

    QUrl requestUrl(kUrlFindAddressCandidates);
    requestUrl.setQuery(queryItems);

    QNetworkRequest networkRequest(requestUrl);
    networkRequest.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply *networkReply = m_networkManager->get(networkRequest);

    PlaceSearchReplyEsri *reply = new PlaceSearchReplyEsri(request, networkReply, m_candidateFieldsLocale, m_countriesLocale, this);
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(reply, SIGNAL(error(QPlaceReply::Error,QString)), this, SLOT(replyError(QPlaceReply::Error,QString)));

    return reply;
}

void PlaceManagerEngineEsri::replyFinished()
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit finished(reply);
}

void PlaceManagerEngineEsri::replyError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit error(reply, errorCode, errorString);
}

/***** Categories *****/

QPlaceReply *PlaceManagerEngineEsri::initializeCategories()
{
    initializeGeocodeServer();

    PlaceCategoriesReplyEsri *reply = new PlaceCategoriesReplyEsri(this);
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(reply, SIGNAL(error(QPlaceReply::Error,QString)), this, SLOT(replyError(QPlaceReply::Error,QString)));

    // TODO delayed finished() emission
    if (!m_categories.isEmpty())
        reply->emitFinished();

    m_pendingCategoriesReply.append(reply);
    return reply;
}

void PlaceManagerEngineEsri::parseCategories(const QJsonArray &jsonArray, const QString &parentCategoryId)
{
    foreach (const QJsonValue &jsonValue, jsonArray)
    {
        if (!jsonValue.isObject())
            continue;

        const QJsonObject jsonCategory = jsonValue.toObject();
        const QString key = jsonCategory.value(kNameKey).toString();
        const QString localeName = localizedName(jsonCategory);

        if (key.isEmpty())
            continue;

        QPlaceCategory category;
        category.setCategoryId(key);
        category.setName(localeName.isEmpty() ? key : localeName); // localizedNames
        m_categories.insert(key, category);
        m_subcategories[parentCategoryId].append(key);
        m_parentCategory.insert(key, parentCategoryId);
        emit categoryAdded(category, parentCategoryId);

        if (jsonCategory.contains(kCategoriesKey))
        {
            const QJsonArray jsonArray = jsonCategory.value(kCategoriesKey).toArray();
            parseCategories(jsonArray, key);
        }
    }
}

QString PlaceManagerEngineEsri::parentCategoryId(const QString &categoryId) const
{
    return m_parentCategory.value(categoryId);
}

QStringList PlaceManagerEngineEsri::childCategoryIds(const QString &categoryId) const
{
    return m_subcategories.value(categoryId);
}

QPlaceCategory PlaceManagerEngineEsri::category(const QString &categoryId) const
{
    return m_categories.value(categoryId);
}

QList<QPlaceCategory> PlaceManagerEngineEsri::childCategories(const QString &parentId) const
{
    QList<QPlaceCategory> categories;
    foreach (const QString &id, m_subcategories.value(parentId))
        categories.append(m_categories.value(id));
    return categories;
}

void PlaceManagerEngineEsri::finishCategories()
{
    foreach (PlaceCategoriesReplyEsri *reply, m_pendingCategoriesReply)
        reply->emitFinished();
    m_pendingCategoriesReply.clear();
}

void PlaceManagerEngineEsri::errorCaterogies(const QString &error)
{
    foreach (PlaceCategoriesReplyEsri *reply, m_pendingCategoriesReply)
        reply->setError(QPlaceReply::CommunicationError, error);
}

/***** GeocodeServer *****/

void PlaceManagerEngineEsri::initializeGeocodeServer()
{
    // Only fetch categories once
    if (m_categories.isEmpty() && !m_geocodeServerReply)
    {
        m_geocodeServerReply = m_networkManager->get(QNetworkRequest(kUrlGeocodeServer));
        connect(m_geocodeServerReply, SIGNAL(finished()), this, SLOT(geocodeServerReplyFinished()));
        connect(m_geocodeServerReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(geocodeServerReplyError()));
    }
}

QString PlaceManagerEngineEsri::localizedName(const QJsonObject &jsonObject)
{
    const QJsonObject localizedNames = jsonObject.value(kLocalizedNamesKey).toObject();

    foreach (const QLocale &locale, m_locales)
    {
        const QString localeStr = locale.name();
        if (localizedNames.contains(localeStr))
        {
            return localizedNames.value(localeStr).toString();
        }

        const QString shortLocale = localeStr.left(2);
        if (localizedNames.contains(shortLocale))
        {
            return localizedNames.value(shortLocale).toString();
        }
    }
    return QString();
}

void PlaceManagerEngineEsri::parseCandidateFields(const QJsonArray &jsonArray)
{
    foreach (const QJsonValue &jsonValue, jsonArray)
    {
        if (!jsonValue.isObject())
            continue;

        const QJsonObject jsonCandidateField = jsonValue.toObject();
        if (!jsonCandidateField.contains(kLocalizedNamesKey))
            continue;

        const QString key = jsonCandidateField.value(kNameKey).toString();
        m_candidateFieldsLocale.insert(key, localizedName(jsonCandidateField));
    }
}

void PlaceManagerEngineEsri::parseCountries(const QJsonArray &jsonArray)
{
    foreach (const QJsonValue &jsonValue, jsonArray)
    {
        if (!jsonValue.isObject())
            continue;

        const QJsonObject jsonCountry = jsonValue.toObject();
        if (!jsonCountry.contains(kLocalizedNamesKey))
            continue;

        const QString key = jsonCountry.value(kNameKey).toString();
        m_countriesLocale.insert(key, localizedName(jsonCountry));
    }
}

void PlaceManagerEngineEsri::geocodeServerReplyFinished()
{
    if (!m_geocodeServerReply)
        return;

    QJsonDocument document = QJsonDocument::fromJson(m_geocodeServerReply->readAll());
    if (!document.isObject())
    {
        errorCaterogies(m_geocodeServerReply->errorString());
        return;
    }

    QJsonObject jsonObject = document.object();

    // parse categories
    if (jsonObject.contains(kCategoriesKey))
    {
        const QJsonArray jsonArray = jsonObject.value(kCategoriesKey).toArray();
        parseCategories(jsonArray, QString());
    }

    // parse candidateFields
    if (jsonObject.contains(kCandidateFieldsKey))
    {
        const QJsonArray jsonArray = jsonObject.value(kCandidateFieldsKey).toArray();
        parseCandidateFields(jsonArray);
    }

    // parse countries
    if (jsonObject.contains(kCountriesKey))
    {
        const QJsonArray jsonArray = jsonObject.value(kCountriesKey).toArray();
        parseCountries(jsonArray);
    }

    finishCategories();

    m_geocodeServerReply->deleteLater();
}

void PlaceManagerEngineEsri::geocodeServerReplyError()
{
    if (m_categories.isEmpty() && !m_geocodeServerReply)
        return;

    errorCaterogies(m_geocodeServerReply->errorString());
}

QT_END_NAMESPACE
