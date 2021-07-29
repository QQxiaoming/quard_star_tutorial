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

#ifndef URL_REQUEST_CUSTOM_JOB_H_
#define URL_REQUEST_CUSTOM_JOB_H_

#include "net/url_request/url_request_job.h"
#include "url/gurl.h"
#include <QtCore/QPointer>

QT_FORWARD_DECLARE_CLASS(QIODevice)

namespace QtWebEngineCore {

class ProfileAdapter;
class URLRequestCustomJobDelegate;
class URLRequestCustomJobProxy;

// A request job that handles reading custom URL schemes
class URLRequestCustomJob : public net::URLRequestJob {
public:
    URLRequestCustomJob(net::URLRequest *request,
                        net::NetworkDelegate *networkDelegate,
                        const std::string &scheme,
                        QPointer<ProfileAdapter> profileAdapter);
    void Start() override;
    void Kill() override;
    int ReadRawData(net::IOBuffer *buf, int buf_size)  override;
    bool GetMimeType(std::string *mimeType) const override;
    bool GetCharset(std::string *charset) override;
    bool IsRedirectResponse(GURL* location, int* http_status_code, bool* insecure_scheme_was_upgraded) override;

protected:
    virtual ~URLRequestCustomJob();

private:
    void notifyReadyRead();
    scoped_refptr<URLRequestCustomJobProxy> m_proxy;
    std::string m_mimeType;
    std::string m_charset;
    GURL m_redirect;
    QIODevice *m_device;
    int m_error;
    int m_pendingReadSize;
    int m_pendingReadPos;
    net::IOBuffer *m_pendingReadBuffer;

    friend class URLRequestCustomJobProxy;

    DISALLOW_COPY_AND_ASSIGN(URLRequestCustomJob);
};
} // namespace QtWebEngineCore

#endif // URL_REQUEST_CUSTOM_JOB_H_
