// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_FEEDBACK_FEEDBACK_REPORT_H_
#define COMPONENTS_FEEDBACK_FEEDBACK_REPORT_H_

#include <string>

#include "base/callback_forward.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/time/time.h"

namespace base {
class SequencedTaskRunner;
}

namespace feedback {

// Repeating since for every feedback report file on disk, the callback to
// queue it in the uploader needs to be invoked.
using QueueCallback =
    base::RepeatingCallback<void(std::unique_ptr<std::string>)>;

// This class holds a feedback report. Once a report is created, a disk backup
// for it is created automatically. This backup needs to explicitly be
// deleted by calling DeleteReportOnDisk.
class FeedbackReport : public base::RefCountedThreadSafe<FeedbackReport> {
 public:
  FeedbackReport(const base::FilePath& path,
                 const base::Time& upload_at,
                 std::unique_ptr<std::string> data,
                 scoped_refptr<base::SequencedTaskRunner> task_runner);

  // The ID of the product specific data for the crash report IDs as stored by
  // the feedback server.
  static const char kCrashReportIdsKey[];

  // Loads the reports still on disk and queues then using the given callback.
  // This call blocks on the file reads.
  static void LoadReportsAndQueue(const base::FilePath& user_dir,
                                  const QueueCallback& callback);

  // Stops the disk write of the report and deletes the report file if already
  // written.
  void DeleteReportOnDisk();

  const base::Time& upload_at() const { return upload_at_; }
  void set_upload_at(const base::Time& time) { upload_at_ = time; }
  const std::string& data() const { return *data_; }

 private:
  friend class base::RefCountedThreadSafe<FeedbackReport>;
  virtual ~FeedbackReport();

  // Name of the file corresponding to this report.
  base::FilePath file_;

  base::FilePath reports_path_;
  base::Time upload_at_;  // Upload this report at or after this time.
  std::unique_ptr<std::string> data_;

  scoped_refptr<base::SequencedTaskRunner> reports_task_runner_;

  DISALLOW_COPY_AND_ASSIGN(FeedbackReport);
};

}  // namespace feedback

#endif  // COMPONENTS_FEEDBACK_FEEDBACK_REPORT_H_
