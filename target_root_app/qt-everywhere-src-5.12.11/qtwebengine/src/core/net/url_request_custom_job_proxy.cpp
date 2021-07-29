/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "url_request_custom_job_proxy.h"
#include "url_request_custom_job.h"
#include "url_request_custom_job_delegate.h"
#include "api/qwebengineurlrequestjob.h"
#include "profile_adapter.h"
#include "type_conversion.h"
#include "content/public/browser/browser_thread.h"
#include "web_engine_context.h"

using namespace net;

namespace QtWebEngineCore {

URLRequestCustomJobProxy::URLRequestCustomJobProxy(URLRequestCustomJob *job,
                                                   const std::string &scheme,
                                                   QPointer<ProfileAdapter> profileAdapter)
    : m_job(job)
    , m_started(false)
    , m_scheme(scheme)
    , m_delegate(nullptr)
    , m_profileAdapter(profileAdapter)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
}

URLRequestCustomJobProxy::~URLRequestCustomJobProxy()
{
}

void URLRequestCustomJobProxy::release()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    if (m_delegate) {
        m_delegate->deleteLater();
        m_delegate = nullptr;
    }
}

// Fix me: this is  never used
/*
void URLRequestCustomJobProxy::setReplyCharset(const std::string &charset)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (!m_job)
        return;
    m_job->m_charset = charset;
}
*/
void URLRequestCustomJobProxy::reply(std::string mimeType, QIODevice *device)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (!m_job)
        return;
    m_job->m_mimeType = mimeType;
    m_job->m_device = device;
    if (m_job->m_device && !m_job->m_device->isReadable())
        m_job->m_device->open(QIODevice::ReadOnly);

    qint64 size = m_job->m_device ? m_job->m_device->size() : -1;
    if (size > 0)
        m_job->set_expected_content_size(size);
    if (m_job->m_device && m_job->m_device->isReadable()) {
        m_started = true;
        m_job->NotifyHeadersComplete();
    } else {
        fail(ERR_INVALID_URL);
    }
}

void URLRequestCustomJobProxy::redirect(GURL url)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (!m_job)
        return;
    if (m_job->m_device || m_job->m_error)
        return;
    m_job->m_redirect = url;
    m_started = true;
    m_job->NotifyHeadersComplete();
}

void URLRequestCustomJobProxy::abort()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (!m_job)
        return;
    if (m_job->m_device && m_job->m_device->isOpen())
        m_job->m_device->close();
    m_job->m_device = nullptr;
    if (m_started)
        m_job->NotifyCanceled();
    else
        m_job->NotifyStartError(URLRequestStatus(URLRequestStatus::CANCELED, ERR_ABORTED));
}

void URLRequestCustomJobProxy::fail(int error)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (!m_job)
        return;
    m_job->m_error = error;
    if (m_job->m_device)
        m_job->m_device->close();
    if (!m_started)
        m_job->NotifyStartError(URLRequestStatus::FromError(error));
    // else we fail on the next read, or the read that might already be in progress
}

void URLRequestCustomJobProxy::readyRead()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (m_job)
        m_job->notifyReadyRead();
}

void URLRequestCustomJobProxy::initialize(GURL url, std::string method, base::Optional<url::Origin> initiator)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    Q_ASSERT(!m_delegate);

    QUrl initiatorOrigin;
    if (initiator.has_value())
        initiatorOrigin = QUrl::fromEncoded(QByteArray::fromStdString(initiator.value().Serialize()));

    QWebEngineUrlSchemeHandler *schemeHandler = nullptr;

    if (m_profileAdapter)
        schemeHandler = m_profileAdapter->customUrlSchemeHandlers()[toQByteArray(m_scheme)];

    if (schemeHandler) {
        m_delegate = new URLRequestCustomJobDelegate(this, toQt(url),
                                                     QByteArray::fromStdString(method),
                                                     initiatorOrigin);
        QWebEngineUrlRequestJob *requestJob = new QWebEngineUrlRequestJob(m_delegate);
        schemeHandler->requestStarted(requestJob);
    }
}

} // namespace
