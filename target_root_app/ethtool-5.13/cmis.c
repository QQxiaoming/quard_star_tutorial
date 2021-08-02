/**
 * Description:
 *
 * This module adds QSFP-DD support to ethtool. The changes are similar to
 * the ones already existing in qsfp.c, but customized to use the memory
 * addresses and logic as defined in the specification's document.
 *
 */

#include <stdio.h>
#include <math.h>
#include "internal.h"
#include "sff-common.h"
#include "cmis.h"

static void cmis_show_identifier(const __u8 *id)
{
	sff8024_show_identifier(id, CMIS_ID_OFFSET);
}

static void cmis_show_connector(const __u8 *id)
{
	sff8024_show_connector(id, CMIS_CTOR_OFFSET);
}

static void cmis_show_oui(const __u8 *id)
{
	sff8024_show_oui(id, CMIS_VENDOR_OUI_OFFSET);
}

/**
 * Print the revision compliance. Relevant documents:
 * [1] CMIS Rev. 3, pag. 45, section 1.7.2.1, Table 18
 * [2] CMIS Rev. 4, pag. 81, section 8.2.1, Table 8-2
 */
static void cmis_show_rev_compliance(const __u8 *id)
{
	__u8 rev = id[CMIS_REV_COMPLIANCE_OFFSET];
	int major = (rev >> 4) & 0x0F;
	int minor = rev & 0x0F;

	printf("\t%-41s : Rev. %d.%d\n", "Revision compliance", major, minor);
}

/**
 * Print information about the device's power consumption.
 * Relevant documents:
 * [1] CMIS Rev. 3, pag. 59, section 1.7.3.9, Table 30
 * [2] CMIS Rev. 4, pag. 94, section 8.3.9, Table 8-18
 * [3] QSFP-DD Hardware Rev 5.0, pag. 22, section 4.2.1
 */
static void cmis_show_power_info(const __u8 *id)
{
	float max_power = 0.0f;
	__u8 base_power = 0;
	__u8 power_class;

	/* Get the power class (first 3 most significat bytes) */
	power_class = (id[CMIS_PWR_CLASS_OFFSET] >> 5) & 0x07;

	/* Get the base power in multiples of 0.25W */
	base_power = id[CMIS_PWR_MAX_POWER_OFFSET];
	max_power = base_power * 0.25f;

	printf("\t%-41s : %d\n", "Power class", power_class + 1);
	printf("\t%-41s : %.02fW\n", "Max power", max_power);
}

/**
 * Print the cable assembly length, for both passive copper and active
 * optical or electrical cables. The base length (bits 5-0) must be
 * multiplied with the SMF length multiplier (bits 7-6) to obtain the
 * correct value. Relevant documents:
 * [1] CMIS Rev. 3, pag. 59, section 1.7.3.10, Table 31
 * [2] CMIS Rev. 4, pag. 94, section 8.3.10, Table 8-19
 */
static void cmis_show_cbl_asm_len(const __u8 *id)
{
	static const char *fn = "Cable assembly length";
	float mul = 1.0f;
	float val = 0.0f;

	/* Check if max length */
	if (id[CMIS_CBL_ASM_LEN_OFFSET] == CMIS_6300M_MAX_LEN) {
		printf("\t%-41s : > 6.3km\n", fn);
		return;
	}

	/* Get the multiplier from the first two bits */
	switch (id[CMIS_CBL_ASM_LEN_OFFSET] & CMIS_LEN_MUL_MASK) {
	case CMIS_MULTIPLIER_00:
		mul = 0.1f;
		break;
	case CMIS_MULTIPLIER_01:
		mul = 1.0f;
		break;
	case CMIS_MULTIPLIER_10:
		mul = 10.0f;
		break;
	case CMIS_MULTIPLIER_11:
		mul = 100.0f;
		break;
	default:
		break;
	}

	/* Get base value from first 6 bits and multiply by mul */
	val = (id[CMIS_CBL_ASM_LEN_OFFSET] & CMIS_LEN_VAL_MASK);
	val = (float)val * mul;
	printf("\t%-41s : %0.2fm\n", fn, val);
}

/**
 * Print the length for SMF fiber. The base length (bits 5-0) must be
 * multiplied with the SMF length multiplier (bits 7-6) to obtain the
 * correct value. Relevant documents:
 * [1] CMIS Rev. 3, pag. 63, section 1.7.4.2, Table 39
 * [2] CMIS Rev. 4, pag. 99, section 8.4.2, Table 8-27
 */
static void cmis_print_smf_cbl_len(const __u8 *id)
{
	static const char *fn = "Length (SMF)";
	float mul = 1.0f;
	float val = 0.0f;

	/* Get the multiplier from the first two bits */
	switch (id[CMIS_SMF_LEN_OFFSET] & CMIS_LEN_MUL_MASK) {
	case CMIS_MULTIPLIER_00:
		mul = 0.1f;
		break;
	case CMIS_MULTIPLIER_01:
		mul = 1.0f;
		break;
	default:
		break;
	}

	/* Get base value from first 6 bits and multiply by mul */
	val = (id[CMIS_SMF_LEN_OFFSET] & CMIS_LEN_VAL_MASK);
	val = (float)val * mul;
	printf("\t%-41s : %0.2fkm\n", fn, val);
}

