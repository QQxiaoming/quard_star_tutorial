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

#include "qmesh.h"
#include "qmesh_p.h"

#include <QtCore/private/qfactoryloader_p.h>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QScopedPointer>
#include <QMimeDatabase>
#include <QMimeType>
#include <QtCore/QBuffer>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DCore/QAspectEngine>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DCore/private/qdownloadhelperservice_p.h>
#include <Qt3DRender/private/qrenderaspect_p.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/qgeometryloaderinterface_p.h>
#include <Qt3DRender/private/renderlogging_p.h>
#include <Qt3DRender/private/qurlhelper_p.h>
#include <Qt3DRender/private/qgeometryloaderfactory_p.h>
#include <Qt3DRender/private/geometryrenderermanager_p.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, geometryLoader, (QGeometryLoaderFactory_iid, QLatin1String("/geometryloaders"), Qt::CaseInsensitive))

QMeshPrivate::QMeshPrivate()
    : QGeometryRendererPrivate()
    , m_status(QMesh::None)
{
}

QMeshPrivate *QMeshPrivate::get(QMesh *q)
{
    return q->d_func();
}

void QMeshPrivate::setScene(Qt3DCore::QScene *scene)
{
    QGeometryRendererPrivate::setScene(scene);
    updateFunctor();
}

void QMeshPrivate::updateFunctor()
{
    Q_Q(QMesh);
    q->setGeometryFactory(QGeometryFactoryPtr(new MeshLoaderFunctor(q)));
}

void QMeshPrivate::setStatus(QMesh::Status status)
{
    if (m_status != status) {
        Q_Q(QMesh);
        m_status = status;
        const bool wasBlocked = q->blockNotifications(true);
        emit q->statusChanged(status);
        q->blockNotifications(wasBlocked);
    }
}

/*!
 * \qmltype Mesh
 * \instantiates Qt3DRender::QMesh
 * \inqmlmodule Qt3D.Render
 * \brief A custom mesh loader.
 *
 * Loads mesh data from external files in a variety of formats.
 *
 * In Qt3D 5.9, Mesh supports the following formats:
 *
 * \list
 * \li Wavefront OBJ
 * \li Stanford Triangle Format PLY
 * \li STL (STereoLithography)
 * \endlist
 *
 * QMesh will also support the following format if the SDK is installed and the fbx geometry loader plugin is built and found.
 * \list
 * \li Autodesk FBX
 * \endlist
 */

/*!
 * \qmlproperty url Mesh::source
 *
 * Holds the source url to the file containing the custom mesh.
 */

/*!
 * \qmlproperty string Mesh::meshName
 *
 * Filter indicating which part of the mesh should be loaded.
 *
 * If meshName is empty (the default), then the entire mesh is loaded.
 *
 * If meshName is a plain string, then only the sub-mesh matching that name, if present, will be loaded.
 *
 * If meshName is a regular expression, than all sub-meshes matching the expression will be loaded.
 *
 * \note Only Wavefront OBJ files support sub-meshes.
 *
 * \sa QRegularExpression
 */

/*!
    \enum QMesh::Status

    This enum identifies the status of shader used.

    \value None              A source mesh hasn't been assigned a source yet
    \value Loading           The mesh geometry is loading
    \value Ready             The mesh geometry was successfully loaded
    \value Error             An error occurred while loading the mesh
*/

/*!
    \qmlproperty enumeration Mesh::status

    Holds the status of the mesh loading.
    \sa Qt3DRender::QMesh::Status
    \readonly
 */

/*!
 * \class Qt3DRender::QMesh
 * \inheaderfile Qt3DRender/QMesh
 * \inmodule Qt3DRender
 *
 * \inherits Qt3DRender::QGeometryRenderer
 *
 * \brief A custom mesh loader.
 *
 * Loads mesh data from external files in a variety of formats.
 * Qt3DRender::QMesh loads data into a single mesh.
 *
 * In Qt3D 5.9, QMesh supports the following formats:
 *
 * \list
 * \li Wavefront OBJ
 * \li Stanford Triangle Format PLY
 * \li STL (STereoLithography)
 * \endlist
 *
 * QMesh will also support the following format if the SDK is installed and the fbx geometry loader plugin is built and found:
 * \list
 * \li Autodesk FBX
 * \endlist
 *
 * If you wish to load an entire scene made of several objects, you should rather use the Qt3DRender::QSceneLoader instead.
 *
 * \sa Qt3DRender::QSceneLoader
 */

