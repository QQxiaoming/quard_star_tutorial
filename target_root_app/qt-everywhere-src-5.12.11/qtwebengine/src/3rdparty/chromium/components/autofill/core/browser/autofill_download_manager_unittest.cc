// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/autofill_download_manager.h"

#include <stddef.h>

#include <list>
#include <memory>
#include <utility>
#include <vector>

#include "base/format_macros.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task_scheduler/post_task.h"
#include "base/test/metrics/histogram_tester.h"
#include "base/test/scoped_command_line.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/scoped_task_environment.h"
#include "base/test/test_timeouts.h"
#include "base/threading/thread_task_runner_handle.h"
#include "components/autofill/core/browser/autofill_field.h"
#include "components/autofill/core/browser/autofill_metrics.h"
#include "components/autofill/core/browser/autofill_type.h"
#include "components/autofill/core/browser/form_structure.h"
#include "components/autofill/core/browser/test_autofill_driver.h"
#include "components/autofill/core/common/autofill_features.h"
#include "components/autofill/core/common/autofill_switches.h"
#include "components/autofill/core/common/form_data.h"
#include "net/http/http_status_code.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "net/test/embedded_test_server/http_request.h"
#include "net/test/embedded_test_server/http_response.h"
#include "net/url_request/url_request_status.h"
#include "net/url_request/url_request_test_util.h"
#include "services/network/public/cpp/weak_wrapper_shared_url_loader_factory.h"
#include "services/network/test/test_shared_url_loader_factory.h"
#include "services/network/test/test_url_loader_factory.h"
#include "services/network/test/test_utils.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using base::ASCIIToUTF16;
using net::test_server::EmbeddedTestServer;
using net::test_server::BasicHttpResponse;
using net::test_server::HttpRequest;
using net::test_server::HttpResponse;
namespace autofill {

namespace {

const int METHOD_GET = 0;
const int METHOD_POST = 1;
const int CACHE_MISS = 0;
const int CACHE_HIT = 1;

std::vector<FormStructure*> ToRawPointerVector(
    const std::vector<std::unique_ptr<FormStructure>>& list) {
  std::vector<FormStructure*> result;
  for (const auto& item : list)
    result.push_back(item.get());
  return result;
}

}  // namespace

// This tests AutofillDownloadManager. AutofillDownloadManagerTest implements
// AutofillDownloadManager::Observer and creates an instance of
// AutofillDownloadManager. Then it records responses to different initiated
// requests, which are verified later. To mock network requests
// TestURLLoaderFactory is used, which creates SimpleURLLoaders that do not
// go over the wire, but allow calling back HTTP responses directly.
// The responses in test are out of order and verify: successful query request,
// successful upload request, failed upload request.
class AutofillDownloadManagerTest : public AutofillDownloadManager::Observer,
                                    public testing::Test {
 public:
  AutofillDownloadManagerTest()
      : test_shared_loader_factory_(
            base::MakeRefCounted<network::WeakWrapperSharedURLLoaderFactory>(
                &test_url_loader_factory_)),
        download_manager_(&driver_, this) {
    driver_.SetSharedURLLoaderFactory(test_shared_loader_factory_);
  }

  void LimitCache(size_t cache_size) {
    download_manager_.set_max_form_cache_size(cache_size);
  }

  // AutofillDownloadManager::Observer implementation.
  void OnLoadedServerPredictions(
      std::string response_xml,
      const std::vector<std::string>& form_signatures) override {
    ResponseData response;
    response.response = std::move(response_xml);
    response.type_of_response = QUERY_SUCCESSFULL;
    responses_.push_back(response);
  }

  void OnUploadedPossibleFieldTypes() override {
    ResponseData response;
    response.type_of_response = UPLOAD_SUCCESSFULL;
    responses_.push_back(response);
  }

  void OnServerRequestError(const std::string& form_signature,
                            AutofillDownloadManager::RequestType request_type,
                            int http_error) override {
    ResponseData response;
    response.signature = form_signature;
    response.error = http_error;
    response.type_of_response =
        request_type == AutofillDownloadManager::REQUEST_QUERY ?
            REQUEST_QUERY_FAILED : REQUEST_UPLOAD_FAILED;
    responses_.push_back(response);
  }

  enum ResponseType {
    QUERY_SUCCESSFULL,
    UPLOAD_SUCCESSFULL,
    REQUEST_QUERY_FAILED,
    REQUEST_UPLOAD_FAILED,
  };

  struct ResponseData {
    ResponseType type_of_response;
    int error;
    std::string signature;
    std::string response;

    ResponseData() : type_of_response(REQUEST_QUERY_FAILED), error(0) {}
  };

  network::TestURLLoaderFactory::PendingRequest* GetPendingRequest(
      size_t index = 0) {
    if (index >= test_url_loader_factory_.pending_requests()->size())
      return nullptr;
    auto* request = &(*test_url_loader_factory_.pending_requests())[index];
    DCHECK(request);
    return request;
  }

  base::MessageLoop message_loop_;
  std::list<ResponseData> responses_;
  scoped_refptr<network::SharedURLLoaderFactory> test_shared_loader_factory_;
  network::TestURLLoaderFactory test_url_loader_factory_;
  TestAutofillDriver driver_;
  AutofillDownloadManager download_manager_;
};

TEST_F(AutofillDownloadManagerTest, QueryAndUploadTest) {
  base::test::ScopedFeatureList fl;
  fl.InitAndEnableFeature(features::kAutofillCacheQueryResponses);

  FormData form;

  FormFieldData field;
  field.label = ASCIIToUTF16("username");
  field.name = ASCIIToUTF16("username");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("First Name");
  field.name = ASCIIToUTF16("firstname");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("Last Name");
  field.name = ASCIIToUTF16("lastname");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("email");
  field.name = ASCIIToUTF16("email");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("email2");
  field.name = ASCIIToUTF16("email2");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("password");
  field.name = ASCIIToUTF16("password");
  field.form_control_type = "password";
  form.fields.push_back(field);

  field.label = base::string16();
  field.name = ASCIIToUTF16("Submit");
  field.form_control_type = "submit";
  form.fields.push_back(field);

  std::vector<std::unique_ptr<FormStructure>> form_structures;
  form_structures.push_back(std::make_unique<FormStructure>(form));

  form.fields.clear();

  field.label = ASCIIToUTF16("address");
  field.name = ASCIIToUTF16("address");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("address2");
  field.name = ASCIIToUTF16("address2");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("city");
  field.name = ASCIIToUTF16("city");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = base::string16();
  field.name = ASCIIToUTF16("Submit");
  field.form_control_type = "submit";
  form.fields.push_back(field);

  form_structures.push_back(std::make_unique<FormStructure>(form));

  form.fields.clear();

  field.label = ASCIIToUTF16("username");
  field.name = ASCIIToUTF16("username");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("password");
  field.name = ASCIIToUTF16("password");
  field.form_control_type = "password";
  form.fields.push_back(field);

  field.label = base::string16();
  field.name = ASCIIToUTF16("Submit");
  field.form_control_type = "submit";
  form.fields.push_back(field);

  form_structures.push_back(std::make_unique<FormStructure>(form));

  // Request with id 0.
  base::HistogramTester histogram;
  EXPECT_TRUE(
      download_manager_.StartQueryRequest(ToRawPointerVector(form_structures)));
  histogram.ExpectUniqueSample("Autofill.ServerQueryResponse",
                               AutofillMetrics::QUERY_SENT, 1);
  histogram.ExpectUniqueSample("Autofill.Query.Method", METHOD_GET, 1);

  // Request with id 1.
  EXPECT_TRUE(download_manager_.StartUploadRequest(
      *(form_structures[0]), true, ServerFieldTypeSet(), std::string(), true));
  // Request with id 2.
  EXPECT_TRUE(download_manager_.StartUploadRequest(
      *(form_structures[1]), false, ServerFieldTypeSet(), std::string(), true));
  // Request with id 3. Upload request with a non-empty additional password form
  // signature.
  EXPECT_TRUE(download_manager_.StartUploadRequest(
      *(form_structures[2]), false, ServerFieldTypeSet(), "42", true));

  const char* responses[] = {
      "<autofillqueryresponse>"
      "<field autofilltype=\"0\" />"
      "<field autofilltype=\"3\" />"
      "<field autofilltype=\"5\" />"
      "<field autofilltype=\"9\" />"
      "<field autofilltype=\"0\" />"
      "<field autofilltype=\"30\" />"
      "<field autofilltype=\"31\" />"
      "<field autofilltype=\"33\" />"
      "</autofillqueryresponse>",
      "", "<html></html>",
  };

  // Return them out of sequence.

  // Request 1: Successful upload.
  auto* request = GetPendingRequest(1);
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request, responses[1]);
  histogram.ExpectBucketCount("Autofill.Upload.HttpResponseCode", net::HTTP_OK,
                              1);