/**
 * Print relevant signal integrity control properties. Relevant documents:
 * [1] CMIS Rev. 3, pag. 71, section 1.7.4.10, Table 46
 * [2] CMIS Rev. 4, pag. 105, section 8.4.10, Table 8-34
 */
static void cmis_show_sig_integrity(const __u8 *id)
{
	/* CDR Bypass control: 2nd bit from each byte */
	printf("\t%-41s : ", "Tx CDR bypass control");
	printf("%s\n", YESNO(id[CMIS_SIG_INTEG_TX_OFFSET] & 0x02));

	printf("\t%-41s : ", "Rx CDR bypass control");
	printf("%s\n", YESNO(id[CMIS_SIG_INTEG_RX_OFFSET] & 0x02));

	/* CDR Implementation: 1st bit from each byte */
	printf("\t%-41s : ", "Tx CDR");
	printf("%s\n", YESNO(id[CMIS_SIG_INTEG_TX_OFFSET] & 0x01));

	printf("\t%-41s : ", "Rx CDR");
	printf("%s\n", YESNO(id[CMIS_SIG_INTEG_RX_OFFSET] & 0x01));
}

/**
 * Print relevant media interface technology info. Relevant documents:
 * [1] CMIS Rev. 3
 * --> pag. 61, section 1.7.3.14, Table 36
 * --> pag. 64, section 1.7.4.3, 1.7.4.4
 * [2] CMIS Rev. 4
 * --> pag. 97, section 8.3.14, Table 8-24
 * --> pag. 98, section 8.4, Table 8-25
 * --> page 100, section 8.4.3, 8.4.4
 */
static void cmis_show_mit_compliance(const __u8 *id)
{
	static const char *cc = " (Copper cable,";

	printf("\t%-41s : 0x%02x", "Transmitter technology",
	       id[CMIS_MEDIA_INTF_TECH_OFFSET]);

	switch (id[CMIS_MEDIA_INTF_TECH_OFFSET]) {
	case CMIS_850_VCSEL:
		printf(" (850 nm VCSEL)\n");
		break;
	case CMIS_1310_VCSEL:
		printf(" (1310 nm VCSEL)\n");
		break;
	case CMIS_1550_VCSEL:
		printf(" (1550 nm VCSEL)\n");
		break;
	case CMIS_1310_FP:
		printf(" (1310 nm FP)\n");
		break;
	case CMIS_1310_DFB:
		printf(" (1310 nm DFB)\n");
		break;
	case CMIS_1550_DFB:
		printf(" (1550 nm DFB)\n");
		break;
	case CMIS_1310_EML:
		printf(" (1310 nm EML)\n");
		break;
	case CMIS_1550_EML:
		printf(" (1550 nm EML)\n");
		break;
	case CMIS_OTHERS:
		printf(" (Others/Undefined)\n");
		break;
	case CMIS_1490_DFB:
		printf(" (1490 nm DFB)\n");
		break;
	case CMIS_COPPER_UNEQUAL:
		printf("%s unequalized)\n", cc);
		break;
	case CMIS_COPPER_PASS_EQUAL:
		printf("%s passive equalized)\n", cc);
		break;
	case CMIS_COPPER_NF_EQUAL:
		printf("%s near and far end limiting active equalizers)\n", cc);
		break;
	case CMIS_COPPER_F_EQUAL:
		printf("%s far end limiting active equalizers)\n", cc);
		break;
	case CMIS_COPPER_N_EQUAL:
		printf("%s near end limiting active equalizers)\n", cc);
		break;
	case CMIS_COPPER_LINEAR_EQUAL:
		printf("%s linear active equalizers)\n", cc);
		break;
	}

	if (id[CMIS_MEDIA_INTF_TECH_OFFSET] >= CMIS_COPPER_UNEQUAL) {
		printf("\t%-41s : %udb\n", "Attenuation at 5GHz",
		       id[CMIS_COPPER_ATT_5GHZ]);
		printf("\t%-41s : %udb\n", "Attenuation at 7GHz",
		       id[CMIS_COPPER_ATT_7GHZ]);
		printf("\t%-41s : %udb\n", "Attenuation at 12.9GHz",
		       id[CMIS_COPPER_ATT_12P9GHZ]);
		printf("\t%-41s : %udb\n", "Attenuation at 25.8GHz",
		       id[CMIS_COPPER_ATT_25P8GHZ]);
	} else {
		printf("\t%-41s : %.3lfnm\n", "Laser wavelength",
		       (((id[CMIS_NOM_WAVELENGTH_MSB] << 8) |
				id[CMIS_NOM_WAVELENGTH_LSB]) * 0.05));
		printf("\t%-41s : %.3lfnm\n", "Laser wavelength tolerance",
		       (((id[CMIS_WAVELENGTH_TOL_MSB] << 8) |
		       id[CMIS_WAVELENGTH_TOL_LSB]) * 0.005));
	}
}

