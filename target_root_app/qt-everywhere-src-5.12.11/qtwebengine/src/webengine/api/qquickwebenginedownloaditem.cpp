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

#include "qquickwebenginedownloaditem_p.h"
#include "qquickwebenginedownloaditem_p_p.h"

#include "profile_adapter.h"
#include "qquickwebengineprofile_p.h"

#include "QFileInfo"

using QtWebEngineCore::ProfileAdapterClient;

QT_BEGIN_NAMESPACE

ASSERT_ENUMS_MATCH(ProfileAdapterClient::NoReason, QQuickWebEngineDownloadItem::NoReason)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileFailed, QQuickWebEngineDownloadItem::FileFailed)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileAccessDenied, QQuickWebEngineDownloadItem::FileAccessDenied)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileNoSpace, QQuickWebEngineDownloadItem::FileNoSpace)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileNameTooLong, QQuickWebEngineDownloadItem::FileNameTooLong)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileTooLarge, QQuickWebEngineDownloadItem::FileTooLarge)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileVirusInfected, QQuickWebEngineDownloadItem::FileVirusInfected)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileTransientError, QQuickWebEngineDownloadItem::FileTransientError)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileBlocked, QQuickWebEngineDownloadItem::FileBlocked)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileSecurityCheckFailed, QQuickWebEngineDownloadItem::FileSecurityCheckFailed)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileTooShort, QQuickWebEngineDownloadItem::FileTooShort)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileHashMismatch, QQuickWebEngineDownloadItem::FileHashMismatch)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkFailed, QQuickWebEngineDownloadItem::NetworkFailed)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkTimeout, QQuickWebEngineDownloadItem::NetworkTimeout)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkDisconnected, QQuickWebEngineDownloadItem::NetworkDisconnected)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkServerDown, QQuickWebEngineDownloadItem::NetworkServerDown)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkInvalidRequest, QQuickWebEngineDownloadItem::NetworkInvalidRequest)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerFailed, QQuickWebEngineDownloadItem::ServerFailed)
//ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerNoRange, QQuickWebEngineDownloadItem::ServerNoRange)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerBadContent, QQuickWebEngineDownloadItem::ServerBadContent)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerUnauthorized, QQuickWebEngineDownloadItem::ServerUnauthorized)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerCertProblem, QQuickWebEngineDownloadItem::ServerCertProblem)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerForbidden, QQuickWebEngineDownloadItem::ServerForbidden)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerUnreachable, QQuickWebEngineDownloadItem::ServerUnreachable)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::UserCanceled, QQuickWebEngineDownloadItem::UserCanceled)
//ASSERT_ENUMS_MATCH(ProfileAdapterClient::UserShutdown, QQuickWebEngineDownloadItem::UserShutdown)
//ASSERT_ENUMS_MATCH(ProfileAdapterClient::Crash, QQuickWebEngineDownloadItem::Crash)

static inline QQuickWebEngineDownloadItem::DownloadState toDownloadState(int state) {
    switch (state) {
    case ProfileAdapterClient::DownloadInProgress:
        return QQuickWebEngineDownloadItem::DownloadInProgress;
    case ProfileAdapterClient::DownloadCompleted:
        return QQuickWebEngineDownloadItem::DownloadCompleted;
    case ProfileAdapterClient::DownloadCancelled:
        return QQuickWebEngineDownloadItem::DownloadCancelled;
    case ProfileAdapterClient::DownloadInterrupted:
        return QQuickWebEngineDownloadItem::DownloadInterrupted;
    default:
        Q_UNREACHABLE();
        return QQuickWebEngineDownloadItem::DownloadCancelled;
    }
}

static inline QQuickWebEngineDownloadItem::DownloadInterruptReason toDownloadInterruptReason(int reason)
{
    return static_cast<QQuickWebEngineDownloadItem::DownloadInterruptReason>(reason);
}

QQuickWebEngineDownloadItemPrivate::QQuickWebEngineDownloadItemPrivate(QQuickWebEngineProfile *p)
    : profile(p)
    , downloadId(-1)
    , downloadState(QQuickWebEngineDownloadItem::DownloadCancelled)
    , savePageFormat(QQuickWebEngineDownloadItem::UnknownSaveFormat)
    , type(QQuickWebEngineDownloadItem::Attachment)
    , interruptReason(QQuickWebEngineDownloadItem::NoReason)
    , totalBytes(-1)
    , receivedBytes(0)
    , downloadFinished(false)
    , downloadPaused(false)
    , view(nullptr)
{
}

