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

#include "compositor.h"

#include "delegated_frame_node.h"
#include "render_widget_host_view_qt.h"

#include "components/viz/common/resources/returned_resource.h"
#include "content/public/browser/browser_thread.h"
#include "services/viz/public/interfaces/compositing/compositor_frame_sink.mojom.h"

namespace QtWebEngineCore {

Compositor::Compositor(RenderWidgetHostViewQt *hostView)
    : m_chromiumCompositorData(new ChromiumCompositorData)
    , m_view(hostView)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    base::SingleThreadTaskRunner *taskRunner =
        content::BrowserThread::GetTaskRunnerForThread(content::BrowserThread::UI).get();
    m_beginFrameSource =
        std::make_unique<viz::DelayBasedBeginFrameSource>(
            std::make_unique<viz::DelayBasedTimeSource>(taskRunner),
            viz::BeginFrameSource::kNotRestartableId);
}

Compositor::~Compositor()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

void Compositor::setViewDelegate(RenderWidgetHostViewQtDelegate *viewDelegate)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    m_viewDelegate = viewDelegate;
}

void Compositor::setFrameSinkClient(viz::mojom::CompositorFrameSinkClient *frameSinkClient)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    if (m_frameSinkClient == frameSinkClient)
        return;

    // Accumulated resources belong to the old RendererCompositorFrameSink and
    // should not be returned.
    //
    // TODO(juvaldma): Can there be a pending frame from the old client?
    m_resourcesToRelease.clear();
    m_frameSinkClient = frameSinkClient;
}

void Compositor::setNeedsBeginFrames(bool needsBeginFrames)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    if (m_needsBeginFrames == needsBeginFrames)
        return;

    if (needsBeginFrames)
        m_beginFrameSource->AddObserver(this);
    else
        m_beginFrameSource->RemoveObserver(this);

    m_needsBeginFrames = needsBeginFrames;
}

void Compositor::submitFrame(viz::CompositorFrame frame)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    DCHECK(!m_havePendingFrame);

    m_chromiumCompositorData->frameDevicePixelRatio = frame.metadata.device_scale_factor;
    m_chromiumCompositorData->previousFrameData = std::move(m_chromiumCompositorData->frameData);
    m_chromiumCompositorData->frameData = std::move(frame);
    m_havePendingFrame = true;

    // Tell viewDelegate to call updatePaintNode() soon.
    m_viewDelegate->update();
}

QSGNode *Compositor::updatePaintNode(QSGNode *oldNode)
{
    // DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    //
    // This might be called from a Qt Quick render thread, but the UI thread
    // will still be blocked for the duration of this call.

    DelegatedFrameNode *frameNode = static_cast<DelegatedFrameNode *>(oldNode);
    if (!frameNode)
        frameNode = new DelegatedFrameNode;

    frameNode->commit(m_chromiumCompositorData.data(), &m_resourcesToRelease, m_viewDelegate);

    if (m_havePendingFrame) {
        m_havePendingFrame = false;
        content::BrowserThread::PostTask(
            content::BrowserThread::UI, FROM_HERE,
            base::BindOnce(&Compositor::notifyFrameCommitted, m_weakPtrFactory.GetWeakPtr()));
    }
    if (m_chromiumCompositorData->frameData.metadata.request_presentation_feedback)
        content::BrowserThread::PostTask(
            content::BrowserThread::UI, FROM_HERE,
            base::BindOnce(&Compositor::sendPresentationFeedback, m_weakPtrFactory.GetWeakPtr(), m_chromiumCompositorData->frameData.metadata.frame_token));

    return frameNode;
}

void Compositor::notifyFrameCommitted()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    m_beginFrameSource->DidFinishFrame(this);
    if (m_frameSinkClient)
        m_frameSinkClient->DidReceiveCompositorFrameAck(m_resourcesToRelease);
    m_resourcesToRelease.clear();
}

void Compositor::sendPresentationFeedback(uint frame_token)
{
    gfx::PresentationFeedback dummyFeedback(base::TimeTicks::Now(), base::TimeDelta(), gfx::PresentationFeedback::Flags::kVSync);
    m_frameSinkClient->DidPresentCompositorFrame(frame_token, dummyFeedback);
}

bool Compositor::OnBeginFrameDerivedImpl(const viz::BeginFrameArgs &args)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    m_view->OnBeginFrame(args.frame_time);
    m_beginFrameSource->OnUpdateVSyncParameters(args.frame_time, args.interval);
    if (m_frameSinkClient)
        m_frameSinkClient->OnBeginFrame(args);

    return true;
}

void Compositor::OnBeginFrameSourcePausedChanged(bool)
{
    // Ignored for now.  If the begin frame source is paused, the renderer
    // doesn't need to be informed about it and will just not receive more
    // begin frames.
}

} // namespace QtWebEngineCore
