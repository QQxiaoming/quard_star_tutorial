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

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#ifndef PRINT_VIEW_MANAGER_BASE_QT_H
#define PRINT_VIEW_MANAGER_BASE_QT_H

#include "base/memory/ref_counted_memory.h"
#include "base/strings/string16.h"
#include "components/prefs/pref_member.h"
#include "components/printing/browser/print_manager.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

struct PrintHostMsg_DidPrintDocument_Params;

namespace base {
class RefCountedBytes;
}

namespace content {
class RenderFrameHost;
class RenderViewHost;
}

namespace printing {
class JobEventDetails;
class MetafilePlayer;
class PrintJob;
class PrintJobWorkerOwner;
class PrintQueriesQueue;
class PrintedDocument;
class PrinterQuery;
}

namespace QtWebEngineCore {
class PrintViewManagerBaseQt : public content::NotificationObserver
                             , public printing::PrintManager
{
public:
    ~PrintViewManagerBaseQt() override;

    // Whether printing is enabled or not.
    void UpdatePrintingEnabled();

    virtual base::string16 RenderSourceName();

protected:
    explicit PrintViewManagerBaseQt(content::WebContents*);

    void SetPrintingRFH(content::RenderFrameHost* rfh);

    // content::WebContentsObserver implementation.
    // Cancels the print job.
    void NavigationStopped() override;

    // content::WebContentsObserver implementation.
    void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) override;
    bool OnMessageReceived(const IPC::Message& message,
                           content::RenderFrameHost* render_frame_host) override;

    // IPC Message handlers.
    void OnDidPrintDocument(content::RenderFrameHost* render_frame_host,
                            const PrintHostMsg_DidPrintDocument_Params& params);
    void OnShowInvalidPrinterSettingsError();

    // Processes a NOTIFY_PRINT_JOB_EVENT notification.
    void OnNotifyPrintJobEvent(const printing::JobEventDetails& event_details);

    // content::NotificationObserver implementation.
    void Observe(int,
                 const content::NotificationSource&,
                 const content::NotificationDetails&) override;
    void StopWorker(int document_cookie);

    // In the case of Scripted Printing, where the renderer is controlling the
    // control flow, print_job_ is initialized whenever possible. No-op is
    // print_job_ is initialized.
    bool OpportunisticallyCreatePrintJob(int cookie);

    // Requests the RenderView to render all the missing pages for the print job.
    // No-op if no print job is pending. Returns true if at least one page has
    // been requested to the renderer.
    bool RenderAllMissingPagesNow();

    // Checks that synchronization is correct and a print query exists for
    // |cookie|. If so, returns the document associated with the cookie.
    printing::PrintedDocument* GetDocument(int cookie);

    // Starts printing a document with data given in |print_data|. |print_data|
    // must successfully initialize a metafile. |document| is the printed
    // document associated with the print job. Returns true if successful.
    void PrintDocument(printing::PrintedDocument *document,
                       const scoped_refptr<base::RefCountedMemory> &print_data,
                       const gfx::Size &page_size,
                       const gfx::Rect &content_area,
                       const gfx::Point &offsets);

    // Quits the current message loop if these conditions hold true: a document is
    // loaded and is complete and waiting_for_pages_to_be_rendered_ is true. This
    // function is called in DidPrintDocument() or on ALL_PAGES_REQUESTED
    // notification. The inner message loop is created was created by
    // RenderAllMissingPagesNow().
    void ShouldQuitFromInnerMessageLoop();

    bool RunInnerMessageLoop();

    void TerminatePrintJob(bool cancel);
    void DisconnectFromCurrentPrintJob();

    bool CreateNewPrintJob(printing::PrinterQuery *job);
    void ReleasePrintJob();
    void ReleasePrinterQuery();

private:
    // Helper method for UpdatePrintingEnabled().
    void SendPrintingEnabled(bool enabled, content::RenderFrameHost* rfh);
    // content::WebContentsObserver implementation.
    void DidStartLoading() override;

private:
    content::NotificationRegistrar m_registrar;
    scoped_refptr<printing::PrintJob> m_printJob;
    bool m_isInsideInnerMessageLoop;
    bool m_didPrintingSucceed;
    scoped_refptr<printing::PrintQueriesQueue> m_printerQueriesQueue;
    // The current RFH that is printing with a system printing dialog.
    content::RenderFrameHost* m_printingRFH;
    DISALLOW_COPY_AND_ASSIGN(PrintViewManagerBaseQt);
};

} // namespace QtWebEngineCore
#endif // PRINT_VIEW_MANAGER_BASE_QT_H

