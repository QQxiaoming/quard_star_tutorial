// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/callback_forward.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "base/optional.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/post_task.h"
#include "base/task/task_traits.h"
#include "base/test/scoped_feature_list.h"
#include "content/browser/service_worker/service_worker_context_core.h"
#include "content/browser/service_worker/service_worker_context_wrapper.h"
#include "content/browser/service_worker/service_worker_version.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/platform_notification_context.h"
#include "content/public/browser/service_worker_context_observer.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/content_browser_test.h"
#include "content/public/test/content_browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "content/shell/browser/shell.h"
#include "content/shell/browser/shell_content_browser_client.h"
#include "content/test/test_content_browser_client.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "third_party/blink/public/mojom/service_worker/service_worker_event_status.mojom.h"
#include "url/gurl.h"

namespace content {

namespace {
// Waits for a service worker to be activated.
class ActivatedServiceWorkerObserver : public ServiceWorkerContextObserver {
 public:
  ActivatedServiceWorkerObserver() : ServiceWorkerContextObserver() {}
  ~ActivatedServiceWorkerObserver() override {}

  void OnVersionActivated(int64_t version_id, const GURL& scope) override {
    version_id_ = version_id;
    if (activated_callback_)
      std::move(activated_callback_).Run();
  }

  void WaitForActivated() {
    if (version_id_ != -1)
      return;
    base::RunLoop loop;
    activated_callback_ = loop.QuitClosure();
    loop.Run();
  }

  int64_t version_id() const { return version_id_; }

 private:
  base::OnceClosure activated_callback_;
  int64_t version_id_ = -1;

  DISALLOW_COPY_AND_ASSIGN(ActivatedServiceWorkerObserver);
};

// Mainly for testing that ShouldAllowOpenURL() is properly consulted.
class ServiceWorkerClientsContentBrowserClient
    : public TestContentBrowserClient {
 public:
  ServiceWorkerClientsContentBrowserClient() : TestContentBrowserClient() {}
  ~ServiceWorkerClientsContentBrowserClient() override {}

  const GURL& opened_url() const { return opened_url_; }

  void SetToAllowOpenURL(bool allow) { allow_open_url_ = allow; }

  void WaitForOpenURL() {
    if (!opened_url_.is_empty())
      return;
    base::RunLoop run_loop;
    opened_url_callback_ = run_loop.QuitClosure();
    run_loop.Run();
  }

  // ContentBrowserClient overrides:
  bool ShouldAllowOpenURL(SiteInstance* site_instance,
                          const GURL& url) override {
    return allow_open_url_;
  }

  void OpenURL(
      SiteInstance* site_instance,
      const OpenURLParams& params,
      const base::RepeatingCallback<void(WebContents*)>& callback) override {
    opened_url_ = params.url;
    callback.Run(nullptr);
    if (opened_url_callback_)
      std::move(opened_url_callback_).Run();
  }

 private:
  bool allow_open_url_ = true;
  GURL opened_url_;
  base::OnceClosure opened_url_callback_;

  DISALLOW_COPY_AND_ASSIGN(ServiceWorkerClientsContentBrowserClient);
};

}  // namespace

// Tests for the service worker Clients API.
class ServiceWorkerClientsApiBrowserTest : public ContentBrowserTest {
 public:
  ServiceWorkerClientsApiBrowserTest() = default;

  void SetUp() override {
    ASSERT_TRUE(embedded_test_server()->InitializeAndListen());
    ContentBrowserTest::SetUp();
  }

  void SetUpOnMainThread() override {
    embedded_test_server()->StartAcceptingConnections();

    StoragePartition* partition = BrowserContext::GetDefaultStoragePartition(
        shell()->web_contents()->GetBrowserContext());
    wrapper_ = static_cast<ServiceWorkerContextWrapper*>(
        partition->GetServiceWorkerContext());
  }

  void DispatchNotificationClickEvent(int64_t version_id) {
    base::RunLoop run_loop;
    base::PostTaskWithTraits(
        FROM_HERE, {BrowserThread::IO},
        base::BindOnce(&ServiceWorkerClientsApiBrowserTest::
                           DispatchNotificationClickEventOnIOThread,
                       base::Unretained(this), version_id,
                       run_loop.QuitClosure()));
    run_loop.Run();
  }

