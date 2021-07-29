// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/autofill_download_manager.h"

#include <tuple>
#include <utility>

#include "base/base64url.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "base/numerics/safe_conversions.h"
#include "base/rand_util.h"
#include "base/strings/strcat.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_task_runner_handle.h"
#include "components/autofill/core/browser/autofill_driver.h"
#include "components/autofill/core/browser/autofill_metrics.h"
#include "components/autofill/core/browser/form_structure.h"
#include "components/autofill/core/browser/proto/server.pb.h"
#include "components/autofill/core/common/autofill_features.h"
#include "components/autofill/core/common/autofill_pref_names.h"
#include "components/autofill/core/common/autofill_switches.h"
#include "components/data_use_measurement/core/data_use_user_data.h"
#include "components/variations/net/variations_http_headers.h"
#include "net/base/load_flags.h"
#include "net/http/http_request_headers.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_status_code.h"
#include "net/http/http_util.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace autofill {

namespace {

const size_t kMaxQueryGetSize = 1400;  // 1.25KB
const size_t kMaxFormCacheSize = 16;
const size_t kMaxFieldsPerQueryRequest = 100;

const net::BackoffEntry::Policy kAutofillBackoffPolicy = {
    // Number of initial errors (in sequence) to ignore before applying
    // exponential back-off rules.
    0,

    // Initial delay for exponential back-off in ms.
    1000,  // 1 second.

    // Factor by which the waiting time will be multiplied.
    2,

    // Fuzzing percentage. ex: 10% will spread requests randomly
    // between 90%-100% of the calculated time.
    0.33,  // 33%.

    // Maximum amount of time we are willing to delay our request in ms.
    30 * 1000,  // 30 seconds.

    // Time to keep an entry from being discarded even when it
    // has no significant state, -1 to never discard.
    -1,

    // Don't use initial delay unless the last request was an error.
    false,
};

const char kDefaultAutofillServerURL[] =
    "https://clients1.google.com/tbproxy/af/";

// Returns the base URL for the autofill server.
GURL GetAutofillServerURL() {
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  if (command_line.HasSwitch(switches::kAutofillServerURL)) {
    GURL url(command_line.GetSwitchValueASCII(switches::kAutofillServerURL));
    if (url.is_valid())
      return url;

    LOG(ERROR) << "Invalid URL given for --" << switches::kAutofillServerURL
               << ". Using default value.";
  }

  GURL default_url(kDefaultAutofillServerURL);
  DCHECK(default_url.is_valid());
  return default_url;
}

// Helper to log the HTTP |response_code| received for |request_type| to UMA.
void LogHttpResponseCode(AutofillDownloadManager::RequestType request_type,
                         int response_code) {
  const char* name = nullptr;
  switch (request_type) {
    case AutofillDownloadManager::REQUEST_QUERY:
      name = "Autofill.Query.HttpResponseCode";
      break;
    case AutofillDownloadManager::REQUEST_UPLOAD:
      name = "Autofill.Upload.HttpResponseCode";
      break;
    default:
      NOTREACHED();
      name = "Autofill.Unknown.HttpResponseCode";
  }

  if (response_code < 100 || response_code > 599)
    response_code = 0;

  // An expanded version of UMA_HISTOGRAM_ENUMERATION that supports using
  // a different name with each invocation.
  base::HistogramBase* histogram = base::LinearHistogram::FactoryGet(
      name, 1, 599, 600, base::HistogramBase::kUmaTargetedHistogramFlag);
  histogram->Add(response_code);
}

// Helper to log, to UMA, the |num_bytes| sent for a failing instance of
// |request_type|.
void LogFailingPayloadSize(AutofillDownloadManager::RequestType request_type,
                           size_t num_bytes) {
  const char* name = nullptr;
  switch (request_type) {
    case AutofillDownloadManager::REQUEST_QUERY:
      name = "Autofill.Query.FailingPayloadSize";
      break;
    case AutofillDownloadManager::REQUEST_UPLOAD:
      name = "Autofill.Upload.FailingPayloadSize";
      break;
    default:
      NOTREACHED();
      name = "Autofill.Unknown.FailingPayloadSize";
  }

  // An expanded version of UMA_HISTOGRAM_COUNTS_100000 that supports using
  // a different name with each invocation.
  base::HistogramBase* histogram = base::Histogram::FactoryGet(
      name, 1, 100000, 50, base::HistogramBase::kUmaTargetedHistogramFlag);
  histogram->Add(num_bytes);
}

net::NetworkTrafficAnnotationTag GetNetworkTrafficAnnotation(
    const autofill::AutofillDownloadManager::RequestType& request_type) {
  if (request_type == autofill::AutofillDownloadManager::REQUEST_QUERY) {
    return net::DefineNetworkTrafficAnnotation("autofill_query", R"(
        semantics {
          sender: "Autofill"
          description:
            "Chromium can automatically fill in web forms. If the feature is "
            "enabled, Chromium will send a non-identifying description of the "
            "form to Google's servers, which will respond with the type of "
            "data required by each of the form's fields, if known. I.e., if a "
            "field expects to receive a name, phone number, street address, "
            "etc."
          trigger: "User encounters a web form."
          data:
            "Hashed descriptions of the form and its fields. User data is not "
            "sent."
          destination: GOOGLE_OWNED_SERVICE
        }
        policy {
          cookies_allowed: NO
          setting:
            "You can enable or disable this feature via 'Enable autofill to "
            "fill out web forms in a single click.' in Chromium's settings "
            "under 'Passwords and forms'. The feature is enabled by default."
          chrome_policy {
            AutoFillEnabled {
                policy_options {mode: MANDATORY}
                AutoFillEnabled: false
            }
          }
        })");
  }

  DCHECK_EQ(request_type, autofill::AutofillDownloadManager::REQUEST_UPLOAD);
  return net::DefineNetworkTrafficAnnotation("autofill_upload", R"(
      semantics {
        sender: "Autofill"
        description:
          "Chromium relies on crowd-sourced field type classifications to "
          "help it automatically fill in web forms. If the feature is "
          "enabled, Chromium will send a non-identifying description of the "
          "form to Google's servers along with the type of data Chromium "
          "observed being given to the form. I.e., if you entered your first "
          "name into a form field, Chromium will 'vote' for that form field "
          "being a first name field."
        trigger: "User submits a web form."
        data:
          "Hashed descriptions of the form and its fields along with type of "
          "data given to each field, if recognized from the user's "
          "profile(s). User data is not sent."
        destination: GOOGLE_OWNED_SERVICE
      }
      policy {
        cookies_allowed: NO
        setting:
          "You can enable or disable this feature via 'Enable autofill to "
          "fill out web forms in a single click.' in Chromium's settings "
          "under 'Passwords and forms'. The feature is enabled by default."
        chrome_policy {
          AutoFillEnabled {
              policy_options {mode: MANDATORY}
              AutoFillEnabled: false
          }
        }
      })");
}

size_t CountActiveFieldsInForms(const std::vector<FormStructure*>& forms) {
  size_t active_field_count = 0;
  for (const auto* form : forms)
    active_field_count += form->active_field_count();
  return active_field_count;
}

const char* RequestTypeToString(AutofillDownloadManager::RequestType type) {
  switch (type) {
    case AutofillDownloadManager::REQUEST_QUERY:
      return "query";
    case AutofillDownloadManager::REQUEST_UPLOAD:
      return "upload";
  }
  NOTREACHED();
  return "";
}

std::ostream& operator<<(std::ostream& out,
                         const autofill::AutofillQueryContents& query) {
  out << "client_version: " << query.client_version();
  for (const auto& form : query.form()) {
    out << "\nForm\n signature: " << form.signature();
    for (const auto& field : form.field()) {
      out << "\n Field\n  signature: " << field.signature();
      if (!field.name().empty())
        out << "\n  name: " << field.name();
      if (!field.type().empty())
        out << "\n  type: " << field.type();
    }
  }
  return out;
}

std::ostream& operator<<(std::ostream& out,
                         const autofill::AutofillUploadContents& upload) {
  out << "client_version: " << upload.client_version() << "\n";
  out << "form_signature: " << upload.form_signature() << "\n";
  out << "data_present: " << upload.data_present() << "\n";
  out << "submission: " << upload.submission() << "\n";
  if (!upload.action_signature())
    out << "action_signature: " << upload.action_signature() << "\n";
  if (!upload.login_form_signature())
    out << "login_form_signature: " << upload.login_form_signature() << "\n";
  if (!upload.form_name().empty())
    out << "form_name: " << upload.form_name() << "\n";

  for (const auto& field : upload.field()) {
    out << "\n Field"
      << "\n signature: " << field.signature()
      << "\n autofill_type: " << field.autofill_type();
    if (!field.name().empty())
      out << "\n name: " << field.name();
    if (!field.autocomplete().empty())
      out << "\n autocomplete: " << field.autocomplete();
    if (!field.type().empty())
      out << "\n type: " << field.type();
    if (field.generation_type())
      out << "\n generation_type: " << field.generation_type();
  }
  return out;
}

}  // namespace