  // Request 2: Unsuccessful upload.
  request = GetPendingRequest(2);
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request, network::CreateResourceResponseHead(net::HTTP_NOT_FOUND),
      responses[2], network::URLLoaderCompletionStatus(net::OK));
  histogram.ExpectBucketCount("Autofill.Upload.HttpResponseCode",
                              net::HTTP_NOT_FOUND, 1);

  // Request 0: Successful query.
  request = GetPendingRequest(0);
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request, responses[0]);
  EXPECT_EQ(3U, responses_.size());
  histogram.ExpectBucketCount("Autofill.Query.WasInCache", CACHE_MISS, 1);
  histogram.ExpectBucketCount("Autofill.Query.HttpResponseCode", net::HTTP_OK,
                              1);

  // Check Request 1.
  EXPECT_EQ(AutofillDownloadManagerTest::UPLOAD_SUCCESSFULL,
            responses_.front().type_of_response);
  EXPECT_EQ(0, responses_.front().error);
  EXPECT_EQ(std::string(), responses_.front().signature);
  // Expected response on non-query request is an empty string.
  EXPECT_EQ(std::string(), responses_.front().response);
  responses_.pop_front();

  // Check Request 2.
  EXPECT_EQ(AutofillDownloadManagerTest::REQUEST_UPLOAD_FAILED,
            responses_.front().type_of_response);
  EXPECT_EQ(net::HTTP_NOT_FOUND, responses_.front().error);
  EXPECT_EQ(form_structures[1]->FormSignatureAsStr(),
            responses_.front().signature);
  // Expected response on non-query request is an empty string.
  EXPECT_EQ(std::string(), responses_.front().response);
  responses_.pop_front();

  // Check Request 0.
  EXPECT_EQ(responses_.front().type_of_response,
            AutofillDownloadManagerTest::QUERY_SUCCESSFULL);
  EXPECT_EQ(0, responses_.front().error);
  EXPECT_EQ(std::string(), responses_.front().signature);
  EXPECT_EQ(responses[0], responses_.front().response);
  responses_.pop_front();

  // Modify form structures to miss the cache.
  field.label = ASCIIToUTF16("Address line 2");
  field.name = ASCIIToUTF16("address2");
  field.form_control_type = "text";
  form.fields.push_back(field);
  form_structures.push_back(std::make_unique<FormStructure>(form));

  // Request with id 4, not successful.
  EXPECT_TRUE(
      download_manager_.StartQueryRequest(ToRawPointerVector(form_structures)));
  request = GetPendingRequest(4);
  histogram.ExpectUniqueSample("Autofill.ServerQueryResponse",
                               AutofillMetrics::QUERY_SENT, 2);
  histogram.ExpectUniqueSample("Autofill.Query.Method", METHOD_GET, 2);
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request,
      network::CreateResourceResponseHead(net::HTTP_INTERNAL_SERVER_ERROR),
      responses[0], network::URLLoaderCompletionStatus(net::OK));
  histogram.ExpectBucketCount("Autofill.Query.HttpResponseCode",
                              net::HTTP_INTERNAL_SERVER_ERROR, 1);

  // Check Request 4.
  EXPECT_EQ(AutofillDownloadManagerTest::REQUEST_QUERY_FAILED,
            responses_.front().type_of_response);
  EXPECT_EQ(net::HTTP_INTERNAL_SERVER_ERROR, responses_.front().error);
  // Expected response on non-query request is an empty string.
  EXPECT_EQ(std::string(), responses_.front().response);
  responses_.pop_front();

  // Request with id 5. Let's pretend we hit the cache.
  EXPECT_TRUE(
      download_manager_.StartQueryRequest(ToRawPointerVector(form_structures)));
  histogram.ExpectBucketCount("Autofill.ServerQueryResponse",
                              AutofillMetrics::QUERY_SENT, 3);
  histogram.ExpectBucketCount("Autofill.Query.Method", METHOD_GET, 3);
  request = GetPendingRequest(5);

  network::URLLoaderCompletionStatus status(net::OK);
  status.exists_in_cache = true;
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request, network::CreateResourceResponseHead(net::HTTP_OK), responses[0],
      status);

  // Check Request 5.
  EXPECT_EQ(responses_.front().type_of_response,
            AutofillDownloadManagerTest::QUERY_SUCCESSFULL);
  responses_.pop_front();
  histogram.ExpectBucketCount("Autofill.Query.WasInCache", CACHE_HIT, 1);

  // Test query with caching disabled.
  base::test::ScopedFeatureList fl2;
  fl2.InitAndDisableFeature(features::kAutofillCacheQueryResponses);

  // Don't hit the in-mem cache.
  field.label = ASCIIToUTF16("Address line 3");
  field.name = ASCIIToUTF16("address3");
  field.form_control_type = "text";
  form.fields.push_back(field);
  form_structures.push_back(std::make_unique<FormStructure>(form));

  // Request with id 6
  EXPECT_TRUE(
      download_manager_.StartQueryRequest(ToRawPointerVector(form_structures)));
  histogram.ExpectBucketCount("Autofill.ServerQueryResponse",
                              AutofillMetrics::QUERY_SENT, 4);
  histogram.ExpectBucketCount("Autofill.Query.Method", METHOD_POST, 1);
  request = GetPendingRequest(6);
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request, responses[0]);
  histogram.ExpectBucketCount("Autofill.Query.WasInCache", CACHE_MISS, 2);
}

