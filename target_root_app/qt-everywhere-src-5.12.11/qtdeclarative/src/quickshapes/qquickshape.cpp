/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
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

#include "qquickshape_p.h"
#include "qquickshape_p_p.h"
#include "qquickshapegenericrenderer_p.h"
#include "qquickshapenvprrenderer_p.h"
#include "qquickshapesoftwarerenderer_p.h"
#include <private/qsgtexture_p.h>
#include <private/qquicksvgparser_p.h>
#include <QtGui/private/qdrawhelper_p.h>
#include <QOpenGLFunctions>
#include <QLoggingCategory>

#if defined(QT_STATIC)
static void initResources()
{
    Q_INIT_RESOURCE(qtquickshapes);
}
#endif

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(QQSHAPE_LOG_TIME_DIRTY_SYNC, "qt.shape.time.sync")

/*!
    \qmlmodule QtQuick.Shapes 1.\QtMinorVersion
    \title Qt Quick Shapes QML Types
    \ingroup qmlmodules
    \brief Provides QML types for drawing stroked and filled shapes.

    To use the types in this module, import the module with the following line:

    \qml \QtMinorVersion
    import QtQuick.Shapes 1.\1
    \endqml
*/

QQuickShapeStrokeFillParams::QQuickShapeStrokeFillParams()
    : strokeColor(Qt::white),
      strokeWidth(1),
      fillColor(Qt::white),
      fillRule(QQuickShapePath::OddEvenFill),
      joinStyle(QQuickShapePath::BevelJoin),
      miterLimit(2),
      capStyle(QQuickShapePath::SquareCap),
      strokeStyle(QQuickShapePath::SolidLine),
      dashOffset(0),
      fillGradient(nullptr)
{
    dashPattern << 4 << 2; // 4 * strokeWidth dash followed by 2 * strokeWidth space
}

/*!
    \qmltype ShapePath
    \instantiates QQuickShapePath
    \inqmlmodule QtQuick.Shapes
    \ingroup qtquick-paths
    \ingroup qtquick-views
    \inherits Path
    \brief Describes a Path and associated properties for stroking and filling.
    \since 5.10

    A \l Shape contains one or more ShapePath elements. At least one ShapePath is
    necessary in order to have a Shape output anything visible. A ShapePath
    itself is a \l Path with additional properties describing the stroking and
    filling parameters, such as the stroke width and color, the fill color or
    gradient, join and cap styles, and so on. As with ordinary \l Path objects,
    ShapePath also contains a list of path elements like \l PathMove, \l PathLine,
    \l PathCubic, \l PathQuad, \l PathArc, together with a starting position.

    Any property changes in these data sets will be bubble up and change the
    output of the Shape. This means that it is simple and easy to change, or
    even animate, the starting and ending position, control points, or any
    stroke or fill parameters using the usual QML bindings and animation types
    like NumberAnimation.

    In the following example the line join style changes automatically based on
    the value of joinStyleIndex:

    \qml
    ShapePath {
        strokeColor: "black"
        strokeWidth: 16
        fillColor: "transparent"
        capStyle: ShapePath.RoundCap

        property int joinStyleIndex: 0

        property variant styles: [
            ShapePath.BevelJoin,
            ShapePath.MiterJoin,
            ShapePath.RoundJoin
        ]

        joinStyle: styles[joinStyleIndex]

        startX: 30
        startY: 30
        PathLine { x: 100; y: 100 }
        PathLine { x: 30; y: 100 }
    }
    \endqml

    Once associated with a Shape, here is the output with a joinStyleIndex
    of 2 (ShapePath.RoundJoin):

    \image visualpath-code-example.png

    \sa {Qt Quick Examples - Shapes}, Shape
 */

QQuickShapePathPrivate::QQuickShapePathPrivate()
    : dirty(DirtyAll)
{
    // Set this QQuickPath to be a ShapePath
    isShapePath = true;
}

QQuickShapePath::QQuickShapePath(QObject *parent)
    : QQuickPath(*(new QQuickShapePathPrivate), parent)
{
    // The inherited changed() and the shapePathChanged() signals remain
    // distinct, and this is intentional. Combining the two is not possible due
    // to the difference in semantics and the need to act (see dirty flag
    // below) differently on QQuickPath-related changes.

    connect(this, &QQuickPath::changed, [this]() {
        Q_D(QQuickShapePath);
        d->dirty |= QQuickShapePathPrivate::DirtyPath;
        emit shapePathChanged();
    });
}

QQuickShapePath::~QQuickShapePath()
{
}

/*!
    \qmlproperty color QtQuick.Shapes::ShapePath::strokeColor

    This property holds the stroking color.

    When set to \c transparent, no stroking occurs.

    The default value is \c white.
 */

QColor QQuickShapePath::strokeColor() const
{
    Q_D(const QQuickShapePath);
    return d->sfp.strokeColor;
}

void QQuickShapePath::setStrokeColor(const QColor &color)
{
    Q_D(QQuickShapePath);
    if (d->sfp.strokeColor != color) {
        d->sfp.strokeColor = color;
        d->dirty |= QQuickShapePathPrivate::DirtyStrokeColor;
        emit strokeColorChanged();
        emit shapePathChanged();
    }
}

/*!
    \qmlproperty real QtQuick.Shapes::ShapePath::strokeWidth

    This property holds the stroke width.

    When set to a negative value, no stroking occurs.

    The default value is 1.
 */

qreal QQuickShapePath::strokeWidth() const
{
    Q_D(const QQuickShapePath);
    return d->sfp.strokeWidth;
}

void QQuickShapePath::setStrokeWidth(qreal w)
{
    Q_D(QQuickShapePath);
    if (d->sfp.strokeWidth != w) {
        d->sfp.strokeWidth = w;
        d->dirty |= QQuickShapePathPrivate::DirtyStrokeWidth;
        emit strokeWidthChanged();
        emit shapePathChanged();
    }
}

/*!
    \qmlproperty color QtQuick.Shapes::ShapePath::fillColor

    This property holds the fill color.

    When set to \c transparent, no filling occurs.

    The default value is \c white.
 */

