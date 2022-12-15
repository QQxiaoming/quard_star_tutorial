/*
 * QEMU RISC-V Quard Star Board system control
 *
 * Copyright (c) 2021-2022 qiao qiming <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HW_QUARD_STAR_SYSCON_H
#define HW_QUARD_STAR_SYSCON_H

#include "hw/sysbus.h"
#include "qom/object.h"

#define TYPE_QUARD_STAR_SYSCON "quard-star-syscon"

typedef struct QuardStarSysconState QuardStarSysconState;
DECLARE_INSTANCE_CHECKER(QuardStarSysconState, QUARD_STAR_SYSCON,
                         TYPE_QUARD_STAR_SYSCON)

struct QuardStarSysconState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion mmio;
    char *boot_cfg;
    bool update_cfg;
};

DeviceState *quard_star_syscon_create(hwaddr addr);

#endif