struct AutofillDownloadManager::FormRequestData {
  std::vector<std::string> form_signatures;
  RequestType request_type;
  std::string payload;
};

AutofillDownloadManager::AutofillDownloadManager(AutofillDriver* driver,
                                                 Observer* observer)
    : driver_(driver),
      observer_(observer),
      autofill_server_url_(GetAutofillServerURL()),
      max_form_cache_size_(kMaxFormCacheSize),
      loader_backoff_(&kAutofillBackoffPolicy),
      weak_factory_(this) {
  DCHECK(observer_);
}

AutofillDownloadManager::~AutofillDownloadManager() = default;

bool AutofillDownloadManager::StartQueryRequest(
    const std::vector<FormStructure*>& forms) {
  // Do not send the request if it contains more fields than the server can
  // accept.
  if (CountActiveFieldsInForms(forms) > kMaxFieldsPerQueryRequest)
    return false;

  AutofillQueryContents query;
  FormRequestData request_data;
  if (!FormStructure::EncodeQueryRequest(forms, &request_data.form_signatures,
                                         &query)) {
    return false;
  }

  std::string payload;
  if (!query.SerializeToString(&payload))
    return false;

  request_data.request_type = AutofillDownloadManager::REQUEST_QUERY;
  request_data.payload = std::move(payload);
  AutofillMetrics::LogServerQueryMetric(AutofillMetrics::QUERY_SENT);

  std::string query_data;
  if (CheckCacheForQueryRequest(request_data.form_signatures, &query_data)) {
    DVLOG(1) << "AutofillDownloadManager: query request has been retrieved "
             << "from the cache, form signatures: "
             << GetCombinedSignature(request_data.form_signatures);
    observer_->OnLoadedServerPredictions(std::move(query_data),
                                         request_data.form_signatures);
    return true;
  }

  DVLOG(1) << "Sending Autofill Query Request:\n" << query;

  return StartRequest(std::move(request_data));
}