TEST_F(AutofillDownloadManagerTest, BackoffLogic_Query) {
  FormData form;
  FormFieldData field;
  field.label = ASCIIToUTF16("address");
  field.name = ASCIIToUTF16("address");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("address2");
  field.name = ASCIIToUTF16("address2");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("city");
  field.name = ASCIIToUTF16("city");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = base::string16();
  field.name = ASCIIToUTF16("Submit");
  field.form_control_type = "submit";
  form.fields.push_back(field);

  std::vector<std::unique_ptr<FormStructure>> form_structures;
  form_structures.push_back(std::make_unique<FormStructure>(form));

  // Request with id 0.
  base::HistogramTester histogram;
  EXPECT_TRUE(
      download_manager_.StartQueryRequest(ToRawPointerVector(form_structures)));
  histogram.ExpectUniqueSample("Autofill.ServerQueryResponse",
                               AutofillMetrics::QUERY_SENT, 1);

  auto* request = GetPendingRequest(0);

  // Request error incurs a retry after 1 second.
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request,
      network::CreateResourceResponseHead(net::HTTP_INTERNAL_SERVER_ERROR), "",
      network::URLLoaderCompletionStatus(net::OK));

  EXPECT_EQ(1U, responses_.size());
  EXPECT_LT(download_manager_.loader_backoff_.GetTimeUntilRelease(),
            base::TimeDelta::FromMilliseconds(1100));
  base::RunLoop run_loop;
  base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
      FROM_HERE, run_loop.QuitClosure(),
      base::TimeDelta::FromMilliseconds(1100));
  run_loop.Run();

  // Get the retried request.
  request = GetPendingRequest(1);

  // Next error incurs a retry after 2 seconds.
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request,
      network::CreateResourceResponseHead(net::HTTP_REQUEST_ENTITY_TOO_LARGE),
      "<html></html>", network::URLLoaderCompletionStatus(net::OK));

  EXPECT_EQ(2U, responses_.size());
  EXPECT_LT(download_manager_.loader_backoff_.GetTimeUntilRelease(),
            base::TimeDelta::FromMilliseconds(2100));

  // There should not be an additional retry.
  ASSERT_EQ(test_url_loader_factory_.NumPending(), 0);
  histogram.ExpectBucketCount("Autofill.Query.HttpResponseCode",
                              net::HTTP_REQUEST_ENTITY_TOO_LARGE, 1);
  auto buckets = histogram.GetAllSamples("Autofill.Query.FailingPayloadSize");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_EQ(2, buckets[0].count);
}

