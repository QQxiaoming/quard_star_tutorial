// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <memory>
#include <vector>

#include "base/logging.h"
#include "components/zucchini/buffer_view.h"
#include "components/zucchini/disassembler.h"
#include "components/zucchini/disassembler_win32.h"

namespace {

struct Environment {
  Environment() {
    logging::SetMinLogLevel(logging::LOG_FATAL);  // Disable console spamming.
  }
};

// Helper function that uses |disassembler| to read all references from
// |mutable_image| and write them back.
void ReadAndWriteReferences(
    std::unique_ptr<zucchini::Disassembler> disassembler,
    std::vector<uint8_t>* mutable_data) {
  zucchini::MutableBufferView mutable_image(mutable_data->data(),
                                            disassembler->size());
  std::vector<zucchini::Reference> references;
  auto groups = disassembler->MakeReferenceGroups();
  std::map<zucchini::PoolTag, std::vector<zucchini::Reference>>
      references_of_pool;
  for (const auto& group : groups) {
    auto reader = group.GetReader(disassembler.get());
    std::vector<zucchini::Reference>* refs =
        &references_of_pool[group.pool_tag()];
    for (auto ref = reader->GetNext(); ref.has_value();
         ref = reader->GetNext()) {
      refs->push_back(ref.value());
    }
  }
  for (const auto& group : groups) {
    auto writer = group.GetWriter(mutable_image, disassembler.get());
    for (const auto& ref : references_of_pool[group.pool_tag()])
      writer->PutNext(ref);
  }
}

}  // namespace

// Entry point for LibFuzzer.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  static Environment env;
  if (!size)
    return 0;
  // Prepare data.
  std::vector<uint8_t> mutable_data(data, data + size);
  zucchini::ConstBufferView image(mutable_data.data(), mutable_data.size());

  // One of x86 or x64 should return a non-nullptr if the data is valid.
  auto disassembler_win32x86 =
      zucchini::Disassembler::Make<zucchini::DisassemblerWin32X86>(image);
  if (disassembler_win32x86) {
    ReadAndWriteReferences(std::move(disassembler_win32x86), &mutable_data);
    return 0;
  }

  auto disassembler_win32x64 =
      zucchini::Disassembler::Make<zucchini::DisassemblerWin32X64>(image);
  if (disassembler_win32x64)
    ReadAndWriteReferences(std::move(disassembler_win32x64), &mutable_data);
  return 0;
}