QColor QQuickShapePath::fillColor() const
{
    Q_D(const QQuickShapePath);
    return d->sfp.fillColor;
}

void QQuickShapePath::setFillColor(const QColor &color)
{
    Q_D(QQuickShapePath);
    if (d->sfp.fillColor != color) {
        d->sfp.fillColor = color;
        d->dirty |= QQuickShapePathPrivate::DirtyFillColor;
        emit fillColorChanged();
        emit shapePathChanged();
    }
}

/*!
    \qmlproperty enumeration QtQuick.Shapes::ShapePath::fillRule

    This property holds the fill rule. The default value is
    \c ShapePath.OddEvenFill. For an explanation on fill rules, see
    QPainterPath::setFillRule().

    \value ShapePath.OddEvenFill
           Odd-even fill rule.

    \value ShapePath.WindingFill
           Non-zero winding fill rule.
 */

QQuickShapePath::FillRule QQuickShapePath::fillRule() const
{
    Q_D(const QQuickShapePath);
    return d->sfp.fillRule;
}

void QQuickShapePath::setFillRule(FillRule fillRule)
{
    Q_D(QQuickShapePath);
    if (d->sfp.fillRule != fillRule) {
        d->sfp.fillRule = fillRule;
        d->dirty |= QQuickShapePathPrivate::DirtyFillRule;
        emit fillRuleChanged();
        emit shapePathChanged();
    }
}

/*!
    \qmlproperty enumeration QtQuick.Shapes::ShapePath::joinStyle

    This property defines how joins between two connected lines are drawn. The
    default value is \c ShapePath.BevelJoin.

    \value ShapePath.MiterJoin
           The outer edges of the lines are extended to meet at an angle, and
           this area is filled.

    \value ShapePath.BevelJoin
           The triangular notch between the two lines is filled.

    \value ShapePath.RoundJoin
           A circular arc between the two lines is filled.
 */

QQuickShapePath::JoinStyle QQuickShapePath::joinStyle() const
{
    Q_D(const QQuickShapePath);
    return d->sfp.joinStyle;
}

void QQuickShapePath::setJoinStyle(JoinStyle style)
{
    Q_D(QQuickShapePath);
    if (d->sfp.joinStyle != style) {
        d->sfp.joinStyle = style;
        d->dirty |= QQuickShapePathPrivate::DirtyStyle;
        emit joinStyleChanged();
        emit shapePathChanged();
    }
}

/*!
    \qmlproperty int QtQuick.Shapes::ShapePath::miterLimit

    When joinStyle is set to \c ShapePath.MiterJoin, this property
    specifies how far the miter join can extend from the join point.

    The default value is 2.
 */

int QQuickShapePath::miterLimit() const
{
    Q_D(const QQuickShapePath);
    return d->sfp.miterLimit;
}

void QQuickShapePath::setMiterLimit(int limit)
{
    Q_D(QQuickShapePath);
    if (d->sfp.miterLimit != limit) {
        d->sfp.miterLimit = limit;
        d->dirty |= QQuickShapePathPrivate::DirtyStyle;
        emit miterLimitChanged();
        emit shapePathChanged();
    }
}

/*!
    \qmlproperty enumeration QtQuick.Shapes::ShapePath::capStyle

    This property defines how the end points of lines are drawn. The
    default value is \c ShapePath.SquareCap.

    \value ShapePath.FlatCap
           A square line end that does not cover the end point of the line.

    \value ShapePath.SquareCap
           A square line end that covers the end point and extends beyond it
           by half the line width.

    \value ShapePath.RoundCap
           A rounded line end.
 */

QQuickShapePath::CapStyle QQuickShapePath::capStyle() const
{
    Q_D(const QQuickShapePath);
    return d->sfp.capStyle;
}

void QQuickShapePath::setCapStyle(CapStyle style)
{
    Q_D(QQuickShapePath);
    if (d->sfp.capStyle != style) {
        d->sfp.capStyle = style;
        d->dirty |= QQuickShapePathPrivate::DirtyStyle;
        emit capStyleChanged();
        emit shapePathChanged();
    }
}

/*!
    \qmlproperty enumeration QtQuick.Shapes::ShapePath::strokeStyle

    This property defines the style of stroking. The default value is
    ShapePath.SolidLine.

    \list
    \li ShapePath.SolidLine - A plain line.
    \li ShapePath.DashLine - Dashes separated by a few pixels.
    \endlist
 */

QQuickShapePath::StrokeStyle QQuickShapePath::strokeStyle() const
{
    Q_D(const QQuickShapePath);
    return d->sfp.strokeStyle;
}

void QQuickShapePath::setStrokeStyle(StrokeStyle style)
{
    Q_D(QQuickShapePath);
    if (d->sfp.strokeStyle != style) {
        d->sfp.strokeStyle = style;
        d->dirty |= QQuickShapePathPrivate::DirtyDash;
        emit strokeStyleChanged();
        emit shapePathChanged();
    }
}

/*!
    \qmlproperty real QtQuick.Shapes::ShapePath::dashOffset

    This property defines the starting point on the dash pattern, measured in
    units used to specify the dash pattern.

    The default value is 0.

    \sa QPen::setDashOffset()
 */

qreal QQuickShapePath::dashOffset() const
{
    Q_D(const QQuickShapePath);
    return d->sfp.dashOffset;
}

void QQuickShapePath::setDashOffset(qreal offset)
{
    Q_D(QQuickShapePath);
    if (d->sfp.dashOffset != offset) {
        d->sfp.dashOffset = offset;
        d->dirty |= QQuickShapePathPrivate::DirtyDash;
        emit dashOffsetChanged();
        emit shapePathChanged();
    }
}

/*!
    \qmlproperty list<real> QtQuick.Shapes::ShapePath::dashPattern

    This property defines the dash pattern when ShapePath.strokeStyle is set
    to ShapePath.DashLine. The pattern must be specified as an even number of
    positive entries where the entries 1, 3, 5... are the dashes and 2, 4,
    6... are the spaces. The pattern is specified in units of the pen's width.

    The default value is (4, 2), meaning a dash of 4 * ShapePath.strokeWidth
    pixels followed by a space of 2 * ShapePath.strokeWidth pixels.

    \sa QPen::setDashPattern()
 */

