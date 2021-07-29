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

// This is based on chrome/browser/printing/print_view_manager_base.cc:
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "print_view_manager_qt.h"

#include "type_conversion.h"
#include "web_engine_context.h"

#include "base/memory/ref_counted_memory.h"
#include "base/memory/shared_memory.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/timer/timer.h"
#include "base/values.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/printing/print_job.h"
#include "chrome/browser/printing/print_job_manager.h"
#include "chrome/browser/printing/printer_query.h"
#include "components/printing/common/print_messages.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "printing/pdf_metafile_skia.h"
#include "printing/print_job_constants.h"
#include "printing/printed_document.h"

namespace QtWebEngineCore {

PrintViewManagerBaseQt::PrintViewManagerBaseQt(content::WebContents *contents)
    : printing::PrintManager(contents)
    , m_isInsideInnerMessageLoop(false)
    , m_didPrintingSucceed(false)
    , m_printerQueriesQueue(WebEngineContext::current()->getPrintJobManager()->queue())
    , m_printingRFH(nullptr)
{
    // FIXME: Check if this needs to be executed async:
    // TODO: Add isEnabled to profile
    PrintViewManagerBaseQt::UpdatePrintingEnabled();
}

PrintViewManagerBaseQt::~PrintViewManagerBaseQt()
{
    ReleasePrinterQuery();
    DisconnectFromCurrentPrintJob();
}

void PrintViewManagerBaseQt::SetPrintingRFH(content::RenderFrameHost *rfh)
{
    DCHECK(!m_printingRFH);
    m_printingRFH = rfh;
}

void PrintViewManagerBaseQt::UpdatePrintingEnabled()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    bool enabled = false;
#if QT_CONFIG(webengine_printing_and_pdf)
    enabled = true;
#endif
    web_contents()->ForEachFrame(
            base::Bind(&PrintViewManagerBaseQt::SendPrintingEnabled,
                    base::Unretained(this), enabled));
}

void PrintViewManagerBaseQt::NavigationStopped()
{
    // Cancel the current job, wait for the worker to finish.
    TerminatePrintJob(true);
}

base::string16 PrintViewManagerBaseQt::RenderSourceName()
{
     return toString16(QLatin1String(""));
}

void PrintViewManagerBaseQt::PrintDocument(printing::PrintedDocument *document,
                                           const scoped_refptr<base::RefCountedMemory> &print_data,
                                           const gfx::Size &page_size,
                                           const gfx::Rect &content_area,
                                           const gfx::Point &offsets)
{
    std::unique_ptr<printing::PdfMetafileSkia> metafile =
            std::make_unique<printing::PdfMetafileSkia>();
    CHECK(metafile->InitFromData(print_data->front(), print_data->size()));

    // Update the rendered document. It will send notifications to the listener.
    document->SetDocument(std::move(metafile), page_size, content_area);
    ShouldQuitFromInnerMessageLoop();
}

printing::PrintedDocument *PrintViewManagerBaseQt::GetDocument(int cookie)
{
    if (!OpportunisticallyCreatePrintJob(cookie))
        return nullptr;

    printing::PrintedDocument* document = m_printJob->document();
    if (!document || cookie != document->cookie()) {
        // Out of sync. It may happen since we are completely asynchronous. Old
        // spurious messages can be received if one of the processes is overloaded.
        return nullptr;
    }
    return document;
}

// IPC handlers
void PrintViewManagerBaseQt::OnDidPrintDocument(content::RenderFrameHost* /*render_frame_host*/,
                                                const PrintHostMsg_DidPrintDocument_Params &params)
{
    printing::PrintedDocument *document = GetDocument(params.document_cookie);
    if (!document)
        return;

    const PrintHostMsg_DidPrintContent_Params &content = params.content;
    if (!base::SharedMemory::IsHandleValid(content.metafile_data_handle)) {
        NOTREACHED() << "invalid memory handle";
        web_contents()->Stop();
        return;
    }

    std::unique_ptr<base::SharedMemory> shared_buf =
            std::make_unique<base::SharedMemory>(content.metafile_data_handle, true);
    if (!shared_buf->Map(content.data_size)) {
        NOTREACHED() << "couldn't map";
        web_contents()->Stop();
        return;
    }
    auto data = base::MakeRefCounted<base::RefCountedSharedMemory>(
        std::move(shared_buf), content.data_size);
    PrintDocument(document, data, params.page_size, params.content_area,
                  params.physical_offsets);
}

