// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_COMMON_FILLING_STATUS_H_
#define COMPONENTS_AUTOFILL_CORE_COMMON_FILLING_STATUS_H_

namespace autofill {

// Describes how a manual filling attempt ended.
enum class FillingStatus {
  SUCCESS,               // The selected credential was filled into the field.
  ERROR_NO_VALID_FIELD,  // The focused field was invalid or focus was lost.
  ERROR_NOT_ALLOWED,  // Filling not permitted (e.g. password in email field).
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_COMMON_FILLING_STATUS_H_
