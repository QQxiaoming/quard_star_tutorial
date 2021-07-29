/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#ifndef DELEGATED_FRAME_NODE_H
#define DELEGATED_FRAME_NODE_H

#include "base/containers/circular_deque.h"
#include "components/viz/common/quads/compositor_frame.h"
#include "components/viz/common/quads/render_pass.h"
#include "components/viz/common/resources/transferable_resource.h"
#include "gpu/command_buffer/service/sync_point_manager.h"
#include "ui/gl/gl_fence.h"
#include <QMutex>
#include <QSGNode>
#include <QSharedData>
#include <QSharedPointer>
#include <QWaitCondition>
#include <QtGui/QOffscreenSurface>

#include "chromium_gpu_helper.h"
#include "render_widget_host_view_qt_delegate.h"

QT_BEGIN_NAMESPACE
class QSGLayer;
QT_END_NAMESPACE

namespace gfx {
class QuadF;
}

namespace viz {
class DelegatedFrameData;
class DrawQuad;
class DrawPolygon;
}

namespace QtWebEngineCore {

class DelegatedNodeTreeHandler;
class MailboxTexture;
class ResourceHolder;

// Separating this data allows another DelegatedFrameNode to reconstruct the QSGNode tree from the mailbox textures
// and render pass information.
class ChromiumCompositorData : public QSharedData {
public:
    ChromiumCompositorData() : frameDevicePixelRatio(1) { }
    QHash<unsigned, QSharedPointer<ResourceHolder> > resourceHolders;
    viz::CompositorFrame frameData;
    viz::CompositorFrame previousFrameData;
    qreal frameDevicePixelRatio;
};

class DelegatedFrameNode : public QSGTransformNode {
public:
    DelegatedFrameNode();
    ~DelegatedFrameNode();
    void preprocess();
    void commit(ChromiumCompositorData *chromiumCompositorData, std::vector<viz::ReturnedResource> *resourcesToRelease, RenderWidgetHostViewQtDelegate *apiDelegate);

private:
    void flushPolygons(base::circular_deque<std::unique_ptr<viz::DrawPolygon> > *polygonQueue,
        QSGNode *renderPassChain,
        DelegatedNodeTreeHandler *nodeHandler,
        QHash<unsigned, QSharedPointer<ResourceHolder> > &resourceCandidates,
        RenderWidgetHostViewQtDelegate *apiDelegate);
    void handlePolygon(
        const viz::DrawPolygon *polygon,
        QSGNode *currentLayerChain,
        DelegatedNodeTreeHandler *nodeHandler,
        QHash<unsigned, QSharedPointer<ResourceHolder> > &resourceCandidates,
        RenderWidgetHostViewQtDelegate *apiDelegate);
    void handleClippedQuad(
        const viz::DrawQuad *quad,
        const gfx::QuadF &clipRegion,
        QSGNode *currentLayerChain,
        DelegatedNodeTreeHandler *nodeHandler,
        QHash<unsigned, QSharedPointer<ResourceHolder> > &resourceCandidates,
        RenderWidgetHostViewQtDelegate *apiDelegate);
    void handleQuad(
        const viz::DrawQuad *quad,
        QSGNode *currentLayerChain,
        DelegatedNodeTreeHandler *nodeHandler,
        QHash<unsigned, QSharedPointer<ResourceHolder> > &resourceCandidates,
        RenderWidgetHostViewQtDelegate *apiDelegate);
    void fetchAndSyncMailboxes(QList<MailboxTexture *> &mailboxesToFetch);
    // Making those callbacks static bypasses base::Bind's ref-counting requirement
    // of the this pointer when the callback is a method.
    static void pullTexture(DelegatedFrameNode *frameNode, MailboxTexture *mailbox);
    static void pullTextures(DelegatedFrameNode *frameNode, const QVector<MailboxTexture *> mailboxes);
    static void fenceAndUnlockQt(DelegatedFrameNode *frameNode);
    static void unlockQt(DelegatedFrameNode *frameNode);

    ResourceHolder *findAndHoldResource(unsigned resourceId, QHash<unsigned, QSharedPointer<ResourceHolder> > &candidates);
    void holdResources(const viz::DrawQuad *quad, QHash<unsigned, QSharedPointer<ResourceHolder> > &candidates);
    void holdResources(const viz::RenderPass *pass, QHash<unsigned, QSharedPointer<ResourceHolder> > &candidates);
    QSGTexture *initAndHoldTexture(ResourceHolder *resource, bool quadIsAllOpaque, RenderWidgetHostViewQtDelegate *apiDelegate = 0);

    QExplicitlySharedDataPointer<ChromiumCompositorData> m_chromiumCompositorData;
    struct SGObjects {
        QVector<QPair<int, QSharedPointer<QSGLayer> > > renderPassLayers;
        QVector<QSharedPointer<QSGRootNode> > renderPassRootNodes;
        QVector<QSharedPointer<QSGTexture> > textureStrongRefs;
    } m_sgObjects;
    QVector<QSGNode*> m_sceneGraphNodes;
    int m_numPendingSyncPoints;
    QWaitCondition m_mailboxesFetchedWaitCond;
    QMutex m_mutex;
    QList<gl::TransferableFence> m_textureFences;
#if defined(USE_OZONE)
    bool m_contextShared;
    QScopedPointer<QOffscreenSurface> m_offsurface;
#endif
    QSize m_previousViewportSize;
};

} // namespace QtWebEngineCore

#endif // DELEGATED_FRAME_NODE_H