void PrintViewManagerBaseQt::OnShowInvalidPrinterSettingsError()
{
}

void PrintViewManagerBaseQt::DidStartLoading()
{
    UpdatePrintingEnabled();
}

// Note: In PrintViewManagerQt we always initiate printing with PrintMsg_InitiatePrintPreview
// so m_printingRFH is never set and used at the moment.
void PrintViewManagerBaseQt::RenderFrameDeleted(content::RenderFrameHost *render_frame_host)
{
    // Terminates or cancels the print job if one was pending.
    if (render_frame_host != m_printingRFH)
        return;

    m_printingRFH = nullptr;

    PrintManager::PrintingRenderFrameDeleted();
    ReleasePrinterQuery();

    if (!m_printJob.get())
        return;

    scoped_refptr<printing::PrintedDocument> document(m_printJob->document());
    if (document.get()) {
        // If IsComplete() returns false, the document isn't completely rendered.
        // Since our renderer is gone, there's nothing to do, cancel it. Otherwise,
        // the print job may finish without problem.
        TerminatePrintJob(!document->IsComplete());
    }
}

bool PrintViewManagerBaseQt::OnMessageReceived(const IPC::Message& message,
                                               content::RenderFrameHost* render_frame_host)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP_WITH_PARAM(PrintViewManagerBaseQt, message, render_frame_host)
        IPC_MESSAGE_HANDLER(PrintHostMsg_DidPrintDocument, OnDidPrintDocument)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    if (handled)
        return true;
    handled = true;
    IPC_BEGIN_MESSAGE_MAP(PrintViewManagerBaseQt, message)
        IPC_MESSAGE_HANDLER(PrintHostMsg_ShowInvalidPrinterSettingsError,
                            OnShowInvalidPrinterSettingsError);
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled || PrintManager::OnMessageReceived(message, render_frame_host);
}

void PrintViewManagerBaseQt::Observe(int type,
        const content::NotificationSource& /*source*/,
        const content::NotificationDetails& details)
{
    DCHECK_EQ(chrome::NOTIFICATION_PRINT_JOB_EVENT, type);
    OnNotifyPrintJobEvent(*content::Details<printing::JobEventDetails>(details).ptr());
}

void PrintViewManagerBaseQt::OnNotifyPrintJobEvent(const printing::JobEventDetails& event_details)
{
    switch (event_details.type()) {
    case printing::JobEventDetails::FAILED: {
        TerminatePrintJob(true);

        content::NotificationService::current()->Notify(
                chrome::NOTIFICATION_PRINT_JOB_RELEASED,
                content::Source<content::WebContents>(web_contents()),
                content::NotificationService::NoDetails());
        break;
    }
    case printing::JobEventDetails::USER_INIT_DONE:
    case printing::JobEventDetails::DEFAULT_INIT_DONE:
    case printing::JobEventDetails::USER_INIT_CANCELED: {
        NOTREACHED();
        break;
    }
    case printing::JobEventDetails::ALL_PAGES_REQUESTED: {
        ShouldQuitFromInnerMessageLoop();
        break;
    }
    case printing::JobEventDetails::NEW_DOC:
#if defined(OS_WIN)
    case printing::JobEventDetails::PAGE_DONE:
#endif
    case printing::JobEventDetails::DOC_DONE: {
        // Don't care about the actual printing process.
        break;
    }
    case printing::JobEventDetails::JOB_DONE: {
        // Printing is done, we don't need it anymore.
        // print_job_->is_job_pending() may still be true, depending on the order
        // of object registration.
        m_didPrintingSucceed = true;
        ReleasePrintJob();

        content::NotificationService::current()->Notify(
                chrome::NOTIFICATION_PRINT_JOB_RELEASED,
                content::Source<content::WebContents>(web_contents()),
                content::NotificationService::NoDetails());
        break;
    }
    default:
        NOTREACHED();
        break;
    }
}

