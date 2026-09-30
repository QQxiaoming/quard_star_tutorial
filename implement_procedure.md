# 在 QEMU 6.0.0 中添加最小 RISC-V `niubi` 板

## 1. 目标与最终结果

目标不是做一块能启动 Linux 的完整开发板，而是做一块符合 QEMU 现有接口、能被正常创建的最小 RISC-V 板：

- 命令行名字：`niubi`
- 描述：`RISC-V Niubi board`
- 一个 RV64 hart
- 默认 128 MiB RAM
- RAM 基址：`0x80000000`
- CPU 复位地址：默认也是 `0x80000000`
- 支持用 `-kernel` 装载 ELF、uImage 或裸二进制，并把 CPU 复位地址改为镜像入口
- 没有串口、中断控制器、定时器、设备树、固件和其他外设

所以对于当前实现：

```
qemu-system-riscv64 -M niubi -kernel program.elf
```

启动链非常短：

```
QEMU 加载 program.elf
    → 读取 ELF 入口
    → 设置 CPU resetvec
    → CPU 直接从入口执行
```

中间没有 OpenSBI、U-Boot 或其他 bootloader。
```

最终可以看到：

```text
$ ./output/qemu/bin/qemu-system-riscv64 -M help
...
niubi                RISC-V Niubi board
...
```

这块板可以作为研究 QEMU machine/QOM 接口的骨架，也可以运行只依赖 CPU 和 RAM 的裸机代码。它不能直接启动常规 Linux，因为 Linux 通常还需要定时器、中断控制器、串口、设备树，以及 SBI 实现或 M 模式固件。

## 2. 从已知点继续追：`machine_parse()` 的列表从哪来

博客已经找到：

```text
softmmu/main.c:main
    -> softmmu/vl.c:qemu_init
        -> machine_parse
```

下一步不应该立刻全局搜索字符串 `virt`，因为这个词过于常见。更有效的问题是：`machine_parse()` 的参数 `machines` 是谁构造的？

```bash
rg -n "machine_parse|select_machine|object_class_get_list" \
    qemu-6.0.0/softmmu/vl.c qemu-6.0.0/qom
```

在 `softmmu/vl.c:select_machine()` 中能看到：

```c
GSList *machines = object_class_get_list(TYPE_MACHINE, false);
```

所以 `-M help` 不是读取一张手写的字符串表。它向 QOM（QEMU Object Model）索取所有继承自 `TYPE_MACHINE` 的非抽象 class，然后打印每个 `MachineClass` 的 `name` 和 `desc`。

这把问题从“如何修改 `machine_parse()`”变成了“如何注册一个 `TYPE_MACHINE` 子类型”。`machine_parse()` 不需要修改。

## 3. 机器类型是如何进入 QOM 的

搜索现有机器的注册方式：

```bash
rg -n "DEFINE_MACHINE|MACHINE_TYPE_NAME|type_register_static" \
    qemu-6.0.0/hw/riscv qemu-6.0.0/include/hw/boards.h
```

`include/hw/boards.h` 中的 `DEFINE_MACHINE` 宏已经封装了最小注册样板。简化展开后，它做的是：

```c
static void xxx_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    xxx(mc);
}

static const TypeInfo xxx_typeinfo = {
    .name = MACHINE_TYPE_NAME("xxx"),
    .parent = TYPE_MACHINE,
    .class_init = xxx_class_init,
};

static void xxx_register_types(void)
{
    type_register_static(&xxx_typeinfo);
}

type_init(xxx_register_types)
```

其中：

- `MACHINE_TYPE_NAME("niubi")` 得到 QOM 内部名字 `niubi-machine`。
- `.parent = TYPE_MACHINE` 使它能被 `object_class_get_list(TYPE_MACHINE, false)` 找到。
- `machine_class_base_init()` 会检查类型名必须以 `-machine` 结尾，并剥掉后缀，把公开名字设为 `niubi`。
- `type_init()` 把注册函数放进 `MODULE_INIT_QOM` 初始化队列。
- `qemu_init_subsystems()` 调用 `module_call_init(MODULE_INIT_QOM)`，所有 machine 类型因此在解析 `-M` 以前已经注册。

相关调用链是：

```text
ELF constructor
  -> register_module_init(..., MODULE_INIT_QOM)
qemu_init()
  -> qemu_init_subsystems()
    -> module_call_init(MODULE_INIT_QOM)
      -> niubi_machine_init_register_types()
        -> type_register_static()
...
select_machine()
  -> object_class_get_list(TYPE_MACHINE, false)
  -> machine_parse()