/*
 * 2-byte internal temperature conversions:
 * First byte is a signed 8-bit integer, which is the temp decimal part
 * Second byte is a multiple of 1/256th of a degree, which is added to
 * the dec part.
 */
#define OFFSET_TO_TEMP(offset) ((__s16)OFFSET_TO_U16(offset))

/**
 * Print relevant module level monitoring values. Relevant documents:
 * [1] CMIS Rev. 3:
 * --> pag. 50, section 1.7.2.4, Table 22
 *
 * [2] CMIS Rev. 4:
 * --> pag. 84, section 8.2.4, Table 8-6
 */
static void cmis_show_mod_lvl_monitors(const __u8 *id)
{
	PRINT_TEMP("Module temperature",
		   OFFSET_TO_TEMP(CMIS_CURR_TEMP_OFFSET));
	PRINT_VCC("Module voltage",
		  OFFSET_TO_U16(CMIS_CURR_CURR_OFFSET));
}

static void cmis_show_link_len_from_page(const __u8 *page_one_data)
{
	cmis_print_smf_cbl_len(page_one_data);
	sff_show_value_with_unit(page_one_data, CMIS_OM5_LEN_OFFSET,
				 "Length (OM5)", 2, "m");
	sff_show_value_with_unit(page_one_data, CMIS_OM4_LEN_OFFSET,
				 "Length (OM4)", 2, "m");
	sff_show_value_with_unit(page_one_data, CMIS_OM3_LEN_OFFSET,
				 "Length (OM3 50/125um)", 2, "m");
	sff_show_value_with_unit(page_one_data, CMIS_OM2_LEN_OFFSET,
				 "Length (OM2 50/125um)", 1, "m");
}


/**
 * Print relevant info about the maximum supported fiber media length
 * for each type of fiber media at the maximum module-supported bit rate.
 * Relevant documents:
 * [1] CMIS Rev. 3, page 64, section 1.7.4.2, Table 39
 * [2] CMIS Rev. 4, page 99, section 8.4.2, Table 8-27
 */
static void cmis_show_link_len(const __u8 *id)
{
	cmis_show_link_len_from_page(id + PAG01H_UPPER_OFFSET);
}

/**
 * Show relevant information about the vendor. Relevant documents:
 * [1] CMIS Rev. 3, page 56, section 1.7.3, Table 27
 * [2] CMIS Rev. 4, page 91, section 8.2, Table 8-15
 */
static void cmis_show_vendor_info(const __u8 *id)
{
	sff_show_ascii(id, CMIS_VENDOR_NAME_START_OFFSET,
		       CMIS_VENDOR_NAME_END_OFFSET, "Vendor name");
	cmis_show_oui(id);
	sff_show_ascii(id, CMIS_VENDOR_PN_START_OFFSET,
		       CMIS_VENDOR_PN_END_OFFSET, "Vendor PN");
	sff_show_ascii(id, CMIS_VENDOR_REV_START_OFFSET,
		       CMIS_VENDOR_REV_END_OFFSET, "Vendor rev");
	sff_show_ascii(id, CMIS_VENDOR_SN_START_OFFSET,
		       CMIS_VENDOR_SN_END_OFFSET, "Vendor SN");
	sff_show_ascii(id, CMIS_DATE_YEAR_OFFSET,
		       CMIS_DATE_VENDOR_LOT_OFFSET + 1, "Date code");

	if (id[CMIS_CLEI_PRESENT_BYTE] & CMIS_CLEI_PRESENT_MASK)
		sff_show_ascii(id, CMIS_CLEI_START_OFFSET,
			       CMIS_CLEI_END_OFFSET, "CLEI code");
}

void qsfp_dd_show_all(const __u8 *id)
{
	cmis_show_identifier(id);
	cmis_show_power_info(id);
	cmis_show_connector(id);
	cmis_show_cbl_asm_len(id);
	cmis_show_sig_integrity(id);
	cmis_show_mit_compliance(id);
	cmis_show_mod_lvl_monitors(id);
	cmis_show_link_len(id);
	cmis_show_vendor_info(id);
	cmis_show_rev_compliance(id);
}

void cmis_show_all(const struct ethtool_module_eeprom *page_zero,
		   const struct ethtool_module_eeprom *page_one)
{
	const __u8 *page_zero_data = page_zero->data;

	cmis_show_identifier(page_zero_data);
	cmis_show_power_info(page_zero_data);
	cmis_show_connector(page_zero_data);
	cmis_show_cbl_asm_len(page_zero_data);
	cmis_show_sig_integrity(page_zero_data);
	cmis_show_mit_compliance(page_zero_data);
	cmis_show_mod_lvl_monitors(page_zero_data);

	if (page_one)
		cmis_show_link_len_from_page(page_one->data - 0x80);

	cmis_show_vendor_info(page_zero_data);
	cmis_show_rev_compliance(page_zero_data);
}
