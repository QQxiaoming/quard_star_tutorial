// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/frame_host/origin_policy_throttle.h"

#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/no_destructor.h"
#include "content/browser/frame_host/navigation_handle_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/network_service_instance.h"
#include "content/public/common/content_features.h"
#include "content/public/common/content_switches.h"
#include "net/http/http_request_headers.h"
#include "services/network/network_context.h"
#include "services/network/network_service.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "url/origin.h"

namespace {
// Constants derived from the spec, https://github.com/WICG/origin-policy
static const char* kDefaultPolicy = "1";
static const char* kDeletePolicy = "0";
static const char* kWellKnown = "/.well-known/origin-policy/";

// Maximum policy size (implementation-defined limit in bytes).
// (Limit copied from network::SimpleURLLoader::kMaxBoundedStringDownloadSize.)
static const size_t kMaxPolicySize = 1024 * 1024;
}  // namespace

namespace content {

// static
bool OriginPolicyThrottle::ShouldRequestOriginPolicy(
    const GURL& url,
    std::string* request_version) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  if (!base::FeatureList::IsEnabled(features::kOriginPolicy))
    return false;

  if (!url.SchemeIs(url::kHttpsScheme))
    return false;

  if (request_version) {
    const KnownVersionMap& versions = GetKnownVersions();
    const auto iter = versions.find(url::Origin::Create(url));
    *request_version =
        iter == versions.end() ? std::string(kDefaultPolicy) : iter->second;
  }
  return true;
}

// static
std::unique_ptr<NavigationThrottle>
OriginPolicyThrottle::MaybeCreateThrottleFor(NavigationHandle* handle) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK(handle);

  // We use presence of the origin policy request header to determine
  // whether we should create the throttle.
  if (!handle->GetRequestHeaders().HasHeader(
          net::HttpRequestHeaders::kSecOriginPolicy))
    return nullptr;

  DCHECK(base::FeatureList::IsEnabled(features::kOriginPolicy));
  return base::WrapUnique(new OriginPolicyThrottle(handle));
}

OriginPolicyThrottle::~OriginPolicyThrottle() {}

NavigationThrottle::ThrottleCheckResult
OriginPolicyThrottle::WillStartRequest() {
  // TODO(vogelheim): It might be faster in the common case to optimistically
  //     fetch the policy indicated in the request already here. This would
  //     be faster if the last known version is the current version, but
  //     slower (and wasteful of bandwidth) if the server sends us a new
  //     version. It's unclear how much the upside is, though.
  return NavigationThrottle::PROCEED;
}

NavigationThrottle::ThrottleCheckResult
OriginPolicyThrottle::WillProcessResponse() {
  DCHECK(navigation_handle());

  // Per spec, Origin Policies are only fetched for https:-requests. So we
  // should always have HTTP headers at this point.
  // However, some unit tests generate responses without headers, so we still
  // need to check.
  if (!navigation_handle()->GetResponseHeaders())
    return NavigationThrottle::PROCEED;

  // This determines whether and which policy version applies and fetches it.
  //
  // Inputs are the kSecOriginPolicy HTTP header, and the version
  // we've last seen from this particular origin.
  //
  // - header with kDeletePolicy received: No policy applies, and delete the
  //       last-known policy for this origin.
  // - header received: Use header version and update last-known policy.
  // - no header received, last-known version exists: Use last-known version
  // - no header, no last-known version: No policy applies.

  std::string response_version;
  bool header_found =
      navigation_handle()->GetResponseHeaders()->GetNormalizedHeader(
          net::HttpRequestHeaders::kSecOriginPolicy, &response_version);

  url::Origin origin = GetRequestOrigin();
  DCHECK(!origin.Serialize().empty());
  DCHECK(!origin.unique());
  KnownVersionMap& versions = GetKnownVersions();
  KnownVersionMap::iterator iter = versions.find(origin);

  // Process policy deletion first!
  if (header_found && response_version == kDeletePolicy) {
    if (iter != versions.end())
      versions.erase(iter);
    return NavigationThrottle::PROCEED;
  }

  // No policy applies to this request?
  if (!header_found && iter == versions.end()) {
    return NavigationThrottle::PROCEED;
  }

  if (!header_found)
    response_version = iter->second;
  else if (iter == versions.end())
    versions.insert(std::make_pair(origin, response_version));
  else
    iter->second = response_version;

  GURL policy = GURL(origin.Serialize() + kWellKnown + response_version);
  FetchCallback done =
      base::BindOnce(&OriginPolicyThrottle::OnTheGloriousPolicyHasArrived,
                     base::Unretained(this));
  FetchPolicy(policy, std::move(done));
  return NavigationThrottle::DEFER;
}

