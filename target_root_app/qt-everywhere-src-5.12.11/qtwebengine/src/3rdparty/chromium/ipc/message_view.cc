// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ipc/message_view.h"

namespace IPC {

MessageView::MessageView() = default;

MessageView::MessageView(
    base::span<const uint8_t> bytes,
    base::Optional<std::vector<mojo::native::SerializedHandlePtr>> handles)
    : bytes_(bytes), handles_(std::move(handles)) {}

MessageView::MessageView(MessageView&&) = default;

MessageView::~MessageView() = default;

MessageView& MessageView::operator=(MessageView&&) = default;

}  // namespace IPC
