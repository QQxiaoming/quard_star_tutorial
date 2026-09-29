# `git diff HEAD^^` 修改说明

## 1. 总体结论

你的理解基本正确：这组修改的主要目的，是在仓库自带的 QEMU 6.0.0 中加入一个名为 `quard-star` 的 RISC-V 机器（board/machine）骨架，使下面的命令能够识别 `-M quard-star`：

```text
qemu-system-riscv64 -M quard-star ...
```

不过，准确地说，它目前完成的是“板级模型的第一步”，而不是一块已经可以正常启动固件或操作系统的完整开发板。当前代码已经实现：

- 将 `quard-star` 加入 RISC-V 32 位和 64 位的默认配置；
- 将板级源文件接入 Meson 构建；
- 向 QEMU 的 QOM 类型系统注册 `quard-star` 机器；
- 创建最多 8 个 RISC-V hart；
- 映射 MROM、SRAM 和 DRAM；
- 向 MROM 写入一段复位跳板代码；
- 提供编译和运行脚本。

当前尚未真正实现：

- 固件、内核和 initrd 的加载；
- 设备树（FDT）的创建和加载；
- UART 设备本身；
- CLINT、PLIC 等中断/定时器设备；
- VirtIO、存储、网络等外设；
- 一个能够跳转到有效固件入口的完整启动流程。

所以更精确的描述是：**这些代码把 quard-star 板子的最小机器骨架加入了 QEMU 6.0.0，但当前版本还不能正常引导软件。**

`git diff HEAD^^` 覆盖当前 `HEAD` 之前的两个提交：

```text
3ae0efefd  添加quard-star板定义
76910d6ba  16 -> 10 to fit my machine
```

前一个提交是主体功能，后一个提交只把编译并行度由 16 调低到 10。

## 2. 修改之间的关系

整个接入链可以概括为：

```text
default-configs/*.mak
        |
        | enable CONFIG_QUARD_STAR
        v
hw/riscv/Kconfig
        |
        | declare option and select SERIAL
        v
hw/riscv/meson.build
        |
        | compile quard_star.c when enabled
        v
hw/riscv/quard_star.c + include/hw/riscv/quard_star.h
        |
        | register QOM machine type "quard-star"
        v
qemu-system-riscv{32,64} -M quard-star
```

其中三层构建修改缺一不可：默认配置负责打开选项，Kconfig 负责定义及解析依赖，Meson 负责真正把 C 文件编进 QEMU。

## 3. `build.sh`

这次 diff 中只有一行变化：

```diff
-make -j16
+make -j10
```

它把 `make` 同时运行的编译任务数由 16 降到 10，用较少的 CPU 和内存换取更低的构建压力；它不改变生成的 QEMU 功能。

脚本其余部分虽然不是本次新增，也与理解流程有关：

```sh
SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
```

计算脚本自身所在目录，后面用它拼接安装目录。

```sh
cd qemu-6.0.0
if [ ! -d "$SHELL_FOLDER/output/qemu" ]; then
./configure --prefix=$SHELL_FOLDER/output/qemu \
    --target-list=riscv64-softmmu \
    --enable-gtk --enable-virtfs --disable-gio
fi
```

- 进入 QEMU 6.0.0 源码目录；
- 如果安装目录 `output/qemu` 不存在，就运行 `configure`；
- `--prefix` 指定 `make install` 的目标目录；
- `--target-list=riscv64-softmmu` 只构建 64 位 RISC-V 的完整系统模拟器；
- 其余选项分别启用 GTK、VirtFS，并禁用 GIO。

这里有两个小细节：

1. 虽然代码同时修改了 `riscv32-softmmu.mak`，这个脚本实际只构建 `riscv64-softmmu`，所以 32 位配置不会由该脚本验证。
2. `cd qemu-6.0.0` 仍是相对当前工作目录执行，而不是相对 `SHELL_FOLDER`，因此从仓库目录之外直接调用脚本可能失败。

最后的：

```sh
make -j10
make install
cd ..
```

分别负责并行编译、安装到 `output/qemu`，以及返回上一级目录。

## 4. RISC-V 默认配置

修改了：

