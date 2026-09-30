# `qemu-system-riscv64 -M help` 执行路径分析

本文记录一次不依赖大模型、以“提出假设—搜索源码—二进制验证—调试器验证”为主线的传统大型 C 项目分析过程。分析对象是当前仓库构建出的 QEMU 6.0.0。

## 先给结论

`./output/qemu/bin/qemu-system-riscv64 -M help` 最终输出 machine 列表的核心代码是：

```text
qemu-6.0.0/softmmu/vl.c:1535  machine_parse()
qemu-6.0.0/softmmu/vl.c:1540  判断 name 是否是 "help" 或 "?"
qemu-6.0.0/softmmu/vl.c:1541  输出 "Supported machines are:"
qemu-6.0.0/softmmu/vl.c:1542  排序 machine class
qemu-6.0.0/softmmu/vl.c:1543  遍历并输出每个 machine
qemu-6.0.0/softmmu/vl.c:1552  exit(0)
```

完整的关键调用链是：

```text
softmmu/main.c:47  main(argc, argv, envp)
    |
    +-- softmmu/main.c:50  qemu_init(argc, argv, envp)
          |
          +-- softmmu/vl.c:2666  qemu_init_subsystems()
          |     `-- softmmu/runstate.c:761
          |         module_call_init(MODULE_INIT_QOM)
          |         （注册各个 RISC-V machine 类型）
          |
          +-- softmmu/vl.c:2677  lookup_opt()  第一次参数扫描
          |
          +-- softmmu/vl.c:2701  lookup_opt()  第二次参数扫描
          |     `-- softmmu/vl.c:3192  case QEMU_OPTION_M
          |         `-- qemu_opts_parse_noisily(..., "help", true)
          |             `-- 将短写的 "help" 保存为 machine 的 type="help"
          |
          `-- softmmu/vl.c:3545
              qemu_create_machine(select_machine())
                                  |
                                  +-- softmmu/vl.c:1616
                                  |   object_class_get_list(TYPE_MACHINE,
                                  |                         false)
                                  +-- softmmu/vl.c:1627
                                  |   qemu_opt_get(opts, "type") -> "help"
                                  `-- softmmu/vl.c:1629
                                      machine_parse("help", machines)
                                          `-- 打印列表并 exit(0)
```

这里有一个很重要的 C 语言求值细节：源代码写的是
`qemu_create_machine(select_machine())`，必须先求出实参
`select_machine()`，才能调用 `qemu_create_machine()`。但是
`select_machine()` 内部的 `machine_parse("help", ...)` 已经调用
`exit(0)`，所以本次命令实际上**不会进入** `qemu_create_machine()`。
同理，它也不会回到 `main()` 执行 `qemu_main_loop()` 和
`qemu_cleanup()`，更不会运行某块板子的 `*_machine_init()`。

## 1. 从运行现象建立搜索锚点

先运行程序，同时分开保存标准输出、标准错误和退出码：

```bash
./output/qemu/bin/qemu-system-riscv64 -M help \
    >/tmp/qemu-m-help.out 2>/tmp/qemu-m-help.err
echo $?
cat /tmp/qemu-m-help.out
cat /tmp/qemu-m-help.err
```

本仓库当前结果为：

```text
exit code: 0

stdout:
Supported machines are:
microchip-icicle-kit Microchip PolarFire SoC Icicle Kit
none                 empty machine
quard-star           RISC-V Quard Star board
sifive_e             RISC-V Board compatible with SiFive E SDK
sifive_u             RISC-V Board compatible with SiFive U SDK
spike                RISC-V Spike board (default)
virt                 RISC-V VirtIO board