QVector<qreal> QQuickShapePath::dashPattern() const
{
    Q_D(const QQuickShapePath);
    return d->sfp.dashPattern;
}

void QQuickShapePath::setDashPattern(const QVector<qreal> &array)
{
    Q_D(QQuickShapePath);
    if (d->sfp.dashPattern != array) {
        d->sfp.dashPattern = array;
        d->dirty |= QQuickShapePathPrivate::DirtyDash;
        emit dashPatternChanged();
        emit shapePathChanged();
    }
}

/*!
    \qmlproperty ShapeGradient QtQuick.Shapes::ShapePath::fillGradient

    This property defines the fill gradient. By default no gradient is enabled
    and the value is \c null. In this case the fill uses a solid color based
    on the value of ShapePath.fillColor.

    When set, ShapePath.fillColor is ignored and filling is done using one of
    the ShapeGradient subtypes.

    \note The Gradient type cannot be used here. Rather, prefer using one of
    the advanced subtypes, like LinearGradient.
 */

QQuickShapeGradient *QQuickShapePath::fillGradient() const
{
    Q_D(const QQuickShapePath);
    return d->sfp.fillGradient;
}

void QQuickShapePath::setFillGradient(QQuickShapeGradient *gradient)
{
    Q_D(QQuickShapePath);
    if (d->sfp.fillGradient != gradient) {
        if (d->sfp.fillGradient)
            qmlobject_disconnect(d->sfp.fillGradient, QQuickShapeGradient, SIGNAL(updated()),
                                 this, QQuickShapePath, SLOT(_q_fillGradientChanged()));
        d->sfp.fillGradient = gradient;
        if (d->sfp.fillGradient)
            qmlobject_connect(d->sfp.fillGradient, QQuickShapeGradient, SIGNAL(updated()),
                              this, QQuickShapePath, SLOT(_q_fillGradientChanged()));
        d->dirty |= QQuickShapePathPrivate::DirtyFillGradient;
        emit shapePathChanged();
    }
}

void QQuickShapePathPrivate::_q_fillGradientChanged()
{
    Q_Q(QQuickShapePath);
    dirty |= DirtyFillGradient;
    emit q->shapePathChanged();
}

void QQuickShapePath::resetFillGradient()
{
    setFillGradient(nullptr);
}

/*!
    \qmltype Shape
    \instantiates QQuickShape
    \inqmlmodule QtQuick.Shapes
    \ingroup qtquick-paths
    \ingroup qtquick-views
    \inherits Item
    \brief Renders a path.
    \since 5.10

    Renders a path either by generating geometry via QPainterPath and manual
    triangulation or by using a GPU vendor extension like
    \c{GL_NV_path_rendering}.

    This approach is different from rendering shapes via QQuickPaintedItem or
    the 2D Canvas because the path never gets rasterized in software.
    Therefore Shape is suitable for creating shapes spreading over larger
    areas of the screen, avoiding the performance penalty for texture uploads
    or framebuffer blits. In addition, the declarative API allows manipulating,
    binding to, and even animating the path element properties like starting
    and ending position, the control points, and so on.

    The types for specifying path elements are shared between \l PathView and
    Shape. However, not all Shape implementations support all path
    element types, while some may not make sense for PathView. Shape's
    currently supported subset is: PathMove, PathLine, PathQuad, PathCubic,
    PathArc, and PathSvg.

    See \l Path for a detailed overview of the supported path elements.

    \qml
    Shape {
        width: 200
        height: 150
        anchors.centerIn: parent
        ShapePath {
            strokeWidth: 4
            strokeColor: "red"
            fillGradient: LinearGradient {
                x1: 20; y1: 20
                x2: 180; y2: 130
                GradientStop { position: 0; color: "blue" }
                GradientStop { position: 0.2; color: "green" }
                GradientStop { position: 0.4; color: "red" }
                GradientStop { position: 0.6; color: "yellow" }
                GradientStop { position: 1; color: "cyan" }
            }
            strokeStyle: ShapePath.DashLine
            dashPattern: [ 1, 4 ]
            startX: 20; startY: 20
            PathLine { x: 180; y: 130 }
            PathLine { x: 20; y: 130 }
            PathLine { x: 20; y: 20 }
        }
    }
    \endqml

    \image pathitem-code-example.png

    Like \l Item, Shape also allows any visual or non-visual objects to be
    declared as children. ShapePath objects are handled specially. This is
    useful since it allows adding visual items, like \l Rectangle or \l Image,
    and non-visual objects, like \l Timer directly as children of Shape.

    The following list summarizes the available Shape rendering approaches:

    \list

    \li When running with the default, OpenGL backend of Qt Quick, both the
    generic, triangulation-based and the NVIDIA-specific
    \c{GL_NV_path_rendering} methods are available. By default only the generic
    approach is used. Setting Shape.vendorExtensionsEnabled property to \c true
    leads to using NV_path_rendering on NVIDIA systems, and the generic method
    on others.

    \li The \c software backend is fully supported. The path is rendered via
    QPainter::strokePath() and QPainter::fillPath() in this case.

    \li The Direct 3D 12 backend is not currently supported.

    \li The OpenVG backend is not currently supported.

    \endlist

    When using Shape, it is important to be aware of potential performance
    implications:

    \list

    \li When the application is running with the generic, triangulation-based
    Shape implementation, the geometry generation happens entirely on the
    CPU. This is potentially expensive. Changing the set of path elements,
    changing the properties of these elements, or changing certain properties
    of the Shape itself all lead to retriangulation of the affected paths on
    every change. Therefore, applying animation to such properties can affect
    performance on less powerful systems.

    \li However, the data-driven, declarative nature of the Shape API often
    means better cacheability for the underlying CPU and GPU resources. A
    property change in one ShapePath will only lead to reprocessing the
    affected ShapePath, leaving other parts of the Shape unchanged. Therefore,
    a frequently changing property can still result in a lower overall system
    load than with imperative painting approaches (for example, QPainter).

    \li If animating properties other than stroke and fill colors is a must,
    it is recommended to target systems providing \c{GL_NV_path_rendering}
    where the cost of property changes is smaller.

    \li At the same time, attention must be paid to the number of Shape
    elements in the scene, in particular when using this special accelerated
    approach for \c{GL_NV_path_rendering}. The way such a Shape item is
    represented in the scene graph is different from an ordinary
    geometry-based item, and incurs a certain cost when it comes to OpenGL
    state changes.

    \li As a general rule, scenes should avoid using separate Shape items when
    it is not absolutely necessary. Prefer using one Shape item with multiple
    ShapePath elements over multiple Shape items.

    \endlist

    \sa {Qt Quick Examples - Shapes}, Path, PathMove, PathLine, PathQuad, PathCubic, PathArc, PathSvg
*/