QQuickWebEngineDownloadItemPrivate::~QQuickWebEngineDownloadItemPrivate()
{
    if (profile)
        profile->d_ptr->downloadDestroyed(downloadId);
}

/*!
    \qmltype WebEngineDownloadItem
    \instantiates QQuickWebEngineDownloadItem
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.1
    \brief Provides information about a download.

    WebEngineDownloadItem models a download throughout its life cycle, starting
    with a pending download request and finishing with a completed download. It
    can be used, for example, to get information about new downloads, to monitor
    progress, and to pause, resume, and cancel downloads.

    Downloads are usually triggered by user interaction on a web page. It is the
    WebEngineProfile's responsibility to notify the application of new download
    requests, which it does by emitting the
    \l{WebEngineProfile::downloadRequested}{downloadRequested} signal together
    with a newly created WebEngineDownloadItem. The application can then examine
    this item and decide whether to accept it or not. A signal handler must
    explicitly call accept() on the item for \QWE to actually start
    downloading and writing data to disk. If no signal handler calls accept(),
    then the download request will be automatically rejected and nothing will be
    written to disk.

    \note Some properties, like the \l path under which the file will be saved,
    can only be changed before calling accept().

    \section2 Object Life Cycle

    All items are guaranteed to be valid during the emission of the
    \l{WebEngineProfile::downloadRequested}{downloadRequested} signal. If
    accept() is \e not called by any signal handler, then the item will be
    deleted \e immediately after signal emission. This means that the
    application \b{must not} keep references to rejected download items.

    \section2 Web Page Downloads

    In addition to normal file downloads, which consist simply of retrieving
    some raw bytes from the network and writing them to disk, \QWE also
    supports saving complete web pages, which involves parsing the page's HTML,
    downloading any dependent resources, and potentially packaging everything
    into a special file format (\l savePageFormat). To check if a download is
    for a file or a web page, use \l isSavePageDownload.

    \sa WebEngineProfile, WebEngineProfile::downloadRequested,
    WebEngineProfile::downloadFinished
*/

void QQuickWebEngineDownloadItemPrivate::update(const ProfileAdapterClient::DownloadItemInfo &info)
{
    Q_Q(QQuickWebEngineDownloadItem);

    updateState(toDownloadState(info.state));

    if (toDownloadInterruptReason(info.downloadInterruptReason) != interruptReason) {
        interruptReason = toDownloadInterruptReason(info.downloadInterruptReason);
        Q_EMIT q->interruptReasonChanged();
    }

    if (info.receivedBytes != receivedBytes) {
        receivedBytes = info.receivedBytes;
        Q_EMIT q->receivedBytesChanged();
    }

    if (info.totalBytes != totalBytes) {
        totalBytes = info.totalBytes;
        Q_EMIT q->totalBytesChanged();
    }

    if (info.done)
        setFinished();

    if (info.paused != downloadPaused) {
        downloadPaused = info.paused;
        Q_EMIT q->isPausedChanged();
    }
}

void QQuickWebEngineDownloadItemPrivate::updateState(QQuickWebEngineDownloadItem::DownloadState newState)
{
    Q_Q(QQuickWebEngineDownloadItem);

    if (downloadState != newState) {
        downloadState = newState;
        Q_EMIT q->stateChanged();
    }
}

void QQuickWebEngineDownloadItemPrivate::setFinished()
{
    Q_Q(QQuickWebEngineDownloadItem);

    if (downloadFinished)
        return;

    downloadFinished = true;
    Q_EMIT q->isFinishedChanged();
}

/*!
    \qmlmethod void WebEngineDownloadItem::accept()

    Accepts the download request, which will start the download.

    If the item is in the \c DownloadRequested state, then it will transition
    into the \c DownloadInProgress state and the downloading will begin. If the
    item is in any other state, then nothing will happen.

    \sa state
*/

void QQuickWebEngineDownloadItem::accept()
{
    Q_D(QQuickWebEngineDownloadItem);

    if (d->downloadState != QQuickWebEngineDownloadItem::DownloadRequested)
        return;

    d->updateState(QQuickWebEngineDownloadItem::DownloadInProgress);
}

