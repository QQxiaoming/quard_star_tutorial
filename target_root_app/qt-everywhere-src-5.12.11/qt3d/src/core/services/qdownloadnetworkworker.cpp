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

#include "qdownloadhelperservice_p.h"
#include "qdownloadnetworkworker_p.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDataStream>

QT_BEGIN_NAMESPACE

namespace Qt3DCore {

QDownloadNetworkWorker::QDownloadNetworkWorker(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
{
    connect(this, &QDownloadNetworkWorker::submitRequest,
            this, &QDownloadNetworkWorker::onRequestSubmited);
    connect(this, &QDownloadNetworkWorker::cancelRequest,
            this, &QDownloadNetworkWorker::onRequestCancelled);
    connect(this, &QDownloadNetworkWorker::cancelAllRequests,
            this, &QDownloadNetworkWorker::onAllRequestsCancelled);
}

void QDownloadNetworkWorker::onRequestSubmited(const QDownloadRequestPtr &request)
{
    QMutexLocker l(&m_mutex);
    if (!m_networkManager) {
        m_networkManager = new QNetworkAccessManager(this);
        connect(m_networkManager, &QNetworkAccessManager::finished,
                  this, &QDownloadNetworkWorker::onRequestFinished);
    }
    auto reply = m_networkManager->get(QNetworkRequest(request->url()));
    m_requests << QPair<QDownloadRequestPtr, QNetworkReply *>(request, reply);
    connect(reply, &QNetworkReply::downloadProgress, this, &QDownloadNetworkWorker::onDownloadProgressed);
}

void QDownloadNetworkWorker::onRequestCancelled(const QDownloadRequestPtr &request)
{
    QMutexLocker l(&m_mutex);
    auto it = std::find_if(m_requests.begin(), m_requests.end(),
                           [request](QPair<QDownloadRequestPtr, QNetworkReply *> e) {
                                        return e.first == request;
                            });
    if (it == m_requests.end())
        return;

    (*it).first->m_cancelled = true;
    (*it).second->abort();
}

void QDownloadNetworkWorker::onAllRequestsCancelled()
{
    QMutexLocker l(&m_mutex);
    for (auto &e: qAsConst(m_requests)) {
        e.first->m_cancelled = true;
        e.second->abort();
    }
    m_requests.clear();
}

void QDownloadNetworkWorker::onRequestFinished(QNetworkReply *reply)
{
    QMutexLocker l(&m_mutex);
    auto it = std::find_if(m_requests.begin(), m_requests.end(),
                           [reply](QPair<QDownloadRequestPtr, QNetworkReply *> e) {
                                        return e.second == reply;
                            });
    if (it == m_requests.end())
        return;

    auto request = (*it).first;
    if (reply->error() == QNetworkReply::NoError) {
        request->m_succeeded = true;
    }
    request->onDownloaded();
    emit requestDownloaded(request);

    m_requests.erase(it);
}

void QDownloadNetworkWorker::onDownloadProgressed(qint64 bytesReceived, qint64 bytesTotal)
{
    Q_UNUSED(bytesReceived);
    Q_UNUSED(bytesTotal);
    // TODO forward progress details somewhere

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    QMutexLocker l(&m_mutex);
    auto it = std::find_if(m_requests.begin(), m_requests.end(),
                           [reply](QPair<QDownloadRequestPtr, QNetworkReply *> e) {
                                        return e.second == reply;
                            });
    if (it == m_requests.end())
        return;

    auto request = (*it).first;
    QDataStream stream(&request->m_data, QIODevice::Append);
    QByteArray data = reply->readAll();
    stream.writeRawData(data.data(), data.size());
}

} // namespace Qt3DCore

QT_END_NAMESPACE

