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

#ifndef QGEOTILEDMAPPINGMANAGERENGINE_TEST_H
#define QGEOTILEDMAPPINGMANAGERENGINE_TEST_H

#include <QtCore/QLocale>
#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>
#include <QtLocation/private/qgeotiledmapreply_p.h>
#include <QtLocation/private/qgeomaptype_p.h>
#include <QtLocation/private/qgeocameracapabilities_p.h>

#include "qgeotiledmap_test.h"
#include "qgeotilefetcher_test.h"

QT_USE_NAMESPACE

class QGeoTiledMappingManagerEngineTest: public QGeoTiledMappingManagerEngine
{
Q_OBJECT
public:
    QGeoTiledMappingManagerEngineTest(const QVariantMap &parameters,
        QGeoServiceProvider::Error *error, QString *errorString) :
        QGeoTiledMappingManagerEngine()
    {
        Q_UNUSED(error)
        Q_UNUSED(errorString)

        setLocale(QLocale (QLocale::German, QLocale::Germany));
        QGeoCameraCapabilities capabilities;
        capabilities.setMinimumZoomLevel(0.0);
        capabilities.setMaximumZoomLevel(20.0);
        capabilities.setSupportsBearing(true);
        capabilities.setSupportsTilting(true);
        capabilities.setMinimumTilt(0);
        capabilities.setMaximumTilt(60);
        setTileSize(QSize(256, 256));

        const QByteArray pluginName = "qmlgeo.test.plugin";
        QList<QGeoMapType> mapTypes;

        mapTypes << QGeoMapType(QGeoMapType::StreetMap, tr("StreetMap"), tr("StreetMap"), false, false, 1, pluginName, capabilities);
        mapTypes << QGeoMapType(QGeoMapType::SatelliteMapDay, tr("SatelliteMapDay"), tr("SatelliteMapDay"), false, false, 2, pluginName, capabilities);
        mapTypes << QGeoMapType(QGeoMapType::CycleMap, tr("CycleMap"), tr("CycleMap"), false, false, 3, pluginName, capabilities);

        QGeoCameraCapabilities capabilities4;
        capabilities4.setMinimumZoomLevel(0.0);
        capabilities4.setMaximumZoomLevel(19.0);
        capabilities4.setSupportsBearing(true);
        capabilities4.setSupportsTilting(true);
        capabilities4.setMinimumTilt(0);
        capabilities4.setMaximumTilt(80);
        capabilities4.setMinimumFieldOfView(1);
        capabilities4.setMaximumFieldOfView(179);
        QVariantMap meta;
        meta["foo"] = 42;
        mapTypes << QGeoMapType(QGeoMapType::CustomMap, tr("AlternateCameraCapabilities"), tr("AlternateCameraCapabilities"), false, false, 4, pluginName, capabilities4, meta);

        if (parameters.contains(QStringLiteral("extraMapTypeName"))) {
            QString  extraMapTypeName = parameters.value(QStringLiteral("extraMapTypeName")).toString();
            mapTypes << QGeoMapType(QGeoMapType::CustomMap, extraMapTypeName, extraMapTypeName, false, false, 5, pluginName, capabilities);
        }

        setSupportedMapTypes(mapTypes);

        QGeoTileFetcherTest *fetcher = new QGeoTileFetcherTest(this);
        if (parameters.contains(QStringLiteral("finishRequestImmediately")))
            fetcher->setFinishRequestImmediately(parameters.value(QStringLiteral("finishRequestImmediately")).toBool());
        if (parameters.contains(QStringLiteral("tileSize"))) {
            int tileSize = parameters.value(QStringLiteral("tileSize")).toInt();
            setTileSize(QSize(tileSize, tileSize));
        }
        if (parameters.contains(QStringLiteral("maxZoomLevel"))) {
            double maxZoomLevel = parameters.value(QStringLiteral("maxZoomLevel")).toDouble();
            capabilities.setMaximumZoomLevel(maxZoomLevel);
        }
        if (parameters.contains(QStringLiteral("supportVisibleArea"))) {
            bool supportVisibleArea = parameters.value(QStringLiteral("supportVisibleArea")).toBool();
            m_supportVisibleArea = supportVisibleArea;
        }

        setCameraCapabilities(capabilities);
        fetcher->setTileSize(tileSize());
        setTileFetcher(fetcher);
    }

    QGeoMap *createMap() override
    {
        QGeoTiledMapTestOptions opts;
        opts.supportVisibleArea = m_supportVisibleArea;
        return new QGeoTiledMapTest(this, opts);
    }

    bool m_supportVisibleArea = true;
};

#endif