QQuickShapePrivate::QQuickShapePrivate()
      : effectRefCount(0)
{
}

QQuickShapePrivate::~QQuickShapePrivate()
{
    delete renderer;
}

void QQuickShapePrivate::_q_shapePathChanged()
{
    Q_Q(QQuickShape);
    spChanged = true;
    q->polish();
}

void QQuickShapePrivate::setStatus(QQuickShape::Status newStatus)
{
    Q_Q(QQuickShape);
    if (status != newStatus) {
        status = newStatus;
        emit q->statusChanged();
    }
}

struct QQuickShapeResourceInitializer
{
    QQuickShapeResourceInitializer()
    {
#if defined(QT_STATIC)
        initResources();
#endif
    }
};

Q_GLOBAL_STATIC(QQuickShapeResourceInitializer, initQQuickShapeResources)

QQuickShape::QQuickShape(QQuickItem *parent)
  : QQuickItem(*(new QQuickShapePrivate), parent)
{
    initQQuickShapeResources();
    setFlag(ItemHasContents);
}

QQuickShape::~QQuickShape()
{
}

/*!
    \qmlproperty enumeration QtQuick.Shapes::Shape::rendererType

    This property determines which path rendering backend is active.

    \value Shape.UnknownRenderer
           The renderer is unknown.

    \value Shape.GeometryRenderer
           The generic, driver independent solution for OpenGL. Uses the same
           CPU-based triangulation approach as QPainter's OpenGL 2 paint
           engine. This is the default on non-NVIDIA hardware when the default,
           OpenGL Qt Quick scenegraph backend is in use.

    \value Shape.NvprRenderer
           Path items are rendered by performing OpenGL calls using the
           \c{GL_NV_path_rendering} extension. This is the default on NVIDIA
           hardware when the default, OpenGL Qt Quick scenegraph backend is in
           use.

    \value Shape.SoftwareRenderer
           Pure QPainter drawing using the raster paint engine. This is the
           default, and only, option when the Qt Quick scenegraph is running
           with the \c software backend.
*/

QQuickShape::RendererType QQuickShape::rendererType() const
{
    Q_D(const QQuickShape);
    return d->rendererType;
}

/*!
    \qmlproperty bool QtQuick.Shapes::Shape::asynchronous

    When rendererType is \c Shape.GeometryRenderer, the input path is
    triangulated on the CPU during the polishing phase of the Shape. This is
    potentially expensive. To offload this work to separate worker threads,
    set this property to \c true.

    When enabled, making a Shape visible will not wait for the content to
    become available. Instead, the gui/main thread is not blocked and the
    results of the path rendering are shown only when all the asynchronous
    work has been finished.

    The default value is \c false.
 */

bool QQuickShape::asynchronous() const
{
    Q_D(const QQuickShape);
    return d->async;
}

void QQuickShape::setAsynchronous(bool async)
{
    Q_D(QQuickShape);
    if (d->async != async) {
        d->async = async;
        emit asynchronousChanged();
        if (d->componentComplete)
            d->_q_shapePathChanged();
    }
}

/*!
    \qmlproperty bool QtQuick.Shapes::Shape::vendorExtensionsEnabled

    This property controls the usage of non-standard OpenGL extensions like
    \c GL_NV_path_rendering.

    The default value is \c false.

    As of Qt 5.12 Shape.NvprRenderer is disabled by default and a uniform
    behavior, based on triangulating the path and generating QSGGeometryNode
    instances, is used regardless of the graphics card and drivers. To enable
    using vendor-specific path rendering approaches set the value to \c true.
    Depending on the platform and content, this can lead to improved
    performance. Setting the value to \c true is safe in any case since
    rendering falls back to the default method when the vendor-specific
    approach, such as \c GL_NV_path_rendering, is not supported at run time.
 */

bool QQuickShape::vendorExtensionsEnabled() const
{
    Q_D(const QQuickShape);
    return d->enableVendorExts;
}

void QQuickShape::setVendorExtensionsEnabled(bool enable)
{
    Q_D(QQuickShape);
    if (d->enableVendorExts != enable) {
        d->enableVendorExts = enable;
        emit vendorExtensionsEnabledChanged();
    }
}

/*!
    \qmlproperty enumeration QtQuick.Shapes::Shape::status

    This property determines the status of the Shape and is relevant when
    Shape.asynchronous is set to \c true.

    \value Shape.Null
           Not yet initialized.

    \value Shape.Ready
           The Shape has finished processing.

    \value Shape.Processing
           The path is being processed.
 */

QQuickShape::Status QQuickShape::status() const
{
    Q_D(const QQuickShape);
    return d->status;
}