const char* OriginPolicyThrottle::GetNameForLogging() {
  return "OriginPolicyThrottle";
}

// static
OriginPolicyThrottle::KnownVersionMap&
OriginPolicyThrottle::GetKnownVersionsForTesting() {
  return GetKnownVersions();
}

OriginPolicyThrottle::OriginPolicyThrottle(NavigationHandle* handle)
    : NavigationThrottle(handle) {}

OriginPolicyThrottle::KnownVersionMap&
OriginPolicyThrottle::GetKnownVersions() {
  static base::NoDestructor<KnownVersionMap> map_instance;
  return *map_instance;
}

const url::Origin OriginPolicyThrottle::GetRequestOrigin() {
  return url::Origin::Create(navigation_handle()->GetURL());
}

void OriginPolicyThrottle::FetchPolicy(const GURL& url, FetchCallback done) {
  // Obtain a network context from the network service.
  network::mojom::NetworkContextParamsPtr context_params =
      network::mojom::NetworkContextParams::New();
  content::GetNetworkService()->CreateNetworkContext(
      mojo::MakeRequest(&network_context_ptr_), std::move(context_params));

  // Create URLLoaderFactory
  network::mojom::URLLoaderFactoryParamsPtr url_loader_factory_params =
      network::mojom::URLLoaderFactoryParams::New();
  url_loader_factory_params->process_id = network::mojom::kBrowserProcessId;
  url_loader_factory_params->is_corb_enabled = false;
  url_loader_factory_params->disable_web_security =
      base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kDisableWebSecurity);
  network_context_ptr_->CreateURLLoaderFactory(
      mojo::MakeRequest(&url_loader_factory_),
      std::move(url_loader_factory_params));

  // Create the traffic annotation
  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("origin_policy_loader", R"(
        semantics {
          sender: "Origin Policy URL Loader Throttle"
          description:
            "Fetches the Origin Policy with a given version from an origin."
          trigger:
            "In case the Origin Policy with a given version does not "
            "exist in the cache, it is fetched from the origin at a "
            "well-known location."
          data:
            "None, the URL itself contains the origin and Origin Policy "
            "version."
          destination: OTHER
        }
        policy {
          cookies_allowed: NO
          setting: "This feature cannot be disabled by settings. Server "
            "opt-in or out of this mechanism."
          policy_exception_justification:
            "Not implemented, considered not useful."})");

  // Create the SimpleURLLoader for the policy.
  std::unique_ptr<network::ResourceRequest> policy_request =
      std::make_unique<network::ResourceRequest>();
  policy_request->url = url;
  policy_request->fetch_redirect_mode =
      network::mojom::FetchRedirectMode::kError;
  url_loader_ = network::SimpleURLLoader::Create(std::move(policy_request),
                                                 traffic_annotation);
  url_loader_->SetAllowHttpErrorResults(false);

  // Start the download, and pass the callback for when we're finished.
  url_loader_->DownloadToString(url_loader_factory_.get(), std::move(done),
                                kMaxPolicySize);
}

void OriginPolicyThrottle::InjectPolicyForTesting(
    const std::string& policy_content) {
  OnTheGloriousPolicyHasArrived(std::make_unique<std::string>(policy_content));
}

void OriginPolicyThrottle::OnTheGloriousPolicyHasArrived(
    std::unique_ptr<std::string> policy_content) {
  // Release resources associated with the loading.
  url_loader_.reset();
  url_loader_factory_.reset();
  network_context_ptr_.reset();

  // Fail hard if the policy could not be loaded.
  if (!policy_content) {
    CancelDeferredNavigation(NavigationThrottle::CANCEL_AND_IGNORE);
    return;
  }

  // TODO(vogelheim): Determine whether we need to parse or sanity check
  //                  the policy content at this point.

  static_cast<NavigationHandleImpl*>(navigation_handle())
      ->set_origin_policy(*policy_content);
  Resume();
}

}  // namespace content