// Requests the RenderView to render all the missing pages for the print job.
// No-op if no print job is pending. Returns true if at least one page has
// been requested to the renderer.
bool PrintViewManagerBaseQt::RenderAllMissingPagesNow()
{
    if (!m_printJob.get() || !m_printJob->is_job_pending())
      return false;

    // Is the document already complete?
    if (m_printJob->document() && m_printJob->document()->IsComplete()) {
        m_didPrintingSucceed = true;
        return true;
    }

    // We can't print if there is no renderer.
    if (!web_contents() ||
        !web_contents()->GetRenderViewHost() ||
        !web_contents()->GetRenderViewHost()->IsRenderViewLive()) {
      return false;
    }

    // WebContents is either dying or a second consecutive request to print
    // happened before the first had time to finish. We need to render all the
    // pages in an hurry if a print_job_ is still pending. No need to wait for it
    // to actually spool the pages, only to have the renderer generate them. Run
    // a message loop until we get our signal that the print job is satisfied.
    // PrintJob will send a ALL_PAGES_REQUESTED after having received all the
    // pages it needs. MessageLoop::current()->Quit() will be called as soon as
    // print_job_->document()->IsComplete() is true on either ALL_PAGES_REQUESTED
    // or in DidPrintPage(). The check is done in
    // ShouldQuitFromInnerMessageLoop().
    // BLOCKS until all the pages are received. (Need to enable recursive task)
    if (!RunInnerMessageLoop()) {
      // This function is always called from DisconnectFromCurrentPrintJob() so we
      // know that the job will be stopped/canceled in any case.
      return false;
    }
    return true;
}

// Quits the current message loop if these conditions hold true: a document is
// loaded and is complete and waiting_for_pages_to_be_rendered_ is true. This
// function is called in DidPrintPage() or on ALL_PAGES_REQUESTED
// notification. The inner message loop is created was created by
// RenderAllMissingPagesNow().
void PrintViewManagerBaseQt::ShouldQuitFromInnerMessageLoop()
{
    // Look at the reason.
    DCHECK(m_printJob->document());
    if (m_printJob->document() &&
        m_printJob->document()->IsComplete() &&
        m_isInsideInnerMessageLoop) {
      // We are in a message loop created by RenderAllMissingPagesNow. Quit from
      // it.
      base::RunLoop::QuitCurrentWhenIdleDeprecated();
      m_isInsideInnerMessageLoop = false;
    }
}

bool PrintViewManagerBaseQt::CreateNewPrintJob(printing::PrinterQuery *job)
{
    DCHECK(!m_isInsideInnerMessageLoop);

    // Disconnect the current |m_printJob|.
    DisconnectFromCurrentPrintJob();

    // We can't print if there is no renderer.
    if (!web_contents()->GetRenderViewHost() ||
        !web_contents()->GetRenderViewHost()->IsRenderViewLive()) {
        return false;
    }

    // Ask the renderer to generate the print preview, create the print preview
    // view and switch to it, initialize the printer and show the print dialog.
    DCHECK(!m_printJob.get());
    DCHECK(job);
    if (!job)
        return false;

    m_printJob = base::MakeRefCounted<printing::PrintJob>();
    m_printJob->Initialize(job, RenderSourceName(), number_pages_);
    m_registrar.Add(this, chrome::NOTIFICATION_PRINT_JOB_EVENT,
                    content::Source<printing::PrintJob>(m_printJob.get()));
    m_didPrintingSucceed = false;
    return true;
}

void PrintViewManagerBaseQt::DisconnectFromCurrentPrintJob()
{
    // Make sure all the necessary rendered page are done. Don't bother with the
    // return value.
    bool result = RenderAllMissingPagesNow();

    // Verify that assertion.
    if (m_printJob.get() &&
        m_printJob->document() &&
        !m_printJob->document()->IsComplete()) {
        DCHECK(!result);
        // That failed.
        TerminatePrintJob(true);
    } else {
        // DO NOT wait for the job to finish.
        ReleasePrintJob();
    }
}

void PrintViewManagerBaseQt::TerminatePrintJob(bool cancel)
{
    if (!m_printJob.get())
        return;

    if (cancel) {
        // We don't need the metafile data anymore because the printing is canceled.
        m_printJob->Cancel();
        m_isInsideInnerMessageLoop = false;
    } else {
        DCHECK(!m_isInsideInnerMessageLoop);
        DCHECK(!m_printJob->document() || m_printJob->document()->IsComplete());

        // WebContents is either dying or navigating elsewhere. We need to render
        // all the pages in an hurry if a print job is still pending. This does the
        // trick since it runs a blocking message loop:
        m_printJob->Stop();
    }
    ReleasePrintJob();
}