TEST_F(AutofillDownloadManagerTest, BackoffLogic_Upload) {
  FormData form;
  FormFieldData field;
  field.label = ASCIIToUTF16("address");
  field.name = ASCIIToUTF16("address");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("address2");
  field.name = ASCIIToUTF16("address2");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("city");
  field.name = ASCIIToUTF16("city");
  field.form_control_type = "text";
  form.fields.push_back(field);

  field.label = base::string16();
  field.name = ASCIIToUTF16("Submit");
  field.form_control_type = "submit";
  form.fields.push_back(field);

  auto form_structure = std::make_unique<FormStructure>(form);

  // Request with id 0.
  EXPECT_TRUE(download_manager_.StartUploadRequest(
      *form_structure, true, ServerFieldTypeSet(), std::string(), true));

  auto* request = GetPendingRequest(0);

  // Error incurs a retry after 1 second.
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request,
      network::CreateResourceResponseHead(net::HTTP_INTERNAL_SERVER_ERROR), "",
      network::URLLoaderCompletionStatus(net::OK));
  EXPECT_EQ(1U, responses_.size());
  EXPECT_LT(download_manager_.loader_backoff_.GetTimeUntilRelease(),
            base::TimeDelta::FromMilliseconds(1100));
  base::RunLoop run_loop;
  base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
      FROM_HERE, run_loop.QuitClosure(),
      base::TimeDelta::FromMilliseconds(1100));
  run_loop.Run();

  // Check that it was a failure.
  EXPECT_EQ(AutofillDownloadManagerTest::REQUEST_UPLOAD_FAILED,
            responses_.front().type_of_response);
  EXPECT_EQ(net::HTTP_INTERNAL_SERVER_ERROR, responses_.front().error);
  EXPECT_EQ(form_structure->FormSignatureAsStr(), responses_.front().signature);
  // Expected response on non-query request is an empty string.
  EXPECT_EQ(std::string(), responses_.front().response);
  responses_.pop_front();

  // Get the retried request, and make it successful.
  request = GetPendingRequest(1);
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request, "");

  // Check success of response.
  EXPECT_EQ(AutofillDownloadManagerTest::UPLOAD_SUCCESSFULL,
            responses_.front().type_of_response);
  EXPECT_EQ(0, responses_.front().error);
  EXPECT_EQ(std::string(), responses_.front().signature);
  // Expected response on non-query request is an empty string.
  EXPECT_EQ(std::string(), responses_.front().response);
  responses_.pop_front();

  // Validate no retry on sending a bad request.
  base::HistogramTester histogram;
  EXPECT_TRUE(download_manager_.StartUploadRequest(
      *form_structure, true, ServerFieldTypeSet(), std::string(), true));
  request = GetPendingRequest(2);
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request,
      network::CreateResourceResponseHead(net::HTTP_REQUEST_ENTITY_TOO_LARGE),
      "", network::URLLoaderCompletionStatus(net::OK));
  ASSERT_EQ(test_url_loader_factory_.NumPending(), 0);
  histogram.ExpectBucketCount("Autofill.Upload.HttpResponseCode",
                              net::HTTP_REQUEST_ENTITY_TOO_LARGE, 1);
  auto buckets = histogram.GetAllSamples("Autofill.Upload.FailingPayloadSize");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_EQ(1, buckets[0].count);
}

