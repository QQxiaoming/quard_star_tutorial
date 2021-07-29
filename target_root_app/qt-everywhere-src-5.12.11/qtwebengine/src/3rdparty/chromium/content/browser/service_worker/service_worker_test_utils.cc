// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/service_worker/service_worker_test_utils.h"

#include <memory>
#include <utility>

#include "base/barrier_closure.h"
#include "base/run_loop.h"
#include "base/time/time.h"
#include "content/browser/service_worker/service_worker_context_core.h"
#include "content/browser/service_worker/service_worker_database.h"
#include "content/browser/service_worker/service_worker_disk_cache.h"
#include "content/browser/service_worker/service_worker_provider_host.h"
#include "content/browser/service_worker/service_worker_registration.h"
#include "content/browser/service_worker/service_worker_storage.h"
#include "content/public/common/child_process_host.h"
#include "net/base/io_buffer.h"
#include "net/base/test_completion_callback.h"
#include "net/http/http_response_info.h"
#include "third_party/blink/public/common/service_worker/service_worker_utils.h"

namespace content {

namespace {

// A mock SharedURLLoaderFactory that always fails to start.
// TODO(bashi): Make this factory not to fail when unit tests actually need
// this to be working.
class MockSharedURLLoaderFactory final
    : public network::SharedURLLoaderFactory {
 public:
  MockSharedURLLoaderFactory() = default;

  // network::mojom::URLLoaderFactory:
  void CreateLoaderAndStart(network::mojom::URLLoaderRequest request,
                            int32_t routing_id,
                            int32_t request_id,
                            uint32_t options,
                            const network::ResourceRequest& url_request,
                            network::mojom::URLLoaderClientPtr client,
                            const net::MutableNetworkTrafficAnnotationTag&
                                traffic_annotation) override {
    client->OnComplete(
        network::URLLoaderCompletionStatus(net::ERR_NOT_IMPLEMENTED));
  }
  void Clone(network::mojom::URLLoaderFactoryRequest request) override {
    NOTREACHED();
  }

  // network::SharedURLLoaderFactory:
  std::unique_ptr<network::SharedURLLoaderFactoryInfo> Clone() override {
    NOTREACHED();
    return nullptr;
  }

 private:
  friend class base::RefCounted<MockSharedURLLoaderFactory>;

  ~MockSharedURLLoaderFactory() override = default;

  DISALLOW_COPY_AND_ASSIGN(MockSharedURLLoaderFactory);
};

// Returns MockSharedURLLoaderFactory.
class MockSharedURLLoaderFactoryInfo final
    : public network::SharedURLLoaderFactoryInfo {
 public:
  MockSharedURLLoaderFactoryInfo() = default;
  ~MockSharedURLLoaderFactoryInfo() override = default;

 protected:
  scoped_refptr<network::SharedURLLoaderFactory> CreateFactory() override {
    return base::MakeRefCounted<MockSharedURLLoaderFactory>();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(MockSharedURLLoaderFactoryInfo);
};

void OnWriteBodyInfoToDiskCache(
    std::unique_ptr<ServiceWorkerResponseWriter> writer,
    const std::string& body,
    base::OnceClosure callback,
    int result) {
  EXPECT_GE(result, 0);
  scoped_refptr<net::IOBuffer> body_buffer =
      base::MakeRefCounted<net::StringIOBuffer>(body);
  ServiceWorkerResponseWriter* writer_rawptr = writer.get();
  writer_rawptr->WriteData(
      body_buffer.get(), body.size(),
      base::BindOnce(
          [](std::unique_ptr<ServiceWorkerResponseWriter> /* unused */,
             base::OnceClosure callback, int expected, int result) {
            EXPECT_EQ(expected, result);
            std::move(callback).Run();
          },
          std::move(writer), std::move(callback), body.size()));
}

void WriteBodyToDiskCache(std::unique_ptr<ServiceWorkerResponseWriter> writer,
                          std::unique_ptr<net::HttpResponseInfo> info,
                          const std::string& body,
                          base::OnceClosure callback) {
  scoped_refptr<HttpResponseInfoIOBuffer> info_buffer =
      base::MakeRefCounted<HttpResponseInfoIOBuffer>(info.release());
  info_buffer->response_data_size = body.size();
  ServiceWorkerResponseWriter* writer_rawptr = writer.get();
  writer_rawptr->WriteInfo(
      info_buffer.get(),
      base::BindOnce(&OnWriteBodyInfoToDiskCache, std::move(writer), body,
                     std::move(callback)));
}

void WriteMetaDataToDiskCache(
    std::unique_ptr<ServiceWorkerResponseMetadataWriter> writer,
    const std::string& meta_data,
    base::OnceClosure callback) {
  scoped_refptr<net::IOBuffer> meta_data_buffer =
      base::MakeRefCounted<net::StringIOBuffer>(meta_data);
  ServiceWorkerResponseMetadataWriter* writer_rawptr = writer.get();
  writer_rawptr->WriteMetadata(
      meta_data_buffer.get(), meta_data.size(),
      base::Bind(
          [](std::unique_ptr<ServiceWorkerResponseMetadataWriter> /* unused */,
             base::OnceClosure callback, int expected, int result) {
            EXPECT_EQ(expected, result);
            std::move(callback).Run();
          },
          base::Passed(&writer), base::Passed(&callback), meta_data.size()));
}

}  // namespace

ServiceWorkerRemoteProviderEndpoint::ServiceWorkerRemoteProviderEndpoint() {}
ServiceWorkerRemoteProviderEndpoint::ServiceWorkerRemoteProviderEndpoint(
    ServiceWorkerRemoteProviderEndpoint&& other)
    : host_ptr_(std::move(other.host_ptr_)),
      client_request_(std::move(other.client_request_)) {}

ServiceWorkerRemoteProviderEndpoint::~ServiceWorkerRemoteProviderEndpoint() {}

void ServiceWorkerRemoteProviderEndpoint::BindWithProviderHostInfo(
    mojom::ServiceWorkerProviderHostInfoPtr* info) {
  mojom::ServiceWorkerContainerAssociatedPtr client_ptr;
  client_request_ = mojo::MakeRequestAssociatedWithDedicatedPipe(&client_ptr);
  (*info)->client_ptr_info = client_ptr.PassInterface();
  (*info)->host_request =
      mojo::MakeRequestAssociatedWithDedicatedPipe(&host_ptr_);
}

void ServiceWorkerRemoteProviderEndpoint::BindWithProviderInfo(
    mojom::ServiceWorkerProviderInfoForStartWorkerPtr info) {
  client_request_ = std::move(info->client_request);
  host_ptr_.Bind(std::move(info->host_ptr_info));
}

mojom::ServiceWorkerProviderHostInfoPtr CreateProviderHostInfoForWindow(
    int provider_id,
    int route_id) {
  return mojom::ServiceWorkerProviderHostInfo::New(
      provider_id, route_id,
      blink::mojom::ServiceWorkerProviderType::kForWindow,
      true /* is_parent_frame_secure */, nullptr /* host_request */,
      nullptr /* client_ptr_info */);
}

std::unique_ptr<ServiceWorkerProviderHost> CreateProviderHostForWindow(
    int process_id,
    int provider_id,
    bool is_parent_frame_secure,
    base::WeakPtr<ServiceWorkerContextCore> context,
    ServiceWorkerRemoteProviderEndpoint* output_endpoint) {
  mojom::ServiceWorkerProviderHostInfoPtr info =
      CreateProviderHostInfoForWindow(provider_id, 1 /* route_id */);
  info->is_parent_frame_secure = is_parent_frame_secure;
  output_endpoint->BindWithProviderHostInfo(&info);
  return ServiceWorkerProviderHost::Create(process_id, std::move(info),
                                           std::move(context));
}

base::WeakPtr<ServiceWorkerProviderHost>
CreateProviderHostForServiceWorkerContext(
    int process_id,
    bool is_parent_frame_secure,
    ServiceWorkerVersion* hosted_version,
    base::WeakPtr<ServiceWorkerContextCore> context,
    ServiceWorkerRemoteProviderEndpoint* output_endpoint) {
  auto provider_info = mojom::ServiceWorkerProviderInfoForStartWorker::New();
  base::WeakPtr<ServiceWorkerProviderHost> host =
      ServiceWorkerProviderHost::PreCreateForController(
          std::move(context), base::WrapRefCounted(hosted_version),
          &provider_info);

  host->SetDocumentUrl(hosted_version->script_url());

  scoped_refptr<network::SharedURLLoaderFactory> loader_factory;
  if (blink::ServiceWorkerUtils::IsServicificationEnabled()) {
    loader_factory = network::SharedURLLoaderFactory::Create(
        std::make_unique<MockSharedURLLoaderFactoryInfo>());
  }

  provider_info = host->CompleteStartWorkerPreparation(
      process_id, loader_factory, std::move(provider_info));
  output_endpoint->BindWithProviderInfo(std::move(provider_info));
  return host;
}

ServiceWorkerDatabase::ResourceRecord WriteToDiskCacheSync(
    ServiceWorkerStorage* storage,
    const GURL& script_url,
    int64_t resource_id,
    const std::vector<std::pair<std::string, std::string>>& headers,
    const std::string& body,
    const std::string& meta_data) {
  base::RunLoop loop;
  ServiceWorkerDatabase::ResourceRecord record =
      WriteToDiskCacheAsync(storage, script_url, resource_id, headers, body,
                            meta_data, loop.QuitClosure());
  loop.Run();
  return record;
}

ServiceWorkerDatabase::ResourceRecord
WriteToDiskCacheWithCustomResponseInfoSync(
    ServiceWorkerStorage* storage,
    const GURL& script_url,
    int64_t resource_id,
    std::unique_ptr<net::HttpResponseInfo> http_info,
    const std::string& body,
    const std::string& meta_data) {
  base::RunLoop loop;
  ServiceWorkerDatabase::ResourceRecord record =
      WriteToDiskCacheWithCustomResponseInfoAsync(
          storage, script_url, resource_id, std::move(http_info), body,
          meta_data, loop.QuitClosure());
  loop.Run();
  return record;
}

ServiceWorkerDatabase::ResourceRecord WriteToDiskCacheAsync(
    ServiceWorkerStorage* storage,
    const GURL& script_url,
    int64_t resource_id,
    const std::vector<std::pair<std::string, std::string>>& headers,
    const std::string& body,
    const std::string& meta_data,
    base::OnceClosure callback) {
  std::unique_ptr<net::HttpResponseInfo> info =
      std::make_unique<net::HttpResponseInfo>();
  info->request_time = base::Time::Now();
  info->response_time = base::Time::Now();
  info->headers =
      base::MakeRefCounted<net::HttpResponseHeaders>("HTTP/1.0 200 OK\0\0");
  for (const auto& header : headers)
    info->headers->AddHeader(header.first + ": " + header.second);
  return WriteToDiskCacheWithCustomResponseInfoAsync(
      storage, script_url, resource_id, std::move(info), body, meta_data,
      std::move(callback));
}

ServiceWorkerDatabase::ResourceRecord
WriteToDiskCacheWithCustomResponseInfoAsync(
    ServiceWorkerStorage* storage,
    const GURL& script_url,
    int64_t resource_id,
    std::unique_ptr<net::HttpResponseInfo> http_info,
    const std::string& body,
    const std::string& meta_data,
    base::OnceClosure callback) {
  base::RepeatingClosure barrier = base::BarrierClosure(2, std::move(callback));
  auto body_writer = storage->CreateResponseWriter(resource_id);
  WriteBodyToDiskCache(std::move(body_writer), std::move(http_info), body,
                       barrier);
  auto metadata_writer = storage->CreateResponseMetadataWriter(resource_id);
  WriteMetaDataToDiskCache(std::move(metadata_writer), meta_data,
                           std::move(barrier));
  return ServiceWorkerDatabase::ResourceRecord(resource_id, script_url,
                                               body.size());
}

}  // namespace content
