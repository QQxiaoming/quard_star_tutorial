/* Copyright 2013-2014 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <skiboot.h>
#include <console.h>
#include <device.h>
#include <ipmi.h>

#include <platforms/astbmc/astbmc.h>

static bool bt_device_present;

static bool qemu_probe_common(const char *compat)
{
	struct dt_node *n;

	if (!dt_node_is_compatible(dt_root, compat))
		return false;

        astbmc_early_init();

	/* check if the BT device was defined by QEMU */
	dt_for_each_compatible(dt_root, n, "bt") {
		bt_device_present = true;
	}

	return true;
}

static bool qemu_probe(void)
{
	return qemu_probe_common("qemu,powernv");
}

static bool qemu_probe_powernv8(void)
{
	return qemu_probe_common("qemu,powernv8");
}

static bool qemu_probe_powernv9(void)
{
	return qemu_probe_common("qemu,powernv9");
}

static void qemu_init(void)
{
	if (!bt_device_present) {
		set_opal_console(&uart_opal_con);
	} else {
		astbmc_init();
	}
}

DECLARE_PLATFORM(qemu) = {
	.name		= "QEMU",
	.probe		= qemu_probe,
	.init		= qemu_init,
	.external_irq   = astbmc_ext_irq_serirq_cpld,
	.cec_power_down = astbmc_ipmi_power_down,
	.cec_reboot     = astbmc_ipmi_reboot,
	.start_preload_resource	= flash_start_preload_resource,
	.resource_loaded	= flash_resource_loaded,
	.terminate	= ipmi_terminate,
};

/*
 * For a QEMU PowerNV machine using POWER8 CPUs (Palmetto)
 */
DECLARE_PLATFORM(qemu_powernv8) = {
	.name		= "QEMU POWER8",
	.probe		= qemu_probe_powernv8,
	.bmc		= &bmc_plat_ast2400_ami,
	.init		= qemu_init,
	.external_irq   = astbmc_ext_irq_serirq_cpld,
	.cec_power_down = astbmc_ipmi_power_down,
	.cec_reboot     = astbmc_ipmi_reboot,
	.start_preload_resource	= flash_start_preload_resource,
	.resource_loaded	= flash_resource_loaded,
	.exit			= astbmc_exit,
	.terminate	= ipmi_terminate,
};

/*
 * For a QEMU PowerNV machine using POWER9 CPUs (Witherspoon)
 */
DECLARE_PLATFORM(qemu_powernv9) = {
	.name		= "QEMU POWER9",
	.probe		= qemu_probe_powernv9,
	.bmc		= &bmc_plat_ast2500_openbmc,
	.init		= qemu_init,
	.external_irq   = astbmc_ext_irq_serirq_cpld,
	.cec_power_down = astbmc_ipmi_power_down,
	.cec_reboot     = astbmc_ipmi_reboot,
	.start_preload_resource	= flash_start_preload_resource,
	.resource_loaded	= flash_resource_loaded,
	.exit			= astbmc_exit,
	.terminate	= ipmi_terminate,
};
