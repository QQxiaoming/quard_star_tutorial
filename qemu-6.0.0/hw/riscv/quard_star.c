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
    [QUARD_STAR_MROM]    = { 0x00000000,    0x8000 },
    [QUARD_STAR_SRAM]    = { 0x00008000,    0x8000 },

    [QUARD_STAR_TEST]    = { 0x00100000,    0x1000 },
    [QUARD_STAR_CLINT]   = { 0x02000000,   0x10000 },
    [QUARD_STAR_PLIC]    = { 0x0c000000, 0x4000000 },

    [QUARD_STAR_UART0]   = { 0x10000000,    0x1000 },
    [QUARD_STAR_UART1]   = { 0x10001000,    0x1000 },
    [QUARD_STAR_UART2]   = { 0x10002000,    0x1000 },
    [QUARD_STAR_RTC]     = { 0x10003000,    0x1000 },
    [QUARD_STAR_I2C0]    = { 0x10004000,    0x1000 },
    [QUARD_STAR_I2C1]    = { 0x10005000,    0x1000 },
    [QUARD_STAR_I2C2]    = { 0x10006000,    0x1000 },
    [QUARD_STAR_SPI0]    = { 0x10007000,    0x1000 },
    [QUARD_STAR_SPI1]    = { 0x10008000,    0x1000 },

    [QUARD_STAR_VIRTIO0] = { 0x10100000,    0x1000 },
    [QUARD_STAR_VIRTIO1] = { 0x10101000,    0x1000 },
    [QUARD_STAR_VIRTIO2] = { 0x10102000,    0x1000 },
    [QUARD_STAR_VIRTIO3] = { 0x10103000,    0x1000 },
    [QUARD_STAR_VIRTIO4] = { 0x10104000,    0x1000 },
    [QUARD_STAR_VIRTIO5] = { 0x10105000,    0x1000 },
    [QUARD_STAR_VIRTIO6] = { 0x10106000,    0x1000 },
    [QUARD_STAR_VIRTIO7] = { 0x10107000,    0x1000 },
    [QUARD_STAR_FW_CFG]  = { 0x10108000,      0x18 },

    [QUARD_STAR_FLASH]   = { 0x20000000, 0x2000000 },
    [QUARD_STAR_DRAM]    = { 0x80000000,       0x0 },
};

static void quard_star_setup_rom_reset_vec(MachineState *machine, 
                                RISCVHartArrayState *harts, hwaddr start_addr,
                                hwaddr rom_base, hwaddr rom_size,
                                uint64_t kernel_entry, uint32_t fdt_load_addr)
{
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
    for (int i = 0; i < ARRAY_SIZE(reset_vec); i++) {
        reset_vec[i] = cpu_to_le32(reset_vec[i]);
    }

    rom_add_blob_fixed_as("mrom.reset", reset_vec, sizeof(reset_vec),
                          rom_base, &address_space_memory);
}

static void quard_star_cpu_create(MachineState *machine)
{
    QuardStarState *s = RISCV_VIRT_MACHINE(machine);

    object_initialize_child(OBJECT(machine), "c-cluster",
                                    &s->c_cluster, TYPE_CPU_CLUSTER);
    qdev_prop_set_uint32(DEVICE(&s->c_cluster), "cluster-id", 0);
    object_initialize_child(OBJECT(&s->c_cluster), "c-cpus",
                                &s->c_cpus, TYPE_RISCV_HART_ARRAY);
    object_property_set_str(OBJECT(&s->c_cpus), "cpu-type",
                            machine->cpu_type, &error_abort);
    object_property_set_int(OBJECT(&s->c_cpus), "hartid-base",
                            0, &error_abort);
    object_property_set_int(OBJECT(&s->c_cpus), "num-harts",
                            QUARD_STAR_COMPUTE_CPU_COUNT, &error_abort);
    object_property_set_int(OBJECT(&s->c_cpus), "resetvec", 
                            virt_memmap[QUARD_STAR_MROM].base, 
                            &error_abort);
    sysbus_realize(SYS_BUS_DEVICE(&s->c_cpus), &error_abort);
    qdev_realize(DEVICE(&s->c_cluster), NULL, &error_abort);

    object_initialize_child(OBJECT(machine), "r-cluster", 
                                    &s->r_cluster, TYPE_CPU_CLUSTER);
    qdev_prop_set_uint32(DEVICE(&s->r_cluster), "cluster-id", 1);
    object_initialize_child(OBJECT(&s->r_cluster), "r-cpus", 
                            &s->r_cpus, TYPE_RISCV_HART_ARRAY);
    object_property_set_str(OBJECT(&s->r_cpus), "cpu-type",
                            machine->cpu_type, &error_abort);
    object_property_set_int(OBJECT(&s->r_cpus), "hartid-base",
                            7, &error_abort);
    object_property_set_int(OBJECT(&s->r_cpus), "num-harts",
                            QUARD_STAR_MANAGEMENT_CPU_COUNT, &error_abort);
    object_property_set_int(OBJECT(&s->r_cpus), "resetvec", 
                            virt_memmap[QUARD_STAR_MROM].base, 
                            &error_abort);
    sysbus_realize(SYS_BUS_DEVICE(&s->r_cpus), &error_abort);
    qdev_realize(DEVICE(&s->r_cluster), NULL, &error_abort);
}

