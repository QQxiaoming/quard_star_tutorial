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

#include "qdeclarativepolygonmapitem_p.h"
#include "qlocationutils_p.h"
#include "error_messages_p.h"
#include "locationvaluetypehelper_p.h"
#include <QtLocation/private/qgeomap_p.h>

#include <QtCore/QScopedValueRollback>
#include <QtGui/private/qtriangulator_p.h>
#include <QtQml/QQmlInfo>
#include <QtQml/private/qqmlengine_p.h>
#include <QPainter>
#include <QPainterPath>
#include <qnumeric.h>

#include <QtPositioning/private/qdoublevector2d_p.h>
#include <QtPositioning/private/qclipperutils_p.h>

/* poly2tri triangulator includes */
#include <clip2tri.h>
#include <earcut.hpp>
#include <array>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MapPolygon
    \instantiates QDeclarativePolygonMapItem
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.5

    \brief The MapPolygon type displays a polygon on a Map.

    The MapPolygon type displays a polygon on a Map, specified in terms of an ordered list of
    \l {QtPositioning::coordinate}{coordinates}. For best appearance and results, polygons should be
    simple (not self-intersecting).

    The \l {QtPositioning::coordinate}{coordinates} on the path cannot be directly changed after
    being added to the Polygon.  Instead, copy the \l path into a var, modify the copy and reassign
    the copy back to the \l path.

    \code
    var path = mapPolygon.path;
    path[0].latitude = 5;
    mapPolygon.path = path;
    \endcode

    Coordinates can also be added and removed at any time using the \l addCoordinate and
    \l removeCoordinate methods.

    For drawing rectangles with "straight" edges (same latitude across one
    edge, same latitude across the other), the \l MapRectangle type provides
    a simpler, two-point API.

    By default, the polygon is displayed as a 1 pixel black border with no
    fill. To change its appearance, use the \l color, \l border.color and
    \l border.width properties.

    \note Since MapPolygons are geographic items, dragging a MapPolygon
    (through the use of \l MouseArea) causes its vertices to be
    recalculated in the geographic coordinate space. The edges retain the
    same geographic lengths (latitude and longitude differences between the
    vertices), but they remain straight. Apparent stretching of the item occurs
    when dragged to a different latitude.

    \section2 Performance

    MapPolygons have a rendering cost that is O(n) with respect to the number
    of vertices. This means that the per frame cost of having a Polygon on the
    Map grows in direct proportion to the number of points on the Polygon. There
    is an additional triangulation cost (approximately O(n log n)) which is
    currently paid with each frame, but in future may be paid only upon adding
    or removing points.

    Like the other map objects, MapPolygon is normally drawn without a smooth
    appearance. Setting the \l {Item::opacity}{opacity} property will force the object to
    be blended, which decreases performance considerably depending on the hardware in use.

    \section2 Example Usage

    The following snippet shows a MapPolygon being used to display a triangle,
    with three vertices near Brisbane, Australia. The triangle is filled in
    green, with a 1 pixel black border.

    \code
    Map {
        MapPolygon {
            color: 'green'
            path: [
                { latitude: -27, longitude: 153.0 },
                { latitude: -27, longitude: 154.1 },
                { latitude: -28, longitude: 153.5 }
            ]
        }
    }
    \endcode

    \image api-mappolygon.png
*/

QGeoMapPolygonGeometry::QGeoMapPolygonGeometry()
:   assumeSimple_(false)
{
}

