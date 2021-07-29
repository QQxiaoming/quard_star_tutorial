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

#include "qwebengineurlrequestjob.h"

#include "net/url_request_custom_job_proxy.h"
#include "net/url_request_custom_job_delegate.h"

using QtWebEngineCore::URLRequestCustomJobDelegate;

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineUrlRequestJob
    \brief The QWebEngineUrlRequestJob class represents a custom URL request.
    \since 5.6

    A QWebEngineUrlRequestJob is given to QWebEngineUrlSchemeHandler::requestStarted() and must
    be handled by the derived implementations of the class. The job can be handled by calling
    either reply(), redirect(), or fail().

    The class is owned by the web engine and does not need to be deleted. However, the web engine
    may delete the job when it is no longer needed, and therefore the signal QObject::destroyed()
    must be monitored if a pointer to the object is stored.

    \inmodule QtWebEngineCore
*/

/*!
    \enum QWebEngineUrlRequestJob::Error

    This enum type holds the type of the error that occurred:

    \value  NoError
            The request was successful.
    \value  UrlNotFound
            The requested URL was not found.
    \value  UrlInvalid
            The requested URL is invalid.
    \value  RequestAborted
            The request was canceled.
    \value  RequestDenied
            The request was denied.
    \value  RequestFailed
            The request failed.
*/

/*!
    \internal
 */
QWebEngineUrlRequestJob::QWebEngineUrlRequestJob(URLRequestCustomJobDelegate * p)
    : QObject(p) // owned by the jobdelegate and deleted when the job is done
    , d_ptr(p)
{
}

/*!
    \internal
 */
QWebEngineUrlRequestJob::~QWebEngineUrlRequestJob()
{
}

/*!
    Returns the requested URL.
*/
QUrl QWebEngineUrlRequestJob::requestUrl() const
{
    return d_ptr->url();
}

/*!
    Returns the HTTP method of the request (for example, GET or POST).
*/
QByteArray QWebEngineUrlRequestJob::requestMethod() const
{
    return d_ptr->method();
}

/*!
    \since 5.11
    Returns the serialized origin of the content that initiated the request.

    Generally, the origin consists of a scheme, hostname, and port. For example,
    \c "http://localhost:8080" would be a valid origin. The port is omitted if
    it is the scheme's default port (80 for \c http, 443 for \c https). The
    hostname is omitted for non-network schemes such as \c file and \c qrc.

    However, there is also the special value \c "null" representing a unique
    origin. It is, for example, the origin of a sandboxed iframe. The purpose of
    this special origin is to be always different from all other origins in the
    same-origin check. In other words, content with a unique origin should never
    have privileged access to any other content.

    Finally, if the request was not initiated by web content, the function will
    return an empty QUrl. This happens, for example, when you call \l
    QWebEnginePage::setUrl().

    This value can be used for implementing secure cross-origin checks.
*/
QUrl QWebEngineUrlRequestJob::initiator() const
{
    return d_ptr->initiator();
}

/*!
    Replies to the request with \a device and the MIME type \a contentType.

    The user has to be aware that \a device will be used on another thread
    until the job is deleted. In case simultaneous access from the main thread
    is desired, the user is reponsible for making access to \a device thread-safe
    for example by using QMutex. Note that the \a device object is not owned by
    the web engine. Therefore, the signal QObject::destroyed() of
    QWebEngineUrlRequestJob must be monitored.

    The device should remain available at least as long as the job exists.
    When calling this method with a newly constructed device, one solution is to
    make the device as a child of the job or delete itself when job is deleted,
    like this:
    \code
    connect(job, &QObject::destroyed, device, &QObject::deleteLater);
    \endcode
 */
void QWebEngineUrlRequestJob::reply(const QByteArray &contentType, QIODevice *device)
{
    d_ptr->reply(contentType, device);
}

/*!
    Fails the request with the error \a r.

    \sa Error
 */
void QWebEngineUrlRequestJob::fail(Error r)
{
    d_ptr->fail((URLRequestCustomJobDelegate::Error)r);
}

/*!
    Redirects the request to \a url.
 */
void QWebEngineUrlRequestJob::redirect(const QUrl &url)
{
    d_ptr->redirect(url);
}

QT_END_NAMESPACE
