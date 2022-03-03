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

最后一般情况下的无线网卡sta是不支持网桥的，因此tap方式只能和有线网卡桥接。[更多参考](https://blog.stdio.io/954)。

## 字符设备（-chardev）

qemu中创建字符设备的典型应用就是模拟ttyUSB的设备，如下参数配置可以在宿主机打开一个telnet服务器，我们可以从外部连接到qemu内部，配合device选项可以关联到内部的usb-serial。

```
-chardev socket,telnet=on,host=127.0.0.1,port=3450,server=on,wait=off,id=usb1
```

```
-device usb-serial,always-plugged=true,chardev=usb1 \
```

字符设备支持如下类型：

```
qqm@ubuntu: qemu-system-riscv64 -chardev help      
Available chardev backend types: 
  ringbuf
  mux
  pipe
  null
  msmouse
  socket
  vc
  parallel
  memory
  udp
  file
  pty
  serial
  wctablet
  stdio
  testdev
```

具体字符设备类型还可以指定更多参数，示例如下：

```
qemu-system-riscv64 -chardev sockeet,help
chardev options:
  abstract=<bool (on/off)>
  append=<bool (on/off)>
  backend=<str>
  chardev=<str>
  cols=<num>
  debug=<num>
  delay=<bool (on/off)>
  fd=<str>
  height=<num>
  host=<str>
  ipv4=<bool (on/off)>
  ipv6=<bool (on/off)>
  localaddr=<str>
  localport=<str>
  logappend=<bool (on/off)>
  logfile=<str>
  mux=<bool (on/off)>
  name=<str>
  nodelay=<bool (on/off)>
  path=<str>
  port=<str>
  reconnect=<num>
  rows=<num>
  server=<bool (on/off)>
  signal=<bool (on/off)>
  size=<size>
  telnet=<bool (on/off)>
  tight=<bool (on/off)>
  tls-authz=<str>
  tls-creds=<str>
  tn3270=<bool (on/off)>
  to=<num>
  wait=<bool (on/off)>
  websocket=<bool (on/off)>
  width=<num>
```

## 文件系统设备（-fsdev）

-fsdev并不常用，但在使用virtio-9p-device设备时会很有用，使用虚拟机软件时，一般都提供一个共享目录功能，让虚拟机和主机可以同时访问同一目录，方便交互文件，virtio-9p-device设备就能实现类似功能，示例参数配置如下：

```
-fsdev local,security_model=mapped-xattr,path=$SHELL_FOLDER,id=fsdev0
-device virtio-9p-device,fsdev=fsdev0,mount_tag=hostshare,id=fs0
```

由-fsdev选项配置宿主机要共享的路径（path参数），共享模型（security_model参数）指的是关于共享目录的权限处理，这里建议时使用mapped-xattr映射权限，这样在host下权限为运行qemu用户的权限，guest有权进行读和写。

另外qemu支持virtio-9p-device需要的libattr1-dev，否则无法支持这一功能。 

## 音频设备（-audiodev）

-audiodev用于指定创建host上的音频设备，配置id，以便连接到guest上的音频仿真硬件,该选项参数如下：

```
-audiodev [driver=]driver,id=id[,prop[=value][,...]]
                specifies the audio backend to use
                id= identifier of the backend
                timer-period= timer period in microseconds
                in|out.mixing-engine= use mixing engine to mix streams inside QEMU
                in|out.fixed-settings= use fixed settings for host audio
                in|out.frequency= frequency to use with fixed settings
                in|out.channels= number of channels to use with fixed settings
                in|out.format= sample format to use with fixed settings
                valid values: s8, s16, s32, u8, u16, u32, f32
                in|out.voices= number of voices to use
                in|out.buffer-length= length of buffer in microseconds
-audiodev none,id=id,[,prop[=value][,...]]
                dummy driver that discards all output
-audiodev alsa,id=id[,prop[=value][,...]]
                in|out.dev= name of the audio device to use
                in|out.period-length= length of period in microseconds
                in|out.try-poll= attempt to use poll mode
                threshold= threshold (in microseconds) when playback starts
-audiodev oss,id=id[,prop[=value][,...]]
                in|out.dev= path of the audio device to use
                in|out.buffer-count= number of buffers
                in|out.try-poll= attempt to use poll mode
                try-mmap= try using memory mapped access
                exclusive= open device in exclusive mode
                dsp-policy= set timing policy (0..10), -1 to use fragment mode
-audiodev pa,id=id[,prop[=value][,...]]
                server= PulseAudio server address
                in|out.name= source/sink device name
                in|out.latency= desired latency in microseconds
-audiodev sdl,id=id[,prop[=value][,...]]
                in|out.buffer-count= number of buffers
-audiodev wav,id=id[,prop[=value][,...]]
                path= path of wav file to record
```

这里要注意你的qemu不一定支持这么多音频框架，需要在编译qemu时指定config选项：

```
--audio-drv-list=pa,alsa,sdl,oss
```

如果在windows上的qemu，则不支持以上框架，一般用dsound框架。

## 全局参数（-global）

-global一般用于配置qemu的全局参数选项，可配置的内容非常多，格式如下：

```
global options:
  driver=<str>
  property=<str>
  value=<str>
```

示例如下：

```
-global virtio-mmio.force-legacy=false
-global quard-star-syscon.boot-cfg="$DBOOTCFG"
```

如何查看可用的选项呢？我这里没找到更好的办法，原则上如果你有qemu的源码，在源码中搜索DEFINE_PROP即可看到类似如下的代码：

```c
    DEFINE_PROP_STRING("boot-cfg", QuardStarSysconState, boot_cfg),
    DEFINE_PROP_BOOL("update-cfg", QuardStarSysconState, update_cfg, true),
```

这里的选项均作为global的可配置参数使用，这部分参数配置最好能阅读qemu的源码理解使用，否则不建议配置这些参数。

## 设备（-device）

-device常用于指定guest上总线挂载的外部设备，例如virtio-mmio、usb、pci等总线，示例如下：

```
-device virtio-blk-device,drive=disk0,id=hd0 \
-device virtio-gpu-device,xres=$WIDTH,yres=$HEIGHT,id=video0 \
-device virtio-mouse-device,id=input0 \
-device virtio-keyboard-device,id=input1 \
-device virtio-9p-device,fsdev=fsdev0,mount_tag=hostshare,id=fs0 \
-device virtio-net-device,netdev=net0 \
-device usb-storage,drive=usb0 \
-device usb-serial,always-plugged=true,chardev=usb1 \
-device wm8750,audiodev=audio0 \
```

注意有些设备需要匹配host上的真实设备驱动模拟，一般要匹配类似chardev\audiodev\fsdev\netdev。

## 显示选项（-display）

-display与-audiodev情况类似，主要是看qemu在编译时配置那些gui框架，目前建议在ubuntu上使用gtk效果良好。

```
-display sdl[,alt_grab=on|off][,ctrl_grab=on|off]
            [,window_close=on|off][,gl=on|core|es|off]
-display gtk[,grab_on_hover=on|off][,gl=on|off]|
-display vnc=<display>[,<optargs>]
-display curses[,charset=<encoding>]
-display egl-headless[,rendernode=<file>]
-display none
                select display backend type
                The default display is equivalent to
                "-display gtk"
-nographic      disable graphical output and redirect serial I/Os to console
```

配置示例：

```
--display gtk,zoom-to-fit=false
```

如果你不想仿真GUI时可以使用-nographic选项。

## 终端选项（--serial　--parallel　--monitor）

终端包括三类串行终端，并行终端，qemu命令监控终端。

终端对应的host设备可以是以下选项：

```
--serial stdio
--serial vc:1280x720
--serial telnet:127.0.0.1:3441,server,nowait
```

绑定stdio即host的标准输入输出，vc:1280x720即GUI显示窗口（注意配置合适的分辨率），telnet为打开一个服务器用于终端交互。
