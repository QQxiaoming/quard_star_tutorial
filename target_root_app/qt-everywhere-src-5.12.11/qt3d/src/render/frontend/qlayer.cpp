/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include "qlayer.h"
#include "qlayer_p.h"

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

QLayerPrivate::QLayerPrivate()
    : QComponentPrivate()
    , m_recursive(false)
{
}

/*!
    \class Qt3DRender::QLayer
    \inmodule Qt3DRender
    \since 5.5
    \brief The QLayer class provides a way of filtering which entities will be rendered.

    Qt3DRender::QLayer works in conjunction with the Qt3DRender::QLayerFilter in the FrameGraph.
    \sa Qt3DRender::QLayerFilter

    A QLayer can be applied to a subtree of entities by setting the recursive property to true.

    \code
     #include <Qt3DCore/QEntity>
     #include <Qt3DRender/QGeometryRenderer>
     #include <Qt3DRender/QLayer>
     #include <Qt3DRender/QLayerFilter>
     #include <Qt3DRender/QViewport>

    // Scene
    Qt3DCore::QEntity *rootEntity = new Qt3DCore::Qt3DCore::QEntity;

    Qt3DCore::QEntity *renderableEntity = new Qt3DCore::Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QGeometryRenderer *geometryRenderer = new Qt3DCore::QGeometryRenderer(renderableEntity);
    Qt3DRender::QLayer *layer1 = new Qt3DCore::QLayer(renderableEntity);
    layer1->setRecursive(true);
    renderableEntity->addComponent(geometryRenderer);
    renderableEntity->addComponent(layer1);

    ...

    // FrameGraph
    Qt3DRender::QViewport *viewport = new Qt3DRender::QViewport;
    Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(viewport);
    layerFilter->addLayer(layer1);

    ...
    \endcode
*/
/*!
    \property QLayer::recursive
    Specifies if the layer is also applied to the entity subtree.
*/

/*!
    \qmltype Layer
    \instantiates Qt3DRender::QLayer
    \inherits Component3D
    \inqmlmodule Qt3D.Render
    \since 5.5
    \sa LayerFilter
    \brief Layer provides a way of filtering which entities will be rendered.

    Layer works in conjunction with the LayerFilter in the FrameGraph.

    A Layer can be applied to a subtree of entities by setting the recursive property to true.

    \code
    import Qt3D.Core 2.0
    import Qt3D.Render 2.0

    Entity {
        id: root

        components: RenderSettings {
            // FrameGraph
            Viewport {
                ClearBuffers {
                    buffers: ClearBuffers.ColorDepthBuffer
                    CameraSelector {
                        camera: mainCamera
                        LayerFilter {
                            layers: [layer1]
                        }
                    }
                }
            }
        }

        // Scene
        Camera { id: mainCamera }

        Layer {
            id: layer1
            recursive: true
        }

        GeometryRenderer { id: mesh }

        Entity {
            id: renderableEntity
            components: [ mesh, layer1 ]
        }
    }
    \endcode
*/

/*!
    \qmlproperty bool Layer::recursive

    Specifies if the layer is also applied to the entity subtree.
*/

/*! \fn Qt3DRender::QLayer::QLayer(Qt3DCore::QNode *parent)
  Constructs a new QLayer with the specified \a parent.
 */

QLayer::QLayer(QNode *parent)
    : QComponent(*new QLayerPrivate, parent)
{
}

/*! \internal */
QLayer::~QLayer()
{
}

bool QLayer::recursive() const
{
    Q_D(const QLayer);
    return d->m_recursive;
}

void QLayer::setRecursive(bool recursive)
{
    Q_D(QLayer);
    if (d->m_recursive != recursive) {
        d->m_recursive = recursive;
        emit recursiveChanged();
    }
}

/*! \internal */
QLayer::QLayer(QLayerPrivate &dd, QNode *parent)
    : QComponent(dd, parent)
{
}

Qt3DCore::QNodeCreatedChangeBasePtr QLayer::createNodeCreationChange() const
{
    auto creationChange = Qt3DCore::QNodeCreatedChangePtr<QLayerData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QLayer);
    data.m_recursive = d->m_recursive;
    return creationChange;
}

} // namespace Qt3DRender

QT_END_NAMESPACE