/*!
    \qmlmethod void WebEngineDownloadItem::cancel()

    Cancels the download.

    If the item is in the \c DownloadInProgress state, then it will transition
    into the \c DownloadCancelled state, the downloading will stop, and
    partially downloaded files will be deleted from disk.

    If the item is in the \c DownloadCompleted state, then nothing will happen.
    If the item is in any other state, then it will transition into the \c
    DownloadCancelled state without further effect.

    \sa state
*/

void QQuickWebEngineDownloadItem::cancel()
{
    Q_D(QQuickWebEngineDownloadItem);

    QQuickWebEngineDownloadItem::DownloadState state = d->downloadState;

    if (state == QQuickWebEngineDownloadItem::DownloadCompleted
            || state == QQuickWebEngineDownloadItem::DownloadCancelled)
        return;

    d->updateState(QQuickWebEngineDownloadItem::DownloadCancelled);
    d->setFinished();

    // We directly cancel the download if the user cancels before
    // it even started, so no need to notify the profile here.
    if (state == QQuickWebEngineDownloadItem::DownloadInProgress) {
        if (d->profile)
            d->profile->d_ptr->cancelDownload(d->downloadId);
    }
}


/*!
    \qmlmethod void WebEngineDownloadItem::pause()
    \since QtWebEngine 1.6

    Pauses the download.

    Has no effect if the state is not \c DownloadInProgress. Does not change the
    state.

    \sa resume, isPaused
*/

void QQuickWebEngineDownloadItem::pause()
{
    Q_D(QQuickWebEngineDownloadItem);

    QQuickWebEngineDownloadItem::DownloadState state = d->downloadState;

    if (state != QQuickWebEngineDownloadItem::DownloadInProgress)
        return;

    if (d->profile)
        d->profile->d_ptr->profileAdapter()->pauseDownload(d->downloadId);
}

/*!
    \qmlmethod void WebEngineDownloadItem::resume()
    \since QtWebEngine 1.6

    Resumes the download if it was paused or interrupted.

    Has no effect if the state is not \c DownloadInProgress or \c
    DownloadInterrupted. Does not change the state.

    \sa pause, isPaused
*/
void QQuickWebEngineDownloadItem::resume()
{
    Q_D(QQuickWebEngineDownloadItem);

    QQuickWebEngineDownloadItem::DownloadState state = d->downloadState;

    if (d->downloadFinished || (state != QQuickWebEngineDownloadItem::DownloadInProgress && state != QQuickWebEngineDownloadItem::DownloadInterrupted))
        return;

    if (d->profile)
        d->profile->d_ptr->profileAdapter()->resumeDownload(d->downloadId);
}

/*!
    \qmlproperty int WebEngineDownloadItem::id

    Holds the download item's ID.
*/

quint32 QQuickWebEngineDownloadItem::id() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->downloadId;
}

/*!
    \qmlproperty enumeration WebEngineDownloadItem::state

    Describes the state of the download:

    \value  WebEngineDownloadItem.DownloadRequested
            Download has been requested, but it has not been accepted yet.
    \value  WebEngineDownloadItem.DownloadInProgress
            Download is in progress.
    \value  WebEngineDownloadItem.DownloadCompleted
            Download completed successfully.
    \value  WebEngineDownloadItem.DownloadCancelled
            Download was cancelled by the user.
    \value  WebEngineDownloadItem.DownloadInterrupted
            Download has been interrupted (by the server or because of lost connectivity).
*/

QQuickWebEngineDownloadItem::DownloadState QQuickWebEngineDownloadItem::state() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->downloadState;
}

/*!
    \qmlproperty int WebEngineDownloadItem::totalBytes

    Holds the total amount of data to download in bytes.

    \c -1 means the total size is unknown.
*/

qint64 QQuickWebEngineDownloadItem::totalBytes() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->totalBytes;
}

/*!
    \qmlproperty int WebEngineDownloadItem::receivedBytes

    Holds the amount of data in bytes that has been downloaded so far.
*/

qint64 QQuickWebEngineDownloadItem::receivedBytes() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->receivedBytes;
}

/*!
    \qmlproperty string WebEngineDownloadItem::mimeType
    \since QtWebEngine 1.2

    Holds the MIME type of the download.
*/

