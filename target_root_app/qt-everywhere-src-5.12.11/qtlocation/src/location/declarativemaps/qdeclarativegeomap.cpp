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

#include "qdeclarativegeomap_p.h"
#include "qdeclarativegeomapquickitem_p.h"
#include "qdeclarativegeomapcopyrightsnotice_p.h"
#include "qdeclarativegeoserviceprovider_p.h"
#include "qdeclarativegeomaptype_p.h"
#include "qgeomappingmanager_p.h"
#include "qgeocameracapabilities_p.h"
#include "qgeomap_p.h"
#include "qdeclarativegeomapparameter_p.h"
#include "qgeomapobject_p.h"
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoRectangle>
#include <QtPositioning/QGeoPath>
#include <QtPositioning/QGeoPolygon>
#include <QtQuick/QQuickWindow>
#include <QtQuick/QSGRectangleNode>
#include <QtQuick/private/qquickwindow_p.h>
#include <QtQml/qqmlinfo.h>
#include <QtQuick/private/qquickitem_p.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.141592653589793238463
#endif


QT_BEGIN_NAMESPACE

static qreal sanitizeBearing(qreal bearing)
{
    bearing = std::fmod(bearing, qreal(360.0));
    if (bearing < 0.0)
        bearing += 360.0;

    return bearing;
}

/*!
    \qmltype Map
    \instantiates QDeclarativeGeoMap
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.0

    \brief The Map type displays a map.

    The Map type is used to display a map or image of the Earth, with
    the capability to also display interactive objects tied to the map's
    surface.

    There are a variety of different ways to visualize the Earth's surface
    in a 2-dimensional manner, but all of them involve some kind of projection:
    a mathematical relationship between the 3D coordinates (latitude, longitude
    and altitude) and 2D coordinates (X and Y in pixels) on the screen.

    Different sources of map data can use different projections, and from the
    point of view of the Map type, we treat these as one replaceable unit:
    the Map plugin. A Map plugin consists of a data source, as well as all other
    details needed to display its data on-screen.

    The current Map plugin in use is contained in the \l plugin property of
    the Map item. In order to display any image in a Map item, you will need
    to set this property. See the \l Plugin type for a description of how
    to retrieve an appropriate plugin for use.

    The geographic region displayed in the Map item is referred to as its
    viewport, and this is defined by the properties \l center, and
    \l zoomLevel. The \l center property contains a \l {coordinate}
    specifying the center of the viewport, while \l zoomLevel controls the scale of the
    map. See each of these properties for further details about their values.

    When the map is displayed, each possible geographic coordinate that is
    visible will map to some pixel X and Y coordinate on the screen. To perform
    conversions between these two, Map provides the \l toCoordinate and
    \l fromCoordinate functions, which are of general utility.

    \section2 Map Objects

    Map related objects can be declared within the body of a Map object in Qt Quick and will
    automatically appear on the Map. To add an object programmatically, first be
    sure it is created with the Map as its parent (for example in an argument to
    Component::createObject).
    Then call the \l addMapItem method on the Map, if the type of this object is one of
    \l MapCircle, \l MapRectangle, \l MapPolyline, \l MapPolygon, \l MapRoute or \l MapQuickItem.
    A corresponding \l removeMapItem method also exists to do the opposite and
    remove any of the above types of map objects from the Map.

    Moving Map objects around, resizing them or changing their shape normally
    does not involve any special interaction with Map itself -- changing these
    properties in a map object will automatically update the display.

    \section2 Interaction

    The Map type includes support for pinch and flick gestures to control
    zooming and panning. These are enabled by default, and available at any
    time by using the \l gesture object. The actual GestureArea is constructed
    specially at startup and cannot be replaced or destroyed. Its properties
    can be altered, however, to control its behavior.

    \section2 Performance

    Maps are rendered using OpenGL (ES) and the Qt Scene Graph stack, and as
    a result perform quite well where GL accelerated hardware is available.

    For "online" Map plugins, network bandwidth and latency can be major
    contributors to the user's perception of performance. Extensive caching is
    performed to mitigate this, but such mitigation is not always perfect. For
    "offline" plugins, the time spent retrieving the stored geographic data
    and rendering the basic map features can often play a dominant role. Some
    offline plugins may use hardware acceleration themselves to (partially)
    avert this.

    In general, large and complex Map items such as polygons and polylines with
    large numbers of vertices can have an adverse effect on UI performance.
    Further, more detailed notes on this are in the documentation for each
    map item type.

    \section2 Example Usage

    The following snippet shows a simple Map and the necessary Plugin type
    to use it. The map is centered over Oslo, Norway, with zoom level 14.

    \quotefromfile minimal_map/main.qml
    \skipto import
    \printuntil }
    \printline }
    \skipto Map
    \printuntil }
    \printline }

    \image minimal_map.png
*/

/*!
    \qmlsignal QtLocation::Map::copyrightLinkActivated(string link)

    This signal is emitted when the user clicks on a \a link in the copyright notice. The
    application should open the link in a browser or display its contents to the user.
*/

QDeclarativeGeoMap::QDeclarativeGeoMap(QQuickItem *parent)
        : QQuickItem(parent),
        m_plugin(0),
        m_mappingManager(0),
        m_activeMapType(0),
        m_gestureArea(new QQuickGeoMapGestureArea(this)),
        m_map(0),
        m_error(QGeoServiceProvider::NoError),
        m_color(QColor::fromRgbF(0.9, 0.9, 0.9)),
        m_componentCompleted(false),
        m_pendingFitViewport(false),
        m_copyrightsVisible(true),
        m_maximumViewportLatitude(0.0),
        m_initialized(false),
        m_userMinimumZoomLevel(qQNaN()),
        m_userMaximumZoomLevel(qQNaN()),
        m_userMinimumTilt(qQNaN()),
        m_userMaximumTilt(qQNaN()),
        m_userMinimumFieldOfView(qQNaN()),
        m_userMaximumFieldOfView(qQNaN())
{
    setAcceptHoverEvents(false);
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlags(QQuickItem::ItemHasContents | QQuickItem::ItemClipsChildrenToShape);
    setFiltersChildMouseEvents(true);

    m_activeMapType = new QDeclarativeGeoMapType(QGeoMapType(QGeoMapType::NoMap,
                                                             tr("No Map"),
                                                             tr("No Map"),
                                                             false, false,
                                                             0,
                                                             QByteArrayLiteral(""),
                                                             QGeoCameraCapabilities()), this);
    m_cameraData.setCenter(QGeoCoordinate(51.5073,-0.1277)); //London city center
    m_cameraData.setZoomLevel(8.0);

    m_cameraCapabilities.setTileSize(256);
    m_cameraCapabilities.setSupportsBearing(true);
    m_cameraCapabilities.setSupportsTilting(true);
    m_cameraCapabilities.setMinimumZoomLevel(0);
    m_cameraCapabilities.setMaximumZoomLevel(30);
    m_cameraCapabilities.setMinimumTilt(0);
    m_cameraCapabilities.setMaximumTilt(89.5);
    m_cameraCapabilities.setMinimumFieldOfView(1);
    m_cameraCapabilities.setMaximumFieldOfView(179);

    m_minimumTilt = m_cameraCapabilities.minimumTilt();
    m_maximumTilt = m_cameraCapabilities.maximumTilt();
    m_minimumFieldOfView = m_cameraCapabilities.minimumFieldOfView();
    m_maximumFieldOfView = m_cameraCapabilities.maximumFieldOfView();
}

QDeclarativeGeoMap::~QDeclarativeGeoMap()
{
    // Removing map parameters and map items from m_map
    if (m_map) {
        m_map->clearParameters();
        m_map->clearMapItems();
    }

    // Remove the items from the map, making them deletable.
    // Go in the same order as in removeMapChild: views, groups, then items
    if (!m_mapViews.isEmpty()) {
        const auto mapViews = m_mapViews;
        for (QDeclarativeGeoMapItemView *v : mapViews) { // so that removeMapItemView_real can safely modify m_mapViews;
            if (!v)
                continue;

            QQuickItem *parent = v->parentItem();
            QDeclarativeGeoMapItemGroup *group = qobject_cast<QDeclarativeGeoMapItemGroup *>(parent);
            if (group)
                continue; // Ignore non-top-level MIVs. They will be recursively processed.
                          // Identify them as being parented by a MapItemGroup.

            removeMapItemView_real(v);
        }
    }

    if (!m_mapItemGroups.isEmpty()) {
        const auto mapGroups = m_mapItemGroups;
        for (QDeclarativeGeoMapItemGroup *g : mapGroups) {
            if (!g)
                continue;

            QQuickItem *parent =g->parentItem();
            QDeclarativeGeoMapItemGroup *group = qobject_cast<QDeclarativeGeoMapItemGroup *>(parent);
            if (group)
                continue; // Ignore non-top-level Groups. They will be recursively processed.
                          // Identify them as being parented by a MapItemGroup.

            removeMapItemGroup_real(g);
        }
    }

    // remove any remaining map items associations
    const auto mapItems = m_mapItems;
    for (auto mi: mapItems)
        removeMapItem_real(mi.data());

    if (m_copyrights.data())
        delete m_copyrights.data();
    m_copyrights.clear();

    for (auto obj: qAsConst(m_pendingMapObjects))
        obj->setMap(nullptr); // worst case: going to be setMap(nullptr)'d twice

    delete m_map; // map objects get reset here
}

static QDeclarativeGeoMapType *findMapType(const QList<QDeclarativeGeoMapType *> &types, const QGeoMapType &type)
{
    for (int i = 0; i < types.size(); ++i)
        if (types[i]->mapType() == type)
            return types[i];
    return nullptr;
}

void QDeclarativeGeoMap::onSupportedMapTypesChanged()
{
    QList<QDeclarativeGeoMapType *> supportedMapTypes;
    QList<QGeoMapType> types = m_mappingManager->supportedMapTypes();
    for (int i = 0; i < types.size(); ++i) {
        // types that are present and get removed will be deleted at QObject destruction
        QDeclarativeGeoMapType *type = findMapType(m_supportedMapTypes, types[i]);
        if (!type)
            type = new QDeclarativeGeoMapType(types[i], this);
        supportedMapTypes.append(type);
    }
    m_supportedMapTypes.swap(supportedMapTypes);
    if (m_supportedMapTypes.isEmpty()) {
        m_map->setActiveMapType(QGeoMapType()); // no supported map types: setting an invalid one
    } else {
        bool hasMapType = false;
        foreach (QDeclarativeGeoMapType *declarativeType, m_supportedMapTypes) {
            if (declarativeType->mapType() == m_map->activeMapType())
                hasMapType = true;
        }
        if (!hasMapType) {
            QDeclarativeGeoMapType *type = m_supportedMapTypes.at(0);
            m_activeMapType = type;
            m_map->setActiveMapType(type->mapType());
        }
    }

    emit supportedMapTypesChanged();
}

void QDeclarativeGeoMap::setError(QGeoServiceProvider::Error error, const QString &errorString)
{
    if (m_error == error && m_errorString == errorString)
        return;
    m_error = error;
    m_errorString = errorString;
    emit errorChanged();
}

