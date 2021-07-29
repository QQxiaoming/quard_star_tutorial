// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/content/service.h"

#include <utility>

#include "base/bind.h"
#include "base/macros.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "services/content/navigable_contents_factory_impl.h"
#include "services/content/navigable_contents_impl.h"
#include "services/content/public/mojom/navigable_contents_factory.mojom.h"
#include "services/content/service_delegate.h"
#include "services/service_manager/public/cpp/service_context.h"

namespace content {

Service::Service(ServiceDelegate* delegate) : delegate_(delegate) {
  binders_.AddInterface(base::BindRepeating(
      [](Service* service, mojom::NavigableContentsFactoryRequest request) {
        service->AddNavigableContentsFactory(
            std::make_unique<NavigableContentsFactoryImpl>(service,
                                                           std::move(request)));
      },
      this));
}

Service::~Service() {
  delegate_->WillDestroyServiceInstance(this);
}

void Service::ForceQuit() {
  // Ensure that all bound interfaces are disconnected and no further interface
  // requests will be handled.
  navigable_contents_factories_.clear();
  navigable_contents_.clear();
  binders_.RemoveInterface<mojom::NavigableContentsFactory>();

  // Force-disconnect from the Service Mangager. Under normal circumstances
  // (i.e. in non-test code), the call below destroys |this|.
  context()->QuitNow();
}

void Service::AddNavigableContentsFactory(
    std::unique_ptr<NavigableContentsFactoryImpl> factory) {
  auto* raw_factory = factory.get();
  navigable_contents_factories_.emplace(raw_factory, std::move(factory));
}

void Service::RemoveNavigableContentsFactory(
    NavigableContentsFactoryImpl* factory) {
  navigable_contents_factories_.erase(factory);
}

void Service::AddNavigableContents(
    std::unique_ptr<NavigableContentsImpl> contents) {
  auto* raw_contents = contents.get();
  navigable_contents_.emplace(raw_contents, std::move(contents));
}

void Service::RemoveNavigableContents(NavigableContentsImpl* contents) {
  navigable_contents_.erase(contents);
}

void Service::OnBindInterface(const service_manager::BindSourceInfo& source,
                              const std::string& interface_name,
                              mojo::ScopedMessagePipeHandle pipe) {
  binders_.BindInterface(interface_name, std::move(pipe));
}

}  // namespace content
