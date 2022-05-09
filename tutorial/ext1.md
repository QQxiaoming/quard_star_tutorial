# kgdb调试linux内核以及驱动模块

本文将简要描述如何配置kgdb进行内核以及驱动模块调试，以嵌入式开发为例，但同样对于其他有需要调试kernel有一定的参考价值。本文实验环境为qemu搭建的riscv64模拟器环境，笔者之前有系列博客详细描述了环境搭建，可供参考——《[基于qemu-riscv从0开始构建嵌入式linux系统](https://blog.csdn.net/weixin_39871788/article/details/118469061)》。

## 修改内核配置

在linux 5.10版本上，KGDB已经受支持，因此仅需要修改内核配置就可以打开此功能。

### CONFIG_KGDB

配置CONFIG_KGDB=y，以启用kgdb功能。

### CONFIG_GDB_SCRIPTS

配置CONFIG_GDB_SCRIPTS=y，这样在内核编译时会在根目录生成vmlinux-gdb.py文件，这个python脚本需要在gdb调试时加载，以此提供一些供gdb调试内核的扩展命令，对于调试外部ko文件带来极大的方便。

### CONFIG_DEBUG_INFO

配置CONFIG_DEBUG_INFO=y，gdb调试内核需要从vmlinx中加载相关符号信息，但默认的内核编译选项没有-g选项，因此该elf文件将不含有调试信息，调试时看不到源码，仅能做汇编级别的调试跟踪，因此需要配置该选项，打开调试信息（打开后会增大内核文件）。

### CONFIG_DEBUG_INFO_DWARF4

配置CONFIG_DEBUG_INFO_DWARF4=y，如果不启用这个选项内核调试信息将不包含DWARF4信息，只能进行一些backtarce的查看，而无法debug变量，打开该选项可以使用（打开后会进一步增大内核文件）。

### CONFIG_STRICT_KERNEL_RWX/CONFIG_STRICT_MODULE_RWX

配置CONFIG_STRICT_KERNEL_RWX/CONFIG_STRICT_MODULE_RWX选项关闭，这个选项的存在会对内核代码段进行写保护，因此在使用kgdb时如果进行添加断点操作，目标系统如果不支持硬件断点机制，就必须改写内核代码段来使用断点指令触发软件断点达到目的，而该选项的存在会导致断点指令无法写入，因此需要关闭（关闭后会增加内核所在内存被破坏风险）。

## 修改内核启动参数

进行如下修改：

```patch
- bootargs = "root=/dev/vda2 rw console=ttyS0 earlycon=sbi";
+ bootargs = "root=/dev/vda2 rw console=ttyS0 earlycon=sbi nokaslr kgdboc=ttyS1,115200 kgdbwait";
```

### nokaslr

KASLR是一种安全性的手段，会让代码运行在随机化的地址上，传入nokaslr会关闭这个功能以此方便调试的时候地址匹配vmlinx中的符号信息，不过并不是所有arch都支持KASLR功能的，也许在一些架构上就不支持KASLR，这里传不传这个选项也就无所谓了。

### kgdboc

kgdboc用来指定内核调试信息从哪里输出，这里我们使用了ttyS1串口输出，未来gdb便需要连接到对应串口来接收调试数据。

### kgdbwait

添加该参数可以让内核启动时准备好数据后等待gdb接入再继续启动内核。

## 启动调试

完成修改后重新编译内核和设备树文件就可以开始内核调试了，由于我们使用的是qemu模拟器，因此需要打开三个终端。

### 启动qemu

首先运行qemu：

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
$GRAPHIC_PARAM
```

qemu的设备配置不是本文重点，这里注意GRAPHIC_PARAM配置，将仿真模拟的三个串口分别映射到telnet::3441、3442、3443这三个端口，而我们的内核启动后将从串口0输出启动log，从串口1输出kgdb数据包，那么主机则需要连接3441和3442进行调试。

### 启动telnet连接

使用telnet连接串口0：

```shell
telnet localhost 3441
```

可以从这里看到uboot终端输出，启动内核后，内核将进行一部分初始化后，开始等等gdb连接。

### 启动gdb

首先编写一个gdb.script脚本。

```
add-auto-load-safe-path ./linux-5.10.42/
file ./linux-5.10.42/vmlinux 
source ./linux-5.10.42/vmlinux-gdb.py
target remote :3442
```

以上gdb命令分别用于

- 指定./linux-5.10.42/路径为可信的路径，便于gdb执行启动的python脚本；
- 指定符号文件./linux-5.10.42/vmlinux 
- 执行./linux-5.10.42/vmlinux-gdb.py添加环境用于kgdb的命令扩展
- 连接到本地:3442端口接受gdb数据包

启动GDB

```shell
riscv64-unknown-linux-gnu-gdb -x ./gdb.script
```

三个终端最终启动后结果如图：

![ext1-0](./img/ext1-0.png)

## 调试技巧

上一文中，在gdb终端内输入c，内核将继续完成初始化工作，进入系统，此时是无法在gdb中主动停止运行的，因为内核的调试是一种被动调试，如果内核没有主动进入断点也没有触发断点指令，gdb是无法将内核进入调试状态的，此时可以在目标系统内输入如下指令：

```shell
echo g > /proc/sysrq-trigger
```

内核将进入假死状态，即主动触发了bearkpoint断点，此时gdb中可以输入命令bt，查看断点处的调用栈。

```
(gdb) bt
#0  kgdb_breakpoint () at kernel/debug/debug_core.c:1234                  
#1  0xffffffe0000c35d4 in sysrq_handle_dbg (key=<optimized out>)          
    at kernel/debug/debug_core.c:978                                      
#2  0xffffffe00036b55c in __handle_sysrq (key=<optimized out>,            
    check_mask=check_mask@entry=false) at drivers/tty/sysrq.c:598         
