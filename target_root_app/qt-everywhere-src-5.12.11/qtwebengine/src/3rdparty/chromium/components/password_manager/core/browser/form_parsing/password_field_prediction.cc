// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/password_manager/core/browser/form_parsing/password_field_prediction.h"

#include "base/logging.h"
#include "components/autofill/core/browser/form_structure.h"
#include "components/autofill/core/common/form_data.h"

using autofill::FormData;
using autofill::FormStructure;
using autofill::ServerFieldType;

namespace password_manager {

namespace {

// Returns true if the field is password or username prediction.
bool IsCredentialRelatedPrediction(ServerFieldType type) {
  return DeriveFromServerFieldType(type) != CredentialFieldType::kNone;
}

}  // namespace

CredentialFieldType DeriveFromServerFieldType(ServerFieldType type) {
  switch (type) {
    case autofill::USERNAME:
    case autofill::USERNAME_AND_EMAIL_ADDRESS:
      return CredentialFieldType::kUsername;
    case autofill::PASSWORD:
      return CredentialFieldType::kCurrentPassword;
    case autofill::ACCOUNT_CREATION_PASSWORD:
    case autofill::NEW_PASSWORD:
      return CredentialFieldType::kNewPassword;
    case autofill::CONFIRMATION_PASSWORD:
      return CredentialFieldType::kConfirmationPassword;
    default:
      return CredentialFieldType::kNone;
  }
}

FormPredictions ConvertToFormPredictions(const FormStructure& form_structure) {
  FormPredictions result;
  for (const auto& field : form_structure) {
    ServerFieldType server_type = field->server_type();
    if (IsCredentialRelatedPrediction(server_type)) {
      bool may_use_prefilled_placeholder = false;
      for (const auto& predictions : field->server_predictions()) {
        may_use_prefilled_placeholder |=
            predictions.may_use_prefilled_placeholder();
      }

      result[field->unique_renderer_id] = PasswordFieldPrediction{
          .type = server_type,
          .may_use_prefilled_placeholder = may_use_prefilled_placeholder};
    }
  }

  return result;
}

}  // namespace password_manager
