// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/media/key_system_support_impl.h"

#include <string>
#include <vector>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/cdm_registry.h"
#include "content/public/browser/plugin_service.h"
#include "content/public/common/cdm_info.h"
#include "content/public/common/webplugininfo.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "media/base/decrypt_config.h"
#include "media/base/video_codecs.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace content {

namespace {

using VideoCodec = media::VideoCodec;
using EncryptionMode = media::EncryptionMode;
using CdmSessionType = media::CdmSessionType;

const char kTestCdmGuid[] = "62FE9C4B-384E-48FD-B28A-9F6F248BC8CC";
const char kVersion[] = "1.1.1.1";
const char kTestPath[] = "/aa/bb";
const char kTestFileSystemId[] = "file_system_id";

// Helper function to compare a STL container to an initializer_list.
template <typename Container, typename T>
bool StlEquals(const Container a, std::initializer_list<T> b) {
  return a == Container(b);
}

#define EXPECT_STL_EQ(a, ...)                 \
  do {                                        \
    EXPECT_TRUE(StlEquals(a, {__VA_ARGS__})); \
  } while (false)

#define EXPECT_VIDEO_CODECS(...) \
  EXPECT_STL_EQ(capability_->video_codecs, __VA_ARGS__)

#define EXPECT_ENCRYPTION_SCHEMES(...) \
  EXPECT_STL_EQ(capability_->encryption_schemes, __VA_ARGS__)

#define EXPECT_SESSION_TYPES(...) \
  EXPECT_STL_EQ(capability_->session_types, __VA_ARGS__)

}  // namespace

class KeySystemSupportTest : public testing::Test {
 protected:
  void SetUp() final {
    DVLOG(1) << __func__;
    KeySystemSupportImpl::Create(mojo::MakeRequest(&key_system_support_));
  }

  // TODO(xhwang): Add tests for hardware secure video codecs and encryption
  // schemes.
  CdmCapability GetTestCdmCapability() {
    return CdmCapability(
        {VideoCodec::kCodecVP8, VideoCodec::kCodecVP9},
        {EncryptionMode::kCenc, EncryptionMode::kCbcs},
        {CdmSessionType::kTemporary, CdmSessionType::kPersistentLicense}, {});
  }

  // Registers |key_system| with |capability|. All other values for CdmInfo have
  // some default value as they're not returned by IsKeySystemSupported().
  void Register(const std::string& key_system, CdmCapability capability) {
    DVLOG(1) << __func__;

    CdmRegistry::GetInstance()->RegisterCdm(
        CdmInfo(key_system, kTestCdmGuid, base::Version(kVersion),
                base::FilePath::FromUTF8Unsafe(kTestPath), kTestFileSystemId,
                std::move(capability), key_system, false));
  }

  // Determines if |key_system| is registered. If it is, updates |codecs_|
  // and |persistent_|.
  bool IsSupported(const std::string& key_system) {
    DVLOG(1) << __func__;
    bool is_available = false;
    key_system_support_->IsKeySystemSupported(key_system, &is_available,
                                              &capability_);
    return is_available;
  }

  media::mojom::KeySystemSupportPtr key_system_support_;
  TestBrowserThreadBundle test_browser_thread_bundle_;

  // Updated by IsSupported().
  media::mojom::KeySystemCapabilityPtr capability_;
};

// Note that as CdmRegistry::GetInstance() is a static, it is shared between
// tests. So use unique key system names in each test below to avoid
// interactions between the tests.

TEST_F(KeySystemSupportTest, NoKeySystems) {
  EXPECT_FALSE(IsSupported("KeySystem1"));
  EXPECT_FALSE(capability_);
}

TEST_F(KeySystemSupportTest, OneKeySystem) {
  Register("KeySystem2", GetTestCdmCapability());

  EXPECT_TRUE(IsSupported("KeySystem2"));
  EXPECT_VIDEO_CODECS(VideoCodec::kCodecVP8, VideoCodec::kCodecVP9);
  EXPECT_ENCRYPTION_SCHEMES(EncryptionMode::kCenc, EncryptionMode::kCbcs);
  EXPECT_SESSION_TYPES(CdmSessionType::kTemporary,
                       CdmSessionType::kPersistentLicense);
}

TEST_F(KeySystemSupportTest, MultipleKeySystems) {
  Register("KeySystem3", GetTestCdmCapability());
  Register("KeySystem4", GetTestCdmCapability());

  EXPECT_TRUE(IsSupported("KeySystem3"));
  EXPECT_TRUE(IsSupported("KeySystem4"));
}

TEST_F(KeySystemSupportTest, MissingKeySystem) {
  Register("KeySystem5", GetTestCdmCapability());

  EXPECT_FALSE(IsSupported("KeySystem6"));
  EXPECT_FALSE(capability_);
}

}  // namespace content
