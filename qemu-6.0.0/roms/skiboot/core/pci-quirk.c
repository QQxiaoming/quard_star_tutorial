/* Copyright 2017 IBM Corp.
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

#define pr_fmt(fmt)  "PCI-QUIRK: " fmt

#include <skiboot.h>
#include <pci.h>
#include <pci-quirk.h>
#include <platform.h>
#include <ast.h>

static void quirk_astbmc_vga(struct phb *phb __unused,
			     struct pci_device *pd)
{
	struct dt_node *np = pd->dn;
	uint32_t revision, mcr_configuration, mcr_scu_mpll, mcr_scu_strap;

	if (ast_sio_is_enabled()) {
		revision = ast_ahb_readl(SCU_REVISION_ID);
		mcr_configuration = ast_ahb_readl(MCR_CONFIGURATION);
		mcr_scu_mpll = ast_ahb_readl(MCR_SCU_MPLL);
		mcr_scu_strap = ast_ahb_readl(MCR_SCU_STRAP);
	} else {
		/* Previously we would warn, now SIO disabled by design */
		prlog(PR_INFO, "Assumed platform default parameters for %s\n",
		      __func__);
		revision = bmc_platform->hw->scu_revision_id;
		mcr_configuration = bmc_platform->hw->mcr_configuration;
		mcr_scu_mpll = bmc_platform->hw->mcr_scu_mpll;
		mcr_scu_strap = bmc_platform->hw->mcr_scu_strap;
	}

	dt_add_property_cells(np, "aspeed,scu-revision-id", revision);
	dt_add_property_cells(np, "aspeed,mcr-configuration", mcr_configuration);
	dt_add_property_cells(np, "aspeed,mcr-scu-mpll", mcr_scu_mpll);
	dt_add_property_cells(np, "aspeed,mcr-scu-strap", mcr_scu_strap);
}

/* Quirks are: {fixup function, vendor ID, (device ID or PCI_ANY_ID)} */
static const struct pci_quirk quirk_table[] = {
	/* ASPEED 2400 VGA device */
	{ &quirk_astbmc_vga, 0x1a03, 0x2000 },
	{ NULL, 0, 0 }
};

static void __pci_handle_quirk(struct phb *phb, struct pci_device *pd,
			       const struct pci_quirk *quirks)
{
	while (quirks->vendor_id) {
		if (quirks->vendor_id == PCI_VENDOR_ID(pd->vdid) &&
		    (quirks->device_id == PCI_ANY_ID ||
		     quirks->device_id == PCI_DEVICE_ID(pd->vdid)))
			quirks->fixup(phb, pd);
		quirks++;
	}
}

void pci_handle_quirk(struct phb *phb, struct pci_device *pd)
{
	__pci_handle_quirk(phb, pd, quirk_table);
}
