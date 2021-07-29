/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Copyright (C) 2016 The Qt Company Ltd and/or its subsidiary(-ies).
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

#include "renderer_p.h"

#include <Qt3DCore/qentity.h>

#include <Qt3DRender/qmaterial.h>
#include <Qt3DRender/qmesh.h>
#include <Qt3DRender/qrenderpass.h>
#include <Qt3DRender/qshaderprogram.h>
#include <Qt3DRender/qtechnique.h>
#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DRender/qeffect.h>

#include <Qt3DRender/private/qsceneimporter_p.h>
#include <Qt3DRender/private/renderstates_p.h>
#include <Qt3DRender/private/cameraselectornode_p.h>
#include <Qt3DRender/private/framegraphvisitor_p.h>
#include <Qt3DRender/private/graphicscontext_p.h>
#include <Qt3DRender/private/cameralens_p.h>
#include <Qt3DRender/private/rendercommand_p.h>
#include <Qt3DRender/private/entity_p.h>
#include <Qt3DRender/private/renderlogging_p.h>
#include <Qt3DRender/private/material_p.h>
#include <Qt3DRender/private/renderpassfilternode_p.h>
#include <Qt3DRender/private/renderqueue_p.h>
#include <Qt3DRender/private/shader_p.h>
#include <Qt3DRender/private/buffer_p.h>
#include <Qt3DRender/private/glbuffer_p.h>
#include <Qt3DRender/private/renderstateset_p.h>
#include <Qt3DRender/private/technique_p.h>
#include <Qt3DRender/private/renderthread_p.h>
#include <Qt3DRender/private/renderview_p.h>
#include <Qt3DRender/private/scenemanager_p.h>
#include <Qt3DRender/private/techniquefilternode_p.h>
#include <Qt3DRender/private/viewportnode_p.h>
#include <Qt3DRender/private/vsyncframeadvanceservice_p.h>
#include <Qt3DRender/private/pickeventfilter_p.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/buffermanager_p.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/gltexturemanager_p.h>
#include <Qt3DRender/private/gltexture_p.h>
#include <Qt3DRender/private/geometryrenderermanager_p.h>
#include <Qt3DRender/private/techniquemanager_p.h>
#include <Qt3DRender/private/openglvertexarrayobject_p.h>
#include <Qt3DRender/private/platformsurfacefilter_p.h>
#include <Qt3DRender/private/loadbufferjob_p.h>
#include <Qt3DRender/private/rendercapture_p.h>
#include <Qt3DRender/private/updatelevelofdetailjob_p.h>
#include <Qt3DRender/private/buffercapture_p.h>
#include <Qt3DRender/private/offscreensurfacehelper_p.h>
#include <Qt3DRender/private/renderviewbuilder_p.h>
#include <Qt3DRender/private/commandthread_p.h>
#include <Qt3DRender/private/glcommands_p.h>

#include <Qt3DRender/qcameralens.h>
#include <Qt3DCore/private/qeventfilterservice_p.h>
#include <Qt3DCore/private/qabstractaspectjobmanager_p.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>

#if QT_CONFIG(qt3d_profile_jobs)
#include <Qt3DCore/private/aspectcommanddebugger_p.h>
#endif

#include <QStack>
#include <QOffscreenSurface>
#include <QSurface>
#include <QElapsedTimer>
#include <QLibraryInfo>
#include <QMutexLocker>
#include <QPluginLoader>
#include <QDir>
#include <QUrl>
#include <QOffscreenSurface>
#include <QWindow>

#include <QtGui/private/qopenglcontext_p.h>

// For Debug purposes only
#include <QThread>


#if QT_CONFIG(qt3d_profile_jobs)
#include <Qt3DCore/private/qthreadpooler_p.h>
#include <Qt3DRender/private/job_common_p.h>
#include <Qt3DRender/private/commandexecuter_p.h>
#endif

#include <Qt3DRender/private/frameprofiler_p.h>

QT_BEGIN_NAMESPACE

// Crashes on AMD Radeon drivers on Windows. Disable for now.
//#define SHADER_LOADING_IN_COMMAND_THREAD

using namespace Qt3DCore;