/*!
    \qmlproperty enumeration QtQuick.Shapes::Shape::containsMode
    \since QtQuick.Shapes 1.11

    This property determines the definition of \l {QQuickItem::contains()}{contains()}
    for the Shape. It is useful in case you add \l {Qt Quick Input Handlers} and you want to
    react only when the mouse or touchpoint is fully inside the Shape.

    \value Shape.BoundingRectContains
        The default implementation of \l QQuickItem::contains() checks only
        whether the given point is inside the rectangular bounding box. This is
        the most efficient implementation, which is why it's the default.

    \value Shape.FillContains
        Check whether the interior (the part that would be filled if you are
        rendering it with fill) of any \l ShapePath that makes up this Shape
        contains the given point. The more complex and numerous ShapePaths you
        add, the less efficient this is to check, which can potentially slow
        down event delivery in your application. So it should be used with care.

    One way to speed up the \c FillContains check is to generate an approximate
    outline with as few points as possible, place that in a transparent Shape
    on top, and add your Pointer Handlers to that, so that the containment
    check is cheaper during event delivery.
*/
QQuickShape::ContainsMode QQuickShape::containsMode() const
{
    Q_D(const QQuickShape);
    return d->containsMode;
}

void QQuickShape::setContainsMode(QQuickShape::ContainsMode containsMode)
{
    Q_D(QQuickShape);
    if (d->containsMode == containsMode)
        return;

    d->containsMode = containsMode;
    emit containsModeChanged();
}

bool QQuickShape::contains(const QPointF &point) const
{
    Q_D(const QQuickShape);
    switch (d->containsMode) {
    case BoundingRectContains:
        return QQuickItem::contains(point);
    case FillContains:
        for (QQuickShapePath *path : d->sp) {
            if (path->path().contains(point))
                return true;
        }
    }
    return false;
}

static void vpe_append(QQmlListProperty<QObject> *property, QObject *obj)
{
    QQuickShape *item = static_cast<QQuickShape *>(property->object);
    QQuickShapePrivate *d = QQuickShapePrivate::get(item);
    QQuickShapePath *path = qobject_cast<QQuickShapePath *>(obj);
    if (path)
        d->sp.append(path);

    QQuickItemPrivate::data_append(property, obj);

    if (path && d->componentComplete) {
        QObject::connect(path, SIGNAL(shapePathChanged()), item, SLOT(_q_shapePathChanged()));
        d->_q_shapePathChanged();
    }
}

static void vpe_clear(QQmlListProperty<QObject> *property)
{
    QQuickShape *item = static_cast<QQuickShape *>(property->object);
    QQuickShapePrivate *d = QQuickShapePrivate::get(item);

    for (QQuickShapePath *p : d->sp)
        QObject::disconnect(p, SIGNAL(shapePathChanged()), item, SLOT(_q_shapePathChanged()));

    d->sp.clear();

    QQuickItemPrivate::data_clear(property);

    if (d->componentComplete)
        d->_q_shapePathChanged();
}

/*!
    \qmlproperty list<Object> QtQuick.Shapes::Shape::data

    This property holds the ShapePath objects that define the contents of the
    Shape. It can also contain any other type of objects, since Shape, like
    Item, allows adding any visual or non-visual objects as children.

    \default
 */

QQmlListProperty<QObject> QQuickShape::data()
{
    return QQmlListProperty<QObject>(this,
                                     nullptr,
                                     vpe_append,
                                     QQuickItemPrivate::data_count,
                                     QQuickItemPrivate::data_at,
                                     vpe_clear);
}

void QQuickShape::classBegin()
{
    QQuickItem::classBegin();
}

void QQuickShape::componentComplete()
{
    Q_D(QQuickShape);

    QQuickItem::componentComplete();

    for (QQuickShapePath *p : d->sp)
        connect(p, SIGNAL(shapePathChanged()), this, SLOT(_q_shapePathChanged()));

    d->_q_shapePathChanged();
}

void QQuickShape::updatePolish()
{
    Q_D(QQuickShape);

    const int currentEffectRefCount = d->extra.isAllocated() ? d->extra->recursiveEffectRefCount : 0;
    if (!d->spChanged && currentEffectRefCount <= d->effectRefCount)
        return;

    d->spChanged = false;
    d->effectRefCount = currentEffectRefCount;

    if (!d->renderer) {
        d->createRenderer();
        if (!d->renderer)
            return;
        emit rendererChanged();
    }

    // endSync() is where expensive calculations may happen (or get kicked off
    // on worker threads), depending on the backend. Therefore do this only
    // when the item is visible.
    if (isVisible() || d->effectRefCount > 0)
        d->sync();

    update();
}

void QQuickShape::itemChange(ItemChange change, const ItemChangeData &data)
{
    Q_D(QQuickShape);

    // sync may have been deferred; do it now if the item became visible
    if (change == ItemVisibleHasChanged && data.boolValue)
        d->_q_shapePathChanged();

    QQuickItem::itemChange(change, data);
}

QSGNode *QQuickShape::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
    // Called on the render thread, with the gui thread blocked. We can now
    // safely access gui thread data.

    Q_D(QQuickShape);
    if (d->renderer) {
        if (!node)
            node = d->createNode();
        d->renderer->updateNode();
    }
    return node;
}

// the renderer object lives on the gui thread
void QQuickShapePrivate::createRenderer()
{
    Q_Q(QQuickShape);
    QSGRendererInterface *ri = q->window()->rendererInterface();
    if (!ri)
        return;

    switch (ri->graphicsApi()) {
#if QT_CONFIG(opengl)
    case QSGRendererInterface::OpenGL:
        if (enableVendorExts && QQuickShapeNvprRenderNode::isSupported()) {
            rendererType = QQuickShape::NvprRenderer;
            renderer = new QQuickShapeNvprRenderer;
        } else {
            rendererType = QQuickShape::GeometryRenderer;
            renderer = new QQuickShapeGenericRenderer(q);
        }
        break;
#endif
    case QSGRendererInterface::Software:
        rendererType = QQuickShape::SoftwareRenderer;
        renderer = new QQuickShapeSoftwareRenderer;
        break;
    default:
        qWarning("No path backend for this graphics API yet");
        break;
    }
}

