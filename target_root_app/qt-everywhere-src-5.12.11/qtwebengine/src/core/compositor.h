/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <base/memory/weak_ptr.h>
#include <components/viz/common/frame_sinks/begin_frame_source.h>

#include <QtCore/qglobal.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE
class QSGNode;
QT_END_NAMESPACE

namespace viz {
class CompositorFrame;
struct ReturnedResource;
namespace mojom {
class CompositorFrameSinkClient;
} // namespace mojom
} // namespace viz

namespace QtWebEngineCore {

class RenderWidgetHostViewQt;
class RenderWidgetHostViewQtDelegate;
class ChromiumCompositorData;

// Receives viz::CompositorFrames from child compositors and provides QSGNodes
// to the Qt Quick renderer.
//
// The life cycle of a frame:
//
//   Step 1. A new CompositorFrame is received from child compositors and handed
//   off to submitFrame(). The new frame will start off in a pending state.
//
//   Step 2. Once the new frame is ready to be rendered, Compositor will call
//   update() on the delegate.
//
//   Step 3. Once the delegate is ready to render, updatePaintNode() should be
//   called to receive the scene graph for the new frame. This call will commit
//   the pending frame. Until the next frame is ready, all subsequent calls to
//   updatePaintNode() will keep using this same committed frame.
//
//   Step 4. The Compositor will return unneeded resources back to the child
//   compositors. Go to step 1.
class Compositor final : private viz::BeginFrameObserverBase
{
public:
    explicit Compositor(RenderWidgetHostViewQt *hostView);
    ~Compositor() override;

    void setViewDelegate(RenderWidgetHostViewQtDelegate *viewDelegate);
    void setFrameSinkClient(viz::mojom::CompositorFrameSinkClient *frameSinkClient);
    void setNeedsBeginFrames(bool needsBeginFrames);

    void submitFrame(viz::CompositorFrame frame);

    QSGNode *updatePaintNode(QSGNode *oldNode);

private:
    void notifyFrameCommitted();
    void sendPresentationFeedback(uint frame_token);

    // viz::BeginFrameObserverBase
    bool OnBeginFrameDerivedImpl(const viz::BeginFrameArgs &args) override;
    void OnBeginFrameSourcePausedChanged(bool paused) override;

    std::vector<viz::ReturnedResource> m_resourcesToRelease;
    QExplicitlySharedDataPointer<ChromiumCompositorData> m_chromiumCompositorData;
    RenderWidgetHostViewQt *m_view;
    RenderWidgetHostViewQtDelegate *m_viewDelegate = nullptr;
    std::unique_ptr<viz::SyntheticBeginFrameSource> m_beginFrameSource;
    viz::mojom::CompositorFrameSinkClient *m_frameSinkClient = nullptr;
    bool m_havePendingFrame = false;
    bool m_needsBeginFrames = false;

    base::WeakPtrFactory<Compositor> m_weakPtrFactory{this};

    DISALLOW_COPY_AND_ASSIGN(Compositor);
};

} // namespace QtWebEngineCore

#endif // !COMPOSITOR_H
