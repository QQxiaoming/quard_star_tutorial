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

#include "scenemanager_p.h"

QT_BEGIN_NAMESPACE

namespace Qt3DRender {
namespace Render {

SceneManager::SceneManager()
    : Qt3DCore::QResourceManager<Scene,
                                 Qt3DCore::QNodeId,
                                 Qt3DCore::ObjectLevelLockingPolicy>()
    , m_service(nullptr)
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::setDownloadService(Qt3DCore::QDownloadHelperService *service)
{
    m_service = service;
}

void SceneManager::addSceneData(const QUrl &source,
                                Qt3DCore::QNodeId sceneUuid,
                                const QByteArray &data)
{
    LoadSceneJobPtr newJob(new LoadSceneJob(source, sceneUuid));

    if (!data.isEmpty())
        newJob->setData(data);

    // We cannot run two jobs that use the same scene loader plugin
    // in two different threads at the same time
    if (!m_pendingJobs.isEmpty())
        newJob->addDependency(m_pendingJobs.last());

    m_pendingJobs.push_back(newJob);
}

QVector<LoadSceneJobPtr> SceneManager::takePendingSceneLoaderJobs()
{
    // Explicitly use std::move to clear the m_pendingJobs vector
    return std::move(m_pendingJobs);
}

void SceneManager::startSceneDownload(const QUrl &source, Qt3DCore::QNodeId sceneUuid)
{
    if (!m_service)
        return;
    SceneDownloaderPtr request = SceneDownloaderPtr::create(source, sceneUuid, this);
    m_pendingDownloads << request;
    m_service->submitRequest(request);
}

void SceneManager::clearSceneDownload(SceneDownloader *downloader)
{
    for (auto it = m_pendingDownloads.begin(); it != m_pendingDownloads.end(); ++it) {
        if ((*it).data() == downloader) {
            m_pendingDownloads.erase(it);
            return;
        }
    }
}


SceneDownloader::SceneDownloader(const QUrl &source, Qt3DCore::QNodeId sceneComponent, SceneManager *manager)
    : Qt3DCore::QDownloadRequest(source)
    , m_sceneComponent(sceneComponent)
    , m_manager(manager)
{

}

void SceneDownloader::onCompleted()
{
    if (!m_manager)
        return;
    if (succeeded())
        m_manager->addSceneData(url(), m_sceneComponent, m_data);
    else
        qWarning() << "Failed to download scene at" << url();
    m_manager->clearSceneDownload(this);
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
