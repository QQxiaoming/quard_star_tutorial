// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/bindings/lib/control_message_proxy.h"

#include <stddef.h>
#include <stdint.h>
#include <utility>

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/macros.h"
#include "base/run_loop.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "mojo/public/cpp/bindings/lib/serialization.h"
#include "mojo/public/cpp/bindings/lib/validation_util.h"
#include "mojo/public/cpp/bindings/message.h"
#include "mojo/public/interfaces/bindings/interface_control_messages.mojom.h"

namespace mojo {
namespace internal {

namespace {

bool ValidateControlResponse(Message* message) {
  ValidationContext validation_context(message->payload(),
                                       message->payload_num_bytes(), 0, 0,
                                       message, "ControlResponseValidator");
  if (!ValidateMessageIsResponse(message, &validation_context))
    return false;

  switch (message->header()->name) {
    case interface_control::kRunMessageId:
      return ValidateMessagePayload<
          interface_control::internal::RunResponseMessageParams_Data>(
          message, &validation_context);
  }
  return false;
}

using RunCallback =
    base::OnceCallback<void(interface_control::RunResponseMessageParamsPtr)>;

class RunResponseForwardToCallback : public MessageReceiver {
 public:
  explicit RunResponseForwardToCallback(RunCallback callback)
      : callback_(std::move(callback)) {}
  bool Accept(Message* message) override;

 private:
  RunCallback callback_;
  DISALLOW_COPY_AND_ASSIGN(RunResponseForwardToCallback);
};

bool RunResponseForwardToCallback::Accept(Message* message) {
  if (!ValidateControlResponse(message))
    return false;

  interface_control::internal::RunResponseMessageParams_Data* params =
      reinterpret_cast<
          interface_control::internal::RunResponseMessageParams_Data*>(
          message->mutable_payload());
  interface_control::RunResponseMessageParamsPtr params_ptr;
  SerializationContext context;
  Deserialize<interface_control::RunResponseMessageParamsDataView>(
      params, &params_ptr, &context);

  std::move(callback_).Run(std::move(params_ptr));
  return true;
}

void SendRunMessage(MessageReceiverWithResponder* receiver,
                    interface_control::RunInputPtr input_ptr,
                    RunCallback callback) {
  auto params_ptr = interface_control::RunMessageParams::New();
  params_ptr->input = std::move(input_ptr);
  Message message(interface_control::kRunMessageId,
                  Message::kFlagExpectsResponse, 0, 0, nullptr);
  SerializationContext context;
  interface_control::internal::RunMessageParams_Data::BufferWriter params;
  Serialize<interface_control::RunMessageParamsDataView>(
      params_ptr, message.payload_buffer(), &params, &context);
  std::unique_ptr<MessageReceiver> responder =
      std::make_unique<RunResponseForwardToCallback>(std::move(callback));
  ignore_result(receiver->AcceptWithResponder(&message, std::move(responder)));
}

Message ConstructRunOrClosePipeMessage(
    interface_control::RunOrClosePipeInputPtr input_ptr) {
  auto params_ptr = interface_control::RunOrClosePipeMessageParams::New();
  params_ptr->input = std::move(input_ptr);
  Message message(interface_control::kRunOrClosePipeMessageId, 0, 0, 0,
                  nullptr);
  SerializationContext context;
  interface_control::internal::RunOrClosePipeMessageParams_Data::BufferWriter
      params;
  Serialize<interface_control::RunOrClosePipeMessageParamsDataView>(
      params_ptr, message.payload_buffer(), &params, &context);
  return message;
}

void SendRunOrClosePipeMessage(
    MessageReceiverWithResponder* receiver,
    interface_control::RunOrClosePipeInputPtr input_ptr) {
  Message message(ConstructRunOrClosePipeMessage(std::move(input_ptr)));
  ignore_result(receiver->Accept(&message));
}

void RunVersionCallback(
    const base::Callback<void(uint32_t)>& callback,
    interface_control::RunResponseMessageParamsPtr run_response) {
  uint32_t version = 0u;
  if (run_response->output && run_response->output->is_query_version_result())
    version = run_response->output->get_query_version_result()->version;
  callback.Run(version);
}

void RunClosure(base::OnceClosure callback,
                interface_control::RunResponseMessageParamsPtr run_response) {
  std::move(callback).Run();
}

}  // namespace

ControlMessageProxy::ControlMessageProxy(MessageReceiverWithResponder* receiver)
    : receiver_(receiver) {
}

ControlMessageProxy::~ControlMessageProxy() = default;

void ControlMessageProxy::QueryVersion(
    const base::Callback<void(uint32_t)>& callback) {
  auto input_ptr = interface_control::RunInput::New();
  input_ptr->set_query_version(interface_control::QueryVersion::New());
  SendRunMessage(receiver_, std::move(input_ptr),
                 base::BindOnce(&RunVersionCallback, callback));
}

void ControlMessageProxy::RequireVersion(uint32_t version) {
  auto require_version = interface_control::RequireVersion::New();
  require_version->version = version;
  auto input_ptr = interface_control::RunOrClosePipeInput::New();
  input_ptr->set_require_version(std::move(require_version));
  SendRunOrClosePipeMessage(receiver_, std::move(input_ptr));
}

void ControlMessageProxy::FlushForTesting() {
  base::RunLoop run_loop(base::RunLoop::Type::kNestableTasksAllowed);
  FlushAsyncForTesting(run_loop.QuitClosure());
  run_loop.Run();
}

void ControlMessageProxy::FlushAsyncForTesting(base::OnceClosure callback) {
  if (encountered_error_) {
    base::SequencedTaskRunnerHandle::Get()->PostTask(FROM_HERE,
                                                     std::move(callback));
    return;
  }

  auto input_ptr = interface_control::RunInput::New();
  input_ptr->set_flush_for_testing(interface_control::FlushForTesting::New());
  DCHECK(!pending_flush_callback_);
  pending_flush_callback_ = std::move(callback);
  SendRunMessage(
      receiver_, std::move(input_ptr),
      base::BindOnce(
          &RunClosure,
          base::BindOnce(&ControlMessageProxy::RunFlushForTestingClosure,
                         base::Unretained(this))));
}

void ControlMessageProxy::RunFlushForTestingClosure() {
  DCHECK(!pending_flush_callback_.is_null());
  std::move(pending_flush_callback_).Run();
}

void ControlMessageProxy::OnConnectionError() {
  encountered_error_ = true;
  if (!pending_flush_callback_.is_null())
    RunFlushForTestingClosure();
}

}  // namespace internal
}  // namespace mojo
