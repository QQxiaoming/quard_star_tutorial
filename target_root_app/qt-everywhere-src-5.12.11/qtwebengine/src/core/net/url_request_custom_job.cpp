/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "url_request_custom_job.h"
#include "url_request_custom_job_proxy.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/io_buffer.h"

#include <QIODevice>

using namespace net;

namespace QtWebEngineCore {

URLRequestCustomJob::URLRequestCustomJob(URLRequest *request,
                                         NetworkDelegate *networkDelegate,
                                         const std::string &scheme,
                                         QPointer<ProfileAdapter> profileAdapter)
    : URLRequestJob(request, networkDelegate)
    , m_proxy(new URLRequestCustomJobProxy(this, scheme, profileAdapter))
    , m_device(nullptr)
    , m_error(0)
    , m_pendingReadSize(0)
    , m_pendingReadPos(0)
    , m_pendingReadBuffer(nullptr)
{
}

URLRequestCustomJob::~URLRequestCustomJob()
{
    m_proxy->m_job = nullptr;
    if (m_device && m_device->isOpen())
        m_device->close();
    m_device = nullptr;
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
                                     base::Bind(&URLRequestCustomJobProxy::release,
                                     m_proxy));
}

void URLRequestCustomJob::Start()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
                                     base::Bind(&URLRequestCustomJobProxy::initialize,
                                     m_proxy, request()->url(), request()->method(), request()->initiator()));
}

void URLRequestCustomJob::Kill()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    m_proxy->m_job = nullptr;
    if (m_device && m_device->isOpen())
        m_device->close();
    if (m_pendingReadBuffer) {
        m_pendingReadBuffer->Release();
        m_pendingReadBuffer = nullptr;
        m_pendingReadSize = 0;
        m_pendingReadPos = 0;
    }
    m_device = nullptr;
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
                                     base::Bind(&URLRequestCustomJobProxy::release,
                                     m_proxy));
    URLRequestJob::Kill();
}

bool URLRequestCustomJob::GetMimeType(std::string *mimeType) const
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (m_mimeType.size() > 0) {
        *mimeType = m_mimeType;
        return true;
    }
    return false;
}

bool URLRequestCustomJob::GetCharset(std::string* charset)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (m_charset.size() > 0) {
        *charset = m_charset;
        return true;
    }
    return false;
}

bool URLRequestCustomJob::IsRedirectResponse(GURL* location, int* http_status_code, bool* /*insecure_scheme_was_upgraded*/)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (m_redirect.is_valid()) {
        *location = m_redirect;
        *http_status_code = 303;
        return true;
    }
    return false;
}

int URLRequestCustomJob::ReadRawData(IOBuffer *buf, int bufSize)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (m_error)
        return m_error;
    qint64 rv = m_device ? m_device->read(buf->data(), bufSize) : -1;
    if (rv > 0) {
        return static_cast<int>(rv);
    } else if (rv == 0) {
        // Returning zero is interpreted as EOF by Chromium, so only
        // return zero if we are the end of the file.
        if (m_device->atEnd())
            return 0;
        // Otherwise return IO_PENDING and call ReadRawDataComplete when we have data
        // for them.
        buf->AddRef();
        m_pendingReadPos = 0;
        m_pendingReadSize = bufSize;
        m_pendingReadBuffer = buf;
        return ERR_IO_PENDING;
    } else {
        // QIODevice::read might have called fail on us.
        if (m_error)
            return m_error;
        if (m_device && m_device->atEnd())
            return 0;
        return ERR_FAILED;
    }
}

void URLRequestCustomJob::notifyReadyRead()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (!m_device)
        return;
    if (!m_pendingReadSize)
        return;
    Q_ASSERT(m_pendingReadBuffer);
    if (!m_pendingReadBuffer)
        return;

    qint64 rv = m_device->read(m_pendingReadBuffer->data() + m_pendingReadPos, m_pendingReadSize - m_pendingReadPos);
    if (rv == 0)
        return;
    if (rv < 0) {
        if (m_error)
            rv = m_error;
        else if (m_device->atEnd())
            rv = 0;
        else
            rv = ERR_FAILED;
    } else {
        m_pendingReadPos += rv;
        if (m_pendingReadPos < m_pendingReadSize && !m_device->atEnd())
            return;
        rv = m_pendingReadPos;
    }
    // killJob may be called from ReadRawDataComplete
    net::IOBuffer *buf = m_pendingReadBuffer;
    m_pendingReadBuffer = nullptr;
    m_pendingReadSize = 0;
    m_pendingReadPos = 0;
    ReadRawDataComplete(rv);
    buf->Release();
}

} // namespace