- `qemu-6.0.0/default-configs/devices/riscv32-softmmu.mak`
- `qemu-6.0.0/default-configs/devices/riscv64-softmmu.mak`

两处都新增：

```make
CONFIG_QUARD_STAR=y
```

含义是在默认构建 RISC-V 32 位或 64 位 system emulator 时启用 `QUARD_STAR`。这样正常配置出的 `qemu-system-riscv32` 和 `qemu-system-riscv64` 都应该包含这块板，而不需要用户额外打开选项。

这两个文件只负责给配置符号赋值；仅仅增加这一行并不会自动编译 `quard_star.c`，还需要后面的 Kconfig 与 Meson 修改。

## 5. `hw/riscv/Kconfig`

新增：

```kconfig
config QUARD_STAR
    bool
    select SERIAL
```

逐项解释：

- `config QUARD_STAR`：声明名为 `QUARD_STAR` 的配置符号；
- `bool`：它是一个开/关选项；
- `select SERIAL`：启用该板时，同时确保 QEMU 的通用串口设备代码被编译。

这里的意图很明显：板子的内存布局中预留了 UART0，未来准备用 QEMU 的通用串口模型实现它。

但当前 `quard_star.c` 没有调用 `serial_mm_init()` 或创建其他 UART 设备，所以 `select SERIAL` 现在只是准备依赖，尚未产生一个可访问的串口。

## 6. `hw/riscv/meson.build`

新增：

```meson
riscv_ss.add(when: 'CONFIG_QUARD_STAR', if_true: files('quard_star.c'))
```

`riscv_ss` 是 RISC-V 硬件代码的 Meson source set。这一行表示：当配置系统得到 `CONFIG_QUARD_STAR=y` 时，把 `hw/riscv/quard_star.c` 加入编译。

这把上一节的配置开关与真正的 C 实现连接起来。如果缺少这一行，即使配置符号为 `y`，QEMU 中也不会出现 `quard-star` 机器类型。

## 7. `include/hw/riscv/quard_star.h`

### 7.1 头文件保护与依赖

```c
#ifndef HW_RISCV_QUARD_STAR__H
#define HW_RISCV_QUARD_STAR__H
...
#endif
```

这是标准的 include guard，避免同一翻译单元重复包含头文件。

包含的头文件提供：

- `RISCVHartArrayState`：一组同构 RISC-V hart 的 QEMU 设备模型；
- `MachineState`、SysBus/QOM 等板级模型所需类型和宏。

### 7.2 CPU 和 socket 上限

```c
#define QUARD_STAR_CPUS_MAX 8
#define QUARD_STAR_SOCKETS_MAX 8
```

- 整台机器最多允许 8 个 CPU/hart；
- 状态结构最多为 8 个 socket 保存 hart array。

注意这两个上限不是相乘关系。`MachineClass.max_cpus` 最终把整机 CPU 总数限制为 8；socket 数量也不能超过 8。

### 7.3 QOM 机器类型声明

```c
#define TYPE_RISCV_QUARD_STAR_MACHINE MACHINE_TYPE_NAME("quard-star")
typedef struct RISCVVirtState RISCVVirtState;
DECLARE_INSTANCE_CHECKER(RISCVVirtState, RISCV_VIRT_MACHINE,
                         TYPE_RISCV_QUARD_STAR_MACHINE)
```

`MACHINE_TYPE_NAME("quard-star")` 生成 QEMU 内部使用的机器类型名。`DECLARE_INSTANCE_CHECKER` 生成 `RISCV_VIRT_MACHINE(obj)` 检查/转换宏，用于确认传入对象确实是 quard-star machine，然后把它转换成 `RISCVVirtState *`。

这里的 `RISCVVirtState` 和 `RISCV_VIRT_MACHINE` 名称明显沿用了 `virt` 板代码。功能上可以工作，因为 `quard_star.c` 没有同时包含 `virt.h`，但名称容易让读者误以为它就是 QEMU `virt` 板。更清晰的命名会是 `QuardStarState` 和 `QUARD_STAR_MACHINE`。

### 7.4 板级实例状态

```c
struct RISCVVirtState {
    MachineState parent;
    RISCVHartArrayState soc[QUARD_STAR_SOCKETS_MAX];
};
```

