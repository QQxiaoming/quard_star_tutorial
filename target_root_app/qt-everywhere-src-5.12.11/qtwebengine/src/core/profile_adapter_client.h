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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef PROFILE_ADAPTER_CLIENT_H
#define PROFILE_ADAPTER_CLIENT_H

#include "qtwebenginecoreglobal_p.h"
#include <QString>
#include <QUrl>

namespace QtWebEngineCore {

class WebContentsAdapterClient;

class QWEBENGINECORE_PRIVATE_EXPORT ProfileAdapterClient
{
public:
    // Keep in sync with content::DownloadItem::DownloadState
    enum DownloadState {
        // Download is actively progressing.
        DownloadInProgress = 0,
        // Download is completely finished.
        DownloadCompleted,
        // Download has been cancelled.
        DownloadCancelled,
        // This state indicates that the download has been interrupted.
        DownloadInterrupted
    };

    // Keep in sync with content::SavePageType
    enum SavePageFormat {
        UnknownSavePageFormat = -1,
        SingleHtmlSaveFormat,
        CompleteHtmlSaveFormat,
        MimeHtmlSaveFormat
    };

    enum DownloadType {
        Attachment = 0,
        DownloadAttribute,
        UserRequested,
        SavePage
    };

    // Keep in sync with content::DownloadInterruptReason
    enum DownloadInterruptReason {
        NoReason = 0,
        FileFailed = 1,
        FileAccessDenied = 2,
        FileNoSpace = 3,
        FileNameTooLong = 5,
        FileTooLarge = 6,
        FileVirusInfected = 7,
        FileTransientError = 10,
        FileBlocked = 11,
        FileSecurityCheckFailed = 12,
        FileTooShort = 13,
        FileHashMismatch = 14,
        NetworkFailed = 20,
        NetworkTimeout = 21,
        NetworkDisconnected = 22,
        NetworkServerDown = 23,
        NetworkInvalidRequest = 24,
        ServerFailed = 30,
        //ServerNoRange = 31,
        ServerBadContent = 33,
        ServerUnauthorized = 34,
        ServerCertProblem = 35,
        ServerForbidden = 36,
        ServerUnreachable = 37,
        UserCanceled = 40,
        //UserShutdown = 41,
        //Crash = 50
    };

    struct DownloadItemInfo {
        const quint32 id;
        const QUrl url;
        const int state;
        const qint64 totalBytes;
        const qint64 receivedBytes;
        const QString mimeType;

        QString path;
        int savePageFormat;
        bool accepted;
        bool paused;
        bool done;
        int downloadType;
        int downloadInterruptReason;
        WebContentsAdapterClient *page;
    };

    virtual ~ProfileAdapterClient() { }

    virtual void downloadRequested(DownloadItemInfo &info) = 0;
    virtual void downloadUpdated(const DownloadItemInfo &info) = 0;
    virtual void addWebContentsAdapterClient(WebContentsAdapterClient *adapter) = 0;
    virtual void removeWebContentsAdapterClient(WebContentsAdapterClient *adapter) = 0;
    static QString downloadInterruptReasonToString(DownloadInterruptReason reason);
};

} // namespace

#endif // PROFILE_ADAPTER_CLIENT_H