QString QQuickWebEngineDownloadItem::mimeType() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->mimeType;
}

/*!
    \qmlproperty string WebEngineDownloadItem::path

    Holds the full target path where data is being downloaded to.

    The path includes the file name. The default suggested path is the standard
    download location and file name is deduced not to overwrite already existing files.

    The download path can only be set in the
    \l{WebEngineProfile::downloadRequested}{downloadRequested} handler before
    the download is accepted.

    \sa WebEngineProfile::downloadRequested(), accept()
*/

QString QQuickWebEngineDownloadItem::path() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->downloadPath;
}

void QQuickWebEngineDownloadItem::setPath(QString path)
{
    Q_D(QQuickWebEngineDownloadItem);
    if (d->downloadState != QQuickWebEngineDownloadItem::DownloadRequested) {
        qWarning("Setting the download path is not allowed after the download has been accepted.");
        return;
    }
    if (d->downloadPath != path) {
        if (QFileInfo(path).fileName().isEmpty()) {
            qWarning("The download path does not include file name.");
            return;
        }

        if (QFileInfo(path).isDir()) {
            qWarning("The download path matches with an already existing directory path.");
            return;
        }

        d->downloadPath = path;
        Q_EMIT pathChanged();
    }
}

/*!
    \qmlproperty enumeration WebEngineDownloadItem::savePageFormat
    \since QtWebEngine 1.3

    Describes the format that is used to save a web page.

    \value  WebEngineDownloadItem.UnknownSaveFormat
            This is not a request for downloading a complete web page.
    \value  WebEngineDownloadItem.SingleHtmlSaveFormat
            The page is saved as a single HTML page. Resources such as images
            are not saved.
    \value  WebEngineDownloadItem.CompleteHtmlSaveFormat
            The page is saved as a complete HTML page, for example a directory
            containing the single HTML page and the resources.
    \value  WebEngineDownloadItem.MimeHtmlSaveFormat
            The page is saved as a complete web page in the MIME HTML format.
*/

QQuickWebEngineDownloadItem::SavePageFormat QQuickWebEngineDownloadItem::savePageFormat() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->savePageFormat;
}

void QQuickWebEngineDownloadItem::setSavePageFormat(QQuickWebEngineDownloadItem::SavePageFormat format)
{
    Q_D(QQuickWebEngineDownloadItem);
    if (d->savePageFormat != format) {
        d->savePageFormat = format;
        Q_EMIT savePageFormatChanged();
    }
}

/*!
    \qmlproperty enumeration WebEngineDownloadItem::type
    \readonly
    \since QtWebEngine 1.4
    \obsolete

    Describes the requested download's type.

    \note This property works unreliably, except for \c SavePage
    downloads. Use \l isSavePageDownload instead.

    \value WebEngineDownloadItem.Attachment The web server's response includes a
           \c Content-Disposition header with the \c attachment directive. If \c Content-Disposition
           is present in the reply, the web server is indicating that the client should prompt the
           user to save the content regardless of the content type.
           See \l {RFC 2616 section 19.5.1} for details.
    \value WebEngineDownloadItem.DownloadAttribute The user clicked a link with the \c download
           attribute.
    \value WebEngineDownloadItem.UserRequested The user initiated the download, for example by
           selecting a web action.
    \value WebEngineDownloadItem.SavePage Saving of the current page was requested (for example by
           the \l{WebEngineView::WebAction}{WebEngineView.SavePage} web action).

*/

QQuickWebEngineDownloadItem::DownloadType QQuickWebEngineDownloadItem::type() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->type;
}

/*!
    \qmlproperty bool WebEngineDownloadItem::isSavePageDownload
    \readonly
    \since QtWebEngine 1.7

    Whether this is a download request for saving a web page or a file.

    \sa savePageFormat
*/
bool QQuickWebEngineDownloadItem::isSavePageDownload() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->type == QQuickWebEngineDownloadItem::SavePage;
}

