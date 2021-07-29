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

#include "loadscenejob_p.h"
#include <private/renderer_p.h>
#include <private/nodemanagers_p.h>
#include <private/scenemanager_p.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DRender/private/job_common_p.h>
#include <Qt3DRender/private/qsceneimporter_p.h>
#include <Qt3DRender/private/qurlhelper_p.h>
#include <Qt3DRender/qsceneloader.h>
#include <Qt3DRender/private/renderlogging_p.h>
#include <QFileInfo>
#include <QMimeDatabase>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {
namespace Render {

LoadSceneJob::LoadSceneJob(const QUrl &source, Qt3DCore::QNodeId m_sceneComponent)
    : QAspectJob()
    , m_source(source)
    , m_sceneComponent(m_sceneComponent)
    , m_managers(nullptr)
{
    SET_JOB_RUN_STAT_TYPE(this, JobTypes::LoadScene, 0);
}

void LoadSceneJob::setData(const QByteArray &data)
{
    m_data = data;
}

NodeManagers *LoadSceneJob::nodeManagers() const
{
    return m_managers;
}

QList<QSceneImporter *> LoadSceneJob::sceneImporters() const
{
    return m_sceneImporters;
}

QUrl LoadSceneJob::source() const
{
    return m_source;
}

Qt3DCore::QNodeId LoadSceneJob::sceneComponentId() const
{
    return m_sceneComponent;
}

void LoadSceneJob::run()
{
    // Iterate scene IO handlers until we find one that can handle this file type
    Qt3DCore::QEntity *sceneSubTree = nullptr;
    Scene *scene = m_managers->sceneManager()->lookupResource(m_sceneComponent);
    Q_ASSERT(scene);

    // Reset status
    scene->setStatus(QSceneLoader::None);
    QSceneLoader::Status finalStatus = QSceneLoader::None;

    // Perform the loading only if the source wasn't explicitly set to empty
    if (!m_source.isEmpty()) {
        finalStatus = QSceneLoader::Error;

        if (m_data.isEmpty()) {
            const QString path = QUrlHelper::urlToLocalFileOrQrc(m_source);
            const QFileInfo finfo(path);
            qCDebug(SceneLoaders) << Q_FUNC_INFO << "Attempting to load" << finfo.filePath();
            if (finfo.exists()) {
                const QStringList extensions(finfo.suffix());
                sceneSubTree = tryLoadScene(scene,
                                            finalStatus,
                                            extensions,
                                            [this] (QSceneImporter *importer) {
                        importer->setSource(m_source);
            });
            } else {
                qCWarning(SceneLoaders) << Q_FUNC_INFO << finfo.filePath() << "doesn't exist";
            }
        } else {
            QStringList extensions;
            QMimeDatabase db;
            const QMimeType mtype = db.mimeTypeForData(m_data);

            if (mtype.isValid())
                extensions = mtype.suffixes();
            else
                qCWarning(SceneLoaders) << Q_FUNC_INFO << "Invalid mime type" << mtype;

            const QString basePath = m_source.adjusted(QUrl::RemoveFilename).toString();

            sceneSubTree = tryLoadScene(scene,
                                        finalStatus,
                                        extensions,
                                        [this, basePath] (QSceneImporter *importer) {
                importer->setData(m_data, basePath);
            });
        }
    }

    // If the sceneSubTree is null it will trigger the frontend to unload
    // any subtree it may hold
    // Set clone of sceneTree in sceneComponent. This will move the sceneSubTree
    // to the QCoreApplication thread which is where the frontend object tree lives.
    scene->setSceneSubtree(sceneSubTree);

    // Note: the status is set after the subtree so that bindinds depending on the status
    // in the frontend will be consistent
    scene->setStatus(finalStatus);
}

Qt3DCore::QEntity *LoadSceneJob::tryLoadScene(Scene *scene,
                                              QSceneLoader::Status &finalStatus,
                                              const QStringList &extensions,
                                              const std::function<void (QSceneImporter *)> &importerSetupFunc)
{
    Qt3DCore::QEntity *sceneSubTree = nullptr;
    bool foundSuitableLoggerPlugin = false;

    for (QSceneImporter *sceneImporter : qAsConst(m_sceneImporters)) {
        if (!sceneImporter->areFileTypesSupported(extensions))
            continue;

        foundSuitableLoggerPlugin = true;

        // If the file type is supported -> enter Loading status
        scene->setStatus(QSceneLoader::Loading);

        // Set source file or data on importer
        importerSetupFunc(sceneImporter);

        // File type is supported, try to load it
        sceneSubTree = sceneImporter->scene();
        if (sceneSubTree != nullptr) {
            // Successfully built a subtree
            finalStatus = QSceneLoader::Ready;
            break;
        }

        qCWarning(SceneLoaders) << Q_FUNC_INFO << "Failed to import" << m_source << "with errors" << sceneImporter->errors();
    }

    if (!foundSuitableLoggerPlugin)
        qCWarning(SceneLoaders) << Q_FUNC_INFO << "Found no suitable importer plugin for" << m_source;

    return sceneSubTree;
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
