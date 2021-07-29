// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "base/files/file.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "tools/gn/functions.h"
#include "tools/gn/scheduler.h"
#include "tools/gn/test_with_scheduler.h"
#include "tools/gn/test_with_scope.h"
#include "util/test/test.h"

#if defined(OS_LINUX) || defined(OS_MACOSX)
#include <sys/time.h>
#endif

namespace {

// Returns true on success, false if write_file signaled an error.
bool CallWriteFile(Scope* scope,
                   const std::string& filename,
                   const Value& data) {
  Err err;

  std::vector<Value> args;
  args.push_back(Value(nullptr, filename));
  args.push_back(data);

  FunctionCallNode function_call;
  Value result = functions::RunWriteFile(scope, &function_call, args, &err);
  EXPECT_EQ(Value::NONE, result.type());  // Should always return none.

  return !err.has_error();
}

}  // namespace

using WriteFileTest = TestWithScheduler;

TEST_F(WriteFileTest, WithData) {
  TestWithScope setup;

  // Make a real directory for writing the files.
  base::ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
  setup.build_settings()->SetRootPath(temp_dir.GetPath());
  setup.build_settings()->SetBuildDir(SourceDir("//out/"));

  Value some_string(nullptr, "some string contents");

  // Should refuse to write files outside of the output dir.
  EXPECT_FALSE(CallWriteFile(setup.scope(), "//in_root.txt", some_string));
  EXPECT_FALSE(
      CallWriteFile(setup.scope(), "//other_dir/foo.txt", some_string));

  // Should be able to write to a new dir inside the out dir.
  EXPECT_TRUE(CallWriteFile(setup.scope(), "//out/foo.txt", some_string));
  base::FilePath foo_name = temp_dir.GetPath()
                                .Append(FILE_PATH_LITERAL("out"))
                                .Append(FILE_PATH_LITERAL("foo.txt"));
  std::string result_contents;
  EXPECT_TRUE(base::ReadFileToString(foo_name, &result_contents));
  EXPECT_EQ(some_string.string_value(), result_contents);

  // Update the contents with a list of a string and a number.
  Value some_list(nullptr, Value::LIST);
  some_list.list_value().push_back(Value(nullptr, "line 1"));
  some_list.list_value().push_back(Value(nullptr, static_cast<int64_t>(2)));
  EXPECT_TRUE(CallWriteFile(setup.scope(), "//out/foo.txt", some_list));
  EXPECT_TRUE(base::ReadFileToString(foo_name, &result_contents));
  EXPECT_EQ("line 1\n2\n", result_contents);

  // Test that the file is not rewritten if the contents are not changed.
  base::File foo_file(foo_name, base::File::FLAG_OPEN | base::File::FLAG_READ |
                                    base::File::FLAG_WRITE);
  ASSERT_TRUE(foo_file.IsValid());

  // Start by setting the modified time to something old to avoid clock
  // resolution issues.
#if defined(OS_WIN)
  FILETIME last_access_filetime = {};
  FILETIME last_modified_filetime = {};
  ASSERT_TRUE(::SetFileTime(foo_file.GetPlatformFile(), nullptr,
                            &last_access_filetime, &last_modified_filetime));
#elif defined(OS_AIX)
  struct timeval times[2] = {};
  ASSERT_EQ(utimes(foo_name.AsUTF8Unsafe().c_str(), times), 0);
#else
  struct timeval times[2] = {};
  ASSERT_EQ(futimes(foo_file.GetPlatformFile(), times), 0);
#endif

  // Read the current time to avoid timer resolution issues when comparing
  // below.
  base::File::Info original_info;
  foo_file.GetInfo(&original_info);

  EXPECT_TRUE(CallWriteFile(setup.scope(), "//out/foo.txt", some_list));

  // Verify that the last modified time is the same as before.
  base::File::Info new_info;
  foo_file.GetInfo(&new_info);
  EXPECT_EQ(original_info.last_modified, new_info.last_modified);
}