// the node lives on the render thread
QSGNode *QQuickShapePrivate::createNode()
{
    Q_Q(QQuickShape);
    QSGNode *node = nullptr;
    if (!q->window())
        return node;
    QSGRendererInterface *ri = q->window()->rendererInterface();
    if (!ri)
        return node;

    switch (ri->graphicsApi()) {
#if QT_CONFIG(opengl)
    case QSGRendererInterface::OpenGL:
        if (enableVendorExts && QQuickShapeNvprRenderNode::isSupported()) {
            node = new QQuickShapeNvprRenderNode;
            static_cast<QQuickShapeNvprRenderer *>(renderer)->setNode(
                static_cast<QQuickShapeNvprRenderNode *>(node));
        } else {
            node = new QQuickShapeGenericNode;
            static_cast<QQuickShapeGenericRenderer *>(renderer)->setRootNode(
                static_cast<QQuickShapeGenericNode *>(node));
        }
        break;
#endif
    case QSGRendererInterface::Software:
        node = new QQuickShapeSoftwareRenderNode(q);
        static_cast<QQuickShapeSoftwareRenderer *>(renderer)->setNode(
                    static_cast<QQuickShapeSoftwareRenderNode *>(node));
        break;
    default:
        qWarning("No path backend for this graphics API yet");
        break;
    }

    return node;
}

void QQuickShapePrivate::asyncShapeReady(void *data)
{
    QQuickShapePrivate *self = static_cast<QQuickShapePrivate *>(data);
    self->setStatus(QQuickShape::Ready);
    if (self->syncTimingActive)
        qDebug("[Shape %p] [%d] [dirty=0x%x] async update took %lld ms",
               self->q_func(), self->syncTimeCounter, self->syncTimingTotalDirty, self->syncTimer.elapsed());
}

void QQuickShapePrivate::sync()
{
    syncTimingTotalDirty = 0;
    syncTimingActive = QQSHAPE_LOG_TIME_DIRTY_SYNC().isDebugEnabled();
    if (syncTimingActive)
        syncTimer.start();

    const bool useAsync = async && renderer->flags().testFlag(QQuickAbstractPathRenderer::SupportsAsync);
    if (useAsync) {
        setStatus(QQuickShape::Processing);
        renderer->setAsyncCallback(asyncShapeReady, this);
    }

    const int count = sp.count();
    renderer->beginSync(count);

    for (int i = 0; i < count; ++i) {
        QQuickShapePath *p = sp[i];
        int &dirty(QQuickShapePathPrivate::get(p)->dirty);
        syncTimingTotalDirty |= dirty;

        if (dirty & QQuickShapePathPrivate::DirtyPath)
            renderer->setPath(i, p);
        if (dirty & QQuickShapePathPrivate::DirtyStrokeColor)
            renderer->setStrokeColor(i, p->strokeColor());
        if (dirty & QQuickShapePathPrivate::DirtyStrokeWidth)
            renderer->setStrokeWidth(i, p->strokeWidth());
        if (dirty & QQuickShapePathPrivate::DirtyFillColor)
            renderer->setFillColor(i, p->fillColor());
        if (dirty & QQuickShapePathPrivate::DirtyFillRule)
            renderer->setFillRule(i, p->fillRule());
        if (dirty & QQuickShapePathPrivate::DirtyStyle) {
            renderer->setJoinStyle(i, p->joinStyle(), p->miterLimit());
            renderer->setCapStyle(i, p->capStyle());
        }
        if (dirty & QQuickShapePathPrivate::DirtyDash)
            renderer->setStrokeStyle(i, p->strokeStyle(), p->dashOffset(), p->dashPattern());
        if (dirty & QQuickShapePathPrivate::DirtyFillGradient)
            renderer->setFillGradient(i, p->fillGradient());

        dirty = 0;
    }

    if (syncTimingTotalDirty)
        ++syncTimeCounter;
    else
        syncTimingActive = false;

    renderer->endSync(useAsync);

    if (!useAsync) {
        setStatus(QQuickShape::Ready);
        if (syncTimingActive)
            qDebug("[Shape %p] [%d] [dirty=0x%x] update took %lld ms",
                   q_func(), syncTimeCounter, syncTimingTotalDirty, syncTimer.elapsed());
    }
}

// ***** gradient support *****

/*!
    \qmltype ShapeGradient
    \instantiates QQuickShapeGradient
    \inqmlmodule QtQuick.Shapes
    \ingroup qtquick-paths
    \ingroup qtquick-views
    \inherits Gradient
    \brief Base type of Shape fill gradients.
    \since 5.10

    This is an abstract base class for gradients like LinearGradient and
    cannot be created directly. It extends \l Gradient with properties like the
    spread mode.
 */

QQuickShapeGradient::QQuickShapeGradient(QObject *parent)
    : QQuickGradient(parent),
      m_spread(PadSpread)
{
}

/*!
    \qmlproperty enumeration QtQuick.Shapes::ShapeGradient::spread

    Specifies how the area outside the gradient area should be filled. The
    default value is \c ShapeGradient.PadSpread.

    \value ShapeGradient.PadSpread
           The area is filled with the closest stop color.

    \value ShapeGradient.RepeatSpread
           The gradient is repeated outside the gradient area.

    \value ShapeGradient.ReflectSpread
           The gradient is reflected outside the gradient area.
 */

QQuickShapeGradient::SpreadMode QQuickShapeGradient::spread() const
{
    return m_spread;
}

void QQuickShapeGradient::setSpread(SpreadMode mode)
{
    if (m_spread != mode) {
        m_spread = mode;
        emit spreadChanged();
        emit updated();
    }
}

/*!
    \qmltype LinearGradient
    \instantiates QQuickShapeLinearGradient
    \inqmlmodule QtQuick.Shapes
    \ingroup qtquick-paths
    \ingroup qtquick-views
    \inherits ShapeGradient
    \brief Linear gradient.
    \since 5.10

    Linear gradients interpolate colors between start and end points in Shape
    items. Outside these points the gradient is either padded, reflected or
    repeated depending on the spread type.

    \note LinearGradient is only supported in combination with Shape items. It
    is not compatible with \l Rectangle, as that only supports \l Gradient.

    \sa QLinearGradient
 */

QQuickShapeLinearGradient::QQuickShapeLinearGradient(QObject *parent)
    : QQuickShapeGradient(parent)
{
}

