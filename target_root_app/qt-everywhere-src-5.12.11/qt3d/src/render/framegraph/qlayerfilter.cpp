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

#include "qlayerfilter.h"
#include "qlayerfilter_p.h"
#include "qlayer.h"
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qpropertynodeaddedchange.h>
#include <Qt3DCore/qpropertynoderemovedchange.h>
#include <Qt3DRender/qframegraphnodecreatedchange.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

QLayerFilterPrivate::QLayerFilterPrivate()
    : QFrameGraphNodePrivate()
    , m_filterMode(QLayerFilter::AcceptAnyMatchingLayers)
{
}

/*!
    \class Qt3DRender::QLayerFilter
    \inmodule Qt3DRender
    \since 5.5
    \brief Controls layers drawn in a frame graph branch.

    A Qt3DRender::QLayerFilter can be used to instruct the renderer as to
    which layer(s) to draw in that branch of the frame graph. QLayerFilter
    selects which entities to draw based on the QLayer instance(s) added to
    the QLayerFilter and as components to Qt3DCore::QEntity.

    QLayerFilter can be configured to select or discard entities with a
    specific \l QLayer depending on the filterMode property. By default,
    entities referencing one of the \l QLayer objects that are also being
    referenced by the \l QLayerFilter are selected (AcceptAnyMatchingLayers).

    Within the FrameGraph tree, multiple \l QLayerFilter nodes can be nested
    within a branch going from root to a leaf. In that case the filtering will
    first operate on all entities of the scene using the filtering method
    specified by the first declared \l QLayerFilter. Then the filtered subset
    of entities will be filtered again based on the filtering method set on the
    second \l QLayerFilter declared. This is then repeated until all \l
    QLayerFilter nodes of the branch have been consumed.
*/

/*!
    \enum QLayerFilter::FilterMode

    Specifies the rules for selecting entities to draw.

    \value AcceptAnyMatchingLayers
    Accept entities that reference one or more \l QLayer objects added to this
    QLayerFilter. This is the default

    \value AcceptAllMatchingLayers
    Accept entities that reference all the \l QLayer objects added to this
    QLayerFilter

    \value DiscardAnyMatchingLayers
    Discard entities that reference one or more \l QLayer objects added to this
    QLayerFilter

    \value DiscardAllMatchingLayers
    Discard entities that reference all \l QLayer objects added to this
    QLayerFilter
*/

/*!
    \property Qt3DRender::QLayerFilter::filterMode

    Holds the filter mode specifying the entities to select for drawing.

    The default value is AcceptMatchingLayers.
*/

/*!
    \qmltype LayerFilter
    \instantiates Qt3DRender::QLayerFilter
    \inherits FrameGraphNode
    \inqmlmodule Qt3D.Render
    \since 5.5
    \brief Controls layers drawn in a frame graph branch.

    A LayerFilter can be used to instruct the renderer as to which layer(s)
    to draw in that branch of the frame graph. The LayerFilter selects which
    entities to draw based on the \l Layer instances added to the LayerFilter
    and as components to the \l Entity.

    The LayerFilter can be configured to select or discard entities with a
    specific \l Layer depending on the filterMode property. By default,
    entities referencing one of the \l Layer objects that are also being
    referenced by the \l LayerFilter are selected (AcceptAnyMatchingLayers).

    Within the FrameGraph tree, multiple \l LayerFilter nodes can be nested
    within a branch going from root to a leaf. In that case the filtering will
    first operate on all entities of the scene using the filtering method
    specified by the first declared \l LayerFilter. Then the filtered subset of
    entities will be filtered again based on the filtering method set on the
    second \l LayerFilter declared. This is then repeated until all \l
    LayerFilter nodes of the branch have been consumed.
*/

/*!
    \qmlproperty list<Layer> Qt3D.Render::LayerFilter::layers
    Holds a list of layers specifying the layers to select for drawing.
    \readonly
 */

/*!
    \qmlproperty enumeration Qt3DRender::LayerFilter::filterMode

    Holds the filter mode specifying the entities to select for drawing.

    The default value is \c {LayerFilter.AcceptMatchingLayers}.

    \value LayerFilter.AcceptAnyMatchingLayers
    Accept entities that reference one or more \l Layer objects added to this
    LayerFilter. This is the default

    \value LayerFilter.AcceptAllMatchingLayers
    Accept entities that reference all the \l Layer objects added to this
    LayerFilter

    \value LayerFilter.DiscardAnyMatchingLayers
    Discard entities that reference one or more \l Layer objects added to this
    LayerFilter

    \value LayerFilter.DiscardAllMatchingLayers
    Discard entities that reference all \l Layer objects added to this
    LayerFilter
*/

/*!
    The constructor creates an instance with the specified \a parent.
 */
QLayerFilter::QLayerFilter(QNode *parent)
    : QFrameGraphNode(*new QLayerFilterPrivate, parent)
{
}

/*! \internal */
QLayerFilter::QLayerFilter(QLayerFilterPrivate &dd, QNode *parent)
    : QFrameGraphNode(dd, parent)
{
}

/*! \internal */
QLayerFilter::~QLayerFilter()
{
}

/*!
    Add \a layer to the current list of layers
 */
void QLayerFilter::addLayer(QLayer *layer)
{
    Q_ASSERT(layer);
    Q_D(QLayerFilter);
    if (!d->m_layers.contains(layer)) {
        d->m_layers.append(layer);

        // Ensures proper bookkeeping
        d->registerDestructionHelper(layer, &QLayerFilter::removeLayer, d->m_layers);

        // We need to add it as a child of the current node if it has been declared inline
        // Or not previously added as a child of the current node so that
        // 1) The backend gets notified about it's creation
        // 2) When the current node is destroyed, it gets destroyed as well
        if (!layer->parent())
            layer->setParent(this);

        if (d->m_changeArbiter != nullptr) {
            const auto change = Qt3DCore::QPropertyNodeAddedChangePtr::create(id(), layer);
            change->setPropertyName("layer");
            d->notifyObservers(change);
        }
    }
}

/*!
    Remove \a layer from the current list of layers
 */
void QLayerFilter::removeLayer(QLayer *layer)
{
    Q_ASSERT(layer);
    Q_D(QLayerFilter);
    if (d->m_changeArbiter != nullptr) {
        const auto change = Qt3DCore::QPropertyNodeRemovedChangePtr::create(id(), layer);
        change->setPropertyName("layer");
        d->notifyObservers(change);
    }
    d->m_layers.removeOne(layer);
    // Remove bookkeeping connection
    d->unregisterDestructionHelper(layer);
}

/*!
    \return the current list of layers
 */
QVector<QLayer *> QLayerFilter::layers() const
{
    Q_D(const QLayerFilter);
    return d->m_layers;
}

QLayerFilter::FilterMode QLayerFilter::filterMode() const
{
    Q_D(const QLayerFilter);
    return d->m_filterMode;
}

void QLayerFilter::setFilterMode(QLayerFilter::FilterMode filterMode)
{
    Q_D(QLayerFilter);
    if (d->m_filterMode != filterMode) {
        d->m_filterMode = filterMode;
        emit filterModeChanged(filterMode);
    }
}

Qt3DCore::QNodeCreatedChangeBasePtr QLayerFilter::createNodeCreationChange() const
{
    auto creationChange = QFrameGraphNodeCreatedChangePtr<QLayerFilterData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QLayerFilter);
    data.layerIds = qIdsForNodes(d->m_layers);
    data.filterMode = d->m_filterMode;
    return creationChange;
}

} // namespace Qt3DRender

QT_END_NAMESPACE
