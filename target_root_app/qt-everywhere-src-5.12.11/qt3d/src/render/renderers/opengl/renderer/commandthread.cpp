/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
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

#include "commandthread_p.h"
#include <Qt3DRender/private/glcommands_p.h>
#include <Qt3DRender/private/offscreensurfacehelper_p.h>
#include <Qt3DRender/private/graphicscontext_p.h>
#include <Qt3DRender/private/shadercache_p.h>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QDebug>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

namespace Render {

CommandThread::CommandThread(Renderer *renderer)
    : QThread()
    , m_renderer(renderer)
    , m_waitForStartSemaphore(0)
    , m_initializedSemaphore(0)
    , m_commandRequestedSemaphore(0)
    , m_commandExecutionSemaphore(0)
    , m_mainContext(nullptr)
    , m_shaderCache(nullptr)
    , m_offsreenSurfaceHelper(nullptr)
    , m_currentCommand(nullptr)
    , m_running(0)
{
}

CommandThread::~CommandThread()
{
    Q_ASSERT(!isRunning());
}

void CommandThread::setShaderCache(ShaderCache *shaderCache)
{
    m_shaderCache = shaderCache;
}

// Called by RenderThread or MainThread (Scene3d)
void CommandThread::initialize(QOpenGLContext *mainContext, OffscreenSurfaceHelper *offsreenSurfaceHelper)
{
    // Start the thread
    start();

    // Wait for thread to be started
    m_waitForStartSemaphore.acquire();

    m_mainContext = mainContext;
    m_offsreenSurfaceHelper = offsreenSurfaceHelper;
    Q_ASSERT(m_mainContext && offsreenSurfaceHelper);

    // Initialize shared context and resources for the thread. This must be
    // done here since some platforms do not allow context sharing to be set up
    // with contexts created on different threads. (Windows with WGL, in
    // particular; resource sharing works fine later on, what matters is the
    // thread the wglShareLists call is made on)
    m_localContext.reset(new QOpenGLContext());
    m_localContext->setFormat(m_mainContext->format());
    m_localContext->setScreen(m_mainContext->screen());
    m_localContext->setShareContext(m_mainContext);
    if (!m_localContext->create())
        qWarning("CommandThread: Failed to create local context");
    m_localContext->moveToThread(this);

    m_running.fetchAndStoreOrdered(1);

    // Allow thread to proceed
    m_initializedSemaphore.release();
}

// Called by RenderThread or MainThread (Scene3D)
void CommandThread::shutdown()
{
    m_running.fetchAndStoreOrdered(0);

    // Unblock thread
    m_commandRequestedSemaphore.release(1);

    // Wait for thread to exit
    wait();

    // Reset semaphores (in case we ever want to restart)
    m_waitForStartSemaphore.acquire(m_waitForStartSemaphore.available());
    m_initializedSemaphore.acquire(m_initializedSemaphore.available());
    m_commandRequestedSemaphore.acquire(m_commandRequestedSemaphore.available());
    m_commandExecutionSemaphore.acquire(m_commandExecutionSemaphore.available());
    m_localContext.reset();
}

// Any thread can call this, this is a blocking command
void CommandThread::executeCommand(GLCommand *command)
{
    if (!isRunning())
        return;

    // We lock to prevent any other call to executeCommand to be executed
    // before we have received the result of our command
    m_blockingCallerMutex.lock();

    // Store command to be executed
    m_currentCommand = command;

    // Allow thread to proceed and execute command
    m_commandRequestedSemaphore.release();

    // Wait for thread to be done
    m_commandExecutionSemaphore.acquire();

    // Reset command
    m_currentCommand = nullptr;

    // Unlock blocking semaphore so that other calls to executeCommand
    // can proceed
    m_blockingCallerMutex.unlock();
}

void CommandThread::run()
{
    // Allow initialize to proceed
    m_waitForStartSemaphore.release();

    // Wait for initialize to be completed
    m_initializedSemaphore.acquire();

    Q_ASSERT(m_mainContext && m_shaderCache);

    // Initialize GraphicsContext
    m_graphicsContext.reset(new GraphicsContext());
    m_graphicsContext->setShaderCache(m_shaderCache);
    m_graphicsContext->setOpenGLContext(m_localContext.data());

    bool initialized = false;
    while (true) {

        // Wait for command
        m_commandRequestedSemaphore.acquire();

        // Are we still running?
        if (!m_running.load()) {
            m_graphicsContext->doneCurrent();
            // to prevent executeCommand being locked
            m_commandExecutionSemaphore.release();
            break;
        }

        if (Q_UNLIKELY(!initialized)) {
            QOffscreenSurface *offscreenSurface = m_offsreenSurfaceHelper->offscreenSurface();
            Q_ASSERT(offscreenSurface);
            m_graphicsContext->makeCurrent(offscreenSurface);
            initialized = true;
        }

        m_currentCommand->execute(m_renderer, m_graphicsContext.data());

        // Allow caller to proceed as we are done with the command
        m_commandExecutionSemaphore.release();
    }
}

} // Render

} // Qt3DRender

QT_END_NAMESPACE