/*!
    \internal
*/
void QGeoMapPolygonGeometry::updateSourcePoints(const QGeoMap &map,
                                                const QList<QDoubleVector2D> &path)
{
    if (!sourceDirty_)
        return;
    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map.geoProjection());
    srcPath_ = QPainterPath();

    // build the actual path
    // The approach is the same as described in QGeoMapPolylineGeometry::updateSourcePoints
    srcOrigin_ = geoLeftBound_;
    double unwrapBelowX = 0;
    QDoubleVector2D leftBoundWrapped = p.wrapMapProjection(p.geoToMapProjection(geoLeftBound_));
    if (preserveGeometry_)
        unwrapBelowX = leftBoundWrapped.x();

    QList<QDoubleVector2D> wrappedPath;
    wrappedPath.reserve(path.size());
    QDoubleVector2D wrappedLeftBound(qInf(), qInf());
    // 1)
    for (int i = 0; i < path.size(); ++i) {
        const QDoubleVector2D &coord = path.at(i);
        QDoubleVector2D wrappedProjection = p.wrapMapProjection(coord);

        // We can get NaN if the map isn't set up correctly, or the projection
        // is faulty -- probably best thing to do is abort
        if (!qIsFinite(wrappedProjection.x()) || !qIsFinite(wrappedProjection.y()))
            return;

        const bool isPointLessThanUnwrapBelowX = (wrappedProjection.x() < leftBoundWrapped.x());
        // unwrap x to preserve geometry if moved to border of map
        if (preserveGeometry_ && isPointLessThanUnwrapBelowX) {
            double distance = wrappedProjection.x() - unwrapBelowX;
            if (distance < 0.0)
                distance += 1.0;
            wrappedProjection.setX(unwrapBelowX + distance);
        }
        if (wrappedProjection.x() < wrappedLeftBound.x() || (wrappedProjection.x() == wrappedLeftBound.x() && wrappedProjection.y() < wrappedLeftBound.y())) {
            wrappedLeftBound = wrappedProjection;
        }
        wrappedPath.append(wrappedProjection);
    }

    // 2)
    QList<QList<QDoubleVector2D> > clippedPaths;
    const QList<QDoubleVector2D> &visibleRegion = p.projectableGeometry();
    if (visibleRegion.size()) {
        c2t::clip2tri clipper;
        clipper.addSubjectPath(QClipperUtils::qListToPath(wrappedPath), true);
        clipper.addClipPolygon(QClipperUtils::qListToPath(visibleRegion));
        Paths res = clipper.execute(c2t::clip2tri::Intersection, QtClipperLib::pftEvenOdd, QtClipperLib::pftEvenOdd);
        clippedPaths = QClipperUtils::pathsToQList(res);

        // 2.1) update srcOrigin_ and leftBoundWrapped with the point with minimum X
        QDoubleVector2D lb(qInf(), qInf());
        for (const QList<QDoubleVector2D> &path: clippedPaths)
            for (const QDoubleVector2D &p: path)
                if (p.x() < lb.x() || (p.x() == lb.x() && p.y() < lb.y()))
                    // y-minimization needed to find the same point on polygon and border
                    lb = p;

        if (qIsInf(lb.x())) // e.g., when the polygon is clipped entirely
            return;

        // 2.2) Prevent the conversion to and from clipper from introducing negative offsets which
        //      in turn will make the geometry wrap around.
        lb.setX(qMax(wrappedLeftBound.x(), lb.x()));
        leftBoundWrapped = lb;
        srcOrigin_ = p.mapProjectionToGeo(p.unwrapMapProjection(lb));
    } else {
        clippedPaths.append(wrappedPath);
    }

    // 3)
    QDoubleVector2D origin = p.wrappedMapProjectionToItemPosition(leftBoundWrapped);
    for (const QList<QDoubleVector2D> &path: clippedPaths) {
        QDoubleVector2D lastAddedPoint;
        for (int i = 0; i < path.size(); ++i) {
            QDoubleVector2D point = p.wrappedMapProjectionToItemPosition(path.at(i));
            point = point - origin; // (0,0) if point == geoLeftBound_

            if (i == 0) {
                srcPath_.moveTo(point.toPointF());
                lastAddedPoint = point;
            } else {
                if ((point - lastAddedPoint).manhattanLength() > 3 ||
                        i == path.size() - 1) {
                    srcPath_.lineTo(point.toPointF());
                    lastAddedPoint = point;
                }
            }
        }
        srcPath_.closeSubpath();
    }

    if (!assumeSimple_)
        srcPath_ = srcPath_.simplified();

    sourceBounds_ = srcPath_.boundingRect();
}

