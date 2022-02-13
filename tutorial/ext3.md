# qemu常用参数选项说明

在我的系列博客《[基于qemu-riscv从0开始构建嵌入式linux系统](https://blog.csdn.net/weixin_39871788/article/details/118469061)》中使用qemu项目一步步构造搭建了嵌入式linux系统，而其中在run.sh中qemu的启动参数选项非常多且繁杂，因此本文将对其进行归纳整理。

## 设备类型（-machine/-M）

在qemu中，不同的指令集的模拟器会编译成不同的可执行文件，诸如：qemu-system-x86_64/qemu-system-aarch64/qemu-system-arm/qemu-system-mips/qemu-system-riscv64，但相同指令集肯恩那个存在不同的设备，尤其是在嵌入式设备上。使用参数-machine help运行模拟器可以查看当前模拟器支持的设备信息，例如：

```
qqm@ubuntu: qemu-system-riscv64 -machine help      
Supported machines are:
microchip-icicle-kit Microchip PolarFire SoC Icicle Kit
none                 empty machine
quard-star           RISC-V Quard Star board
sifive_e             RISC-V Board compatible with SiFive E SDK
sifive_u             RISC-V Board compatible with SiFive U SDK
spike                RISC-V Spike board (default)
virt                 RISC-V VirtIO board

qqm@ubuntu: qemu-system-aarch64 -machine help 
Supported machines are:
akita                Sharp SL-C1000 (Akita) PDA (PXA270)
ast2500-evb          Aspeed AST2500 EVB (ARM1176)
ast2600-evb          Aspeed AST2600 EVB (Cortex A7)
borzoi               Sharp SL-C3100 (Borzoi) PDA (PXA270)
canon-a1100          Canon PowerShot A1100 IS (ARM946)
cheetah              Palm Tungsten|E aka. Cheetah PDA (OMAP310)
collie               Sharp SL-5500 (Collie) PDA (SA-1110)
connex               Gumstix Connex (PXA255)
cubieboard           cubietech cubieboard (Cortex-A8)
emcraft-sf2          SmartFusion2 SOM kit from Emcraft (M2S010)
g220a-bmc            Bytedance G220A BMC (ARM1176)
highbank             Calxeda Highbank (ECX-1000)
...
```

模拟器运行时必须选择一个具体的设备，比如运行树梅派3B则执行qemu-system-aarch64 -machine raspi3b或qemu-system-aarch64 -M raspi3b，运行Quard Star board则是执行qemu-system-riscv64 -machine quard-star或qemu-system-aarch64 -M quard-star。qemu定义使用virt来进行通用模拟而非实际的某SOC。

具体到设备本身可能还有额外的参数作为可选项供使用者配置，要查看设备的可选参数可以使用-machine 'id',help，如下示例：

```
qqm@ubuntu: qemu-system-riscv64 -machine quard-star,help
quard-star.hmat=bool (Set on/off to enable/disable ACPI Heterogeneous Memory Attribute Table (HMAT))
quard-star.mask-rom-path=string (Quard Star Mask ROM image file)
quard-star.suppress-vmdesc=bool (Set on to disable self-describing migration)
quard-star.append=string (Linux kernel command line)
quard-star.mem-merge=bool (Enable/disable memory merge support)
quard-star.dtb=string (Linux kernel device tree file)
quard-star.memory-backend=string (Set RAM backendValid value is ID of hostmem based backend)
quard-star.dumpdtb=string (Dump current dtb to a file and quit)
quard-star.phandle-start=int (The first phandle ID we may generate dynamically)
quard-star.dump-guest-core=bool (Include guest memory in a core dump)
quard-star.memory-encryption=string (Set memory encryption object to use)
quard-star.firmware=string (Firmware image)
quard-star.usb=bool (Set on/off to enable/disable usb)
quard-star.graphics=bool (Set on/off to enable/disable graphics emulation)
quard-star.confidential-guest-support=link<confidential-guest-support> (Set confidential guest scheme to support)
quard-star.initrd=string (Linux initial ramdisk file)
quard-star.dt-compatible=string (Overrides the "compatible" property of the dt root node)
quard-star.kernel=string (Linux kernel image file)
```

因此你可以看到我们的run.sh使用了mask-rom-path参数为quard-star指定maskrom固件路径

```
-M quard-star,mask-rom-path="$SHELL_FOLDER/output/mask_rom/mask_rom.bin"
```

## 内存大小（-m）

参数-m 1G就是指定虚拟机内部的内存大小为1GB，帮助说明如下：

```
qqm@ubuntu: qemu-system-riscv64 -m help
qemu-system-riscv64: -m help: Parameter 'size' expects a non-negative number below 2^64
Optional suffix k, M, G, T, P or E means kilo-, mega-, giga-, tera-, peta-
and exabytes, respectively.
```

## 核心数（-smp）

现代cpu往往是对称多核心的，因此通过指定-smp 8可以指定虚拟机核心数。

```
qqm@ubuntu: qemu-system-riscv64 -smp help
qemu-system-riscv64: -smp help: Parameter 'cpus' expects a number
```

## 驱动器映像文件（-drive）

-drive用来添加映像文件，一般用于块设备，常见配置如下：

```
-drive if=none,format=raw,file=./usb.img,id=usb0
```

一般来讲必须要配置file参数，qemu会根据文件内容自动匹配format，但是最好还是显式声明format=raw更好，qemu支持的format非常多，甚至包括ftp、http等网络地址，详细可以查阅[qemu文档-磁盘映像](https://www.qemu.org/docs/master/system/images.html)，我们最简单透明的就是直接使用raw格式。if参数用来指定匹配设备内部的块设备，例如：

```
-drive if=pflash,bus=0,unit=0,format=raw,file=$SHELL_FOLDER/output/fw/fw.bin,id=mtd0 \
-drive if=mtd,format=raw,file=$SHELL_FOLDER/output/fw/norflash.img,id=mtd1 \
-drive if=none,format=raw,file=$SHELL_FOLDER/output/fw/usb.img,id=usb0 \
-drive if=sd,format=raw,file=$SHELL_FOLDER/output/fw/sd.img,id=sd0 \
-drive if=none,format=raw,file=$SHELL_FOLDER/output/rootfs/rootfs.img,id=disk0 \
```

这样设备映像就匹配到设备源码drive_get_next中的具体ip，

```c
drive_get_next(IF_SD);
drive_get_next(IF_MTD);
drive_get_next(IF_PFLASH);
```

## 网络设备（-netdev）

网络设备配置常见两种，一种是user模式，一种是tap模式，这里分别介绍：

### user模式

```
-netdev user,net=192.168.31.0/24,host=192.168.31.2,hostname=qemu,dns=192.168.31.56,tftp=$SHELL_FOLDER/output,bootfile=/linux_kernel/Image,dhcpstart=192.168.31.100,hostfwd=tcp::3522-:22,hostfwd=tcp::3580-:80,id=net0
```

如上为user模式配置的一个实例，user模式下，客户机和宿主机之间可以建立tcp/udp连接，且可以配置端口转发将客户机中的端口转发到宿主机上。

- net=192.168.31.0/24 ：配置网关地址/子网掩码
- host=192.168.31.2 ：配置主机地址
- hostname=qemu ：配置主机名称
- dns=192.168.31.56 ：创建一个dns服务器
- tftp=$SHELL_FOLDER/output,bootfile=/linux_kernel/Image：创建一个tftp服务器
- dhcpstart=192.168.31.100：dhcp地址分配起始地址
- hostfwd=tcp::3522-:22：端口转发将客户机的22端口转发到宿主机的3522端口

user模式的缺点是客户机无法支持全部的ip协议，但其使用简单，无需对宿主机进行额外的配置。

### tap模式

tap即在宿主机上创建一个虚拟网卡设备tap0，客户机使用这个网卡设备进行网络通信，优点是对于客户机网卡设备与真实网卡相似，但是缺点是宿主机需要额外工具进行大量配置，网络拓扑结构复杂。

网桥一般需要手动配置，不过qemu提供了一个qemu-bridge-helper工具可以自动配置网桥，需要配置echo "allow br0" > /etc/qemu/bridge.conf文件，启动qemu参数如下：

```
-netdev tap,helper=/libexec/qemu-bridge-helper,id=net0
```

不过如此使用需要qemu使用root权限才能创建网桥，如此对整个qemu提高权限非常不合适，这里我还是建议自己配置网桥后使用如下命令启动qemu。

```
-netdev tap,ifname=tap0,script=no,downscript=no,id=net0
```

网桥的配置方法如下：

```shell
USER_NAME=$(whoami)
ETH_NAME=enp2s0f0 #eth0

config_tap()
{
    brctl addbr br0
    ip addr flush dev $ETH_NAME
    brctl addif br0 $ETH_NAME
    tunctl -t tap0 -u $USER_NAME
    brctl addif br0 tap0
    ifconfig $ETH_NAME up
    ifconfig tap0 up
    ifconfig br0 up
    ip addr flush dev br0
    ip addr flush dev tap0
    ip addr flush dev $ETH_NAME

    # static
    ip addr add 169.254.105.176/16 broadcast 169.254.255.255 dev br0
    # dynamic
    #dhclient -v br0
}

release_tap()
{
    brctl delif br0 tap0
    tunctl -d tap0
    brctl delif br0 $ETH_NAME
    ifconfig br0 down
    brctl delbr br0
    ifconfig $ETH_NAME up

    # static
    ip addr add 169.254.105.176/16 broadcast 169.254.255.255 dev $ETH_NAME
    # dynamic
    #dhclient -v $ETH_NAME
}
```

完成配置后：

```
br0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 169.254.105.176  netmask 255.255.0.0  broadcast 169.254.255.255
        ether 0a:0e:24:90:6a:e9  txqueuelen 1000  (以太网)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 7  bytes 811 (811.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

enp2s0f0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether 02:42:41:29:3f:f3  txqueuelen 1000  (以太网)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

tap0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether 0a:0e:24:90:6a:e9  txqueuelen 1000  (以太网)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```

## 字符设备（-chardev）

## 文件系统设备（-fsdev）

## 音频设备（-audiodev）

## 全局参数（-global）

## 其他设备（-device）

## 显示选项（-display）

## 终端选项（--serial　--parallel　--monitor）
