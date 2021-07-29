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

#include "qgeomapobjectqsgsupport_p.h"
#include <QtLocation/private/qgeomap_p_p.h>

QT_BEGIN_NAMESPACE

static int findMapObject(QGeoMapObject *o, const QList<MapObject> &list)
{
    for (int i = 0; i < list.size(); ++i)
    {
        if (list.at(i).object.data() == o)
            return i;
    }
    return -1;
}

bool QGeoMapObjectQSGSupport::createMapObjectImplementation(QGeoMapObject *obj, QGeoMapPrivate *d)
{
    QExplicitlySharedDataPointer<QGeoMapObjectPrivate> pimpl =
            QExplicitlySharedDataPointer<QGeoMapObjectPrivate>(d->createMapObjectImplementation(obj));
    if (pimpl.constData()) {
        bool res = obj->setImplementation(pimpl);
        if (res)
            emit m_map->sgNodeChanged();
        return res;
    }
    return false;
}

QGeoMapObjectPrivate *QGeoMapObjectQSGSupport::createMapObjectImplementationPrivate(QGeoMapObject *obj)
{
    QGeoMapObjectPrivate *res = nullptr;

    {
        QQSGMapObject *sgo = nullptr;
        switch (obj->type()) {
        case QGeoMapObject::PolylineType: {
            QMapPolylineObjectPrivate &oldImpl = static_cast<QMapPolylineObjectPrivate &>(*obj->implementation());
            QMapPolylineObjectPrivateQSG *pimpl =
                    new QMapPolylineObjectPrivateQSG(oldImpl);
            sgo = pimpl;
            res = pimpl;
            break;
        }
        case QGeoMapObject::PolygonType: {
            QMapPolygonObjectPrivate &oldImpl = static_cast<QMapPolygonObjectPrivate &>(*obj->implementation());
            QMapPolygonObjectPrivateQSG *pimpl =
                    new QMapPolygonObjectPrivateQSG(oldImpl);
            sgo = pimpl;
            res = pimpl;
            break;
        }
        case QGeoMapObject::CircleType: {
            QMapCircleObjectPrivate &oldImpl = static_cast<QMapCircleObjectPrivate &>(*obj->implementation());
            QMapCircleObjectPrivateQSG *pimpl =
                    new QMapCircleObjectPrivateQSG(oldImpl);
            sgo = pimpl;
            res = pimpl;
            break;
        }
        case QGeoMapObject::RouteType: {
            QMapRouteObjectPrivate &oldImpl = static_cast<QMapRouteObjectPrivate &>(*obj->implementation());
            QMapRouteObjectPrivateQSG *pimpl =
                    new QMapRouteObjectPrivateQSG(oldImpl);
            sgo = pimpl;
            res = pimpl;
            break;
        }
        case QGeoMapObject::IconType: {
            QMapIconObjectPrivate &oldImpl = static_cast<QMapIconObjectPrivate &>(*obj->implementation());
            QMapIconObjectPrivateQSG *pimpl =
                    new QMapIconObjectPrivateQSG(oldImpl);
            sgo = pimpl;
            res = pimpl;
            break;
        }
        default:
            // Use the following warning only for debugging purposes.
            // qWarning() << "QGeoMapObjectQSGSupport::createMapObjectImplementationPrivate: not instantiating pimpl for unsupported object type " << obj->type();
            break;
        }

        if (res) {
            QPointer<QGeoMapObject> p(obj);
            MapObject mo(p, sgo);
            m_pendingMapObjects << mo;
        }
    }
    return res;
}

QList<QGeoMapObject *> QGeoMapObjectQSGSupport::mapObjects() const
{
    return QList<QGeoMapObject *>();
}

void QGeoMapObjectQSGSupport::removeMapObject(QGeoMapObject *obj)
{
    int idx = findMapObject(obj, m_mapObjects);
    if (idx >= 0) {
        const MapObject &mo = m_mapObjects.takeAt(idx);
        obj->disconnect(m_map);
        m_removedMapObjects << mo;
        emit m_map->sgNodeChanged();
    } else {
        idx = findMapObject(obj, m_pendingMapObjects);
        if (idx >= 0) {
            m_pendingMapObjects.removeAt(idx);
            obj->disconnect(m_map);
        } else {
            // obj not here.
        }
    }
}

void QGeoMapObjectQSGSupport::updateMapObjects(QSGNode *root, QQuickWindow *window)
{
    for (int i = 0; i < m_removedMapObjects.size(); ++i) {
        MapObject mo = m_removedMapObjects[i];
        if (mo.qsgNode)  {
            root->removeChildNode(mo.qsgNode);
            delete mo.qsgNode;
            mo.qsgNode = nullptr;
            // mo.sgObject is now invalid as it is destroyed right after appending
            // mo to m_removedMapObjects
        }
    }
    m_removedMapObjects.clear();

    for (int i = 0; i < m_mapObjects.size(); ++i) {
        // already added as node
        if (Q_UNLIKELY(!m_mapObjects.at(i).object)) {
            qWarning() << "unexpected NULL pointer in m_mapObjects at "<<i;
            continue;
        }

        MapObject &mo = m_mapObjects[i];
        QQSGMapObject *sgo = mo.sgObject;
        QSGNode *oldNode = mo.qsgNode;
        mo.qsgNode = sgo->updateMapObjectNode(oldNode, &mo.visibleNode, root, window);
        if (Q_UNLIKELY(!mo.qsgNode)) {
            qWarning() << "updateMapObjectNode for "<<mo.object->type() << " returned NULL";
        } else if (mo.visibleNode && (mo.visibleNode->visible() != mo.object->visible())) {
            mo.visibleNode->setVisible(mo.object->visible());
            mo.qsgNode->markDirty(QSGNode::DirtySubtreeBlocked);
        }
    }

    QList<int> toRemove;
    for (int i = 0; i < m_pendingMapObjects.size(); ++i) {
        // already added as node
        MapObject &mo = m_pendingMapObjects[i];
        QQSGMapObject *sgo = mo.sgObject;
        QSGNode *oldNode = mo.qsgNode;
        sgo->updateGeometry(); // or subtree will be blocked
        mo.qsgNode = sgo->updateMapObjectNode(oldNode, &mo.visibleNode, root, window);
        if (mo.qsgNode) {
            if (mo.visibleNode && (mo.visibleNode->visible() != mo.object->visible())) {
                mo.visibleNode->setVisible(mo.object->visible());
                mo.qsgNode->markDirty(QSGNode::DirtySubtreeBlocked);
            }
            m_mapObjects << mo;
            toRemove.push_front(i);
            QObject::connect(mo.object, SIGNAL(visibleChanged()), m_map, SIGNAL(sgNodeChanged()));
        } else {
            // leave it to be processed, don't spit warnings
        }
    }

    for (int i: qAsConst(toRemove))
        m_pendingMapObjects.removeAt(i);
}

void QGeoMapObjectQSGSupport::updateObjectsGeometry()
{
    for (int i = 0; i < m_mapObjects.size(); ++i) {
        // already added as node
        if (Q_UNLIKELY(!m_mapObjects.at(i).object)) {
            qWarning() << "unexpected NULL pointer in m_mapObjects at "<<i;
            continue;
        }

        QQSGMapObject *sgo = m_mapObjects.at(i).sgObject;
        sgo->updateGeometry();
    }
    emit m_map->sgNodeChanged();
}

QT_END_NAMESPACE