/*!
    \internal
*/
void QGeoMapPolygonGeometry::updateScreenPoints(const QGeoMap &map, qreal strokeWidth)
{
    if (!screenDirty_)
        return;

    if (map.viewportWidth() == 0 || map.viewportHeight() == 0) {
        clear();
        return;
    }

    // The geometry has already been clipped against the visible region projection in wrapped mercator space.
    QPainterPath ppi = srcPath_;
    clear();

    // a polygon requires at least 3 points;
    if (ppi.elementCount() < 3)
        return;

    // translate the path into top-left-centric coordinates
    QRectF bb = ppi.boundingRect();
    ppi.translate(-bb.left(), -bb.top());
    firstPointOffset_ = -1 * bb.topLeft();

    ppi.closeSubpath();
    screenOutline_ = ppi;

    using Coord = double;
    using N = uint32_t;
    using Point = std::array<Coord, 2>;

    std::vector<std::vector<Point>> polygon;
    polygon.push_back(std::vector<Point>());
    std::vector<Point> &poly = polygon.front();
    // ... fill polygon structure with actual data

    for (int i = 0; i < ppi.elementCount(); ++i) {
        const QPainterPath::Element e = ppi.elementAt(i);
        if (e.isMoveTo() || i == ppi.elementCount() - 1
                || (qAbs(e.x - poly.front()[0]) < 0.1
                    && qAbs(e.y - poly.front()[1]) < 0.1)) {
            Point p = {{ e.x, e.y }};
            poly.push_back( p );
        } else if (e.isLineTo()) {
            Point p = {{ e.x, e.y }};
            poly.push_back( p );
        } else {
            qWarning("Unhandled element type in polygon painterpath");
        }
    }

    if (poly.size() > 2) {
        // Run tessellation
        // Returns array of indices that refer to the vertices of the input polygon.
        // Three subsequent indices form a triangle.
        screenVertices_.clear();
        screenIndices_.clear();
        for (const auto &p : poly)
            screenVertices_ << QPointF(p[0], p[1]);
        std::vector<N> indices = qt_mapbox::earcut<N>(polygon);
        for (const auto &i: indices)
            screenIndices_ << quint32(i);
    }

    screenBounds_ = ppi.boundingRect();
    if (strokeWidth != 0.0)
        this->translate(QPointF(strokeWidth, strokeWidth));
}

QDeclarativePolygonMapItem::QDeclarativePolygonMapItem(QQuickItem *parent)
:   QDeclarativeGeoMapItemBase(parent), border_(this), color_(Qt::transparent), dirtyMaterial_(true),
    updatingGeometry_(false)
{
    m_itemType = QGeoMap::MapPolygon;
    setFlag(ItemHasContents, true);
    QObject::connect(&border_, SIGNAL(colorChanged(QColor)),
                     this, SLOT(markSourceDirtyAndUpdate()));
    QObject::connect(&border_, SIGNAL(widthChanged(qreal)),
                     this, SLOT(markSourceDirtyAndUpdate()));
}

QDeclarativePolygonMapItem::~QDeclarativePolygonMapItem()
{
}

/*!
    \qmlpropertygroup Location::MapPolygon::border
    \qmlproperty int MapPolygon::border.width
    \qmlproperty color MapPolygon::border.color

    This property is part of the border property group. The border property
    group holds the width and color used to draw the border of the polygon.

    The width is in pixels and is independent of the zoom level of the map.

    The default values correspond to a black border with a width of 1 pixel.
    For no line, use a width of 0 or a transparent color.
*/

QDeclarativeMapLineProperties *QDeclarativePolygonMapItem::border()
{
    return &border_;
}

/*!
    \internal
*/
void QDeclarativePolygonMapItem::setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map)
{
    QDeclarativeGeoMapItemBase::setMap(quickMap,map);
    if (map) {
        regenerateCache();
        geometry_.markSourceDirty();
        borderGeometry_.markSourceDirty();
        polishAndUpdate();
    }
}

/*!
    \qmlproperty list<coordinate> MapPolygon::path

    This property holds the ordered list of coordinates which
    define the polygon.
    Having less than 3 different coordinates in the path results in undefined behavior.

    \sa addCoordinate, removeCoordinate
*/
QJSValue QDeclarativePolygonMapItem::path() const
{
    return fromList(this, geopath_.path());
}

void QDeclarativePolygonMapItem::setPath(const QJSValue &value)
{
    if (!value.isArray())
        return;

    QList<QGeoCoordinate> pathList = toList(this, value);

    // Equivalent to QDeclarativePolylineMapItem::setPathFromGeoList
    if (geopath_.path() == pathList)
        return;

    geopath_.setPath(pathList);

    regenerateCache();
    geometry_.setPreserveGeometry(true, geopath_.boundingGeoRectangle().topLeft());
    borderGeometry_.setPreserveGeometry(true, geopath_.boundingGeoRectangle().topLeft());
    markSourceDirtyAndUpdate();
    emit pathChanged();
}

/*!
    \qmlmethod void MapPolygon::addCoordinate(coordinate)

    Adds a coordinate to the path.

    \sa removeCoordinate, path
*/

void QDeclarativePolygonMapItem::addCoordinate(const QGeoCoordinate &coordinate)
{
    if (!coordinate.isValid())
        return;

    geopath_.addCoordinate(coordinate);
    updateCache();
    geometry_.setPreserveGeometry(true, geopath_.boundingGeoRectangle().topLeft());
    borderGeometry_.setPreserveGeometry(true, geopath_.boundingGeoRectangle().topLeft());
    markSourceDirtyAndUpdate();
    emit pathChanged();
}