#3  0xffffffe00036ba22 in write_sysrq_trigger (file=<optimized out>,      
    buf=0xd1810 <error: Cannot access memory at address 0xd1810>,         
    count=2, ppos=<optimized out>) at drivers/tty/sysrq.c:1157            
#4  0xffffffe0001b8ca4 in pde_write (ppos=<optimized out>,                
    count=<optimized out>, buf=<optimized out>, file=<optimized out>,     
    pde=0xffffffe00187ab00) at fs/proc/inode.c:345                        
#5  proc_reg_write (file=<optimized out>, buf=<optimized out>,            
    count=<optimized out>, ppos=<optimized out>) at fs/proc/inode.c:357   
#6  0xffffffe000154cc6 in vfs_write (                                     
    file=file@entry=0xffffffe001921540,                                   
    buf=buf@entry=0xd1810 <error: Cannot access memory at address 0xd1810>, 
    count=count@entry=2, pos=pos@entry=0xffffffe00302fe80)                 
    at fs/read_write.c:603                                                
#7  0xffffffe000154fb0 in ksys_write (fd=<optimized out>,                 
    buf=0xd1810 <error: Cannot access memory at address 0xd1810>,         
    count=2) at fs/read_write.c:658                                       
#8  0xffffffe00015502a in __do_sys_write (count=<optimized out>,          
    buf=<optimized out>, fd=<optimized out>) at fs/read_write.c:670       
#9  __se_sys_write (fd=<optimized out>, buf=<optimized out>,              
    count=<optimized out>) at fs/read_write.c:667                         
#10 0xffffffe00003b30a in handle_exception ()                             
    at arch/riscv/kernel/entry.S:205                                      
Backtrace stopped: frame did not save the PC
```

使用b可以打断点，例如：b filp_open，使用s，可以单步跳入执行，使用n可以单步跳过执行，使用c可以连续执行，使用p可以打印变量，使用info可以查看一些信息如寄存器，内存信息，使用l可以查看当前位置的c源码片段，使用disassemble可以查看当前位置汇编指令片段，输入-可以进入交互式的调试界面，当前操作与平时操作gdb无异。

这里我们再了解一下kgdb带来的额外扩展命令，输入lx-然后按tab补全就可以看到kgdb的扩展命令，基本上顾名思义是比较容易理解的。

```
(gdb) lx-                                                                 
lx-clk-summary        lx-dmesg              lx-mounts                     
lx-cmdline            lx-fdtdump            lx-ps                         
lx-configdump         lx-genpd-summary      lx-symbols                    
lx-cpus               lx-iomem              lx-timerlist                  
lx-device-list-bus    lx-ioports            lx-version                    
lx-device-list-class  lx-list-check                                       
lx-device-list-tree   lx-lsmod                                            
(gdb) lx- 
```

这里我们着重了解下lx-symbols，使用这个命令将便于我们调试没有随内核一起初始化而是后加载到内核的ko文件。

首先编译生成一个测试用的ko模块，这里用我自己写的一个纯软件的简单开源驱动为例子——[虚拟串口](https://github.com/QQxiaoming/virts)。我们生成了virts.ko文件，将它放在运行gdb所在的当前目录，然后gdb使用c命令释放假死的目标调试机，在目标qemu模拟器内insmod安装该模块，lsmod可以看到ko文件已被安装。重新输入echo g > /proc/sysrq-trigger进入假死。

此时使用b命令给virts.c:transmission_data打断点是无法成功的，因为无法索引这个符号，此时我们需要使用lx-symbols，稍等片刻可以看到如下输出

```
(gdb) lx-symbols                                                          
loading vmlinux                                                           
scanning for modules in /home/qqm/quard_star_tutorial           
loading @0xffffffdf80af8000: /home/qqm/quard_star_tutorial/target_driver_module/virts_tty/virts.ko 
```

此时符合已正确加载到gdb环境，使用b virts.c:transmission_data可以成功断点。这里稍微注意下，如没你不使用lx-symbols，而是使用原生的gdb命令：

```
add-symbol-file {filename} {addr}
```

就需要手动查找ko文件加载到的地址，如果你的系统有很多ko，这将是很麻烦的事情，所以lx-symbols可以自动查找所有ko文件并加载符号可以带来极大的便利。

另附：笔者使用的内核版本5.10.42中lx-symbols存在不成功的问题，错误如下：

```
Python Exception <class 'gdb.MemoryError'> Cannot access memory at address
```

经分析定位将linux-5.10.42/scripts/gdb/linux/symbols.py:_section_arguments函数做以下修改，就可以解决该问题。

```python
    def _section_arguments(self, module):
        try:
            sect_attrs = module['sect_attrs'].dereference()
            attrs = sect_attrs['attrs']
            section_name_to_address = {
                attrs[n]['battr']['attr']['name'].string(): attrs[n]['address']
                for n in range(int(sect_attrs['nsections']))}
        except gdb.error:
            return ""
        args = []
        for section_name in [".data", ".data..read_mostly", ".rodata", ".bss",
                            ".text", ".text.hot", ".text.unlikely"]:
            address = section_name_to_address.get(section_name)
            if address:
                args.append(" -s {name} {addr}".format(
                    name=section_name, addr=str(address)))
        return "".join(args)
```

## 总结

OK，kgdb的简单使用基本上就没有问题，kgdb不是一个很常用的调试手段，但也许在一些特殊情况下能解决一些问题，尤其是嵌入式开发中。本文涉及到的代码均已开源，可以在 https://github.com/QQxiaoming/quard_star_tutorial/tree/experiment/kgdb ，experiment/kgdb分支中找到。
