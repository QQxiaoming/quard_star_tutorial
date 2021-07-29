// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/hid/hid_connection.h"

#include <algorithm>

#include "base/memory/ref_counted_memory.h"
#include "base/stl_util.h"
#include "components/device_event_log/device_event_log.h"
#include "services/device/public/cpp/hid/hid_usage_and_page.h"
#include "services/device/public/mojom/hid.mojom.h"

namespace device {

namespace {

// Functor used to filter collections by report ID.
struct CollectionHasReportId {
  explicit CollectionHasReportId(uint8_t report_id) : report_id_(report_id) {}

  bool operator()(const mojom::HidCollectionInfoPtr& info) const {
    if (info->report_ids.size() == 0 ||
        report_id_ == HidConnection::kNullReportId)
      return false;

    if (report_id_ == HidConnection::kAnyReportId)
      return true;

    return base::ContainsValue(info->report_ids, report_id_);
  }

 private:
  const uint8_t report_id_;
};

// Functor returning true if collection has a protected usage.
struct CollectionIsProtected {
  bool operator()(const mojom::HidCollectionInfoPtr& info) const {
    return IsProtected(*info->usage);
  }
};

const mojom::HidCollectionInfo* FindCollectionByReportId(
    const std::vector<mojom::HidCollectionInfoPtr>& collections,
    uint8_t report_id) {
  auto collection_iter = std::find_if(collections.begin(), collections.end(),
                                      CollectionHasReportId(report_id));
  if (collection_iter != collections.end()) {
    DCHECK(collection_iter->get());
    return collection_iter->get();
  }

  return nullptr;
}

bool HasProtectedCollection(
    const std::vector<mojom::HidCollectionInfoPtr>& collections) {
  return std::find_if(collections.begin(), collections.end(),
                      CollectionIsProtected()) != collections.end();
}

}  // namespace

struct HidConnection::PendingHidReport {
  PendingHidReport() = default;
  PendingHidReport(const PendingHidReport& other) = default;
  ~PendingHidReport() = default;

  scoped_refptr<base::RefCountedBytes> buffer;
  size_t size;
};

struct HidConnection::PendingHidRead {
  PendingHidRead() = default;
  PendingHidRead(PendingHidRead&& other) = default;
  ~PendingHidRead() = default;

  HidConnection::ReadCallback callback;
};

HidConnection::HidConnection(scoped_refptr<HidDeviceInfo> device_info)
    : device_info_(device_info), closed_(false) {
  has_protected_collection_ =
      HasProtectedCollection(device_info->collections());
}

HidConnection::~HidConnection() {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(closed_);
}

void HidConnection::Close() {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(!closed_);

  PlatformClose();
  closed_ = true;
}

void HidConnection::Read(ReadCallback callback) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (device_info_->max_input_report_size() == 0) {
    HID_LOG(USER) << "This device does not support input reports.";
    std::move(callback).Run(false, NULL, 0);
    return;
  }

  PendingHidRead pending_read;
  pending_read.callback = std::move(callback);
  pending_reads_.push(std::move(pending_read));
  ProcessReadQueue();
}

void HidConnection::Write(scoped_refptr<base::RefCountedBytes> buffer,
                          WriteCallback callback) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (device_info_->max_output_report_size() == 0) {
    HID_LOG(USER) << "This device does not support output reports.";
    std::move(callback).Run(false);
    return;
  }
  if (buffer->size() > device_info_->max_output_report_size() + 1) {
    HID_LOG(USER) << "Output report buffer too long (" << buffer->size()
                  << " > " << (device_info_->max_output_report_size() + 1)
                  << ").";
    std::move(callback).Run(false);
    return;
  }
  DCHECK_GE(buffer->size(), 1u);
  uint8_t report_id = buffer->data()[0];
  if (device_info_->has_report_id() != (report_id != 0)) {
    HID_LOG(USER) << "Invalid output report ID.";
    std::move(callback).Run(false);
    return;
  }
  if (IsReportIdProtected(report_id)) {
    HID_LOG(USER) << "Attempt to set a protected output report.";
    std::move(callback).Run(false);
    return;
  }

  PlatformWrite(buffer, std::move(callback));
}

void HidConnection::GetFeatureReport(uint8_t report_id, ReadCallback callback) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (device_info_->max_feature_report_size() == 0) {
    HID_LOG(USER) << "This device does not support feature reports.";
    std::move(callback).Run(false, NULL, 0);
    return;
  }
  if (device_info_->has_report_id() != (report_id != 0)) {
    HID_LOG(USER) << "Invalid feature report ID.";
    std::move(callback).Run(false, NULL, 0);
    return;
  }
  if (IsReportIdProtected(report_id)) {
    HID_LOG(USER) << "Attempt to get a protected feature report.";
    std::move(callback).Run(false, NULL, 0);
    return;
  }

  PlatformGetFeatureReport(report_id, std::move(callback));
}

void HidConnection::SendFeatureReport(
    scoped_refptr<base::RefCountedBytes> buffer,
    WriteCallback callback) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (device_info_->max_feature_report_size() == 0) {
    HID_LOG(USER) << "This device does not support feature reports.";
    std::move(callback).Run(false);
    return;
  }
  DCHECK_GE(buffer->size(), 1u);
  uint8_t report_id = buffer->data()[0];
  if (device_info_->has_report_id() != (report_id != 0)) {
    HID_LOG(USER) << "Invalid feature report ID.";
    std::move(callback).Run(false);
    return;
  }
  if (IsReportIdProtected(report_id)) {
    HID_LOG(USER) << "Attempt to set a protected feature report.";
    std::move(callback).Run(false);
    return;
  }

  PlatformSendFeatureReport(buffer, std::move(callback));
}

bool HidConnection::IsReportIdProtected(uint8_t report_id) {
  auto* collection_info =
      FindCollectionByReportId(device_info_->collections(), report_id);
  if (collection_info) {
    return IsProtected(*collection_info->usage);
  }

  return has_protected_collection();
}

void HidConnection::ProcessInputReport(
    scoped_refptr<base::RefCountedBytes> buffer,
    size_t size) {
  DCHECK(thread_checker().CalledOnValidThread());
  DCHECK_GE(size, 1u);

  uint8_t report_id = buffer->data()[0];
  if (IsReportIdProtected(report_id))
    return;

  PendingHidReport report;
  report.buffer = buffer;
  report.size = size;
  pending_reports_.push(report);
  ProcessReadQueue();
}

void HidConnection::ProcessReadQueue() {
  DCHECK(thread_checker().CalledOnValidThread());

  // Hold a reference to |this| to prevent a callback from freeing this object
  // during the loop.
  scoped_refptr<HidConnection> self(this);
  while (pending_reads_.size() && pending_reports_.size()) {
    PendingHidRead read = std::move(pending_reads_.front());
    PendingHidReport report = std::move(pending_reports_.front());

    pending_reads_.pop();
    pending_reports_.pop();
    std::move(read.callback).Run(true, std::move(report.buffer), report.size);
  }
}

}  // namespace device