TEST_F(AutofillDownloadManagerTest, QueryTooManyFieldsTest) {
  // Create a query that contains too many fields for the server.
  std::vector<FormData> forms(21);
  std::vector<std::unique_ptr<FormStructure>> form_structures;
  for (auto& form : forms) {
    for (size_t i = 0; i < 5; ++i) {
      FormFieldData field;
      field.label = base::IntToString16(i);
      field.name = base::IntToString16(i);
      field.form_control_type = "text";
      form.fields.push_back(field);
    }
    form_structures.push_back(std::make_unique<FormStructure>(form));
  }

  // Check whether the query is aborted.
  EXPECT_FALSE(
      download_manager_.StartQueryRequest(ToRawPointerVector(form_structures)));
}

TEST_F(AutofillDownloadManagerTest, QueryNotTooManyFieldsTest) {
  // Create a query that contains a lot of fields, but not too many for the
  // server.
  std::vector<FormData> forms(25);
  std::vector<std::unique_ptr<FormStructure>> form_structures;
  for (auto& form : forms) {
    for (size_t i = 0; i < 4; ++i) {
      FormFieldData field;
      field.label = base::IntToString16(i);
      field.name = base::IntToString16(i);
      field.form_control_type = "text";
      form.fields.push_back(field);
    }
    form_structures.push_back(std::make_unique<FormStructure>(form));
  }

  // Check that the query is not aborted.
  EXPECT_TRUE(
      download_manager_.StartQueryRequest(ToRawPointerVector(form_structures)));
}