/*!
    \internal
    Called when the mapping manager is initialized AND the declarative element has a valid size > 0
*/
void QDeclarativeGeoMap::initialize()
{
    // try to keep change signals in the end
    bool visibleAreaHasChanged = false;

    QGeoCoordinate center = m_cameraData.center();

    if (!qIsFinite(m_userMinimumZoomLevel))
        setMinimumZoomLevel(m_map->minimumZoom(), false);
    else
        setMinimumZoomLevel(qMax<qreal>(m_map->minimumZoom(), m_userMinimumZoomLevel), false);

    double bearing = m_cameraData.bearing();
    double tilt = m_cameraData.tilt();
    double fov = m_cameraData.fieldOfView(); // Must be 45.0
    QGeoCameraData cameraData = m_cameraData;

    if (!m_cameraCapabilities.supportsBearing() && bearing != 0.0)
        cameraData.setBearing(0);

    if (!m_cameraCapabilities.supportsTilting() && tilt != 0.0)
        cameraData.setTilt(0);

    m_map->setVisibleArea(m_visibleArea);
    if (m_map->visibleArea() != m_visibleArea)
        visibleAreaHasChanged = true;

    cameraData.setFieldOfView(qBound(m_cameraCapabilities.minimumFieldOfView(),
                                       fov,
                                       m_cameraCapabilities.maximumFieldOfView()));

    // set latitude boundary check
    m_maximumViewportLatitude = m_map->maximumCenterLatitudeAtZoom(cameraData);
    m_minimumViewportLatitude = m_map->minimumCenterLatitudeAtZoom(cameraData);

    center.setLatitude(qBound(m_minimumViewportLatitude, center.latitude(), m_maximumViewportLatitude));
    cameraData.setCenter(center);

    connect(m_map.data(), &QGeoMap::cameraDataChanged,
            this,  &QDeclarativeGeoMap::onCameraDataChanged);
    m_map->setCameraData(cameraData);

    for (auto obj : qAsConst(m_pendingMapObjects))
        obj->setMap(m_map);

    m_initialized = true;

    if (visibleAreaHasChanged)
        emit visibleAreaChanged();
    connect(m_map.data(), &QGeoMap::visibleAreaChanged, this, &QDeclarativeGeoMap::visibleAreaChanged);

    emit mapReadyChanged(true);

    if (m_copyrights) // To not update during initialize()
         update();
}

/*!
    \internal
*/
void QDeclarativeGeoMap::pluginReady()
{
    QGeoServiceProvider *provider = m_plugin->sharedGeoServiceProvider();
    m_mappingManager = provider->mappingManager();

    if (provider->error() != QGeoServiceProvider::NoError) {
        setError(provider->error(), provider->errorString());
        return;
    }

    if (!m_mappingManager) {
        //TODO Should really be EngineNotSetError (see QML GeoCodeModel)
        setError(QGeoServiceProvider::NotSupportedError, tr("Plugin does not support mapping."));
        return;
    }

    if (!m_mappingManager->isInitialized())
        connect(m_mappingManager, SIGNAL(initialized()), this, SLOT(mappingManagerInitialized()));
    else
        mappingManagerInitialized();

    // make sure this is only called once
    disconnect(this, SLOT(pluginReady()));
}

/*!
    \internal
*/
void QDeclarativeGeoMap::componentComplete()
{
    m_componentCompleted = true;
    populateParameters();
    populateMap();
    QQuickItem::componentComplete();
}

/*!
    \qmlproperty MapGestureArea QtLocation::Map::gesture

    Contains the MapGestureArea created with the Map. This covers pan, flick and pinch gestures.
    Use \c{gesture.enabled: true} to enable basic gestures, or see \l{MapGestureArea} for
    further details.
*/

QQuickGeoMapGestureArea *QDeclarativeGeoMap::gesture()
{
    return m_gestureArea;
}

/*!
    \internal

    This may happen before mappingManagerInitialized()
*/
void QDeclarativeGeoMap::populateMap()
{
    QSet<QObject *> kids = children().toSet();
    const QList<QQuickItem *> quickKids = childItems();
    for (QQuickItem *ite: quickKids)
        kids.insert(ite);

    for (QObject *k : qAsConst(kids)) {
        addMapChild(k);
    }
}

void QDeclarativeGeoMap::populateParameters()
{
    QObjectList kids = children();
    QList<QQuickItem *> quickKids = childItems();
    for (int i = 0; i < quickKids.count(); ++i)
        kids.append(quickKids.at(i));
    for (int i = 0; i < kids.size(); ++i) {
        QDeclarativeGeoMapParameter *mapParameter = qobject_cast<QDeclarativeGeoMapParameter *>(kids.at(i));
        if (mapParameter)
            addMapParameter(mapParameter);
    }
}

/*!
    \internal
*/
void QDeclarativeGeoMap::setupMapView(QDeclarativeGeoMapItemView *view)
{
    view->setMap(this);
}

/*!
 * \internal
 */
QSGNode *QDeclarativeGeoMap::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (!m_map) {
        delete oldNode;
        return 0;
    }

    QSGRectangleNode *root = static_cast<QSGRectangleNode *>(oldNode);
    if (!root)
        root = window()->createRectangleNode();

    root->setRect(boundingRect());
    root->setColor(m_color);

    QSGNode *content = root->childCount() ? root->firstChild() : 0;
    content = m_map->updateSceneGraph(content, window());
    if (content && root->childCount() == 0)
        root->appendChildNode(content);

    return root;
}

/*!
    \qmlproperty Plugin QtLocation::Map::plugin

    This property holds the plugin which provides the mapping functionality.

    This is a write-once property. Once the map has a plugin associated with
    it, any attempted modifications of the plugin will be ignored.
*/

void QDeclarativeGeoMap::setPlugin(QDeclarativeGeoServiceProvider *plugin)
{
    if (m_plugin) {
        qmlWarning(this) << QStringLiteral("Plugin is a write-once property, and cannot be set again.");
        return;
    }
    m_plugin = plugin;
    emit pluginChanged(m_plugin);

    if (m_plugin->isAttached()) {
        pluginReady();
    } else {
        connect(m_plugin, SIGNAL(attached()),
                this, SLOT(pluginReady()));
    }
}

/*!
    \internal
*/
void QDeclarativeGeoMap::onCameraCapabilitiesChanged(const QGeoCameraCapabilities &oldCameraCapabilities)
{
    if (m_map->cameraCapabilities() == oldCameraCapabilities)
        return;
    m_cameraCapabilities = m_map->cameraCapabilities();

    //The zoom level limits are only restricted by the plugins values, if the user has set a more
    //strict zoom level limit before initialization nothing is done here.
    //minimum zoom level might be changed to limit gray bundaries
    //This code assumes that plugins' maximum zoom level will never exceed 30.0
    if (m_cameraCapabilities.maximumZoomLevelAt256() < m_gestureArea->maximumZoomLevel()) {
        setMaximumZoomLevel(m_cameraCapabilities.maximumZoomLevelAt256(), false);
    } else if (m_cameraCapabilities.maximumZoomLevelAt256() > m_gestureArea->maximumZoomLevel()) {
        if (!qIsFinite(m_userMaximumZoomLevel)) {
            // If the user didn't set anything
            setMaximumZoomLevel(m_cameraCapabilities.maximumZoomLevelAt256(), false);
        } else {  // Try to set what the user requested
            // Else if the user set something larger, but that got clamped by the previous camera caps
            setMaximumZoomLevel(qMin<qreal>(m_cameraCapabilities.maximumZoomLevelAt256(),
                                            m_userMaximumZoomLevel), false);
        }
    }

    if (m_cameraCapabilities.minimumZoomLevelAt256() > m_gestureArea->minimumZoomLevel()) {
        setMinimumZoomLevel(m_cameraCapabilities.minimumZoomLevelAt256(), false);
    } else if (m_cameraCapabilities.minimumZoomLevelAt256() < m_gestureArea->minimumZoomLevel()) {
        if (!qIsFinite(m_userMinimumZoomLevel)) {
            // If the user didn't set anything, trying to set the new caps.
            setMinimumZoomLevel(m_cameraCapabilities.minimumZoomLevelAt256(), false);
        } else {  // Try to set what the user requested
            // Else if the user set a minimum, m_gestureArea->minimumZoomLevel() might be larger
            // because of different reasons. Resetting it, as if it ends to be the same,
            // no signal will be emitted.
            setMinimumZoomLevel(qMax<qreal>(m_cameraCapabilities.minimumZoomLevelAt256(),
                                            m_userMinimumZoomLevel), false);
        }
    }

    // Tilt
    if (m_cameraCapabilities.maximumTilt() < m_maximumTilt) {
        setMaximumTilt(m_cameraCapabilities.maximumTilt(), false);
    } else if (m_cameraCapabilities.maximumTilt() > m_maximumTilt) {
        if (!qIsFinite(m_userMaximumTilt))
            setMaximumTilt(m_cameraCapabilities.maximumTilt(), false);
        else // Try to set what the user requested
            setMaximumTilt(qMin<qreal>(m_cameraCapabilities.maximumTilt(), m_userMaximumTilt), false);
    }

    if (m_cameraCapabilities.minimumTilt() > m_minimumTilt) {
        setMinimumTilt(m_cameraCapabilities.minimumTilt(), false);
    } else if (m_cameraCapabilities.minimumTilt() < m_minimumTilt) {
        if (!qIsFinite(m_userMinimumTilt))
            setMinimumTilt(m_cameraCapabilities.minimumTilt(), false);
        else // Try to set what the user requested
            setMinimumTilt(qMax<qreal>(m_cameraCapabilities.minimumTilt(), m_userMinimumTilt), false);
    }

    // FoV
    if (m_cameraCapabilities.maximumFieldOfView() < m_maximumFieldOfView) {
        setMaximumFieldOfView(m_cameraCapabilities.maximumFieldOfView(), false);
    } else if (m_cameraCapabilities.maximumFieldOfView() > m_maximumFieldOfView) {
        if (!qIsFinite(m_userMaximumFieldOfView))
            setMaximumFieldOfView(m_cameraCapabilities.maximumFieldOfView(), false);
        else // Try to set what the user requested
            setMaximumFieldOfView(qMin<qreal>(m_cameraCapabilities.maximumFieldOfView(), m_userMaximumFieldOfView), false);
    }

    if (m_cameraCapabilities.minimumFieldOfView() > m_minimumFieldOfView) {
        setMinimumFieldOfView(m_cameraCapabilities.minimumFieldOfView(), false);
    } else if (m_cameraCapabilities.minimumFieldOfView() < m_minimumFieldOfView) {
        if (!qIsFinite(m_userMinimumFieldOfView))
            setMinimumFieldOfView(m_cameraCapabilities.minimumFieldOfView(), false);
        else // Try to set what the user requested
            setMinimumFieldOfView(qMax<qreal>(m_cameraCapabilities.minimumFieldOfView(), m_userMinimumFieldOfView), false);
    }
}