/*!
 * Constructs a new QMesh with \a parent.
 */
QMesh::QMesh(QNode *parent)
    : QGeometryRenderer(*new QMeshPrivate, parent)
{
}

/*! \internal */
QMesh::~QMesh()
{
}

/*! \internal */
QMesh::QMesh(QMeshPrivate &dd, QNode *parent)
    : QGeometryRenderer(dd, parent)
{
}

/*! \internal */
void QMesh::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &change)
{
    Q_D(QMesh);
    if (change->type() == Qt3DCore::PropertyUpdated) {
        const Qt3DCore::QPropertyUpdatedChangePtr e = qSharedPointerCast<Qt3DCore::QPropertyUpdatedChange>(change);
        if (e->propertyName() == QByteArrayLiteral("status"))
            d->setStatus(e->value().value<QMesh::Status>());
    }
    Qt3DRender::QGeometryRenderer::sceneChangeEvent(change);
}

void QMesh::setSource(const QUrl& source)
{
    Q_D(QMesh);
    if (d->m_source == source)
        return;
    d->m_source = source;
    d->updateFunctor();
    const bool blocked = blockNotifications(true);
    emit sourceChanged(source);
    blockNotifications(blocked);
}

/*!
 * \property QMesh::source
 *
 * Holds the \a source url to the file containing the custom mesh.
 */
QUrl QMesh::source() const
{
    Q_D(const QMesh);
    return d->m_source;
}

void QMesh::setMeshName(const QString &meshName)
{
    Q_D(QMesh);
    if (d->m_meshName == meshName)
        return;
    d->m_meshName = meshName;
    d->updateFunctor();
    const bool blocked = blockNotifications(true);
    emit meshNameChanged(meshName);
    blockNotifications(blocked);
}

/*!
 * \property QMesh::meshName
 *
 * Holds the name of the mesh.
 */
QString QMesh::meshName() const
{
    Q_D(const QMesh);
    return d->m_meshName;
}

/*!
    \property QMesh::status

    Holds the status of the mesh loading.
    \sa Qt3DRender::QMesh::Status
 */
QMesh::Status QMesh::status() const
{
    Q_D(const QMesh);
    return d->m_status;
}

/*!
 * \internal
 */
MeshLoaderFunctor::MeshLoaderFunctor(QMesh *mesh, const QByteArray &sourceData)
    : QGeometryFactory()
    , m_mesh(mesh->id())
    , m_sourcePath(mesh->source())
    , m_meshName(mesh->meshName())
    , m_sourceData(sourceData)
    , m_nodeManagers(nullptr)
    , m_downloaderService(nullptr)
    , m_status(QMesh::None)
{
}

/*!
 * \internal
 */
