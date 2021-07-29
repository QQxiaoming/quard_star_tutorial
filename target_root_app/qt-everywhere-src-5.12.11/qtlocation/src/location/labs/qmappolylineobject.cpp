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

#include "qmappolylineobject_p.h"
#include "qmappolylineobject_p_p.h"
#include <QtLocation/private/locationvaluetypehelper_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MapPolylineObject
    \instantiates QMapPolylineObject
    \inqmlmodule Qt.labs.location
    \ingroup qml-QtLocation5-maps
    \inherits QGeoMapObject

    \brief The MapPolylineObject displays a polyline on a Map.

    The MapPolylineObject displays a polyline on a Map.
    The MapPolylineObject type only makes sense when contained in a Map or in a \l MapObjectView.
*/

QMapPolylineObjectPrivate::QMapPolylineObjectPrivate(QGeoMapObject *q) : QGeoMapObjectPrivate(q)
{

}

QMapPolylineObjectPrivate::~QMapPolylineObjectPrivate()
{

}

QGeoMapObject::Type QMapPolylineObjectPrivate::type() const
{
    return QGeoMapObject::PolylineType;
}

QMapPolylineObjectPrivateDefault::QMapPolylineObjectPrivateDefault(QGeoMapObject *q) : QMapPolylineObjectPrivate(q)
{

}

QMapPolylineObjectPrivateDefault::QMapPolylineObjectPrivateDefault(const QMapPolylineObjectPrivate &other) : QMapPolylineObjectPrivate(other.q)
{
    m_path = other.path();
    m_color = other.color();
    m_width = other.width();
}

QMapPolylineObjectPrivateDefault::~QMapPolylineObjectPrivateDefault()
{

}

QList<QGeoCoordinate> QMapPolylineObjectPrivateDefault::path() const
{
    return m_path;
}

void QMapPolylineObjectPrivateDefault::setPath(const QList<QGeoCoordinate> &path)
{
    m_path = path;
}

QColor QMapPolylineObjectPrivateDefault::color() const
{
    return m_color;
}

void QMapPolylineObjectPrivateDefault::setColor(const QColor &color)
{
    m_color = color;
}

qreal QMapPolylineObjectPrivateDefault::width() const
{
    return m_width;
}

void QMapPolylineObjectPrivateDefault::setWidth(qreal width)
{
    m_width = width;
}

bool QMapPolylineObjectPrivate::equals(const QGeoMapObjectPrivate &other) const
{
    if (other.type() != type()) // This check might be unnecessary, depending on how equals gets used
        return false;

    const QMapPolylineObjectPrivate &o = static_cast<const QMapPolylineObjectPrivate &>(other);
    return (QGeoMapObjectPrivate::equals(o)
            && path() == o.path()
            && color() == o.color()
            && width() == o.width());
}

QGeoMapObjectPrivate *QMapPolylineObjectPrivateDefault::clone()
{
    return new QMapPolylineObjectPrivateDefault(static_cast<QMapPolylineObjectPrivate &>(*this));
}

QMapPolylineObject::QMapPolylineObject(QObject *parent)
 : QGeoMapObject(QExplicitlySharedDataPointer<QGeoMapObjectPrivate>(new QMapPolylineObjectPrivateDefault(this)), parent)
{
    QMapPolylineObjectPrivate *d = static_cast<QMapPolylineObjectPrivate*>(d_ptr.data());
    d->setColor(QColor(Qt::black)); // These are QDeclarativeMapLineProperties defaults
    d->setWidth(1.0);
}

QMapPolylineObject::~QMapPolylineObject()
{}

/*!
    \qmlproperty VariantList Qt.labs.location::MapPolylineObject::path

    This property holds the ordered list of coordinates which
    define the polyline.
*/
QVariantList QMapPolylineObject::path() const
{
    QVariantList p;
    for (const QGeoCoordinate &c: static_cast<const QMapPolylineObjectPrivate*>(d_ptr.data())->path())
        p << QVariant::fromValue(c);
    return p;
}

/*!
    \qmlpropertygroup Qt.labs.location::MapPolylineObject::line
    \qmlproperty int MapPolylineObject::line.width
    \qmlproperty color MapPolylineObject::line.color

    This property is part of the line property group. The line
    property group holds the width and color used to draw the line.

    The width is in pixels and is independent of the zoom level of the map.
    The default values correspond to a black border with a width of 1 pixel.

    For no line, use a width of 0 or a transparent color.
*/
QDeclarativeMapLineProperties *QMapPolylineObject::border()
{
    if (!m_border) {
        m_border = new QDeclarativeMapLineProperties;
        connect(m_border, &QDeclarativeMapLineProperties::colorChanged, this, [this](const QColor &color){
            static_cast<QMapPolylineObjectPrivate*>(d_ptr.data())->setColor(color);
        });
        connect(m_border, &QDeclarativeMapLineProperties::widthChanged, this, [this](qreal width){
            static_cast<QMapPolylineObjectPrivate*>(d_ptr.data())->setWidth(width);
        });
    }
    return m_border;
}

void QMapPolylineObject::setPath(const QVariantList &path)
{
    QList<QGeoCoordinate> p;
    bool ok = false;
    for (const auto &c: path) {
        const QGeoCoordinate coord = parseCoordinate(c, &ok);
        if (ok)
            p << coord;
    }
    auto pimpl = static_cast<QMapPolylineObjectPrivate *>(d_ptr.data());
    if (p != pimpl->path()) {
        pimpl->setPath(p);
        emit pathChanged();
    }
}

void QMapPolylineObject::setMap(QGeoMap *map)
{
    QMapPolylineObjectPrivate *d = static_cast<QMapPolylineObjectPrivate *>(d_ptr.data());
    if (d->m_map == map)
        return;

    QGeoMapObject::setMap(map); // This is where the specialized pimpl gets created and injected

    if (!map) {
        // Map was set, now it has ben re-set to NULL, but not inside d_ptr.
        // so m_map inside d_ptr can still be used to remove itself, inside the destructor.
        d_ptr = new QMapPolylineObjectPrivateDefault(*d);
        // Old pimpl deleted implicitly by QExplicitlySharedDataPointer
    }
}

QT_END_NAMESPACE
