// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_PAYMENTS_TEST_PAYMENTS_CLIENT_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_PAYMENTS_TEST_PAYMENTS_CLIENT_H_

#include <string>
#include <vector>

#include "components/autofill/core/browser/payments/payments_client.h"

namespace network {
class SharedURLLoaderFactory;
}  // namespace network

namespace autofill {
namespace payments {

class TestPaymentsClient : public payments::PaymentsClient {
 public:
  TestPaymentsClient(
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_,
      PrefService* pref_service,
      identity::IdentityManager* identity_manager,
      payments::PaymentsClientUnmaskDelegate* unmask_delegate,
      payments::PaymentsClientSaveDelegate* save_delegate);

  ~TestPaymentsClient() override;

  void GetUploadDetails(const std::vector<AutofillProfile>& addresses,
                        const int detected_values,
                        const std::string& pan_first_six,
                        const std::vector<const char*>& active_experiments,
                        const std::string& app_locale) override;

  void UploadCard(const payments::PaymentsClient::UploadRequestDetails&
                      request_details) override;

  void SetSaveDelegate(
      payments::PaymentsClientSaveDelegate* save_delegate) override;

  void SetServerIdForCardUpload(std::string);

  int detected_values_in_upload_details() const { return detected_values_; }
  const std::vector<AutofillProfile>& addresses_in_upload_details() const {
    return upload_details_addresses_;
  }
  std::string pan_first_six_in_upload_details() const { return pan_first_six_; }
  const std::vector<const char*>& active_experiments_in_request() const {
    return active_experiments_;
  }

 private:
  payments::PaymentsClientSaveDelegate* save_delegate_;
  std::string server_id_;
  std::vector<AutofillProfile> upload_details_addresses_;
  int detected_values_;
  std::string pan_first_six_;
  std::vector<const char*> active_experiments_;

  DISALLOW_COPY_AND_ASSIGN(TestPaymentsClient);
};

}  // namespace payments
}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_PAYMENTS_TEST_PAYMENTS_CLIENT_H_
