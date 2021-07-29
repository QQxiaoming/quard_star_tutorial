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

#include "qdeclarativegeomap_p.h"
#include "qgeomapobject_p.h"
#include "qgeomapobject_p_p.h"

QT_BEGIN_NAMESPACE

/*!
    \internal

    \qmltype GeoMapObject
    \instantiates QGeoMapObject
    \inqmlmodule Qt.labs.location
    \ingroup qml-QtLocation5-maps

    \brief The GeoObject type is a base class for geographical objects that can be added to a map.

    The difference between a GeoMapObject and a MapItem is twofold. First, GeoMapObject are always backed
    by a plugin-specific implementation and do not come with a default implementation. If a plugin does
    not support a specific GeoMapObject type, adding such a GeoMapObject will have no effect.
    Second, GeoMapObject are not QQuickItems, thus being a much more lightweight way to add content to
    a map.

    GeoMapObject support is plugin-dependent, and is documented per plugin.
*/

template<>
QGeoMapObjectPrivate *QExplicitlySharedDataPointer<QGeoMapObjectPrivate>::clone()
{
    return d->clone();
}

QGeoMapObject::~QGeoMapObject()
{

}

/*!
    Returns whether this geographical object and \a other are equal.
*/
bool QGeoMapObject::operator ==(const QGeoMapObject &other) const
{
    return ( (d_ptr.constData() == other.d_ptr.constData())
            || (*d_ptr) == (*other.d_ptr));
}

/*!
    Returns whether this geographical object and \a other are not equal.
*/
bool QGeoMapObject::operator !=(const QGeoMapObject &other) const
{
    return !(operator==(other));
}

/*!
  \internal
  Returns which features are supported by the geographical object
*/
QGeoMapObject::Features QGeoMapObject::features() const
{
    return d_ptr->features();
}

QGeoMapObjectPrivate *QGeoMapObject::implementation() const
{
    return d_ptr.data();
}

bool QGeoMapObject::setImplementation(const QExplicitlySharedDataPointer<QGeoMapObjectPrivate> &pimpl)
{
    if (d_ptr->type() != pimpl->type())
        return false;
    d_ptr = pimpl;
    return true;
}

bool QGeoMapObject::implemented() const
{
    return !d_ptr->engineName().isEmpty();
}

bool QGeoMapObject::visible() const
{
    return d_ptr->visible();
}

void QGeoMapObject::setVisible(bool visible)
{
    if (visible == d_ptr->m_visible)
        return;

    const bool oldVisible = QGeoMapObject::visible();
    d_ptr->setVisible(visible);
    if (d_ptr->m_componentCompleted)
        setChildrenVisibility();
    if (QGeoMapObject::visible() != oldVisible)
        emit visibleChanged();
}

void QGeoMapObject::setParentVisiblity(bool visible)
{
    if (visible == d_ptr->m_parentVisible)
        return;

    const bool oldVisible = QGeoMapObject::visible();
    d_ptr->setParentVisibility(visible);
    if (d_ptr->m_componentCompleted)
        setChildrenVisibility();
    if (QGeoMapObject::visible() != oldVisible)
        emit visibleChanged();
}

QGeoMapObject::Type QGeoMapObject::type() const
{
    return d_ptr->type();
}

QList<QGeoMapObject *> QGeoMapObject::geoMapObjectChildren() const
{
    return quickChildren<QGeoMapObject>();
}

QGeoMapObject::QGeoMapObject(const QExplicitlySharedDataPointer<QGeoMapObjectPrivate> &dd, QObject *parent)
    : QParameterizableObject(parent), d_ptr(dd)
{
}

void QGeoMapObject::setChildrenVisibility()
{
    const bool v = visible();
    const QList<QGeoMapObject *> kids = geoMapObjectChildren();
    for (auto kid : qAsConst(kids))
        kid->setParentVisiblity(v);
}

void QGeoMapObject::classBegin()
{

}

void QGeoMapObject::completeComponent()
{
    d_ptr->m_componentCompleted = true;
    setChildrenVisibility();
}

void QGeoMapObject::componentComplete()
{
    completeComponent();
    emit completed();
}

void QGeoMapObject::setMap(QGeoMap *map)
{
    if (d_ptr->m_map == map)
        return;

    if (map) {
        bool oldVisible = d_ptr->m_visible;
        bool oldCmponentCompleted = d_ptr->m_componentCompleted;
        if (!map->createMapObjectImplementation(this))
            qWarning() << "Unsupported type " << type();
        // old implementation gets destroyed if/when d_ptr gets replaced
        d_ptr->m_componentCompleted = oldCmponentCompleted;
        d_ptr->setVisible(oldVisible);
        d_ptr->setMap(map);
    }

    const QList<QGeoMapObject *> kids = geoMapObjectChildren();
    for (auto kid : kids)
        kid->setMap(map);

    // Each subclass is in charge to do the equivalent of
    //    if (!map) {
    //        // Map was set, now it has ben re-set to NULL, but not inside d_ptr.
    //        // so m_map inside d_ptr can still be used to remove itself, inside the destructor.
    //        d_ptr = new QMapCircleObjectPrivateDefault(*d);
    //        // Old pimpl deleted implicitly by QExplicitlySharedDataPointer
    //    }
    // After this method is called.
}

QGeoMap *QGeoMapObject::map() const
{
    return d_ptr->m_map;
}


//
// QGeoMapObjectPrivate
//

QGeoMapObjectPrivate::QGeoMapObjectPrivate()
{
}

QGeoMapObjectPrivate::QGeoMapObjectPrivate(QGeoMapObject *q) : q(q)
{
}

QGeoMapObjectPrivate::QGeoMapObjectPrivate(const QGeoMapObjectPrivate &other)
    :QSharedData(other)
    ,q(other.q)
    ,m_componentCompleted(other.m_componentCompleted)
    ,m_visible(other.m_visible)
{

}

QGeoMapObjectPrivate::~QGeoMapObjectPrivate()
{
}

bool QGeoMapObjectPrivate::operator ==(const QGeoMapObjectPrivate &other) const
{
    return (type() == other.type() && engineName() == other.engineName()
            && equals(other));
}

QByteArray QGeoMapObjectPrivate::engineName() const
{
    return QByteArray();
}

QGeoMapObject::Type QGeoMapObjectPrivate::type() const
{
    return QGeoMapObject::InvalidType;
}

QGeoMapObject::Features QGeoMapObjectPrivate::features() const
{
    return QGeoMapObject::NoFeature;
}

bool QGeoMapObjectPrivate::equals(const QGeoMapObjectPrivate &other) const
{
    return (visible() == other.visible() && type() == other.type()
            && engineName() == other.engineName() && features() == other.features()
            && m_map == other.m_map);
}

bool QGeoMapObjectPrivate::visible() const
{
    return m_visible && m_parentVisible;
}

void QGeoMapObjectPrivate::setVisible(bool visible)
{
    m_visible = visible;
}

void QGeoMapObjectPrivate::setParentVisibility(bool visible)
{
    m_parentVisible = visible;
}

void QGeoMapObjectPrivate::setMap(QGeoMap *map)
{
    m_map = map;
}

QT_END_NAMESPACE