```

这里有一个容易误判的地方：只在 `machine_parse()` 附近加一条输出，最多能证明控制流经过那里，却不能解释列表的数据来源。继续追参数的生产者，才能找到 QOM 这个真正的扩展接口。

## 4. 横向阅读已有板子，提炼“加板规范”

我按“从最小到复杂”的顺序查看了这些文件：

```bash
sed -n '1,100p' qemu-6.0.0/hw/core/null-machine.c
sed -n '1,180p' qemu-6.0.0/hw/riscv/opentitan.c
sed -n '1,220p' qemu-6.0.0/hw/riscv/sifive_e.c
sed -n '1,240p' qemu-6.0.0/hw/riscv/quard_star.c
rg -n "riscv_load_kernel|TYPE_RISCV_HART_ARRAY|resetvec" \
    qemu-6.0.0/hw/riscv qemu-6.0.0/target/riscv
```

得到的结论如下。

| 样本 | 学到的规则 |
| --- | --- |
| `hw/core/null-machine.c` | 只想注册名字时，`DEFINE_MACHINE` 加一个空 init 就够了；但这样的 machine 没有 CPU，不算一块有用的板。 |
| `hw/riscv/opentitan.c` | 简单 machine 可以直接使用 `DEFINE_MACHINE`；class 至少设置 `desc`、`init`、`max_cpus` 和 `default_cpu_type`。 |
| `hw/riscv/sifive_e.c` | 板初始化负责创建 CPU/SoC、映射 MemoryRegion，并规定 reset vector。 |
| `hw/riscv/quard_star.c` | 当 machine 自己需要保存复杂实例状态或嵌入多个子对象时，应定义专有 `MachineState` 和完整 `TypeInfo`。 |
| `hw/riscv/riscv_hart.c` | RISC-V 已经提供 hart array，属性包括 `cpu-type`、`num-harts`、`hartid-base` 和 `resetvec`；新板应复用它。 |
| `hw/riscv/boot.c` | `riscv_load_kernel()` 按 ELF、uImage、裸镜像依次尝试，并返回真正入口地址。 |

由此可以把最小但真实可创建的板级契约概括为：

1. 注册一个以 `TYPE_MACHINE` 为父类的 QOM 类型。
2. 填写命令行描述和板初始化回调。
3. 指定默认 CPU 类型及 CPU 数量约束。
4. 在板初始化回调里至少创建 CPU，并为 CPU 提供可取指的地址空间。
5. 源文件必须通过目标架构的 Kconfig 和 Meson source set 进入最终链接。

`niubi` 没有自己的可变属性或复杂实例状态，所以使用 `DEFINE_MACHINE`，没有照抄 `quard_star` 的完整 `TypeInfo`。这是根据需求选择现有最小接口，而不是删除必要结构。

## 5. 硬件模型的最小设计

内存图只有一项：

```text
0x80000000 +--------------------------+
           | RAM                      |
           | 默认 128 MiB，可用 -m 改 |
           +--------------------------+
```

CPU 设计：

- `max_cpus = 1`，因此 `-smp 2` 应被 QEMU 公共 machine 层拒绝。
- 默认 CPU 是 `TYPE_RISCV_CPU_BASE`。在 riscv64-softmmu 构建里，它展开为 `rv64-riscv-cpu`。
- 使用 `TYPE_RISCV_HART_ARRAY` 创建一个 hart，不重复实现 hart ID、CPU realize 和 reset 回调。
- 没有 `-kernel` 时，PC 从空 RAM 的 `0x80000000` 开始；这适合调试板模型，但没有有效程序可运行。
- 有 `-kernel` 时，先把镜像装入 RAM，再把 hart 的 `resetvec` 设置为加载器返回的入口地址。

没有创建 mask ROM 或 reset trampoline，因为本实验没有 OpenSBI/内核参数/设备树之间的跳转协议。对于“CPU 直接进入裸机入口”的板，它们不是必要条件。

## 6. 实际修改的文件

### 6.1 `hw/riscv/niubi.c`

核心初始化顺序是：

```text
取得 system memory
  -> 在 0x80000000 映射 machine->ram_size 大小的 RAM
  -> 可选：装载 -kernel 并取得入口地址
  -> 创建 TYPE_RISCV_HART_ARRAY
  -> 设置 cpu-type、num-harts=1、resetvec
  -> realize hart array
