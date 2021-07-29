/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
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

#ifndef TESTRENDERER_H
#define TESTRENDERER_H

#include <Qt3DRender/private/abstractrenderer_p.h>

QT_BEGIN_NAMESPACE

class TestRenderer : public Qt3DRender::Render::AbstractRenderer
{
public:
    TestRenderer();
    ~TestRenderer();

    void dumpInfo() const override {}
    API api() const override { return AbstractRenderer::OpenGL; }
    qint64 time() const override { return 0; }
    void setTime(qint64 time) override { Q_UNUSED(time); }
    void setNodeManagers(Qt3DRender::Render::NodeManagers *m) override { m_managers = m; }
    void setServices(Qt3DCore::QServiceLocator *services) override { Q_UNUSED(services); }
    void setSurfaceExposed(bool exposed) override { Q_UNUSED(exposed); }
    Qt3DRender::Render::NodeManagers *nodeManagers() const override { return m_managers; }
    Qt3DCore::QServiceLocator *services() const override { return nullptr; }
    void initialize() override {}
    void shutdown() override {}
    void releaseGraphicsResources() override {}
    void render() override {}
    void doRender(bool scene3dBlocking = false) override { Q_UNUSED(scene3dBlocking); }
    void cleanGraphicsResources() override {}
    bool isRunning() const override { return true; }
    bool shouldRender() override { return true; }
    void skipNextFrame() override {}
    QVector<Qt3DCore::QAspectJobPtr> renderBinJobs() override { return QVector<Qt3DCore::QAspectJobPtr>(); }
    Qt3DCore::QAspectJobPtr pickBoundingVolumeJob() override { return Qt3DCore::QAspectJobPtr(); }
    Qt3DCore::QAspectJobPtr rayCastingJob() override { return Qt3DCore::QAspectJobPtr(); }
    Qt3DCore::QAspectJobPtr syncTextureLoadingJob() override { return Qt3DCore::QAspectJobPtr(); }
    Qt3DCore::QAspectJobPtr expandBoundingVolumeJob() override { return Qt3DCore::QAspectJobPtr(); }
    void setSceneRoot(Qt3DCore::QBackendNodeFactory *factory, Qt3DRender::Render::Entity *root) override { Q_UNUSED(factory);  Q_UNUSED(root); }
    Qt3DRender::Render::Entity *sceneRoot() const override { return nullptr; }
    Qt3DRender::Render::FrameGraphNode *frameGraphRoot() const override { return nullptr; }
    Qt3DCore::QAbstractFrameAdvanceService *frameAdvanceService() const override { return nullptr; }
    void registerEventFilter(Qt3DCore::QEventFilterService *service) override { Q_UNUSED(service); }
    void setSettings(Qt3DRender::Render::RenderSettings *settings) override { Q_UNUSED(settings); }
    Qt3DRender::Render::RenderSettings *settings() const override { return nullptr; }

    void markDirty(Qt3DRender::Render::AbstractRenderer::BackendNodeDirtySet changes, Qt3DRender::Render::BackendNode *node) override;
    Qt3DRender::Render::AbstractRenderer::BackendNodeDirtySet dirtyBits() override;
#if defined(QT_BUILD_INTERNAL)
    void clearDirtyBits(Qt3DRender::Render::AbstractRenderer::BackendNodeDirtySet changes) override;
#endif

    void resetDirty();
    QVariant executeCommand(const QStringList &args) override;
    QOpenGLContext *shareContext() const override;

    void setOffscreenSurfaceHelper(Qt3DRender::Render::OffscreenSurfaceHelper *helper) override;
    QSurfaceFormat format() override;

    void setOpenGLContext(QOpenGLContext *) override {}

    void loadShader(Qt3DRender::Render::Shader *, Qt3DRender::Render::HShader) override {}

protected:
    Qt3DRender::Render::AbstractRenderer::BackendNodeDirtySet m_changes;
    Qt3DRender::Render::NodeManagers *m_managers;
};

QT_END_NAMESPACE

#endif // TESTRENDERER_H
