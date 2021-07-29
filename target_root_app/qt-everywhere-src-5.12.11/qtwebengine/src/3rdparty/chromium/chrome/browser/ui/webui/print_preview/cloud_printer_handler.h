// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_PRINT_PREVIEW_CLOUD_PRINTER_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_PRINT_PREVIEW_CLOUD_PRINTER_HANDLER_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "chrome/browser/ui/webui/print_preview/printer_handler.h"

// Implementation of PrinterHandler interface
class CloudPrinterHandler : public PrinterHandler {
 public:
  CloudPrinterHandler();

  ~CloudPrinterHandler() override;

  // PrinterHandler implementation:
  void Reset() override;
  void StartGetPrinters(const AddedPrintersCallback& added_printers_callback,
                        GetPrintersDoneCallback done_callback) override;
  void StartGetCapability(const std::string& destination_id,
                          GetCapabilityCallback calback) override;
  void StartPrint(const std::string& destination_id,
                  const std::string& capability,
                  const base::string16& job_title,
                  const std::string& ticket_json,
                  const gfx::Size& page_size,
                  const scoped_refptr<base::RefCountedMemory>& print_data,
                  PrintCallback callback) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(CloudPrinterHandler);
};
#endif  // CHROME_BROWSER_UI_WEBUI_PRINT_PREVIEW_CLOUD_PRINTER_HANDLER_H_