```

先映射 RAM、再装镜像、最后 realize CPU，使 CPU 第一次复位时已经拥有正确入口和可取指内存。

class 初始化设置：

```c
mc->desc = "RISC-V Niubi board";
mc->init = niubi_board_init;
mc->max_cpus = 1;
mc->default_cpu_type = TYPE_RISCV_CPU_BASE;
mc->default_ram_size = 128 * MiB;
```

串口、并口、软驱、光驱和 SD 卡默认设备都被显式关闭，以免一个声称“只有 CPU 和 RAM”的板被公共代码悄悄加上无关设备。

最后一行：

```c
DEFINE_MACHINE("niubi", niubi_machine_init)
```

负责生成 `niubi-machine` 的 `TypeInfo` 和 QOM 注册入口。

### 6.2 `hw/riscv/Kconfig`

加入：

```text
config NIUBI
    bool
```

它没有 `select` 其他设备，因为 hart 和 RAM 都属于此目标已有的核心设施。

### 6.3 `default-configs/devices/riscv64-softmmu.mak`

加入：

```text
CONFIG_NIUBI=y
```

这表示默认的 `riscv64-softmmu` 构建包含该板。目前没有把它默认加入 riscv32 构建，因为需求明确针对 `qemu-system-riscv64`。

### 6.4 `hw/riscv/meson.build`

加入：

```meson
riscv_ss.add(when: 'CONFIG_NIUBI', if_true: files('niubi.c'))
```

Kconfig 决定功能是否开启，Meson source set 决定开启时编译哪个源文件。两者缺一不可：只注册 Kconfig 不会编译 C 文件；只把文件写进 Meson 而没有启用配置，也不会进入目标。

## 7. 构建和逐层验证

### 7.1 构建

```bash
cd qemu-6.0.0
make -j4
make install
```

修改 Kconfig、默认配置或 `meson.build` 后，构建过程会重新生成 Meson 文件。不能只盯着源文件是否编译，还要检查生成配置：

```bash
rg -n "CONFIG_NIUBI" \
    build/riscv64-softmmu-config-devices.mak \
    build/riscv64-softmmu-config-devices.h
```

本次得到：

```text
build/riscv64-softmmu-config-devices.mak:CONFIG_NIUBI=y
build/riscv64-softmmu-config-devices.h:#define CONFIG_NIUBI 1
```

编译日志中也出现：

```text
Compiling C object libqemu-riscv64-softmmu.fa.p/hw_riscv_niubi.c.o
Linking target qemu-system-riscv64
```

### 7.2 验证注册，而不只验证编译

```bash
./qemu-6.0.0/build/qemu-system-riscv64 -M help | rg niubi
```

结果：

```text
niubi                RISC-V Niubi board
```

还可以用传统二进制工具验证注册函数确实在最终 ELF 中：

```bash
nm -an qemu-6.0.0/build/qemu-system-riscv64 | rg niubi
```

能看到 `niubi_machine_init_register_types`、`niubi_machine_init_class_init`、`niubi_board_init` 和 `niubi_machine_init_typeinfo` 等局部符号。

这组检查把故障范围分开了：

- 生成配置里没有 `CONFIG_NIUBI`：检查 default config/Kconfig。
- 配置存在但没有 `.o`：检查 `meson.build` 条件。
- `.o` 存在但最终 ELF 没符号：检查 source set 是否链接到目标、是否被链接器丢弃。
- ELF 有注册符号但 `-M help` 没名字：检查 `type_init()`、父类型和类型名后缀。

### 7.3 验证 machine 真能创建

仅有 `-M help` 还不够，它只证明 class 注册成功，不证明 `MachineClass.init` 能执行完。使用 QMP 启动暂停态虚拟机并查询 CPU：

```bash
printf '%s\n' \
  '{"execute":"qmp_capabilities"}' \
  '{"execute":"query-cpus-fast"}' \
  '{"execute":"quit"}' |
./qemu-6.0.0/build/qemu-system-riscv64 \
  -M niubi -m 16M -S -display none -monitor none -qmp stdio
```

关键返回值：

```json
{
  "qom-path": "/machine/unattached/device[0]/harts[0]",
  "cpu-index": 0,
  "target": "riscv64"
}
```

这证明不是只有名字，而是 hart array 确实 realize 出了一个 RV64 CPU。

### 7.4 验证物理内存图

通过 QMP 的 `human-monitor-command` 执行 `info mtree`，本次 `-m 16M` 得到：

```text
address-space: memory
  0000000000000000-ffffffffffffffff (prio 0, i/o): system
    0000000080000000-0000000080ffffff (prio 0, ram): niubi.ram
```

`0x80000000` 到 `0x80ffffff` 正好是 16 MiB，证明 `-m` 进入了 `machine->ram_size`，且内存被映射到设计地址。

### 7.5 验证公共 machine 约束

```bash
./qemu-6.0.0/build/qemu-system-riscv64 \
  -M niubi -smp 2 -display none
