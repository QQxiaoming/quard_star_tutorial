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

#include "qmapiconobjectqsg_p_p.h"
#include <QtQuick/qsgimagenode.h>
#include <QtQuick/qsgnode.h>
#include <QtQuick/private/qquickimage_p.h>
#include <QtQuick/qquickimageprovider.h>
#include <QtQuick/qquickwindow.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqml.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtLocation/private/qdeclarativepolylinemapitem_p.h>

QT_BEGIN_NAMESPACE

class RootNode : public QSGTransformNode, public VisibleNode
{
public:
    RootNode() { }

    bool isSubtreeBlocked() const override
    {
        return subtreeBlocked();
    }
};

QMapIconObjectPrivateQSG::QMapIconObjectPrivateQSG(QGeoMapObject *q)
    : QMapIconObjectPrivateDefault(q)
{

}

QMapIconObjectPrivateQSG::QMapIconObjectPrivateQSG(const QMapIconObjectPrivate &other)
    : QMapIconObjectPrivateDefault(other)
{
    setContent(content());
    setCoordinate(coordinate());
}

QMapIconObjectPrivateQSG::~QMapIconObjectPrivateQSG()
{
    if (m_map)
        m_map->removeMapObject(q);
}

void QMapIconObjectPrivateQSG::updateGeometry()
{
    if (!m_map)
        return;

    m_geometryDirty = true;
    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(m_map->geoProjection());

    m_itemPosition = p.coordinateToItemPosition(coordinate());
    if (m_itemPosition.isFinite()) {
        m_transformation.setToIdentity();
        m_transformation.translate(QVector3D(m_itemPosition.x(), m_itemPosition.y(), 0));
    }

    // TODO: support and test for zoomLevel
}

QSGNode *QMapIconObjectPrivateQSG::updateMapObjectNode(QSGNode *oldNode,
                                                       VisibleNode **visibleNode,
                                                       QSGNode *root,
                                                       QQuickWindow *window)
{
    Q_UNUSED(visibleNode)
    bool created = false;
    RootNode *node = static_cast<RootNode *>(oldNode);
    if (!node) {
        node = new RootNode();
        m_imageNode = window->createImageNode();
        m_imageNode->setOwnsTexture(true);
        node->appendChildNode(m_imageNode);
        *visibleNode = static_cast<VisibleNode *>(node);
        created = true;
    }

    if (m_imageDirty) {
        m_imageDirty = false;
        m_imageNode->setTexture(window->createTextureFromImage(m_image));
        QRect rect = m_image.rect();
        m_imageNode->setSourceRect(rect);
        m_imageNode->setRect(QRectF(QPointF(0,0), m_size));
    }

    if (m_geometryDirty) {
        m_geometryDirty = false;
        if (!m_itemPosition.isFinite()) {
            node->setSubtreeBlocked(true);
        } else {
            node->setSubtreeBlocked(false);
            node->setMatrix(m_transformation);
        }
    }

    if (created)
        root->appendChildNode(node);

    return node;
}

void QMapIconObjectPrivateQSG::setCoordinate(const QGeoCoordinate &coordinate)
{
    QMapIconObjectPrivateDefault::setCoordinate(coordinate);
    updateGeometry();
}

template<typename T>
static T *getContent(const QVariant &content)
{
    QObject *obj = qvariant_cast<QObject *>(content);
    return qobject_cast<T *>(obj);
}

static inline QString imageId(const QUrl &url)
{
    return url.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority).mid(1);
}

void QMapIconObjectPrivateQSG::clearContent()
{
    m_image = QImage();
}

void QMapIconObjectPrivateQSG::setContent(const QVariant &content)
{
    // First reset all local containers
    clearContent();
    QQmlEngine *engine = qmlEngine(q);

    // Then pull the new content
    QMapIconObjectPrivateDefault::setContent(content);
    switch (content.type()) {
        case QVariant::UserType: {
            // TODO: Handle QObject subclasses -- first decide which ones
            break;
        }
        case QVariant::String:
        case QVariant::Url: {
            // URL, including image/texture providers
            // Supporting only image providers for now
            const QUrl url = content.toUrl();
            if (!url.isValid()) {
                m_image = QImage(content.toString());
                m_imageDirty = true;
                updateGeometry();
            } else if (url.scheme().isEmpty() || url.scheme() == QLatin1String("file")) {
                m_image = QImage(url.toString(QUrl::RemoveScheme));
                m_imageDirty = true;
                updateGeometry();
            } else if (url.scheme() == QLatin1String("image")) {
                QQuickImageProvider *provider = static_cast<QQuickImageProvider *>(engine->imageProvider(url.host()));
                QSize outSize;
                m_image = provider->requestImage(imageId(url), &outSize, QSize());
                if (outSize.isEmpty())
                    break;
                m_imageDirty = true;
                updateGeometry();
            } else { // ToDo: Use QNAM

            }

            break;
        }
        case QVariant::ByteArray: {
            // ToDo: Build the image from bytearray
            break;
        }
        default:
            qWarning() << "Unsupported parameter type: " << content.type();
            break;
    }

    if (m_map && m_imageDirty)
        emit m_map->sgNodeChanged();
}

void QMapIconObjectPrivateQSG::setSize(const QSizeF &size)
{
    QMapIconObjectPrivateDefault::setSize(size);
    updateGeometry();
}

QGeoMapObjectPrivate *QMapIconObjectPrivateQSG::clone()
{
    return new QMapIconObjectPrivateQSG(static_cast<QMapIconObjectPrivate &>(*this));
}

QT_END_NAMESPACE