  void DispatchNotificationClickEventOnIOThread(int64_t version_id,
                                                base::OnceClosure done) {
    ServiceWorkerVersion* version =
        wrapper_->context()->GetLiveVersion(version_id);
    ASSERT_TRUE(version);
    version->RunAfterStartWorker(
        ServiceWorkerMetrics::EventType::NOTIFICATION_CLICK,
        base::BindOnce(&ServiceWorkerClientsApiBrowserTest::
                           DispatchNotificationClickAfterStartWorker,
                       base::Unretained(this), base::WrapRefCounted(version),
                       std::move(done)));
  }

  void DispatchNotificationClickAfterStartWorker(
      scoped_refptr<ServiceWorkerVersion> version,
      base::OnceClosure done,
      blink::ServiceWorkerStatusCode status) {
    EXPECT_EQ(blink::ServiceWorkerStatusCode::kOk, status);
    version->StartRequest(ServiceWorkerMetrics::EventType::NOTIFICATION_CLICK,
                          base::DoNothing());
    version->endpoint()->DispatchNotificationClickEvent(
        "notification_id", {} /* notification_data */, -1 /* action_index */,
        base::nullopt /* reply */,
        base::BindOnce(
            [](base::OnceClosure done,
               blink::mojom::ServiceWorkerEventStatus event_status) {
              EXPECT_EQ(blink::mojom::ServiceWorkerEventStatus::COMPLETED,
                        event_status);
              std::move(done).Run();
            },
            std::move(done)));
  }

  ServiceWorkerContextWrapper* wrapper() { return wrapper_.get(); }

 private:
  scoped_refptr<ServiceWorkerContextWrapper> wrapper_;
};

// Tests a successful WindowClient.navigate() call.
IN_PROC_BROWSER_TEST_F(ServiceWorkerClientsApiBrowserTest, Navigate) {
  // Load a page that registers a service worker.
  EXPECT_TRUE(NavigateToURL(shell(),
                            embedded_test_server()->GetURL(
                                "/service_worker/create_service_worker.html")));
  EXPECT_EQ("DONE", EvalJs(shell(), "register('client_api_worker.js');"));

  // Load the page again so we are controlled.
  EXPECT_TRUE(NavigateToURL(shell(),
                            embedded_test_server()->GetURL(
                                "/service_worker/create_service_worker.html")));
  EXPECT_EQ(true, EvalJs(shell(), "!!navigator.serviceWorker.controller"));

  // Have the service worker call client.navigate() on the page.
  const std::string navigate_script = R"(
    (async () => {
      const registration = await navigator.serviceWorker.ready;
      registration.active.postMessage({command: 'navigate', url: 'empty.html'});
      return true;
    })();
  )";
  EXPECT_EQ(true, EvalJs(shell(), navigate_script));

  // The page should be navigated to empty.html.
  const base::string16 title =
      base::ASCIIToUTF16("ServiceWorker test - empty page");
  TitleWatcher title_watcher(shell()->web_contents(), title);
  EXPECT_EQ(title, title_watcher.WaitAndGetTitle());
}

// Tests a WindowClient.navigate() call to a disallowed URL.
IN_PROC_BROWSER_TEST_F(ServiceWorkerClientsApiBrowserTest,
                       NavigateDisallowedUrl) {
  ServiceWorkerClientsContentBrowserClient content_browser_client;
  content_browser_client.SetToAllowOpenURL(false);
  auto* old_content_browser_client =
      SetBrowserClientForTesting(&content_browser_client);

  // Load a page that registers a service worker.
  EXPECT_TRUE(NavigateToURL(shell(),
                            embedded_test_server()->GetURL(
                                "/service_worker/create_service_worker.html")));
  EXPECT_EQ("DONE", EvalJs(shell(), "register('client_api_worker.js');"));

  // Load the page again so we are controlled.
  EXPECT_TRUE(NavigateToURL(shell(),
                            embedded_test_server()->GetURL(
                                "/service_worker/create_service_worker.html")));
  EXPECT_EQ(true, EvalJs(shell(), "!!navigator.serviceWorker.controller"));

  // Have the service worker call client.navigate() on the page.
  const std::string navigate_script = R"(
    (async () => {
      const registration = await navigator.serviceWorker.ready;
      registration.active.postMessage({command: 'navigate', url: 'empty.html'});
      return true;
    })();
  )";
  EXPECT_EQ(true, EvalJs(shell(), navigate_script));

  // The page should be navigated to about:blank instead of the requested URL.
  const base::string16 title = base::ASCIIToUTF16("about:blank");
  TitleWatcher title_watcher(shell()->web_contents(), title);
  EXPECT_EQ(title, title_watcher.WaitAndGetTitle());

  SetBrowserClientForTesting(old_content_browser_client);
}

