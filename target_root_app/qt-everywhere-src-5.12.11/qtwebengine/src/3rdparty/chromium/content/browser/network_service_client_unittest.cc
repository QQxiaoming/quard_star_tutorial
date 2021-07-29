// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/network_service_client.h"

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/test/scoped_task_environment.h"
#include "content/browser/child_process_security_policy_impl.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace content {

namespace {

struct UploadResponse {
  UploadResponse()
      : callback(base::BindOnce(&UploadResponse::OnComplete,
                                base::Unretained(this))) {}

  void OnComplete(int error_code, std::vector<base::File> opened_files) {
    this->error_code = error_code;
    this->opened_files = std::move(opened_files);
  }

  network::mojom::NetworkServiceClient::OnFileUploadRequestedCallback callback;
  int error_code;
  std::vector<base::File> opened_files;
};

void GrantAccess(const base::FilePath& file, int process_id) {
  ChildProcessSecurityPolicy::GetInstance()->GrantReadFile(process_id, file);
}

void CreateFile(const base::FilePath& path, const char* content) {
  base::File file(path, base::File::FLAG_CREATE | base::File::FLAG_WRITE);
  ASSERT_TRUE(file.IsValid());
  int content_size = strlen(content);
  int bytes_written = file.Write(0, content, content_size);
  EXPECT_EQ(bytes_written, content_size);
}

void ValidateFileContents(base::File& file, const char* expected_content) {
  int expected_length = strlen(expected_content);
  ASSERT_EQ(file.GetLength(), expected_length);
  char content[expected_length];
  file.Read(0, content, expected_length);
  EXPECT_EQ(0, strncmp(content, expected_content, expected_length));
}

const int kBrowserProcessId = 0;
const int kRendererProcessId = 1;
const char kFileContent1[] = "test file content one";
const char kFileContent2[] = "test file content two";

}  // namespace

class NetworkServiceClientTest : public testing::Test {
 public:
  NetworkServiceClientTest() : client_(mojo::MakeRequest(&client_ptr_)) {}

  void SetUp() override {
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
    ChildProcessSecurityPolicyImpl::GetInstance()->Add(kRendererProcessId);
  }

  void TearDown() override {
    ChildProcessSecurityPolicyImpl::GetInstance()->Remove(kRendererProcessId);
  }

 protected:
  base::test::ScopedTaskEnvironment scoped_task_environment_;
  network::mojom::NetworkServiceClientPtr client_ptr_;
  NetworkServiceClient client_;
  base::ScopedTempDir temp_dir_;
};

TEST_F(NetworkServiceClientTest, UploadNoFiles) {
  UploadResponse response;
  client_.OnFileUploadRequested(kRendererProcessId, true, {},
                                std::move(response.callback));
  scoped_task_environment_.RunUntilIdle();
  EXPECT_EQ(net::OK, response.error_code);
  EXPECT_EQ(0U, response.opened_files.size());
}

TEST_F(NetworkServiceClientTest, UploadOneValidAsyncFile) {
  base::FilePath path = temp_dir_.GetPath().AppendASCII("filename");
  CreateFile(path, kFileContent1);
  GrantAccess(path, kRendererProcessId);

  UploadResponse response;
  client_.OnFileUploadRequested(kRendererProcessId, true, {path},
                                std::move(response.callback));
  scoped_task_environment_.RunUntilIdle();
  EXPECT_EQ(net::OK, response.error_code);
  ASSERT_EQ(1U, response.opened_files.size());
  EXPECT_TRUE(response.opened_files[0].async());
}

TEST_F(NetworkServiceClientTest, UploadOneValidFile) {
  base::FilePath path = temp_dir_.GetPath().AppendASCII("filename");
  CreateFile(path, kFileContent1);
  GrantAccess(path, kRendererProcessId);

  UploadResponse response;
  client_.OnFileUploadRequested(kRendererProcessId, false, {path},
                                std::move(response.callback));
  scoped_task_environment_.RunUntilIdle();
  EXPECT_EQ(net::OK, response.error_code);
  ASSERT_EQ(1U, response.opened_files.size());
  EXPECT_FALSE(response.opened_files[0].async());
  ValidateFileContents(response.opened_files[0], kFileContent1);
}

