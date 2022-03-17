# 基于qemu-riscv从0开始构建嵌入式linux系统ch3. 添加中断控制器、串口、pflash支持

### 添加串口支持

真实板卡的初期调试中，串口打印可谓神器，几乎所有的SOC都会带有串口这个外设，因此我们也需要添加串口到我们的定义中。

- 首先添加三个串口基地址，之所以添加三个的目的是为了后面我们可能会在多个权限域空间内跑不同的系统因此可能需要不同权限的串口打印输出。

```c
{
    [QUARD_STAR_UART0] = { 0x10000000,         0x100 },
    [QUARD_STAR_UART1] = { 0x10001000,         0x100 },
    [QUARD_STAR_UART2] = { 0x10002000,         0x100 },
}
```

- 在初始化函数中调用serial_mm_init创建串口实例，这个串口仿真ns16550a的定义，在后续opensbi、u-boot、kernel中都自带这个串口的驱动，方面后面移植。

```c
serial_mm_init(system_memory, memmap[QUARD_STAR_UART0].base,
    0, qdev_get_gpio_in(DEVICE(mmio_plic), QUARD_STAR_UART0_IRQ), 399193,
    serial_hd(0), DEVICE_LITTLE_ENDIAN);
serial_mm_init(system_memory, memmap[QUARD_STAR_UART1].base,
    0, qdev_get_gpio_in(DEVICE(mmio_plic), QUARD_STAR_UART1_IRQ), 399193,
    serial_hd(1), DEVICE_LITTLE_ENDIAN);
serial_mm_init(system_memory, memmap[QUARD_STAR_UART2].base,
    0, qdev_get_gpio_in(DEVICE(mmio_plic), QUARD_STAR_UART2_IRQ), 399193,
    serial_hd(2), DEVICE_LITTLE_ENDIAN);
```

> 注意上述qdev_get_gpio_in函数用来配置串口中断信号，因此我们必须要增加中断控制器的相关代码才能正确使用串口。

### 添加中断控制器

既然串口需要使用到中断，后续很多外设都需要中断控制器，因此我们在这一节将中断控制器一并添加了。

- 依然是增加中断控制器基地址,RISCV标准的中断控制器分为两部分内核中断CLINT(Core Local Interrupt)和外设中断控制器Platform-Level Interrupt Controller(PLIC)，CLINT在每一个smp架构下每个core都各有自己私有的中断，PLIC则是所以core共用的外部中断控制器。

```c
{
    [QUARD_STAR_CLINT] = {  0x2000000,       0x10000 },
    [QUARD_STAR_PLIC]  = {  0xc000000, QUARD_STAR_PLIC_SIZE(QUARD_STAR_CPUS_MAX * 2) },
}
```

- 在初始化函数内，分别使用以下两个函数创建相关ip，注意要根据内核smp个数分别为每个core创建对应的资源。

```c
sifive_clint_create
sifive_plic_create
```

截止目前，我们有了中断控制器和串口，就可以进行简单的串口功能测试了，但遗憾的是目前还没有可存放我们的固件程序并加载到ddr中使用的设备，因此我们打算增加pflash仿真器件作为固件的载体。

### 添加pflash支持

pflash是并行flash，目前真实的器件已经很少见了，市面上常见的是qspi的nor flash作为低阶固件的载体，而nor flash一般都支持XIP运行，那么基本上等同于pflash了，那么为了方便我们就使用qemu提供的plash作为早期固件的载体。

- 依然是增加pflash的基地址，这里不是寄存器，而是pflash的数据起始地址，大小是32M，完全足够我们存放固件了。

```c
{
    [QUARD_STAR_FLASH] = { 0x20000000,     0x2000000 },
}
```

- 创建pflash器件并映射到我们的系统总线对应地址上，最后将其关联到“-drive if=pflash,bus=0,unit=0,…………”的qemu参数上，这样启动仿真时就可以加载固件文件到这片flash上，以下这些代码都不难理解，这里不在赘述。

```c
static PFlashCFI01 *quard_star_flash_create(RISCVVirtState *s,
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

s->flash = quard_star_flash_create(s, "quard-star.flash0", "pflash0");
pflash_cfi01_legacy_drive(s->flash, drive_get(IF_PFLASH, 0, 0));
quard_star_flash_map(s->flash, virt_memmap[QUARD_STAR_FLASH].base,
                        virt_memmap[QUARD_STAR_FLASH].size, system_memory);
```

- 最后修改quard_star_setup_rom_reset_vec函数中bootrom的跳转地址为pflash上的地址。

到这里本篇顺利完成。这一节主要添加中断控制，串口，flash。下一篇就要开始制作一个简单的固件加载到flash中简单测试以下串口输出是否正常。

> 本教程的<br>github仓库：https://github.com/QQxiaoming/quard_star_tutorial<br>gitee仓库：https://gitee.com/QQxiaoming/quard_star_tutorial<br>本节所在tag：ch3