QGeometry *MeshLoaderFunctor::operator()()
{
    m_status = QMesh::Loading;

    if (m_sourcePath.isEmpty()) {
        qCWarning(Render::Jobs) << Q_FUNC_INFO << "Mesh is empty, nothing to load";
        m_status = QMesh::Error;
        return nullptr;
    }

    QStringList ext;
    if (!Qt3DCore::QDownloadHelperService::isLocal(m_sourcePath)) {
        if (m_sourceData.isEmpty()) {
            if (m_mesh) {
                // Output a warning in the case a user is calling the functor directly
                // in the frontend
                if (m_nodeManagers == nullptr || m_downloaderService == nullptr) {
                    qWarning() << "Mesh source points to a remote URL. Remotes meshes can only be loaded if the geometry is processed by the Qt3DRender backend";
                    m_status = QMesh::Error;
                    return nullptr;
                }
                Qt3DCore::QDownloadRequestPtr request(new MeshDownloadRequest(m_mesh, m_sourcePath, m_nodeManagers));
                m_downloaderService->submitRequest(request);
            }
            return nullptr;
        }

        QMimeDatabase db;
        QMimeType mtype = db.mimeTypeForData(m_sourceData);
        if (mtype.isValid()) {
            ext = mtype.suffixes();
        }
        QFileInfo finfo(m_sourcePath.path());
        ext << finfo.suffix();
        ext.removeAll(QLatin1String(""));
        if (!ext.contains(QLatin1String("obj")))
            ext << QLatin1String("obj");
    } else {
        QString filePath = Qt3DRender::QUrlHelper::urlToLocalFileOrQrc(m_sourcePath);
        QFileInfo finfo(filePath);
        if (finfo.suffix().isEmpty())
            ext << QLatin1String("obj");
        else
            ext << finfo.suffix();
    }

    QScopedPointer<QGeometryLoaderInterface> loader;
    for (const QString &e: qAsConst(ext)) {
        loader.reset(qLoadPlugin<QGeometryLoaderInterface, QGeometryLoaderFactory>(geometryLoader(), e));
        if (loader)
            break;
    }
    if (!loader) {
        qCWarning(Render::Jobs, "unsupported format encountered (%s)", qPrintable(ext.join(QLatin1String(", "))));
        m_status = QMesh::Error;
        return nullptr;
    }

    if (m_sourceData.isEmpty()) {
        QString filePath = Qt3DRender::QUrlHelper::urlToLocalFileOrQrc(m_sourcePath);
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qCDebug(Render::Jobs) << "Could not open file" << filePath << "for reading";
            m_status = QMesh::Error;
            return nullptr;
        }

        if (loader->load(&file, m_meshName)) {
            Qt3DRender::QGeometry *geometry = loader->geometry();
            m_status = geometry != nullptr ? QMesh::Ready : QMesh::Error;
            return geometry;
        }
        qCWarning(Render::Jobs) << Q_FUNC_INFO << "Mesh loading failure for:" << filePath;
    } else {
        QT_PREPEND_NAMESPACE(QBuffer) buffer(&m_sourceData);
        if (!buffer.open(QIODevice::ReadOnly)) {
            m_status = QMesh::Error;
            return nullptr;
        }

        if (loader->load(&buffer, m_meshName)) {
            Qt3DRender::QGeometry *geometry = loader->geometry();
            m_status = geometry != nullptr ? QMesh::Ready : QMesh::Error;
            return geometry;
        }

        qCWarning(Render::Jobs) << Q_FUNC_INFO << "Mesh loading failure for:" << m_sourcePath;
    }

    return nullptr;
}

/*!
 * \internal
 */
bool MeshLoaderFunctor::operator ==(const QGeometryFactory &other) const
{
    const MeshLoaderFunctor *otherFunctor = functor_cast<MeshLoaderFunctor>(&other);
    if (otherFunctor != nullptr)
        return (otherFunctor->m_sourcePath == m_sourcePath &&
                otherFunctor->m_sourceData.isEmpty() == m_sourceData.isEmpty() &&
                otherFunctor->m_meshName == m_meshName &&
                otherFunctor->m_downloaderService == m_downloaderService &&
                otherFunctor->m_nodeManagers == m_nodeManagers);
    return false;
}

/*!
 * \internal
 */
MeshDownloadRequest::MeshDownloadRequest(Qt3DCore::QNodeId mesh, QUrl source, Render::NodeManagers *managers)
    : Qt3DCore::QDownloadRequest(source)
    , m_mesh(mesh)
    , m_nodeManagers(managers)
{
}

// Called in Aspect Thread context (not a Qt3D AspectJob)
// We are sure that when this is called, no AspectJob are running
void MeshDownloadRequest::onCompleted()
{
    if (cancelled() || !succeeded())
        return;

    if (!m_nodeManagers)
        return;

    Render::GeometryRenderer *renderer = m_nodeManagers->geometryRendererManager()->lookupResource(m_mesh);
    if (!renderer)
        return;

    QGeometryFactoryPtr geometryFactory = renderer->geometryFactory();
    if (!geometryFactory.isNull() && geometryFactory->id() == Qt3DRender::functorTypeId<MeshLoaderFunctor>()) {
        QSharedPointer<MeshLoaderFunctor> functor = qSharedPointerCast<MeshLoaderFunctor>(geometryFactory);

        // We make sure we are setting the result for the right request
        // (the functor for the mesh could have changed in the meantime)
        if (m_url == functor->sourcePath()) {
            functor->setSourceData(m_data);

            // mark the component as dirty so that the functor runs again in the correct job
            m_nodeManagers->geometryRendererManager()->addDirtyGeometryRenderer(m_mesh);
        }
    }
}

} // namespace Qt3DRender

QT_END_NAMESPACE