/*!
    \internal
    this function will only be ever called once
*/
void QDeclarativeGeoMap::mappingManagerInitialized()
{
    m_map = m_mappingManager->createMap(this);

    if (!m_map)
        return;

    // Any map items that were added before the plugin was ready
    // need to have setMap called again
    for (const QPointer<QDeclarativeGeoMapItemBase> &item : qAsConst(m_mapItems)) {
        if (item) {
            item->setMap(this, m_map);
            m_map->addMapItem(item.data()); // m_map filters out what is not supported.
        }
    }

    /* COPY NOTICE SETUP */
    m_copyrights = new QDeclarativeGeoMapCopyrightNotice(this);
    m_copyrights->setCopyrightsZ(m_maxChildZ + 1);
    m_copyrights->setCopyrightsVisible(m_copyrightsVisible);
    m_copyrights->setMapSource(this);

    m_gestureArea->setMap(m_map);

    QList<QGeoMapType> types = m_mappingManager->supportedMapTypes();
    for (int i = 0; i < types.size(); ++i) {
        QDeclarativeGeoMapType *type = new QDeclarativeGeoMapType(types[i], this);
        m_supportedMapTypes.append(type);
    }

    if (m_activeMapType && m_plugin->name().toLatin1() == m_activeMapType->mapType().pluginName()) {
        m_map->setActiveMapType(m_activeMapType->mapType());
    } else {
        if (m_activeMapType)
            m_activeMapType->deleteLater();

        if (!m_supportedMapTypes.isEmpty()) {
                m_activeMapType = m_supportedMapTypes.at(0);
                m_map->setActiveMapType(m_activeMapType->mapType());
        } else {
            m_activeMapType = new QDeclarativeGeoMapType(QGeoMapType(QGeoMapType::NoMap,
                                                                     tr("No Map"),
                                                                     tr("No Map"),
                                                                     false,
                                                                     false,
                                                                     0,
                                                                     QByteArrayLiteral(""),
                                                                     QGeoCameraCapabilities()), this);
        }
    }

    // Update camera capabilities
    onCameraCapabilitiesChanged(m_cameraCapabilities);

    // Map tiles are built in this call. m_map->minimumZoom() becomes operational
    // after this has been called at least once, after creation.
    // However, getting into the following block may fire a copyrightsChanged that would get lost,
    // as the connections are set up after.
    QString copyrightString;
    QImage copyrightImage;
    if (!m_initialized && width() > 0 && height() > 0) {
        QMetaObject::Connection copyrightStringCatcherConnection =
                connect(m_map.data(),
                        QOverload<const QString &>::of(&QGeoMap::copyrightsChanged),
                        [&copyrightString](const QString &copy){ copyrightString = copy; });
        QMetaObject::Connection copyrightImageCatcherConnection =
                connect(m_map.data(),
                        QOverload<const QImage &>::of(&QGeoMap::copyrightsChanged),
                        [&copyrightImage](const QImage &copy){ copyrightImage = copy; });
        m_map->setViewportSize(QSize(width(), height()));
        initialize(); // This emits the caught signals above
        QObject::disconnect(copyrightStringCatcherConnection);
        QObject::disconnect(copyrightImageCatcherConnection);
    }


    /* COPYRIGHT SIGNALS REWIRING */
    connect(m_map.data(), SIGNAL(copyrightsChanged(QImage)),
            this,  SIGNAL(copyrightsChanged(QImage)));
    connect(m_map.data(), SIGNAL(copyrightsChanged(QString)),
            this,  SIGNAL(copyrightsChanged(QString)));
    if (!copyrightString.isEmpty())
        emit m_map->copyrightsChanged(copyrightString);
    else if (!copyrightImage.isNull())
        emit m_map->copyrightsChanged(copyrightImage);


    connect(m_map.data(), &QGeoMap::sgNodeChanged, this, &QQuickItem::update);
    connect(m_map.data(), &QGeoMap::cameraCapabilitiesChanged, this, &QDeclarativeGeoMap::onCameraCapabilitiesChanged);

    // This prefetches a buffer around the map
    m_map->prefetchData();

    connect(m_mappingManager, SIGNAL(supportedMapTypesChanged()), this, SLOT(onSupportedMapTypesChanged()));
    emit minimumZoomLevelChanged();
    emit maximumZoomLevelChanged();
    emit supportedMapTypesChanged();
    emit activeMapTypeChanged();

    // Any map item groups that were added before the plugin was ready
    // DO NOT need to have setMap called again on their children map items
    // because they have been added to m_mapItems, which is processed right above.


    // All map parameters that were added before the plugin was ready
    // need to be added to m_map
    for (QDeclarativeGeoMapParameter *p : qAsConst(m_mapParameters))
        m_map->addParameter(p);

    if (m_initialized)
        update();
}

/*!
    \internal
*/
QDeclarativeGeoServiceProvider *QDeclarativeGeoMap::plugin() const
{
    return m_plugin;
}

/*!
    \internal
    Sets the gesture areas minimum zoom level. If the camera capabilities
    has been set this method honors the boundaries set by it.
    The minimum zoom level will also have a lower bound dependent on the size
    of the canvas, effectively preventing to display out of bounds areas.
*/
void QDeclarativeGeoMap::setMinimumZoomLevel(qreal minimumZoomLevel, bool userSet)
{
    if (minimumZoomLevel >= 0) {
        qreal oldUserMinimumZoomLevel = m_userMinimumZoomLevel;
        if (userSet)
            m_userMinimumZoomLevel = minimumZoomLevel;
        qreal oldMinimumZoomLevel = this->minimumZoomLevel();

        minimumZoomLevel = qBound(qreal(m_cameraCapabilities.minimumZoomLevelAt256()), minimumZoomLevel, maximumZoomLevel());
        if (m_map)
             minimumZoomLevel = qMax<qreal>(minimumZoomLevel, m_map->minimumZoom());

        // minimumZoomLevel is, at this point, the implicit minimum zoom level
        m_gestureArea->setMinimumZoomLevel(minimumZoomLevel);

        if (zoomLevel() < minimumZoomLevel && (m_gestureArea->enabled() || !m_cameraCapabilities.overzoomEnabled()))
            setZoomLevel(minimumZoomLevel);

        if (qIsNaN(m_userMinimumZoomLevel) && oldMinimumZoomLevel != minimumZoomLevel)
            emit minimumZoomLevelChanged();
        else if (userSet && oldUserMinimumZoomLevel != m_userMinimumZoomLevel)
            emit minimumZoomLevelChanged();
    }
}

/*!
    \qmlproperty real QtLocation::Map::minimumZoomLevel

    This property holds the minimum valid zoom level for the map.

    The minimum zoom level defined by the \l plugin used is a lower bound for
    this property. However, the returned value is also canvas-size-dependent, and
    can be higher than the user-specified value, or than the minimum zoom level
    defined by the plugin used, to prevent the map from being smaller than the
    viewport in either dimension.

    If the \l plugin property is not set or the plugin does not support mapping, this property is \c 0.
*/

qreal QDeclarativeGeoMap::minimumZoomLevel() const
{
    if (!qIsNaN(m_userMinimumZoomLevel))
        return m_userMinimumZoomLevel;
    else
        return m_gestureArea->minimumZoomLevel();
}

/*!
    \internal
*/
qreal QDeclarativeGeoMap::implicitMinimumZoomLevel() const
{
    return m_gestureArea->minimumZoomLevel();
}

/*!
    \internal
*/
qreal QDeclarativeGeoMap::effectiveMinimumZoomLevel() const
{
    return qMax<qreal>(minimumZoomLevel(), implicitMinimumZoomLevel());
}

/*!
    \internal
    Sets the gesture areas maximum zoom level. If the camera capabilities
    has been set this method honors the boundaries set by it.
*/
void QDeclarativeGeoMap::setMaximumZoomLevel(qreal maximumZoomLevel, bool userSet)
{
    if (maximumZoomLevel >= 0) {
        if (userSet)
            m_userMaximumZoomLevel = maximumZoomLevel;
        qreal oldMaximumZoomLevel = this->maximumZoomLevel();

        maximumZoomLevel = qBound(minimumZoomLevel(), maximumZoomLevel, qreal(m_cameraCapabilities.maximumZoomLevelAt256()));

        m_gestureArea->setMaximumZoomLevel(maximumZoomLevel);

        if (zoomLevel() > maximumZoomLevel && (m_gestureArea->enabled() || !m_cameraCapabilities.overzoomEnabled()))
            setZoomLevel(maximumZoomLevel);

        if (oldMaximumZoomLevel != maximumZoomLevel)
            emit maximumZoomLevelChanged();
    }
}

/*!
    \qmlproperty real QtLocation::Map::maximumZoomLevel

    This property holds the maximum valid zoom level for the map.

    The maximum zoom level is defined by the \l plugin used.
    If the \l plugin property is not set or the plugin does not support mapping, this property is \c 30.
*/

qreal QDeclarativeGeoMap::maximumZoomLevel() const
{
    return m_gestureArea->maximumZoomLevel();
}

/*!
    \qmlproperty real QtLocation::Map::zoomLevel

    This property holds the zoom level for the map.

    Larger values for the zoom level provide more detail. Zoom levels
    are always non-negative. The default value is 8.0. Depending on the plugin in use,
    values outside the [minimumZoomLevel, maximumZoomLevel] range, which represent the range for which
    tiles are available, may be accepted, or clamped.
*/
void QDeclarativeGeoMap::setZoomLevel(qreal zoomLevel)
{
    return setZoomLevel(zoomLevel, m_cameraCapabilities.overzoomEnabled());
}

/*!
    \internal

    Sets the zoom level.
    Larger values for the zoom level provide more detail. Zoom levels
    are always non-negative. The default value is 8.0. Values outside the
    [minimumZoomLevel, maximumZoomLevel] range, which represent the range for which
    tiles are available, can be accepted or clamped by setting the overzoom argument
    to true or false respectively.
*/
void QDeclarativeGeoMap::setZoomLevel(qreal zoomLevel, bool overzoom)
{
    if (zoomLevel < 0)
        return;

    if (m_initialized) {
        QGeoCameraData cameraData = m_map->cameraData();
        if (cameraData.zoomLevel() == zoomLevel)
            return;

        cameraData.setZoomLevel(qBound<qreal>(overzoom ? m_map->minimumZoom() : effectiveMinimumZoomLevel(),
                                                zoomLevel,
                                                overzoom ? 30 : maximumZoomLevel()));
        m_maximumViewportLatitude = m_map->maximumCenterLatitudeAtZoom(cameraData);
        m_minimumViewportLatitude = m_map->minimumCenterLatitudeAtZoom(cameraData);
        QGeoCoordinate coord = cameraData.center();
        coord.setLatitude(qBound(m_minimumViewportLatitude, coord.latitude(), m_maximumViewportLatitude));
        cameraData.setCenter(coord);
        m_map->setCameraData(cameraData);
    } else {
        const bool zlHasChanged = zoomLevel != m_cameraData.zoomLevel();
        m_cameraData.setZoomLevel(zoomLevel);
        if (zlHasChanged)
            emit zoomLevelChanged(zoomLevel);
    }
}