这是每个 quard-star 机器实例保存的状态：

- `MachineState parent` 是 QOM 继承要求的父对象，必须放在结构开头；
- `soc[]` 为每个 socket 保存一个 hart array，每个 array 再包含该 socket 下的一个或多个 CPU/hart。

状态中没有 UART、PLIC、flash、FDT 等字段，也从侧面说明当前实现还很精简。

### 7.5 内存区域编号

```c
enum {
    QUARD_STAR_MROM,
    QUARD_STAR_SRAM,
    QUARD_STAR_UART0,
    QUARD_STAR_DRAM,
};
```

枚举值依次为 0、1、2、3。它们不是硬件地址，而是后面 `virt_memmap[]` 数组的下标。使用命名下标可以避免依赖数组元素的书写顺序。

### 7.6 UART 中断号

```c
enum {
    QUARD_STAR_UART0_IRQ = 10,
};
```

计划把 UART0 接到中断源 10。当前没有创建 UART，也没有创建 PLIC 或进行 IRQ 连线，所以这个常量暂未使用。

## 8. `hw/riscv/quard_star.c`

### 8.1 头文件

文件包含了 QEMU OS 兼容层、错误处理、板卡/QOM、内存加载、串口、RISC-V CPU/hart、启动、NUMA 和设备树等接口。

这是从其他 RISC-V 板代码裁剪而来的常见依赖集合。当前版本实际没有用到其中若干接口，例如 units、log、loader、serial、chardev、device tree 等；它们大多是为后续固件加载和外设实现预留，或者是复制代码后留下的 include。

### 8.2 地址空间表

```c
static const MemMapEntry virt_memmap[] = {
    [QUARD_STAR_MROM]  = {        0x0,        0x8000 },
    [QUARD_STAR_SRAM]  = {     0x8000,        0x8000 },
    [QUARD_STAR_UART0] = { 0x10000000,         0x100 },
    [QUARD_STAR_DRAM]  = { 0x80000000,           0x0 },
};
```

每个 `MemMapEntry` 保存 `{ base, size }`：

| 区域 | 起始地址 | 大小 | 结束地址 | 当前是否真正映射 |
|---|---:|---:|---:|---|
| MROM | `0x00000000` | `0x8000`，32 KiB | `0x00007fff` | 是，只读存储区 |
| SRAM | `0x00008000` | `0x8000`，32 KiB | `0x0000ffff` | 是，RAM |
| UART0 | `0x10000000` | `0x100`，256 B | `0x100000ff` | 否，只声明了地址 |
| DRAM | `0x80000000` | 运行时决定 | 取决于 `-m` | 是，RAM |

DRAM 表中的 size 写成 0，是因为真正大小来自 `machine->ram_size`，也就是命令行的 `-m`。变量名 `virt_memmap` 同样是从 `virt` 板遗留的命名，叫 `quard_star_memmap` 会更准确。

内存布局可表示为：

```text
0x00000000 +--------------------+
           | MROM, 32 KiB       |
0x00008000 +--------------------+
           | SRAM, 32 KiB       |
0x00010000 +--------------------+
           | unmapped           |
           |        ...         |
0x10000000 +--------------------+  UART0 planned, currently unmapped
0x10000100 +--------------------+
           | unmapped           |
           |        ...         |
0x80000000 +--------------------+
           | DRAM, -m size      |
           | e.g. 1 GiB         |
0xc0000000 +--------------------+  when -m 1G
```

### 8.3 `quard_star_setup_rom_reset_vec()`：生成复位跳板

函数签名接收机器、hart 数组、跳转目标、ROM 范围、内核入口和 FDT 地址。当前实现中实际只使用了：

- `harts`：判断目标是 RV32 还是 RV64；
- `start_addr`：最终跳转地址；
- `rom_base`：跳板写入地址；
- `fdt_load_addr`：传给下一级软件的设备树地址。

`machine`、`rom_size` 和 `kernel_entry` 当前没有被使用。这一点很重要：函数看起来支持 kernel，实际上并没有把 kernel 入口写进任何启动数据。

首先处理 64 位跳转地址的高 32 位：

