// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/payments/core/payment_manifest_downloader.h"

#include <unordered_map>
#include <utility>

#include "base/logging.h"
#include "base/optional.h"
#include "base/stl_util.h"
#include "base/strings/string_split.h"
#include "components/data_use_measurement/core/data_use_user_data.h"
#include "components/link_header_util/link_header_util.h"
#include "net/base/load_flags.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "net/base/url_util.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_status_code.h"
#include "net/http/http_util.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "url/url_constants.h"

namespace payments {
namespace {

GURL ParseResponseHeader(const GURL& original_url,
                         const GURL& final_url,
                         scoped_refptr<net::HttpResponseHeaders> headers) {
  if (!headers) {
    LOG(ERROR) << "No HTTP headers found on " << final_url
               << " for payment method manifest.";
    return GURL();
  }

  int response_code = headers->response_code();
  if (response_code != net::HTTP_OK && response_code != net::HTTP_NO_CONTENT) {
    LOG(ERROR) << "Unable to make a HEAD request to " << final_url
               << " for payment method manifest.";
    return GURL();
  }

  std::string link_header;
  headers->GetNormalizedHeader("link", &link_header);
  if (link_header.empty()) {
    LOG(ERROR) << "No HTTP Link headers found on " << final_url
               << " for payment method manifest.";
    return GURL();
  }

  for (const auto& value : link_header_util::SplitLinkHeader(link_header)) {
    std::string payment_method_manifest_url;
    std::unordered_map<std::string, base::Optional<std::string>> params;
    if (!link_header_util::ParseLinkHeaderValue(
            value.first, value.second, &payment_method_manifest_url, &params)) {
      continue;
    }

    auto rel = params.find("rel");
    if (rel == params.end())
      continue;

    std::vector<std::string> rel_parts =
        base::SplitString(rel->second.value_or(""), HTTP_LWS,
                          base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
    if (base::ContainsValue(rel_parts, "payment-method-manifest"))
      return original_url.Resolve(payment_method_manifest_url);
  }

  LOG(ERROR) << "No rel=\"payment-method-manifest\" HTTP Link headers found on "
             << final_url << " for payment method manifest.";
  return GURL();
}

bool IsValidManifestUrl(const GURL& url) {
  return url.is_valid() &&
         (url.SchemeIs(url::kHttpsScheme) ||
          (url.SchemeIs(url::kHttpScheme) && net::IsLocalhost(url)));
}

GURL ParseRedirectUrl(const net::RedirectInfo& redirect_info) {
  // Do not follow net::HTTP_MULTIPLE_CHOICES, net::HTTP_NOT_MODIFIED and
  // net::HTTP_USE_PROXY redirects.
  if (redirect_info.status_code != net::HTTP_MOVED_PERMANENTLY &&
      redirect_info.status_code != net::HTTP_FOUND &&
      redirect_info.status_code != net::HTTP_SEE_OTHER &&
      redirect_info.status_code != net::HTTP_TEMPORARY_REDIRECT &&
      redirect_info.status_code != net::HTTP_PERMANENT_REDIRECT) {
    return GURL();
  }

  if (!IsValidManifestUrl(redirect_info.new_url))
    return GURL();

  return redirect_info.new_url;
}

std::string ParseResponseContent(
    const GURL& final_url,
    const std::string& response_body,
    scoped_refptr<net::HttpResponseHeaders> headers) {
  if (!headers || headers->response_code() != net::HTTP_OK) {
    LOG(ERROR) << "Unable to download " << final_url
               << " for payment manifests.";
    return std::string();
  }

  return response_body;
}

}  // namespace

PaymentManifestDownloader::PaymentManifestDownloader(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
    : url_loader_factory_(std::move(url_loader_factory)) {}

PaymentManifestDownloader::~PaymentManifestDownloader() {}

void PaymentManifestDownloader::DownloadPaymentMethodManifest(
    const GURL& url,
    PaymentManifestDownloadCallback callback) {
  DCHECK(IsValidManifestUrl(url));
  // Restrict number of redirects for efficiency and breaking circle.
  InitiateDownload(url, "HEAD",
                   /*allowed_number_of_redirects=*/3, std::move(callback));
}

void PaymentManifestDownloader::DownloadWebAppManifest(
    const GURL& url,
    PaymentManifestDownloadCallback callback) {
  DCHECK(IsValidManifestUrl(url));
  InitiateDownload(url, "GET", /*allowed_number_of_redirects=*/0,
                   std::move(callback));
}

PaymentManifestDownloader::Download::Download() {}

PaymentManifestDownloader::Download::~Download() {}

void PaymentManifestDownloader::OnURLLoaderRedirect(
    network::SimpleURLLoader* url_loader,
    const net::RedirectInfo& redirect_info,
    const network::ResourceResponseHead& response_head,
    std::vector<std::string>* to_be_removed_headers) {
  auto download_it = downloads_.find(url_loader);
  DCHECK(download_it != downloads_.end());

  std::unique_ptr<Download> download = std::move(download_it->second);
  downloads_.erase(download_it);

  // Manually follow some type of redirects.
  if (download->allowed_number_of_redirects > 0) {
    DCHECK(download->method == "HEAD");
    GURL redirect_url = ParseRedirectUrl(redirect_info);
    if (!redirect_url.is_empty() &&
        // Do not allow cross site redirects.
        net::registry_controlled_domains::SameDomainOrHost(
            download->original_url, redirect_url,
            net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES)) {
      InitiateDownload(redirect_url, "HEAD",
                       --download->allowed_number_of_redirects,
                       std::move(download->callback));
      return;
    }
  }

  std::move(download->callback).Run(std::string());
}

void PaymentManifestDownloader::OnURLLoaderComplete(
    network::SimpleURLLoader* url_loader,
    std::unique_ptr<std::string> response_body) {
  scoped_refptr<net::HttpResponseHeaders> headers;
  if (url_loader->ResponseInfo())
    headers = url_loader->ResponseInfo()->headers;

  std::string response_body_str;
  if (response_body.get())
    response_body_str = std::move(*response_body);

  OnURLLoaderCompleteInternal(url_loader, url_loader->GetFinalURL(),
                              response_body_str, headers,
                              url_loader->NetError());
}

void PaymentManifestDownloader::OnURLLoaderCompleteInternal(
    network::SimpleURLLoader* url_loader,
    const GURL& final_url,
    const std::string& response_body,
    scoped_refptr<net::HttpResponseHeaders> headers,
    int net_error) {
  auto download_it = downloads_.find(url_loader);
  DCHECK(download_it != downloads_.end());

  std::unique_ptr<Download> download = std::move(download_it->second);
  downloads_.erase(download_it);

  if (download->method == "HEAD") {
    GURL url = ParseResponseHeader(download->original_url, final_url, headers);
    if (IsValidManifestUrl(url)) {
      InitiateDownload(url, "GET",
                       /*allowed_number_of_redirects=*/0,
                       std::move(download->callback));
    } else {
      // If the URL is empty, then ParseResponseHeader() has already printed
      // an explanation.
      if (!url.is_empty())
        LOG(ERROR) << url << " is not a valid payment method manifest URL.";
      std::move(download->callback).Run(std::string());
    }

    return;
  }

  std::move(download->callback)
      .Run(ParseResponseContent(final_url, response_body, headers));
}

network::SimpleURLLoader* PaymentManifestDownloader::GetLoaderForTesting() {
  CHECK_EQ(downloads_.size(), 1u);
  return downloads_.begin()->second->loader.get();
}

GURL PaymentManifestDownloader::GetLoaderOriginalURLForTesting() {
  CHECK_EQ(downloads_.size(), 1u);
  return downloads_.begin()->second->original_url;
}

void PaymentManifestDownloader::InitiateDownload(
    const GURL& url,
    const std::string& method,
    int allowed_number_of_redirects,
    PaymentManifestDownloadCallback callback) {
  DCHECK(IsValidManifestUrl(url));

  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("payment_manifest_downloader", R"(
        semantics {
          sender: "Web Payments"
          description:
            "Chromium downloads manifest files for web payments API to help "
            "users make secure and convenient payments on the web."
          trigger:
            "A user that has a payment app visits a website that uses the web "
            "payments API."
          data: "None."
          destination: WEBSITE
        }
        policy {
          cookies_allowed: NO
          setting:
            "This feature cannot be disabled in settings. Users can uninstall/"
            "disable all payment apps to stop this feature."
          policy_exception_justification: "Not implemented."
        })");
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = url;
  resource_request->method = method;
  resource_request->load_flags =
      net::LOAD_DO_NOT_SEND_COOKIES | net::LOAD_DO_NOT_SAVE_COOKIES;
  std::unique_ptr<network::SimpleURLLoader> loader =
      network::SimpleURLLoader::Create(std::move(resource_request),
                                       traffic_annotation);
  loader->SetOnRedirectCallback(
      base::BindRepeating(&PaymentManifestDownloader::OnURLLoaderRedirect,
                          base::Unretained(this), loader.get()));
  loader->DownloadToStringOfUnboundedSizeUntilCrashAndDie(
      url_loader_factory_.get(),
      base::BindOnce(&PaymentManifestDownloader::OnURLLoaderComplete,
                     base::Unretained(this), loader.get()));

  auto download = std::make_unique<Download>();
  download->method = method;
  download->original_url = url;
  download->loader = std::move(loader);
  download->callback = std::move(callback);
  download->allowed_number_of_redirects = allowed_number_of_redirects;

  const network::SimpleURLLoader* identifier = download->loader.get();
  auto insert_result =
      downloads_.insert(std::make_pair(identifier, std::move(download)));
  DCHECK(insert_result.second);  // Whether the insert has succeeded.
}

}  // namespace payments
