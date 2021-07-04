/**
 * Copyright (c) 2019 YADRO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <skiboot.h>
#include <device.h>
#include <ipmi.h>

#include "astbmc.h"

#define CHIP_ID_CPU0 0x00
#define CHIP_ID_CPU1 0x08

ST_PLUGGABLE(nicole_backplane0, "Backplane0 (16x)");
ST_PLUGGABLE(nicole_backplane1, "Backplane1 (16x)");

ST_BUILTIN_DEV(nicole_builtin_net, "Builtin Network");
ST_BUILTIN_DEV(nicole_builtin_ssd0, "Builtin SSD0");
ST_BUILTIN_DEV(nicole_builtin_ssd1, "Builtin SSD1");
ST_BUILTIN_DEV(nicole_builtin_vga, "Builtin VGA");
ST_BUILTIN_DEV(nicole_builtin_usb, "Builtin USB");

static const struct slot_table_entry nicole_phb_table[] = {
	ST_PHB_ENTRY(CHIP_ID_CPU0, 0, nicole_backplane0),
	ST_PHB_ENTRY(CHIP_ID_CPU0, 1, nicole_builtin_net),
	ST_PHB_ENTRY(CHIP_ID_CPU0, 2, nicole_builtin_ssd0),
	ST_PHB_ENTRY(CHIP_ID_CPU0, 3, nicole_backplane1),

	ST_PHB_ENTRY(CHIP_ID_CPU1, 3, nicole_builtin_ssd1),
	ST_PHB_ENTRY(CHIP_ID_CPU1, 4, nicole_builtin_vga),
	ST_PHB_ENTRY(CHIP_ID_CPU1, 5, nicole_builtin_usb),

	{ .etype = st_end },
};

static bool nicole_probe(void)
{
	if (!dt_node_is_compatible(dt_root, "YADRO,nicole"))
		return false;

	/* Lot of common early inits here */
	astbmc_early_init();

	/* Setup UART for use by OPAL (Linux hvc) */
	uart_set_console_policy(UART_CONSOLE_OPAL);

	slot_table_init(nicole_phb_table);

	return true;
}

DECLARE_PLATFORM(nicole) = {
	.name			= "Nicole",
	.probe			= nicole_probe,
	.init			= astbmc_init,
	.start_preload_resource	= flash_start_preload_resource,
	.resource_loaded	= flash_resource_loaded,
	.bmc			= &bmc_plat_ast2500_openbmc,
	.pci_get_slot_info	= slot_table_get_slot_info,
	.pci_probe_complete	= check_all_slot_table,
	.cec_power_down         = astbmc_ipmi_power_down,
	.cec_reboot             = astbmc_ipmi_reboot,
	.elog_commit		= ipmi_elog_commit,
	.exit			= astbmc_exit,
	.terminate		= ipmi_terminate,
};
