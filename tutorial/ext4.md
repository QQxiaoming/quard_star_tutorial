# 使用qemu的gdbstub调试

在之前我的一篇博客——《[kgdb调试linux内核以及驱动模块](https://blog.csdn.net/weixin_39871788/article/details/120313821)》中简要描述如何配置kgdb进行内核以及驱动模块调试，这种方法既适用于qemu仿真环境也适用于真实的物理开发板，但对于做早期cpu core bridge up时未必能在物理设备上搭建好稳定可靠的kgdb环境。一般来讲使用qemu仿真来发现一些早期软件问题是有一定的帮助的，因此本文将描述，借助qemu的gdbsever模式，在仿真环境上更早的介入gdb工具进行非常底层的debug。本文实验环境依然为qemu搭建的riscv64模拟器环境，笔者之前有系列博客详细描述了环境搭建，可供参考——《[基于qemu-riscv从0开始构建嵌入式linux系统](https://blog.csdn.net/weixin_39871788/article/details/118469061)》。

## 修改内核配置

与kgdb类似，使用为方便gdb加载内核符号，我们依然要在编译kernel是配置如下相关选项，本文不再赘述原因，详情查看之前的[文章](https://blog.csdn.net/weixin_39871788/article/details/120313821)。

- CONFIG_GDB_SCRIPTS set
- CONFIG_DEBUG_INFO set
- CONFIG_DEBUG_INFO_DWARF4 set
- CONFIG_STRICT_KERNEL_RWX/CONFIG_STRICT_MODULE_RWX not set

### 启动qemu

正常启动qemu时，目标板将立即启动，如要开启gdbsever并等待gdb连接，则仅需添加"-s -S"参数即可。

```shell
SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)

GRAPHIC_PARAM="-nographic --serial telnet::3441,server,nowait --serial telnet::3442,server,nowait --serial telnet::3443,server,nowait --monitor stdio --parallel none"

$SHELL_FOLDER/output/qemu/bin/qemu-system-riscv64 \
-M quard-star \
-m 1G \
-smp 8 \
-drive if=pflash,bus=0,unit=0,format=raw,file=$SHELL_FOLDER/output/fw/fw.bin \
-drive file=$SHELL_FOLDER/output/rootfs/rootfs.img,format=raw,id=drive0 \
-fsdev local,security_model=mapped-xattr,id=fsdev0,path=$SHELL_FOLDER \
-netdev user,id=net0,net=192.168.31.0/24,dhcpstart=192.168.31.100,hostfwd=tcp::3522-:22,hostfwd=tcp::3580-:80 \
-global virtio-mmio.force-legacy=false \
-device virtio-blk-device,id=hd0,drive=drive0 \
-device virtio-gpu-device,id=video0,xres=1280,yres=720\
-device virtio-mouse-device,id=input0 \
-device virtio-keyboard-device,id=input1 \
-device virtio-9p-device,id=fs0,fsdev=fsdev0,mount_tag=hostshare \
-device virtio-net-device,netdev=net0 \
-fw_cfg name="opt/qemu_cmdline",string="qemu_vc=:vn:24x80:" \
$GRAPHIC_PARAM -s -S
```

此时不会有任何输出，我们另开一个新终端，准备gdb连接。

### 启动gdb

首先编写一个gdb.script脚本。

```
add-auto-load-safe-path ./linux-5.10.42/
file ./linux-5.10.42/vmlinux 
source ./linux-5.10.42/vmlinux-gdb.py
target remote :1234
```

以上gdb命令分别用于

- 指定./linux-5.10.42/路径为可信的路径，便于gdb执行启动的python脚本；
- 指定符号文件./linux-5.10.42/vmlinux 
- 执行./linux-5.10.42/vmlinux-gdb.py添加环境用于kgdb的命令扩展
- 连接到本地:1234端口接受gdb数据包

启动GDB

```shell
riscv64-unknown-linux-gnu-gdb -x ./gdb.script
```

### 设置断点

gdb启动后，没有使用c执行前，目标仍是hart状态的，此时我们可以先添加断点，以便跟踪一些早期代码，这里输入

```shell
hbreak start_kernel
```

该命令设置硬件断点到linux-5.10.42/init/main.c:start_kernel函数，这里必须使用硬件断点而不是break软件断点，因为软件断点通过修改内存对应的指令实现，这在现阶段cpu完全没有boot的情况下可能无法实现断点功能。这里还存在一个问题，如果你使用该命令后报如下错误：

```shell
(gdb) hbreak start_kernel
Cannot access memory at address 0xffffffff809f619c
```

那么需要先执行以下命令允许qemu以物理内存方式设置硬件断点，再重新执行hbreak start_kernel即可。

```shell
maintenance packet Qqemu.PhyMemMode:1
```

接下来输入c命令，等待bootloader和uboot阶段结束后，将在断点出停下，之后就可以进行内核的跟踪了。

## 总结

OK，到这里之后很多调试方法和模块的调试方法与kgdb基本相同，qemu+gdb作为一种调试手段的补充，也许在一些特殊情况下能解决一些问题，尤其是嵌入式开发中。本文涉及到的代码均已开源，可以在 https://github.com/QQxiaoming/quard_star_tutorial找到。

### 参考资料
  
- https://futurewei-cloud.github.io/ARM-Datacenter/qemu/aarch64-debug-kernel/
- https://qemu-project.gitlab.io/qemu/system/gdb.html
