/*
 * QEMU RISC-V Quard Star Board
 *
 * Copyright (c) 2021 qiao qiming
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
#include "qemu/units.h"
#include "qemu/log.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "hw/qdev-properties.h"
#include "hw/char/serial.h"
#include "target/riscv/cpu.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/quard_star.h"
#include "hw/riscv/boot.h"
#include "hw/riscv/numa.h"
#include "hw/intc/sifive_clint.h"
#include "hw/intc/sifive_plic.h"
#include "hw/misc/sifive_test.h"
#include "chardev/char.h"
#include "sysemu/arch_init.h"
#include "sysemu/device_tree.h"
#include "sysemu/sysemu.h"

static const MemMapEntry virt_memmap[] = {
    [QUARD_STAR_MROM]   = {        0x0,        0x8000 },
    [QUARD_STAR_SRAM]   = {     0x8000,        0x8000 },
    [QUARD_STAR_TEST]   = {   0x100000,        0x1000 },
    [QUARD_STAR_CLINT]  = {  0x2000000,       0x10000 },
    [QUARD_STAR_PLIC]   = {  0xc000000,     0x4000000 },
    [QUARD_STAR_UART0]  = { 0x10000000,        0x1000 },
    [QUARD_STAR_UART1]  = { 0x10001000,        0x1000 },
    [QUARD_STAR_UART2]  = { 0x10002000,        0x1000 },
    [QUARD_STAR_RTC]    = { 0x10003000,        0x1000 },
    [QUARD_STAR_I2C0]   = { 0x10004000,        0x1000 },
    [QUARD_STAR_I2C1]   = { 0x10005000,        0x1000 },
    [QUARD_STAR_I2C2]   = { 0x10006000,        0x1000 },
    [QUARD_STAR_VIRTIO] = { 0x10100000,        0x1000 }, //Eight consecutive groups
    [QUARD_STAR_FW_CFG] = { 0x10108000,          0x18 },
    [QUARD_STAR_FLASH]  = { 0x20000000,     0x2000000 },
    [QUARD_STAR_DRAM]   = { 0x80000000,           0x0 },
};

#define QUARD_STAR_FLASH_SECTOR_SIZE (256 * KiB)

static PFlashCFI01 *quard_star_flash_create(QuardStarState *s,
                                       const char *name,
                                       const char *alias_prop_name)
{
    DeviceState *dev = qdev_new(TYPE_PFLASH_CFI01);

    qdev_prop_set_uint64(dev, "sector-length", QUARD_STAR_FLASH_SECTOR_SIZE);
    qdev_prop_set_uint8(dev, "width", 4);
    qdev_prop_set_uint8(dev, "device-width", 2);
    qdev_prop_set_bit(dev, "big-endian", false);
    qdev_prop_set_uint16(dev, "id0", 0x89);
    qdev_prop_set_uint16(dev, "id1", 0x18);
    qdev_prop_set_uint16(dev, "id2", 0x00);
    qdev_prop_set_uint16(dev, "id3", 0x00);
    qdev_prop_set_string(dev, "name", name);

    object_property_add_child(OBJECT(s), name, OBJECT(dev));
    object_property_add_alias(OBJECT(s), alias_prop_name,
                              OBJECT(dev), "drive");

    return PFLASH_CFI01(dev);
}

static void quard_star_flash_map(PFlashCFI01 *flash,
                            hwaddr base, hwaddr size,
                            MemoryRegion *sysmem)
{
    DeviceState *dev = DEVICE(flash);

    assert(QEMU_IS_ALIGNED(size, QUARD_STAR_FLASH_SECTOR_SIZE));
    assert(size / QUARD_STAR_FLASH_SECTOR_SIZE <= UINT32_MAX);
    qdev_prop_set_uint32(dev, "num-blocks", size / QUARD_STAR_FLASH_SECTOR_SIZE);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);

    memory_region_add_subregion(sysmem, base,
                                sysbus_mmio_get_region(SYS_BUS_DEVICE(dev),
                                                       0));
}

static void quard_star_setup_rom_reset_vec(MachineState *machine, RISCVHartArrayState *harts,
                               hwaddr start_addr,
                               hwaddr rom_base, hwaddr rom_size,
                               uint64_t kernel_entry,
                               uint32_t fdt_load_addr)
{
    int i;
    uint32_t start_addr_hi32 = 0x00000000;

    if (!riscv_is_32bit(harts)) {
        start_addr_hi32 = start_addr >> 32;
    }
    /* reset vector */
    uint32_t reset_vec[10] = {
        0x00000297,                  /* 1:  auipc  t0, %pcrel_hi(fw_dyn) */
        0x02828613,                  /*     addi   a2, t0, %pcrel_lo(1b) */
        0xf1402573,                  /*     csrr   a0, mhartid  */
        0,
        0,
        0x00028067,                  /*     jr     t0 */
        start_addr,                  /* start: .dword */
        start_addr_hi32,
        fdt_load_addr,               /* fdt_laddr: .dword */
        0x00000000,
                                     /* fw_dyn: */
    };
    if (riscv_is_32bit(harts)) {
        reset_vec[3] = 0x0202a583;   /*     lw     a1, 32(t0) */
        reset_vec[4] = 0x0182a283;   /*     lw     t0, 24(t0) */
    } else {
        reset_vec[3] = 0x0202b583;   /*     ld     a1, 32(t0) */
        reset_vec[4] = 0x0182b283;   /*     ld     t0, 24(t0) */
    }

    /* copy in the reset vector in little_endian byte order */
    for (i = 0; i < ARRAY_SIZE(reset_vec); i++) {
        reset_vec[i] = cpu_to_le32(reset_vec[i]);
    }

    rom_add_blob_fixed_as("mrom.reset", reset_vec, sizeof(reset_vec),
                          rom_base, &address_space_memory);
}