bool QDeclarativeGeoMap::addMapChild(QObject *child)
{
    // dispatch items appropriately
    QDeclarativeGeoMapItemView *mapView = qobject_cast<QDeclarativeGeoMapItemView *>(child);
    if (mapView)
        return addMapItemView_real(mapView);

    QDeclarativeGeoMapItemGroup *itemGroup = qobject_cast<QDeclarativeGeoMapItemGroup *>(child);
    if (itemGroup) // addMapItemView calls addMapItemGroup
        return addMapItemGroup_real(itemGroup);

    QDeclarativeGeoMapItemBase *mapItem = qobject_cast<QDeclarativeGeoMapItemBase *>(child);
    if (mapItem)
        return addMapItem_real(mapItem);

    QGeoMapObject *mapObject = qobject_cast<QGeoMapObject *>(child);
    if (mapObject)
        addMapObject(mapObject); // this emits mapObjectsChanged, != mapItemsChanged
    return false;
}

bool QDeclarativeGeoMap::removeMapChild(QObject *child)
{
    // dispatch items appropriately
    QDeclarativeGeoMapItemView *mapView = qobject_cast<QDeclarativeGeoMapItemView *>(child);
    if (mapView)
        return removeMapItemView_real(mapView);

    QDeclarativeGeoMapItemGroup *itemGroup = qobject_cast<QDeclarativeGeoMapItemGroup *>(child);
    if (itemGroup) // removeMapItemView calls removeMapItemGroup for itself.
        return removeMapItemGroup_real(itemGroup);

    QDeclarativeGeoMapItemBase *mapItem = qobject_cast<QDeclarativeGeoMapItemBase *>(child);
    if (mapItem)
        return removeMapItem_real(mapItem);

    QGeoMapObject *mapObject = qobject_cast<QGeoMapObject *>(child);
    if (mapObject)
        removeMapObject(mapObject); // this emits mapObjectsChanged, != mapItemsChanged
    return false;
}

bool QDeclarativeGeoMap::isGroupNested(QDeclarativeGeoMapItemGroup *group)
{
    QObject *parent = group->parent();
    // Nested groups have parent set in parent's componentComplete()
    // Those instantiated by MapItemView's delegateModel, however, do not,
    // but have setParentItem set.
    return qobject_cast<QDeclarativeGeoMapItemGroup *>(parent)
            || qobject_cast<QDeclarativeGeoMapItemGroup *>(group->parentItem());
}

qreal QDeclarativeGeoMap::zoomLevel() const
{
    if (m_initialized)
        return m_map->cameraData().zoomLevel();
    return m_cameraData.zoomLevel();
}

/*!
    \qmlproperty real QtLocation::Map::bearing

    This property holds the bearing for the map.
    The default value is 0.
    If the Plugin used for the Map supports bearing, the valid range for this value is between 0 and 360.
    If the Plugin used for the Map does not support bearing, changing this property will have no effect.

    \since QtLocation 5.9
*/
void QDeclarativeGeoMap::setBearing(qreal bearing)
{
    bearing = sanitizeBearing(bearing);
    if (m_initialized) {
        QGeoCameraData cameraData = m_map->cameraData();
        cameraData.setBearing(bearing);
        m_map->setCameraData(cameraData);
    } else {
        const bool bearingHasChanged = bearing != m_cameraData.bearing();
        m_cameraData.setBearing(bearing);
        if (bearingHasChanged)
            emit bearingChanged(bearing);
    }
}

/*!
    \qmlmethod void QtLocation::Map::setBearing(real bearing, coordinate coordinate)

    Sets the bearing for the map to \a bearing, rotating it around \a coordinate.
    If the Plugin used for the Map supports bearing, the valid range for \a bearing is between 0 and 360.
    If the Plugin used for the Map does not support bearing, or if the map is tilted and \a coordinate happens
    to be behind the camera, or if the map is not ready (see \l mapReady), calling this method will have no effect.

    The release of this API with Qt 5.10 is a Technology Preview.

    \since 5.10
*/
void QDeclarativeGeoMap::setBearing(qreal bearing, const QGeoCoordinate &coordinate)
{
    if (!m_initialized)
        return;

    const QGeoCoordinate currentCenter = center();
    const qreal currentBearing = QDeclarativeGeoMap::bearing();
    bearing = sanitizeBearing(bearing);

    if (!coordinate.isValid()
            || !qIsFinite(bearing)
            || (coordinate == currentCenter && bearing == currentBearing))
        return;

    if (m_map->capabilities() & QGeoMap::SupportsSetBearing)
        m_map->setBearing(bearing, coordinate);
}

qreal QDeclarativeGeoMap::bearing() const
{
    if (m_initialized)
        return m_map->cameraData().bearing();
    return m_cameraData.bearing();
}

/*!
    \qmlproperty real QtLocation::Map::tilt

    This property holds the tilt for the map, in degrees.
    The default value is 0.
    The valid range for this value is [ minimumTilt, maximumTilt ].
    If the Plugin used for the Map does not support tilting, changing this property will have no effect.

    \sa minimumTilt, maximumTilt

    \since QtLocation 5.9
*/
void QDeclarativeGeoMap::setTilt(qreal tilt)
{
    tilt = qBound(minimumTilt(), tilt, maximumTilt());

    if (m_initialized) {
        QGeoCameraData cameraData = m_map->cameraData();
        cameraData.setTilt(tilt);
        m_map->setCameraData(cameraData);
    } else {
        const bool tiltHasChanged = tilt != m_cameraData.tilt();
        m_cameraData.setTilt(tilt);
        if (tiltHasChanged)
            emit tiltChanged(tilt);
    }
}

qreal QDeclarativeGeoMap::tilt() const
{
    if (m_initialized)
        return m_map->cameraData().tilt();
    return m_cameraData.tilt();
}

void QDeclarativeGeoMap::setMinimumTilt(qreal minimumTilt, bool userSet)
{
    if (minimumTilt >= 0) {
        if (userSet)
            m_userMinimumTilt = minimumTilt;
        qreal oldMinimumTilt = this->minimumTilt();

        m_minimumTilt = qBound<double>(m_cameraCapabilities.minimumTilt(),
                                       minimumTilt,
                                       m_cameraCapabilities.maximumTilt());

        if (tilt() < m_minimumTilt)
            setTilt(m_minimumTilt);

        if (oldMinimumTilt != m_minimumTilt)
            emit minimumTiltChanged(m_minimumTilt);
    }
}

/*!
    \qmlproperty real QtLocation::Map::fieldOfView

    This property holds the field of view of the camera used to look at the map, in degrees.
    If the plugin property of the map is not set, or the plugin does not support mapping, the value is 45 degrees.

    Note that changing this value implicitly changes also the distance between the camera and the map,
    so that, at a tilting angle of 0 degrees, the resulting image is identical for any value used for this property.

    For more information about this parameter, consult the Wikipedia articles about \l {https://en.wikipedia.org/wiki/Field_of_view} {Field of view} and
    \l {https://en.wikipedia.org/wiki/Angle_of_view} {Angle of view}.

    \sa minimumFieldOfView, maximumFieldOfView

    \since QtLocation 5.9
*/
void QDeclarativeGeoMap::setFieldOfView(qreal fieldOfView)
{
    fieldOfView = qBound(minimumFieldOfView(), fieldOfView, maximumFieldOfView());

    if (m_initialized) {
        QGeoCameraData cameraData = m_map->cameraData();
        cameraData.setFieldOfView(fieldOfView);
        m_map->setCameraData(cameraData);
    } else {
        const bool fovChanged = fieldOfView != m_cameraData.fieldOfView();
        m_cameraData.setFieldOfView(fieldOfView);
        if (fovChanged)
            emit fieldOfViewChanged(fieldOfView);
    }
}

qreal QDeclarativeGeoMap::fieldOfView() const
{
    if (m_initialized)
        return m_map->cameraData().fieldOfView();
    return m_cameraData.fieldOfView();
}

void QDeclarativeGeoMap::setMinimumFieldOfView(qreal minimumFieldOfView, bool userSet)
{
    if (minimumFieldOfView > 0 && minimumFieldOfView < 180.0) {
        if (userSet)
            m_userMinimumFieldOfView = minimumFieldOfView;
        qreal oldMinimumFoV = this->minimumFieldOfView();

        m_minimumFieldOfView = qBound<double>(m_cameraCapabilities.minimumFieldOfView(),
                                              minimumFieldOfView,
                                              m_cameraCapabilities.maximumFieldOfView());

        if (fieldOfView() < m_minimumFieldOfView)
            setFieldOfView(m_minimumFieldOfView);

        if (oldMinimumFoV != m_minimumFieldOfView)
            emit minimumFieldOfViewChanged(m_minimumFieldOfView);
    }
}

/*!
    \qmlproperty real QtLocation::Map::minimumFieldOfView

    This property holds the minimum valid field of view for the map, in degrees.

    The minimum tilt field of view by the \l plugin used is a lower bound for
    this property.
    If the \l plugin property is not set or the plugin does not support mapping, this property is \c 1.

    \sa fieldOfView, maximumFieldOfView

    \since QtLocation 5.9
*/
qreal QDeclarativeGeoMap::minimumFieldOfView() const
{
    return m_minimumFieldOfView;
}

void QDeclarativeGeoMap::setMaximumFieldOfView(qreal maximumFieldOfView, bool userSet)
{
    if (maximumFieldOfView > 0 && maximumFieldOfView < 180.0) {
        if (userSet)
            m_userMaximumFieldOfView = maximumFieldOfView;
        qreal oldMaximumFoV = this->maximumFieldOfView();

        m_maximumFieldOfView = qBound<double>(m_cameraCapabilities.minimumFieldOfView(),
                                              maximumFieldOfView,
                                              m_cameraCapabilities.maximumFieldOfView());

        if (fieldOfView() > m_maximumFieldOfView)
            setFieldOfView(m_maximumFieldOfView);

        if (oldMaximumFoV != m_maximumFieldOfView)
            emit maximumFieldOfViewChanged(m_maximumFieldOfView);
    }
}

/*!
    \qmlproperty real QtLocation::Map::maximumFieldOfView

    This property holds the maximum valid field of view for the map, in degrees.

    The minimum tilt field of view by the \l plugin used is an upper bound for
    this property.
    If the \l plugin property is not set or the plugin does not support mapping, this property is \c 179.

    \sa fieldOfView, minimumFieldOfView

    \since QtLocation 5.9
*/
qreal QDeclarativeGeoMap::maximumFieldOfView() const
{
    return m_maximumFieldOfView;
}

/*!
    \qmlproperty real QtLocation::Map::minimumTilt

    This property holds the minimum valid tilt for the map, in degrees.

    The minimum tilt defined by the \l plugin used is a lower bound for
    this property.
    If the \l plugin property is not set or the plugin does not support mapping, this property is \c 0.

    Since QtLocation 5.12, plugins can additionally restrict this value depending on the current zoom level.

    \sa tilt, maximumTilt

    \since QtLocation 5.9
*/
qreal QDeclarativeGeoMap::minimumTilt() const
{
    return m_minimumTilt;
}