/*!
    \qmlmethod void MapPolygon::removeCoordinate(coordinate)

    Removes \a coordinate from the path. If there are multiple instances of the
    same coordinate, the one added last is removed.

    If \a coordinate is not in the path this method does nothing.

    \sa addCoordinate, path
*/
void QDeclarativePolygonMapItem::removeCoordinate(const QGeoCoordinate &coordinate)
{
    int length = geopath_.path().length();
    geopath_.removeCoordinate(coordinate);
    if (geopath_.path().length() == length)
        return;

    regenerateCache();
    geometry_.setPreserveGeometry(true, geopath_.boundingGeoRectangle().topLeft());
    borderGeometry_.setPreserveGeometry(true, geopath_.boundingGeoRectangle().topLeft());
    markSourceDirtyAndUpdate();
    emit pathChanged();
}

/*!
    \qmlproperty color MapPolygon::color

    This property holds the color used to fill the polygon.

    The default value is transparent.
*/

QColor QDeclarativePolygonMapItem::color() const
{
    return color_;
}

void QDeclarativePolygonMapItem::setColor(const QColor &color)
{
    if (color_ == color)
        return;

    color_ = color;
    dirtyMaterial_ = true;
    update();
    emit colorChanged(color_);
}

/*!
    \internal
*/
QSGNode *QDeclarativePolygonMapItem::updateMapItemPaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    MapPolygonNode *node = static_cast<MapPolygonNode *>(oldNode);

    if (!node)
        node = new MapPolygonNode();

    //TODO: update only material
    if (geometry_.isScreenDirty() || borderGeometry_.isScreenDirty() || dirtyMaterial_) {
        node->update(color_, border_.color(), &geometry_, &borderGeometry_);
        geometry_.setPreserveGeometry(false);
        borderGeometry_.setPreserveGeometry(false);
        geometry_.markClean();
        borderGeometry_.markClean();
        dirtyMaterial_ = false;
    }
    return node;
}

/*!
    \internal
*/
void QDeclarativePolygonMapItem::updatePolish()
{
    if (!map() || map()->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
        return;
    if (geopath_.path().length() == 0) { // Possibly cleared
        geometry_.clear();
        borderGeometry_.clear();
        setWidth(0);
        setHeight(0);
        return;
    }

    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map()->geoProjection());
    QScopedValueRollback<bool> rollback(updatingGeometry_);
    updatingGeometry_ = true;

    geometry_.updateSourcePoints(*map(), geopathProjected_);
    geometry_.updateScreenPoints(*map(), border_.width());

    QList<QGeoMapItemGeometry *> geoms;
    geoms << &geometry_;
    borderGeometry_.clear();

    if (border_.color() != Qt::transparent && border_.width() > 0) {
        QList<QDoubleVector2D> closedPath = geopathProjected_;
        closedPath << closedPath.first();

        borderGeometry_.setPreserveGeometry(true, geopath_.boundingGeoRectangle().topLeft());

        const QGeoCoordinate &geometryOrigin = geometry_.origin();

        borderGeometry_.srcPoints_.clear();
        borderGeometry_.srcPointTypes_.clear();

        QDoubleVector2D borderLeftBoundWrapped;
        QList<QList<QDoubleVector2D > > clippedPaths = borderGeometry_.clipPath(*map(), closedPath, borderLeftBoundWrapped);
        if (clippedPaths.size()) {
            borderLeftBoundWrapped = p.geoToWrappedMapProjection(geometryOrigin);
            borderGeometry_.pathToScreen(*map(), clippedPaths, borderLeftBoundWrapped);
            borderGeometry_.updateScreenPoints(*map(), border_.width());

            geoms << &borderGeometry_;
        } else {
            borderGeometry_.clear();
        }
    }

    QRectF combined = QGeoMapItemGeometry::translateToCommonOrigin(geoms);
    setWidth(combined.width() + 2 * border_.width());
    setHeight(combined.height() + 2 * border_.width());

    setPositionOnMap(geometry_.origin(), -1 * geometry_.sourceBoundingBox().topLeft()
                                            + QPointF(border_.width(), border_.width()));
}

void QDeclarativePolygonMapItem::markSourceDirtyAndUpdate()
{
    geometry_.markSourceDirty();
    borderGeometry_.markSourceDirty();
    polishAndUpdate();
}

/*!
    \internal
*/
void QDeclarativePolygonMapItem::afterViewportChanged(const QGeoMapViewportChangeEvent &event)
{
    if (event.mapSize.width() <= 0 || event.mapSize.height() <= 0)
        return;

    geometry_.setPreserveGeometry(true, geometry_.geoLeftBound());
    borderGeometry_.setPreserveGeometry(true, borderGeometry_.geoLeftBound());
    geometry_.markSourceDirty();
    borderGeometry_.markSourceDirty();
    polishAndUpdate();
}

