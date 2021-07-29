/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>
#include <Qt3DRender/private/renderer_p.h>
#include <Qt3DRender/private/viewportnode_p.h>
#include <Qt3DRender/private/renderview_p.h>
#include <Qt3DRender/private/renderviewbuilder_p.h>
#include <Qt3DRender/private/offscreensurfacehelper_p.h>
#include <Qt3DRender/private/renderqueue_p.h>

class tst_Renderer : public QObject
{
    Q_OBJECT
public :
    tst_Renderer() {}
    ~tst_Renderer() {}

private Q_SLOTS:
    void checkRenderBinJobs()
    {
        // GIVEN
        Qt3DRender::Render::NodeManagers nodeManagers;
        Qt3DRender::Render::Renderer renderer(Qt3DRender::QRenderAspect::Synchronous);
        Qt3DRender::Render::RenderQueue *renderQueue = renderer.renderQueue();
        Qt3DRender::Render::OffscreenSurfaceHelper offscreenHelper(&renderer);
        Qt3DRender::Render::RenderSettings settings;
        // owned by FG manager
        Qt3DRender::Render::ViewportNode *fgRoot = new Qt3DRender::Render::ViewportNode();
        const Qt3DCore::QNodeId fgRootId = Qt3DCore::QNodeId::createId();

        nodeManagers.frameGraphManager()->appendNode(fgRootId, fgRoot);
        settings.setActiveFrameGraphId(fgRootId);

        renderer.setNodeManagers(&nodeManagers);
        renderer.setSettings(&settings);
        renderer.setOffscreenSurfaceHelper(&offscreenHelper);
        renderer.initialize();

        // Ensure invoke calls are performed
        QCoreApplication::processEvents();

        // NOTE: FilterCompatibleTechniqueJob and ShaderGathererJob cannot run because the context
        // is not initialized in this test

        const int renderViewBuilderMaterialCacheJobCount = 1 + Qt3DRender::Render::RenderViewBuilder::optimalJobCount();
        // syncMaterialGathererJob
        // n * materialGathererJob
        const int layerCacheJobCount = 2;
        // filterEntityByLayerJob,
        // syncFilterEntityByLayerJob
        const int singleRenderViewJobCount = 8 + Qt3DRender::Render::RenderViewBuilder::optimalJobCount();
        // RenderViewBuilder renderViewJob,
        //                   syncRenderViewInitializationJob,
        //                   syncFrustumCullingJob,
        //                   filterProximityJob,
        //                   setClearDrawBufferIndexJob,
        //                   frustumCullingJob,
        //                   syncRenderCommandBuldingJob,
        //                   syncRenderViewCommandBuilderJob
        //                   n * (RenderViewCommandBuildJobs)

        // WHEN (nothing dirty, no buffers, no layers to be rebuilt, no materials to be rebuilt)
        QVector<Qt3DCore::QAspectJobPtr> jobs = renderer.renderBinJobs();

        // THEN (level
        QCOMPARE(jobs.size(),
                 1 + // updateLevelOfDetailJob
                 1 + // cleanupJob
                 1 + // sendBufferCaptureJob
                 1 + // VAOGatherer
                 1 + // updateSkinningPaletteJob
                 singleRenderViewJobCount); // Only valid for the first call to renderBinJobs(), since subsequent calls won't have the renderqueue reset

        renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
        renderQueue->reset();

        // WHEN
        renderer.addRenderCaptureSendRequest(Qt3DCore::QNodeId::createId());
        jobs = renderer.renderBinJobs();

        // THEN
        QCOMPARE(jobs.size(),
                 1 + // updateLevelOfDetailJob
                 1 + // cleanupJob
                 1 + // sendBufferCaptureJob
                 1 + // sendRenderCaptureJob
                 1 + // VAOGatherer
                 1 + // updateSkinningPaletteJob
                 singleRenderViewJobCount);

        renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
        renderQueue->reset();

        // WHEN
        renderer.markDirty(Qt3DRender::Render::AbstractRenderer::EntityEnabledDirty, nullptr);
        jobs = renderer.renderBinJobs();

        // THEN (level
        QCOMPARE(jobs.size(),
                 1 + // updateLevelOfDetailJob
                 1 + // cleanupJob
                 1 + // sendBufferCaptureJob
                 1 + // VAOGatherer
                 1 + // updateSkinningPaletteJob
                 1 + // EntityEnabledDirty
                 singleRenderViewJobCount +
                 layerCacheJobCount);

        renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
        renderQueue->reset();

        // WHEN
        renderer.markDirty(Qt3DRender::Render::AbstractRenderer::TransformDirty, nullptr);
        jobs = renderer.renderBinJobs();

        // THEN (level
        QCOMPARE(jobs.size(),
                 1 + // updateLevelOfDetailJob
                 1 + // cleanupJob
                 1 + // sendBufferCaptureJob
                 1 + // VAOGatherer
                 1 + // WorldTransformJob
                 1 + // UpdateWorldBoundingVolume
                 1 + // UpdateShaderDataTransform
                 1 + // updateSkinningPaletteJob
                 1 + // ExpandBoundingVolumeJob
                 singleRenderViewJobCount);

        renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
        renderQueue->reset();

        // WHEN
        renderer.markDirty(Qt3DRender::Render::AbstractRenderer::MaterialDirty, nullptr);
        jobs = renderer.renderBinJobs();

        // THEN (level
        QCOMPARE(jobs.size(),
                 1 + // updateLevelOfDetailJob
                 1 + // cleanupJob
                 1 + // VAOGatherer
                 1 + // updateSkinningPaletteJob
                 1 + // sendBufferCaptureJob
                 singleRenderViewJobCount +
                 renderViewBuilderMaterialCacheJobCount);

        renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
        renderQueue->reset();

        // WHEN
        renderer.markDirty(Qt3DRender::Render::AbstractRenderer::GeometryDirty, nullptr);
        jobs = renderer.renderBinJobs();

        // THEN (level
        QCOMPARE(jobs.size(),
                 1 + // updateLevelOfDetailJob
                 1 + // cleanupJob
                 1 + // sendBufferCaptureJob
                 1 + // VAOGatherer
                 1 + // CalculateBoundingVolumeJob
                 1 + // UpdateMeshTriangleListJob
                 1 + // updateSkinningPaletteJob
                 1 + // ExpandBoundingVolumeJob
                 1 + // RenderableEntityFilterPtr
                 1 + // SyncRenderableEntities
                 singleRenderViewJobCount);

        renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
        renderQueue->reset();

        // WHEN
        renderer.markDirty(Qt3DRender::Render::AbstractRenderer::BuffersDirty, nullptr);
        jobs = renderer.renderBinJobs();

        // THEN (level
        QCOMPARE(jobs.size(),
                 1 + // updateLevelOfDetailJob
                 1 + // cleanupJob
                 1 + // sendBufferCaptureJob
                 1 + // VAOGatherer
                 1 + // updateSkinningPaletteJob
                 1 + // CalculateBoundingVolumeJob
                 1 + // UpdateMeshTriangleListJob
                 1 + // BufferGathererJob
                 singleRenderViewJobCount);

        renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
        renderQueue->reset();

        // WHEN
        renderer.markDirty(Qt3DRender::Render::AbstractRenderer::TexturesDirty, nullptr);
        jobs = renderer.renderBinJobs();

        // THEN (level
        QCOMPARE(jobs.size(),
                 1 + // updateLevelOfDetailJob
                 1 + // cleanupJob
                 1 + // sendBufferCaptureJob
                 1 + // VAOGatherer
                 1 + // TexturesGathererJob
                 1 + // updateSkinningPaletteJob
                 1 + // SyncTexturesGathererJob
                 singleRenderViewJobCount);

        renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
        renderQueue->reset();

        // WHEN
        renderer.markDirty(Qt3DRender::Render::AbstractRenderer::FrameGraphDirty, nullptr);
        jobs = renderer.renderBinJobs();

        QCOMPARE(jobs.size(),
                 1 + // updateLevelOfDetailJob
                 1 + // cleanupJob
                 1 + // VAOGatherer
                 1 + // sendBufferCaptureJob
                 1 + // updateSkinningPaletteJob
                 singleRenderViewJobCount +
                 layerCacheJobCount +
                 renderViewBuilderMaterialCacheJobCount);

        renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
        renderQueue->reset();

        // WHEN
        renderer.markDirty(Qt3DRender::Render::AbstractRenderer::EntityHierarchyDirty, nullptr);
        jobs = renderer.renderBinJobs();

        // THEN
        QCOMPARE(jobs.size(),
                 1 + // EntityEnabledDirty
                 1 + // EntityHierarchyJob
                 1 + // WorldTransformJob
                 1 + // UpdateWorldBoundingVolume
                 1 + // UpdateShaderDataTransform
                 1 + // ExpandBoundingVolumeJob
                 1 + // CalculateBoundingVolumeJob
                 1 + // UpdateEntityLayersJob
                 1 + // updateLevelOfDetailJob
                 1 + // updateSkinningPaletteJob
                 1 + // cleanupJob
                 1 + // sendBufferCaptureJob
                 singleRenderViewJobCount +
                 layerCacheJobCount);

        renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
        renderQueue->reset();

        // WHEN
        renderer.markDirty(Qt3DRender::Render::AbstractRenderer::AllDirty, nullptr);
        jobs = renderer.renderBinJobs();

        // THEN (Renderer is not initialized so FilterCompatibleTechniqueJob
        // and ShaderGathererJob are not added here)
        QCOMPARE(jobs.size(),
                 1 + // EntityEnabledDirty
                 1 + // EntityHierarchyDirty
                 1 + // WorldTransformJob
                 1 + // UpdateWorldBoundingVolume
                 1 + // UpdateShaderDataTransform
                 1 + // ExpandBoundingVolumeJob
                 1 + // CalculateBoundingVolumeJob
                 1 + // UpdateMeshTriangleListJob
                 1 + // updateSkinningPaletteJob
                 1 + // updateLevelOfDetailJob
                 1 + // cleanupJob
                 1 + // sendBufferCaptureJob
                 1 + // VAOGatherer
                 1 + // BufferGathererJob
                 1 + // TexturesGathererJob
                 1 + // SyncTextureLoadingJob
                 1 + // UpdateEntityLayersJob
                 1 + // RenderableEntityFilterPtr
                 1 + // SyncRenderableEntities
                 1 + // ComputableEntityFilterPtr
                 1 + // SyncComputableEntities
                 1 + // LightGathererPtr
                 1 + // SyncLightsGatherer
                 singleRenderViewJobCount +
                 layerCacheJobCount +
                 renderViewBuilderMaterialCacheJobCount);

        renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
        renderQueue->reset();

        // Properly shutdown command thread
        renderer.shutdown();
    }
};

QTEST_MAIN(tst_Renderer)

#include "tst_renderer.moc"
