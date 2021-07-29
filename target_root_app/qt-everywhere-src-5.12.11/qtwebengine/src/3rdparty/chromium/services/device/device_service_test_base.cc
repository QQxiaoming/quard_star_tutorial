// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/device_service_test_base.h"

#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"
#include "base/single_thread_task_runner.h"
#include "mojo/public/cpp/bindings/binding_set.h"
#include "services/device/device_service.h"
#include "services/device/public/cpp/geolocation/location_provider.h"
#include "services/device/public/mojom/constants.mojom.h"
#include "services/network/public/cpp/weak_wrapper_shared_url_loader_factory.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "services/service_manager/public/cpp/service_context.h"
#include "services/service_manager/public/mojom/service_factory.mojom.h"

namespace device {

namespace {

const char kTestServiceName[] = "device_unittests";

// Simply return a nullptr which means no CustomLocationProvider from embedder.
std::unique_ptr<LocationProvider> GetCustomLocationProviderForTest() {
  return nullptr;
}

// The test service responsible to package Device Service.
class ServiceTestClient : public service_manager::test::ServiceTestClient,
                          public service_manager::mojom::ServiceFactory {
 public:
  explicit ServiceTestClient(
      service_manager::test::ServiceTest* test,
      scoped_refptr<base::SingleThreadTaskRunner> file_task_runner,
      scoped_refptr<base::SingleThreadTaskRunner> io_task_runner,
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
      : service_manager::test::ServiceTestClient(test),
        file_task_runner_(std::move(file_task_runner)),
        io_task_runner_(std::move(io_task_runner)),
        url_loader_factory_(std::move(url_loader_factory)) {
    registry_.AddInterface<service_manager::mojom::ServiceFactory>(
        base::Bind(&ServiceTestClient::Create, base::Unretained(this)));
  }
  ~ServiceTestClient() override {}

 protected:
  void OnBindInterface(const service_manager::BindSourceInfo& source_info,
                       const std::string& interface_name,
                       mojo::ScopedMessagePipeHandle interface_pipe) override {
    registry_.BindInterface(interface_name, std::move(interface_pipe));
  }

  void CreateService(
      service_manager::mojom::ServiceRequest request,
      const std::string& name,
      service_manager::mojom::PIDReceiverPtr pid_receiver) override {
    if (name == device::mojom::kServiceName) {
#if defined(OS_ANDROID)
      device_service_context_.reset(new service_manager::ServiceContext(
          CreateDeviceService(
              file_task_runner_, io_task_runner_, url_loader_factory_,
              kTestGeolocationApiKey, false, wake_lock_context_callback_,
              base::BindRepeating(&GetCustomLocationProviderForTest), nullptr),
          std::move(request)));
#else
      device_service_context_.reset(new service_manager::ServiceContext(
          CreateDeviceService(
              file_task_runner_, io_task_runner_, url_loader_factory_,
              kTestGeolocationApiKey,
              base::BindRepeating(&GetCustomLocationProviderForTest)),
          std::move(request)));
#endif
    }
  }

  void Create(service_manager::mojom::ServiceFactoryRequest request) {
    service_factory_bindings_.AddBinding(this, std::move(request));
  }

 private:
  service_manager::BinderRegistry registry_;
  mojo::BindingSet<service_manager::mojom::ServiceFactory>
      service_factory_bindings_;
  std::unique_ptr<service_manager::ServiceContext> device_service_context_;
  const scoped_refptr<base::SingleThreadTaskRunner> file_task_runner_;
  const scoped_refptr<base::SingleThreadTaskRunner> io_task_runner_;
  const scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;

  WakeLockContextCallback wake_lock_context_callback_;

  DISALLOW_COPY_AND_ASSIGN(ServiceTestClient);
};

}  // namespace

DeviceServiceTestBase::DeviceServiceTestBase()
    : ServiceTest(kTestServiceName),
      file_thread_("DeviceServiceTestFileThread"),
      io_thread_("DeviceServiceTestIOThread") {
  file_thread_.Start();
  io_thread_.StartWithOptions(
      base::Thread::Options(base::MessageLoop::TYPE_IO, 0));
}

DeviceServiceTestBase::~DeviceServiceTestBase() {}

std::unique_ptr<service_manager::Service>
DeviceServiceTestBase::CreateService() {
  return std::make_unique<ServiceTestClient>(
      this, file_thread_.task_runner(), io_thread_.task_runner(),
      base::MakeRefCounted<network::WeakWrapperSharedURLLoaderFactory>(
          &test_url_loader_factory_));
}

}  // namespace device
