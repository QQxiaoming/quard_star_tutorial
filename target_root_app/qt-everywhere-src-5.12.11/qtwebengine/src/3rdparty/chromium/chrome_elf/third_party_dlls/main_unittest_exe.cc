// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <windows.h>

#include "chrome_elf/third_party_dlls/main_unittest_exe.h"

#include <stdlib.h>

#include <shellapi.h>

#include "base/files/file.h"
#include "base/files/file_util.h"
#include "base/scoped_native_library.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/test_reg_util_win.h"
#include "chrome/install_static/install_util.h"
#include "chrome/install_static/product_install_details.h"
#include "chrome_elf/nt_registry/nt_registry.h"
#include "chrome_elf/third_party_dlls/logging_api.h"
#include "chrome_elf/third_party_dlls/main.h"
#include "chrome_elf/third_party_dlls/packed_list_file.h"

using namespace third_party_dlls::main_unittest_exe;

namespace {

// Function object which invokes LocalFree on its parameter, which must be
// a pointer.  To be used with std::unique_ptr and CommandLineToArgvW().
struct LocalFreeDeleter {
  inline void operator()(wchar_t** ptr) const { ::LocalFree(ptr); }
};

// Attempt to load a given DLL.
ExitCode LoadDll(std::wstring name) {
  base::FilePath dll_path(name);
  base::ScopedNativeLibrary dll(dll_path);
  if (!dll.is_valid())
    return kDllLoadFailed;

  return kDllLoadSuccess;
}

// Utility function to protect the local registry.
void RegRedirect(registry_util::RegistryOverrideManager* rom) {
  base::string16 temp;
  rom->OverrideRegistry(HKEY_CURRENT_USER, &temp);
  nt::SetTestingOverride(nt::HKCU, temp);
}

// Compare an argument path with a module-load log path.
// - |arg_path| is a UTF-16 drive path.
// - |log.section_path| is UTF-8, and will be a device path, so convert to drive
//   letter before comparing.
bool MatchPath(const wchar_t* arg_path, const third_party_dlls::LogEntry& log) {
  base::FilePath drive_path;
  if (!base::DevicePathToDriveLetterPath(
          base::FilePath(base::UTF8ToUTF16(log.path)), &drive_path)) {
    return false;
  }

  if (drive_path.value().compare(arg_path) != 0)
    return false;

  return true;
}

}  // namespace

//------------------------------------------------------------------------------
// PUBLIC
//------------------------------------------------------------------------------

// Good ol' main.
// - Init third_party_dlls, which will apply a hook to NtMapViewOfSection.
// - Attempt to load a specific DLL.
//
// Arguments:
// #1: path to test blacklist file (mandatory).
// #2: test identifier (mandatory).
// #3: path to dll (test-identifier dependent).
//
// Returns:
// - Negative values in case of unexpected error.
// - 0 for successful DLL load.
// - 1 for failed DLL load.
int main() {
  // NOTE: The arguments must be treated as unicode for these tests.
  int argument_count = 0;
  std::unique_ptr<wchar_t* [], LocalFreeDeleter> argv(
      ::CommandLineToArgvW(::GetCommandLineW(), &argument_count));
  if (!argv)
    return kBadCommandLine;

  if (third_party_dlls::IsThirdPartyInitialized())
    return kThirdPartyAlreadyInitialized;

  install_static::InitializeProductDetailsForPrimaryModule();
  install_static::InitializeProcessType();

  // Get the required arguments, path to blacklist file and test id to run.
  if (argument_count < 3)
    return kMissingArgument;

  const wchar_t* blacklist_path = argv[1];
  if (!blacklist_path || ::wcslen(blacklist_path) == 0) {
    return kBadBlacklistPath;
  }

  const wchar_t* arg2 = argv[2];
  int test_id = ::_wtoi(arg2);
  if (!test_id)
    return kUnsupportedTestId;

  // Override blacklist path before initializing.
  third_party_dlls::OverrideFilePathForTesting(blacklist_path);

  // Enable a registry test net before initializing.
  registry_util::RegistryOverrideManager rom;
  RegRedirect(&rom);

  if (!third_party_dlls::Init())
    return kThirdPartyInitFailure;

  // Switch on test id.
  switch (test_id) {
    // Just initialization.
    case kTestOnlyInitialization:
      break;
    // Single DLL load.
    case kTestSingleDllLoad:
    // Single DLL load with log path scrutiny.
    case kTestLogPath: {
      if (argument_count < 4)
        return kMissingArgument;
      const wchar_t* dll_name = argv[3];
      if (!dll_name || ::wcslen(dll_name) == 0)
        return kBadArgument;
      ExitCode code = LoadDll(dll_name);

      // Get logging.  Ensure the log is as expected.
      uint32_t bytes = 0;
      DrainLog(nullptr, 0, &bytes);
      if (!bytes)
        return kEmptyLog;
      auto buffer = std::unique_ptr<uint8_t[]>(new uint8_t[bytes]);
      bytes = DrainLog(&buffer[0], bytes, nullptr);
      third_party_dlls::LogEntry* entry =
          reinterpret_cast<third_party_dlls::LogEntry*>(&buffer[0]);
      if (!bytes || bytes < third_party_dlls::GetLogEntrySize(entry->path_len))
        return kBadLogEntrySize;

      if ((code == kDllLoadFailed &&
           entry->type != third_party_dlls::kBlocked) ||
          (code == kDllLoadSuccess &&
           entry->type != third_party_dlls::kAllowed)) {
        return kUnexpectedLog;
      }

      if (test_id == kTestLogPath && !MatchPath(dll_name, *entry))
        return kUnexpectedSectionPath;

      return code;
    }
    // Unsupported argument.
    default:
      return kUnsupportedTestId;
  }

  return 0;
}