void PrintViewManagerBaseQt::ReleasePrintJob()
{
    content::RenderFrameHost* rfh = m_printingRFH;
    m_printingRFH = nullptr;

    if (!m_printJob.get())
        return;

    if (rfh) {
        auto msg = std::make_unique<PrintMsg_PrintingDone>(rfh->GetRoutingID(),
                                                           m_didPrintingSucceed);
        rfh->Send(msg.release());
    }


    m_registrar.Remove(this, chrome::NOTIFICATION_PRINT_JOB_EVENT,
                       content::Source<printing::PrintJob>(m_printJob.get()));
    // Don't close the worker thread.
    m_printJob = nullptr;
}

bool PrintViewManagerBaseQt::RunInnerMessageLoop() {
  // This value may actually be too low:
  //
  // - If we're looping because of printer settings initialization, the premise
  // here is that some poor users have their print server away on a VPN over a
  // slow connection. In this situation, the simple fact of opening the printer
  // can be dead slow. On the other side, we don't want to die infinitely for a
  // real network error. Give the printer 60 seconds to comply.
  //
  // - If we're looping because of renderer page generation, the renderer could
  // be CPU bound, the page overly complex/large or the system just
  // memory-bound.
  static const int kPrinterSettingsTimeout = 60000;
  base::OneShotTimer quit_timer;
  base::RunLoop run_loop;
  quit_timer.Start(FROM_HERE,
                   base::TimeDelta::FromMilliseconds(kPrinterSettingsTimeout),
                   run_loop.QuitWhenIdleClosure());

  m_isInsideInnerMessageLoop = true;

  // Need to enable recursive task.
  {
      base::MessageLoop::ScopedNestableTaskAllower allow;
      run_loop.Run();
  }

  bool success = true;
  if (m_isInsideInnerMessageLoop) {
    // Ok we timed out. That's sad.
    m_isInsideInnerMessageLoop = false;
    success = false;
  }

  return success;
}

bool PrintViewManagerBaseQt::OpportunisticallyCreatePrintJob(int cookie)
{
    if (m_printJob.get())
      return true;

    if (!cookie) {
      // Out of sync. It may happens since we are completely asynchronous. Old
      // spurious message can happen if one of the processes is overloaded.
      return false;
    }

    // The job was initiated by a script. Time to get the corresponding worker
    // thread.
    scoped_refptr<printing::PrinterQuery> queued_query = m_printerQueriesQueue->PopPrinterQuery(cookie);
    if (!queued_query.get()) {
      NOTREACHED();
      return false;
    }

    if (!CreateNewPrintJob(queued_query.get())) {
      // Don't kill anything.
      return false;
    }

    // Settings are already loaded. Go ahead. This will set
    // print_job_->is_job_pending() to true.
    m_printJob->StartPrinting();
    return true;
}

void PrintViewManagerBaseQt::ReleasePrinterQuery()
{
    if (!cookie_)
        return;

    int cookie = cookie_;
    cookie_ = 0;

    printing::PrintJobManager* printJobManager = WebEngineContext::current()->getPrintJobManager();
    // May be NULL in tests.
    if (!printJobManager)
        return;

    scoped_refptr<printing::PrinterQuery> printerQuery;
    printerQuery = m_printerQueriesQueue->PopPrinterQuery(cookie);
    if (!printerQuery.get())
        return;
    content::BrowserThread::PostTask(
            content::BrowserThread::IO, FROM_HERE,
            base::BindOnce(&printing::PrinterQuery::StopWorker, printerQuery.get()));
}

// Originally from print_preview_message_handler.cc:
void PrintViewManagerBaseQt::StopWorker(int documentCookie) {
  if (documentCookie <= 0)
    return;
  scoped_refptr<printing::PrinterQuery> printer_query =
      m_printerQueriesQueue->PopPrinterQuery(documentCookie);
  if (printer_query.get()) {
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
                            base::BindOnce(&printing::PrinterQuery::StopWorker, printer_query));
  }
}

void PrintViewManagerBaseQt::SendPrintingEnabled(bool enabled, content::RenderFrameHost* rfh)
{
    rfh->Send(new PrintMsg_SetPrintingEnabled(rfh->GetRoutingID(), enabled));
}

} // namespace QtWebEngineCore