stderr:
yoooo !!!! mother fucking world !!!!
```

标准错误中的这一行来自当前工作区对 `softmmu/main.c:49` 的本地修改。
它说明运行时确实经过了已知的 `main()`，也可以作为后续判断“二进制是否
由当前源码构建”的简单探针。

传统分析中，运行输出里的固定字符串是最便宜、最可靠的源码锚点。因此先搜：

```bash
rg -n 'Supported machines are:' qemu-6.0.0
```

它会直接定位到 `softmmu/vl.c:1541`。向上阅读所在函数，立即看到
`machine_parse()` 的 `help` 分支、遍历打印和 `exit(0)`。这已经得到
“输出是哪块代码产生的”这个问题的直接答案，但还没有解释控制流如何到达这里。

## 2. 从 `main()` 向下追踪

搜索入口：

```bash
rg -n 'int main\s*\(|main\s*\(int argc' \
    qemu-6.0.0/softmmu qemu-6.0.0
```

`softmmu/main.c:47-54` 的主体很短：

```c
int main(int argc, char **argv, char **envp)
{
    fprintf(stderr, "yoooo !!!! mother fucking world !!!!\n");
    qemu_init(argc, argv, envp);
    qemu_main_loop();
    qemu_cleanup();
    return 0;
}
```

声明可由 `rg -n 'qemu_init\s*\('` 找到，定义位于
`softmmu/vl.c:2618`。所以接下来围绕 `qemu_init()` 查找命令行解析。

## 3. 确认 `-M` 的定义

在大型项目中，选项往往由宏表生成，直接搜索 `case '-M'` 通常找不到。
搜索枚举名和选项文本更有效：

```bash
rg -n 'QEMU_OPTION_M|DEF\("M"|DEF\("machine"' \
    qemu-6.0.0/softmmu/vl.c qemu-6.0.0/qemu-options.hx
```

得到两处关键信息：

- `qemu-options.hx:25` 定义 `-machine`，参数形式为
  `[type=]name[,prop=value...]`。
- `qemu-options.hx:125-126` 定义 `-M`，注释明确写着
  `Deprecated by -machine`。因此 `-M help` 等价于 `-machine help`。

`softmmu/vl.c:882-887` 中的 `qemu_options[]` 通过
`qemu-options-wrapper.h` 把这个宏表展开成运行时查询表。之后
`lookup_opt()` 在 `softmmu/vl.c:1574-1612` 中线性查询该表；因为
`-M` 带 `HAS_ARG`，它还会取下一个 `argv` 元素作为 `optarg`，即
`"help"`。

## 4. 注意 QEMU 对参数扫描了两遍

阅读 `qemu_init()` 可见：

- `softmmu/vl.c:2668-2684` 是第一遍，只优先处理
  `-nouserconfig`，以决定是否读取默认配置文件；
- `softmmu/vl.c:2690` 开始第二遍，才执行各选项的主要逻辑。

所以单步调试时看到 `lookup_opt()` 对 `-M` 命中两次并不是循环错误。
第二遍在 `softmmu/vl.c:3192-3199` 进入：

```c
case QEMU_OPTION_M:
case QEMU_OPTION_machine:
    olist = qemu_find_opts("machine");
    opts = qemu_opts_parse_noisily(olist, optarg, true);
    if (!opts) {
        exit(1);
    }
    break;
```

最后一个参数 `true` 允许省略第一个选项名。对应的 machine 选项表在
`softmmu/vl.c:238-251`：

```c
static QemuOptsList qemu_machine_opts = {
    .name = "machine",
    .implied_opt_name = "type",
    .merge_lists = true,
    ...
};
```

`util/qemu-option.c:895-926` 会在允许短写时取
`list->implied_opt_name`；`util/qemu-option.c:768-775` 再把没有
`=` 的首项按这个名字保存。因此：

```text
-M help
    -> optarg = "help"
    -> qemu_opts_parse_noisily(machine_opts, "help", true)
    -> machine option: type = "help"