TEST_F(AutofillDownloadManagerTest, CacheQueryTest) {
  FormData form;

  FormFieldData field;
  field.form_control_type = "text";

  field.label = ASCIIToUTF16("username");
  field.name = ASCIIToUTF16("username");
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("First Name");
  field.name = ASCIIToUTF16("firstname");
  form.fields.push_back(field);

  field.label = ASCIIToUTF16("Last Name");
  field.name = ASCIIToUTF16("lastname");
  form.fields.push_back(field);

  std::vector<std::unique_ptr<FormStructure>> form_structures0;
  form_structures0.push_back(std::make_unique<FormStructure>(form));

  // Add a slightly different form, which should result in a different request.
  field.label = ASCIIToUTF16("email");
  field.name = ASCIIToUTF16("email");
  form.fields.push_back(field);
  std::vector<std::unique_ptr<FormStructure>> form_structures1;
  form_structures1.push_back(std::make_unique<FormStructure>(form));

  // Add another slightly different form, which should also result in a
  // different request.
  field.label = ASCIIToUTF16("email2");
  field.name = ASCIIToUTF16("email2");
  form.fields.push_back(field);
  std::vector<std::unique_ptr<FormStructure>> form_structures2;
  form_structures2.push_back(std::make_unique<FormStructure>(form));

  // Limit cache to two forms.
  LimitCache(2);

  const char *responses[] = {
    "<autofillqueryresponse>"
      "<field autofilltype=\"0\" />"
      "<field autofilltype=\"3\" />"
      "<field autofilltype=\"5\" />"
    "</autofillqueryresponse>",
    "<autofillqueryresponse>"
      "<field autofilltype=\"0\" />"
      "<field autofilltype=\"3\" />"
      "<field autofilltype=\"5\" />"
      "<field autofilltype=\"9\" />"
    "</autofillqueryresponse>",
    "<autofillqueryresponse>"
      "<field autofilltype=\"0\" />"
      "<field autofilltype=\"3\" />"
      "<field autofilltype=\"5\" />"
      "<field autofilltype=\"9\" />"
      "<field autofilltype=\"0\" />"
    "</autofillqueryresponse>",
  };

  base::HistogramTester histogram;
  // Request with id 0.
  EXPECT_TRUE(download_manager_.StartQueryRequest(
      ToRawPointerVector(form_structures0)));
  histogram.ExpectUniqueSample("Autofill.ServerQueryResponse",
                               AutofillMetrics::QUERY_SENT, 1);

  // No responses yet
  EXPECT_EQ(0U, responses_.size());

  auto* request = GetPendingRequest(0);
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request, responses[0]);
  ASSERT_EQ(1U, responses_.size());
  EXPECT_EQ(responses[0], responses_.front().response);

  responses_.clear();

  // No actual request - should be a cache hit.
  EXPECT_TRUE(download_manager_.StartQueryRequest(
      ToRawPointerVector(form_structures0)));
  histogram.ExpectUniqueSample("Autofill.ServerQueryResponse",
                               AutofillMetrics::QUERY_SENT, 2);
  // Data is available immediately from cache - no over-the-wire trip.
  ASSERT_EQ(1U, responses_.size());
  EXPECT_EQ(responses[0], responses_.front().response);
  responses_.clear();

  // Request with id 1.
  EXPECT_TRUE(download_manager_.StartQueryRequest(
      ToRawPointerVector(form_structures1)));
  histogram.ExpectUniqueSample("Autofill.ServerQueryResponse",
                               AutofillMetrics::QUERY_SENT, 3);
  // No responses yet
  EXPECT_EQ(0U, responses_.size());

  request = GetPendingRequest(1);
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request, responses[1]);
  ASSERT_EQ(1U, responses_.size());
  EXPECT_EQ(responses[1], responses_.front().response);

  responses_.clear();

  // Request with id 2.
  EXPECT_TRUE(download_manager_.StartQueryRequest(
      ToRawPointerVector(form_structures2)));
  histogram.ExpectUniqueSample("Autofill.ServerQueryResponse",
                               AutofillMetrics::QUERY_SENT, 4);

  request = GetPendingRequest(2);
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request, responses[2]);
  ASSERT_EQ(1U, responses_.size());
  EXPECT_EQ(responses[2], responses_.front().response);

  responses_.clear();

  // No actual requests - should be a cache hit.
  EXPECT_TRUE(download_manager_.StartQueryRequest(
      ToRawPointerVector(form_structures1)));
  histogram.ExpectUniqueSample("Autofill.ServerQueryResponse",
                               AutofillMetrics::QUERY_SENT, 5);

  EXPECT_TRUE(download_manager_.StartQueryRequest(
      ToRawPointerVector(form_structures2)));
  histogram.ExpectUniqueSample("Autofill.ServerQueryResponse",
                               AutofillMetrics::QUERY_SENT, 6);

  ASSERT_EQ(2U, responses_.size());
  EXPECT_EQ(responses[1], responses_.front().response);
  EXPECT_EQ(responses[2], responses_.back().response);
  responses_.clear();

  // The first structure should have expired.
  // Request with id 3.
  EXPECT_TRUE(download_manager_.StartQueryRequest(
      ToRawPointerVector(form_structures0)));
  histogram.ExpectUniqueSample("Autofill.ServerQueryResponse",
                               AutofillMetrics::QUERY_SENT, 7);
  // No responses yet
  EXPECT_EQ(0U, responses_.size());

  request = GetPendingRequest(3);
  test_url_loader_factory_.SimulateResponseWithoutRemovingFromPendingList(
      request, responses[0]);
  ASSERT_EQ(1U, responses_.size());
  EXPECT_EQ(responses[0], responses_.front().response);
}