void QDeclarativeGeoMap::setMaximumTilt(qreal maximumTilt, bool userSet)
{
    if (maximumTilt >= 0) {
        if (userSet)
            m_userMaximumTilt = maximumTilt;
        qreal oldMaximumTilt = this->maximumTilt();

        m_maximumTilt = qBound<double>(m_cameraCapabilities.minimumTilt(),
                                       maximumTilt,
                                       m_cameraCapabilities.maximumTilt());

        if (tilt() > m_maximumTilt)
            setTilt(m_maximumTilt);

        if (oldMaximumTilt != m_maximumTilt)
            emit maximumTiltChanged(m_maximumTilt);
    }
}

/*!
    \qmlproperty real QtLocation::Map::maximumTilt

    This property holds the maximum valid tilt for the map, in degrees.

    The maximum tilt defined by the \l plugin used is an upper bound for
    this property.
    If the \l plugin property is not set or the plugin does not support mapping, this property is \c 89.5.

    Since QtLocation 5.12, plugins can additionally restrict this value depending on the current zoom level.

    \sa tilt, minimumTilt

    \since QtLocation 5.9
*/
qreal QDeclarativeGeoMap::maximumTilt() const
{
    return m_maximumTilt;
}

/*!
    \qmlproperty coordinate QtLocation::Map::center

    This property holds the coordinate which occupies the center of the
    mapping viewport. Invalid center coordinates are ignored.

    The default value is an arbitrary valid coordinate.
*/
void QDeclarativeGeoMap::setCenter(const QGeoCoordinate &center)
{
    if (!center.isValid())
        return;

    if (m_initialized) {
        QGeoCoordinate coord(center);
        coord.setLatitude(qBound(m_minimumViewportLatitude, center.latitude(), m_maximumViewportLatitude));
        QGeoCameraData cameraData = m_map->cameraData();
        cameraData.setCenter(coord);
        m_map->setCameraData(cameraData);
    } else {
        const bool centerHasChanged = center != m_cameraData.center();
        m_cameraData.setCenter(center);
        if (centerHasChanged)
            emit centerChanged(center);
    }
}

QGeoCoordinate QDeclarativeGeoMap::center() const
{
    if (m_initialized)
        return m_map->cameraData().center();
    return m_cameraData.center();
}


/*!
    \qmlproperty geoshape QtLocation::Map::visibleRegion

    This property holds the region which occupies the viewport of
    the map. The camera is positioned in the center of the shape, and
    at the largest integral zoom level possible which allows the
    whole shape to be visible on the screen. This implies that
    reading this property back shortly after having been set the
    returned area is equal or larger than the set area.

    Setting this property implicitly changes the \l center and
    \l zoomLevel of the map. Any previously set value to those
    properties will be overridden.

    This property does not provide any change notifications.

    \since 5.6
*/
void QDeclarativeGeoMap::setVisibleRegion(const QGeoShape &shape)
{
    if (shape.boundingGeoRectangle() == visibleRegion())
        return;

    m_visibleRegion = shape.boundingGeoRectangle();
    if (!m_visibleRegion.isValid()
        || (m_visibleRegion.bottomRight().latitude() >= 85.0) // rect entirely outside web mercator
        || (m_visibleRegion.topLeft().latitude() <= -85.0)) {
        // shape invalidated -> nothing to fit anymore
        m_visibleRegion = QGeoRectangle();
        m_pendingFitViewport = false;
        return;
    }

    if (!m_map || !width() || !height()) {
        m_pendingFitViewport = true;
        return;
    }

    fitViewportToGeoShape();
}

QGeoShape QDeclarativeGeoMap::visibleRegion() const
{
    if (!m_map || !width() || !height())
        return m_visibleRegion;

    if (m_map->capabilities() & QGeoMap::SupportsVisibleRegion) {
        return m_map->visibleRegion();
    } else {
        // ToDo: handle projections not supporting visible region in a better way.
        // This approach will fail when horizon is in the view or the map is greatly zoomed out.
        QList<QGeoCoordinate> visiblePoly;
        visiblePoly << m_map->geoProjection().itemPositionToCoordinate(QDoubleVector2D(0,0), false);
        visiblePoly << m_map->geoProjection().itemPositionToCoordinate(QDoubleVector2D(m_map->viewportWidth() - 1,
                                                                                       0), false);
        visiblePoly << m_map->geoProjection().itemPositionToCoordinate(QDoubleVector2D(m_map->viewportWidth() - 1,
                                                                                       m_map->viewportHeight() - 1), false);
        visiblePoly << m_map->geoProjection().itemPositionToCoordinate(QDoubleVector2D(0,
                                                                                       m_map->viewportHeight() - 1), false);
        QGeoPath path;
        path.setPath(visiblePoly);
        return path.boundingGeoRectangle();
    }
}

/*!
    \qmlproperty bool QtLocation::Map::copyrightsVisible

    This property holds the visibility of the copyrights notice. The notice is usually
    displayed in the bottom left corner. By default, this property is set to \c true.

    \note Many map providers require the notice to be visible as part of the terms and conditions.
    Please consult the relevant provider documentation before turning this notice off.

    \since 5.7
*/
void QDeclarativeGeoMap::setCopyrightsVisible(bool visible)
{
    if (m_copyrightsVisible == visible)
        return;

    if (!m_copyrights.isNull())
        m_copyrights->setCopyrightsVisible(visible);

    m_copyrightsVisible = visible;
    emit copyrightsVisibleChanged(visible);
}

bool QDeclarativeGeoMap::copyrightsVisible() const
{
    return m_copyrightsVisible;
}



/*!
    \qmlproperty color QtLocation::Map::color

    This property holds the background color of the map element.

    \since 5.6
*/
void QDeclarativeGeoMap::setColor(const QColor &color)
{
    if (color != m_color) {
        m_color = color;
        update();
        emit colorChanged(m_color);
    }
}

QColor QDeclarativeGeoMap::color() const
{
    return m_color;
}

/*!
    \qmlproperty rect QtLocation::Map::visibleArea

    This property holds the visible area inside the Map QML element.
    It is a rect whose coordinates are relative to the Map element.
    Its size will be clamped to the size of the Map element.
    A null visibleArea means that the whole Map is visible.

    \since 5.12
*/
QRectF QDeclarativeGeoMap::visibleArea() const
{
    if (m_initialized)
        return m_map->visibleArea();
    return m_visibleArea;
}

void QDeclarativeGeoMap::setVisibleArea(const QRectF &visibleArea)
{
    const QRectF oldVisibleArea = QDeclarativeGeoMap::visibleArea();
    if (visibleArea == oldVisibleArea)
        return;

    if (!visibleArea.isValid() && !visibleArea.isEmpty()) // values < 0
        return;

    if (m_initialized) {
        m_map->setVisibleArea(visibleArea);
        const QRectF newVisibleArea = QDeclarativeGeoMap::visibleArea();
        if (newVisibleArea != oldVisibleArea) {
            // polish map items
            for (const QPointer<QDeclarativeGeoMapItemBase> &i: qAsConst(m_mapItems)) {
                if (i)
                    i->visibleAreaChanged();
            }
        }
    } else {
        m_visibleArea = visibleArea;
        const QRectF newVisibleArea = QDeclarativeGeoMap::visibleArea();
        if (newVisibleArea != oldVisibleArea)
            emit visibleAreaChanged();
    }
}

/*!
    \qmlproperty bool QtLocation::Map::mapReady

    This property holds whether the map has been successfully initialized and is ready to be used.
    Some methods, such as \l fromCoordinate and \l toCoordinate, will not work before the map is ready.
    Due to the architecture of the \l Map, it's advised to use the signal emitted for this property
    in place of \l {QtQml::Component::completed()}{Component.onCompleted}, to make sure that everything behaves as expected.

    \since 5.9
*/
bool QDeclarativeGeoMap::mapReady() const
{
    return m_initialized;
}

QMargins QDeclarativeGeoMap::mapMargins() const
{
    const QRectF va = m_map->visibleArea();
    if (va.isEmpty())
        return QMargins();
    return QMargins( va.x()
                    , va.y()
                    , width() - va.width() - va.x()
                    , height() - va.height() - va.y());
}

// TODO: offer the possibility to specify the margins.
void QDeclarativeGeoMap::fitViewportToGeoShape()
{
    if (m_map->geoProjection().projectionType() == QGeoProjection::ProjectionWebMercator) {
        // This case remains handled here, and not inside QGeoMap*::fitViewportToGeoRectangle,
        // in order to honor animations on center and zoomLevel
        const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(m_map->geoProjection());
        const int borderSize = 10;
        const QMargins borders(borderSize, borderSize, borderSize, borderSize);

        if (!m_map  || !m_visibleRegion.isValid())
            return;

        const QMargins margins = borders + mapMargins();
        const QPair<QGeoCoordinate, qreal> fitData = p.fitViewportToGeoRectangle(m_visibleRegion,
                                                                                 margins);
        if (!fitData.first.isValid())
            return;

        // position camera to the center of bounding box
        setProperty("center", QVariant::fromValue(fitData.first)); // not using setCenter(centerCoordinate) to honor a possible animation set on the center property

        if (!qIsFinite(fitData.second))
            return;
        double newZoom = qMax<double>(minimumZoomLevel(), fitData.second);
        setProperty("zoomLevel", QVariant::fromValue(newZoom)); // not using setZoomLevel(newZoom)  to honor a possible animation set on the zoomLevel property
    } else if (m_map->capabilities() & QGeoMap::SupportsFittingViewportToGeoRectangle) {
        // Animations cannot be honored in this case, as m_map acts as a black box
        m_map->fitViewportToGeoRectangle(m_visibleRegion);
    }
}


/*!
    \qmlproperty list<MapType> QtLocation::Map::supportedMapTypes

    This read-only property holds the set of \l{MapType}{map types} supported by this map.

    \sa activeMapType
*/
QQmlListProperty<QDeclarativeGeoMapType> QDeclarativeGeoMap::supportedMapTypes()
{
    return QQmlListProperty<QDeclarativeGeoMapType>(this, m_supportedMapTypes);
}

/*!
    \qmlmethod void QtLocation::Map::alignCoordinateToPoint(coordinate coordinate, QPointF point)

    Aligns \a coordinate to \a point.
    This method effectively extends the functionality offered by the \l center qml property, allowing
    to align a coordinate to point of the Map element other than its center.
    This is useful in those applications where the center of the scene (e.g., a cursor) is not to be
    placed exactly in the center of the map.

    If the map is tilted, and \a coordinate happens to be behind the camera, or if the map is not ready
    (see \l mapReady), calling this method will have no effect.

    The release of this API with Qt 5.10 is a Technology Preview.

    \sa center

    \since 5.10
*/
void QDeclarativeGeoMap::alignCoordinateToPoint(const QGeoCoordinate &coordinate, const QPointF &point)
{
    if (!m_map || !(m_map->capabilities() & QGeoMap::SupportsAnchoringCoordinate))
        return;

    if (!coordinate.isValid()
            || !qIsFinite(point.x())
            || !qIsFinite(point.y()))
        return;

    m_map->anchorCoordinateToPoint(coordinate, point);
}

