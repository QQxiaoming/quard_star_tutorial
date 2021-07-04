#include <assert.h>
#include <stdint.h>
#include <compiler.h>

/* Stubs for quirk_astbmc_vga() */

struct dt_property;
struct dt_node;

static struct bmc_platform fake_bmc;
const struct bmc_platform *bmc_platform = &fake_bmc;

static int ast_sio_is_enabled(void)
{
	return 0;
}

static uint32_t ast_ahb_readl(uint32_t reg)
{
	return reg;
}

static struct dt_property *__dt_add_property_cells(
		struct dt_node *node __unused, const char *name __unused,
		int count __unused, ...)
{
	return (void *)0;
}

#include "../pci-quirk.c"

struct pci_device test_pd;
int test_fixup_ran;

static void test_fixup(struct phb *phb __unused, struct pci_device *pd __unused)
{
	assert(PCI_VENDOR_ID(pd->vdid) == 0x1a03);
	assert(PCI_DEVICE_ID(pd->vdid) == 0x2000);
	test_fixup_ran = 1;
}

/* Quirks are: {fixup function, vendor ID, (device ID or PCI_ANY_ID)} */
static const struct pci_quirk test_quirk_table[] = {
	/* ASPEED 2400 VGA device */
	{ &test_fixup, 0x1a03, 0x2000 },
	{ NULL, 0, 0 }
};

#define PCI_COMPOSE_VDID(vendor, device) (((device) << 16) | (vendor))

int main(void)
{
	/* Unrecognised vendor and device ID */
	test_pd.vdid = PCI_COMPOSE_VDID(0xabcd, 0xef01);
	__pci_handle_quirk(NULL, &test_pd, test_quirk_table);
	assert(test_fixup_ran == 0);

	/* Unrecognised vendor ID, matching device ID */
	test_pd.vdid = PCI_COMPOSE_VDID(0xabcd, 0x2000);
	__pci_handle_quirk(NULL, &test_pd, test_quirk_table);
	assert(test_fixup_ran == 0);

	/* Matching vendor ID, unrecognised device ID */
	test_pd.vdid = PCI_COMPOSE_VDID(0x1a03, 0xef01);
	__pci_handle_quirk(NULL, &test_pd, test_quirk_table);
	assert(test_fixup_ran == 0);

	/* Matching vendor and device ID */
	test_pd.vdid = PCI_COMPOSE_VDID(0x1a03, 0x2000);
	__pci_handle_quirk(NULL, &test_pd, test_quirk_table);
	assert(test_fixup_ran == 1);

	return 0;
}
