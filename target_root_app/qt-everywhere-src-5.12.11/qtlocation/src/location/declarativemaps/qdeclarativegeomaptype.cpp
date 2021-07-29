/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#include "qdeclarativegeomaptype_p.h"
#include <qnumeric.h>
#include <QtQml/qqml.h>
#include <QDebug>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MapType
    \instantiates QDeclarativeGeoMapType
    \inherits QObject
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.5

    \brief The MapType type holds information about a map type.

    This includes the map type's \l name and \l description, the \l style and
    a flag to indicate if the map type is optimized for mobile devices (\l mobile).
*/

QDeclarativeGeoMapType::QDeclarativeGeoMapType(const QGeoMapType mapType, QObject *parent)
    : QObject(parent),
      mapType_(mapType),
      cameraCapabilities_(new QDeclarativeGeoCameraCapabilities(mapType.cameraCapabilities(), this)) {}

QDeclarativeGeoMapType::~QDeclarativeGeoMapType() {}

/*!
    \qmlproperty enumeration MapType::style

    This read-only property gives access to the style of the map type.

    \list
    \li MapType.NoMap - No map.
    \li MapType.StreetMap - A street map.
    \li MapType.SatelliteMapDay - A map with day-time satellite imagery.
    \li MapType.SatelliteMapNight - A map with night-time satellite imagery.
    \li MapType.TerrainMap - A terrain map.
    \li MapType.HybridMap - A map with satellite imagery and street information.
    \li MapType.GrayStreetMap - A gray-shaded street map.
    \li MapType.PedestrianMap - A street map suitable for pedestriants.
    \li MapType.CarNavigationMap - A street map suitable for car navigation.
    \li MapType.CycleMap - A street map suitable for cyclists.
    \li MapType.CustomMap - A custom map type.
    \endlist
*/
QDeclarativeGeoMapType::MapStyle QDeclarativeGeoMapType::style() const
{
    return QDeclarativeGeoMapType::MapStyle(mapType_.style());
}

/*!
    \qmlproperty string MapType::name

    This read-only property holds the name of the map type as a single formatted string.
*/
QString QDeclarativeGeoMapType::name() const
{
    return mapType_.name();
}

/*!
    \qmlproperty string MapType::description

    This read-only property holds the description of the map type as a single formatted string.
*/
QString QDeclarativeGeoMapType::description() const
{
    return mapType_.description();
}

/*!
    \qmlproperty bool MapType::mobile

    \brief Whether the map type is optimized for the use on a mobile device.

    Map types for mobile devices usually have higher constrast to counteract the
    effects of sunlight and a reduced color for improved readability.
*/
bool QDeclarativeGeoMapType::mobile() const
{
    return mapType_.mobile();
}

/*!
    \qmlproperty bool MapType::night
    \since QtLocation 5.4

    \brief Whether the map type is optimized for use at night.

    Map types suitable for use at night usually have a dark background.
*/
bool QDeclarativeGeoMapType::night() const
{
    return mapType_.night();
}

/*!
    \qmlproperty CameraCapabilities MapType::cameraCapabilities
    \since QtLocation 5.10

    This property holds the camera capabilities for this map type.
*/
QDeclarativeGeoCameraCapabilities *QDeclarativeGeoMapType::cameraCapabilities() const
{
    return cameraCapabilities_;
}

/*!
    \qmlproperty VariantMap MapType::metadata
    \since QtLocation 5.10

    This property holds optional, extra metadata related to a specific map type.
    The content of this property is entirely plugin-specific.
*/
QVariantMap QDeclarativeGeoMapType::metadata() const
{
    return mapType_.metadata();
}

/*
 *      QDeclarativeGeoCameraCapabilities implementation
 */

/*!
    \qmltype CameraCapabilities
    \instantiates QDeclarativeGeoCameraCapabilities
    \inherits QObject
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.10

    \brief The CameraCapabilities type holds information about the camera capabilities for a specific map type.

    This includes the map minimum and maximum zoom level, minimum and maximum tilt angle and
    minimum and maximum field of view.
*/

QDeclarativeGeoCameraCapabilities::QDeclarativeGeoCameraCapabilities(const QGeoCameraCapabilities &cameraCaps, QObject *parent)
    : QObject(parent), cameraCaps_(cameraCaps)
{

}

QDeclarativeGeoCameraCapabilities::~QDeclarativeGeoCameraCapabilities()
{

}

/*!
    \qmlproperty qreal CameraCapabilities::minimumZoomLevel

    This read-only property holds the minimum available zoom level with this map type.
*/
qreal QDeclarativeGeoCameraCapabilities::minimumZoomLevel() const
{
    return cameraCaps_.minimumZoomLevelAt256();
}

/*!
    \qmlproperty qreal CameraCapabilities::maximumZoomLevel

    This read-only property holds the maximum available zoom level with this map type.
*/
qreal QDeclarativeGeoCameraCapabilities::maximumZoomLevel() const
{
    return cameraCaps_.maximumZoomLevelAt256();
}

/*!
    \qmlproperty qreal CameraCapabilities::minimumTilt

    This read-only property holds the minimum available tilt with this map type.
*/
qreal QDeclarativeGeoCameraCapabilities::minimumTilt() const
{
    return cameraCaps_.minimumTilt();
}

/*!
    \qmlproperty qreal CameraCapabilities::maximumTilt

    This read-only property holds the maximum available tilt with this map type.
*/
qreal QDeclarativeGeoCameraCapabilities::maximumTilt() const
{
    return cameraCaps_.maximumTilt();
}

/*!
    \qmlproperty qreal CameraCapabilities::minimumFieldOfView

    This read-only property holds the minimum available field of view with this map type.
*/
qreal QDeclarativeGeoCameraCapabilities::minimumFieldOfView() const
{
    return cameraCaps_.minimumFieldOfView();
}

/*!
    \qmlproperty qreal CameraCapabilities::maximumFieldOfView

    This read-only property holds the maximum available field of view with this map type.
*/
qreal QDeclarativeGeoCameraCapabilities::maximumFieldOfView() const
{
    return cameraCaps_.maximumFieldOfView();
}

QT_END_NAMESPACE