/*!
    \qmlmethod coordinate QtLocation::Map::toCoordinate(QPointF position, bool clipToViewPort)

    Returns the coordinate which corresponds to the \a position relative to the map item.

    If \a cliptoViewPort is \c true, or not supplied then returns an invalid coordinate if
    \a position is not within the current viewport.
*/
QGeoCoordinate QDeclarativeGeoMap::toCoordinate(const QPointF &position, bool clipToViewPort) const
{
    if (m_map)
        return m_map->geoProjection().itemPositionToCoordinate(QDoubleVector2D(position), clipToViewPort);
    else
        return QGeoCoordinate();
}

/*!
    \qmlmethod point QtLocation::Map::fromCoordinate(coordinate coordinate, bool clipToViewPort)

    Returns the position relative to the map item which corresponds to the \a coordinate.

    If \a cliptoViewPort is \c true, or not supplied then returns an invalid QPointF if
    \a coordinate is not within the current viewport.
*/
QPointF QDeclarativeGeoMap::fromCoordinate(const QGeoCoordinate &coordinate, bool clipToViewPort) const
{
    if (m_map)
        return m_map->geoProjection().coordinateToItemPosition(coordinate, clipToViewPort).toPointF();
    else
        return QPointF(qQNaN(), qQNaN());
}

/*!
    \qmlmethod void QtLocation::Map::pan(int dx, int dy)

    Starts panning the map by \a dx pixels along the x-axis and
    by \a dy pixels along the y-axis.

    Positive values for \a dx move the map right, negative values left.
    Positive values for \a dy move the map down, negative values up.

    During panning the \l center, and \l zoomLevel may change.
*/
void QDeclarativeGeoMap::pan(int dx, int dy)
{
    if (!m_map)
        return;
    if (dx == 0 && dy == 0)
        return;

    QGeoCoordinate coord = m_map->geoProjection().itemPositionToCoordinate(
                                QDoubleVector2D(m_map->viewportWidth() / 2 + dx,
                                        m_map->viewportHeight() / 2 + dy));
    setCenter(coord);
}


/*!
    \qmlmethod void QtLocation::Map::prefetchData()

    Optional hint that allows the map to prefetch during this idle period
*/
void QDeclarativeGeoMap::prefetchData()
{
    if (!m_map)
        return;
    m_map->prefetchData();
}

/*!
    \qmlmethod void QtLocation::Map::clearData()

    Clears map data collected by the currently selected plugin.
    \note This method will delete cached files.
    \sa plugin
*/
void QDeclarativeGeoMap::clearData()
{
    if (m_map)
        m_map->clearData();
}

/*!
    \qmlproperty string QtLocation::Map::errorString

    This read-only property holds the textual presentation of the latest mapping provider error.
    If no error has occurred, an empty string is returned.

    An empty string may also be returned if an error occurred which has no associated
    textual representation.

    \sa QGeoServiceProvider::errorString()
*/

QString QDeclarativeGeoMap::errorString() const
{
    return m_errorString;
}

/*!
    \qmlproperty enumeration QtLocation::Map::error

    This read-only property holds the last occurred mapping service provider error.

    \list
    \li Map.NoError - No error has occurred.
    \li Map.NotSupportedError -The maps plugin property was not set or there is no mapping manager associated with the plugin.
    \li Map.UnknownParameterError -The plugin did not recognize one of the parameters it was given.
    \li Map.MissingRequiredParameterError - The plugin did not find one of the parameters it was expecting.
    \li Map.ConnectionError - The plugin could not connect to its backend service or database.
    \endlist

    \sa QGeoServiceProvider::Error
*/

QGeoServiceProvider::Error QDeclarativeGeoMap::error() const
{
    return m_error;
}

QGeoMap *QDeclarativeGeoMap::map() const
{
    return m_map;
}

void QDeclarativeGeoMap::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == ItemChildAddedChange) {
        QQuickItem *child = value.item;
        QQuickItem *mapItem = qobject_cast<QDeclarativeGeoMapItemBase *>(child);
        if (!mapItem)
            mapItem = qobject_cast<QDeclarativeGeoMapItemGroup *>(child);

        if (mapItem) {
            qreal z = mapItem->z();
            if (z > m_maxChildZ) { // Ignore children removal
                m_maxChildZ = z;
                // put the copyrights notice object at the highest z order
                if (m_copyrights)
                    m_copyrights->setCopyrightsZ(m_maxChildZ + 1);
            }
        }
    }
    QQuickItem::itemChange(change, value);
}

bool QDeclarativeGeoMap::isInteractive()
{
    return (m_gestureArea->enabled() && m_gestureArea->acceptedGestures()) || m_gestureArea->isActive();
}

void QDeclarativeGeoMap::attachCopyrightNotice(bool initialVisibility)
{
    if (initialVisibility) {
        ++m_copyNoticesVisible;
        if (m_map)
            m_map->setCopyrightVisible(m_copyNoticesVisible > 0);
    }
}

void QDeclarativeGeoMap::detachCopyrightNotice(bool currentVisibility)
{
    if (currentVisibility) {
        --m_copyNoticesVisible;
        if (m_map)
            m_map->setCopyrightVisible(m_copyNoticesVisible > 0);
    }
}

void QDeclarativeGeoMap::onAttachedCopyrightNoticeVisibilityChanged()
{
    QDeclarativeGeoMapCopyrightNotice *copy = static_cast<QDeclarativeGeoMapCopyrightNotice *>(sender());
    m_copyNoticesVisible += ( int(copy->copyrightsVisible()) * 2 - 1);
    if (m_map)
        m_map->setCopyrightVisible(m_copyNoticesVisible > 0);
}

void QDeclarativeGeoMap::onCameraDataChanged(const QGeoCameraData &cameraData)
{
    bool centerHasChanged = cameraData.center() != m_cameraData.center();
    bool bearingHasChanged = cameraData.bearing() != m_cameraData.bearing();
    bool tiltHasChanged = cameraData.tilt() != m_cameraData.tilt();
    bool fovHasChanged = cameraData.fieldOfView() != m_cameraData.fieldOfView();
    bool zoomHasChanged = cameraData.zoomLevel() != m_cameraData.zoomLevel();

    m_cameraData = cameraData;
    // polish map items
    for (const QPointer<QDeclarativeGeoMapItemBase> &i: qAsConst(m_mapItems)) {
        if (i)
            i->baseCameraDataChanged(m_cameraData); // Consider optimizing this further, removing the contained duplicate if conditions.
    }

    if (centerHasChanged)
        emit centerChanged(m_cameraData.center());
    if (zoomHasChanged)
        emit zoomLevelChanged(m_cameraData.zoomLevel());
    if (bearingHasChanged)
        emit bearingChanged(m_cameraData.bearing());
    if (tiltHasChanged)
        emit tiltChanged(m_cameraData.tilt());
    if (fovHasChanged)
        emit fieldOfViewChanged(m_cameraData.fieldOfView());
}

/*!
    \qmlmethod void QtLocation::Map::addMapParameter(MapParameter parameter)

    Adds a MapParameter object to the map. The effect of this call is dependent
    on the combination of the content of the MapParameter and the type of
    underlying QGeoMap. If a MapParameter that is not supported by the underlying
    QGeoMap gets added, the call has no effect.

    The release of this API with Qt 5.9 is a Technology Preview.

    \sa MapParameter, removeMapParameter, mapParameters, clearMapParameters

    \since 5.9
*/
void QDeclarativeGeoMap::addMapParameter(QDeclarativeGeoMapParameter *parameter)
{
    if (!parameter->isComponentComplete()) {
        connect(parameter, &QDeclarativeGeoMapParameter::completed, this, &QDeclarativeGeoMap::addMapParameter);
        return;
    }

    disconnect(parameter);
    if (m_mapParameters.contains(parameter))
        return;
    parameter->setParent(this);
    m_mapParameters.append(parameter); // parameter now owned by QDeclarativeGeoMap
    if (m_map)
        m_map->addParameter(parameter);
}

/*!
    \qmlmethod void QtLocation::Map::removeMapParameter(MapParameter parameter)

    Removes the given MapParameter object from the map.

    The release of this API with Qt 5.9 is a Technology Preview.

    \sa MapParameter, addMapParameter, mapParameters, clearMapParameters

    \since 5.9
*/
void QDeclarativeGeoMap::removeMapParameter(QDeclarativeGeoMapParameter *parameter)
{
    if (!m_mapParameters.contains(parameter))
        return;
    if (m_map)
        m_map->removeParameter(parameter);
    m_mapParameters.removeOne(parameter);
}

/*!
    \qmlmethod void QtLocation::Map::clearMapParameters()

    Removes all map parameters from the map.

    The release of this API with Qt 5.9 is a Technology Preview.

    \sa MapParameter, mapParameters, addMapParameter, removeMapParameter, clearMapParameters

    \since 5.9
*/
void QDeclarativeGeoMap::clearMapParameters()
{
    if (m_map)
        m_map->clearParameters();
    m_mapParameters.clear();
}

/*!
    \qmlproperty list<MapParameters> QtLocation::Map::mapParameters

    Returns the list of all map parameters in no particular order.
    These items include map parameters that were declared statically as part of
    the type declaration, as well as dynamical map parameters (\l addMapParameter).

    The release of this API with Qt 5.9 is a Technology Preview.

    \sa MapParameter, addMapParameter, removeMapParameter, clearMapParameters

    \since 5.9
*/
QList<QObject *> QDeclarativeGeoMap::mapParameters()
{
    QList<QObject *> ret;
    for (QDeclarativeGeoMapParameter *p : qAsConst(m_mapParameters))
        ret << p;
    return ret;
}

/*
    \internal
*/
void QDeclarativeGeoMap::addMapObject(QGeoMapObject *object)
{
    if (!object || object->map())
        return;

    if (!m_initialized) {
        m_pendingMapObjects.append(object);
        return;
    }

    int curObjects = m_map->mapObjects().size();
    // object adds itself to the map
    object->setMap(m_map);

    if (curObjects != m_map->mapObjects().size())
        emit mapObjectsChanged();
}

/*
    \internal
*/
void QDeclarativeGeoMap::removeMapObject(QGeoMapObject *object)
{
    if (!object || object->map() != m_map) // if !initialized this is fine, since both object and m_map are supposed to be NULL
        return;

    if (!m_initialized) {
        m_pendingMapObjects.removeOne(object);
        return;
    }

    int curObjects = m_map->mapObjects().size();
    // object adds itself to the map
    object->setMap(nullptr);

    if (curObjects != m_map->mapObjects().size())
        emit mapObjectsChanged();
}

/*
    \internal
*/
void QDeclarativeGeoMap::clearMapObjects()
{
    if (!m_initialized) {
        m_pendingMapObjects.clear();
    } else {
        const QList<QGeoMapObject *> objs = m_map->mapObjects();
        for (QGeoMapObject *o: objs)
            o->setMap(nullptr);
        if (objs.size())
            emit mapObjectsChanged();
    }
}

/*
    \internal
*/
QList<QGeoMapObject *> QDeclarativeGeoMap::mapObjects()
{
    if (!m_initialized)
        return m_pendingMapObjects;
    else
        return m_map->mapObjects();
}