```

注意，这里的 `qemu_opts_parse_noisily()` 并不负责打印 machine 列表；
它只把选项保存起来。实际打印发生得更晚。

## 5. 从保存的 `type=help` 找到最终分支

继续阅读到 `softmmu/vl.c:3545`：

```c
qemu_create_machine(select_machine());
```

`select_machine()` 位于 `softmmu/vl.c:1614-1640`，它做三件与本问题
直接相关的事：

1. `object_class_get_list(TYPE_MACHINE, false)` 取得所有非抽象 machine
   子类；
2. 从 machine 选项中读出 `type`；此时值为 `"help"`；
3. 调用 `machine_parse(optarg, machines)`。

`is_help_option()` 定义在 `include/qemu/help_option.h:17-20`，接受
`"help"` 和为了兼容而保留的 `"?"`。于是进入
`softmmu/vl.c:1540-1552` 的打印分支。

列表不是硬编码的字符串数组，而是 QOM（QEMU Object Model）中所有
`TYPE_MACHINE` 子类的 `MachineClass`。`machine_class_cmp()` 决定排序；
循环读取每个类的 `name`、`desc`、`alias`、`is_default` 和
`deprecation_reason`。这也解释了输出中的描述、`(default)` 等文字。

## 6. machine 列表是怎样来的

`qemu_init()` 早期调用 `qemu_init_subsystems()`；后者在
`softmmu/runstate.c:761` 执行：

```c
module_call_init(MODULE_INIT_QOM);
```

各板级文件末尾的 `type_init(...)` 宏把注册函数归入
`MODULE_INIT_QOM`。宏定义见 `include/qemu/module.h:35-56`。注册函数再
调用 `type_register_static()`，把 `.parent = TYPE_MACHINE` 的
`TypeInfo` 注册到 QOM。

本次 riscv64 构建启用了哪些板子，由
`default-configs/devices/riscv64-softmmu.mak:11-16` 决定；
`hw/riscv/meson.build:5-11` 决定相应源文件是否参加编译。输出项与源码
的对应关系如下：

| 输出名称 | 描述/默认属性来源 | QOM 注册位置 |
| --- | --- | --- |
| `microchip-icicle-kit` | `hw/riscv/microchip_pfsoc.c:527` | 同文件 `:545-556` |
| `none` | `hw/core/null-machine.c:48` | 同文件 `:60` 的 `DEFINE_MACHINE` |
| `quard-star` | `hw/riscv/quard_star.c:166` | 同文件 `:178-190` |
| `sifive_e` | `hw/riscv/sifive_e.c:144` | 同文件 `:157-169` |
| `sifive_u` | `hw/riscv/sifive_u.c:702` | 同文件 `:719-731` |
| `spike` | `hw/riscv/spike.c:316`，`:319` 设为默认 | 同文件 `:328-340` |
| `virt` | `hw/riscv/virt.c:776` | 同文件 `:790-802` |

例如注册名实际是 `"virt-machine"`。`include/hw/boards.h:15-22`
定义了 `MACHINE_TYPE_NAME("virt")` 的 `-machine` 后缀；
`hw/core/machine.c:902-915` 初始化 `MachineClass` 时去掉该后缀，得到
命令行和列表显示的 `"virt"`。

`opentitan.c` 虽然也在源码树中，但 riscv64 配置没有启用
`CONFIG_OPENTITAN`；它只在 riscv32 默认配置中启用，所以本次列表没有
`opentitan`。这类“源码存在但运行时不可见”的现象必须结合构建配置
判断，不能只凭目录内容下结论。

## 7. 用二进制工具验证源码和产物的关系

安装后的文件是 stripped，不能直接依靠它查看完整静态符号：

```bash
file output/qemu/bin/qemu-system-riscv64
```

但构建目录保留了带 DWARF 调试信息、未 strip 的对应文件：

```bash
file qemu-6.0.0/build/qemu-system-riscv64
readelf -S qemu-6.0.0/build/qemu-system-riscv64 | rg 'debug|symtab'
```

用下面的命令比较二者 Build ID：

```bash
readelf -n output/qemu/bin/qemu-system-riscv64 | rg -A1 'Build ID'
readelf -n qemu-6.0.0/build/qemu-system-riscv64 | rg -A1 'Build ID'
```

二者都是：

```text
ea4bf1cdd0e41335bcf7b00d23bafc6c9d8ea88f
```

因此可以用构建目录的未 strip 文件调试安装后的同一份代码。进一步：

```bash
nm -an qemu-6.0.0/build/qemu-system-riscv64 | \
    rg ' (main|qemu_init|lookup_opt|machine_class_cmp)$'