```c
uint32_t start_addr_hi32 = 0x00000000;
if (!riscv_is_32bit(harts)) {
    start_addr_hi32 = start_addr >> 32;
}
```

随后构造由 10 个 32 位 word 组成的 `reset_vec`。前 6 个 word 是 RISC-V 指令，后 4 个 word 是两个 64 位数据槽：

```text
offset  contents
0x00    auipc t0, 0
0x04    addi  a2, t0, 40
0x08    csrr  a0, mhartid
0x0c    load  a1, 32(t0)       # RV32 uses lw, RV64 uses ld
0x10    load  t0, 24(t0)       # RV32 uses lw, RV64 uses ld
0x14    jr    t0
0x18    start_addr low 32 bits
0x1c    start_addr high 32 bits
0x20    FDT address low 32 bits
0x24    FDT address high 32 bits, fixed to 0
0x28    expected fw_dynamic_info location
```

按 RISC-V 启动约定，这段代码准备的寄存器是：

- `a0 = mhartid`：当前硬件线程编号；
- `a1 = fdt_load_addr`：设备树地址；
- `a2 = rom_base + 0x28`：预期的 OpenSBI dynamic firmware info 地址；
- `t0 = start_addr`，随后 `jr t0` 跳到固件入口。

RV32 用 `lw` 读取 32 位值，RV64 用 `ld` 读取完整的 64 位值。数组随后逐 word 调用 `cpu_to_le32()`，保证无论宿主机字节序如何，写入客体内存的指令都是 RISC-V 所需的小端格式。

```c
rom_add_blob_fixed_as("mrom.reset", reset_vec, sizeof(reset_vec),
                      rom_base, &address_space_memory);
```

最后把这 40 字节作为固定 ROM blob 放到系统地址空间的 `rom_base`。

这段代码基本复制了 QEMU 6.0.0 的通用 `riscv_setup_rom_reset_vec()`，但删掉了最后的：

```c
riscv_rom_copy_firmware_info(...);
```

因此 `a2` 虽然指向 `0x28`，那里并没有有效的 `fw_dynamic_info`。对于 OpenSBI `fw_dynamic` 启动流程，这是不完整的。

### 8.4 `quard_star_machine_init()`：创建机器实例

#### 8.4.1 取状态和分配内存区域对象

```c
const MemMapEntry *memmap = virt_memmap;
RISCVVirtState *s = RISCV_VIRT_MACHINE(machine);
MemoryRegion *system_memory = get_system_memory();
MemoryRegion *main_mem = g_new(MemoryRegion, 1);
MemoryRegion *sram_mem = g_new(MemoryRegion, 1);
MemoryRegion *mask_rom = g_new(MemoryRegion, 1);
```

- `memmap` 是地址表的便捷别名；
- QOM 检查并取得 quard-star 的实例状态；
- `system_memory` 是整个客体物理地址空间；
- 为 DRAM、SRAM、MROM 分配三个 `MemoryRegion` 描述对象。

`MemoryRegion` 是 QEMU 对物理地址区间的抽象，不是这几块内存内容本身。

#### 8.4.2 检查 socket 数量

```c
if (QUARD_STAR_SOCKETS_MAX < riscv_socket_count(machine)) {
    error_report(...);
    exit(1);
}
```

从 QEMU 已解析的 SMP 拓扑中取得 socket 数量，超过 8 就报告错误并终止。

#### 8.4.3 为每个 socket 创建 hart array

循环中的检查：

```c
riscv_socket_check_hartids(machine, i)
riscv_socket_first_hartid(machine, i)
riscv_socket_hart_count(machine, i)
```

分别保证该 socket 的 hart ID 连续、取得第一个 hart ID，以及取得 hart 数量。失败时直接退出，避免生成无法描述的 CPU 拓扑。

随后：

```c
soc_name = g_strdup_printf("soc%d", i);
object_initialize_child(OBJECT(machine), soc_name, &s->soc[i],
                        TYPE_RISCV_HART_ARRAY);
```

把每个 hart array 初始化为 machine 的 QOM 子对象，名字为 `soc0`、`soc1` 等。成为子对象后，其生命周期由父 machine 管理。