/*!
    \qmlproperty real QtQuick.Shapes::LinearGradient::x1
    \qmlproperty real QtQuick.Shapes::LinearGradient::y1
    \qmlproperty real QtQuick.Shapes::LinearGradient::x2
    \qmlproperty real QtQuick.Shapes::LinearGradient::y2

    These properties define the start and end points between which color
    interpolation occurs. By default both points are set to (0, 0).
 */

qreal QQuickShapeLinearGradient::x1() const
{
    return m_start.x();
}

void QQuickShapeLinearGradient::setX1(qreal v)
{
    if (m_start.x() != v) {
        m_start.setX(v);
        emit x1Changed();
        emit updated();
    }
}

qreal QQuickShapeLinearGradient::y1() const
{
    return m_start.y();
}

void QQuickShapeLinearGradient::setY1(qreal v)
{
    if (m_start.y() != v) {
        m_start.setY(v);
        emit y1Changed();
        emit updated();
    }
}

qreal QQuickShapeLinearGradient::x2() const
{
    return m_end.x();
}

void QQuickShapeLinearGradient::setX2(qreal v)
{
    if (m_end.x() != v) {
        m_end.setX(v);
        emit x2Changed();
        emit updated();
    }
}

qreal QQuickShapeLinearGradient::y2() const
{
    return m_end.y();
}

void QQuickShapeLinearGradient::setY2(qreal v)
{
    if (m_end.y() != v) {
        m_end.setY(v);
        emit y2Changed();
        emit updated();
    }
}

/*!
    \qmltype RadialGradient
    \instantiates QQuickShapeRadialGradient
    \inqmlmodule QtQuick.Shapes
    \ingroup qtquick-paths
    \ingroup qtquick-views
    \inherits ShapeGradient
    \brief Radial gradient.
    \since 5.10

    Radial gradients interpolate colors between a focal circle and a center
    circle in Shape items. Points outside the cone defined by the two circles
    will be transparent.

    Outside the end points the gradient is either padded, reflected or repeated
    depending on the spread type.

    Below is an example of a simple radial gradient. Here the colors are
    interpolated between the specified point and the end points on a circle
    specified by the radius:

    \code
        fillGradient: RadialGradient {
            centerX: 50; centerY: 50
            centerRadius: 100
            focalX: centerX; focalY: centerY
            GradientStop { position: 0; color: "blue" }
            GradientStop { position: 0.2; color: "green" }
            GradientStop { position: 0.4; color: "red" }
            GradientStop { position: 0.6; color: "yellow" }
            GradientStop { position: 1; color: "cyan" }
        }
    \endcode

    \image shape-radial-gradient.png

    Extended radial gradients, where a separate focal circle is specified, are
    also supported.

    \note RadialGradient is only supported in combination with Shape items. It
    is not compatible with \l Rectangle, as that only supports \l Gradient.

    \sa QRadialGradient
 */

QQuickShapeRadialGradient::QQuickShapeRadialGradient(QObject *parent)
    : QQuickShapeGradient(parent)
{
}

/*!
    \qmlproperty real QtQuick.Shapes::RadialGradient::centerX
    \qmlproperty real QtQuick.Shapes::RadialGradient::centerY
    \qmlproperty real QtQuick.Shapes::RadialGradient::focalX
    \qmlproperty real QtQuick.Shapes::RadialGradient::focalY

    These properties define the center and focal points. To specify a simple
    radial gradient, set focalX and focalY to the value of centerX and
    centerY, respectively.
 */

qreal QQuickShapeRadialGradient::centerX() const
{
    return m_centerPoint.x();
}

void QQuickShapeRadialGradient::setCenterX(qreal v)
{
    if (m_centerPoint.x() != v) {
        m_centerPoint.setX(v);
        emit centerXChanged();
        emit updated();
    }
}

qreal QQuickShapeRadialGradient::centerY() const
{
    return m_centerPoint.y();
}

void QQuickShapeRadialGradient::setCenterY(qreal v)
{
    if (m_centerPoint.y() != v) {
        m_centerPoint.setY(v);
        emit centerYChanged();
        emit updated();
    }
}

/*!
    \qmlproperty real QtQuick.Shapes::RadialGradient::centerRadius
    \qmlproperty real QtQuick.Shapes::RadialGradient::focalRadius

    These properties define the center and focal radius. For simple radial
    gradients, focalRadius should be set to \c 0 (the default value).
 */

qreal QQuickShapeRadialGradient::centerRadius() const
{
    return m_centerRadius;
}

void QQuickShapeRadialGradient::setCenterRadius(qreal v)
{
    if (m_centerRadius != v) {
        m_centerRadius = v;
        emit centerRadiusChanged();
        emit updated();
    }
}

qreal QQuickShapeRadialGradient::focalX() const
{
    return m_focalPoint.x();
}

void QQuickShapeRadialGradient::setFocalX(qreal v)
{
    if (m_focalPoint.x() != v) {
        m_focalPoint.setX(v);
        emit focalXChanged();
        emit updated();
    }
}

qreal QQuickShapeRadialGradient::focalY() const
{
    return m_focalPoint.y();
}

void QQuickShapeRadialGradient::setFocalY(qreal v)
{
    if (m_focalPoint.y() != v) {
        m_focalPoint.setY(v);
        emit focalYChanged();
        emit updated();
    }
}

qreal QQuickShapeRadialGradient::focalRadius() const
{
    return m_focalRadius;
}

void QQuickShapeRadialGradient::setFocalRadius(qreal v)
{
    if (m_focalRadius != v) {
        m_focalRadius = v;
        emit focalRadiusChanged();
        emit updated();
    }
}

/*!
    \qmltype ConicalGradient
    \instantiates QQuickShapeConicalGradient
    \inqmlmodule QtQuick.Shapes
    \ingroup qtquick-paths
    \ingroup qtquick-views
    \inherits ShapeGradient
    \brief Conical gradient.
    \since 5.10

    Conical gradients interpolate colors counter-clockwise around a center
    point in Shape items.

    \note The \l{ShapeGradient::spread}{spread mode} setting has no effect for
    conical gradients.

    \note ConicalGradient is only supported in combination with Shape items. It
    is not compatible with \l Rectangle, as that only supports \l Gradient.

    \sa QConicalGradient
 */