namespace {

class AutofillQueryTest : public AutofillDownloadManager::Observer,
                          public testing::Test {
 protected:
  void SetUp() override {
    scoped_feature_list_.InitAndEnableFeature(
        features::kAutofillCacheQueryResponses);

    // Setup the server.
    server_.RegisterRequestHandler(base::BindRepeating(
        &AutofillQueryTest::RequestHandler, base::Unretained(this)));
    ASSERT_TRUE(server_.Start());

    scoped_command_line_.GetProcessCommandLine()->AppendSwitchASCII(
        switches::kAutofillServerURL,
        server_.base_url().Resolve("/tbproxy/af/").spec().c_str());

    // Intialize the autofill driver.
    shared_url_loader_factory_ =
        base::MakeRefCounted<network::TestSharedURLLoaderFactory>();
    driver_ = std::make_unique<TestAutofillDriver>();
    driver_->SetSharedURLLoaderFactory(shared_url_loader_factory_);
  }

  void TearDown() override {
    if (server_.Started())
      ASSERT_TRUE(server_.ShutdownAndWaitUntilComplete());
  }

  // AutofillDownloadManager::Observer implementation.
  void OnLoadedServerPredictions(
      std::string /* response_xml */,
      const std::vector<std::string>& /*form_signatures */) override {
    ASSERT_TRUE(run_loop_);
    run_loop_->QuitWhenIdle();
  }

  std::unique_ptr<HttpResponse> RequestHandler(const HttpRequest& request) {
    GURL absolute_url = server_.GetURL(request.relative_url);
    ++call_count_;

    if (absolute_url.path() != "/tbproxy/af/query")
      return nullptr;

    AutofillQueryResponseContents proto;
    proto.add_field()->set_overall_type_prediction(NAME_FIRST);

    auto response = std::make_unique<BasicHttpResponse>();
    response->set_code(net::HTTP_OK);
    response->set_content(proto.SerializeAsString());
    response->set_content_type("text/proto");
    response->AddCustomHeader(
        "Cache-Control",
        base::StringPrintf(
            "max-age=%" PRId64,
            base::TimeDelta::FromMilliseconds(cache_expiration_in_milliseconds_)
                .InSeconds()));
    return response;
  }

  void SendQueryRequest(
      const std::vector<std::unique_ptr<FormStructure>>& form_structures) {
    ASSERT_TRUE(run_loop_ == nullptr);
    run_loop_ = std::make_unique<base::RunLoop>();

    AutofillDownloadManager download_manager(driver_.get(), this);
    ASSERT_TRUE(download_manager.StartQueryRequest(
        ToRawPointerVector(form_structures)));
    run_loop_->Run();
    run_loop_.reset();
  }

  base::test::ScopedTaskEnvironment scoped_task_environment_{
      base::test::ScopedTaskEnvironment::MainThreadType::IO};
  base::test::ScopedCommandLine scoped_command_line_;
  base::test::ScopedFeatureList scoped_feature_list_;
  EmbeddedTestServer server_;
  int cache_expiration_in_milliseconds_ = 100000;
  std::unique_ptr<base::RunLoop> run_loop_;
  size_t call_count_ = 0;
  scoped_refptr<network::TestSharedURLLoaderFactory> shared_url_loader_factory_;
  std::unique_ptr<TestAutofillDriver> driver_;
};

}  // namespace

