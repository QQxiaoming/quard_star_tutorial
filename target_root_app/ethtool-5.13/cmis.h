#ifndef CMIS_H__
#define CMIS_H__

/* Identifier and revision compliance (Page 0) */
#define CMIS_ID_OFFSET				0x00
#define CMIS_REV_COMPLIANCE_OFFSET		0x01

#define CMIS_MODULE_TYPE_OFFSET			0x55
#define CMIS_MT_MMF				0x01
#define CMIS_MT_SMF				0x02

/* Module-Level Monitors (Page 0) */
#define CMIS_CURR_TEMP_OFFSET			0x0E
#define CMIS_CURR_CURR_OFFSET			0x10

#define CMIS_CTOR_OFFSET			0xCB

/* Vendor related information (Page 0) */
#define CMIS_VENDOR_NAME_START_OFFSET		0x81
#define CMIS_VENDOR_NAME_END_OFFSET		0x90

#define CMIS_VENDOR_OUI_OFFSET			0x91

#define CMIS_VENDOR_PN_START_OFFSET		0x94
#define CMIS_VENDOR_PN_END_OFFSET		0xA3

#define CMIS_VENDOR_REV_START_OFFSET		0xA4
#define CMIS_VENDOR_REV_END_OFFSET		0xA5

#define CMIS_VENDOR_SN_START_OFFSET		0xA6
#define CMIS_VENDOR_SN_END_OFFSET		0xB5

#define CMIS_DATE_YEAR_OFFSET			0xB6
#define CMIS_DATE_VENDOR_LOT_OFFSET		0xBC

/* CLEI Code (Page 0) */
#define CMIS_CLEI_PRESENT_BYTE			0x02
#define CMIS_CLEI_PRESENT_MASK			0x20
#define CMIS_CLEI_START_OFFSET			0xBE
#define CMIS_CLEI_END_OFFSET			0xC7

/* Cable assembly length */
#define CMIS_CBL_ASM_LEN_OFFSET			0xCA
#define CMIS_6300M_MAX_LEN			0xFF

/* Cable length with multiplier */
#define CMIS_MULTIPLIER_00			0x00
#define CMIS_MULTIPLIER_01			0x40
#define CMIS_MULTIPLIER_10			0x80
#define CMIS_MULTIPLIER_11			0xC0
#define CMIS_LEN_MUL_MASK			0xC0
#define CMIS_LEN_VAL_MASK			0x3F

/* Module power characteristics */
#define CMIS_PWR_CLASS_OFFSET			0xC8
#define CMIS_PWR_MAX_POWER_OFFSET		0xC9
#define CMIS_PWR_CLASS_MASK			0xE0
#define CMIS_PWR_CLASS_1			0x00
#define CMIS_PWR_CLASS_2			0x01
#define CMIS_PWR_CLASS_3			0x02
#define CMIS_PWR_CLASS_4			0x03
#define CMIS_PWR_CLASS_5			0x04
#define CMIS_PWR_CLASS_6			0x05
#define CMIS_PWR_CLASS_7			0x06
#define CMIS_PWR_CLASS_8			0x07

/* Copper cable attenuation */
#define CMIS_COPPER_ATT_5GHZ			0xCC
#define CMIS_COPPER_ATT_7GHZ			0xCD
#define CMIS_COPPER_ATT_12P9GHZ			0xCE
#define CMIS_COPPER_ATT_25P8GHZ			0xCF

/* Cable assembly lane */
#define CMIS_CABLE_ASM_NEAR_END_OFFSET		0xD2
#define CMIS_CABLE_ASM_FAR_END_OFFSET		0xD3

/* Media interface technology */
#define CMIS_MEDIA_INTF_TECH_OFFSET		0xD4
#define CMIS_850_VCSEL				0x00
#define CMIS_1310_VCSEL				0x01
#define CMIS_1550_VCSEL				0x02
#define CMIS_1310_FP				0x03
#define CMIS_1310_DFB				0x04
#define CMIS_1550_DFB				0x05
#define CMIS_1310_EML				0x06
#define CMIS_1550_EML				0x07
#define CMIS_OTHERS				0x08
#define CMIS_1490_DFB				0x09
#define CMIS_COPPER_UNEQUAL			0x0A
#define CMIS_COPPER_PASS_EQUAL			0x0B
#define CMIS_COPPER_NF_EQUAL			0x0C
#define CMIS_COPPER_F_EQUAL			0x0D
#define CMIS_COPPER_N_EQUAL			0x0E
#define CMIS_COPPER_LINEAR_EQUAL		0x0F

/*-----------------------------------------------------------------------
 * Upper Memory Page 0x01: contains advertising fields that define properties
 * that are unique to active modules and cable assemblies.
 * GlobalOffset = 2 * 0x80 + LocalOffset
 */
#define PAG01H_UPPER_OFFSET			(0x02 * 0x80)

/* Supported Link Length (Page 1) */
#define CMIS_SMF_LEN_OFFSET			0x84
#define CMIS_OM5_LEN_OFFSET			0x85
#define CMIS_OM4_LEN_OFFSET			0x86
#define CMIS_OM3_LEN_OFFSET			0x87
#define CMIS_OM2_LEN_OFFSET			0x88

/* Wavelength (Page 1) */
#define CMIS_NOM_WAVELENGTH_MSB			0x8A
#define CMIS_NOM_WAVELENGTH_LSB			0x8B
#define CMIS_WAVELENGTH_TOL_MSB			0x8C
#define CMIS_WAVELENGTH_TOL_LSB			0x8D

/* Signal integrity controls */
#define CMIS_SIG_INTEG_TX_OFFSET		0xA1
#define CMIS_SIG_INTEG_RX_OFFSET		0xA2

#define YESNO(x) (((x) != 0) ? "Yes" : "No")
#define ONOFF(x) (((x) != 0) ? "On" : "Off")

void qsfp_dd_show_all(const __u8 *id);

void cmis_show_all(const struct ethtool_module_eeprom *page_zero,
		   const struct ethtool_module_eeprom *page_one);

#endif /* CMIS_H__ */