static void quard_star_machine_init(MachineState *machine)
{
    const MemMapEntry *memmap = virt_memmap;
    QuardStarState *s = RISCV_VIRT_MACHINE(machine);
    MemoryRegion *system_memory = get_system_memory();
    MemoryRegion *main_mem = g_new(MemoryRegion, 1);
    MemoryRegion *sram_mem = g_new(MemoryRegion, 1);
    MemoryRegion *mask_rom = g_new(MemoryRegion, 1);
    RISCVHartArrayState *cpus;
    int i, j, base_hartid, hart_count;
    char *plic_hart_config, *cpus_name;
    size_t plic_hart_config_len;

    object_initialize_child(OBJECT(machine), "r-cluster", &s->r_cluster, TYPE_CPU_CLUSTER);
    qdev_prop_set_uint32(DEVICE(&s->r_cluster), "cluster-id", 0);

    object_initialize_child(OBJECT(machine), "c-cluster", &s->c_cluster, TYPE_CPU_CLUSTER);
    qdev_prop_set_uint32(DEVICE(&s->c_cluster), "cluster-id", 1);

    for (i = 0; i < 2; i++) {
        if(i < QUARD_STAR_MANAGEMENT_CPU_COUNT) {
            base_hartid = 0;
            hart_count = QUARD_STAR_MANAGEMENT_CPU_COUNT;
            cpus_name = g_strdup_printf("r_cpus%d", i);
            cpus = &s->r_cpus[i];
            object_initialize_child(OBJECT(&s->r_cluster), cpus_name, cpus,
                                    TYPE_RISCV_HART_ARRAY);
        } else {
            base_hartid = QUARD_STAR_MANAGEMENT_CPU_COUNT;
            hart_count = machine->smp.cpus - QUARD_STAR_MANAGEMENT_CPU_COUNT;
            cpus_name = g_strdup_printf("c_cpus%d", i-QUARD_STAR_MANAGEMENT_CPU_COUNT);
            cpus = &s->c_cpus[i-QUARD_STAR_MANAGEMENT_CPU_COUNT];
            object_initialize_child(OBJECT(&s->c_cluster), cpus_name, cpus,
                                    TYPE_RISCV_HART_ARRAY);
        }
        g_free(cpus_name);
        object_property_set_str(OBJECT(cpus), "cpu-type",
                                machine->cpu_type, &error_abort);
        object_property_set_int(OBJECT(cpus), "hartid-base",
                                base_hartid, &error_abort);
        object_property_set_int(OBJECT(cpus), "num-harts",
                                hart_count, &error_abort);
        object_property_set_int(OBJECT(cpus), "resetvec", 
                                virt_memmap[QUARD_STAR_MROM].base, &error_abort);
        sysbus_realize(SYS_BUS_DEVICE(cpus), &error_abort);
    }

    qdev_realize(DEVICE(&s->r_cluster), NULL, &error_abort);
    qdev_realize(DEVICE(&s->c_cluster), NULL, &error_abort);

    sifive_clint_create(
        memmap[QUARD_STAR_CLINT].base,
        memmap[QUARD_STAR_CLINT].size, 0, machine->smp.cpus,
        SIFIVE_SIP_BASE, SIFIVE_TIMECMP_BASE, SIFIVE_TIME_BASE,
        SIFIVE_CLINT_TIMEBASE_FREQ, true);

    plic_hart_config_len =
        (strlen(QUARD_STAR_PLIC_HART_CONFIG) + 1) * machine->smp.cpus;
    plic_hart_config = g_malloc0(plic_hart_config_len);
    for (j = 0; j < machine->smp.cpus; j++) {
        if (j != 0) {
            strncat(plic_hart_config, ",", plic_hart_config_len);
        }
        strncat(plic_hart_config, QUARD_STAR_PLIC_HART_CONFIG,
            plic_hart_config_len);
        plic_hart_config_len -= (strlen(QUARD_STAR_PLIC_HART_CONFIG) + 1);
    }

    s->plic = sifive_plic_create(
        memmap[QUARD_STAR_PLIC].base,
        plic_hart_config, 0,
        QUARD_STAR_PLIC_NUM_SOURCES,
        QUARD_STAR_PLIC_NUM_PRIORITIES,
        QUARD_STAR_PLIC_PRIORITY_BASE,
        QUARD_STAR_PLIC_PENDING_BASE,
        QUARD_STAR_PLIC_ENABLE_BASE,
        QUARD_STAR_PLIC_ENABLE_STRIDE,
        QUARD_STAR_PLIC_CONTEXT_BASE,
        QUARD_STAR_PLIC_CONTEXT_STRIDE,
        memmap[QUARD_STAR_PLIC].size);
    g_free(plic_hart_config);

    memory_region_init_ram(main_mem, NULL, "riscv_quard_star_board.dram",
                           machine->ram_size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[QUARD_STAR_DRAM].base,
        main_mem);

    memory_region_init_ram(sram_mem, NULL, "riscv_quard_star_board.sram",
                           memmap[QUARD_STAR_SRAM].size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[QUARD_STAR_SRAM].base,
        sram_mem);

    memory_region_init_rom(mask_rom, NULL, "riscv_quard_star_board.mrom",
                           memmap[QUARD_STAR_MROM].size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[QUARD_STAR_MROM].base,
                                mask_rom);

    quard_star_setup_rom_reset_vec(machine, &s->r_cpus[0], virt_memmap[QUARD_STAR_FLASH].base,
                              virt_memmap[QUARD_STAR_MROM].base,
                              virt_memmap[QUARD_STAR_MROM].size,
                              0x0, 0x0);

    serial_mm_init(system_memory, memmap[QUARD_STAR_UART0].base,
        0, qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_UART0_IRQ), 399193,
        serial_hd(0), DEVICE_LITTLE_ENDIAN);
    serial_mm_init(system_memory, memmap[QUARD_STAR_UART1].base,
        0, qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_UART1_IRQ), 399193,
        serial_hd(1), DEVICE_LITTLE_ENDIAN);
    serial_mm_init(system_memory, memmap[QUARD_STAR_UART2].base,
        0, qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_UART2_IRQ), 399193,
        serial_hd(2), DEVICE_LITTLE_ENDIAN);

    sysbus_create_simple("goldfish_rtc", memmap[QUARD_STAR_RTC].base,
        qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_RTC_IRQ));

    sifive_test_create(memmap[QUARD_STAR_TEST].base);

    for (i = 0; i < QUARD_STAR_COUNT; i++) {
        sysbus_create_simple("virtio-mmio",
            memmap[QUARD_STAR_VIRTIO].base + i * memmap[QUARD_STAR_VIRTIO].size,
            qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_IRQ + i));
    }

    s->fw_cfg = fw_cfg_init_mem_wide(memmap[QUARD_STAR_FW_CFG].base + 8, 
                                     memmap[QUARD_STAR_FW_CFG].base,  8, 
                                     memmap[QUARD_STAR_FW_CFG].base + 16,
                                     &address_space_memory);
    fw_cfg_add_i16(s->fw_cfg, FW_CFG_NB_CPUS, (uint16_t)machine->smp.cpus);
    rom_set_fw(s->fw_cfg);

    s->flash = quard_star_flash_create(s, "quard-star.flash0", "pflash0");
    pflash_cfi01_legacy_drive(s->flash, drive_get(IF_PFLASH, 0, 0));
    quard_star_flash_map(s->flash, virt_memmap[QUARD_STAR_FLASH].base,
                         virt_memmap[QUARD_STAR_FLASH].size, system_memory);

    object_initialize_child(OBJECT(machine), "i2c0", &s->i2c[0], TYPE_IMX_I2C);
    sysbus_realize(SYS_BUS_DEVICE(&s->i2c[0]), &error_abort);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->i2c[0]), 0, memmap[QUARD_STAR_I2C0].base);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->i2c[0]), 0,
                    qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_I2C0_IRQ));
    object_initialize_child(OBJECT(machine), "i2c1", &s->i2c[1], TYPE_IMX_I2C);
    sysbus_realize(SYS_BUS_DEVICE(&s->i2c[1]), &error_abort);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->i2c[1]), 0, memmap[QUARD_STAR_I2C1].base);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->i2c[1]), 0,
                    qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_I2C1_IRQ));
    object_initialize_child(OBJECT(machine), "i2c2", &s->i2c[2], TYPE_IMX_I2C);
    sysbus_realize(SYS_BUS_DEVICE(&s->i2c[2]), &error_abort);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->i2c[2]), 0, memmap[QUARD_STAR_I2C2].base);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->i2c[2]), 0,
                    qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_I2C2_IRQ));
    I2CSlave *i2c_dev = i2c_slave_new("at24c-eeprom", 0x50);
    DeviceState *dev = DEVICE(i2c_dev);
    qdev_prop_set_uint32(dev, "rom-size", 8*1024);
    i2c_slave_realize_and_unref(i2c_dev, s->i2c[0].bus, &error_abort);
}