namespace Qt3DRender {
namespace Render {
/*!
    \internal

    Renderer shutdown procedure:

    Since the renderer relies on the surface and OpenGLContext to perform its cleanup,
    it is shutdown when the surface is set to nullptr

    When the surface is set to nullptr this will request the RenderThread to terminate
    and will prevent createRenderBinJobs from returning a set of jobs as there is nothing
    more to be rendered.

    In turn, this will call shutdown which will make the OpenGL context current one last time
    to allow cleanups requiring a call to QOpenGLContext::currentContext to execute properly.
    At the end of that function, the GraphicsContext is set to null.

    At this point though, the QAspectThread is still running its event loop and will only stop
    a short while after.
 */

Renderer::Renderer(QRenderAspect::RenderType type)
    : m_services(nullptr)
    , m_nodesManager(nullptr)
    , m_renderSceneRoot(nullptr)
    , m_defaultRenderStateSet(nullptr)
    , m_submissionContext(nullptr)
    , m_renderQueue(new RenderQueue())
    , m_renderThread(type == QRenderAspect::Threaded ? new RenderThread(this) : nullptr)
    , m_commandThread(new CommandThread(this))
    , m_vsyncFrameAdvanceService(new VSyncFrameAdvanceService(m_renderThread != nullptr))
    , m_waitForInitializationToBeCompleted(0)
    , m_hasBeenInitializedMutex()
    , m_pickEventFilter(new PickEventFilter())
    , m_exposed(0)
    , m_lastFrameCorrect(0)
    , m_glContext(nullptr)
    , m_shareContext(nullptr)
    , m_shaderCache(new ShaderCache())
    , m_pickBoundingVolumeJob(PickBoundingVolumeJobPtr::create())
    , m_rayCastingJob(RayCastingJobPtr::create())
    , m_time(0)
    , m_settings(nullptr)
    , m_updateShaderDataTransformJob(Render::UpdateShaderDataTransformJobPtr::create())
    , m_cleanupJob(Render::FrameCleanupJobPtr::create())
    , m_worldTransformJob(Render::UpdateWorldTransformJobPtr::create())
    , m_expandBoundingVolumeJob(Render::ExpandBoundingVolumeJobPtr::create())
    , m_calculateBoundingVolumeJob(Render::CalculateBoundingVolumeJobPtr::create())
    , m_updateWorldBoundingVolumeJob(Render::UpdateWorldBoundingVolumeJobPtr::create())
    , m_updateTreeEnabledJob(Render::UpdateTreeEnabledJobPtr::create())
    , m_sendRenderCaptureJob(Render::SendRenderCaptureJobPtr::create())
    , m_sendBufferCaptureJob(Render::SendBufferCaptureJobPtr::create())
    , m_updateSkinningPaletteJob(Render::UpdateSkinningPaletteJobPtr::create())
    , m_updateLevelOfDetailJob(Render::UpdateLevelOfDetailJobPtr::create())
    , m_updateMeshTriangleListJob(Render::UpdateMeshTriangleListJobPtr::create())
    , m_filterCompatibleTechniqueJob(Render::FilterCompatibleTechniqueJobPtr::create())
    , m_updateEntityLayersJob(Render::UpdateEntityLayersJobPtr::create())
    , m_updateEntityHierarchyJob(Render::UpdateEntityHierarchyJobPtr::create())
    , m_bufferGathererJob(Render::GenericLambdaJobPtr<std::function<void ()>>::create([this] { lookForDirtyBuffers(); }, JobTypes::DirtyBufferGathering))
    , m_vaoGathererJob(Render::GenericLambdaJobPtr<std::function<void ()>>::create([this] { lookForAbandonedVaos(); }, JobTypes::DirtyVaoGathering))
    , m_textureGathererJob(Render::GenericLambdaJobPtr<std::function<void ()>>::create([this] { lookForDirtyTextures(); }, JobTypes::DirtyTextureGathering))
    , m_sendTextureChangesToFrontendJob(Render::GenericLambdaJobPtr<std::function<void ()>>::create([this] { sendTextureChangesToFrontend(); }, JobTypes::SendTextureChangesToFrontend))
    , m_introspectShaderJob(Render::GenericLambdaJobPtr<std::function<void ()>>::create([this] { reloadDirtyShaders(); }, JobTypes::DirtyShaderGathering))
    , m_syncTextureLoadingJob(Render::GenericLambdaJobPtr<std::function<void ()>>::create([] {}, JobTypes::SyncTextureLoading))
    , m_ownedContext(false)
    , m_offscreenHelper(nullptr)
    #if QT_CONFIG(qt3d_profile_jobs)
    , m_commandExecuter(new Qt3DRender::Debug::CommandExecuter(this))
    #endif
{
    // Set renderer as running - it will wait in the context of the
    // RenderThread for RenderViews to be submitted
    m_running.fetchAndStoreOrdered(1);
    if (m_renderThread)
        m_renderThread->waitForStart();

    m_worldTransformJob->addDependency(m_updateEntityHierarchyJob);
    m_updateEntityLayersJob->addDependency(m_updateEntityHierarchyJob);

    // Create jobs to update transforms and bounding volumes
    // We can only update bounding volumes once all world transforms are known
    m_updateWorldBoundingVolumeJob->addDependency(m_worldTransformJob);
    m_updateWorldBoundingVolumeJob->addDependency(m_calculateBoundingVolumeJob);
    m_expandBoundingVolumeJob->addDependency(m_updateWorldBoundingVolumeJob);
    m_updateShaderDataTransformJob->addDependency(m_worldTransformJob);
    m_pickBoundingVolumeJob->addDependency(m_expandBoundingVolumeJob);
    m_rayCastingJob->addDependency(m_expandBoundingVolumeJob);
    // m_calculateBoundingVolumeJob's dependency on m_updateTreeEnabledJob is set in renderBinJobs

    // Dirty texture gathering depends on m_syncTextureLoadingJob
    // m_syncTextureLoadingJob will depend on the texture loading jobs
    m_textureGathererJob->addDependency(m_syncTextureLoadingJob);

    // Ensures all skeletons are loaded before we try to update them
    m_updateSkinningPaletteJob->addDependency(m_syncTextureLoadingJob);

    // All world stuff depends on the RenderEntity's localBoundingVolume
    m_updateLevelOfDetailJob->addDependency(m_updateMeshTriangleListJob);
    m_pickBoundingVolumeJob->addDependency(m_updateMeshTriangleListJob);
    m_rayCastingJob->addDependency(m_updateMeshTriangleListJob);

    m_introspectShaderJob->addDependency(m_filterCompatibleTechniqueJob);

    m_filterCompatibleTechniqueJob->setRenderer(this);

    m_defaultRenderStateSet = new RenderStateSet;
    m_defaultRenderStateSet->addState(StateVariant::createState<DepthTest>(GL_LESS));
    m_defaultRenderStateSet->addState(StateVariant::createState<CullFace>(GL_BACK));
    m_defaultRenderStateSet->addState(StateVariant::createState<ColorMask>(true, true, true, true));
}

Renderer::~Renderer()
{
    Q_ASSERT(m_running.fetchAndStoreOrdered(0) == 0);
    if (m_renderThread)
        Q_ASSERT(m_renderThread->isFinished());

    delete m_renderQueue;
    delete m_defaultRenderStateSet;
    delete m_shaderCache;

    if (!m_ownedContext)
        QObject::disconnect(m_contextConnection);
}

void Renderer::dumpInfo() const
{
    qDebug() << Q_FUNC_INFO << "t =" << m_time;

    const ShaderManager *shaderManager = m_nodesManager->shaderManager();
    qDebug() << "=== Shader Manager ===";
    qDebug() << *shaderManager;

    const TextureManager *textureManager = m_nodesManager->textureManager();
    qDebug() << "=== Texture Manager ===";
    qDebug() << *textureManager;

    const TextureImageManager *textureImageManager = m_nodesManager->textureImageManager();
    qDebug() << "=== Texture Image Manager ===";
    qDebug() << *textureImageManager;
}

qint64 Renderer::time() const
{
    return m_time;
}

void Renderer::setTime(qint64 time)
{
    m_time = time;
}

void Renderer::setNodeManagers(NodeManagers *managers)
{
    m_nodesManager = managers;

    m_updateShaderDataTransformJob->setManagers(m_nodesManager);
    m_cleanupJob->setManagers(m_nodesManager);
    m_calculateBoundingVolumeJob->setManagers(m_nodesManager);
    m_pickBoundingVolumeJob->setManagers(m_nodesManager);
    m_rayCastingJob->setManagers(m_nodesManager);
    m_updateWorldBoundingVolumeJob->setManager(m_nodesManager->renderNodesManager());
    m_sendRenderCaptureJob->setManagers(m_nodesManager);
    m_updateLevelOfDetailJob->setManagers(m_nodesManager);
    m_updateSkinningPaletteJob->setManagers(m_nodesManager);
    m_updateMeshTriangleListJob->setManagers(m_nodesManager);
    m_filterCompatibleTechniqueJob->setManager(m_nodesManager->techniqueManager());
    m_updateEntityLayersJob->setManager(m_nodesManager);
    m_updateEntityHierarchyJob->setManager(m_nodesManager);
}

void Renderer::setServices(QServiceLocator *services)
{
    m_services = services;

    m_nodesManager->sceneManager()->setDownloadService(m_services->downloadHelperService());
}

NodeManagers *Renderer::nodeManagers() const
{
    return m_nodesManager;
}

/*!
    \internal

    Return context which can be used to share resources safely
    with qt3d main render context.
*/
QOpenGLContext *Renderer::shareContext() const
{
    QMutexLocker lock(&m_shareContextMutex);
    return m_shareContext ? m_shareContext
                          : (m_submissionContext->openGLContext()
                             ? m_submissionContext->openGLContext()->shareContext()
                             : nullptr);
}

// Executed in the command thread
void Renderer::loadShader(Shader *shader, HShader shaderHandle)
{
#ifdef SHADER_LOADING_IN_COMMAND_THREAD
    Q_UNUSED(shaderHandle);
    Profiling::GLTimeRecorder recorder(Profiling::ShaderUpload);
    LoadShaderCommand cmd(shader);
    m_commandThread->executeCommand(&cmd);
#else
    Q_UNUSED(shader);
    m_dirtyShaders.push_back(shaderHandle);
#endif
}

void Renderer::setOpenGLContext(QOpenGLContext *context)
{
    m_glContext = context;
}

void Renderer::setScreen(QScreen *scr)
{
    m_screen = scr;
}

QScreen *Renderer::screen() const
{
    return m_screen;
}

// Called in RenderThread context by the run method of RenderThread
// RenderThread has locked the mutex already and unlocks it when this
// method termintates
void Renderer::initialize()
{
    QMutexLocker lock(&m_hasBeenInitializedMutex);
    m_submissionContext.reset(new SubmissionContext);
    m_submissionContext->setRenderer(this);

    QOpenGLContext* ctx = m_glContext;

    {
        QMutexLocker lock(&m_shareContextMutex);
        // If we are using our own context (not provided by QtQuick),
        // we need to create it
        if (!m_glContext) {
            ctx = new QOpenGLContext;
            if (m_screen)
                ctx->setScreen(m_screen);
            ctx->setShareContext(qt_gl_global_share_context());

            // TO DO: Shouldn't we use the highest context available and trust
            // QOpenGLContext to fall back on the best lowest supported ?
            const QByteArray debugLoggingMode = qgetenv("QT3DRENDER_DEBUG_LOGGING");

            if (!debugLoggingMode.isEmpty()) {
                QSurfaceFormat sf = ctx->format();
                sf.setOption(QSurfaceFormat::DebugContext);
                ctx->setFormat(sf);
            }

            // Create OpenGL context
            if (ctx->create())
                qCDebug(Backend) << "OpenGL context created with actual format" << ctx->format();
            else
                qCWarning(Backend) << Q_FUNC_INFO << "OpenGL context creation failed";
            m_ownedContext = true;
        } else {
            // Context is not owned by us, so we need to know if it gets destroyed
            m_contextConnection = QObject::connect(m_glContext, &QOpenGLContext::aboutToBeDestroyed,
                                                   [this] { releaseGraphicsResources(); });
        }

        if (!ctx->shareContext()) {
            m_shareContext = new QOpenGLContext;
            if (ctx->screen())
                m_shareContext->setScreen(ctx->screen());
            m_shareContext->setFormat(ctx->format());
            m_shareContext->setShareContext(ctx);
            m_shareContext->create();
        }

        // Set shader cache on submission context and command thread
        m_submissionContext->setShaderCache(m_shaderCache);
        m_commandThread->setShaderCache(m_shaderCache);

        // Note: we don't have a surface at this point
        // The context will be made current later on (at render time)
        m_submissionContext->setOpenGLContext(ctx);

        // Store the format used by the context and queue up creating an
        // offscreen surface in the main thread so that it is available
        // for use when we want to shutdown the renderer. We need to create
        // the offscreen surface on the main thread because on some platforms
        // (MS Windows), an offscreen surface is just a hidden QWindow.
        m_format = ctx->format();
        QMetaObject::invokeMethod(m_offscreenHelper, "createOffscreenSurface");

        // Initialize command thread (uses the offscreen surface to make its own ctx current)
        m_commandThread->initialize(ctx, m_offscreenHelper);
        // Note: the offscreen surface is also used at shutdown time to release resources
        // of the submission gl context (when the window is already gone).
        // By that time (in releaseGraphicResources), the commandThread has been destroyed
        // and the offscreenSurface can be reused
    }

    // Awake setScenegraphRoot in case it was waiting
    m_waitForInitializationToBeCompleted.release(1);
    // Allow the aspect manager to proceed
    m_vsyncFrameAdvanceService->proceedToNextFrame();
}

/*!
 * \internal
 *
 * Signals for the renderer to stop rendering. If a threaded renderer is in use,
 * the render thread will call releaseGraphicsResources() just before the thread exits.
 * If rendering synchronously, this function will call releaseGraphicsResources().
 */
void Renderer::shutdown()
{
    // Ensure we have waited to be fully initialized before trying to shut down
    // (in case initialization is taking place at the same time)
    QMutexLocker lock(&m_hasBeenInitializedMutex);

    qCDebug(Backend) << Q_FUNC_INFO << "Requesting renderer shutdown";
    m_running.store(0);

    // We delete any renderqueue that we may not have had time to render
    // before the surface was destroyed
    QMutexLocker lockRenderQueue(m_renderQueue->mutex());
    qDeleteAll(m_renderQueue->nextFrameQueue());
    m_renderQueue->reset();
    lockRenderQueue.unlock();

    m_commandThread->shutdown();

    if (!m_renderThread) {
        releaseGraphicsResources();
    } else {
        // Wake up the render thread in case it is waiting for some renderviews
        // to be ready. The isReadyToSubmit() function checks for a shutdown
        // having been requested.
        m_submitRenderViewsSemaphore.release(1);
        m_renderThread->wait();
    }
}

/*!
    \internal

    When using a threaded renderer this function is called in the context of the
    RenderThread to do any shutdown and cleanup that needs to be performed in the
    thread where the OpenGL context lives.

    When using Scene3D or anything that provides a custom QOpenGLContext (not
    owned by Qt3D) this function is called whenever the signal
    QOpenGLContext::aboutToBeDestroyed is emitted. In that case this function
    is called in the context of the emitter's thread.
*/
void Renderer::releaseGraphicsResources()
{
    // We may get called twice when running inside of a Scene3D. Once when Qt Quick
    // wants to shutdown, and again when the render aspect gets unregistered. So
    // check that we haven't already cleaned up before going any further.
    if (!m_submissionContext)
        return;

    // Try to temporarily make the context current so we can free up any resources
    QMutexLocker locker(&m_offscreenSurfaceMutex);
    QOffscreenSurface *offscreenSurface = m_offscreenHelper->offscreenSurface();
    if (!offscreenSurface) {
        qWarning() << "Failed to make context current: OpenGL resources will not be destroyed";
        // We still need to delete the submission context
        m_submissionContext.reset(nullptr);
        return;
    }

    QOpenGLContext *context = m_submissionContext->openGLContext();
    Q_ASSERT(context);

    if (context->thread() == QThread::currentThread() && context->makeCurrent(offscreenSurface)) {

        // Clean up the graphics context and any resources
        const QVector<GLTexture*> activeTextures = m_nodesManager->glTextureManager()->activeResources();
        for (GLTexture *tex : activeTextures)
            tex->destroyGLTexture();

        // Do the same thing with buffers
        const QVector<HGLBuffer> activeBuffers = m_nodesManager->glBufferManager()->activeHandles();
        for (const HGLBuffer &bufferHandle : activeBuffers) {
            GLBuffer *buffer = m_nodesManager->glBufferManager()->data(bufferHandle);
            buffer->destroy(m_submissionContext.data());
        }

        // Do the same thing with VAOs
        const QVector<HVao> activeVaos = m_nodesManager->vaoManager()->activeHandles();
        for (const HVao &vaoHandle : activeVaos) {
            OpenGLVertexArrayObject *vao = m_nodesManager->vaoManager()->data(vaoHandle);
            vao->destroy();
        }

        m_submissionContext->releaseRenderTargets();

        context->doneCurrent();
    } else {
        qWarning() << "Failed to make context current: OpenGL resources will not be destroyed";
    }

    if (m_ownedContext)
        delete context;
    if (m_shareContext)
        delete m_shareContext;

    m_submissionContext.reset(nullptr);
    qCDebug(Backend) << Q_FUNC_INFO << "Renderer properly shutdown";
}

void Renderer::setSurfaceExposed(bool exposed)
{
    qCDebug(Backend) << "Window exposed: " << exposed;
    m_exposed.fetchAndStoreOrdered(exposed);
}

Render::FrameGraphNode *Renderer::frameGraphRoot() const
{
    Q_ASSERT(m_settings);
    if (m_nodesManager && m_nodesManager->frameGraphManager() && m_settings)
        return m_nodesManager->frameGraphManager()->lookupNode(m_settings->activeFrameGraphID());
    return nullptr;
}

// QAspectThread context
// Order of execution :
// 1) RenderThread is created -> release 1 of m_waitForInitializationToBeCompleted when started
// 2) setSceneRoot waits to acquire initialization
// 3) submitRenderView -> check for surface
//    -> make surface current + create proper glHelper if needed
void Renderer::setSceneRoot(QBackendNodeFactory *factory, Entity *sgRoot)
{
    Q_ASSERT(sgRoot);
    Q_UNUSED(factory);

    // If initialization hasn't been completed we must wait
    m_waitForInitializationToBeCompleted.acquire();

    m_renderSceneRoot = sgRoot;
    if (!m_renderSceneRoot)
        qCWarning(Backend) << "Failed to build render scene";
    m_renderSceneRoot->dump();
    qCDebug(Backend) << Q_FUNC_INFO << "DUMPING SCENE";

    // Set the scene root on the jobs
    m_worldTransformJob->setRoot(m_renderSceneRoot);
    m_expandBoundingVolumeJob->setRoot(m_renderSceneRoot);
    m_calculateBoundingVolumeJob->setRoot(m_renderSceneRoot);
    m_cleanupJob->setRoot(m_renderSceneRoot);
    m_pickBoundingVolumeJob->setRoot(m_renderSceneRoot);
    m_rayCastingJob->setRoot(m_renderSceneRoot);
    m_updateLevelOfDetailJob->setRoot(m_renderSceneRoot);
    m_updateSkinningPaletteJob->setRoot(m_renderSceneRoot);
    m_updateTreeEnabledJob->setRoot(m_renderSceneRoot);

    // Set all flags to dirty
    m_dirtyBits.marked |= AbstractRenderer::AllDirty;
}

void Renderer::registerEventFilter(QEventFilterService *service)
{
    qCDebug(Backend) << Q_FUNC_INFO << QThread::currentThread();
    service->registerEventFilter(m_pickEventFilter.data(), 1024);
}

void Renderer::setSettings(RenderSettings *settings)
{
    m_settings = settings;
}

RenderSettings *Renderer::settings() const
{
    return m_settings;
}

void Renderer::render()
{
    // Traversing the framegraph tree from root to lead node
    // Allows us to define the rendering set up
    // Camera, RenderTarget ...

    // Utimately the renderer should be a framework
    // For the processing of the list of renderviews

    // Matrice update, bounding volumes computation ...
    // Should be jobs

    // namespace Qt3DCore has 2 distincts node trees
    // One scene description
    // One framegraph description

    while (m_running.load() > 0) {
        doRender();
        // TO DO: Restore windows exposed detection
        // Probably needs to happens some place else though
    }
}

void Renderer::doRender(bool scene3dBlocking)
{
    Renderer::ViewSubmissionResultData submissionData;
    bool hasCleanedQueueAndProceeded = false;
    bool preprocessingComplete = false;
    bool beganDrawing = false;
    const bool canSubmit = isReadyToSubmit();

    // Lock the mutex to protect access to the renderQueue while we look for its state
    QMutexLocker locker(m_renderQueue->mutex());
    bool queueIsComplete = m_renderQueue->isFrameQueueComplete();
    bool queueIsEmpty = m_renderQueue->targetRenderViewCount() == 0;

    // Scene3D Blocking Mode
    if (scene3dBlocking && !queueIsComplete && !queueIsEmpty) {
        int i = 0;
        // We wait at most 10ms to avoid a case we could never recover from
        while (!queueIsComplete && !queueIsEmpty && i++ < 10) {
            qCDebug(Backend) << Q_FUNC_INFO << "Waiting for ready queue (try:" << i << "/ 10)";
            locker.unlock();
            // Give worker threads a chance to complete the queue
            QThread::msleep(1);
            locker.relock();
            queueIsComplete = m_renderQueue->isFrameQueueComplete();
            // This could become true if we've tried to shutdown
            queueIsEmpty = m_renderQueue->targetRenderViewCount() == 0;
        }
    }

    // When using synchronous rendering (QtQuick)
    // We are not sure that the frame queue is actually complete
    // Since a call to render may not be synched with the completions
    // of the RenderViewJobs
    // In such a case we return early, waiting for a next call with
    // the frame queue complete at this point

    // RenderQueue is complete (but that means it may be of size 0)
    if (canSubmit && (queueIsComplete && !queueIsEmpty)) {
        const QVector<Render::RenderView *> renderViews = m_renderQueue->nextFrameQueue();

#if QT_CONFIG(qt3d_profile_jobs)
        // Save start of frame
        JobRunStats submissionStatsPart1;
        JobRunStats submissionStatsPart2;
        submissionStatsPart1.jobId.typeAndInstance[0] = JobTypes::FrameSubmissionPart1;
        submissionStatsPart1.jobId.typeAndInstance[1] = 0;
        submissionStatsPart1.threadId = reinterpret_cast<quint64>(QThread::currentThreadId());
        submissionStatsPart1.startTime = QThreadPooler::m_jobsStatTimer.nsecsElapsed();
        submissionStatsPart2.jobId.typeAndInstance[0] = JobTypes::FrameSubmissionPart2;
        submissionStatsPart2.jobId.typeAndInstance[1] = 0;
        submissionStatsPart2.threadId = reinterpret_cast<quint64>(QThread::currentThreadId());
#endif

        if (canRender()) {
            { // Scoped to destroy surfaceLock
                QSurface *surface = nullptr;
                for (const Render::RenderView *rv: renderViews) {
                    surface = rv->surface();
                    if (surface)
                        break;
                }

                SurfaceLocker surfaceLock(surface);
                const bool surfaceIsValid = (surface && surfaceLock.isSurfaceValid());
                if (surfaceIsValid) {
                    // Reset state for each draw if we don't have complete control of the context
                    if (!m_ownedContext)
                        m_submissionContext->setCurrentStateSet(nullptr);
                    beganDrawing = m_submissionContext->beginDrawing(surface);
                    if (beganDrawing) {
                        // 1) Execute commands for buffer uploads, texture updates, shader loading first
                        updateGLResources();
                        // 2) Update VAO and copy data into commands to allow concurrent submission
                        prepareCommandsSubmission(renderViews);
                        preprocessingComplete = true;
                    }
                }
            }
            // 2) Proceed to next frame and start preparing frame n + 1
            m_renderQueue->reset();
            locker.unlock(); // Done protecting RenderQueue
            m_vsyncFrameAdvanceService->proceedToNextFrame();
            hasCleanedQueueAndProceeded = true;

#if QT_CONFIG(qt3d_profile_jobs)
            if (preprocessingComplete) {
                submissionStatsPart2.startTime = QThreadPooler::m_jobsStatTimer.nsecsElapsed();
                submissionStatsPart1.endTime = submissionStatsPart2.startTime;
            }
#endif
            // Only try to submit the RenderViews if the preprocessing was successful
            // This part of the submission is happening in parallel to the RV building for the next frame
            if (preprocessingComplete) {
                // 3) Submit the render commands for frame n (making sure we never reference something that could be changing)
                // Render using current device state and renderer configuration
                submissionData = submitRenderViews(renderViews);

                // Perform any required cleanup of the Graphics resources (Buffers deleted, Shader deleted...)
                cleanGraphicsResources();
            }
        }

#if QT_CONFIG(qt3d_profile_jobs)
        // Execute the pending shell commands
        m_commandExecuter->performAsynchronousCommandExecution(renderViews);
#endif

        // Delete all the RenderViews which will clear the allocators
        // that were used for their allocation
        qDeleteAll(renderViews);

#if QT_CONFIG(qt3d_profile_jobs)
        if (preprocessingComplete) {
            // Save submission elapsed time
            submissionStatsPart2.endTime = QThreadPooler::m_jobsStatTimer.nsecsElapsed();
            // Note this is safe since proceedToNextFrame is the one going to trigger
            // the write to the file, and this is performed after this step
            Qt3DCore::QThreadPooler::addSubmissionLogStatsEntry(submissionStatsPart1);
            Qt3DCore::QThreadPooler::addSubmissionLogStatsEntry(submissionStatsPart2);
            Profiling::GLTimeRecorder::writeResults();
        }
#endif
    }

    // Only reset renderQueue and proceed to next frame if the submission
    // succeeded or if we are using a render thread and that is wasn't performed
    // already

    // If hasCleanedQueueAndProceeded isn't true this implies that something went wrong
    // with the rendering and/or the renderqueue is incomplete from some reason
    // (in the case of scene3d the render jobs may be taking too long ....)
    // or alternatively it could be complete but empty (RenderQueue of size 0)
    if (!hasCleanedQueueAndProceeded &&
        (m_renderThread || queueIsComplete || queueIsEmpty)) {
        // RenderQueue was full but something bad happened when
        // trying to render it and therefore proceedToNextFrame was not called
        // Note: in this case the renderQueue mutex is still locked

        // Reset the m_renderQueue so that we won't try to render
        // with a queue used by a previous frame with corrupted content
        // if the current queue was correctly submitted
        m_renderQueue->reset();

        // We allow the RenderTickClock service to proceed to the next frame
        // In turn this will allow the aspect manager to request a new set of jobs
        // to be performed for each aspect
        m_vsyncFrameAdvanceService->proceedToNextFrame();
    }

    // Perform the last swapBuffers calls after the proceedToNextFrame
    // as this allows us to gain a bit of time for the preparation of the
    // next frame
    // Finish up with last surface used in the list of RenderViews
    if (beganDrawing) {
        SurfaceLocker surfaceLock(submissionData.surface);
        // Finish up with last surface used in the list of RenderViews
        m_submissionContext->endDrawing(submissionData.lastBoundFBOId == m_submissionContext->defaultFBO() && surfaceLock.isSurfaceValid());
    }
}

// Called by RenderViewJobs
// When the frameQueue is complete and we are using a renderThread
// we allow the render thread to proceed
void Renderer::enqueueRenderView(Render::RenderView *renderView, int submitOrder)
{
    QMutexLocker locker(m_renderQueue->mutex()); // Prevent out of order execution
    // We cannot use a lock free primitive here because:
    // - QVector is not thread safe
    // - Even if the insert is made correctly, the isFrameComplete call
    //   could be invalid since depending on the order of execution
    //   the counter could be complete but the renderview not yet added to the
    //   buffer depending on whichever order the cpu decides to process this
    const bool isQueueComplete = m_renderQueue->queueRenderView(renderView, submitOrder);
    locker.unlock(); // We're done protecting the queue at this point
    if (isQueueComplete) {
        if (m_renderThread && m_running.load())
            Q_ASSERT(m_submitRenderViewsSemaphore.available() == 0);
        m_submitRenderViewsSemaphore.release(1);
    }
}

bool Renderer::canRender() const
{
    // Make sure that we've not been told to terminate
    if (m_renderThread && !m_running.load()) {
        qCDebug(Rendering) << "RenderThread termination requested whilst waiting";
        return false;
    }

    // TO DO: Check if all surfaces have been destroyed...
    // It may be better if the last window to be closed trigger a call to shutdown
    // Rather than having checks for the surface everywhere

    return true;
}

bool Renderer::isReadyToSubmit()
{
    // If we are using a render thread, make sure that
    // we've been told to render before rendering
    if (m_renderThread) { // Prevent ouf of order execution
        m_submitRenderViewsSemaphore.acquire(1);

        // Check if shutdown has been requested
        if (m_running.load() == 0)
            return false;

        // When using Thread rendering, the semaphore should only
        // be released when the frame queue is complete and there's
        // something to render
        // The case of shutdown should have been handled just before
        Q_ASSERT(m_renderQueue->isFrameQueueComplete());
    }
    return true;
}

// Main thread
QVariant Renderer::executeCommand(const QStringList &args)
{
#if QT_CONFIG(qt3d_profile_jobs)
    return m_commandExecuter->executeCommand(args);
#else
    Q_UNUSED(args);
#endif
    return QVariant();
}

/*!
    \internal
    Called in the context of the aspect thread from QRenderAspect::onRegistered
*/
void Renderer::setOffscreenSurfaceHelper(OffscreenSurfaceHelper *helper)
{
    QMutexLocker locker(&m_offscreenSurfaceMutex);
    m_offscreenHelper = helper;
}

QSurfaceFormat Renderer::format()
{
    return m_format;
}

// When this function is called, we must not be processing the commands for frame n+1
void Renderer::prepareCommandsSubmission(const QVector<RenderView *> &renderViews)
{
    OpenGLVertexArrayObject *vao = nullptr;
    QHash<HVao, bool> updatedTable;

    for (RenderView *rv: renderViews) {
        const QVector<RenderCommand *> commands = rv->commands();
        for (RenderCommand *command : commands) {
            // Update/Create VAO
            if (command->m_type == RenderCommand::Draw) {
                Geometry *rGeometry = m_nodesManager->data<Geometry, GeometryManager>(command->m_geometry);
                GeometryRenderer *rGeometryRenderer = m_nodesManager->data<GeometryRenderer, GeometryRendererManager>(command->m_geometryRenderer);
                Shader *shader = m_nodesManager->data<Shader, ShaderManager>(command->m_shader);

                // We should never have inserted a command for which these are null
                // in the first place
                Q_ASSERT(rGeometry && rGeometryRenderer && shader);

                // The VAO should be created only once for a QGeometry and a ShaderProgram
                // Manager should have a VAO Manager that are indexed by QMeshData and Shader
                // RenderCommand should have a handle to the corresponding VAO for the Mesh and Shader
                HVao vaoHandle;

                // Create VAO or return already created instance associated with command shader/geometry
                // (VAO is emulated if not supported)
                createOrUpdateVAO(command, &vaoHandle, &vao);
                command->m_vao = vaoHandle;

                // Avoids redoing the same thing for the same VAO
                if (!updatedTable.contains(vaoHandle)) {
                    updatedTable.insert(vaoHandle, true);

                    // Do we have any attributes that are dirty ?
                    const bool requiresPartialVAOUpdate = requiresVAOAttributeUpdate(rGeometry, command);

                    // If true, we need to reupload all attributes to set the VAO
                    // Otherwise only dirty attributes will be updates
                    const bool requiresFullVAOUpdate = (!vao->isSpecified()) || (rGeometry->isDirty() || rGeometryRenderer->isDirty());

                    // Append dirty Geometry to temporary vector
                    // so that its dirtiness can be unset later
                    if (rGeometry->isDirty())
                        m_dirtyGeometry.push_back(rGeometry);

                    if (!command->m_attributes.isEmpty() && (requiresFullVAOUpdate || requiresPartialVAOUpdate)) {
                        Profiling::GLTimeRecorder recorder(Profiling::VAOUpload);
                        // Activate shader
                        m_submissionContext->activateShader(shader->dna());
                        // Bind VAO
                        vao->bind();
                        // Update or set Attributes and Buffers for the given rGeometry and Command
                        // Note: this fills m_dirtyAttributes as well
                        if (updateVAOWithAttributes(rGeometry, command, shader, requiresFullVAOUpdate))
                            vao->setSpecified(true);
                    }
                }

                // Unset dirtiness on rGeometryRenderer only
                // The rGeometry may be shared by several rGeometryRenderer
                // so we cannot unset its dirtiness at this point
                if (rGeometryRenderer->isDirty())
                    rGeometryRenderer->unsetDirty();

                // Prepare the ShaderParameterPack based on the active uniforms of the shader
                shader->prepareUniforms(command->m_parameterPack);

                // TO DO: The step below could be performed by the RenderCommand builder job
                { // Scoped to show extent
                    command->m_isValid = !command->m_attributes.empty();
                    if (!command->m_isValid)
                        continue;

                    // Update the draw command with what's going to be needed for the drawing
                    uint primitiveCount = rGeometryRenderer->vertexCount();
                    uint estimatedCount = 0;
                    Attribute *indexAttribute = nullptr;
                    Attribute *indirectAttribute = nullptr;

                    const QVector<Qt3DCore::QNodeId> attributeIds = rGeometry->attributes();
                    for (Qt3DCore::QNodeId attributeId : attributeIds) {
                        Attribute *attribute = m_nodesManager->attributeManager()->lookupResource(attributeId);
                        switch (attribute->attributeType()) {
                        case QAttribute::IndexAttribute:
                            indexAttribute = attribute;
                            break;
                        case QAttribute::DrawIndirectAttribute:
                            indirectAttribute = attribute;
                            break;
                        case QAttribute::VertexAttribute: {
                            if (command->m_attributes.contains(attribute->nameId()))
                                estimatedCount = qMax(attribute->count(), estimatedCount);
                            break;
                        }
                        default:
                            Q_UNREACHABLE();
                            break;
                        }
                    }

                    command->m_drawIndexed = (indexAttribute != nullptr);
                    command->m_drawIndirect = (indirectAttribute != nullptr);

                    // Update the draw command with all the information required for the drawing
                    if (command->m_drawIndexed) {
                        command->m_indexAttributeDataType = GraphicsContext::glDataTypeFromAttributeDataType(indexAttribute->vertexBaseType());
                        command->m_indexAttributeByteOffset = indexAttribute->byteOffset() + rGeometryRenderer->indexBufferByteOffset();
                    }

                    // Note: we only care about the primitiveCount when using direct draw calls
                    // For indirect draw calls it is assumed the buffer was properly set already
                    if (command->m_drawIndirect) {
                        command->m_indirectAttributeByteOffset = indirectAttribute->byteOffset();
                        command->m_indirectDrawBuffer = m_nodesManager->bufferManager()->lookupHandle(indirectAttribute->bufferId());
                    } else {
                        // Use the count specified by the GeometryRender
                        // If not specify use the indexAttribute count if present
                        // Otherwise tries to use the count from the attribute with the highest count
                        if (primitiveCount == 0) {
                            if (indexAttribute)
                                primitiveCount = indexAttribute->count();
                            else
                                primitiveCount = estimatedCount;
                        }
                    }

                    command->m_primitiveCount = primitiveCount;
                    command->m_primitiveType = rGeometryRenderer->primitiveType();
                    command->m_primitiveRestartEnabled = rGeometryRenderer->primitiveRestartEnabled();
                    command->m_restartIndexValue = rGeometryRenderer->restartIndexValue();
                    command->m_firstInstance = rGeometryRenderer->firstInstance();
                    command->m_instanceCount = rGeometryRenderer->instanceCount();
                    command->m_firstVertex = rGeometryRenderer->firstVertex();
                    command->m_indexOffset = rGeometryRenderer->indexOffset();
                    command->m_verticesPerPatch = rGeometryRenderer->verticesPerPatch();
                } // scope
            } else if (command->m_type == RenderCommand::Compute) {
                Shader *shader = m_nodesManager->data<Shader, ShaderManager>(command->m_shader);
                Q_ASSERT(shader);

                // Prepare the ShaderParameterPack based on the active uniforms of the shader
                shader->prepareUniforms(command->m_parameterPack);
            }
        }
    }

    // Make sure we leave nothing bound
    if (vao)
        vao->release();

    // Unset dirtiness on Geometry and Attributes
    // Note: we cannot do it in the loop above as we want to be sure that all
    // the VAO which reference the geometry/attributes are properly updated
    for (Attribute *attribute : qAsConst(m_dirtyAttributes))
        attribute->unsetDirty();
    m_dirtyAttributes.clear();

    for (Geometry *geometry : qAsConst(m_dirtyGeometry))
        geometry->unsetDirty();
    m_dirtyGeometry.clear();
}

// Executed in a job
void Renderer::lookForAbandonedVaos()
{
    const QVector<HVao> activeVaos = m_nodesManager->vaoManager()->activeHandles();
    for (const HVao &handle : activeVaos) {
        OpenGLVertexArrayObject *vao = m_nodesManager->vaoManager()->data(handle);

        // Make sure to only mark VAOs for deletion that were already created
        // (ignore those that might be currently under construction in the render thread)
        if (vao && vao->isAbandoned(m_nodesManager->geometryManager(), m_nodesManager->shaderManager())) {
            m_abandonedVaosMutex.lock();
            m_abandonedVaos.push_back(handle);
            m_abandonedVaosMutex.unlock();
        }
    }
}

// Executed in a job
void Renderer::lookForDirtyBuffers()
{
    const QVector<HBuffer> activeBufferHandles = m_nodesManager->bufferManager()->activeHandles();
    for (const HBuffer &handle: activeBufferHandles) {
        Buffer *buffer = m_nodesManager->bufferManager()->data(handle);
        if (buffer->isDirty())
            m_dirtyBuffers.push_back(handle);
    }
}

void Renderer::lookForDownloadableBuffers()
{
    m_downloadableBuffers.clear();
    const QVector<HBuffer> activeBufferHandles = m_nodesManager->bufferManager()->activeHandles();
    for (const HBuffer &handle : activeBufferHandles) {
        Buffer *buffer = m_nodesManager->bufferManager()->data(handle);
        if (buffer->access() & QBuffer::Read)
            m_downloadableBuffers.push_back(handle);
    }
}

// Executed in a job
void Renderer::lookForDirtyTextures()
{
    // To avoid having Texture or TextureImage maintain relationships between
    // one another, we instead perform a lookup here to check if a texture
    // image has been updated to then notify textures referencing the image
    // that they need to be updated
    TextureImageManager *imageManager = m_nodesManager->textureImageManager();
    const QVector<HTextureImage> activeTextureImageHandles = imageManager->activeHandles();
    Qt3DCore::QNodeIdVector dirtyImageIds;
    for (const HTextureImage &handle: activeTextureImageHandles) {
        TextureImage *image = imageManager->data(handle);
        if (image->isDirty()) {
            dirtyImageIds.push_back(image->peerId());
            image->unsetDirty();
        }
    }

    TextureManager *textureManager = m_nodesManager->textureManager();
    const QVector<HTexture> activeTextureHandles = textureManager->activeHandles();
    for (const HTexture &handle: activeTextureHandles) {
        Texture *texture = textureManager->data(handle);
        const QNodeIdVector imageIds = texture->textureImageIds();

        // Does the texture reference any of the dirty texture images?
        for (const QNodeId imageId: imageIds) {
            if (dirtyImageIds.contains(imageId)) {
                texture->addDirtyFlag(Texture::DirtyImageGenerators);
                break;
            }
        }

        // Dirty meaning that something has changed on the texture
        // either properties, parameters, generator or a texture image
        if (texture->dirtyFlags() != Texture::NotDirty)
            m_dirtyTextures.push_back(handle);
        // Note: texture dirty flags are reset when actually updating the
        // textures in updateGLResources() as resetting flags here would make
        // us lose information about what was dirty exactly.
    }
}

// Executed in a job
void Renderer::reloadDirtyShaders()
{
    Q_ASSERT(isRunning());
    const QVector<HTechnique> activeTechniques = m_nodesManager->techniqueManager()->activeHandles();
    const QVector<HShaderBuilder> activeBuilders = m_nodesManager->shaderBuilderManager()->activeHandles();
    for (const HTechnique &techniqueHandle : activeTechniques) {
        Technique *technique = m_nodesManager->techniqueManager()->data(techniqueHandle);
        // If api of the renderer matches the one from the technique
        if (technique->isCompatibleWithRenderer()) {
            const auto passIds = technique->renderPasses();
            for (const QNodeId passId : passIds) {
                RenderPass *renderPass = m_nodesManager->renderPassManager()->lookupResource(passId);
                HShader shaderHandle = m_nodesManager->shaderManager()->lookupHandle(renderPass->shaderProgram());
                Shader *shader = m_nodesManager->shaderManager()->data(shaderHandle);

                ShaderBuilder *shaderBuilder = nullptr;
                for (const HShaderBuilder &builderHandle : activeBuilders) {
                    ShaderBuilder *builder = m_nodesManager->shaderBuilderManager()->data(builderHandle);
                    if (builder->shaderProgramId() == shader->peerId()) {
                        shaderBuilder = builder;
                        break;
                    }
                }

                if (shaderBuilder) {
                    shaderBuilder->setGraphicsApi(*technique->graphicsApiFilter());

                    for (int i = 0; i <= ShaderBuilder::Compute; i++) {
                        const auto builderType = static_cast<ShaderBuilder::ShaderType>(i);
                        if (!shaderBuilder->shaderGraph(builderType).isValid())
                            continue;

                        if (shaderBuilder->isShaderCodeDirty(builderType)) {
                            shaderBuilder->generateCode(builderType);
                        }

                        QShaderProgram::ShaderType shaderType = QShaderProgram::Vertex;
                        switch (builderType) {
                        case ShaderBuilder::Vertex:
                            shaderType = QShaderProgram::Vertex;
                            break;
                        case ShaderBuilder::TessellationControl:
                            shaderType = QShaderProgram::TessellationControl;
                            break;
                        case ShaderBuilder::TessellationEvaluation:
                            shaderType = QShaderProgram::TessellationEvaluation;
                            break;
                        case ShaderBuilder::Geometry:
                            shaderType = QShaderProgram::Geometry;
                            break;
                        case ShaderBuilder::Fragment:
                            shaderType = QShaderProgram::Fragment;
                            break;
                        case ShaderBuilder::Compute:
                            shaderType = QShaderProgram::Compute;
                            break;
                        }

                        const auto code = shaderBuilder->shaderCode(builderType);
                        shader->setShaderCode(shaderType, code);
                    }
                }

                if (Q_UNLIKELY(shader->hasPendingNotifications()))
                    shader->submitPendingNotifications();
                // If the shader hasn't be loaded, load it
                if (shader != nullptr && !shader->isLoaded())
                    loadShader(shader, shaderHandle);
            }
        }
    }
}

// Executed in a job
void Renderer::sendTextureChangesToFrontend()
{
    const QVector<QPair<TextureProperties, Qt3DCore::QNodeIdVector>> updateTextureProperties = std::move(m_updatedTextureProperties);
    for (const auto &pair : updateTextureProperties) {
        // Prepare change notification

        const Qt3DCore::QNodeIdVector targetIds = pair.second;
        for (const Qt3DCore::QNodeId targetId: targetIds) {
            // Lookup texture
            Texture *t = m_nodesManager->textureManager()->lookupResource(targetId);

            // Texture might have been deleted between previous and current frame
            if (t == nullptr)
                continue;

            // Send change and update backend
            t->updatePropertiesAndNotify(pair.first);
        }
    }
}

// Render Thread (or QtQuick RenderThread when using Scene3D)
// Scene3D: When using Scene3D rendering, we can't assume that when
// updateGLResources is called, the resource handles points to still existing
// objects. This is because Scene3D calls doRender independently of whether all
// jobs have completed or not which in turn calls proceedToNextFrame under some
// conditions. Such conditions are usually met on startup to avoid deadlocks.
// proceedToNextFrame triggers the syncChanges calls for the next frame, which
// may contain destruction changes targeting resources. When the above
// happens, this can result in the dirtyResource vectors containing handles of
// objects that may already have been destroyed
void Renderer::updateGLResources()
{
    {
        Profiling::GLTimeRecorder recorder(Profiling::BufferUpload);
        const QVector<HBuffer> dirtyBufferHandles = std::move(m_dirtyBuffers);
        for (const HBuffer &handle: dirtyBufferHandles) {
            Buffer *buffer = m_nodesManager->bufferManager()->data(handle);

            // Can be null when using Scene3D rendering
            if (buffer == nullptr)
                continue;

            // Forces creation if it doesn't exit
            // Also note the binding point doesn't really matter here, we just upload data
            if (!m_submissionContext->hasGLBufferForBuffer(buffer))
                m_submissionContext->glBufferForRenderBuffer(buffer, GLBuffer::ArrayBuffer);
            // Update the glBuffer data
            m_submissionContext->updateBuffer(buffer);
            buffer->unsetDirty();
        }
    }

#ifndef SHADER_LOADING_IN_COMMAND_THREAD
    {
        Profiling::GLTimeRecorder recorder(Profiling::ShaderUpload);
        const QVector<HShader> dirtyShaderHandles = std::move(m_dirtyShaders);
        ShaderManager *shaderManager = m_nodesManager->shaderManager();
        for (const HShader &handle: dirtyShaderHandles) {
            Shader *shader = shaderManager->data(handle);

            // Can be null when using Scene3D rendering
            if (shader == nullptr)
                continue;

            // Compile shader
            m_submissionContext->loadShader(shader, shaderManager);
        }
    }
#endif

    {
        Profiling::GLTimeRecorder recorder(Profiling::TextureUpload);
        const QVector<HTexture> activeTextureHandles = std::move(m_dirtyTextures);
        for (const HTexture &handle: activeTextureHandles) {
            Texture *texture = m_nodesManager->textureManager()->data(handle);

            // Can be null when using Scene3D rendering
            if (texture ==  nullptr)
                continue;

            // Create or Update GLTexture (the GLTexture instance is created
            // (not the underlying GL instance) if required and all things that
            // can take place without a GL context are done here)
            updateTexture(texture);
        }
        // We want to upload textures data at this point as the SubmissionThread and
        // AspectThread are locked ensuring no races between Texture/TextureImage and
        // GLTexture
        QNodeIdVector updatedTexturesForFrame;
        if (m_submissionContext != nullptr) {
            GLTextureManager *glTextureManager = m_nodesManager->glTextureManager();
            const QVector<GLTexture *> glTextures = glTextureManager->activeResources();
            // Upload texture data
            for (GLTexture *glTexture : glTextures) {
                const GLTexture::TextureUpdateInfo info = glTexture->createOrUpdateGLTexture();

                // GLTexture creation provides us width/height/format ... information
                // for textures which had not initially specified these information (TargetAutomatic...)
                // Gather these information and store them to be distributed by a change next frame
                const QNodeIdVector referenceTextureIds = glTextureManager->referencedTextureIds(glTexture);
                // Store properties and referenceTextureIds
                if (info.wasUpdated) {
                    m_updatedTextureProperties.push_back({info.properties, referenceTextureIds});
                    updatedTexturesForFrame += referenceTextureIds;
                }
            }
        }

        // If the underlying GL Texture was for whatever reason recreated, we need to make sure
        // that if it is used as a color attachment, we rebuild the FBO next time it is used
        m_submissionContext->setUpdatedTexture(std::move(updatedTexturesForFrame));
    }
    // When Textures are cleaned up, their id is saved so that they can be
    // cleaned up in the render thread Note: we perform this step in second so
    // that the previous updateTexture call has a chance to find a shared
    // texture and avoid possible destroying recreating a new texture
    const QVector<Qt3DCore::QNodeId> cleanedUpTextureIds = m_nodesManager->textureManager()->takeTexturesIdsToCleanup();
    for (const Qt3DCore::QNodeId textureCleanedUpId: cleanedUpTextureIds)
        cleanupTexture(textureCleanedUpId);

    // Remove destroyed FBOs
    {
        const QNodeIdVector destroyedRenderTargetIds = m_nodesManager->renderTargetManager()->takeRenderTargetIdsToCleanup();
        for (const Qt3DCore::QNodeId &renderTargetId : destroyedRenderTargetIds)
            m_submissionContext->releaseRenderTarget(renderTargetId);
    }
}

// Render Thread
void Renderer::updateTexture(Texture *texture)
{
    // Check that the current texture images are still in place, if not, do not update
    const bool isValid = texture->isValid(m_nodesManager->textureImageManager());
    if (!isValid)
        return;

    // For implementing unique, non-shared, non-cached textures.
    // for now, every texture is shared by default

    bool isUnique = false;

    // TO DO: Update the vector once per frame (or in a job)
    const QVector<HAttachment> activeRenderTargetOutputs = m_nodesManager->attachmentManager()->activeHandles();
    // A texture is unique if it's being reference by a render target output
    for (const HAttachment &attachmentHandle : activeRenderTargetOutputs) {
        RenderTargetOutput *attachment = m_nodesManager->attachmentManager()->data(attachmentHandle);
        if (attachment->textureUuid() == texture->peerId()) {
            isUnique = true;
            break;
        }
    }

    // Try to find the associated GLTexture for the backend Texture
    GLTextureManager *glTextureManager = m_nodesManager->glTextureManager();
    GLTexture *glTexture = glTextureManager->lookupResource(texture->peerId());

    auto createOrUpdateGLTexture = [=] () {
        if (isUnique)
            glTextureManager->createUnique(texture);
        else
            glTextureManager->getOrCreateShared(texture);
        texture->unsetDirty();
    };

    // No GLTexture associated yet -> create it
    if (glTexture == nullptr) {
        createOrUpdateGLTexture();
        return;
    }

    // if this texture is a shared texture, we might need to look for a new TextureImpl
    // and abandon the old one
    if (glTextureManager->isShared(glTexture)) {
        glTextureManager->abandon(glTexture, texture->peerId());
        // Note: if isUnique is true, a once shared texture will become unique
        createOrUpdateGLTexture();
        return;
    }

    // this texture node is the only one referring to the GLTexture.
    // we could thus directly modify the texture. Instead, for non-unique textures,
    // we first see if there is already a matching texture present.
    if (!isUnique) {
        GLTexture *newSharedTex = glTextureManager->findMatchingShared(texture);
        if (newSharedTex && newSharedTex != glTexture) {
            glTextureManager->abandon(glTexture, texture->peerId());
            glTextureManager->adoptShared(newSharedTex, texture);
            texture->unsetDirty();
            return;
        }
    }

    // we hold a reference to a unique or exclusive access to a shared texture
    // we can thus modify the texture directly.
    const Texture::DirtyFlags dirtyFlags = texture->dirtyFlags();

    if (dirtyFlags.testFlag(Texture::DirtyProperties) &&
            !glTextureManager->setProperties(glTexture, texture->properties()))
        qWarning() << "[Qt3DRender::TextureNode] updateTexture: TextureImpl.setProperties failed, should be non-shared";

    if (dirtyFlags.testFlag(Texture::DirtyParameters) &&
            !glTextureManager->setParameters(glTexture, texture->parameters()))
        qWarning() << "[Qt3DRender::TextureNode] updateTexture: TextureImpl.setParameters failed, should be non-shared";

    // Will make the texture requestUpload
    if (dirtyFlags.testFlag(Texture::DirtyImageGenerators) &&
            !glTextureManager->setImages(glTexture, texture->textureImageIds()))
        qWarning() << "[Qt3DRender::TextureNode] updateTexture: TextureImpl.setGenerators failed, should be non-shared";

    // Will make the texture requestUpload
    if (dirtyFlags.testFlag(Texture::DirtyDataGenerator) &&
            !glTextureManager->setGenerator(glTexture, texture->dataGenerator()))
        qWarning() << "[Qt3DRender::TextureNode] updateTexture: TextureImpl.setGenerator failed, should be non-shared";

    // Unset the dirty flag on the texture
    texture->unsetDirty();
}

// Render Thread
void Renderer::cleanupTexture(Qt3DCore::QNodeId cleanedUpTextureId)
{
    GLTextureManager *glTextureManager = m_nodesManager->glTextureManager();
    GLTexture *glTexture = glTextureManager->lookupResource(cleanedUpTextureId);

    if (glTexture != nullptr)
        glTextureManager->abandon(glTexture, cleanedUpTextureId);
}

void Renderer::downloadGLBuffers()
{
    lookForDownloadableBuffers();
    const QVector<HBuffer> downloadableHandles = std::move(m_downloadableBuffers);
    for (const HBuffer &handle : downloadableHandles) {
        Buffer *buffer = m_nodesManager->bufferManager()->data(handle);
        QByteArray content = m_submissionContext->downloadBufferContent(buffer);
        m_sendBufferCaptureJob->addRequest(QPair<Buffer*, QByteArray>(buffer, content));
    }
}

// Happens in RenderThread context when all RenderViewJobs are done
// Returns the id of the last bound FBO
Renderer::ViewSubmissionResultData Renderer::submitRenderViews(const QVector<Render::RenderView *> &renderViews)
{
    QElapsedTimer timer;
    quint64 queueElapsed = 0;
    timer.start();

    const int renderViewsCount = renderViews.size();
    quint64 frameElapsed = queueElapsed;
    m_lastFrameCorrect.store(1);    // everything fine until now.....

    qCDebug(Memory) << Q_FUNC_INFO << "rendering frame ";

    // We might not want to render on the default FBO
    uint lastBoundFBOId = m_submissionContext->boundFrameBufferObject();
    QSurface *surface = nullptr;
    QSurface *previousSurface = nullptr;
    for (const Render::RenderView *rv: renderViews) {
        previousSurface = rv->surface();
        if (previousSurface)
            break;
    }
    QSurface *lastUsedSurface = nullptr;

    for (int i = 0; i < renderViewsCount; ++i) {
        // Initialize GraphicsContext for drawing
        // If the RenderView has a RenderStateSet defined
        const RenderView *renderView = renderViews.at(i);

        // Check if using the same surface as the previous RenderView.
        // If not, we have to free up the context from the previous surface
        // and make the context current on the new surface
        surface = renderView->surface();
        SurfaceLocker surfaceLock(surface);

        // TO DO: Make sure that the surface we are rendering too has not been unset

        // For now, if we do not have a surface, skip this renderview
        // TODO: Investigate if it's worth providing a fallback offscreen surface
        //       to use when surface is null. Or if we should instead expose an
        //       offscreensurface to Qt3D.
        if (!surface || !surfaceLock.isSurfaceValid()) {
            m_lastFrameCorrect.store(0);
            continue;
        }

        lastUsedSurface = surface;
        const bool surfaceHasChanged = surface != previousSurface;

        if (surfaceHasChanged && previousSurface) {
            const bool swapBuffers = (lastBoundFBOId == m_submissionContext->defaultFBO()) && PlatformSurfaceFilter::isSurfaceValid(previousSurface);
            // We only call swap buffer if we are sure the previous surface is still valid
            m_submissionContext->endDrawing(swapBuffers);
        }

        if (surfaceHasChanged) {
            // If we can't make the context current on the surface, skip to the
            // next RenderView. We won't get the full frame but we may get something
            if (!m_submissionContext->beginDrawing(surface)) {
                qWarning() << "Failed to make OpenGL context current on surface";
                m_lastFrameCorrect.store(0);
                continue;
            }

            previousSurface = surface;
            lastBoundFBOId = m_submissionContext->boundFrameBufferObject();
        }

        // Apply Memory Barrier if needed
        if (renderView->memoryBarrier() != QMemoryBarrier::None)
            m_submissionContext->memoryBarrier(renderView->memoryBarrier());

        // Note: the RenderStateSet is allocated once per RV if needed
        // and it contains a list of StateVariant value types
        RenderStateSet *renderViewStateSet = renderView->stateSet();

        {
            Profiling::GLTimeRecorder recorder(Profiling::StateUpdate);
            // Set the RV state if not null,
            if (renderViewStateSet != nullptr)
                m_submissionContext->setCurrentStateSet(renderViewStateSet);
            else
                m_submissionContext->setCurrentStateSet(m_defaultRenderStateSet);
        }

        // Set RenderTarget ...
        // Activate RenderTarget
        {
            Profiling::GLTimeRecorder recorder(Profiling::RenderTargetUpdate);
            m_submissionContext->activateRenderTarget(renderView->renderTargetId(),
                                                    renderView->attachmentPack(),
                                                    lastBoundFBOId);
        }

        {
            Profiling::GLTimeRecorder recorder(Profiling::ClearBuffer);
            // set color, depth, stencil clear values (only if needed)
            auto clearBufferTypes = renderView->clearTypes();
            if (clearBufferTypes & QClearBuffers::ColorBuffer) {
                const QVector4D vCol = renderView->globalClearColorBufferInfo().clearColor;
                m_submissionContext->clearColor(QColor::fromRgbF(vCol.x(), vCol.y(), vCol.z(), vCol.w()));
            }
            if (clearBufferTypes & QClearBuffers::DepthBuffer)
                m_submissionContext->clearDepthValue(renderView->clearDepthValue());
            if (clearBufferTypes & QClearBuffers::StencilBuffer)
                m_submissionContext->clearStencilValue(renderView->clearStencilValue());

            // Clear BackBuffer
            m_submissionContext->clearBackBuffer(clearBufferTypes);

            // if there are ClearColors set for different draw buffers,
            // clear each of these draw buffers individually now
            const QVector<ClearBufferInfo> clearDrawBuffers = renderView->specificClearColorBufferInfo();
            for (const ClearBufferInfo &clearBuffer : clearDrawBuffers)
                m_submissionContext->clearBufferf(clearBuffer.drawBufferIndex, clearBuffer.clearColor);
        }

        // Set the Viewport
        m_submissionContext->setViewport(renderView->viewport(), renderView->surfaceSize() * renderView->devicePixelRatio());

        // Execute the render commands
        if (!executeCommandsSubmission(renderView))
            m_lastFrameCorrect.store(0);    // something went wrong; make sure to render the next frame!

        // executeCommandsSubmission takes care of restoring the stateset to the value
        // of gc->currentContext() at the moment it was called (either
        // renderViewStateSet or m_defaultRenderStateSet)
        if (!renderView->renderCaptureNodeId().isNull()) {
            const QRenderCaptureRequest request = renderView->renderCaptureRequest();
            const QSize size = m_submissionContext->renderTargetSize(renderView->surfaceSize() * renderView->devicePixelRatio());
            QRect rect(QPoint(0, 0), size);
            if (!request.rect.isEmpty())
                rect = rect.intersected(request.rect);
            QImage image;
            if (!rect.isEmpty()) {
                // Bind fbo as read framebuffer
                m_submissionContext->bindFramebuffer(m_submissionContext->activeFBO(), GraphicsHelperInterface::FBORead);
                image = m_submissionContext->readFramebuffer(rect);
            } else {
                qWarning() << "Requested capture rectangle is outside framebuffer";
            }
            Render::RenderCapture *renderCapture =
                    static_cast<Render::RenderCapture*>(m_nodesManager->frameGraphManager()->lookupNode(renderView->renderCaptureNodeId()));
            renderCapture->addRenderCapture(request.captureId, image);
            addRenderCaptureSendRequest(renderView->renderCaptureNodeId());
        }

        if (renderView->isDownloadBuffersEnable())
            downloadGLBuffers();

        // Perform BlitFramebuffer operations
        if (renderView->hasBlitFramebufferInfo()) {
            const auto &blitFramebufferInfo = renderView->blitFrameBufferInfo();
            const QNodeId inputTargetId = blitFramebufferInfo.sourceRenderTargetId;
            const QNodeId outputTargetId = blitFramebufferInfo.destinationRenderTargetId;
            const QRect inputRect = blitFramebufferInfo.sourceRect;
            const QRect outputRect = blitFramebufferInfo.destinationRect;
            const QRenderTargetOutput::AttachmentPoint inputAttachmentPoint = blitFramebufferInfo.sourceAttachmentPoint;
            const QRenderTargetOutput::AttachmentPoint outputAttachmentPoint = blitFramebufferInfo.destinationAttachmentPoint;
            const QBlitFramebuffer::InterpolationMethod interpolationMethod = blitFramebufferInfo.interpolationMethod;
            m_submissionContext->blitFramebuffer(inputTargetId, outputTargetId, inputRect, outputRect, lastBoundFBOId,
                                                 inputAttachmentPoint, outputAttachmentPoint,
                                                 interpolationMethod);
        }


        frameElapsed = timer.elapsed() - frameElapsed;
        qCDebug(Rendering) << Q_FUNC_INFO << "Submitted Renderview " << i + 1 << "/" << renderViewsCount  << "in " << frameElapsed << "ms";
        frameElapsed = timer.elapsed();
    }

    // Bind lastBoundFBOId back. Needed also in threaded mode.
    // lastBoundFBOId != m_graphicsContext->activeFBO() when the last FrameGraph leaf node/renderView
    // contains RenderTargetSelector/RenderTarget
    if (lastBoundFBOId != m_submissionContext->activeFBO())
        m_submissionContext->bindFramebuffer(lastBoundFBOId, GraphicsHelperInterface::FBOReadAndDraw);

    // Reset state and call doneCurrent if the surface
    // is valid and was actually activated
    if (lastUsedSurface && m_submissionContext->hasValidGLHelper()) {
        // Reset state to the default state if the last stateset is not the
        // defaultRenderStateSet
        if (m_submissionContext->currentStateSet() != m_defaultRenderStateSet)
            m_submissionContext->setCurrentStateSet(m_defaultRenderStateSet);
    }

    queueElapsed = timer.elapsed() - queueElapsed;
    qCDebug(Rendering) << Q_FUNC_INFO << "Submission of Queue in " << queueElapsed << "ms <=> " << queueElapsed / renderViewsCount << "ms per RenderView <=> Avg " << 1000.0f / (queueElapsed * 1.0f/ renderViewsCount * 1.0f) << " RenderView/s";
    qCDebug(Rendering) << Q_FUNC_INFO << "Submission Completed in " << timer.elapsed() << "ms";

    // Stores the necessary information to safely perform
    // the last swap buffer call
    ViewSubmissionResultData resultData;
    resultData.lastBoundFBOId = lastBoundFBOId;
    resultData.surface = lastUsedSurface;
    return resultData;
}

void Renderer::markDirty(BackendNodeDirtySet changes, BackendNode *node)
{
    Q_UNUSED(node);
    m_dirtyBits.marked |= changes;
}

Renderer::BackendNodeDirtySet Renderer::dirtyBits()
{
    return m_dirtyBits.marked;
}

#if defined(QT_BUILD_INTERNAL)
void Renderer::clearDirtyBits(BackendNodeDirtySet changes)
{
    m_dirtyBits.remaining &= ~changes;
    m_dirtyBits.marked &= ~changes;
}
#endif

bool Renderer::shouldRender()
{
    // Only render if something changed during the last frame, or the last frame
    // was not rendered successfully (or render-on-demand is disabled)
    return (m_settings->renderPolicy() == QRenderSettings::Always
            || m_dirtyBits.marked != 0
            || m_dirtyBits.remaining != 0
            || !m_lastFrameCorrect.load());
}

void Renderer::skipNextFrame()
{
    Q_ASSERT(m_settings->renderPolicy() != QRenderSettings::Always);

    // make submitRenderViews() actually run
    m_renderQueue->setNoRender();
    m_submitRenderViewsSemaphore.release(1);
}

// Waits to be told to create jobs for the next frame
// Called by QRenderAspect jobsToExecute context of QAspectThread
// Returns all the jobs (and with proper dependency chain) required
// for the rendering of the scene
QVector<Qt3DCore::QAspectJobPtr> Renderer::renderBinJobs()
{
    QVector<QAspectJobPtr> renderBinJobs;

    // Create the jobs to build the frame
    const QVector<QAspectJobPtr> bufferJobs = createRenderBufferJobs();

    // Remove previous dependencies
    m_calculateBoundingVolumeJob->removeDependency(QWeakPointer<QAspectJob>());
    m_cleanupJob->removeDependency(QWeakPointer<QAspectJob>());

    // Set dependencies
    for (const QAspectJobPtr &bufferJob : bufferJobs)
        m_calculateBoundingVolumeJob->addDependency(bufferJob);

    m_updateLevelOfDetailJob->setFrameGraphRoot(frameGraphRoot());

    const BackendNodeDirtySet dirtyBitsForFrame = m_dirtyBits.marked | m_dirtyBits.remaining;
    m_dirtyBits.marked = 0;
    m_dirtyBits.remaining = 0;
    BackendNodeDirtySet notCleared = 0;

    // Add jobs
    const bool entitiesEnabledDirty = dirtyBitsForFrame & AbstractRenderer::EntityEnabledDirty;
    const bool entityHierarchyNeedsToBeRebuilt = dirtyBitsForFrame & AbstractRenderer::EntityHierarchyDirty;
    if (entitiesEnabledDirty || entityHierarchyNeedsToBeRebuilt) {
        renderBinJobs.push_back(m_updateTreeEnabledJob);
        // This dependency is added here because we clear all dependencies
        // at the start of this function.
        m_calculateBoundingVolumeJob->addDependency(m_updateTreeEnabledJob);
        m_calculateBoundingVolumeJob->addDependency(m_updateEntityHierarchyJob);
    }

    if (dirtyBitsForFrame & AbstractRenderer::TransformDirty ||
        dirtyBitsForFrame & AbstractRenderer::EntityHierarchyDirty) {
        renderBinJobs.push_back(m_worldTransformJob);
        renderBinJobs.push_back(m_updateWorldBoundingVolumeJob);
        renderBinJobs.push_back(m_updateShaderDataTransformJob);
    }

    if (dirtyBitsForFrame & AbstractRenderer::GeometryDirty ||
        dirtyBitsForFrame & AbstractRenderer::BuffersDirty) {
        renderBinJobs.push_back(m_calculateBoundingVolumeJob);
        renderBinJobs.push_back(m_updateMeshTriangleListJob);
    }

    if (dirtyBitsForFrame & AbstractRenderer::GeometryDirty ||
        dirtyBitsForFrame & AbstractRenderer::EntityHierarchyDirty ||
        dirtyBitsForFrame & AbstractRenderer::TransformDirty) {
        renderBinJobs.push_back(m_expandBoundingVolumeJob);
    }

    m_updateSkinningPaletteJob->setDirtyJoints(m_nodesManager->jointManager()->dirtyJoints());
    renderBinJobs.push_back(m_updateSkinningPaletteJob);
    renderBinJobs.push_back(m_updateLevelOfDetailJob);
    renderBinJobs.push_back(m_cleanupJob);

    const QVector<Qt3DCore::QNodeId> pendingCaptureIds = takePendingRenderCaptureSendRequests();
    if (pendingCaptureIds.size() > 0) {
        m_sendRenderCaptureJob->setPendingCaptureRequests(pendingCaptureIds);
        renderBinJobs.push_back(m_sendRenderCaptureJob);
    }

    // Do we need to notify any texture about property changes?
    if (m_updatedTextureProperties.size() > 0)
        renderBinJobs.push_back(m_sendTextureChangesToFrontendJob);

    renderBinJobs.push_back(m_sendBufferCaptureJob);
    renderBinJobs.append(bufferJobs);

    // Jobs to prepare GL Resource upload
    renderBinJobs.push_back(m_vaoGathererJob);

    if (dirtyBitsForFrame & AbstractRenderer::BuffersDirty)
        renderBinJobs.push_back(m_bufferGathererJob);

    if (dirtyBitsForFrame & AbstractRenderer::TexturesDirty) {
        renderBinJobs.push_back(m_syncTextureLoadingJob);
        renderBinJobs.push_back(m_textureGathererJob);
    }


    // Layer cache is dependent on layers, layer filters (hence FG structure
    // changes) and the enabled flag on entities
    const bool frameGraphDirty = dirtyBitsForFrame & AbstractRenderer::FrameGraphDirty;
    const bool layersDirty = dirtyBitsForFrame & AbstractRenderer::LayersDirty || entityHierarchyNeedsToBeRebuilt;
    const bool layersCacheNeedsToBeRebuilt = layersDirty || entitiesEnabledDirty || frameGraphDirty;
    const bool materialDirty = dirtyBitsForFrame & AbstractRenderer::MaterialDirty;
    const bool materialCacheNeedsToBeRebuilt = materialDirty || frameGraphDirty;
    const bool lightsDirty = dirtyBitsForFrame & AbstractRenderer::LightsDirty;
    const bool computeableDirty = dirtyBitsForFrame & AbstractRenderer::ComputeDirty;
    const bool renderableDirty = dirtyBitsForFrame & AbstractRenderer::GeometryDirty;

    // Rebuild Entity Hierarchy if dirty
    if (entityHierarchyNeedsToBeRebuilt)
        renderBinJobs.push_back(m_updateEntityHierarchyJob);

    // Rebuild Entity Layers list if layers are dirty
    if (layersDirty)
        renderBinJobs.push_back(m_updateEntityLayersJob);

    QMutexLocker lock(m_renderQueue->mutex());
    if (m_renderQueue->wasReset()) { // Have we rendered yet? (Scene3D case)
        // Traverse the current framegraph. For each leaf node create a
        // RenderView and set its configuration then create a job to
        // populate the RenderView with a set of RenderCommands that get
        // their details from the RenderNodes that are visible to the
        // Camera selected by the framegraph configuration
        FrameGraphVisitor visitor(m_nodesManager->frameGraphManager());
        const QVector<FrameGraphNode *> fgLeaves = visitor.traverse(frameGraphRoot());

        // Remove leaf nodes that no longer exist from cache
        const QList<FrameGraphNode *> keys = m_cache.leafNodeCache.keys();
        for (FrameGraphNode *leafNode : keys) {
            if (!fgLeaves.contains(leafNode))
                m_cache.leafNodeCache.remove(leafNode);
        }

        const int fgBranchCount = fgLeaves.size();
        for (int i = 0; i < fgBranchCount; ++i) {
            RenderViewBuilder builder(fgLeaves.at(i), i, this);
            builder.setLayerCacheNeedsToBeRebuilt(layersCacheNeedsToBeRebuilt);
            builder.setMaterialGathererCacheNeedsToBeRebuilt(materialCacheNeedsToBeRebuilt);
            builder.setRenderableCacheNeedsToBeRebuilt(renderableDirty);
            builder.setComputableCacheNeedsToBeRebuilt(computeableDirty);
            builder.setLightGathererCacheNeedsToBeRebuilt(lightsDirty);

            builder.prepareJobs();
            renderBinJobs.append(builder.buildJobHierachy());
        }

        // Set target number of RenderViews
        m_renderQueue->setTargetRenderViewCount(fgBranchCount);
    } else {
        // FilterLayerEntityJob is part of the RenderViewBuilder jobs and must be run later
        // if none of those jobs are started this frame
        notCleared |= AbstractRenderer::EntityEnabledDirty;
        notCleared |= AbstractRenderer::FrameGraphDirty;
        notCleared |= AbstractRenderer::LayersDirty;
    }

    if (isRunning() && m_submissionContext->isInitialized()) {
        if (dirtyBitsForFrame & AbstractRenderer::TechniquesDirty )
            renderBinJobs.push_back(m_filterCompatibleTechniqueJob);
        if (dirtyBitsForFrame & AbstractRenderer::ShadersDirty)
            renderBinJobs.push_back(m_introspectShaderJob);
    } else {
        notCleared |= AbstractRenderer::TechniquesDirty;
        notCleared |= AbstractRenderer::ShadersDirty;
    }

    m_dirtyBits.remaining = dirtyBitsForFrame & notCleared;

    return renderBinJobs;
}

QAspectJobPtr Renderer::pickBoundingVolumeJob()
{
    // Set values on pickBoundingVolumeJob
    RenderSettings *renderSetting = settings();
    if (renderSetting != nullptr) {
        m_pickBoundingVolumeJob->setRenderSettings(renderSetting);
        m_pickBoundingVolumeJob->setFrameGraphRoot(frameGraphRoot());
        m_pickBoundingVolumeJob->setMouseEvents(pendingPickingEvents());
        m_pickBoundingVolumeJob->setKeyEvents(pendingKeyEvents());
    }

    return m_pickBoundingVolumeJob;
}

QAspectJobPtr Renderer::rayCastingJob()
{
    // Set values on rayCastingJob
    RenderSettings *renderSetting = settings();
    if (renderSetting != nullptr) {
        m_rayCastingJob->setRenderSettings(renderSetting);
        m_rayCastingJob->setFrameGraphRoot(frameGraphRoot());
    }

    return m_rayCastingJob;
}

QAspectJobPtr Renderer::syncTextureLoadingJob()
{
    return m_syncTextureLoadingJob;
}

QAspectJobPtr Renderer::expandBoundingVolumeJob()
{
    return m_expandBoundingVolumeJob;
}

QAbstractFrameAdvanceService *Renderer::frameAdvanceService() const
{
    return static_cast<Qt3DCore::QAbstractFrameAdvanceService *>(m_vsyncFrameAdvanceService.data());
}

// Called by executeCommands
void Renderer::performDraw(RenderCommand *command)
{
    // Indirect Draw Calls
    if (command->m_drawIndirect) {

        // Bind the indirect draw buffer
        Buffer *indirectDrawBuffer = m_nodesManager->bufferManager()->data(command->m_indirectDrawBuffer);
        if (Q_UNLIKELY(indirectDrawBuffer == nullptr)) {
            qWarning() << "Invalid Indirect Draw Buffer - failed to retrieve Buffer";
            return;
        }

        // Get GLBuffer from Buffer;
        GLBuffer *indirectDrawGLBuffer = m_submissionContext->glBufferForRenderBuffer(indirectDrawBuffer, GLBuffer::DrawIndirectBuffer);
        if (Q_UNLIKELY(indirectDrawGLBuffer == nullptr)) {
            qWarning() << "Invalid Indirect Draw Buffer - failed to retrieve GLBuffer";
            return;
        }

        // Bind GLBuffer
        const bool successfullyBound = indirectDrawGLBuffer->bind(m_submissionContext.data(), GLBuffer::DrawIndirectBuffer);

        if (Q_LIKELY(successfullyBound)) {
            // TO DO: Handle multi draw variants if attribute count > 1
            if (command->m_drawIndexed) {
                m_submissionContext->drawElementsIndirect(command->m_primitiveType,
                                                        command->m_indexAttributeDataType,
                                                        reinterpret_cast<void*>(quintptr(command->m_indirectAttributeByteOffset)));
            } else {
                m_submissionContext->drawArraysIndirect(command->m_primitiveType,
                                                      reinterpret_cast<void*>(quintptr(command->m_indirectAttributeByteOffset)));
            }
        } else {
            qWarning() << "Failed to bind IndirectDrawBuffer";
        }

    } else { // Direct Draw Calls

        // TO DO: Add glMulti Draw variants
        if (command->m_primitiveType == QGeometryRenderer::Patches)
            m_submissionContext->setVerticesPerPatch(command->m_verticesPerPatch);

        if (command->m_primitiveRestartEnabled)
            m_submissionContext->enablePrimitiveRestart(command->m_restartIndexValue);

        // TO DO: Add glMulti Draw variants
        if (command->m_drawIndexed) {
            Profiling::GLTimeRecorder recorder(Profiling::DrawElement);
            m_submissionContext->drawElementsInstancedBaseVertexBaseInstance(command->m_primitiveType,
                                                                           command->m_primitiveCount,
                                                                           command->m_indexAttributeDataType,
                                                                           reinterpret_cast<void*>(quintptr(command->m_indexAttributeByteOffset)),
                                                                           command->m_instanceCount,
                                                                           command->m_indexOffset,
                                                                           command->m_firstInstance);
        } else {
            Profiling::GLTimeRecorder recorder(Profiling::DrawArray);
            m_submissionContext->drawArraysInstancedBaseInstance(command->m_primitiveType,
                                                               command->m_firstVertex,
                                                               command->m_primitiveCount,
                                                               command->m_instanceCount,
                                                               command->m_firstInstance);
        }
    }

#if defined(QT3D_RENDER_ASPECT_OPENGL_DEBUG)
    int err = m_submissionContext->openGLContext()->functions()->glGetError();
    if (err)
        qCWarning(Rendering) << "GL error after drawing mesh:" << QString::number(err, 16);
#endif

    if (command->m_primitiveRestartEnabled)
        m_submissionContext->disablePrimitiveRestart();
}

void Renderer::performCompute(const RenderView *, RenderCommand *command)
{
    {
        Profiling::GLTimeRecorder recorder(Profiling::ShaderUpdate);
        m_submissionContext->activateShader(command->m_shaderDna);
    }
    {
        Profiling::GLTimeRecorder recorder(Profiling::UniformUpdate);
        m_submissionContext->setParameters(command->m_parameterPack);
    }
    {
        Profiling::GLTimeRecorder recorder(Profiling::DispatchCompute);
        m_submissionContext->dispatchCompute(command->m_workGroups[0],
                command->m_workGroups[1],
                command->m_workGroups[2]);
    }
    // HACK: Reset the compute flag to dirty
    m_dirtyBits.marked |= AbstractRenderer::ComputeDirty;

#if defined(QT3D_RENDER_ASPECT_OPENGL_DEBUG)
    int err = m_submissionContext->openGLContext()->functions()->glGetError();
    if (err)
        qCWarning(Rendering) << "GL error after drawing mesh:" << QString::number(err, 16);
#endif
}

void Renderer::createOrUpdateVAO(RenderCommand *command,
                                 HVao *previousVaoHandle,
                                 OpenGLVertexArrayObject **vao)
{
    const VAOIdentifier vaoKey(command->m_geometry, command->m_shader);

    VAOManager *vaoManager = m_nodesManager->vaoManager();
    command->m_vao = vaoManager->lookupHandle(vaoKey);

    if (command->m_vao.isNull()) {
        qCDebug(Rendering) << Q_FUNC_INFO << "Allocating new VAO";
        command->m_vao = vaoManager->getOrAcquireHandle(vaoKey);
        vaoManager->data(command->m_vao)->create(m_submissionContext.data(), vaoKey);
    }

    if (*previousVaoHandle != command->m_vao) {
        *previousVaoHandle = command->m_vao;
        *vao = vaoManager->data(command->m_vao);
    }
    Q_ASSERT(*vao);
}

// Called by RenderView->submit() in RenderThread context
// Returns true, if all RenderCommands were sent to the GPU
bool Renderer::executeCommandsSubmission(const RenderView *rv)
{
    bool allCommandsIssued = true;

    // Render drawing commands
    const QVector<RenderCommand *> commands = rv->commands();

    // Use the graphicscontext to submit the commands to the underlying
    // graphics API (OpenGL)

    // Save the RenderView base stateset
    RenderStateSet *globalState = m_submissionContext->currentStateSet();
    OpenGLVertexArrayObject *vao = nullptr;

    for (RenderCommand *command : qAsConst(commands)) {

        if (command->m_type == RenderCommand::Compute) { // Compute Call
            performCompute(rv, command);
        } else { // Draw Command
            // Check if we have a valid command that can be drawn
            if (!command->m_isValid) {
                allCommandsIssued = false;
                continue;
            }

            vao = m_nodesManager->vaoManager()->data(command->m_vao);

            // something may have went wrong when initializing the VAO
            if (!vao->isSpecified()) {
                allCommandsIssued = false;
                continue;
            }

            {
                Profiling::GLTimeRecorder recorder(Profiling::ShaderUpdate);
                //// We activate the shader here
                if (!m_submissionContext->activateShader(command->m_shaderDna)) {
                    allCommandsIssued = false;
                    continue;
                }
            }

            {
                Profiling::GLTimeRecorder recorder(Profiling::VAOUpdate);
                // Bind VAO
                vao->bind();
            }

            {
                Profiling::GLTimeRecorder recorder(Profiling::UniformUpdate);
                //// Update program uniforms
                if (!m_submissionContext->setParameters(command->m_parameterPack)) {
                    allCommandsIssued = false;
                    // If we have failed to set uniform (e.g unable to bind a texture)
                    // we won't perform the draw call which could show invalid content
                    continue;
                }
            }

            //// OpenGL State
            // TO DO: Make states not dependendent on their backend node for this step
            // Set state
            RenderStateSet *localState = command->m_stateSet;


            {
                Profiling::GLTimeRecorder recorder(Profiling::StateUpdate);
                // Merge the RenderCommand state with the globalState of the RenderView
                // Or restore the globalState if no stateSet for the RenderCommand
                if (localState != nullptr) {
                    command->m_stateSet->merge(globalState);
                    m_submissionContext->setCurrentStateSet(command->m_stateSet);
                } else {
                    m_submissionContext->setCurrentStateSet(globalState);
                }
            }
            // All Uniforms for a pass are stored in the QUniformPack of the command
            // Uniforms for Effect, Material and Technique should already have been correctly resolved
            // at that point

            //// Draw Calls
            performDraw(command);
        }
    } // end of RenderCommands loop

    // We cache the VAO and release it only at the end of the exectute frame
    // We try to minimize VAO binding between RenderCommands
    if (vao)
        vao->release();

    // Reset to the state we were in before executing the render commands
    m_submissionContext->setCurrentStateSet(globalState);

    return allCommandsIssued;
}

bool Renderer::updateVAOWithAttributes(Geometry *geometry,
                                       RenderCommand *command,
                                       Shader *shader,
                                       bool forceUpdate)
{
    m_dirtyAttributes.reserve(m_dirtyAttributes.size() + geometry->attributes().size());
    const auto attributeIds = geometry->attributes();

    for (QNodeId attributeId : attributeIds) {
        // TO DO: Improvement we could store handles and use the non locking policy on the attributeManager
        Attribute *attribute = m_nodesManager->attributeManager()->lookupResource(attributeId);

        if (attribute == nullptr)
            return false;

        Buffer *buffer = m_nodesManager->bufferManager()->lookupResource(attribute->bufferId());

        // Buffer update was already performed at this point
        // Just make sure the attribute reference a valid buffer
        if (buffer == nullptr)
            return false;

        // Index Attribute
        bool attributeWasDirty = false;
        if (attribute->attributeType() == QAttribute::IndexAttribute) {
            if ((attributeWasDirty = attribute->isDirty()) == true || forceUpdate)
                m_submissionContext->specifyIndices(buffer);
            // Vertex Attribute
        } else if (command->m_attributes.contains(attribute->nameId())) {
            if ((attributeWasDirty = attribute->isDirty()) == true || forceUpdate) {
                // Find the location for the attribute
                const QVector<ShaderAttribute> shaderAttributes = shader->attributes();
                const ShaderAttribute *attributeDescription = nullptr;
                for (const ShaderAttribute &shaderAttribute : shaderAttributes) {
                    if (shaderAttribute.m_nameId == attribute->nameId()) {
                        attributeDescription = &shaderAttribute;
                        break;
                    }
                }
                if (!attributeDescription || attributeDescription->m_location < 0)
                    return false;
                m_submissionContext->specifyAttribute(attribute, buffer, attributeDescription);
            }
        }

        // Append attribute to temporary vector so that its dirtiness
        // can be cleared at the end of the frame
        if (attributeWasDirty)
            m_dirtyAttributes.push_back(attribute);

        // Note: We cannot call unsertDirty on the Attribute at this
        // point as we don't know if the attributes are being shared
        // with other geometry / geometryRenderer in which case they still
        // should remain dirty so that VAO for these commands are properly
        // updated
    }

    return true;
}

bool Renderer::requiresVAOAttributeUpdate(Geometry *geometry,
                                          RenderCommand *command) const
{
    const auto attributeIds = geometry->attributes();

    for (QNodeId attributeId : attributeIds) {
        // TO DO: Improvement we could store handles and use the non locking policy on the attributeManager
        Attribute *attribute = m_nodesManager->attributeManager()->lookupResource(attributeId);

        if (attribute == nullptr)
            continue;

        if ((attribute->attributeType() == QAttribute::IndexAttribute && attribute->isDirty()) ||
                (command->m_attributes.contains(attribute->nameId()) && attribute->isDirty()))
            return true;
    }
    return false;
}

// Erase graphics related resources that may become unused after a frame
void Renderer::cleanGraphicsResources()
{
    // Clean buffers
    const QVector<Qt3DCore::QNodeId> buffersToRelease = m_nodesManager->bufferManager()->takeBuffersToRelease();
    for (Qt3DCore::QNodeId bufferId : buffersToRelease)
        m_submissionContext->releaseBuffer(bufferId);

    // Delete abandoned textures
    const QVector<GLTexture*> abandonedTextures = m_nodesManager->glTextureManager()->takeAbandonedTextures();
    for (GLTexture *tex : abandonedTextures) {
        tex->destroyGLTexture();
        delete tex;
    }

    // Delete abandoned VAOs
    m_abandonedVaosMutex.lock();
    const QVector<HVao> abandonedVaos = std::move(m_abandonedVaos);
    m_abandonedVaosMutex.unlock();
    for (const HVao &vaoHandle : abandonedVaos) {
        // might have already been destroyed last frame, but added by the cleanup job before, so
        // check if the VAO is really still existent
        OpenGLVertexArrayObject *vao = m_nodesManager->vaoManager()->data(vaoHandle);
        if (vao) {
            vao->destroy();
            // We remove VAO from manager using its VAOIdentifier
            m_nodesManager->vaoManager()->releaseResource(vao->key());
        }
    }
}

QList<QPair<QObject *, QMouseEvent>> Renderer::pendingPickingEvents() const
{
    return m_pickEventFilter->pendingMouseEvents();
}

QList<QKeyEvent> Renderer::pendingKeyEvents() const
{
    return m_pickEventFilter->pendingKeyEvents();
}

const GraphicsApiFilterData *Renderer::contextInfo() const
{
    return m_submissionContext->contextInfo();
}

SubmissionContext *Renderer::submissionContext() const
{
    return m_submissionContext.data();
}

void Renderer::addRenderCaptureSendRequest(Qt3DCore::QNodeId nodeId)
{
    if (!m_pendingRenderCaptureSendRequests.contains(nodeId))
        m_pendingRenderCaptureSendRequests.push_back(nodeId);
}

const QVector<Qt3DCore::QNodeId> Renderer::takePendingRenderCaptureSendRequests()
{
    return std::move(m_pendingRenderCaptureSendRequests);
}

// Returns a vector of jobs to be performed for dirty buffers
// 1 dirty buffer == 1 job, all job can be performed in parallel
QVector<Qt3DCore::QAspectJobPtr> Renderer::createRenderBufferJobs() const
{
    const QVector<QNodeId> dirtyBuffers = m_nodesManager->bufferManager()->takeDirtyBuffers();
    QVector<QAspectJobPtr> dirtyBuffersJobs;
    dirtyBuffersJobs.reserve(dirtyBuffers.size());

    for (const QNodeId bufId : dirtyBuffers) {
        Render::HBuffer bufferHandle = m_nodesManager->lookupHandle<Render::Buffer, Render::BufferManager, Render::HBuffer>(bufId);
        if (!bufferHandle.isNull()) {
            // Create new buffer job
            auto job = Render::LoadBufferJobPtr::create(bufferHandle);
            job->setNodeManager(m_nodesManager);
            dirtyBuffersJobs.push_back(job);
        }
    }

    return dirtyBuffersJobs;
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
