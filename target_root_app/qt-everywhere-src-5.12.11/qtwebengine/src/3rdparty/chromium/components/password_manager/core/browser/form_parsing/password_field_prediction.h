// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_FORM_PARSING_PASSWORD_FIELD_PREDICTION_H_
#define COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_FORM_PARSING_PASSWORD_FIELD_PREDICTION_H_

#include <stdint.h>
#include <map>

#include "components/autofill/core/browser/field_types.h"

namespace autofill {
class FormStructure;
}  // namespace autofill

namespace password_manager {

enum class CredentialFieldType {
  kNone,
  kUsername,
  kCurrentPassword,
  kNewPassword,
  kConfirmationPassword
};

// Transforms the general field type to the information useful for password
// forms.
CredentialFieldType DeriveFromServerFieldType(autofill::ServerFieldType type);

// Contains server predictions for a field.
struct PasswordFieldPrediction {
  autofill::ServerFieldType type;
  bool may_use_prefilled_placeholder = false;
};

// Contains server predictions for a form. Keys are unique renderer ids of
// fields.
using FormPredictions = std::map<uint32_t, PasswordFieldPrediction>;

// Extracts all password related server predictions from |form_structure|.
FormPredictions ConvertToFormPredictions(
    const autofill::FormStructure& form_structure);

}  // namespace password_manager

#endif  // COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_FORM_PARSING_PASSWORD_FIELD_PREDICTION_H_