static void quard_star_interrupt_controller_create(MachineState *machine)
{
    QuardStarState *s = RISCV_VIRT_MACHINE(machine);
    size_t plic_hart_config_len;
    char *plic_hart_config;

    sifive_clint_create(
        virt_memmap[QUARD_STAR_CLINT].base,
        virt_memmap[QUARD_STAR_CLINT].size, 0, machine->smp.cpus,
        SIFIVE_SIP_BASE, SIFIVE_TIMECMP_BASE, SIFIVE_TIME_BASE,
        SIFIVE_CLINT_TIMEBASE_FREQ, true);

    plic_hart_config_len =
        (strlen(QUARD_STAR_PLIC_HART_CONFIG) + 1) * machine->smp.cpus;
    plic_hart_config = g_malloc0(plic_hart_config_len);
    for (int i = 0; i < machine->smp.cpus; i++) {
        if (i != 0) {
            strncat(plic_hart_config, ",", plic_hart_config_len);
        }
        strncat(plic_hart_config, QUARD_STAR_PLIC_HART_CONFIG,
            plic_hart_config_len);
        plic_hart_config_len -= (strlen(QUARD_STAR_PLIC_HART_CONFIG) + 1);
    }

    s->plic = sifive_plic_create(
        virt_memmap[QUARD_STAR_PLIC].base,
        plic_hart_config, 0,
        QUARD_STAR_PLIC_NUM_SOURCES,
        QUARD_STAR_PLIC_NUM_PRIORITIES,
        QUARD_STAR_PLIC_PRIORITY_BASE,
        QUARD_STAR_PLIC_PENDING_BASE,
        QUARD_STAR_PLIC_ENABLE_BASE,
        QUARD_STAR_PLIC_ENABLE_STRIDE,
        QUARD_STAR_PLIC_CONTEXT_BASE,
        QUARD_STAR_PLIC_CONTEXT_STRIDE,
        virt_memmap[QUARD_STAR_PLIC].size);
    g_free(plic_hart_config);
}

static void quard_star_memory_create(MachineState *machine)
{
    MemoryRegion *system_memory = get_system_memory();
    QuardStarState *s = RISCV_VIRT_MACHINE(machine);
    MemoryRegion *main_mem = g_new(MemoryRegion, 1);
    MemoryRegion *sram_mem = g_new(MemoryRegion, 1);
    MemoryRegion *mask_rom = g_new(MemoryRegion, 1);

    memory_region_init_ram(main_mem, NULL, "riscv_quard_star_board.dram",
                           machine->ram_size, &error_fatal);
    memory_region_add_subregion(system_memory, 
                                virt_memmap[QUARD_STAR_DRAM].base, main_mem);

    memory_region_init_ram(sram_mem, NULL, "riscv_quard_star_board.sram",
                           virt_memmap[QUARD_STAR_SRAM].size, &error_fatal);
    memory_region_add_subregion(system_memory, 
                                virt_memmap[QUARD_STAR_SRAM].base, sram_mem);

    memory_region_init_rom(mask_rom, NULL, "riscv_quard_star_board.mrom",
                           virt_memmap[QUARD_STAR_MROM].size, &error_fatal);
    memory_region_add_subregion(system_memory, 
                                virt_memmap[QUARD_STAR_MROM].base, mask_rom);

    quard_star_setup_rom_reset_vec(machine, &s->r_cpus, 
                              virt_memmap[QUARD_STAR_FLASH].base,
                              virt_memmap[QUARD_STAR_MROM].base,
                              virt_memmap[QUARD_STAR_MROM].size,
                              0x0, 0x0);
}

