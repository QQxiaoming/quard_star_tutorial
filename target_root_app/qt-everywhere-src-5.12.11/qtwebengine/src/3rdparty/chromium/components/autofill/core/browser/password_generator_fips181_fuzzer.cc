// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "components/autofill/core/browser/password_generator_fips181.h"

namespace autofill {

namespace {

const char* g_password_text = nullptr;

// The "PasswordGeneratorFips181" is a wrapper around Fips181's gen_pron_pass().
// The former processes the random string from the latter and ensures that it
// meets some constraints. GenerateForTest here substitutes for gen_pron_pass(),
// so that the fuzzer tests the wrapper's logic rather than the third-party's
// generator implementation.
int GenerateForTest(char* word,
                    char* hypenated_word,
                    unsigned short minlen,
                    unsigned short maxlen,
                    unsigned int pass_mode) {
  strncpy(word, g_password_text, maxlen);
  g_password_text = nullptr;
  // Resize password to |maxlen|.
  word[maxlen] = '\0';
  return static_cast<int>(strlen(word));
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  autofill::PasswordGeneratorFips181::SetGeneratorForTest(GenerateForTest);
  std::string generator_string(reinterpret_cast<const char*>(data), size);
  g_password_text = generator_string.c_str();
  autofill::PasswordGeneratorFips181 pg(size);
  std::string password = pg.Generate();
  autofill::PasswordGeneratorFips181::SetGeneratorForTest(nullptr);
  return 0;
}

}  // namespace autofill
