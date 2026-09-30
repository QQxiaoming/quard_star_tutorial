/*
 * Minimal RISC-V Niubi board
 *
 * Copyright (c) 2026
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This board deliberately contains only one hart and RAM.  It is intended as
 * a small example of the minimum useful QEMU RISC-V machine.
 */

#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/qdev-properties.h"
#include "hw/riscv/boot.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/sysbus.h"
#include "exec/address-spaces.h"
#include "target/riscv/cpu.h"

#define NIUBI_RAM_BASE 0x80000000

static void niubi_board_init(MachineState *machine)
{
    MemoryRegion *system_memory = get_system_memory();
    MemoryRegion *ram = g_new(MemoryRegion, 1);
    DeviceState *harts = qdev_new(TYPE_RISCV_HART_ARRAY);
    target_ulong resetvec = NIUBI_RAM_BASE;

    memory_region_init_ram(ram, NULL, "niubi.ram", machine->ram_size,
                           &error_fatal);
    memory_region_add_subregion(system_memory, NIUBI_RAM_BASE, ram);

    /* ELF, uImage and raw images are all accepted by this helper. */
    if (machine->kernel_filename) {
        resetvec = riscv_load_kernel(machine->kernel_filename,
                                     NIUBI_RAM_BASE, NULL);
    }

    qdev_prop_set_string(harts, "cpu-type", machine->cpu_type);
    qdev_prop_set_uint32(harts, "num-harts", 1);
    qdev_prop_set_uint64(harts, "resetvec", resetvec);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(harts), &error_fatal);
}

static void niubi_machine_init(MachineClass *mc)
{
    mc->desc = "RISC-V Niubi board";
    mc->init = niubi_board_init;
    mc->max_cpus = 1;
    mc->default_cpu_type = TYPE_RISCV_CPU_BASE;
    mc->default_ram_size = 128 * MiB;
    mc->no_serial = true;
    mc->no_parallel = true;
    mc->no_floppy = true;
    mc->no_cdrom = true;
    mc->no_sdcard = true;
}

DEFINE_MACHINE("niubi", niubi_machine_init)