/*!
    \qmlproperty list<MapItem> QtLocation::Map::mapItems

    Returns the list of all map items in no particular order.
    These items include items that were declared statically as part of
    the type declaration, as well as dynamical items (\l addMapItem,
    \l MapItemView).

    \sa addMapItem, removeMapItem, clearMapItems
*/

QList<QObject *> QDeclarativeGeoMap::mapItems()
{
    QList<QObject *> ret;
    foreach (const QPointer<QDeclarativeGeoMapItemBase> &ptr, m_mapItems) {
        if (ptr)
            ret << ptr.data();
    }
    return ret;
}

/*!
    \qmlmethod void QtLocation::Map::addMapItem(MapItem item)

    Adds the given \a item to the Map (for example MapQuickItem, MapCircle). If the object
    already is on the Map, it will not be added again.

    As an example, consider the case where you have a MapCircle representing your current position:

    \snippet declarative/maps.qml QtQuick import
    \snippet declarative/maps.qml QtLocation import
    \codeline
    \snippet declarative/maps.qml Map addMapItem MapCircle at current position

    \note MapItemViews cannot be added with this method.

    \sa mapItems, removeMapItem, clearMapItems
*/

void QDeclarativeGeoMap::addMapItem(QDeclarativeGeoMapItemBase *item)
{
    if (addMapItem_real(item))
        emit mapItemsChanged();
}

bool QDeclarativeGeoMap::addMapItem_real(QDeclarativeGeoMapItemBase *item)
{
    if (!item || item->quickMap())
        return false;
    // If the item comes from a MapItemGroup, do not reparent it.
    if (!qobject_cast<QDeclarativeGeoMapItemGroup *>(item->parentItem()))
        item->setParentItem(this);
    m_mapItems.append(item);
    if (m_map) {
        item->setMap(this, m_map);
        m_map->addMapItem(item);
    }
    return true;
}

/*!
    \qmlmethod void QtLocation::Map::removeMapItem(MapItem item)

    Removes the given \a item from the Map (for example MapQuickItem, MapCircle). If
    the MapItem does not exist or was not previously added to the map, the
    method does nothing.

    \sa mapItems, addMapItem, clearMapItems
*/
void QDeclarativeGeoMap::removeMapItem(QDeclarativeGeoMapItemBase *ptr)
{
    if (removeMapItem_real(ptr))
        emit mapItemsChanged();
}

bool QDeclarativeGeoMap::removeMapItem_real(QDeclarativeGeoMapItemBase *ptr)
{
    if (!ptr)
        return false;
    QPointer<QDeclarativeGeoMapItemBase> item(ptr);
    if (!m_mapItems.contains(item))
        return false;
    if (m_map)
        m_map->removeMapItem(ptr);
    if (item->parentItem() == this)
        item->setParentItem(0);
    item->setMap(0, 0);
    // these can be optimized for perf, as we already check the 'contains' above
    m_mapItems.removeOne(item);
    return true;
}

/*!
    \qmlmethod void QtLocation::Map::clearMapItems()

    Removes all items and item groups from the map.

    \sa mapItems, addMapItem, removeMapItem, addMapItemGroup, removeMapItemGroup
*/
void QDeclarativeGeoMap::clearMapItems()
{
    if (m_mapItems.isEmpty())
        return;

    int removed = 0;
    for (auto i : qAsConst(m_mapItemGroups)) {
        // Processing only top-level groups (!views)
        QDeclarativeGeoMapItemView *view = qobject_cast<QDeclarativeGeoMapItemView *>(i);
        if (view)
            continue;

        if (i->parentItem() != this)
            continue;

        removed += removeMapItemGroup_real(i);
    }

    for (auto i : qAsConst(m_mapItems))
        removed += removeMapItem_real(i);

    if (removed)
        emit mapItemsChanged();
}

/*!
    \qmlmethod void QtLocation::Map::addMapItemGroup(MapItemGroup itemGroup)

    Adds the map items contained in the given \a itemGroup to the Map
    (for example MapQuickItem, MapCircle).

    \sa MapItemGroup, removeMapItemGroup

    \since 5.9
*/
void QDeclarativeGeoMap::addMapItemGroup(QDeclarativeGeoMapItemGroup *itemGroup)
{
    if (addMapItemGroup_real(itemGroup))
        emit mapItemsChanged();
}

bool QDeclarativeGeoMap::addMapItemGroup_real(QDeclarativeGeoMapItemGroup *itemGroup)
{
    if (!itemGroup || itemGroup->quickMap()) // Already added to some map
        return false;

    itemGroup->setQuickMap(this);

    if (!isGroupNested(itemGroup))
        itemGroup->setParentItem(this);

    QPointer<QDeclarativeGeoMapItemGroup> g(itemGroup);
    m_mapItemGroups.append(g);

    const QList<QQuickItem *> quickKids = itemGroup->childItems();
    int count = 0;
    for (auto c: quickKids) {
        count += addMapChild(c); // this calls addMapItemGroup recursively, if needed
    }
    return count;
}

/*!
    \qmlmethod void QtLocation::Map::removeMapItemGroup(MapItemGroup itemGroup)

    Removes \a itemGroup and the items contained therein from the Map.

    \sa MapItemGroup, addMapItemGroup

    \since 5.9
*/
void QDeclarativeGeoMap::removeMapItemGroup(QDeclarativeGeoMapItemGroup *itemGroup)
{
    if (removeMapItemGroup_real(itemGroup))
        emit mapItemsChanged();
}

bool QDeclarativeGeoMap::removeMapItemGroup_real(QDeclarativeGeoMapItemGroup *itemGroup)
{
    if (!itemGroup || itemGroup->quickMap() != this) // cant remove an itemGroup added to another map
        return false;

    QPointer<QDeclarativeGeoMapItemGroup> g(itemGroup);
    if (!m_mapItemGroups.removeOne(g))
        return false;

    const QList<QQuickItem *> quickKids = itemGroup->childItems();
    int count = 0;
    for (auto c: quickKids) {
        count += removeMapChild(c);
    }
    itemGroup->setQuickMap(nullptr);
    if (itemGroup->parentItem() == this)
        itemGroup->setParentItem(0);
    return count;
}

/*!
    \qmlmethod void QtLocation::Map::removeMapItemView(MapItemView itemView)

    Removes \a itemView and the items instantiated by it from the Map.

    \sa MapItemView, addMapItemView

    \since 5.10
*/
void QDeclarativeGeoMap::removeMapItemView(QDeclarativeGeoMapItemView *itemView)
{
    if (removeMapItemView_real(itemView))
        emit mapItemsChanged();
}

bool QDeclarativeGeoMap::removeMapItemView_real(QDeclarativeGeoMapItemView *itemView)
{
    if (!itemView || itemView->m_map != this) // can't remove a view that is already added to another map
        return false;

    itemView->removeInstantiatedItems(false); // remove the items without using transitions AND abort ongoing ones
    itemView->m_map = 0;
    m_mapViews.removeOne(itemView);
    return removeMapItemGroup_real(itemView); // at this point, all delegate instances have been removed.
}

/*!
    \qmlmethod void QtLocation::Map::addMapItemView(MapItemView itemView)

    Adds \a itemView to the Map.

    \sa MapItemView, removeMapItemView

    \since 5.10
*/
void QDeclarativeGeoMap::addMapItemView(QDeclarativeGeoMapItemView *itemView)
{
    if (addMapItemView_real(itemView))
        emit mapItemsChanged();
}

bool QDeclarativeGeoMap::addMapItemView_real(QDeclarativeGeoMapItemView *itemView)
{
    if (!itemView || itemView->m_map) // can't add a view twice
        return false;

    int count = addMapItemGroup_real(itemView); // at this point, delegates aren't yet incubated.
    // Not appending it to m_mapViews because it seems unnecessary even if the
    // itemView is a child of this (in which case it would be destroyed
    m_mapViews.append(itemView);
    setupMapView(itemView);
    return count;
}

/*!
    \qmlproperty MapType QtLocation::Map::activeMapType

    \brief Access to the currently active \l{MapType}{map type}.

    This property can be set to change the active \l{MapType}{map type}.
    See the \l{Map::supportedMapTypes}{supportedMapTypes} property for possible values.

    \sa MapType
*/
void QDeclarativeGeoMap::setActiveMapType(QDeclarativeGeoMapType *mapType)
{
    if (m_activeMapType->mapType() != mapType->mapType()) {
        if (m_map) {
            if (mapType->mapType().pluginName() == m_plugin->name().toLatin1()) {
                m_map->setActiveMapType(mapType->mapType());
                m_activeMapType = mapType;
                emit activeMapTypeChanged();
            }
        } else {
            m_activeMapType = mapType;
            emit activeMapTypeChanged();
        }
    }
}

QDeclarativeGeoMapType * QDeclarativeGeoMap::activeMapType() const
{
    return m_activeMapType;
}

/*!
    \internal
*/
void QDeclarativeGeoMap::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    m_gestureArea->setSize(newGeometry.size());
    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    if (!m_map || newGeometry.size().isEmpty())
        return;

    m_map->setViewportSize(newGeometry.size().toSize());

    if (!m_initialized) {
        initialize();
    } else {
        setMinimumZoomLevel(m_map->minimumZoom(), false);

        // Update the center latitudinal threshold
        QGeoCameraData cameraData = m_map->cameraData();
        const double maximumCenterLatitudeAtZoom = m_map->maximumCenterLatitudeAtZoom(cameraData);
        const double minimumCenterLatitudeAtZoom = m_map->minimumCenterLatitudeAtZoom(cameraData);
        if (maximumCenterLatitudeAtZoom != m_maximumViewportLatitude
                || minimumCenterLatitudeAtZoom != m_minimumViewportLatitude) {
            m_maximumViewportLatitude = maximumCenterLatitudeAtZoom;
            m_minimumViewportLatitude = minimumCenterLatitudeAtZoom;
            QGeoCoordinate coord = cameraData.center();
            coord.setLatitude(qBound(m_minimumViewportLatitude, coord.latitude(), m_maximumViewportLatitude));
            cameraData.setCenter(coord);
            m_map->setCameraData(cameraData); // this polishes map items
        } else if (oldGeometry.size() != newGeometry.size()) {
            // polish map items
            for (const QPointer<QDeclarativeGeoMapItemBase> &i: qAsConst(m_mapItems)) {
                if (i)
                    i->polishAndUpdate();
            }
        }
    }

    /*
        The fitViewportTo*() functions depend on a valid map geometry.
        If they were called prior to the first resize they cause
        the zoomlevel to jump to 0 (showing the world). Therefore the
        calls were queued up until now.

        Multiple fitViewportTo*() calls replace each other.
     */
    if (m_pendingFitViewport && width() && height()) {
        fitViewportToGeoShape();
        m_pendingFitViewport = false;
    }

}

/*!
    \qmlmethod void QtLocation::Map::fitViewportToMapItems()

    Fits the current viewport to the boundary of all map items. The camera is positioned
    in the center of the map items, and at the largest integral zoom level possible which
    allows all map items to be visible on screen.

    \sa fitViewportToVisibleMapItems
*/
void QDeclarativeGeoMap::fitViewportToMapItems()
{
    fitViewportToMapItemsRefine(true, false);
}