TEST_F(NetworkServiceClientTest, UploadTwoValidFiles) {
  base::FilePath path1 = temp_dir_.GetPath().AppendASCII("filename1");
  base::FilePath path2 = temp_dir_.GetPath().AppendASCII("filename2");
  CreateFile(path1, kFileContent1);
  CreateFile(path2, kFileContent2);
  GrantAccess(path1, kRendererProcessId);
  GrantAccess(path2, kRendererProcessId);

  UploadResponse response;
  client_.OnFileUploadRequested(kRendererProcessId, false, {path1, path2},
                                std::move(response.callback));
  scoped_task_environment_.RunUntilIdle();
  EXPECT_EQ(net::OK, response.error_code);
  ASSERT_EQ(2U, response.opened_files.size());
  ValidateFileContents(response.opened_files[0], kFileContent1);
  ValidateFileContents(response.opened_files[1], kFileContent2);
}

TEST_F(NetworkServiceClientTest, UploadOneUnauthorizedFile) {
  base::FilePath path = temp_dir_.GetPath().AppendASCII("filename");
  CreateFile(path, kFileContent1);

  UploadResponse response;
  client_.OnFileUploadRequested(kRendererProcessId, false, {path},
                                std::move(response.callback));
  scoped_task_environment_.RunUntilIdle();
  EXPECT_EQ(net::ERR_ACCESS_DENIED, response.error_code);
  EXPECT_EQ(0U, response.opened_files.size());
}

TEST_F(NetworkServiceClientTest, UploadOneValidFileAndOneUnauthorized) {
  base::FilePath path1 = temp_dir_.GetPath().AppendASCII("filename1");
  base::FilePath path2 = temp_dir_.GetPath().AppendASCII("filename2");
  CreateFile(path1, kFileContent1);
  CreateFile(path2, kFileContent2);
  GrantAccess(path1, kRendererProcessId);

  UploadResponse response;
  client_.OnFileUploadRequested(kRendererProcessId, false, {path1, path2},
                                std::move(response.callback));
  scoped_task_environment_.RunUntilIdle();
  EXPECT_EQ(net::ERR_ACCESS_DENIED, response.error_code);
  EXPECT_EQ(0U, response.opened_files.size());
}

TEST_F(NetworkServiceClientTest, UploadOneValidFileAndOneNotFound) {
  base::FilePath path1 = temp_dir_.GetPath().AppendASCII("filename1");
  base::FilePath path2 = temp_dir_.GetPath().AppendASCII("filename2");
  CreateFile(path1, kFileContent1);
  GrantAccess(path1, kRendererProcessId);
  GrantAccess(path2, kRendererProcessId);

  UploadResponse response;
  client_.OnFileUploadRequested(kRendererProcessId, false, {path1, path2},
                                std::move(response.callback));
  scoped_task_environment_.RunUntilIdle();
  EXPECT_EQ(net::ERR_FILE_NOT_FOUND, response.error_code);
  EXPECT_EQ(0U, response.opened_files.size());
}

TEST_F(NetworkServiceClientTest, UploadFromBrowserProcess) {
  base::FilePath path = temp_dir_.GetPath().AppendASCII("filename");
  CreateFile(path, kFileContent1);
  // No grant necessary for browser process.

  UploadResponse response;
  client_.OnFileUploadRequested(kBrowserProcessId, false, {path},
                                std::move(response.callback));
  scoped_task_environment_.RunUntilIdle();
  EXPECT_EQ(net::OK, response.error_code);
  ASSERT_EQ(1U, response.opened_files.size());
  ValidateFileContents(response.opened_files[0], kFileContent1);
}

}  // namespace content
