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

#ifndef QPLACEMANAGERENGINEMAPBOX_H
#define QPLACEMANAGERENGINEMAPBOX_H

#include <QtLocation/QPlaceManagerEngine>
#include <QtLocation/QGeoServiceProvider>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;

class QPlaceManagerEngineMapbox : public QPlaceManagerEngine
{
    Q_OBJECT

public:
    QPlaceManagerEngineMapbox(const QVariantMap &parameters, QGeoServiceProvider::Error *,
                           QString *errorString);
    ~QPlaceManagerEngineMapbox();

    QPlaceSearchReply *search(const QPlaceSearchRequest &) override;

    QPlaceSearchSuggestionReply *searchSuggestions(const QPlaceSearchRequest &) override;

    QPlaceReply *initializeCategories() override;
    QString parentCategoryId(const QString &categoryId) const override;
    QStringList childCategoryIds(const QString &categoryId) const override;
    QPlaceCategory category(const QString &categoryId) const override;
    QList<QPlaceCategory> childCategories(const QString &parentId) const override;

    QList<QLocale> locales() const override;
    void setLocales(const QList<QLocale> &locales) override;

    // TODO: icon
    //QPlaceIcon icon(const QString &remotePath,
    //                const QList<QPlaceCategory> &categories = QList<QPlaceCategory>()) const;

    //QUrl constructIconUrl(const QPlaceIcon &icon, const QSize &size) const override;

private slots:
    void onReplyFinished();
    void onReplyError(QPlaceReply::Error, const QString &errorString);

private:
    enum PlaceSearchType {
        CompleteSearch = 0,
        SuggestionSearch
    };

    QPlaceReply *doSearch(const QPlaceSearchRequest&, PlaceSearchType);

    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
    QString m_accessToken;
    QString m_urlPrefix;
    bool m_isEnterprise;

    QList<QLocale> m_locales;
    QHash<QString, QPlaceCategory> m_categories;
};

QT_END_NAMESPACE

#endif // QPLACEMANAGERENGINEMAPBOX_H
