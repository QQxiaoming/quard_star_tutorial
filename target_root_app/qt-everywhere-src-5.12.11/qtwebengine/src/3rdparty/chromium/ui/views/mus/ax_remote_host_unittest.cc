// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/mus/ax_remote_host.h"

#include "base/macros.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/accessibility/mojom/ax_host.mojom.h"
#include "ui/views/accessibility/ax_aura_obj_cache.h"
#include "ui/views/mus/mus_client_test_api.h"
#include "ui/views/test/views_test_base.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

namespace views {
namespace {

// Simulates the AXHostService in the browser.
class TestAXHostService : public ax::mojom::AXHost {
 public:
  explicit TestAXHostService(bool automation_enabled)
      : automation_enabled_(automation_enabled) {}
  ~TestAXHostService() override = default;

  ax::mojom::AXHostPtr CreateInterfacePtr() {
    ax::mojom::AXHostPtr ptr;
    binding_.Bind(mojo::MakeRequest(&ptr));
    return ptr;
  }

  // ax::mojom::AXHost:
  void SetRemoteHost(ax::mojom::AXRemoteHostPtr client) override {
    ++add_client_count_;
    client->OnAutomationEnabled(automation_enabled_);
    client.FlushForTesting();
  }
  void HandleAccessibilityEvent(int32_t tree_id,
                                const std::vector<ui::AXTreeUpdate>& updates,
                                const ui::AXEvent& event) override {
    ++event_count_;
    last_tree_id_ = tree_id;
    last_event_ = event;
  }

  mojo::Binding<ax::mojom::AXHost> binding_{this};
  bool automation_enabled_ = false;
  int add_client_count_ = 0;
  int event_count_ = 0;
  int last_tree_id_ = 0;
  ui::AXEvent last_event_;

 private:
  DISALLOW_COPY_AND_ASSIGN(TestAXHostService);
};

// TestView senses accessibility actions.
class TestView : public View {
 public:
  TestView() = default;
  ~TestView() override = default;

  // View:
  bool HandleAccessibleAction(const ui::AXActionData& action) override {
    ++action_count_;
    last_action_ = action;
    return true;
  }

  int action_count_ = 0;
  ui::AXActionData last_action_;

 private:
  DISALLOW_COPY_AND_ASSIGN(TestView);
};

AXRemoteHost* CreateRemote(TestAXHostService* service) {
  std::unique_ptr<AXRemoteHost> remote = std::make_unique<AXRemoteHost>();
  remote->InitForTesting(service->CreateInterfacePtr());
  remote->FlushForTesting();
  // Install the AXRemoteHost on MusClient so it monitors Widget creation.
  MusClientTestApi::SetAXRemoteHost(std::move(remote));
  return MusClient::Get()->ax_remote_host();
}

std::unique_ptr<Widget> CreateTestWidget() {
  std::unique_ptr<Widget> widget = std::make_unique<Widget>();
  Widget::InitParams params;
  params.ownership = Widget::InitParams::WIDGET_OWNS_NATIVE_WIDGET;
  params.bounds = gfx::Rect(1, 2, 333, 444);
  widget->Init(params);
  return widget;
}

using AXRemoteHostTest = ViewsTestBase;

TEST_F(AXRemoteHostTest, CreateRemote) {
  TestAXHostService service(false /*automation_enabled*/);
  CreateRemote(&service);

  // Client registered itself with service.
  EXPECT_EQ(1, service.add_client_count_);
}

TEST_F(AXRemoteHostTest, AutomationEnabled) {
  TestAXHostService service(true /*automation_enabled*/);
  AXRemoteHost* remote = CreateRemote(&service);
  std::unique_ptr<Widget> widget = CreateTestWidget();
  remote->FlushForTesting();

  // Event was sent with initial hierarchy.
  EXPECT_EQ(ax::mojom::Event::kLoadComplete, service.last_event_.event_type);
  EXPECT_EQ(AXAuraObjCache::GetInstance()->GetID(
                widget->widget_delegate()->GetContentsView()),
            service.last_event_.id);
}

// Views can trigger accessibility events during Widget construction before the
// AXRemoteHost starts monitoring the widget. This happens with the material
// design focus ring on text fields. Verify we don't crash in this case.
// https://crbug.com/862759
TEST_F(AXRemoteHostTest, SendEventBeforeWidgetCreated) {
  TestAXHostService service(true /*automation_enabled*/);
  AXRemoteHost* remote = CreateRemote(&service);
  views::View view;
  remote->HandleEvent(&view, ax::mojom::Event::kLocationChanged);
  // No crash.
}

TEST_F(AXRemoteHostTest, CreateWidgetThenEnableAutomation) {
  TestAXHostService service(false /*automation_enabled*/);
  AXRemoteHost* remote = CreateRemote(&service);
  std::unique_ptr<Widget> widget = CreateTestWidget();
  remote->FlushForTesting();

  // No events were sent because automation isn't enabled.
  EXPECT_EQ(0, service.event_count_);

  remote->OnAutomationEnabled(true);
  remote->FlushForTesting();

  // Event was sent with initial hierarchy.
  EXPECT_EQ(ax::mojom::Event::kLoadComplete, service.last_event_.event_type);
  EXPECT_EQ(AXAuraObjCache::GetInstance()->GetID(
                widget->widget_delegate()->GetContentsView()),
            service.last_event_.id);
}

TEST_F(AXRemoteHostTest, PerformAction) {
  TestAXHostService service(true /*automation_enabled*/);
  AXRemoteHost* remote = CreateRemote(&service);

  // Create a view to sense the action.
  TestView view;
  AXAuraObjCache::GetInstance()->GetOrCreate(&view);

  // Request an action on the view.
  ui::AXActionData action;
  action.action = ax::mojom::Action::kScrollDown;
  action.target_node_id = AXAuraObjCache::GetInstance()->GetID(&view);
  remote->PerformAction(action);

  // View received the action.
  EXPECT_EQ(1, view.action_count_);
  EXPECT_EQ(ax::mojom::Action::kScrollDown, view.last_action_.action);
}

}  // namespace
}  // namespace views
