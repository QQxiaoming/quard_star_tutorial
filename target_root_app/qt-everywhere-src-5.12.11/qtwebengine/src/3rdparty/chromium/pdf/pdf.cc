// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/pdf.h"

#include <stdint.h>

#if defined(OS_WIN)
#include <windows.h>
#endif

#include "base/logging.h"
#include "base/macros.h"
#include "pdf/out_of_process_instance.h"
#include "pdf/pdf_ppapi.h"

namespace chrome_pdf {

namespace {

class ScopedSdkInitializer {
 public:
  ScopedSdkInitializer() {}
  ~ScopedSdkInitializer() {
#if DCHECK_IS_ON()
    DCHECK(initialized_);
#endif
    if (!IsSDKInitializedViaPepper())
      ShutdownSDK();
  }

  // Must be called.
  bool Init() {
#if DCHECK_IS_ON()
    initialized_ = true;
#endif
    return IsSDKInitializedViaPepper() || InitializeSDK();
  }

 private:
#if DCHECK_IS_ON()
  bool initialized_ = false;
#endif

  DISALLOW_COPY_AND_ASSIGN(ScopedSdkInitializer);
};

}  // namespace

#if defined(OS_WIN)
bool RenderPDFPageToDC(const void* pdf_buffer,
                       int buffer_size,
                       int page_number,
                       HDC dc,
                       int dpi_x,
                       int dpi_y,
                       int bounds_origin_x,
                       int bounds_origin_y,
                       int bounds_width,
                       int bounds_height,
                       bool fit_to_bounds,
                       bool stretch_to_bounds,
                       bool keep_aspect_ratio,
                       bool center_in_bounds,
                       bool autorotate,
                       bool use_color) {
  ScopedSdkInitializer scoped_sdk_initializer;
  if (!scoped_sdk_initializer.Init())
    return false;

  PDFEngineExports* engine_exports = PDFEngineExports::Get();
  PDFEngineExports::RenderingSettings settings(
      dpi_x, dpi_y,
      pp::Rect(bounds_origin_x, bounds_origin_y, bounds_width, bounds_height),
      fit_to_bounds, stretch_to_bounds, keep_aspect_ratio, center_in_bounds,
      autorotate, use_color);
  return engine_exports->RenderPDFPageToDC(pdf_buffer, buffer_size, page_number,
                                           settings, dc);
}

void SetPDFEnsureTypefaceCharactersAccessible(
    PDFEnsureTypefaceCharactersAccessible func) {
  PDFEngineExports::Get()->SetPDFEnsureTypefaceCharactersAccessible(func);
}

void SetPDFUseGDIPrinting(bool enable) {
  PDFEngineExports::Get()->SetPDFUseGDIPrinting(enable);
}

void SetPDFUsePrintMode(int mode) {
  PDFEngineExports::Get()->SetPDFUsePrintMode(mode);
}
#endif  // defined(OS_WIN)

bool GetPDFDocInfo(const void* pdf_buffer,
                   int buffer_size,
                   int* page_count,
                   double* max_page_width) {
  ScopedSdkInitializer scoped_sdk_initializer;
  if (!scoped_sdk_initializer.Init())
    return false;

  PDFEngineExports* engine_exports = PDFEngineExports::Get();
  return engine_exports->GetPDFDocInfo(pdf_buffer, buffer_size, page_count,
                                       max_page_width);
}

bool GetPDFPageSizeByIndex(const void* pdf_buffer,
                           int pdf_buffer_size,
                           int page_number,
                           double* width,
                           double* height) {
  ScopedSdkInitializer scoped_sdk_initializer;
  if (!scoped_sdk_initializer.Init())
    return false;

  chrome_pdf::PDFEngineExports* engine_exports =
      chrome_pdf::PDFEngineExports::Get();
  return engine_exports->GetPDFPageSizeByIndex(pdf_buffer, pdf_buffer_size,
                                               page_number, width, height);
}

bool RenderPDFPageToBitmap(const void* pdf_buffer,
                           int pdf_buffer_size,
                           int page_number,
                           void* bitmap_buffer,
                           int bitmap_width,
                           int bitmap_height,
                           int dpi_x,
                           int dpi_y,
                           bool autorotate,
                           bool use_color) {
  ScopedSdkInitializer scoped_sdk_initializer;
  if (!scoped_sdk_initializer.Init())
    return false;

  PDFEngineExports* engine_exports = PDFEngineExports::Get();
  PDFEngineExports::RenderingSettings settings(
      dpi_x, dpi_y, pp::Rect(bitmap_width, bitmap_height), true, false, true,
      true, autorotate, use_color);
  return engine_exports->RenderPDFPageToBitmap(
      pdf_buffer, pdf_buffer_size, page_number, settings, bitmap_buffer);
}

}  // namespace chrome_pdf
