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

#include "qmappolygonobject_p.h"
#include "qmappolygonobject_p_p.h"
#include <QtLocation/private/locationvaluetypehelper_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MapPolygonObject
    \instantiates QMapPolygonObject
    \inqmlmodule Qt.labs.location
    \ingroup qml-QtLocation5-maps
    \inherits QGeoMapObject

    \brief The MapPolygonObject displays a polygon on a Map.

    The MapPolygonObject displays a polygon on a Map.
    The MapPolygonObject type only makes sense when contained in a Map or in a \l MapObjectView.
*/

QMapPolygonObjectPrivate::QMapPolygonObjectPrivate(QGeoMapObject *q) : QGeoMapObjectPrivate(q)
{

}

QMapPolygonObjectPrivate::~QMapPolygonObjectPrivate()
{

}

QMapPolygonObjectPrivateDefault::QMapPolygonObjectPrivateDefault(QGeoMapObject *q) : QMapPolygonObjectPrivate(q)
{

}

QMapPolygonObjectPrivateDefault::QMapPolygonObjectPrivateDefault(const QMapPolygonObjectPrivate &other) : QMapPolygonObjectPrivate(other.q)
{
    m_path = other.path();
    m_borderColor = other.borderColor();
    m_fillColor = other.fillColor();
    m_borderWidth = other.borderWidth();
}

QMapPolygonObjectPrivateDefault::~QMapPolygonObjectPrivateDefault()
{

}

QGeoMapObject::Type QMapPolygonObjectPrivate::type() const
{
    return QGeoMapObject::PolygonType;
}

QList<QGeoCoordinate> QMapPolygonObjectPrivateDefault::path() const
{
    return m_path;
}

void QMapPolygonObjectPrivateDefault::setPath(const QList<QGeoCoordinate> &path)
{
    m_path = path;
}

QColor QMapPolygonObjectPrivateDefault::fillColor() const
{
    return m_fillColor;
}

void QMapPolygonObjectPrivateDefault::setFillColor(const QColor &color)
{
    m_fillColor = color;
}

QColor QMapPolygonObjectPrivateDefault::borderColor() const
{
    return m_borderColor;
}

void QMapPolygonObjectPrivateDefault::setBorderColor(const QColor &color)
{
    m_borderColor = color;
}

qreal QMapPolygonObjectPrivateDefault::borderWidth() const
{
    return m_borderWidth;
}

void QMapPolygonObjectPrivateDefault::setBorderWidth(qreal width)
{
    m_borderWidth = width;
}

QGeoMapObjectPrivate *QMapPolygonObjectPrivateDefault::clone()
{
    return new QMapPolygonObjectPrivateDefault(static_cast<QMapPolygonObjectPrivate &>(*this));
}

bool QMapPolygonObjectPrivate::equals(const QGeoMapObjectPrivate &other) const
{
    if (other.type() != type()) // This check might be unnecessary, depending on how equals gets used
        return false;

    const QMapPolygonObjectPrivate &o = static_cast<const QMapPolygonObjectPrivate &>(other);
    return (QGeoMapObjectPrivate::equals(o)
            && path() == o.path()
            && borderColor() == o.borderColor()
            && fillColor() == o.fillColor()
            && borderWidth() == o.borderWidth());
}




QMapPolygonObject::QMapPolygonObject(QObject *parent)
 : QGeoMapObject(QExplicitlySharedDataPointer<QGeoMapObjectPrivate>(new QMapPolygonObjectPrivateDefault(this)), parent)
{
    QMapPolygonObjectPrivate *d = static_cast<QMapPolygonObjectPrivate*>(d_ptr.data());
    d->setBorderColor(QColor(Qt::black)); // These are QDeclarativeMapLineProperties defaults
    d->setBorderWidth(1.0);
}

QMapPolygonObject::~QMapPolygonObject()
{}

/*!
    \qmlproperty VariantList Qt.labs.location::MapPolygonObject::path

    This property holds the ordered list of coordinates which
    define the polygon border.
*/
QVariantList QMapPolygonObject::path() const
{
    QVariantList p;
    for (const QGeoCoordinate &c: static_cast<const QMapPolygonObjectPrivate *>(d_ptr.data())->path())
        p << QVariant::fromValue(c);
    return p;
}

void QMapPolygonObject::setPath(const QVariantList &path)
{
    QList<QGeoCoordinate> p;
    bool ok = false;
    for (const auto &c: path) {
        const QGeoCoordinate coord = parseCoordinate(c, &ok);
        if (ok)
            p << coord;
    }
    auto pimpl = static_cast<QMapPolygonObjectPrivate *>(d_ptr.data());
    if (p != pimpl->path()) {
        pimpl->setPath(p);
        emit pathChanged();
    }
}

/*!
    \qmlproperty color Qt.labs.location::MapPolygonObject::color

    This property holds the fill color of the polygon when drawn. For no fill,
    use a transparent color.
*/
QColor QMapPolygonObject::color() const
{
    return static_cast<const QMapPolygonObjectPrivate*>(d_ptr.data())->fillColor();
}

/*!
    \qmlpropertygroup Qt.labs.location::MapPolygonObject::border
    \qmlproperty int MapPolygonObject::border.width
    \qmlproperty color MapPolygonObject::border.color

    This property is part of the border property group. The border
    property group holds the width and color used to draw the border.

    The width is in pixels and is independent of the zoom level of the map.
    The default values correspond to a black border with a width of 1 pixel.

    For no border, use a width of 0 or a transparent color.
*/
QDeclarativeMapLineProperties *QMapPolygonObject::border()
{
    if (!m_border) {
        m_border = new QDeclarativeMapLineProperties;
        connect(m_border, &QDeclarativeMapLineProperties::colorChanged, this, [this](const QColor &color){
            static_cast<QMapPolygonObjectPrivate*>(d_ptr.data())->setBorderColor(color);
        });
        connect(m_border, &QDeclarativeMapLineProperties::widthChanged, this, [this](qreal width){
            static_cast<QMapPolygonObjectPrivate*>(d_ptr.data())->setBorderWidth(width);
        });
    }
    return m_border;
}

void QMapPolygonObject::setColor(const QColor &fillColor)
{
    auto ptr = static_cast<QMapPolygonObjectPrivate*>(d_ptr.data());

    if (ptr->fillColor() == fillColor)
        return;

    ptr->setFillColor(fillColor);
    emit colorChanged();
}

void QMapPolygonObject::setMap(QGeoMap *map)
{
    QMapPolygonObjectPrivate *d = static_cast<QMapPolygonObjectPrivate *>(d_ptr.data());
    if (d->m_map == map)
        return;

    QGeoMapObject::setMap(map); // This is where the specialized pimpl gets created and injected

    if (!map) {
        // Map was set, now it has ben re-set to NULL, but not inside d_ptr.
        // so m_map inside d_ptr can still be used to remove itself, inside the destructor.
        d_ptr = new QMapPolygonObjectPrivateDefault(*d);
        // Old pimpl deleted implicitly by QExplicitlySharedDataPointer
    }
}

QT_END_NAMESPACE