bool AutofillDownloadManager::StartUploadRequest(
    const FormStructure& form,
    bool form_was_autofilled,
    const ServerFieldTypeSet& available_field_types,
    const std::string& login_form_signature,
    bool observed_submission) {
  AutofillUploadContents upload;
  if (!form.EncodeUploadRequest(available_field_types, form_was_autofilled,
                                login_form_signature, observed_submission,
                                &upload))
    return false;

  std::string payload;
  if (!upload.SerializeToString(&payload))
    return false;

  if (form.upload_required() == UPLOAD_NOT_REQUIRED) {
    DVLOG(1) << "AutofillDownloadManager: Upload request is ignored.";
    // If we ever need notification that upload was skipped, add it here.
    return false;
  }

  FormRequestData request_data;
  request_data.form_signatures.push_back(form.FormSignatureAsStr());
  request_data.request_type = AutofillDownloadManager::REQUEST_UPLOAD;
  request_data.payload = std::move(payload);

  DVLOG(1) << "Sending Autofill Upload Request:\n" << upload;

  return StartRequest(std::move(request_data));
}

std::tuple<GURL, std::string> AutofillDownloadManager::GetRequestURLAndMethod(
    const FormRequestData& request_data) const {
  std::string method("POST");
  std::string query_str;

  if (request_data.request_type == AutofillDownloadManager::REQUEST_QUERY) {
    if (request_data.payload.length() <= kMaxQueryGetSize &&
        base::FeatureList::IsEnabled(features::kAutofillCacheQueryResponses)) {
      method = "GET";
      std::string base64_payload;
      base::Base64UrlEncode(request_data.payload,
                            base::Base64UrlEncodePolicy::INCLUDE_PADDING,
                            &base64_payload);
      base::StrAppend(&query_str, {"q=", base64_payload});
    }
    UMA_HISTOGRAM_BOOLEAN("Autofill.Query.Method", (method == "GET") ? 0 : 1);
  }

  GURL::Replacements replacements;
  replacements.SetQueryStr(query_str);

  GURL url = autofill_server_url_
                 .Resolve(RequestTypeToString(request_data.request_type))
                 .ReplaceComponents(replacements);
  return std::make_tuple(std::move(url), std::move(method));
}

bool AutofillDownloadManager::StartRequest(FormRequestData request_data) {
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory =
      driver_->GetURLLoaderFactory();
  DCHECK(url_loader_factory);

  // Get the URL and method to use for this request.
  std::string method;
  GURL request_url;
  std::tie(request_url, method) = GetRequestURLAndMethod(request_data);

  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = request_url;
  resource_request->load_flags =
      net::LOAD_DO_NOT_SEND_COOKIES | net::LOAD_DO_NOT_SAVE_COOKIES;
  resource_request->method = method;

  // Add Chrome experiment state to the request headers.
  variations::AppendVariationHeadersUnknownSignedIn(
      request_url,
      driver_->IsIncognito() ? variations::InIncognito::kYes
                             : variations::InIncognito::kNo,
      &resource_request->headers);

  auto simple_loader = network::SimpleURLLoader::Create(
      std::move(resource_request),
      GetNetworkTrafficAnnotation(request_data.request_type));
  if (method == "POST")
    simple_loader->AttachStringForUpload(request_data.payload, "text/proto");

  // Transfer ownership of the loader into url_loaders_. Temporarily hang
  // onto the raw pointer to use it as a key and to kick off the request;
  // transferring ownership (std::move) invalidates the |simple_loader|
  // variable.
  auto* raw_simple_loader = simple_loader.get();
  url_loaders_.push_back(std::move(simple_loader));
  raw_simple_loader->DownloadToStringOfUnboundedSizeUntilCrashAndDie(
      url_loader_factory.get(),
      base::BindOnce(&AutofillDownloadManager::OnSimpleLoaderComplete,
                     base::Unretained(this), std::move(--url_loaders_.end()),
                     std::move(request_data)));
  return true;
}