/*!
    \internal
*/
void QDeclarativePolygonMapItem::regenerateCache()
{
    if (!map() || map()->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
        return;
    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map()->geoProjection());
    geopathProjected_.clear();
    geopathProjected_.reserve(geopath_.path().size());
    for (const QGeoCoordinate &c : geopath_.path())
        geopathProjected_ << p.geoToMapProjection(c);
}

/*!
    \internal
*/
void QDeclarativePolygonMapItem::updateCache()
{
    if (!map() || map()->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
        return;
    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map()->geoProjection());
    geopathProjected_ << p.geoToMapProjection(geopath_.path().last());
}

/*!
    \internal
*/
bool QDeclarativePolygonMapItem::contains(const QPointF &point) const
{
    return (geometry_.contains(point) || borderGeometry_.contains(point));
}

const QGeoShape &QDeclarativePolygonMapItem::geoShape() const
{
    return geopath_;
}

void QDeclarativePolygonMapItem::setGeoShape(const QGeoShape &shape)
{
    if (shape == geopath_)
        return;

    geopath_ = shape;
    regenerateCache();
    geometry_.setPreserveGeometry(true, geopath_.boundingGeoRectangle().topLeft());
    borderGeometry_.setPreserveGeometry(true, geopath_.boundingGeoRectangle().topLeft());
    markSourceDirtyAndUpdate();
    emit pathChanged();
}

/*!
    \internal
*/
void QDeclarativePolygonMapItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (!map() || !geopath_.isValid() || updatingGeometry_ || newGeometry.topLeft() == oldGeometry.topLeft()) {
        QDeclarativeGeoMapItemBase::geometryChanged(newGeometry, oldGeometry);
        return;
    }
    // TODO: change the algorithm to preserve the distances and size!
    QGeoCoordinate newCenter = map()->geoProjection().itemPositionToCoordinate(QDoubleVector2D(newGeometry.center()), false);
    QGeoCoordinate oldCenter = map()->geoProjection().itemPositionToCoordinate(QDoubleVector2D(oldGeometry.center()), false);
    if (!newCenter.isValid() || !oldCenter.isValid())
        return;
    double offsetLongi = newCenter.longitude() - oldCenter.longitude();
    double offsetLati = newCenter.latitude() - oldCenter.latitude();
    if (offsetLati == 0.0 && offsetLongi == 0.0)
        return;

    geopath_.translate(offsetLati, offsetLongi);
    regenerateCache();
    geometry_.setPreserveGeometry(true, geopath_.boundingGeoRectangle().topLeft());
    borderGeometry_.setPreserveGeometry(true, geopath_.boundingGeoRectangle().topLeft());
    markSourceDirtyAndUpdate();
    emit pathChanged();

    // Not calling QDeclarativeGeoMapItemBase::geometryChanged() as it will be called from a nested
    // call to this function.
}

//////////////////////////////////////////////////////////////////////

MapPolygonNode::MapPolygonNode() :
    border_(new MapPolylineNode()),
    geometry_(QSGGeometry::defaultAttributes_Point2D(), 0)
{
    geometry_.setDrawingMode(QSGGeometry::DrawTriangles);
    QSGGeometryNode::setMaterial(&fill_material_);
    QSGGeometryNode::setGeometry(&geometry_);

    appendChildNode(border_);
}

MapPolygonNode::~MapPolygonNode()
{
}

/*!
    \internal
*/
void MapPolygonNode::update(const QColor &fillColor, const QColor &borderColor,
                            const QGeoMapItemGeometry *fillShape,
                            const QGeoMapItemGeometry *borderShape)
{
    /* Do the border update first */
    border_->update(borderColor, borderShape);

    /* If we have neither fill nor border with valid points, block the whole
     * tree. We can't just block the fill without blocking the border too, so
     * we're a little conservative here (maybe at the expense of rendering
     * accuracy) */
    if (fillShape->size() == 0 && borderShape->size() == 0) {
            setSubtreeBlocked(true);
            return;
    }
    setSubtreeBlocked(false);


    QSGGeometry *fill = QSGGeometryNode::geometry();
    fillShape->allocateAndFill(fill);
    markDirty(DirtyGeometry);

    if (fillColor != fill_material_.color()) {
        fill_material_.setColor(fillColor);
        setMaterial(&fill_material_);
        markDirty(DirtyMaterial);
    }
}

QT_END_NAMESPACE