接着设置三个属性：

```c
"cpu-type"    = machine->cpu_type
"hartid-base" = base_hartid
"num-harts"   = hart_count
```

含义分别是 CPU 型号、第一个 hart 的 ID，以及该 socket 中的 hart 数。`sysbus_realize()` 最终实例化 hart array，并在内部真正创建所有 RISC-V CPU。

当前没有在 realize 之前设置 hart array 的 `resetvec` 属性，因此它沿用 QEMU RISC-V 的默认值 `0x1000`。

#### 8.4.4 创建 DRAM

```c
memory_region_init_ram(main_mem, NULL, "riscv_quard_star_board.dram",
                       machine->ram_size, &error_fatal);
memory_region_add_subregion(system_memory, 0x80000000, main_mem);
```

创建由 `-m` 决定大小的可读写内存，并映射到客体物理地址 `0x80000000`。`run.sh` 使用 `-m 1G`，所以实际区间为 `0x80000000` 到 `0xbfffffff`。

#### 8.4.5 创建 SRAM

```c
memory_region_init_ram(sram_mem, NULL, "riscv_quard_star_board.sram",
                       0x8000, &error_fatal);
memory_region_add_subregion(system_memory, 0x8000, sram_mem);
```

创建 32 KiB 可读写 SRAM，映射到 `0x8000`。

#### 8.4.6 创建 MROM

```c
memory_region_init_rom(mask_rom, NULL, "riscv_quard_star_board.mrom",
                       0x8000, &error_fatal);
memory_region_add_subregion(system_memory, 0x0, mask_rom);
```

创建 32 KiB 客体只读的 mask ROM，映射到 `0x0`。QEMU 自己仍可以在初始化阶段用 ROM blob 填入复位指令。

#### 8.4.7 写入复位向量

```c
quard_star_setup_rom_reset_vec(machine, &s->soc[0],
                               0x0, 0x0, 0x8000,
                               0x0, 0x0);
```

按参数顺序，它表示：

- 用 `soc[0]` 判断 RV32/RV64；
- 跳转目标 `start_addr = 0`；
- 将复位跳板写入 `rom_base = 0`；
- ROM 大小为 32 KiB；
- `kernel_entry = 0`；
- `fdt_load_addr = 0`。

因此即使 CPU 执行到这段跳板，它最终也会 `jr 0`，重新跳回跳板开头，形成循环；`a1` 也是 0，表示没有有效设备树。

此外，CPU 的默认复位 PC 是 `0x1000`，而跳板被写在 `0x0`。实际用 `-S` 暂停机器并查看寄存器，PC 确实是：

```text
pc = 0x0000000000001000
```

所以启动地址与跳板地址本身也没有对齐。在 `0x1000` 取到的全零内容会触发非法指令异常；由于初始 `mtvec` 也是 0，CPU 可能因异常落到 `0x0`，但这不是正确的复位启动路径，之后仍会因 `start_addr = 0` 而循环。

### 8.5 空的 instance 初始化函数

```c
static void quard_star_machine_instance_init(Object *obj)
{
}
```

QOM 在创建每个对象实例时调用它。当前没有实例默认属性需要设置，所以函数为空。保留它便于以后加入实例级初始化，但当前也可以不注册该回调。

### 8.6 machine class 初始化

```c
MachineClass *mc = MACHINE_CLASS(oc);
```

把 QOM class 转换成 `MachineClass`，随后填写所有 quard-star 实例共享的类型属性。

```c
mc->desc = "RISC-V Quard Star board";
mc->init = quard_star_machine_init;
mc->max_cpus = QUARD_STAR_CPUS_MAX;
mc->default_cpu_type = TYPE_RISCV_CPU_BASE;
```

- `desc` 是 `-machine help` 中显示的说明；
- `init` 指定真正创建板级硬件的函数；
- 最大 CPU 数为 8；
- 默认 CPU 随构建目标选择：`qemu-system-riscv64` 使用基础 `rv64` CPU，32 位版本使用基础 `rv32` CPU。

```c
mc->pci_allow_0_address = true;
```

允许 PCI BAR 使用地址 0。这是从 `virt` 板继承来的设置，但本板当前没有创建 PCI host bridge，因此暂时没有实际作用。

