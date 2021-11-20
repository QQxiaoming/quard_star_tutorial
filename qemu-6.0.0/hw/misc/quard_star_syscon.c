/*
 * QEMU RISC-V Quard Star Board system control
 *
 * Copyright (c) 2021 qiao qiming
 *
 * system control memory mapped device used to exit simulation
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

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/qdev-properties.h"
#include "sysemu/runstate.h"
#include "hw/hw.h"
#include "hw/misc/quard_star_syscon.h"

#define QUARD_STAR_SYSCON_VERSION (0x00010000)

#define QUARD_STAR_SYSCON_PFLASH_BOOT (0x1<<0)
#define QUARD_STAR_SYSCON_SPI_BOOT    (0x1<<1)
#define QUARD_STAR_SYSCON_SD_BOOT     (0x1<<2)
#define QUARD_STAR_SYSCON_UART_UPDATE (0x1<<16)

enum {
    FINISHER_FAIL = 0x3333,
    FINISHER_PASS = 0x5555,
    FINISHER_RESET = 0x7777
};

enum {
    CONTROL_REG   = 0x0,
    BOOT_REG      = 0x4,
    USER_BOOT_REG = 0x8,
    VERSION_REG   = 0xc
};

static uint64_t quard_star_syscon_bootreg = QUARD_STAR_SYSCON_PFLASH_BOOT | QUARD_STAR_SYSCON_UART_UPDATE;
static uint64_t quard_star_syscon_userbootreg = 0;

static uint64_t quard_star_syscon_read(void *opaque, hwaddr addr, unsigned int size)
{
    switch (addr)
    {
    case CONTROL_REG:
        return 0;
    case BOOT_REG:
        return quard_star_syscon_bootreg;    
    case USER_BOOT_REG:
        return quard_star_syscon_userbootreg;    
    case VERSION_REG:
        return QUARD_STAR_SYSCON_VERSION;
    default:
        break;
    }
    qemu_log_mask(LOG_GUEST_ERROR, "%s: read: addr=0x%x\n", __func__, (int)addr);
    return 0;
}

static void quard_star_syscon_write(void *opaque, hwaddr addr,
           uint64_t val64, unsigned int size)
{
    if (addr == CONTROL_REG) {
        int status = val64 & 0xffff;
        int code = (val64 >> 16) & 0xffff;
        switch (status) {
        case FINISHER_FAIL:
            exit(code);
        case FINISHER_PASS:
            exit(0);
        case FINISHER_RESET:
            qemu_system_reset_request(SHUTDOWN_CAUSE_GUEST_RESET);
            return;
        default:
            break;
        }
    } else if(addr == USER_BOOT_REG) {
        quard_star_syscon_userbootreg = val64;
    }
    qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x val=0x%016" PRIx64 "\n",
                  __func__, (int)addr, val64);
}

static Property quard_star_syscon_properties[] = {
    DEFINE_PROP_STRING("boot-cfg", QuardStarSysconState, boot_cfg),
    DEFINE_PROP_BOOL("update-cfg", QuardStarSysconState, update_cfg, true),
    DEFINE_PROP_END_OF_LIST(),
};

static const MemoryRegionOps quard_star_syscon_ops = {
    .read = quard_star_syscon_read,
    .write = quard_star_syscon_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 2,
        .max_access_size = 4
    }
};

static void quard_star_syscon_realize(DeviceState *dev, Error **errp)
{
    QuardStarSysconState *s = QUARD_STAR_SYSCON(dev);

    if(s->boot_cfg){
        if (!strcmp(s->boot_cfg, "pflash")) {
            quard_star_syscon_bootreg = QUARD_STAR_SYSCON_PFLASH_BOOT;
        } else if (!strcmp(s->boot_cfg, "spi")) {
            quard_star_syscon_bootreg = QUARD_STAR_SYSCON_SPI_BOOT;
        } else if (!strcmp(s->boot_cfg, "sd")) {
            quard_star_syscon_bootreg = QUARD_STAR_SYSCON_SD_BOOT;
        }
    }
    if(s->update_cfg) {
        quard_star_syscon_bootreg |= QUARD_STAR_SYSCON_UART_UPDATE;
    } else {
        quard_star_syscon_bootreg &= ~QUARD_STAR_SYSCON_UART_UPDATE;
    }
}

static void quard_star_syscon_init(Object *obj)
{
    QuardStarSysconState *s = QUARD_STAR_SYSCON(obj);

    memory_region_init_io(&s->mmio, obj, &quard_star_syscon_ops, s,
                          TYPE_QUARD_STAR_SYSCON, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);
}

static void quard_star_syscon_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = quard_star_syscon_realize;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
    device_class_set_props(dc, quard_star_syscon_properties);
}

static const TypeInfo quard_star_syscon_info = {
    .name          = TYPE_QUARD_STAR_SYSCON,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(QuardStarSysconState),
    .instance_init = quard_star_syscon_init,
    .class_init    = quard_star_syscon_class_init,
};

static void quard_star_syscon_register_types(void)
{
    type_register_static(&quard_star_syscon_info);
}

type_init(quard_star_syscon_register_types)


/*
 * Create Test device.
 */
DeviceState *quard_star_syscon_create(hwaddr addr)
{
    DeviceState *dev = qdev_new(TYPE_QUARD_STAR_SYSCON);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
    return dev;
}