static void quard_star_serial_create(MachineState *machine)
{    
    MemoryRegion *system_memory = get_system_memory();
    QuardStarState *s = RISCV_VIRT_MACHINE(machine);

    serial_mm_init(system_memory, virt_memmap[QUARD_STAR_UART0].base,
        0, qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_UART0_IRQ), 399193,
        serial_hd(0), DEVICE_LITTLE_ENDIAN);
    serial_mm_init(system_memory, virt_memmap[QUARD_STAR_UART1].base,
        0, qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_UART1_IRQ), 399193,
        serial_hd(1), DEVICE_LITTLE_ENDIAN);
    serial_mm_init(system_memory, virt_memmap[QUARD_STAR_UART2].base,
        0, qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_UART2_IRQ), 399193,
        serial_hd(2), DEVICE_LITTLE_ENDIAN);
}

static void quard_star_syscon_create(MachineState *machine)
{    
    sifive_test_create(virt_memmap[QUARD_STAR_TEST].base);
}

static void quard_star_rtc_create(MachineState *machine)
{    
    QuardStarState *s = RISCV_VIRT_MACHINE(machine);

    sysbus_create_simple("goldfish_rtc", virt_memmap[QUARD_STAR_RTC].base,
        qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_RTC_IRQ));
}

static void quard_star_flash_create(MachineState *machine)
{ 
    MemoryRegion *system_memory = get_system_memory();
    QuardStarState *s = RISCV_VIRT_MACHINE(machine);
    uint64_t flash_sector_size = 256 * KiB;
    DeviceState *dev = qdev_new(TYPE_PFLASH_CFI01);

    qdev_prop_set_uint64(dev, "sector-length", flash_sector_size);
    qdev_prop_set_uint8(dev, "width", 4);
    qdev_prop_set_uint8(dev, "device-width", 2);
    qdev_prop_set_bit(dev, "big-endian", false);
    qdev_prop_set_uint16(dev, "id0", 0x89);
    qdev_prop_set_uint16(dev, "id1", 0x18);
    qdev_prop_set_uint16(dev, "id2", 0x00);
    qdev_prop_set_uint16(dev, "id3", 0x00);
    qdev_prop_set_string(dev, "name", "quard-star.flash0");
    object_property_add_child(OBJECT(s), "quard-star.flash0", OBJECT(dev));
    object_property_add_alias(OBJECT(s), "pflash0",
                              OBJECT(dev), "drive");
    s->flash = PFLASH_CFI01(dev);
    pflash_cfi01_legacy_drive(s->flash, drive_get(IF_PFLASH, 0, 0));

    assert(QEMU_IS_ALIGNED(virt_memmap[QUARD_STAR_FLASH].size, 
                                flash_sector_size));
    assert(virt_memmap[QUARD_STAR_FLASH].size/flash_sector_size <= UINT32_MAX);
    qdev_prop_set_uint32(dev, "num-blocks", 
                    virt_memmap[QUARD_STAR_FLASH].size / flash_sector_size);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);

    memory_region_add_subregion(system_memory, 
                            virt_memmap[QUARD_STAR_FLASH].base,
                            sysbus_mmio_get_region(SYS_BUS_DEVICE(dev), 0));
}

static void quard_star_i2c_create(MachineState *machine)
{ 
    QuardStarState *s = RISCV_VIRT_MACHINE(machine);

    object_initialize_child(OBJECT(machine), "i2c0", &s->i2c[0], TYPE_IMX_I2C);
    sysbus_realize(SYS_BUS_DEVICE(&s->i2c[0]), &error_abort);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->i2c[0]), 0, 
                        virt_memmap[QUARD_STAR_I2C0].base);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->i2c[0]), 0,
                    qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_I2C0_IRQ));

    object_initialize_child(OBJECT(machine), "i2c1", &s->i2c[1], TYPE_IMX_I2C);
    sysbus_realize(SYS_BUS_DEVICE(&s->i2c[1]), &error_abort);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->i2c[1]), 0, 
                        virt_memmap[QUARD_STAR_I2C1].base);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->i2c[1]), 0,
                    qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_I2C1_IRQ));

    object_initialize_child(OBJECT(machine), "i2c2", &s->i2c[2], TYPE_IMX_I2C);
    sysbus_realize(SYS_BUS_DEVICE(&s->i2c[2]), &error_abort);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->i2c[2]), 0, 
                        virt_memmap[QUARD_STAR_I2C2].base);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->i2c[2]), 0,
                    qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_I2C2_IRQ));

    I2CSlave *i2c_dev = i2c_slave_new("at24c-eeprom", 0x50);
    DeviceState *dev = DEVICE(i2c_dev);
    qdev_prop_set_uint32(dev, "rom-size", 8*1024);
    i2c_slave_realize_and_unref(i2c_dev, s->i2c[0].bus, &error_abort);
}