TEST_F(AutofillQueryTest, CacheableResponse) {
  FormFieldData field;
  field.label = ASCIIToUTF16("First Name:");
  field.name = ASCIIToUTF16("firstname");
  field.form_control_type = "text";

  FormData form;
  form.fields.push_back(field);

  std::vector<std::unique_ptr<FormStructure>> form_structures;
  form_structures.push_back(std::make_unique<FormStructure>(form));

  // Query for the form. This should go to the embedded server.
  {
    SCOPED_TRACE("First Query");
    base::HistogramTester histogram;
    call_count_ = 0;
    ASSERT_NO_FATAL_FAILURE(SendQueryRequest(form_structures));
    EXPECT_EQ(1u, call_count_);
    histogram.ExpectBucketCount("Autofill.ServerQueryResponse",
                                AutofillMetrics::QUERY_SENT, 1);
    histogram.ExpectBucketCount("Autofill.Query.Method", METHOD_GET, 1);
    histogram.ExpectBucketCount("Autofill.Query.WasInCache", CACHE_MISS, 1);
  }

  // Query again the next day. This should go to the cache, since the max-age
  // for the cached response is 2 days.
  {
    SCOPED_TRACE("Second Query");
    base::HistogramTester histogram;
    call_count_ = 0;
    ASSERT_NO_FATAL_FAILURE(SendQueryRequest(form_structures));
    EXPECT_EQ(0u, call_count_);
    histogram.ExpectBucketCount("Autofill.ServerQueryResponse",
                                AutofillMetrics::QUERY_SENT, 1);
    histogram.ExpectBucketCount("Autofill.Query.Method", METHOD_GET, 1);
    histogram.ExpectBucketCount("Autofill.Query.WasInCache", CACHE_HIT, 1);
  }
}

TEST_F(AutofillQueryTest, ExpiredCacheInResponse) {
  FormFieldData field;
  field.label = ASCIIToUTF16("First Name:");
  field.name = ASCIIToUTF16("firstname");
  field.form_control_type = "text";

  FormData form;
  form.fields.push_back(field);

  std::vector<std::unique_ptr<FormStructure>> form_structures;
  form_structures.push_back(std::make_unique<FormStructure>(form));

  // Set the cache expiration interval to 0.
  cache_expiration_in_milliseconds_ = 0;

  // Query for the form. This should go to the embedded server.
  {
    SCOPED_TRACE("First Query");
    base::HistogramTester histogram;
    call_count_ = 0;
    ASSERT_NO_FATAL_FAILURE(SendQueryRequest(form_structures));
    EXPECT_EQ(1u, call_count_);
    histogram.ExpectBucketCount("Autofill.ServerQueryResponse",
                                AutofillMetrics::QUERY_SENT, 1);
    histogram.ExpectBucketCount("Autofill.Query.Method", METHOD_GET, 1);
    histogram.ExpectBucketCount("Autofill.Query.WasInCache", CACHE_MISS, 1);
  }

  // The cache entry had a max age of 0 ms, so delaying only a few milliseconds
  // ensures the cache expires and no request are served by cached content
  // (ie this should go to the embedded server).
  base::RunLoop run_loop;
  base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
      FROM_HERE, run_loop.QuitClosure(),
      base::TimeDelta::FromMilliseconds(100));
  run_loop.Run();

  {
    SCOPED_TRACE("Second Query");
    base::HistogramTester histogram;
    call_count_ = 0;
    ASSERT_NO_FATAL_FAILURE(SendQueryRequest(form_structures));
    EXPECT_EQ(1u, call_count_);
    histogram.ExpectBucketCount("Autofill.ServerQueryResponse",
                                AutofillMetrics::QUERY_SENT, 1);
    histogram.ExpectBucketCount("Autofill.Query.Method", METHOD_GET, 1);
    histogram.ExpectBucketCount("Autofill.Query.WasInCache", CACHE_MISS, 1);
  }
}

}  // namespace autofill