```

预期且实际得到：

```text
Invalid SMP CPUs 2. The max CPUs supported by machine 'niubi' is 1
```

这说明 `MachineClass.max_cpus` 不只是描述字段，QEMU 公共层确实用它做参数校验。

### 7.6 验证 `-kernel` 入口确实成为复位 PC

为了避免只根据 `riscv_load_kernel()` 的源码推断行为，还制作了一个只有自跳转指令的 RISC-V ELF，并把链接入口设为 `0x80001000`：

```asm
.section .text
.globl _start
_start:
    j _start
```

```bash
riscv64-linux-gnu-as -o /tmp/niubi_smoke.o /tmp/niubi_smoke.S
riscv64-linux-gnu-ld -Ttext=0x80001000 -e _start \
  -o /tmp/niubi_smoke.elf /tmp/niubi_smoke.o
riscv64-linux-gnu-readelf -h /tmp/niubi_smoke.elf | rg "Entry point"
```

ELF 报告的入口是：

```text
Entry point address: 0x80001000
```

以 `-M niubi -kernel /tmp/niubi_smoke.elf -S` 启动，再通过 QMP/HMP 查询 `info registers`，得到：

```text
pc       ffffffff80001000
mhartid  0000000000000000
```

QEMU 的 RV64 寄存器显示对这个地址做了 64 位符号扩展；低位入口仍是 ELF 指定的 `0x80001000`。这证明 `riscv_load_kernel()` 返回的入口已实际传给 hart 的 `resetvec`。

## 8. 不借助大模型时，处理大型代码库的工作方法

这次修改所用的方法可以概括为以下循环：

1. **从已确认的动态入口开始。** 已知 `machine_parse()`，先看参数来自哪里，不在整个工程里盲目阅读。
2. **追数据，不只追控制流。** `-M help` 输出的是 `machines`；追到 `object_class_get_list()` 后，问题自然落到 QOM 注册。
3. **搜索抽象接口。** 根据 `TYPE_MACHINE` 搜索宏和 `TypeInfo`，比搜索具体板名更容易找到框架约束。
4. **选择多个对照样本。** `none` 给出注册下限，`opentitan` 给出简单 RISC-V machine，`sifive_e` 给出 reset/RAM，`quard_star` 展示何时需要自定义实例状态。
5. **先写最小假设。** 最小可创建板应是“machine type + CPU + 可取指 RAM”，先不添加 UART、PLIC、CLINT 和 FDT。
6. **分层验证。** 依次检查配置、目标文件、ELF 符号、`-M help`、machine 创建、CPU 对象、内存图和错误路径。
7. **每次失败只回退一层。** 例如列表没有名字时先查注册链，不应立刻怀疑 CPU realize；CPU 创建失败时才进入板初始化和设备属性。

传统源码分析并不是从第一页读到最后一页。它更像建立一串可以被命令或实验否证的小假设：

```text
列表来自静态数组？
  否：来自 object_class_get_list
所有 TYPE_MACHINE 子类都会进入列表？
  是：继续找子类注册模板
只注册 class 就能真正启动？
  否：还要提供 board init、CPU 和地址空间
自定义 MachineState 是强制的吗？
  否：没有实例状态时 DEFINE_MACHINE 已足够
```

`rg`、`sed`、预处理宏、生成的构建文件、`nm`、QMP/HMP 和故意制造的错误输入共同组成了证据链。大模型可以缩短找路径的时间，但可靠结论仍然应该落到这些可复现证据上。

## 9. 当前板子的边界与自然演进方向

当前 `niubi` 是教学骨架，不是 Linux 平台。若继续扩展，建议一次只加一个可验证部件，顺序可以是：

1. 加一个 mask ROM 和明确的 reset trampoline。
2. 加 NS16550A UART，写最小裸机程序验证串口输出。
3. 加 CLINT，验证 timer/software interrupt。
4. 加 PLIC 和一个外部中断源。
5. 生成 FDT。
6. 定义 OpenSBI 到 S-mode payload 的启动约定。
7. 最后再尝试启动 Linux。

这样每一步都有单独的可观测结果，不会在第一次启动失败时同时面对 CPU、内存、串口、中断、设备树和固件六类变量。

## 10. 关于工作树中原有调试输出

开始本任务前，工作树中的 `softmmu/main.c` 和 `softmmu/vl.c` 已经有未提交调试修改，分别输出：

```text
hello from MEEEE!!!
yoooooo!!!
```

本次实现没有修改或清理它们。因此验证输出里仍会出现这两行；它们不是 `niubi` 板实现的一部分。
