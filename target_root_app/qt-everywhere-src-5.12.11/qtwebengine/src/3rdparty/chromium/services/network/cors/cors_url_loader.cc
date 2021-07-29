// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/network/cors/cors_url_loader.h"

#include "base/stl_util.h"
#include "net/base/load_flags.h"
#include "services/network/cors/preflight_controller.h"
#include "services/network/loader_util.h"
#include "services/network/public/cpp/cors/cors.h"
#include "url/url_util.h"

namespace network {

namespace cors {

namespace {

bool CalculateCORSFlag(const ResourceRequest& request) {
  if (request.fetch_request_mode == mojom::FetchRequestMode::kNavigate ||
      request.fetch_request_mode == mojom::FetchRequestMode::kNoCORS) {
    return false;
  }
  // CORS needs a proper origin (including a unique opaque origin). If the
  // request doesn't have one, CORS should not work.
  DCHECK(request.request_initiator);
  url::Origin url_origin = url::Origin::Create(request.url);
  url::Origin security_origin(request.request_initiator.value());
  return !security_origin.IsSameOriginWith(url_origin);
}

base::Optional<std::string> GetHeaderString(
    const scoped_refptr<net::HttpResponseHeaders>& headers,
    const std::string& header_name) {
  std::string header_value;
  if (!headers->GetNormalizedHeader(header_name, &header_value))
    return base::nullopt;
  return header_value;
}

bool NeedsPreflight(const ResourceRequest& request) {
  if (!cors::IsCORSEnabledRequestMode(request.fetch_request_mode))
    return false;

  if (request.is_external_request)
    return true;

  if (request.fetch_request_mode ==
      mojom::FetchRequestMode::kCORSWithForcedPreflight) {
    return true;
  }

  if (request.cors_preflight_policy ==
      mojom::CORSPreflightPolicy::kPreventPreflight) {
    return false;
  }

  if (!IsCORSSafelistedMethod(request.method))
    return true;

  for (const auto& header : request.headers.GetHeaderVector()) {
    if (!IsCORSSafelistedHeader(header.key, header.value) &&
        !IsForbiddenHeader(header.key)) {
      return true;
    }
  }
  return false;
}

}  // namespace

CORSURLLoader::CORSURLLoader(
    mojom::URLLoaderRequest loader_request,
    int32_t routing_id,
    int32_t request_id,
    uint32_t options,
    DeleteCallback delete_callback,
    const ResourceRequest& resource_request,
    mojom::URLLoaderClientPtr client,
    const net::MutableNetworkTrafficAnnotationTag& traffic_annotation,
    mojom::URLLoaderFactory* network_loader_factory,
    const base::RepeatingCallback<void(int)>& request_finalizer)
    : binding_(this, std::move(loader_request)),
      routing_id_(routing_id),
      request_id_(request_id),
      options_(options),
      delete_callback_(std::move(delete_callback)),
      network_loader_factory_(network_loader_factory),
      network_client_binding_(this),
      request_(resource_request),
      forwarding_client_(std::move(client)),
      fetch_cors_flag_(CalculateCORSFlag(resource_request)),
      request_finalizer_(request_finalizer),
      traffic_annotation_(traffic_annotation),
      weak_factory_(this) {
  binding_.set_connection_error_handler(base::BindOnce(
      &CORSURLLoader::OnConnectionError, base::Unretained(this)));
  DCHECK(network_loader_factory_);
}

CORSURLLoader::~CORSURLLoader() = default;

void CORSURLLoader::Start() {
  if (fetch_cors_flag_ &&
      request_.fetch_request_mode == mojom::FetchRequestMode::kSameOrigin) {
    HandleComplete(URLLoaderCompletionStatus(
        CORSErrorStatus(mojom::CORSError::kDisallowedByMode)));
    return;
  }

  if (fetch_cors_flag_ &&
      cors::IsCORSEnabledRequestMode(request_.fetch_request_mode)) {
    // Username and password should be stripped in a CORS-enabled request.
    if (request_.url.has_username() || request_.url.has_password()) {
      GURL::Replacements replacements;
      replacements.SetUsernameStr("");
      replacements.SetPasswordStr("");
      request_.url = request_.url.ReplaceComponents(replacements);
    }
  }
  StartRequest();
}

void CORSURLLoader::FollowRedirect(
    const base::Optional<std::vector<std::string>>&
        to_be_removed_request_headers,
    const base::Optional<net::HttpRequestHeaders>& modified_request_headers) {
  DCHECK(!to_be_removed_request_headers.has_value());
  DCHECK(!modified_request_headers.has_value()) << "Redirect with modified "
                                                   "headers was not supported "
                                                   "yet. crbug.com/845683";
  if (!network_loader_ || !is_waiting_follow_redirect_call_) {
    HandleComplete(URLLoaderCompletionStatus(net::ERR_FAILED));
    return;
  }
  is_waiting_follow_redirect_call_ = false;

  // When the redirect mode is "error", the client is not expected to
  // call this function. Let's abort the request.
  if (request_.fetch_redirect_mode == mojom::FetchRedirectMode::kError) {
    HandleComplete(URLLoaderCompletionStatus(net::ERR_FAILED));
    return;
  }

  if (!AreRequestHeadersSafe(request_.headers)) {
    HandleComplete(URLLoaderCompletionStatus(net::ERR_INVALID_ARGUMENT));
    return;
  }

  request_.url = redirect_info_.new_url;
  request_.method = redirect_info_.new_method;
  request_.referrer = GURL(redirect_info_.new_referrer);
  request_.referrer_policy = redirect_info_.new_referrer_policy;
  const bool original_fetch_cors_flag = fetch_cors_flag_;
  fetch_cors_flag_ = fetch_cors_flag_ || CalculateCORSFlag(request_);

  // We cannot use FollowRedirect for a request with preflight (i.e., when both
  // |fetch_cors_flag_| and |NeedsPreflight(request_)| are true).
  //
  // Additionally, when |original_fetch_cors_flag| is false,
  // |fetch_cors_flag_| is true and |NeedsPreflight(request)| is false, the net/
  // implementation won't attach an "origin" header on redirect, as the original
  // request didn't have one. In such a case we need to re-issue a request
  // manually in order to attach the correct origin header.
  // For "no-cors" requests we rely on redirect logic in net/ (specifically
  // in net/url_request/redirect_util.cc).
  if ((original_fetch_cors_flag && !NeedsPreflight(request_)) ||
      !fetch_cors_flag_) {
    network_loader_->FollowRedirect(base::nullopt, base::nullopt);
    return;
  }
  DCHECK_NE(request_.fetch_request_mode, mojom::FetchRequestMode::kNoCORS);

  if (request_finalizer_)
    request_finalizer_.Run(request_id_);
  network_client_binding_.Unbind();

  if (request_.fetch_credentials_mode ==
      mojom::FetchCredentialsMode::kSameOrigin) {
    // If the credentials mode is "same-origin" and CORS flag is set, we must
    // not send credentials. As network::URLLoaderImpl doesn't understand
    // |fetch_credentials_mode|, we need to set the load flags here.
    request_.load_flags |= net::LOAD_DO_NOT_SAVE_COOKIES;
    request_.load_flags |= net::LOAD_DO_NOT_SEND_COOKIES;
    request_.load_flags |= net::LOAD_DO_NOT_SEND_AUTH_DATA;
  }
  StartRequest();
}

void CORSURLLoader::ProceedWithResponse() {
  NOTREACHED();
}

void CORSURLLoader::SetPriority(net::RequestPriority priority,
                                int32_t intra_priority_value) {
  if (network_loader_)
    network_loader_->SetPriority(priority, intra_priority_value);
}

void CORSURLLoader::PauseReadingBodyFromNet() {
  DCHECK(!is_waiting_follow_redirect_call_);
  if (network_loader_)
    network_loader_->PauseReadingBodyFromNet();
}

void CORSURLLoader::ResumeReadingBodyFromNet() {
  DCHECK(!is_waiting_follow_redirect_call_);
  if (network_loader_)
    network_loader_->ResumeReadingBodyFromNet();
}

void CORSURLLoader::OnReceiveResponse(
    const ResourceResponseHead& response_head) {
  DCHECK(network_loader_);
  DCHECK(forwarding_client_);
  DCHECK(!is_waiting_follow_redirect_call_);
  if (fetch_cors_flag_ &&
      IsCORSEnabledRequestMode(request_.fetch_request_mode)) {
    // TODO(toyoshim): Reflect --allow-file-access-from-files flag.
    const auto error_status = CheckAccess(
        request_.url, response_head.headers->response_code(),
        GetHeaderString(response_head.headers,
                        header_names::kAccessControlAllowOrigin),
        GetHeaderString(response_head.headers,
                        header_names::kAccessControlAllowCredentials),
        request_.fetch_credentials_mode,
        tainted_ ? url::Origin() : *request_.request_initiator);
    if (error_status) {
      HandleComplete(URLLoaderCompletionStatus(*error_status));
      return;
    }
  }
  forwarding_client_->OnReceiveResponse(response_head);
}

void CORSURLLoader::OnReceiveRedirect(
    const net::RedirectInfo& redirect_info,
    const ResourceResponseHead& response_head) {
  DCHECK(network_loader_);
  DCHECK(forwarding_client_);
  DCHECK(!is_waiting_follow_redirect_call_);

  // If |CORS flag| is set and a CORS check for |request| and |response| returns
  // failure, then return a network error.
  if (fetch_cors_flag_ &&
      IsCORSEnabledRequestMode(request_.fetch_request_mode)) {
    // TODO(toyoshim): Reflect --allow-file-access-from-files flag.
    const auto error_status = CheckAccess(
        request_.url, response_head.headers->response_code(),
        GetHeaderString(response_head.headers,
                        header_names::kAccessControlAllowOrigin),
        GetHeaderString(response_head.headers,
                        header_names::kAccessControlAllowCredentials),
        request_.fetch_credentials_mode,
        tainted_ ? url::Origin() : *request_.request_initiator);
    if (error_status) {
      HandleComplete(URLLoaderCompletionStatus(*error_status));
      return;
    }
  }

  // Because we initiate a new request on redirect in some cases, we cannot
  // rely on the redirect logic in the network stack. Hence we need to
  // implement some logic in
  // https://fetch.spec.whatwg.org/#http-redirect-fetch here.

  // If |request|’s redirect count is twenty, return a network error.
  // Increase |request|’s redirect count by one.
  if (redirect_count_++ == 20) {
    HandleComplete(URLLoaderCompletionStatus(net::ERR_TOO_MANY_REDIRECTS));
    return;
  }

  // TODO(yhirano): Implement the following:
  // If |request|’s mode is "cors", |actualResponse|’s location URL includes
  // credentials, and either |request|’s tainted origin flag is set or
  // |request|’s origin is not same origin with |actualResponse|’s location
  // URL’s origin, then return a network error.

  // TODO(yhirano): Implement the following:
  // If |CORS flag| is set and |actualResponse|’s location URL includes
  // credentials, then return a network error.

  // TODO(yhirano): Implement the following (Note: this is needed when upload
  // streaming is implemented):
  // If |actualResponse|’s status is not 303, |request|’s body is non-null, and
  // |request|’s body’s source is null, then return a network error.

  // If |actualResponse|’s location URL’s origin is not same origin with
  // |request|’s current url’s origin and |request|’s origin is not same origin
  // with |request|’s current url’s origin, then set |request|’s tainted origin
  // flag.
  if (request_.request_initiator &&
      (!url::Origin::Create(redirect_info.new_url)
            .IsSameOriginWith(url::Origin::Create(request_.url)) &&
       !request_.request_initiator->IsSameOriginWith(
           url::Origin::Create(request_.url)))) {
    tainted_ = true;
  }

  // TODO(yhirano): Implement the following:
  // If either |actualResponse|’s status is 301 or 302 and |request|’s method is
  // `POST`, or |actualResponse|’s status is 303, set |request|’s method to
  // `GET` and request’s body to null.

  // TODO(yhirano): Implement the following:
  // Invoke |set request’s referrer policy on redirect| on |request| and
  // |actualResponse|.

  redirect_info_ = redirect_info;

  is_waiting_follow_redirect_call_ = true;
  forwarding_client_->OnReceiveRedirect(redirect_info, response_head);
}

void CORSURLLoader::OnUploadProgress(int64_t current_position,
                                     int64_t total_size,
                                     OnUploadProgressCallback ack_callback) {
  DCHECK(network_loader_);
  DCHECK(forwarding_client_);
  DCHECK(!is_waiting_follow_redirect_call_);
  forwarding_client_->OnUploadProgress(current_position, total_size,
                                       std::move(ack_callback));
}

void CORSURLLoader::OnReceiveCachedMetadata(const std::vector<uint8_t>& data) {
  DCHECK(network_loader_);
  DCHECK(forwarding_client_);
  DCHECK(!is_waiting_follow_redirect_call_);
  forwarding_client_->OnReceiveCachedMetadata(data);
}

void CORSURLLoader::OnTransferSizeUpdated(int32_t transfer_size_diff) {
  DCHECK(network_loader_);
  DCHECK(forwarding_client_);
  DCHECK(!is_waiting_follow_redirect_call_);
  forwarding_client_->OnTransferSizeUpdated(transfer_size_diff);
}

void CORSURLLoader::OnStartLoadingResponseBody(
    mojo::ScopedDataPipeConsumerHandle body) {
  DCHECK(network_loader_);
  DCHECK(forwarding_client_);
  DCHECK(!is_waiting_follow_redirect_call_);
  forwarding_client_->OnStartLoadingResponseBody(std::move(body));
}

void CORSURLLoader::OnComplete(const URLLoaderCompletionStatus& status) {
  DCHECK(network_loader_);
  DCHECK(forwarding_client_);
  DCHECK(!is_waiting_follow_redirect_call_);
  HandleComplete(status);
}

void CORSURLLoader::StartRequest() {
  // If the CORS flag is set, |httpRequest|’s method is neither `GET` nor
  // `HEAD`, or |httpRequest|’s mode is "websocket", then append
  // `Origin`/the result of serializing a request origin with |httpRequest|, to
  // |httpRequest|’s header list.
  if (fetch_cors_flag_ ||
      (request_.method != "GET" && request_.method != "HEAD")) {
    if (request_.request_initiator) {
      request_.headers.SetHeader(
          net::HttpRequestHeaders::kOrigin,
          (tainted_ ? url::Origin() : *request_.request_initiator).Serialize());
    }
  }

  if (request_.fetch_request_mode == mojom::FetchRequestMode::kSameOrigin) {
    DCHECK(request_.request_initiator);
    if (!request_.request_initiator->IsSameOriginWith(
            url::Origin::Create(request_.url))) {
      HandleComplete(URLLoaderCompletionStatus(
          CORSErrorStatus(mojom::CORSError::kDisallowedByMode)));
      return;
    }
  }

  // Note that even when |NeedsPreflight(request_)| holds we don't make a
  // preflight request when |fetch_cors_flag_| is false (e.g., when the origin
  // of the url is equal to the origin of the request.
  if (!fetch_cors_flag_ || !NeedsPreflight(request_)) {
    StartNetworkRequest(net::OK, base::nullopt);
    return;
  }

  base::OnceCallback<void()> preflight_finalizer;
  if (request_finalizer_)
    preflight_finalizer = base::BindOnce(request_finalizer_, request_id_);

  PreflightController::GetDefaultController()->PerformPreflightCheck(
      base::BindOnce(&CORSURLLoader::StartNetworkRequest,
                     weak_factory_.GetWeakPtr()),
      request_id_, request_, tainted_,
      net::NetworkTrafficAnnotationTag(traffic_annotation_),
      network_loader_factory_, std::move(preflight_finalizer));
}

void CORSURLLoader::StartNetworkRequest(
    int error_code,
    base::Optional<CORSErrorStatus> status) {
  if (error_code != net::OK) {
    HandleComplete(status ? URLLoaderCompletionStatus(*status)
                          : URLLoaderCompletionStatus(error_code));
    return;
  }

  DCHECK(!status);
  mojom::URLLoaderClientPtr network_client;
  network_client_binding_.Bind(mojo::MakeRequest(&network_client));
  // Binding |this| as an unretained pointer is safe because
  // |network_client_binding_| shares this object's lifetime.
  network_client_binding_.set_connection_error_handler(base::BindOnce(
      &CORSURLLoader::OnConnectionError, base::Unretained(this)));
  network_loader_factory_->CreateLoaderAndStart(
      mojo::MakeRequest(&network_loader_), routing_id_, request_id_, options_,
      request_, std::move(network_client), traffic_annotation_);
}

void CORSURLLoader::HandleComplete(const URLLoaderCompletionStatus& status) {
  forwarding_client_->OnComplete(status);

  // Close pipes to ignore possible subsequent callback invocations.
  network_client_binding_.Close();

  forwarding_client_.reset();
  network_loader_.reset();

  std::move(delete_callback_).Run(this);
  // |this| is deleted here.
}

void CORSURLLoader::OnConnectionError() {
  HandleComplete(URLLoaderCompletionStatus(net::ERR_FAILED));
}

}  // namespace cors

}  // namespace network
