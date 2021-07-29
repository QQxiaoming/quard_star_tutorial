/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qmapcircleobject_p.h"
#include "qmapcircleobject_p_p.h"
#include <QExplicitlySharedDataPointer>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MapCircleObject
    \instantiates QMapCircleObject
    \inqmlmodule Qt.labs.location
    \ingroup qml-QtLocation5-maps
    \inherits QGeoMapObject

    \brief The MapCircleObject displays a circle on a Map.

    The MapCircleObject displays a circle on a Map.
    The MapIconObject type only makes sense when contained in a Map or in a \l MapObjectView.
*/

QMapCircleObjectPrivate::QMapCircleObjectPrivate(QGeoMapObject *q) : QGeoMapObjectPrivate(q)
{

}

QMapCircleObjectPrivate::~QMapCircleObjectPrivate()
{

}

QGeoMapObject::Type QMapCircleObjectPrivate::type() const
{
    return QGeoMapObject::CircleType;
}



//
// QMapCircleObjectPrivate default implementation
//

QMapCircleObjectPrivateDefault::QMapCircleObjectPrivateDefault(QGeoMapObject *q) : QMapCircleObjectPrivate(q)
{

}

QMapCircleObjectPrivateDefault::QMapCircleObjectPrivateDefault(const QMapCircleObjectPrivate &other) : QMapCircleObjectPrivate(other.q)
{
    m_center = other.center();
    m_radius = other.radius();
    m_fillColor = other.color();
    m_borderColor = other.borderColor();
    m_borderWidth = other.borderWidth();
}

QMapCircleObjectPrivateDefault::~QMapCircleObjectPrivateDefault()
{

}

QGeoCoordinate QMapCircleObjectPrivateDefault::center() const
{
    return m_center;
}

void QMapCircleObjectPrivateDefault::setCenter(const QGeoCoordinate &center)
{
    m_center = center;
}

qreal QMapCircleObjectPrivateDefault::radius() const
{
    return m_radius;
}

void QMapCircleObjectPrivateDefault::setRadius(qreal radius)
{
    m_radius = radius;
}

QColor QMapCircleObjectPrivateDefault::color() const
{
    return m_fillColor;
}

void QMapCircleObjectPrivateDefault::setColor(const QColor &color)
{
    m_fillColor = color;
}

QColor QMapCircleObjectPrivateDefault::borderColor() const
{
    return m_borderColor;
}

void QMapCircleObjectPrivateDefault::setBorderColor(const QColor &color)
{
    m_borderColor = color;
}

qreal QMapCircleObjectPrivateDefault::borderWidth() const
{
    return m_borderWidth;
}

void QMapCircleObjectPrivateDefault::setBorderWidth(qreal width)
{
    m_borderWidth = width;
}

bool QMapCircleObjectPrivate::equals(const QGeoMapObjectPrivate &other) const
{
    if (other.type() != type()) // This check might be unnecessary, depending on how equals gets used
        return false;

    const QMapCircleObjectPrivate &o = static_cast<const QMapCircleObjectPrivate &>(other);
    return (QGeoMapObjectPrivate::equals(o)
            && center() == o.center()
            && radius() == o.radius()
            && color() == o.color()
            && borderColor() == o.borderColor()
            && borderWidth() == o.borderWidth());
}

QGeoMapObjectPrivate *QMapCircleObjectPrivateDefault::clone()
{
    return new QMapCircleObjectPrivateDefault(static_cast<QMapCircleObjectPrivate &>(*this));
}



QMapCircleObject::QMapCircleObject(QObject *parent)
    : QGeoMapObject(QExplicitlySharedDataPointer<QGeoMapObjectPrivate>(new QMapCircleObjectPrivateDefault(this)), parent)

{
    QMapCircleObjectPrivate *d = static_cast<QMapCircleObjectPrivate*>(d_ptr.data());
    d->setBorderColor(QColor(Qt::black)); // These are QDeclarativeMapLineProperties defaults
    d->setBorderWidth(1.0);
}

QMapCircleObject::~QMapCircleObject()
{

}

/*!
    \qmlproperty coordinate Qt.labs.location::MapCircleObject::center

    This property holds the central point about which the circle is defined.

    \sa radius
*/
QGeoCoordinate QMapCircleObject::center() const
{
    return static_cast<const QMapCircleObjectPrivate*>(d_ptr.data())->center();
}

/*!
    \qmlproperty real Qt.labs.location::MapCircleObject::radius

    This property holds the radius of the circle, in meters on the ground.

    \sa center
*/
qreal QMapCircleObject::radius() const
{
    return static_cast<const QMapCircleObjectPrivate*>(d_ptr.data())->radius();
}

/*!
    \qmlproperty color Qt.labs.location::MapCircleObject::color

    This property holds the fill color of the circle when drawn. For no fill,
    use a transparent color.
*/
QColor QMapCircleObject::color() const
{
    return static_cast<const QMapCircleObjectPrivate*>(d_ptr.data())->color();
}

/*!
    \qmlpropertygroup Qt.labs.location::MapCircleObject::border
    \qmlproperty int MapCircleObject::border.width
    \qmlproperty color MapCircleObject::border.color

    This property is part of the border group property.
    The border property holds the width and color used to draw the border of the circle.
    The width is in pixels and is independent of the zoom level of the map.

    The default values correspond to a black border with a width of 1 pixel.
    For no line, use a width of 0 or a transparent color.
*/
QDeclarativeMapLineProperties *QMapCircleObject::border()
{
    if (!m_border) {
        m_border = new QDeclarativeMapLineProperties;
        connect(m_border, &QDeclarativeMapLineProperties::colorChanged, this, [this](const QColor &color){
            static_cast<QMapCircleObjectPrivate*>(d_ptr.data())->setBorderColor(color);
        });
        connect(m_border, &QDeclarativeMapLineProperties::widthChanged, this, [this](qreal width){
            static_cast<QMapCircleObjectPrivate*>(d_ptr.data())->setBorderWidth(width);
        });
    }
    return m_border;
}

void QMapCircleObject::setCenter(const QGeoCoordinate &center)
{
    auto ptr = static_cast<QMapCircleObjectPrivate*>(d_ptr.data());
    if (ptr->center() == center)
        return;

    ptr->setCenter(center);
    emit centerChanged();
}

void QMapCircleObject::setRadius(qreal radius)
{
    auto d = static_cast<QMapCircleObjectPrivate*>(d_ptr.data());
    if (d->radius() == radius)
        return;

    d->setRadius(radius);
    emit radiusChanged();
}

void QMapCircleObject::setColor(const QColor &color)
{
    auto d = static_cast<QMapCircleObjectPrivate*>(d_ptr.data());
    if (d->color() == color)
        return;

    d->setColor(color);
    emit colorChanged();
}

void QMapCircleObject::setMap(QGeoMap *map)
{
    QMapCircleObjectPrivate *d = static_cast<QMapCircleObjectPrivate *>(d_ptr.data());
    if (d->m_map == map)
        return;

    QGeoMapObject::setMap(map); // This is where the specialized pimpl gets created and injected

    if (!map) {
        // Map was set, now it has ben re-set to NULL, but not inside d_ptr.
        // so m_map inside d_ptr can still be used to remove itself, inside the destructor.
        d_ptr = new QMapCircleObjectPrivateDefault(*d);
        // Old pimpl deleted implicitly by QExplicitlySharedDataPointer
    }
}

QT_END_NAMESPACE
