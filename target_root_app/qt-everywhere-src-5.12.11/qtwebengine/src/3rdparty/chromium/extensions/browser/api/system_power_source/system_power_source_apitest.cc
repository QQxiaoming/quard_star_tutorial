// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <utility>

#include "base/macros.h"
#include "chromeos/dbus/dbus_thread_manager.h"
#include "chromeos/dbus/fake_power_manager_client.h"
#include "chromeos/dbus/power_manager/power_supply_properties.pb.h"
#include "extensions/common/features/feature_session_type.h"
#include "extensions/shell/test/shell_apitest.h"
#include "extensions/test/result_catcher.h"

namespace extensions {

namespace {

class SystemPowerSourceApiTest : public ShellApiTest {
 public:
  SystemPowerSourceApiTest()
      : session_type_(
            ScopedCurrentFeatureSessionType(FeatureSessionType::KIOSK)) {
    auto power_manager_client =
        std::make_unique<chromeos::FakePowerManagerClient>();

    // These values must match the assertions in
    // extensions/test/data/api_test/system_power_source/api/background.js.
    power_manager::PowerSupplyProperties_PowerSource power_source;
    power_source.set_id("AC");
    power_source.set_type(
        power_manager::PowerSupplyProperties_PowerSource_Type_MAINS);
    power_source.set_max_power(0);

    power_manager::PowerSupplyProperties props;
    props.set_external_power_source_id("AC");
    *props.add_available_external_power_source() = power_source;

    power_manager_client->UpdatePowerProperties(props);

    chromeos::DBusThreadManager::GetSetterForTesting()->SetPowerManagerClient(
        std::move(power_manager_client));
  }

  ~SystemPowerSourceApiTest() override = default;

 private:
  std::unique_ptr<base::AutoReset<FeatureSessionType>> session_type_;
  DISALLOW_COPY_AND_ASSIGN(SystemPowerSourceApiTest);
};

}  // namespace

IN_PROC_BROWSER_TEST_F(SystemPowerSourceApiTest, TestAllApiFunctions) {
  EXPECT_TRUE(RunAppTest("api_test/system_power_source/api"));
}

}  // namespace extensions
