// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/test/test_simple_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "gin/v8_initializer.h"
#include "pdf/pdf.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace chrome_pdf {

namespace {

void LoadV8SnapshotData() {
#if defined(V8_USE_EXTERNAL_STARTUP_DATA)
  static bool loaded = false;
  if (!loaded) {
    loaded = true;
    gin::V8Initializer::LoadV8Snapshot();
    gin::V8Initializer::LoadV8Natives();
  }
#endif
}

class PDFiumEngineExportsTest : public testing::Test {
 public:
  PDFiumEngineExportsTest() = default;
  ~PDFiumEngineExportsTest() override = default;

 protected:
  void SetUp() override {
    LoadV8SnapshotData();

    handle_ = std::make_unique<base::ThreadTaskRunnerHandle>(
        base::MakeRefCounted<base::TestSimpleTaskRunner>());

    CHECK(base::PathService::Get(base::DIR_SOURCE_ROOT, &pdf_data_dir_));
    pdf_data_dir_ = pdf_data_dir_.Append(FILE_PATH_LITERAL("pdf"))
                        .Append(FILE_PATH_LITERAL("test"))
                        .Append(FILE_PATH_LITERAL("data"));
  }

  const base::FilePath& pdf_data_dir() const { return pdf_data_dir_; }

 private:
  std::unique_ptr<base::ThreadTaskRunnerHandle> handle_;
  base::FilePath pdf_data_dir_;

  DISALLOW_COPY_AND_ASSIGN(PDFiumEngineExportsTest);
};

}  // namespace

TEST_F(PDFiumEngineExportsTest, GetPDFDocInfo) {
  base::FilePath pdf_path =
      pdf_data_dir().Append(FILE_PATH_LITERAL("hello_world2.pdf"));
  std::string pdf_data;
  ASSERT_TRUE(base::ReadFileToString(pdf_path, &pdf_data));

  EXPECT_FALSE(GetPDFDocInfo(nullptr, 0, nullptr, nullptr));

  ASSERT_TRUE(
      GetPDFDocInfo(pdf_data.data(), pdf_data.size(), nullptr, nullptr));

  int page_count;
  double max_page_width;
  ASSERT_TRUE(GetPDFDocInfo(pdf_data.data(), pdf_data.size(), &page_count,
                            &max_page_width));
  EXPECT_EQ(2, page_count);
  EXPECT_DOUBLE_EQ(200.0, max_page_width);
}

TEST_F(PDFiumEngineExportsTest, GetPDFPageSizeByIndex) {
  // TODO(thestig): Use a better PDF for this test, as hello_world2.pdf's page
  // dimensions are uninteresting.
  base::FilePath pdf_path =
      pdf_data_dir().Append(FILE_PATH_LITERAL("hello_world2.pdf"));
  std::string pdf_data;
  ASSERT_TRUE(base::ReadFileToString(pdf_path, &pdf_data));

  EXPECT_FALSE(GetPDFPageSizeByIndex(nullptr, 0, 0, nullptr, nullptr));

  int page_count;
  ASSERT_TRUE(
      GetPDFDocInfo(pdf_data.data(), pdf_data.size(), &page_count, nullptr));
  ASSERT_EQ(2, page_count);
  for (int page_number = 0; page_number < page_count; ++page_number) {
    double width;
    double height;
    ASSERT_TRUE(GetPDFPageSizeByIndex(pdf_data.data(), pdf_data.size(),
                                      page_number, &width, &height));
    EXPECT_DOUBLE_EQ(200.0, width);
    EXPECT_DOUBLE_EQ(200.0, height);
  }
}

}  // namespace chrome_pdf