/*!
    \qmlproperty enumeration WebEngineDownloadItem::interruptReason
    \readonly
    \since QtWebEngine 1.5

    Returns the reason why the download was interrupted:

    \value WebEngineDownloadItem.NoReason Unknown reason or not interrupted.
    \value WebEngineDownloadItem.FileFailed General file operation failure.
    \value WebEngineDownloadItem.FileAccessDenied The file cannot be written locally, due to access restrictions.
    \value WebEngineDownloadItem.FileNoSpace Insufficient space on the target drive.
    \value WebEngineDownloadItem.FileNameTooLong The directory or file name is too long.
    \value WebEngineDownloadItem.FileTooLarge The file size exceeds the file system limitation.
    \value WebEngineDownloadItem.FileVirusInfected The file is infected with a virus.
    \value WebEngineDownloadItem.FileTransientError Temporary problem (for example the file is in use,
           out of memory, or too many files are opened at once).
    \value WebEngineDownloadItem.FileBlocked The file was blocked due to local policy.
    \value WebEngineDownloadItem.FileSecurityCheckFailed An attempt to check the safety of the download
           failed due to unexpected reasons.
    \value WebEngineDownloadItem.FileTooShort An attempt was made to seek past the end of a file when
           opening a file (as part of resuming a previously interrupted download).
    \value WebEngineDownloadItem.FileHashMismatch The partial file did not match the expected hash.

    \value WebEngineDownloadItem.NetworkFailed General network failure.
    \value WebEngineDownloadItem.NetworkTimeout The network operation has timed out.
    \value WebEngineDownloadItem.NetworkDisconnected The network connection has been terminated.
    \value WebEngineDownloadItem.NetworkServerDown The server has gone down.
    \value WebEngineDownloadItem.NetworkInvalidRequest The network request was invalid (for example, the
           original or redirected URL is invalid, has an unsupported scheme, or is disallowed by policy).

    \value WebEngineDownloadItem.ServerFailed General server failure.
    \value WebEngineDownloadItem.ServerBadContent The server does not have the requested data.
    \value WebEngineDownloadItem.ServerUnauthorized The server did not authorize access to the resource.
    \value WebEngineDownloadItem.ServerCertProblem A problem with the server certificate occurred.
    \value WebEngineDownloadItem.ServerForbidden Access forbidden by the server.
    \value WebEngineDownloadItem.ServerUnreachable Unexpected server response (might indicate that
           the responding server may not be the intended server).
    \value WebEngineDownloadItem.UserCanceled The user canceled the download.

    \sa interruptReasonString
*/

QQuickWebEngineDownloadItem::DownloadInterruptReason QQuickWebEngineDownloadItem::interruptReason() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->interruptReason;
}

/*!
    \qmlproperty string WebEngineDownloadItem::interruptReasonString
    Returns a human-readable description of the reason for interrupting the download.
    \since QtWebEngine 1.5

    \sa interruptReason
*/
QString QQuickWebEngineDownloadItem::interruptReasonString() const
{
    return ProfileAdapterClient::downloadInterruptReasonToString(
              static_cast<ProfileAdapterClient::DownloadInterruptReason>(interruptReason()));
}

/*!
    \qmlproperty bool WebEngineDownloadItem::isFinished
    \readonly
    \since QtWebEngine 1.6

    Whether this download is finished (completed, cancelled, or non-resumable interrupted state).
 */

bool QQuickWebEngineDownloadItem::isFinished() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->downloadFinished;
}

/*!
    \qmlproperty bool WebEngineDownloadItem::isPaused
    \readonly
    \since QtWebEngine 1.6

    Whether this download is paused.

    \sa pause, resume
 */

bool QQuickWebEngineDownloadItem::isPaused() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->downloadPaused;
}

/*!
    \qmlproperty WebEngineView WebEngineDownloadItem::view
    \readonly
    \since QtWebEngine 1.8

    Returns the view the download was requested on. If the download was not triggered by content in a view,
    \c nullptr is returned.

    \sa WebEngineView
*/
QQuickWebEngineView *QQuickWebEngineDownloadItem::view() const
{
    Q_D(const QQuickWebEngineDownloadItem);
    return d->view;
}

QQuickWebEngineDownloadItem::QQuickWebEngineDownloadItem(QQuickWebEngineDownloadItemPrivate *p, QObject *parent)
    : QObject(parent)
    , d_ptr(p)
{
    p->q_ptr = this;
}

QQuickWebEngineDownloadItem::~QQuickWebEngineDownloadItem()
{
    if (!isFinished())
        cancel();
}

QT_END_NAMESPACE