static void quard_star_spi_create(MachineState *machine)
{ 
    QuardStarState *s = RISCV_VIRT_MACHINE(machine);

    object_initialize_child(OBJECT(machine), "spi0", &s->spi[0],
                                TYPE_SIFIVE_SPI);
    sysbus_realize(SYS_BUS_DEVICE(&s->spi[0]), &error_abort);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->spi[0]), 0, 
                            virt_memmap[QUARD_STAR_SPI0].base);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->spi[0]), 0,
                    qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_SPI0_IRQ));

    object_initialize_child(OBJECT(machine), "spi1", &s->spi[1],
                                TYPE_SIFIVE_SPI);
    sysbus_realize(SYS_BUS_DEVICE(&s->spi[1]), &error_abort);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->spi[1]), 0, 
                            virt_memmap[QUARD_STAR_SPI1].base);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->spi[1]), 0,
                    qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_SPI1_IRQ));
                    
    DeviceState *flash_dev = qdev_new("is25wp256");
    DriveInfo *dinfo = drive_get_next(IF_MTD);
    if (dinfo) {
        qdev_prop_set_drive_err(flash_dev, "drive",
                                blk_by_legacy_dinfo(dinfo),
                                &error_fatal);
    }
    qdev_realize_and_unref(flash_dev, BUS(s->spi[0].spi), &error_fatal);
    qemu_irq flash_cs = qdev_get_gpio_in_named(flash_dev, SSI_GPIO_CS, 0);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->spi[0]), 1, flash_cs);
}

static void quard_star_virtio_mmio_create(MachineState *machine)
{    
    QuardStarState *s = RISCV_VIRT_MACHINE(machine);

    sysbus_create_simple("virtio-mmio",
        virt_memmap[QUARD_STAR_VIRTIO0].base,
        qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_VIRTIO0_IRQ));
    sysbus_create_simple("virtio-mmio",
        virt_memmap[QUARD_STAR_VIRTIO1].base,
        qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_VIRTIO1_IRQ));
    sysbus_create_simple("virtio-mmio",
        virt_memmap[QUARD_STAR_VIRTIO2].base,
        qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_VIRTIO2_IRQ));
    sysbus_create_simple("virtio-mmio",
        virt_memmap[QUARD_STAR_VIRTIO3].base,
        qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_VIRTIO3_IRQ));
    sysbus_create_simple("virtio-mmio",
        virt_memmap[QUARD_STAR_VIRTIO4].base,
        qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_VIRTIO4_IRQ));
    sysbus_create_simple("virtio-mmio",
        virt_memmap[QUARD_STAR_VIRTIO5].base,
        qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_VIRTIO5_IRQ));
    sysbus_create_simple("virtio-mmio",
        virt_memmap[QUARD_STAR_VIRTIO6].base,
        qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_VIRTIO6_IRQ));
    sysbus_create_simple("virtio-mmio",
        virt_memmap[QUARD_STAR_VIRTIO7].base,
        qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_VIRTIO7_IRQ));
}

static void quard_star_fw_cfg_create(MachineState *machine)
{    
    QuardStarState *s = RISCV_VIRT_MACHINE(machine);

    s->fw_cfg = fw_cfg_init_mem_wide(virt_memmap[QUARD_STAR_FW_CFG].base + 8, 
                                     virt_memmap[QUARD_STAR_FW_CFG].base,  8, 
                                     virt_memmap[QUARD_STAR_FW_CFG].base + 16,
                                     &address_space_memory);
    fw_cfg_add_i16(s->fw_cfg, FW_CFG_NB_CPUS, (uint16_t)machine->smp.cpus);
    rom_set_fw(s->fw_cfg);
}

static void quard_star_machine_init(MachineState *machine)
{
    quard_star_cpu_create(machine);
    quard_star_interrupt_controller_create(machine);
    quard_star_memory_create(machine);
    quard_star_flash_create(machine);
    quard_star_syscon_create(machine);
    quard_star_rtc_create(machine);
    quard_star_serial_create(machine);
    quard_star_i2c_create(machine);
    quard_star_spi_create(machine);

    quard_star_virtio_mmio_create(machine);
    quard_star_fw_cfg_create(machine);
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
    mc->min_cpus = QUARD_STAR_MANAGEMENT_CPU_COUNT +
                   QUARD_STAR_COMPUTE_CPU_COUNT;
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