void AutofillDownloadManager::CacheQueryRequest(
    const std::vector<std::string>& forms_in_query,
    const std::string& query_data) {
  std::string signature = GetCombinedSignature(forms_in_query);
  for (auto it = cached_forms_.begin(); it != cached_forms_.end(); ++it) {
    if (it->first == signature) {
      // We hit the cache, move to the first position and return.
      std::pair<std::string, std::string> data = *it;
      cached_forms_.erase(it);
      cached_forms_.push_front(data);
      return;
    }
  }
  std::pair<std::string, std::string> data;
  data.first = signature;
  data.second = query_data;
  cached_forms_.push_front(data);
  while (cached_forms_.size() > max_form_cache_size_)
    cached_forms_.pop_back();
}

bool AutofillDownloadManager::CheckCacheForQueryRequest(
    const std::vector<std::string>& forms_in_query,
    std::string* query_data) const {
  std::string signature = GetCombinedSignature(forms_in_query);
  for (const auto& it : cached_forms_) {
    if (it.first == signature) {
      // We hit the cache, fill the data and return.
      *query_data = it.second;
      return true;
    }
  }
  return false;
}

std::string AutofillDownloadManager::GetCombinedSignature(
    const std::vector<std::string>& forms_in_query) const {
  size_t total_size = forms_in_query.size();
  for (size_t i = 0; i < forms_in_query.size(); ++i)
    total_size += forms_in_query[i].length();
  std::string signature;

  signature.reserve(total_size);

  for (size_t i = 0; i < forms_in_query.size(); ++i) {
    if (i)
      signature.append(",");
    signature.append(forms_in_query[i]);
  }
  return signature;
}

void AutofillDownloadManager::OnSimpleLoaderComplete(
    std::list<std::unique_ptr<network::SimpleURLLoader>>::iterator it,
    FormRequestData request_data,
    std::unique_ptr<std::string> response_body) {
  // Move the loader out of the active loaders list.
  std::unique_ptr<network::SimpleURLLoader> simple_loader = std::move(*it);
  url_loaders_.erase(it);

  CHECK(request_data.form_signatures.size());
  int response_code = -1;
  if (simple_loader->ResponseInfo() && simple_loader->ResponseInfo()->headers) {
    response_code = simple_loader->ResponseInfo()->headers->response_code();
  }

  const bool success = !!response_body;
  loader_backoff_.InformOfRequest(success);

  LogHttpResponseCode(request_data.request_type, response_code);

  if (!success) {
    DVLOG(1) << "AutofillDownloadManager: "
             << RequestTypeToString(request_data.request_type)
             << " request has failed with response " << response_code;

    observer_->OnServerRequestError(request_data.form_signatures[0],
                                    request_data.request_type, response_code);

    LogFailingPayloadSize(request_data.request_type,
                          request_data.payload.length());

    // If the failure was a client error don't retry.
    if (response_code >= 400 && response_code <= 499)
      return;

    // Reschedule with the appropriate delay, ignoring return value because
    // payload is already well formed.
    base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(
            base::IgnoreResult(&AutofillDownloadManager::StartRequest),
            weak_factory_.GetWeakPtr(), std::move(request_data)),
        loader_backoff_.GetTimeUntilRelease());
    return;
  }

  if (request_data.request_type == AutofillDownloadManager::REQUEST_QUERY) {
    CacheQueryRequest(request_data.form_signatures, *response_body);
    UMA_HISTOGRAM_BOOLEAN("Autofill.Query.WasInCache",
                          simple_loader->LoadedFromCache());
    observer_->OnLoadedServerPredictions(std::move(*response_body),
                                         request_data.form_signatures);
    return;
  }

  DCHECK_EQ(request_data.request_type, AutofillDownloadManager::REQUEST_UPLOAD);
  DVLOG(1) << "AutofillDownloadManager: upload request has succeeded.";
  observer_->OnUploadedPossibleFieldTypes();
}

}  // namespace autofill