// Tests a WindowClient.navigate() call during a browser-initiated navigation.
// Regression test for https://crbug.com/930154.
IN_PROC_BROWSER_TEST_F(ServiceWorkerClientsApiBrowserTest,
                       NavigateDuringBrowserNavigation) {
  // Load a page that registers a service worker.
  EXPECT_TRUE(NavigateToURL(shell(),
                            embedded_test_server()->GetURL(
                                "/service_worker/create_service_worker.html")));
  EXPECT_EQ("DONE", EvalJs(shell(), "register('client_api_worker.js');"));

  // Load the test page.
  EXPECT_TRUE(NavigateToURL(
      shell(),
      embedded_test_server()->GetURL("/service_worker/request_navigate.html")));

  // Start a browser-initiated navigation.
  GURL url(embedded_test_server()->GetURL("/title1.html"));
  TestNavigationManager navigation(shell()->web_contents(), url);
  shell()->LoadURL(url);
  EXPECT_TRUE(navigation.WaitForRequestStart());

  // Have the service worker call client.navigate() to try to go to another
  // URL. It should fail.
  EXPECT_EQ("navigate failed", EvalJs(shell(), "requestToNavigate();"));

  // The browser-initiated navigation should finish.
  navigation.WaitForNavigationFinished();  // Resume navigation.
  EXPECT_TRUE(navigation.was_successful());
}

// Tests a successful Clients.openWindow() call.
IN_PROC_BROWSER_TEST_F(ServiceWorkerClientsApiBrowserTest, OpenWindow) {
  ServiceWorkerClientsContentBrowserClient content_browser_client;
  auto* old_content_browser_client =
      SetBrowserClientForTesting(&content_browser_client);

  ActivatedServiceWorkerObserver observer;
  wrapper()->AddObserver(&observer);

  // Load a page that registers a service worker.
  EXPECT_TRUE(NavigateToURL(shell(),
                            embedded_test_server()->GetURL(
                                "/service_worker/create_service_worker.html")));
  EXPECT_EQ("DONE", EvalJs(shell(), "register('client_api_worker.js');"));
  observer.WaitForActivated();

  // Tell the service worker to call clients.openWindow(). Do it from a
  // notification click event so it has a user interaction token that allows
  // popups.
  int64_t id = observer.version_id();
  EXPECT_NE(-1, id);
  DispatchNotificationClickEvent(id);

  // OpenURL() should be called.
  content_browser_client.WaitForOpenURL();
  EXPECT_EQ(embedded_test_server()->GetURL("/service_worker/empty.html"),
            content_browser_client.opened_url());

  wrapper()->RemoveObserver(&observer);
  SetBrowserClientForTesting(old_content_browser_client);
}

// Tests a Clients.openWindow() call to a disallowed URL.
IN_PROC_BROWSER_TEST_F(ServiceWorkerClientsApiBrowserTest,
                       OpenWindowDisallowedUrl) {
  ServiceWorkerClientsContentBrowserClient content_browser_client;
  content_browser_client.SetToAllowOpenURL(false);
  auto* old_content_browser_client =
      SetBrowserClientForTesting(&content_browser_client);

  ActivatedServiceWorkerObserver observer;
  wrapper()->AddObserver(&observer);

  // Load a page that registers a service worker.
  EXPECT_TRUE(NavigateToURL(shell(),
                            embedded_test_server()->GetURL(
                                "/service_worker/create_service_worker.html")));
  EXPECT_EQ("DONE", EvalJs(shell(), "register('client_api_worker.js');"));
  observer.WaitForActivated();

  // Tell the service worker to call clients.openWindow(). Do it from a
  // notification click event so it has a user interaction token that allows
  // popups.
  int64_t id = observer.version_id();
  EXPECT_NE(-1, id);
  DispatchNotificationClickEvent(id);

  // OpenURL() should be called with about:blank instead of the requested URL.
  content_browser_client.WaitForOpenURL();
  EXPECT_EQ(GURL("about:blank"), content_browser_client.opened_url());

  wrapper()->RemoveObserver(&observer);
  SetBrowserClientForTesting(old_content_browser_client);
}

}  // namespace content