addr2line -afiC -e qemu-6.0.0/build/qemu-system-riscv64 \
    0x2bc410 0x6169c0 0x616ac0 0x6193b0

objdump -d --line-numbers --disassemble=main \
    qemu-6.0.0/build/qemu-system-riscv64
```

当前构建能把 `main` 映射到 `softmmu/main.c:48`，把 `qemu_init`
映射到 `softmmu/vl.c:2619`；反汇编也能看到 `main` 对 `qemu_init`、
`qemu_main_loop` 和 `qemu_cleanup` 的调用。

`file` 同时显示该程序是宿主机上运行的 x86-64 ELF。文件名中的
`riscv64` 指它模拟的目标体系结构，并不表示这个 QEMU 可执行文件本身
是 RISC-V 指令。

## 8. 用 GDB 做最终控制流验证

优化构建可能把 `machine_parse()`、`select_machine()` 内联，所以 `nm`
未必列出独立符号；DWARF 仍可能让 GDB 对源码函数下断点。可执行：

```bash
gdb -q qemu-6.0.0/build/qemu-system-riscv64
```

然后在 GDB 中输入：

```gdb
set pagination off
break main
break qemu_init
break lookup_opt
break machine_parse
break qemu_create_machine
run -M help
```

每次停下用以下命令检查：

```gdb
bt
info args
print name
continue
```

本次实测顺序为：

```text
main(argc=3, argv[1]="-M", argv[2]="help")
qemu_init(...)
lookup_opt(... argv[1]="-M")       # 第一遍
lookup_opt(... argv[1]="-M")       # 第二遍
machine_parse(name="help", ...)
```

在 `machine_parse` 处得到的栈是：

```text
#0 machine_parse(name="help", machines=...)
#1 select_machine()
#2 qemu_init(...) at softmmu/vl.c:3545
#3 main(...)      at softmmu/main.c:50
```

之后程序打印列表并正常退出。`qemu_create_machine` 断点没有命中，和
前面的静态推理一致。

## 9. 传统大型项目分析方法总结

这次分析实际遵循的是一套可迁移的方法：

1. **先复现并记录边界条件**：完整命令、stdout、stderr、退出码。
2. **用独特输出字符串反查源码**：比一开始通读目录高效得多。
3. **从已知入口向下读**：只跟与当前输入有关的分支。
4. **同时从目标字符串向上找调用者**：上下两条路线在某个函数会合。
5. **识别代码生成和宏表**：QEMU 的选项表和 QOM 注册大量使用宏。
6. **检查构建配置**：源码存在不等于进入当前目标程序。
7. **静态证据和动态证据互证**：`rg`/源码阅读提出调用链，GDB 验证
   真正走过的路径。
8. **区分“准备工作”和“目标行为”**：本次会执行相当多的 QEMU/QOM
   初始化，但打印列表的业务核心仍是 `machine_parse()`。
9. **明确没有执行什么**：遇到 `exit()`、`return`、错误分支时，不能把
   源码上排在后面的函数误认为实际执行过。

对于更复杂、没有明显输出字符串的问题，传统做法通常是继续加入断点、
临时日志、`strace`/`ltrace`、调用图或覆盖率数据；原则仍然是先缩小范围，
再用运行时证据验证，而不是一次性理解整个项目。
