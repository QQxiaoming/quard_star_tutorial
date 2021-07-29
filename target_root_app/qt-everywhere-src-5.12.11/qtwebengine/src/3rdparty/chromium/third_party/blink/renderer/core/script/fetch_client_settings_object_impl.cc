// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/script/fetch_client_settings_object_impl.h"

#include "third_party/blink/renderer/core/execution_context/execution_context.h"

namespace blink {

FetchClientSettingsObjectImpl::FetchClientSettingsObjectImpl(
    ExecutionContext& execution_context)
    : execution_context_(execution_context) {
  DCHECK(execution_context_->IsContextThread());
}

const KURL& FetchClientSettingsObjectImpl::BaseURL() const {
  DCHECK(execution_context_->IsContextThread());
  return execution_context_->BaseURL();
}

const SecurityOrigin* FetchClientSettingsObjectImpl::GetSecurityOrigin() const {
  DCHECK(execution_context_->IsContextThread());
  return execution_context_->GetSecurityOrigin();
}

ReferrerPolicy FetchClientSettingsObjectImpl::GetReferrerPolicy() const {
  DCHECK(execution_context_->IsContextThread());
  return execution_context_->GetReferrerPolicy();
}

const String FetchClientSettingsObjectImpl::GetOutgoingReferrer() const {
  DCHECK(execution_context_->IsContextThread());
  return execution_context_->OutgoingReferrer();
}

HttpsState FetchClientSettingsObjectImpl::GetHttpsState() const {
  DCHECK(execution_context_->IsContextThread());
  return execution_context_->GetHttpsState();
}

void FetchClientSettingsObjectImpl::Trace(Visitor* visitor) {
  visitor->Trace(execution_context_);
  FetchClientSettingsObject::Trace(visitor);
}

}  // namespace blink