static void quard_star_machine_instance_init(Object *obj)
{
}

static void quard_star_machine_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "RISC-V Quard Star board";
    mc->init = quard_star_machine_init;
    mc->max_cpus = QUARD_STAR_MANAGEMENT_CPU_COUNT +
                   QUARD_STAR_COMPUTE_CPU_COUNT;
    mc->min_cpus = QUARD_STAR_MANAGEMENT_CPU_COUNT + 1;
    mc->default_cpus = mc->min_cpus;
    mc->default_cpu_type = TYPE_RISCV_CPU_BASE;
    mc->pci_allow_0_address = true;
    mc->possible_cpu_arch_ids = riscv_numa_possible_cpu_arch_ids;
    mc->cpu_index_to_instance_props = riscv_numa_cpu_index_to_props;
    mc->get_default_cpu_node_id = riscv_numa_get_default_cpu_node_id;
    mc->numa_mem_supported = true;
}

static const TypeInfo quard_star_machine_typeinfo = {
    .name       = MACHINE_TYPE_NAME("quard-star"),
    .parent     = TYPE_MACHINE,
    .class_init = quard_star_machine_class_init,
    .instance_init = quard_star_machine_instance_init,
    .instance_size = sizeof(QuardStarState),
};

static void quard_star_machine_init_register_types(void)
{
    type_register_static(&quard_star_machine_typeinfo);
}

type_init(quard_star_machine_init_register_types)
