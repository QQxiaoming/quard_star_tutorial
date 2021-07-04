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

#ifndef __PHYS_MAP_H
#define __PHYS_MAP_H

#include <compiler.h>
#include <stdint.h>
#include <processor.h>
#include <ccan/endian/endian.h>
#include <chip.h>

enum phys_map_type {
	NULL_MAP,
	SYSTEM_MEM,
	GPU_MEM_4T_DOWN,
	GPU_MEM_4T_UP,
	OCAPI_MEM,
	PHB4_64BIT_MMIO,
	PHB4_32BIT_MMIO,
	PHB4_XIVE_ESB,
	PHB4_REG_SPC,
	NPU_OCAPI_MMIO,
	XIVE_VC,
	XIVE_PC,
	VAS_USER_WIN,
	VAS_HYP_WIN,
	OCAB_XIVE_ESB,
	LPC_BUS,
	FSP_MMIO,
	NPU_REGS,
	NPU_USR,
	NPU_PHY,
	NPU_NTL,
	NPU_GENID,
	PSIHB_REG,
	XIVE_IC,
	XIVE_TM,
	PSIHB_ESB,
	NX_RNG,
	CENTAUR_SCOM,
	XSCOM,
	RESV
};

extern void phys_map_get(uint64_t gcid, enum phys_map_type type,
			 int index, uint64_t *addr, uint64_t *size);

extern void phys_map_init(void);

#endif /* __PHYS_MAP_H */

//TODO self test overlaps and alignemnt and size.