/*!
    \qmlmethod void QtLocation::Map::fitViewportToVisibleMapItems()

    Fits the current viewport to the boundary of all \b visible map items.
    The camera is positioned in the center of the map items, and at the largest integral
    zoom level possible which allows all map items to be visible on screen.

    \sa fitViewportToMapItems
*/
void QDeclarativeGeoMap::fitViewportToVisibleMapItems()
{
    fitViewportToMapItemsRefine(true, true);
}

/*!
    \internal
*/
void QDeclarativeGeoMap::fitViewportToMapItemsRefine(bool refine, bool onlyVisible)
{
    if (!m_map)
        return;

    if (m_mapItems.size() == 0)
        return;

    double minX = qInf();
    double maxX = -qInf();
    double minY = qInf();
    double maxY = -qInf();
    double topLeftX = 0;
    double topLeftY = 0;
    double bottomRightX = 0;
    double bottomRightY = 0;
    bool haveQuickItem = false;

    // find bounds of all map items
    int itemCount = 0;
    for (int i = 0; i < m_mapItems.count(); ++i) {
        if (!m_mapItems.at(i))
            continue;
        QDeclarativeGeoMapItemBase *item = m_mapItems.at(i).data();
        if (!item || (onlyVisible && (!item->isVisible() || item->mapItemOpacity() <= 0.0)))
            continue;

        // skip quick items in the first pass and refine the fit later
        QDeclarativeGeoMapQuickItem *quickItem =
                qobject_cast<QDeclarativeGeoMapQuickItem*>(item);
        if (refine && quickItem) {
                haveQuickItem = true;
                continue;
        }
        // Force map items to update immediately. Needed to ensure correct item size and positions
        // when recursively calling this function.
        // TODO: See if we really need updatePolish on delegated items, in particular
        // in relation to
        // a) fitViewportToMapItems
        // b) presence of MouseArea
        //
        // This is also legacy code. It must be updated to not operate on screen sizes.
        if (item->isPolishScheduled())
           item->updatePolish();

        if (quickItem && quickItem->matrix_ && !quickItem->matrix_->m_matrix.isIdentity()) {
            // TODO: recalculate the center/zoom level so that the item becomes projectable again
            if (quickItem->zoomLevel() == 0.0) // the item is unprojectable, should be skipped.
                continue;

            QRectF brect = item->boundingRect();
            brect = quickItem->matrix_->m_matrix.mapRect(brect);
            QPointF transformedPosition = quickItem->matrix_->m_matrix * item->position();
            topLeftX = transformedPosition.x();
            topLeftY = transformedPosition.y();
            bottomRightX = topLeftX + brect.width();
            bottomRightY = topLeftY + brect.height();
        } else {
            topLeftX = item->position().x();
            topLeftY = item->position().y();
            bottomRightX = topLeftX + item->width();
            bottomRightY = topLeftY + item->height();
        }

        minX = qMin(minX, topLeftX);
        maxX = qMax(maxX, bottomRightX);
        minY = qMin(minY, topLeftY);
        maxY = qMax(maxY, bottomRightY);

        ++itemCount;
    }

    if (itemCount == 0) {
        if (haveQuickItem)
            fitViewportToMapItemsRefine(false, onlyVisible);
        return;
    }
    double bboxWidth = maxX - minX;
    double bboxHeight = maxY - minY;
    double bboxCenterX = minX + (bboxWidth / 2.0);
    double bboxCenterY = minY + (bboxHeight / 2.0);

    // position camera to the center of bounding box
    QGeoCoordinate coordinate;
    coordinate = m_map->geoProjection().itemPositionToCoordinate(QDoubleVector2D(bboxCenterX, bboxCenterY), false);
    setProperty("center", QVariant::fromValue(coordinate));

    // adjust zoom
    double bboxWidthRatio = bboxWidth / (bboxWidth + bboxHeight);
    double mapWidthRatio = width() / (width() + height());
    double zoomRatio;

    if (bboxWidthRatio > mapWidthRatio)
        zoomRatio = bboxWidth / width();
    else
        zoomRatio = bboxHeight / height();

    qreal newZoom = std::log10(zoomRatio) / std::log10(0.5);
    newZoom = std::floor(qMax(minimumZoomLevel(), (zoomLevel() + newZoom)));
    setProperty("zoomLevel", QVariant::fromValue(newZoom));

    // as map quick items retain the same screen size after the camera zooms in/out
    // we refine the viewport again to achieve better results
    if (refine)
        fitViewportToMapItemsRefine(false, onlyVisible);
}

/*!
    \internal
*/
void QDeclarativeGeoMap::mousePressEvent(QMouseEvent *event)
{
    if (isInteractive())
        m_gestureArea->handleMousePressEvent(event);
    else
        QQuickItem::mousePressEvent(event);
}

/*!
    \internal
*/
void QDeclarativeGeoMap::mouseMoveEvent(QMouseEvent *event)
{
    if (isInteractive())
        m_gestureArea->handleMouseMoveEvent(event);
    else
        QQuickItem::mouseMoveEvent(event);
}

/*!
    \internal
*/
void QDeclarativeGeoMap::mouseReleaseEvent(QMouseEvent *event)
{
    if (isInteractive())
        m_gestureArea->handleMouseReleaseEvent(event);
    else
        QQuickItem::mouseReleaseEvent(event);
}

/*!
    \internal
*/
void QDeclarativeGeoMap::mouseUngrabEvent()
{
    if (isInteractive())
        m_gestureArea->handleMouseUngrabEvent();
    else
        QQuickItem::mouseUngrabEvent();
}

void QDeclarativeGeoMap::touchUngrabEvent()
{
    if (isInteractive())
        m_gestureArea->handleTouchUngrabEvent();
    else
        QQuickItem::touchUngrabEvent();
}

/*!
    \internal
*/
void QDeclarativeGeoMap::touchEvent(QTouchEvent *event)
{
    if (isInteractive()) {
        m_gestureArea->handleTouchEvent(event);
    } else {
        //ignore event so sythesized event is generated;
        QQuickItem::touchEvent(event);
    }
}

#if QT_CONFIG(wheelevent)
/*!
    \internal
*/
void QDeclarativeGeoMap::wheelEvent(QWheelEvent *event)
{
    if (isInteractive())
        m_gestureArea->handleWheelEvent(event);
    else
        QQuickItem::wheelEvent(event);

}
#endif

/*!
    \internal
*/
bool QDeclarativeGeoMap::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    Q_UNUSED(item)
    if (!isVisible() || !isEnabled() || !isInteractive())
        return QQuickItem::childMouseEventFilter(item, event);

    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
        return sendMouseEvent(static_cast<QMouseEvent *>(event));
    case QEvent::UngrabMouse: {
        QQuickWindow *win = window();
        if (!win) break;
        if (!win->mouseGrabberItem() ||
                (win->mouseGrabberItem() &&
                 win->mouseGrabberItem() != this)) {
            // child lost grab, we could even lost
            // some events if grab already belongs for example
            // in item in diffrent window , clear up states
            mouseUngrabEvent();
        }
        break;
    }
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        if (static_cast<QTouchEvent *>(event)->touchPoints().count() >= 2) {
            // 1 touch point = handle with MouseEvent (event is always synthesized)
            // let the synthesized mouse event grab the mouse,
            // note there is no mouse grabber at this point since
            // touch event comes first (see Qt::AA_SynthesizeMouseForUnhandledTouchEvents)
            return sendTouchEvent(static_cast<QTouchEvent *>(event));
        }
    default:
        break;
    }
    return QQuickItem::childMouseEventFilter(item, event);
}

bool QDeclarativeGeoMap::sendMouseEvent(QMouseEvent *event)
{
    QPointF localPos = mapFromScene(event->windowPos());
    QQuickWindow *win = window();
    QQuickItem *grabber = win ? win->mouseGrabberItem() : 0;
    bool stealEvent = m_gestureArea->isActive();

    if ((stealEvent || contains(localPos)) && (!grabber || (!grabber->keepMouseGrab() && !grabber->keepTouchGrab()))) {
        QScopedPointer<QMouseEvent> mouseEvent(QQuickWindowPrivate::cloneMouseEvent(event, &localPos));
        mouseEvent->setAccepted(false);

        switch (mouseEvent->type()) {
        case QEvent::MouseMove:
            m_gestureArea->handleMouseMoveEvent(mouseEvent.data());
            break;
        case QEvent::MouseButtonPress:
            m_gestureArea->handleMousePressEvent(mouseEvent.data());
            break;
        case QEvent::MouseButtonRelease:
            m_gestureArea->handleMouseReleaseEvent(mouseEvent.data());
            break;
        default:
            break;
        }

        stealEvent = m_gestureArea->isActive();
        grabber = win ? win->mouseGrabberItem() : 0;

        if (grabber && stealEvent && !grabber->keepMouseGrab() && !grabber->keepTouchGrab() && grabber != this)
            grabMouse();

        if (stealEvent) {
            //do not deliver
            event->setAccepted(true);
            return true;
        } else {
            return false;
        }
    }

    return false;
}

bool QDeclarativeGeoMap::sendTouchEvent(QTouchEvent *event)
{
    QQuickPointerDevice *touchDevice = QQuickPointerDevice::touchDevice(event->device());
    const QTouchEvent::TouchPoint &point = event->touchPoints().first();
    QQuickWindowPrivate *windowPriv = QQuickWindowPrivate::get(window());

    auto touchPointGrabberItem = [touchDevice, windowPriv](const QTouchEvent::TouchPoint &point) -> QQuickItem* {
        if (QQuickEventPoint *eventPointer = windowPriv->pointerEventInstance(touchDevice)->pointById(point.id()))
            return eventPointer->grabberItem();
        return nullptr;
    };

    QQuickItem *grabber = touchPointGrabberItem(point);

    bool stealEvent = m_gestureArea->isActive();
    bool containsPoint = contains(mapFromScene(point.scenePos()));

    if ((stealEvent || containsPoint) && (!grabber || !grabber->keepTouchGrab())) {
        QScopedPointer<QTouchEvent> touchEvent(new QTouchEvent(event->type(), event->device(), event->modifiers(), event->touchPointStates(), event->touchPoints()));
        touchEvent->setTimestamp(event->timestamp());
        touchEvent->setAccepted(false);

        m_gestureArea->handleTouchEvent(touchEvent.data());
        stealEvent = m_gestureArea->isActive();
        grabber = touchPointGrabberItem(point);

        if (grabber && stealEvent && !grabber->keepTouchGrab() && grabber != this) {
            QVector<int> ids;
            foreach (const QTouchEvent::TouchPoint &tp, event->touchPoints()) {
                if (!(tp.state() & Qt::TouchPointReleased)) {
                    ids.append(tp.id());
                }
            }
            grabTouchPoints(ids);
        }

        if (stealEvent) {
            //do not deliver
            event->setAccepted(true);
            return true;
        } else {
            return false;
        }
    }

    return false;
}

QT_END_NAMESPACE
