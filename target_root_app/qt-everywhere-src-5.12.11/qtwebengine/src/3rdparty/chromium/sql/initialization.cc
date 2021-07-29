// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sql/initialization.h"

#include "base/bind.h"
#include "base/metrics/histogram_macros.h"
#include "base/no_destructor.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "build/build_config.h"
#include "third_party/sqlite/sqlite3.h"

namespace sql {

namespace {

#if !defined(OS_IOS)
void RecordSqliteMemory10Min() {
  const int64_t used = sqlite3_memory_used();
  UMA_HISTOGRAM_COUNTS("Sqlite.MemoryKB.TenMinutes", used / 1024);
}

void RecordSqliteMemoryHour() {
  const int64_t used = sqlite3_memory_used();
  UMA_HISTOGRAM_COUNTS("Sqlite.MemoryKB.OneHour", used / 1024);
}

void RecordSqliteMemoryDay() {
  const int64_t used = sqlite3_memory_used();
  UMA_HISTOGRAM_COUNTS("Sqlite.MemoryKB.OneDay", used / 1024);
}

void RecordSqliteMemoryWeek() {
  const int64_t used = sqlite3_memory_used();
  UMA_HISTOGRAM_COUNTS("Sqlite.MemoryKB.OneWeek", used / 1024);
}
#endif  // !defined(OS_IOS)

}  // anonymous namespace

void EnsureSqliteInitialized() {
  // sqlite3_initialize() uses double-checked locking and thus can have
  // data races.
  static base::NoDestructor<base::Lock> sqlite_init_lock;
  base::AutoLock auto_lock(*sqlite_init_lock);

  static bool first_call = true;
  if (first_call) {
    sqlite3_initialize();

#if !defined(OS_IOS)
    // Schedule callback to record memory footprint histograms at 10m, 1h, and
    // 1d. There may not be a registered task runner in tests.
    // TODO(crbug.com/861889): Disable very long critical tasks on iOS until
    // 861889 is fixed.
    if (base::SequencedTaskRunnerHandle::IsSet()) {
      base::SequencedTaskRunnerHandle::Get()->PostDelayedTask(
          FROM_HERE, base::BindOnce(&RecordSqliteMemory10Min),
          base::TimeDelta::FromMinutes(10));
      base::SequencedTaskRunnerHandle::Get()->PostDelayedTask(
          FROM_HERE, base::BindOnce(&RecordSqliteMemoryHour),
          base::TimeDelta::FromHours(1));
      base::SequencedTaskRunnerHandle::Get()->PostDelayedTask(
          FROM_HERE, base::BindOnce(&RecordSqliteMemoryDay),
          base::TimeDelta::FromDays(1));
      base::SequencedTaskRunnerHandle::Get()->PostDelayedTask(
          FROM_HERE, base::BindOnce(&RecordSqliteMemoryWeek),
          base::TimeDelta::FromDays(7));
    }
#endif  // !defined(OS_IOS)
    first_call = false;
  }
}

}  // namespace sql
