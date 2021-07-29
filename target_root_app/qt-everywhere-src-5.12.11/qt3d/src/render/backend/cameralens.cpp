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

#include "cameralens_p.h"
#include <Qt3DRender/qcameralens.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/qcameralens_p.h>
#include <Qt3DRender/private/renderlogging_p.h>
#include <Qt3DRender/private/renderer_p.h>
#include <Qt3DRender/private/entity_p.h>
#include <Qt3DRender/private/sphere_p.h>
#include <Qt3DRender/private/computefilteredboundingvolumejob_p.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qtransform.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DRender {
namespace Render {


namespace {

class GetBoundingVolumeWithoutCameraJob : public ComputeFilteredBoundingVolumeJob
{
public:
    GetBoundingVolumeWithoutCameraJob(CameraLens *lens,
                                      QNodeCommand::CommandId commandId)
        : m_lens(lens), m_commandId(commandId)
    {
    }

protected:
    void finished(const Sphere &sphere) override
    {
        m_lens->notifySceneBoundingVolume(sphere, m_commandId);
    }

private:
    CameraLens *m_lens;
    QNodeCommand::CommandId m_commandId;
};

} // namespace

CameraLens::CameraLens()
    : BackendNode(QBackendNode::ReadWrite)
    , m_renderAspect(nullptr)
    , m_exposure(0.0f)
{
}

CameraLens::~CameraLens()
{
    cleanup();
}

void CameraLens::cleanup()
{
    QBackendNode::setEnabled(false);
}

void CameraLens::setRenderAspect(QRenderAspect *renderAspect)
{
    m_renderAspect = renderAspect;
}

Matrix4x4 CameraLens::viewMatrix(const Matrix4x4 &worldTransform)
{
    const Vector4D position = worldTransform * Vector4D(0.0f, 0.0f, 0.0f, 1.0f);
    // OpenGL convention is looking down -Z
    const Vector4D viewDirection = worldTransform * Vector4D(0.0f, 0.0f, -1.0f, 0.0f);
    const Vector4D upVector = worldTransform * Vector4D(0.0f, 1.0f, 0.0f, 0.0f);

    QMatrix4x4 m;
    m.lookAt(convertToQVector3D(Vector3D(position)),
             convertToQVector3D(Vector3D(position + viewDirection)),
             convertToQVector3D(Vector3D(upVector)));
    return Matrix4x4(m);
}

void CameraLens::initializeFromPeer(const Qt3DCore::QNodeCreatedChangeBasePtr &change)
{
    const auto typedChange = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<QCameraLensData>>(change);
    const auto &data = typedChange->data;
    m_projection = Matrix4x4(data.projectionMatrix);
    m_exposure = data.exposure;
}

void CameraLens::computeSceneBoundingVolume(QNodeId entityId,
                                            QNodeId cameraId,
                                            QNodeCommand::CommandId commandId)
{
    if (!m_renderer || !m_renderAspect)
        return;
    NodeManagers *nodeManagers = m_renderer->nodeManagers();

    Entity *root = m_renderer->sceneRoot();
    if (!entityId.isNull())
        root = nodeManagers->renderNodesManager()->lookupResource(entityId);
    if (!root)
        return;

    Entity *camNode = nodeManagers->renderNodesManager()->lookupResource(cameraId);
    ComputeFilteredBoundingVolumeJobPtr job(new GetBoundingVolumeWithoutCameraJob(this, commandId));
    job->addDependency(m_renderer->expandBoundingVolumeJob());
    job->setRoot(root);
    job->ignoreSubTree(camNode);
    m_renderAspect->scheduleSingleShotJob(job);
}

void CameraLens::notifySceneBoundingVolume(const Sphere &sphere, QNodeCommand::CommandId commandId)
{
    if (m_pendingViewAllCommand != commandId)
        return;
    if (sphere.radius() > 0.f) {
        QVector<float> data = { sphere.center().x(), sphere.center().y(), sphere.center().z(),
                                sphere.radius() };
        QVariant v;
        v.setValue(data);
        sendCommand(QLatin1Literal("ViewAll"), v, m_pendingViewAllCommand);
    }
}

void CameraLens::setProjection(const Matrix4x4 &projection)
{
    m_projection = projection;
}

void CameraLens::setExposure(float exposure)
{
    m_exposure = exposure;
}

void CameraLens::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &e)
{
    switch (e->type()) {
    case PropertyUpdated: {
        QPropertyUpdatedChangePtr propertyChange = qSharedPointerCast<QPropertyUpdatedChange>(e);

        if (propertyChange->propertyName() == QByteArrayLiteral("projectionMatrix")) {
            QMatrix4x4 projectionMatrix = propertyChange->value().value<QMatrix4x4>();
            m_projection = Matrix4x4(projectionMatrix);
        } else if (propertyChange->propertyName() == QByteArrayLiteral("exposure")) {
            setExposure(propertyChange->value().toFloat());
        }

        markDirty(AbstractRenderer::AllDirty);
    }
        break;

    case CommandRequested: {
        QNodeCommandPtr command = qSharedPointerCast<QNodeCommand>(e);

        if (command->name() == QLatin1Literal("QueryRootBoundingVolume")) {
            m_pendingViewAllCommand = command->commandId();
            QVariant v = command->data();
            QNodeId id = v.value<QNodeId>();
            computeSceneBoundingVolume({}, id, command->commandId());
        } else if (command->name() == QLatin1Literal("QueryEntityBoundingVolume")) {
            m_pendingViewAllCommand = command->commandId();
            QVariant v = command->data();
            QVector<QNodeId> ids = v.value<QVector<QNodeId>>();
            if (ids.size() == 2)
                computeSceneBoundingVolume(ids[0], ids[1], command->commandId());
        }
    }
        break;

    default:
        break;
    }
    BackendNode::sceneChangeEvent(e);
}

bool CameraLens::viewMatrixForCamera(EntityManager* manager, Qt3DCore::QNodeId cameraId,
                                     Matrix4x4 &viewMatrix, Matrix4x4 &projectionMatrix)
{
    Entity *camNode = manager->lookupResource(cameraId);
    if (!camNode)
        return false;
    Render::CameraLens *lens = camNode->renderComponent<CameraLens>();
    if (!lens || !lens->isEnabled())
        return false;

    viewMatrix = lens->viewMatrix(*camNode->worldTransform());
    projectionMatrix = lens->projection();
    return true;
}

CameraLensFunctor::CameraLensFunctor(AbstractRenderer *renderer, QRenderAspect *renderAspect)
    : m_manager(renderer->nodeManagers()->manager<CameraLens, CameraManager>())
    , m_renderer(renderer)
    , m_renderAspect(renderAspect)
{
}

QBackendNode *CameraLensFunctor::create(const QNodeCreatedChangeBasePtr &change) const
{
    CameraLens *backend = m_manager->getOrCreateResource(change->subjectId());
    backend->setRenderer(m_renderer);
    backend->setRenderAspect(m_renderAspect);
    return backend;
}

QBackendNode *CameraLensFunctor::get(QNodeId id) const
{
    return m_manager->lookupResource(id);
}

void CameraLensFunctor::destroy(QNodeId id) const
{
    m_manager->releaseResource(id);
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