```c
mc->possible_cpu_arch_ids = riscv_numa_possible_cpu_arch_ids;
mc->cpu_index_to_instance_props = riscv_numa_cpu_index_to_props;
mc->get_default_cpu_node_id = riscv_numa_get_default_cpu_node_id;
mc->numa_mem_supported = true;
```

这些回调让 QEMU 能按照 RISC-V 的规则描述 CPU 与 NUMA node 的对应关系，并声明支持 NUMA 内存配置。它们也是从 `virt` 板沿用的框架代码；当前板级实现仍然只创建一整块平坦 DRAM，没有逐 NUMA node 建立独立内存区域。

### 8.7 QOM `TypeInfo` 与注册

```c
static const TypeInfo quard_star_machine_typeinfo = {
    .name          = MACHINE_TYPE_NAME("quard-star"),
    .parent        = TYPE_MACHINE,
    .class_init    = quard_star_machine_class_init,
    .instance_init = quard_star_machine_instance_init,
    .instance_size = sizeof(RISCVVirtState),
};
```

它定义一个名为 `quard-star`、继承自通用 `machine` 的 QOM 类型，并指定 class 初始化函数、instance 初始化函数和每个实例所需内存大小。

```c
static void quard_star_machine_init_register_types(void)
{
    type_register_static(&quard_star_machine_typeinfo);
}

type_init(quard_star_machine_init_register_types)
```

QEMU 模块初始化时调用注册函数。完成注册后，命令行机器解析器才能通过 `-M quard-star` 找到这个类型。这也是为什么现有编译结果的 `-machine help` 已经会列出：

```text
quard-star    RISC-V Quard Star board
```

## 9. `run.sh`

```sh
SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
```

取得脚本所在目录，使 QEMU 二进制路径不依赖当前工作目录。

```sh
$SHELL_FOLDER/output/qemu/bin/qemu-system-riscv64 \
    -M quard-star \
    -m 1G \
    -smp 8 \
    -nographic --parallel none
```

各参数含义：

- 启动本仓库安装出的 64 位 RISC-V system emulator；
- `-M quard-star` 选择新注册的板型；
- `-m 1G` 创建 1 GiB DRAM；
- `-smp 8` 创建 8 个 hart，正好达到该板定义的上限；
- `-nographic` 禁用图形窗口，并把串口/monitor 转到终端；
- `--parallel none` 禁用并口后端。

脚本没有传入 `-bios`、`-kernel` 或磁盘镜像。更关键的是，即使传入 `-kernel`，当前 machine init 也没有调用 RISC-V loader 去加载它。

由于 UART 只在地址表中声明、并没有实例化，`-nographic` 也没有可供客体输出的板载串口。当前脚本的预期现象就是 QEMU 进程启动后没有客体输出，并持续运行。

## 10. 当前版本的实际结果与主要缺口

我用已经构建出的 QEMU 6.0.0 做了只读检查：

- `-machine help` 能看到 `quard-star`，证明配置、编译和 QOM 注册链已经生效；
- `-M quard-star -m 1G -smp 8 -S` 能成功创建 8 个 hart；
- `info mtree` 只看到 MROM、SRAM 和 DRAM，确认 UART0 并未映射；
- `info registers` 显示初始 PC 为 `0x1000`，确认它和位于 `0x0` 的 reset vector 不一致。

如果目标是逐步教学，这一提交很像“先让自定义板型出现，并搭好 CPU 与内存”的阶段。后续至少需要按下面的依赖顺序补齐：

```text
correct reset PC / reset vector
            |
            v
load firmware (for example OpenSBI) into a real start address
            |
            v
create FDT and pass its address in a1
            |
            v
add CLINT + PLIC
            |
            v
instantiate UART and connect UART IRQ to PLIC
            |
            v
load kernel/initrd and describe all devices in FDT
```

最直接的参考实现是 QEMU 6.0.0 自带的 `hw/riscv/virt.c` 和 `hw/riscv/boot.c`。本次代码已经从这两处借用了 machine、hart array 和 reset vector 的基本结构，只是尚未把完整启动链和外设链迁移过来。