QQuickShapeConicalGradient::QQuickShapeConicalGradient(QObject *parent)
    : QQuickShapeGradient(parent)
{
}

/*!
    \qmlproperty real QtQuick.Shapes::ConicalGradient::centerX
    \qmlproperty real QtQuick.Shapes::ConicalGradient::centerY

    These properties define the center point of the conical gradient.
 */

qreal QQuickShapeConicalGradient::centerX() const
{
    return m_centerPoint.x();
}

void QQuickShapeConicalGradient::setCenterX(qreal v)
{
    if (m_centerPoint.x() != v) {
        m_centerPoint.setX(v);
        emit centerXChanged();
        emit updated();
    }
}

qreal QQuickShapeConicalGradient::centerY() const
{
    return m_centerPoint.y();
}

void QQuickShapeConicalGradient::setCenterY(qreal v)
{
    if (m_centerPoint.y() != v) {
        m_centerPoint.setY(v);
        emit centerYChanged();
        emit updated();
    }
}

/*!
    \qmlproperty real QtQuick.Shapes::ConicalGradient::angle

    This property defines the start angle for the conical gradient. The value
    is in degrees (0-360).
 */

qreal QQuickShapeConicalGradient::angle() const
{
    return m_angle;
}

void QQuickShapeConicalGradient::setAngle(qreal v)
{
    if (m_angle != v) {
        m_angle = v;
        emit angleChanged();
        emit updated();
    }
}

#if QT_CONFIG(opengl)

// contexts sharing with each other get the same cache instance
class QQuickShapeGradientCacheWrapper
{
public:
    QQuickShapeGradientCache *get(QOpenGLContext *context)
    {
        return m_resource.value<QQuickShapeGradientCache>(context);
    }

private:
    QOpenGLMultiGroupSharedResource m_resource;
};

QQuickShapeGradientCache *QQuickShapeGradientCache::currentCache()
{
    static QQuickShapeGradientCacheWrapper qt_path_gradient_caches;
    return qt_path_gradient_caches.get(QOpenGLContext::currentContext());
}

// let QOpenGLContext manage the lifetime of the cached textures
QQuickShapeGradientCache::~QQuickShapeGradientCache()
{
    m_cache.clear();
}

void QQuickShapeGradientCache::invalidateResource()
{
    m_cache.clear();
}

void QQuickShapeGradientCache::freeResource(QOpenGLContext *)
{
    qDeleteAll(m_cache);
    m_cache.clear();
}

static void generateGradientColorTable(const QQuickShapeGradientCache::Key &gradient,
                                       uint *colorTable, int size, float opacity)
{
    int pos = 0;
    const QGradientStops &s = gradient.stops;
    const bool colorInterpolation = true;

    uint alpha = qRound(opacity * 256);
    uint current_color = ARGB_COMBINE_ALPHA(s[0].second.rgba(), alpha);
    qreal incr = 1.0 / qreal(size);
    qreal fpos = 1.5 * incr;
    colorTable[pos++] = ARGB2RGBA(qPremultiply(current_color));

    while (fpos <= s.first().first) {
        colorTable[pos] = colorTable[pos - 1];
        pos++;
        fpos += incr;
    }

    if (colorInterpolation)
        current_color = qPremultiply(current_color);

    const int sLast = s.size() - 1;
    for (int i = 0; i < sLast; ++i) {
        qreal delta = 1/(s[i+1].first - s[i].first);
        uint next_color = ARGB_COMBINE_ALPHA(s[i + 1].second.rgba(), alpha);
        if (colorInterpolation)
            next_color = qPremultiply(next_color);

        while (fpos < s[i+1].first && pos < size) {
            int dist = int(256 * ((fpos - s[i].first) * delta));
            int idist = 256 - dist;
            if (colorInterpolation)
                colorTable[pos] = ARGB2RGBA(INTERPOLATE_PIXEL_256(current_color, idist, next_color, dist));
            else
                colorTable[pos] = ARGB2RGBA(qPremultiply(INTERPOLATE_PIXEL_256(current_color, idist, next_color, dist)));
            ++pos;
            fpos += incr;
        }
        current_color = next_color;
    }

    Q_ASSERT(s.size() > 0);

    uint last_color = ARGB2RGBA(qPremultiply(ARGB_COMBINE_ALPHA(s[sLast].second.rgba(), alpha)));
    for ( ; pos < size; ++pos)
        colorTable[pos] = last_color;

    colorTable[size-1] = last_color;
}

QSGTexture *QQuickShapeGradientCache::get(const Key &grad)
{
    QSGPlainTexture *tx = m_cache[grad];
    if (!tx) {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        GLuint id;
        f->glGenTextures(1, &id);
        f->glBindTexture(GL_TEXTURE_2D, id);
        static const uint W = 1024; // texture size is 1024x1
        uint buf[W];
        generateGradientColorTable(grad, buf, W, 1.0f);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
        tx = new QSGPlainTexture;
        tx->setTextureId(id);
        switch (grad.spread) {
        case QQuickShapeGradient::PadSpread:
            tx->setHorizontalWrapMode(QSGTexture::ClampToEdge);
            tx->setVerticalWrapMode(QSGTexture::ClampToEdge);
            break;
        case QQuickShapeGradient::RepeatSpread:
            tx->setHorizontalWrapMode(QSGTexture::Repeat);
            tx->setVerticalWrapMode(QSGTexture::Repeat);
            break;
        case QQuickShapeGradient::ReflectSpread:
            tx->setHorizontalWrapMode(QSGTexture::MirroredRepeat);
            tx->setVerticalWrapMode(QSGTexture::MirroredRepeat);
            break;
        default:
            qWarning("Unknown gradient spread mode %d", grad.spread);
            break;
        }
        tx->setFiltering(QSGTexture::Linear);
        m_cache[grad] = tx;
    }
    return tx;
}

#endif // QT_CONFIG(opengl)

QT_END_NAMESPACE

#include "moc_qquickshape_p.cpp"
