[![CI](https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/ci.yml)
[![Documentation Status](https://readthedocs.org/projects/quard-star-tutorial/badge/?version=latest)](https://quard-star-tutorial.readthedocs.io/zh_CN/latest/?badge=latest)
[![CodeFactor](https://www.codefactor.io/repository/github/qqxiaoming/quard_star_tutorial/badge)](https://www.codefactor.io/repository/github/qqxiaoming/quard_star_tutorial)
[![GitHub stars](https://img.shields.io/github/stars/QQxiaoming/quard_star_tutorial.svg)](https://github.com/QQxiaoming/quard_star_tutorial)

# 基于qemu从0开始构建嵌入式linux系统

![logo](./doc/img/img6.gif)

## 引言

本项目旨在真正从0开始构建嵌入式linux系统，为了剖析芯片从上电开始执行第一条指令到整个系统运行，相关应用服务启动，因此不使用市面上真实的板子，基于qemu定制模拟器开发板，且不使用qemu提供的快速加载elf的文件方式，因而我们需要下载qemu-6.0.0源码，自己编写(或使用qemu自带的)硬件ip相关模拟代码定制属于自己的硬件板卡。本项目同步制作[博客专栏](https://blog.csdn.net/weixin_39871788/category_11180842.html)(2021.8.11:目前开始使用readthedocs构建[文档](https://quard-star-tutorial.readthedocs.io/zh_CN/latest/index.html)，与博客内容相同)，因此项目尽量保证每增加一个feature就提交一次，方便索引跟踪开发过程以方便对应到博客的具体章节。

## 环境搭建

ubuntu18.04需要使用apt安装的包:

```shell
sudo apt install ninja-build pkg-config libglib2.0-dev libpixman-1-dev libgtk-3-dev libcap-ng-dev libattr1-dev device-tree-compiler bison flex
```

## 硬件架构

下图为目前的虚拟硬件架构，随着项目推进会持续更新。

![硬件架构](./doc/img/img3.png)

## 软件流程

下图为目前的软件流程，随着项目推进会持续更新。

![软件流程](./doc/img/img4.png)

## 内存分布

下图为目前的内存分布，随着项目推进会持续更新。

![内存分布](./doc/img/img5.png)

## 开发记录

- 
    2021.07.04(下午):增加了Quard Star开发板定义，8核支持，增加中断控制器、串口、pflash。添加低阶启动代码固化在pflash运行。
- 
    2021.07.05(晚上):增加了opensbi源码并添加platform支持代码，低阶启动代码添加加载sbi功能。
- 
    2021.07.09(晚上):增加了opensbi设备树中domain定义，并增加安全域的工程代码，使用安全域串口3打印；增加u-boot代码并引导进入非安全域执行，使用串口0打印。
- 
    2021.07.10(中午):增加uboot的platform定义以及配置等代码。
- 
    2021.07.10(下午):使用之前制作的根文件系统及内核映像尝试引导正确，设备树文件有所改动，且发现了点问题，1.当前版本的opensbi fix下一级设备树文件时会添加不可用的domain到设备树的reserved-memory节点，但存在重叠的情况，这在u-boot中解析设备树时是会报warring的，因此这里将域修改为分块的域，并不优雅但暂时就这么解决。2.u-boot在启动bl1阶段会拷贝代码到ddr高地址，而如果我们将高地址配置为了不可访问的安全domain时，当前u-boot的riscv实现中没有检查是否是reserved-memory区域，这里我们为了简单期间直接在我们的board platform代码层添加hook函数修改gd->ram_top来实现，其实这种方式最简单高效，如果都从设备树里解析未免过度设计了，这里本就只是启动引导的一小段环节而已，但是看起来uboot的趋势越来越向解析设备树方向靠近了。3.另外发现linux5.10.42在qemu使用-device virtio-gpu-device驱动工作不正常，内核oops了，目前不清楚是qemu-6.0.0的问题还是内核的代码，这个我有空去查一下。
- 
    2021.07.11(上午):添加linux kernel的编译，busybox的简易编译，编写根文件系统映像的打包脚本，添加简易etc配置文件，终于shell运行成功，后面要开始完善一些系统配置，用户态程序以及增加新显示驱动设备，我们打算移植Qt界面到系统内。另外安全域内的固件打算移植一些RTOS系统，看是移植FreeRTOS还是RTThread比较好呢(⊙o⊙)?
- 
    2021.07.12(晚上):使用-device virtio-gpu-device选项添加virtio显示设备时，内核出现了oops，地址跑飞的问题，我稍微花了点时间查找了一下，直接错误出现在linux-5.10.42/drivers/gpu/drm/virtio/virtgpu_kms.c:242行此处vgdev为空指针，直接访问会出错。为什么会是空指针呢，原来是因为110行处初始化时检查VIRTIO_F_VERSION_1不通过，因此驱动并没有正常被加载，而后release时出现空指针。明确了这个问题我们就要去qemu源码里查找一下为什么不支持这个VIRTIO_F_VERSION_1这个feature，经过查找终于发现在qemu-6.0.0/hw/virtio/virtio-mmio.c:712处，只有定义了proxy->legacy为false时才能添加这个feature，而722行给出默认配置为true，因此我们在运行qemu选项添加-global virtio-mmio.force-legacy=false选项后，终于一切正常了。这里我们参考一下这篇[qemu开发邮件](https://www.mail-archive.com/search?l=qemu-devel@nongnu.org&q=subject:%22Re%5C%3A+%5C%5BQemu%5C-devel%5C%5D+%5C%5BPATCH+v3%5C%5D+virtio%5C-mmio%5C%3A+implement+modern+%5C%28v2%5C%29%2C+personality+%5C%28virtio%5C-1%5C%29%22&o=newest&f=1)不难发现，force-legacy定义为false才是更新版本的定义，但可能是考虑兼容性的问题qemu默认还是给出了true才用传统的virtio行为。解决了这个问题，我们重新添加内核参数配置，以及内核定义一个我们的quard_star_defconfigs文件，添加显示linux启动logo的配置，再次启动qemu，ok！以下为上文部分描述中代码片段：

    ```c
    ……
    110 if (!virtio_has_feature(dev_to_virtio(dev->dev), VIRTIO_F_VERSION_1))
    111		return -ENODEV;
    ……
    242 virtio_gpu_modeset_fini(vgdev);
    ……
    ```

    ```c
    ……
    712 if (!proxy->legacy) {
    713     virtio_add_feature(&vdev->host_features, VIRTIO_F_VERSION_1);
    714 }
    ……
    722 DEFINE_PROP_BOOL("force-legacy", VirtIOMMIOProxy, legacy, true),
    ……
    ```

- 
    2021.07.13(晚上):将启动使用的shell从busybox提供的sh改为标准的bash，以方便后续开发。不过最麻烦的就是ncurses库的编译安装了，动态库可以从交叉编译工具链中拷贝，但是一些终端相关的配置文件和数据就要去编译ncurses库来安装了，好在基本上搞定了。另外注意从这里开始，我们新增给系统roofs.img中添加的内容都是通过目标虚拟机挂载共享目录然后安装的，而不是最处由脚本构建打包的，因此务必注意除非img被破坏，否则不要重新构建img，尽可能使用虚拟机本身去修改img内的文件。
- 
    2021.07.14(晚上):添加rtc和syscon到系统内，解决了虚拟机内时间戳不准确问题，解决了reboot不能复位系统，另外之前内核resetvec字段没有定义到rom的基地址，而是默认的0x1000(qemu-6.0.0/target/riscv/cpu.c的"resetvec"字段可以看到默认值)，虽没有产生错误，但是cpu启动时确认读入0x1000地址，与我们的期望不符，这次reboot可以看到其错误，修改后问题解决。关于上述cpu执行启动地址问题你可以添加qemu启动选项-d in_asm -D qemu_run_asm.log来观察行为。另外目前的实现reboot和poweroff这两个配置在kernel的设备树中其实是不太优雅的设计，我们使用的linux内核版本中sbi已经支持了poweroff，所以syscon-poweroff注册会失败，而syscon-reboot会注册成功，因为sbi还没有实现reboot，鉴于opensbi还在快速开发阶段，相信这个问题已经在逐渐完善了，感兴趣的朋友可以关注下这些项目的主线开发分支情况，这里我们就不去纠结这个设计了。
- 
    2021.07.15(晚上):将sudo和screenfetch移植到系统内，screenfetch稍微修改了一点，毕竟我们是busybox但是换了标准的bash，检测有的错误。另外今天在@zhiliao007(wenqing.li@aliyun.com)的帮助下确认了开启这个项目的所需要使用apt安装的包，由于我启动项目时使用的pc已经安装了很多开发环境了，因此一直没能确认如果是新的Ubuntu主机需要安装什么基础库，这里提出感谢。
- 
    2021.07.17(下午):这几天工作有点忙，终于周末有空继续这个项目了，首先是运行qemu时使用gtk显示界面是目前看起来最理想的方式，为了使用更大分辨率建议添加配置使其自动缩放启动参数如下--display gtk,zoom-to-fit=false。然后我将我一直在用的一些比较好用的工具继续向系统移植tree,screen,cu这三个工具，尤其是screen，毕竟只有终端的日子可能还要持续很久，那么一个漂亮的多终端工具就非常重要了，这些工具的移植配置还算是容易，但是终端窗口大小从主机测动态传入就比较麻烦了，这时候我们之前的fw_cfg就起到了大作用，我们启动脚本计算了终端窗口大小通过fw_cfg传入目标机，然后目标机通过cat /sys/firmware/qemu_fw_cfg/by_name/opt/qemu_cmdline/raw就可以读取到。不过注意了，由于qemu是在6.0版本才给riscv架构添加fw_cfg，而linux5.10.42还没有更新到，因此我们这里手动稍微修改下内核里的cofing平台过滤配置就可以使用啦。相信最新版主线上linux内核已经更新了这个功能，所以如果想体验新内核也可以更新最新内核来尝鲜。下面放张图看看开发到当前的效果。

    ![img0](./doc/img/img0.png)

- 
    2021.07.18(下午):从昨天晚上开始准备着手把freertos移植到trusted_domain中，但是下载了目前最新版本的freertos发现官方没有只有riscv S模式的移植包，都是基于M模式，那就自己来干吧，反正FreeRTOS是我最最熟悉的操作系统，经过一晚上加今天大半天的时间，终于移植成功了，主要是三部分，Tick使用sbi系统调用sbi_set_timer，任务上下文调度主要在S模式不要操作错CSR寄存器了，最后是portYIELD使用sbi_send_ipi向自己核心发送软中断进行。目前基本功能移植成功，目前已知还有两个细节需要优化，一是使用了中断栈但不能支持嵌套，sscratch没有利用到也不高效，这个需要分析下看究竟能否支持中断嵌套，二是浮点栈帧没有保存恢复，这个下次添加时要仔细考虑，最好能实现惰性压栈。不过目前的移植对于大部分需求来讲已经满足了。不知道是不是全网首发的S模式Freertos移植，反正我是没找到有人发布。毕竟S模式下，实时性可能会备受打击，但是目前我在qemu测试还好，当然我需要进一步测试，后面添加一些实时系统和linux系统ipc通讯的测试看看。
- 
    2021.07.24(上午):这周头痛没太多进展，这次主要是使用了 https://github.com/riscv/riscv-gnu-toolchain （ gitee镜像：https://gitee.com/mirrors/riscv-gnu-toolchain ），仓库分别编译了用于linux的编译器和用于裸机开发的编译器，这样有三个好处，第一是裸机开发能使用newlib的c库，第二是不依赖别人提供的工具链，方便我们debug时跟踪源码，第三是我们编译了gdb工具无论是裸机还是linux上的app应用调试可以使用gdb分析问题。更新了编译器后重新编译target_root_app时我更新了一些编译脚本，优化了之前编译时忽略的细节，详细内容可以查看target_root_app目录内的[README.md](./target_root_app/README.md)，大部分都没有问题了，但是screen工作不正常，粗略的debug了一下还没找到问题，后面有空再查找这个问题了。
- 
    2021.07.26(晚上):周末台风天一直在debug screen工作不正常的问题，今天晚上终于给解决了。这里一定要记录下曲折的心路历程(＞﹏＜)！首先是执行screen后出现“[screen is terminating]”后就瞬间退出了，很莫名其妙，第一想法就是打开编译screen的Makefile文件，把“OPTIONS= -DDEBUG”打开，然后再次执行，会在/tmp/debug/{SCREEN,screen}.*产生log信息，查找log并对比了之前正常工作log，完全没发现什么异常，然后我尝试交叉编译了strace工具，没有什么异常发现，没有办法，只好去交叉编译了gdb工具（交叉编译gdb时会报出很多错误，主要还是官方的编译脚本有漏洞，均是路径错误，我手动修改后最终编译成功）生成了一个可以在目标板运行 的gdb工具，拷贝到目标板中，使用gdb调试一点点定位问题，再次打开编译screen的Makefile文件，修改成“CFLAGS = -g -O0 -D_GNU_SOURCE”方便我们使用gdb调试。很快我就发现screen.c:1235这里会调用fork，并且主进程在之后并没有做产生异常就收到了SIGHUP信号退出了，因此出现异常的应该是这里创建的子进程。我想起来之前strace可能没有检查子进程的log，于是重新使用“strace -f /bin/screen”检查，却还是没发现什么异常，奇怪！只好继续用gdb跟踪子进程，方法是进入gdb后，使用“set follow-fork-mode child”命令来指示gdb跟踪子进程，经过一番定位最终screen.c:1428的MakeWindow(&nwin)返回了-1引起错误，进一步跟踪发现产生错误的调用关系是这样：
    
    ```
    screen.c:1428:MakeWindow(&nwin)
    >>>>window.c:628:OpenDevice(nwin.args, nwin.lflag, &type, &TtyName)
    >>>>>>>>window.c:1132:OpenPTY(namep)
    >>>>>>>>>>>>pty.c:334:openpty(&f, &s, TtyName, NULL, NULL)
    ```
    
    看来问题就出在openpty，这个函数定义在glibc中login/openpty.c:86行，使用gdb深入查找我们发现openpty.c:99:master = getpt ();返回了-1，而getpt的定义有三个分别在login/getpt.c和sysdeps/unix/bsd/getpt.c和sysdeps/unix/sysv/linux/getpt.c，我大概猜到问题了，应该是我们自己编译的编译器和下载bootlin上的编译器这样使用了不同的getpt，经过确认我们自己编译器使用的是sysdeps/unix/sysv/linux/getpt.c，而bootlin上的编译器我只能猜测使用的是sysdeps/unix/bsd/getpt.c，那么区别是什么呢？linux风格的pty设备是打开/dev/ptmx进而在/dev/pts目录内产生pty节点设备，而BSD风格的pty设备则是直接使用/dev/pty*设备。那么我自己编写了简单的测试代码open设备/dev/ptmx，发现果然返回-1，只好进一步查看内核的相关配置，发现其实内核里对两种风格的pty设备都支持了，如下：
    
    ```
    CONFIG_UNIX98_PTYS=y
    CONFIG_LEGACY_PTYS=y
    CONFIG_LEGACY_PTY_COUNT=256
    ```
    
    但是要注意到[内核文档>devices.rst](https://elixir.bootlin.com/linux/v5.10.42/source/Documentation/admin-guide/devices.rst)第155行提到了要将devpts挂载到/dev/pts这样linux风格的pty设备才能使用，原来是这样( ^_^ )！我们在目标文件系统配置文件/etc/fstab使用mdev的方式生成/dev目录，但是并没有给我们创建/dev/pts目录，因此不能直接在/etc/fstab添加挂载设备信息，那我们就还是在/etc/init.d/rcS启动脚本中添加，在/sbin/mdev -s后添加
    
    ```shell
    mkdir /dev/pts
    mount -t devpts devpts /dev/pts
    ```
    
    OK，测试一下screen，完美运行，大功告成。
- 
    2021.07.27(晚上):添加了运行脚本full-screen选项，希望能全屏启动且分辨率适配主机环境，然后发现qemu的gtk前端UI有点问题，在图形驱动工作前处于低分辨率，完成配置后分辨率正确后，gtk会更新窗画面，但是如果gtk进入全面模式就不会更新，最终整个全屏显示就出问题了，最终定位到qemu-6.0.0/ui/gtk.c:601这里gd_update_windowsize函数只会在非全屏模式更新size，于是我们做以下修改:

    ```c
    if(vc->s->full_screen){
        gtk_menu_item_activate(GTK_MENU_ITEM(vc->s->full_screen_item));
        gtk_menu_item_activate(GTK_MENU_ITEM(vc->s->full_screen_item));
    } else {
        gd_update_windowsize(vc);
    }
    ```

    这样就可以针对全屏的情况下刷新正确的size。
- 
    2021.07.28(晚上):trusted_domain裸机程序可以使用newlib了，因此修改了一下启动脚本并做了相关newlib桩函数的移植，然后编译发现出错，原来是因为我们使用编译选项-mcmodel=medany来保证程序可以链接到0x80000000以上的高地址，但是默认编译的带newlib的编译器使用了-mcmodel=medlow来编译c库，因此无法链接，又踩到一个坑，只好重新编译了。最终总结编译命令如下:

    ```shell
    # 用于裸机带newlib的gcc
    ./configure --prefix=/opt/gcc-riscv64-unknown-elf --with-cmodel=medany
    make -j16
    # 用于linux带glibc的gcc
    ./configure --prefix=/opt/gcc-riscv64-unknown-linux-gnu
    make linux -j16
    ```
    到这里我们的编译器基本更换完成，后面如果遇到问题终于不用担心别人提供的二进制开发工具是否存在问题了，我们拥有全部的源码可以自行编译、debug、fix问题，O(∩_∩)O哈哈~ 
- 
    2021.07.29(晚上):添加了qt-5.12.11，顺利完成了交叉编译，之前尝试了qt-5.15.2版本，发现该版本在不打开opengl的情况下存在bug无法正常编译，[Qt bugreports](https://bugreports.qt.io/browse/QTBUG-88017)上已经有描述该问题了，因此我们这里使用比较稳定的lts版本5.12.11。
- 
    2021.07.30(晚上):内核增加CONFIG_INPUT_EVDEV选项以方便qt应用使用/dev/input/event*设备来获取鼠标和键盘输入事件。编译了几个测试示例qt gui应用，一切正常，注意添加一些环境变量确保qt应用运转正常。目前还没完全部署好Qt库到代码里，我想多测试一些功能再考虑细节的部署问题，让部署和调试变得优雅。

    ```
    export QT_QPA_FB_DRM=1
    export QT_QPA_GENERIC_PLUGINS=evdevkeyboard
    export QT_QPA_GENERIC_PLUGINS=evdevmouse
    export QT_QPA_EVDEV_MOUSE_PARAMETERS=/dev/input/event0
    export QT_QPA_EVDEV_KEYBOARD_PARAMETERS=/dev/input/event1
    export QT_PLUGIN_PATH=/home/root/plugins
    ```

    另外年初不是基于Qt移植了个世嘉和FC的模拟器，刚好也把这个项目也部署到现在的环境里看看效果，编译非常顺利，试运行如下图：

    ![img1](./doc/img/img1.gif)

    ![img2](./doc/img/img2.gif)

    哈哈哈哈哈哈哈，这太搞笑了，居然成功在x86的pc中运行qemu模拟的riscv平台里运行linux系统并移植的基于fb的qt框架里跑模拟m68k架构应用程序成功运行了世嘉md游戏，层层套娃，试玩一下，帧率不可言表，哈哈哈哈（笑哭）！
- 
    2021.08.01(下午):添加网络相关的配置脚本，修改启动脚本配置网络dhcp等功能，添加网络相关的工具ethtool和iperf。比较顺利没啥大问题注意下/etc/inittab内的语法是busybox的不是sysvinit的不要搞错了。
- 
    2021.08.02(晚上):添加一系列启动脚本，内容参考自buildroot的配置。添加zlib和openssh源码，添加编译脚本并部署完成，qemu使用端口转发22端口到物理机3522端口，ssh和sftp已测试完全可用，这周末开始完成一些网络相关的内容，不过个人在网路这块还是小白初学者，好在这块目前完全没有遇到坑，而且资料教程颇多，我最近也要开始好好学习这方面的底层知识，后面本项目看情况缓更。
- 
    2021.08.06(晚上):target_root_app里的一些工具编译可能在原版的ubuntu18.04上不能直接编译通过，为了确保能顺利偏移通过，以目前的版本看来，automake的版本选用1.16.1，pkg-config选择0.29.2，我们直接引入源码安装到host_output目录下，后续使用host_output下的automake工具执行编译流程。
- 
    2021.08.14(下午):最近在持续补充博客文档。今天上海暴雨倾盆，一口气完成了三篇已经更新到ch15。唉，说起来今天还是七夕节，为什么我要在家些博客呢？当然是因为今天约女孩子没约到啊（苦涩）！！！这次更新够多了，休息去刷剧了。
- 
    2021.08.18(晚上):尝试了一下openjdk11的交叉编译，依赖太多了，尤其是x11那一套库，不过倒是成功完成了，而且这次是彻底把automake，pkg-config的实现逻辑搞明白了。openjdk zero实在是性能太差了，运行一条查看版本的命令都要执行七八分钟，回头有机会研究下jit的实现代码，虽然我还不觉得自己有能力移植到rv上，但是这个事情还是蛮有趣的。
- 
    2021.08.21(上午):继续加速把系列博客编辑完成，一口气完成3节，目前到了ch19节，已有计划还剩下ch20-ch25这6节，争取接下来几天全部更新完成，加油↖(^ω^)↗。
- 
    2021.08.21(晚上):没想到早上说尽快把文档编辑完成，结果下午到晚上一口气就全部更完了，后面这个项目就将真正进入缓更期了，笔者后面也需要去学习补充自己还不熟悉的技术栈了。

- 
    2021.10.16(下午):又回来更新这个项目了，首先就是给我们的虚拟SOC添加三个i2c控制器，这里我们选择qemu已有的实现imx-i2c，这个是在NXP的imx系列中的i2c ip，还是比较顺利的，很容易就添加好了，之后我们在i2c0上挂载一块 at24c64 eeprom，编写设备树文件，在linux内核中添加相关驱动，很轻松就识别到了i2c控制器以及at24c64，执行如下命令可以测试eeprom的读写情况。

    ```
    echo hello > /sys/bus/i2c/devices/0-0050/eeprom
    cat /sys/bus/i2c/devices/0-0050/eeprom
    ```

- 
    2021.10.17(上午):继续给我们的虚拟SOC添加两个spi控制器，这里我们选择qemu已有的实现sifive_spi，轻车熟路，之后我们在spi0上挂载一块is25wp256的norflash，编写设备树文件，在linux内核中添加相关驱动，记得添加mtd设备驱动，然后就很顺利的看到有/dev/mtd1的设备出现（mtd0是之前我们的SOC内部直连的并行pflash），mtd设备测试就不多说了，直接操作mtdblock1就可以，也可以使用mtd_debug小工具来操作。我们的soc内部控制器开始越来越丰富了，因此这里要整理一下qemu-6.0.0/hw/riscv/quard_star.c代码，之前是基本上从virt.c简化而来的，由于侧重点不同因此对于我们来说其代码可读性不好，这里我重构了下，针对每个不同的控制器分别编写独立的create函数，现在代码整洁多了。

    ```c
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
    ```

- 
    2021.10.30(上午):最近心情状态稳定很多，还挺开心的，继续更新这个项目，首先就是设备树文件，之前也是为了直观一点点添加内容的，这对于初学者渐进式的学习很有帮助，但是随着设备数信息愈发膨胀，平铺式的写法将难以阅读，必须重构为结构化的形式，这里重新编写以下三个文件quard_star_sbi.dts、quard_star_uboot.dts、quard_star.dtsi，dtsi为公共的SOC内部设备信息，另外两份dts则引用dtsi并针对各自的需要使能相关控制器，而挂载到控制器之上的外部设备同样也写在dts中而不是dtsi中。另外将这个仓库导入CodeFactor中，根据提示优化了部分风格不良的代码，后续不仅要让代码正确运行，还要对代码风格和可读性做进一步要求。

- 
    2021.11.06(上午):首先是优化了uboot中distro boot script，现在我们支持tftpboot，后续debug时就不用每次构建带有内核信息的文件系统，仅仅需要编译生成启动固件然后通过tftp加载即可，脚本内容如下，是通过加载主机tftp服务器中boot.cfg文件判断是否内容为“tftp”，如果没有这个文件或内容不是tftp则还是走之前的启动流程。

    ```
    # try use tftp load boot.cfg
    mw.l ${kernel_addr_r} 0x0
    dhcp ${kernel_addr_r} /boot.cfg
    
    # load bootfile according to boot.cfg
    if itest.l *${kernel_addr_r} == 0x70746674; 
    then 
        echo tftpboot...
        dhcp ${kernel_addr_r} /linux_kernel/Image
        dhcp ${fdt_addr_r} /uboot/quard_star_uboot.dtb
    else
        echo virtioboot...
        load virtio 0:1 ${kernel_addr_r} /Image
        load virtio 0:1 ${fdt_addr_r} /quard_star.dtb
    fi
    
    # boot kernel
    booti ${kernel_addr_r} - ${fdt_addr_r}
    ```

- 
    2021.11.06(下午):应该是这周最后一次更新，说起来今天还有点小忧伤，这篇更新完毕晚上准备给自己做顿盐焗鸡翅。不扯废话了，这次我们给虚拟SOC添加一个USB控制器，qemu中有dwc3的半成品实现，为啥是半成品呢——因为只实现了host模式，而没有实现otg。dwc3这个控制器想必做嵌入式的朋友都太熟悉了，Synopsys的ip，我在非常多的SOC中都见过这个控制器。阅读代码看到这个dwc3仿真似乎是Xilinx写的，被包含在xlnx-usb-subsystem中一部分，这里我们不使用XILINX_VERSAL的代码，直接创建一个dwc3设备。代码还是很简单的，如下：

    ```c
    static void quard_star_usbs_create(MachineState *machine)
    {
        QuardStarState *s = RISCV_VIRT_MACHINE(machine);
    
        object_initialize_child(OBJECT(s), "dwc3", &s->usb,
                                TYPE_USB_DWC3);
    
        sysbus_realize(SYS_BUS_DEVICE(&s->usb), &error_fatal);
    
        sysbus_mmio_map(SYS_BUS_DEVICE(&s->usb), 0, 
                                virt_memmap[QUARD_STAR_USB].base);
        qdev_pass_gpios(DEVICE(&s->usb.sysbus_xhci), DEVICE(&s->usb), SYSBUS_DEVICE_GPIO_IRQ);
        sysbus_connect_irq(SYS_BUS_DEVICE(&s->usb), 0,
                        qdev_get_gpio_in(DEVICE(s->plic), QUARD_STAR_USB_IRQ));
    }
    ```

    编写设备树文件，然后配置内核CONFIG_USB_DWC3=y，运行，这里居然出现了oops，看起来是访问异常，难道说部分寄存器没实现导致读写出错了吗？翻看了代码qemu-6.0.0/hw/usb/hcd-dwc3.c:587这边模拟器是register_read_memory/register_write_memory来访问寄存器的，qemu和内核两头加打印，最终定位到是内核linux-5.10.65/drivers/usb/dwc3/core.c:290这里访问DWC3_DCTL出错，其实问题很明确，qemu注释写明了只支持host因此device相关的寄存器都是没有实现的，那么内核里的驱动为什么还访问设备寄存器呢，毕竟我们在设备树里指定了dr_mode为host，最终修改283行判断代码添加另一个条件检查dr_mode来跳过device寄存器。考虑Xilinx自己的驱动代码是5.13才加入内核主线的，因此查了下最新的内核5.15的驱动代码有所变化，但这个判断条件仍没有修改，是否修复了看不出来，这里就先暂且放下，先按我这个修改使用。

    ```c
    if (dwc->current_dr_role == DWC3_GCTL_PRTCAP_HOST ||
        dwc->dr_mode == USB_DR_MODE_HOST)
    return 0;
    ```

    搞定之后进入系统一切顺利，使用lsusb可以看到控制器信息。

    ```
    [~]#lsusb 
    Bus 001 Device 001: ID 1d6b:0002
    Bus 002 Device 001: ID 1d6b:0003
    ```

- 
    2021.11.07(上午):添加了gpio和dma控制器，比较简单好理解的控制器。

- 
    2021.11.07(下午):添加了sdmmc控制器，现在我们可以有sd卡了。

- 
    2021.11.14(下午):这周没怎么开发这个项目，因为R星发布了GTA三部曲最终版，童年经典游戏，所以果断回味经典去了。不过还是有进度的，计划给这个项目做一个maskrom添加多种boot方式和升级固件的功能，因此初步完成了syscon的设计，添加了一个boot状态寄存器，可以通过qemu参数指定boot状态。稍晚点把代码整理好提交。

- 
    2021.11.19(晚上):把maskrom中pflash/spi/sd三种boot的代码完成了，这里要强调下maskrom中的spi和sdhci的驱动编写的非常简单而粗糙，为什么呢，因为我们要简化maskrom的代码设计，尽可能使用极少的rom和ram空间，真实的soc设计时这些静态ram的成本是很高的，当然这份驱动代码还要考虑更多性能和兼容性的问题，这里我们没有仔细斟酌，仅作示例（这里发现了个奇怪的现象，spi的驱动在maskrom和lowlevelboot状态下性能差异很大，这可能是qemu的问题，后续有空研究下）。lowlevelboot代码增加了spi和sd的情况，这里注意下代码改为在sram的地址空间执行，但是之前pflash的代码因为我们使用了纯汇编编写，因此地址是无关的，除了一个_pen变量的取址方式是需要pc，这边特殊处理下即可，这样我们的lowlevelboot代码就可以在pflash和sarm两种方式都能执行正确了。现在maskrom代码就只差升级功能没完成了，这里我计划使用xmodem协议，另外计划再写个PC升级工具用于配合升级流程。另外又整理了一些框图流程图。

- 
    2021.11.20(早上):移植xmodem传输协议到mask_rom，成功测试可以通过uart烧写lowlevelboot.bin到sram内，需要做一个好用的pc工具来对接升级，计划考虑用pyqt或者Qt来写。

- 
    2021.11.21(晚上):优化代码，添加CI来对uart升级功能进行测试，但是CI问题很多，conda的环境总是有问题，好在终于解决了，而且把之前的CI脚本做了更新，现在代码更加干净整洁了。另外，我今天心情特别糟糕，特别糟糕，特别糟糕，特别糟糕，特别糟糕，特别糟糕，特别糟糕，特别糟糕，特别糟糕，特别糟糕，特别糟糕，特别糟糕，特别糟糕，特别糟糕，特别糟糕。我想我是否应该去看心理医生了。哭。

- 
    2021.11.22(晚上):状态恢复了，心情很好很开心，是的，我现在情绪波动真的很大，我也不知道该怎么处理。好了聊正事，心情不错，今天抽空研究下为什么之前写的spi flash的驱动在maskrom上运行及其慢，而进入lowlevelboot阶段则不会运行那么慢了，虽然作为示例驱动实现的写法为低效率的轮寻，但也不应该在不同阶段执行速度不一样，这里怀疑是qemu本身实现的问题。首先qemu是拥有一个tcg子系统，用于将guest指令转换为host上的指令，我们可以从这里入手，首先运行qemu时添加参数-d op -D qemu.log，这样会将运行产生的tcg中间IR码打印出来，如下示例:

    ```
    OP:
        ld_i32 tmp0,env,$0xfffffffffffffff0
        brcond_i32 tmp0,$0x0,lt,$L0

        ---- 0000000000000000
        mov_i32 tmp0,$0x1
        st_i32 tmp0,env,$0xfffffffffffff168
        mov_i64 x3/gp,$0xe2000
        goto_tb $0x0
        mov_i64 pc,$0x4
        exit_tb $0x7fe43effe000
        set_label $L0
        exit_tb $0x7fe43effe003
    
    …………
    ```

    好嘛，这一看才发现，在maskrom阶段执行的IR码的翻译都是一条一条单指令翻译的，而到lowlevelboot阶段则是按照TB块翻译的，难怪仿真效率差这么多，这看起来像是bug，我们来追踪下qemu源码看下为什么会产生这种现象。首先找到：qemu-6.0.0/accel/tcg/translate-all.c:1862，代码片段：

    ```c
    phys_pc = get_page_addr_code(env, pc);

    if (phys_pc == -1) {
        /* Generate a one-shot TB with 1 insn in it */
        //QQM mark
        cflags = (cflags & ~CF_COUNT_MASK) | CF_LAST_IO | 1;
    }
    ```

    这里如果phys_pc为-1,则TB块的大小为1,实测果然是这里为-1了，而我们的pc地址在maskrom阶段是0地址开始的，继续追踪代码到qemu-6.0.0/accel/tcg/cputlb.c:1518，函数get_page_addr_code_hostp中代码片段如下：

    ```c
    if (unlikely(!tlb_hit(entry->addr_code, addr))) {
        if (!VICTIM_TLB_HIT(addr_code, addr)) {
            tlb_fill(env_cpu(env), addr, 0, MMU_INST_FETCH, mmu_idx, 0);
            index = tlb_index(env, mmu_idx, addr);
            entry = tlb_entry(env, mmu_idx, addr);

            if (unlikely(entry->addr_code & TLB_INVALID_MASK)) {
                /*
                 * The MMU protection covers a smaller range than a target
                 * page, so we must redo the MMU check for every insn.
                 */
                return -1;
            }
        }
        assert(tlb_hit(entry->addr_code, addr));
    }
    ```
    
    这里条件entry->addr_code & TLB_INVALID_MASK为真，这里非常有问题，为什么0地址在上电状态下TLB_INVALID_MASK，其他地址就没这个问题，继续跟踪发现函数tlb_set_page_with_attrs中代码片段如下：

    ```
    if (size < TARGET_PAGE_SIZE) {
        /* Repeat the MMU check and TLB fill on every access.  */
        address |= TLB_INVALID_MASK;
    }
    ```

    向上追踪此处的异常size来源于tlb_set_page，而针对RISCV平台的调用在发生在qemu-6.0.0/target/riscv/cpu_helper.c文件内riscv_cpu_tlb_fill函数内通过get_physical_address_pmp获取tlb_size后传递给tlb_set_page，到这里就进入到riscv的平台专有代码的，看来距离真相接近了。继续追踪代码发现在qemu-6.0.0/target/riscv/pmp.c文件内的pmp_is_range_in_tlb-->pmp_get_tlb_size函数tlb_sa在第一个4K区域内，会返回1这个size。

    ```c
    static target_ulong pmp_get_tlb_size(CPURISCVState *env, int pmp_index,
                                        target_ulong tlb_sa, target_ulong tlb_ea)
    {
        target_ulong pmp_sa = env->pmp_state.addr[pmp_index].sa;
        target_ulong pmp_ea = env->pmp_state.addr[pmp_index].ea;

        if (pmp_sa >= tlb_sa && pmp_ea <= tlb_ea) {
            //printf("1 %ld %ld %ld %ld\n",pmp_sa,tlb_sa,pmp_ea,tlb_ea);
            return pmp_ea - pmp_sa + 1;
        }

        if (pmp_sa >= tlb_sa && pmp_sa <= tlb_ea && pmp_ea >= tlb_ea) {
            return tlb_ea - pmp_sa + 1;
        }

        if (pmp_ea <= tlb_ea && pmp_ea >= tlb_sa && pmp_sa <= tlb_sa) {
            return pmp_ea - tlb_sa + 1;
        }

        return 0;
    }
    ```

    这里的问题就是env->pmp_state.addr[pmp_index].sa和env->pmp_state.addr[pmp_index].ea都等于0,而代码运行在这里时，pmp_state根本就没有做任何配置，sa和ea都是内存alloc后的默认值，阅读pmp_update_rule_addr实现不难发现，上电状态下pmp模块应该为PMP_AMATCH_OFF状态sa应为0,ea应为最大值，这样之前的pmp_is_range_in_tlb就不会返回那个奇怪的1值了。这里显然是个严重问题，guest在未初始配置pmp时，tlb_size获取错误。

    ```c
    void pmp_update_rule_addr(CPURISCVState *env, uint32_t pmp_index)
    {
        uint8_t this_cfg = env->pmp_state.pmp[pmp_index].cfg_reg;
        target_ulong this_addr = env->pmp_state.pmp[pmp_index].addr_reg;
        target_ulong prev_addr = 0u;
        target_ulong sa = 0u;
        target_ulong ea = 0u;

        if (pmp_index >= 1u) {
            prev_addr = env->pmp_state.pmp[pmp_index - 1].addr_reg;
        }

        switch (pmp_get_a_field(this_cfg)) {
        case PMP_AMATCH_OFF:
            sa = 0u;
            ea = -1;
            break;

        case PMP_AMATCH_TOR:
            sa = prev_addr << 2; /* shift up from [xx:0] to [xx+2:2] */
            ea = (this_addr << 2) - 1u;
            break;

        case PMP_AMATCH_NA4:
            sa = this_addr << 2; /* shift up from [xx:0] to [xx+2:2] */
            ea = (sa + 4u) - 1u;
            break;

        case PMP_AMATCH_NAPOT:
            pmp_decode_napot(this_addr, &sa, &ea);
            break;

        default:
            sa = 0u;
            ea = 0u;
            break;
        }

        env->pmp_state.addr[pmp_index].sa = sa;
        env->pmp_state.addr[pmp_index].ea = ea;
    }
    ```

    由于这部分代码逻辑比较复杂，目前我还没找到一个合适的地方添加对pmp_state.addr初值的init，所以，目前我的解决办法是在maskrom代码内一开始的位置添加

    ```asm
    csrw pmpcfg0, 0
  	csrw pmpcfg1, 0
  	csrw pmpcfg2, 0
  	csrw pmpcfg3, 0
    ```

    以此将pmp配置到正确的初始值，修改和再次测试spi flash的驱动，果然问题解决。最后qemu的tcg非常有意思，这里推荐一些[博客](https://airbus-seclab.github.io/qemu_blog/tcg_p1.html)给大家，感兴趣可以阅读。

- 
    2021.12.04(凌晨):目前项目功能添加的差不多了，没有太多想法，只差些细节优化可以慢慢来做。然后这两天突发奇想想给这个项目做个漂亮的GUI前端，以一块开发板的形式展现出来，可以点击开发板上的开关启动qemu，并且点击串口，vga接口来查看仿真的输出，点击flash器件加载固件等等，虽说技术价值不大，但是很好玩很有趣就好了。经过一天的努力，终于肝到凌晨这个工具初具形态了。我来录制个gif给大家看看。

    ![img7](./doc/img/img7.gif)

- 
    2021.12.05(晚上):gui前端tools修修代码风格，添加命令行选项，更新主界面里不美观的建模，这里特别鸣谢[MignonZhu](https://github.com/MignonZhu)提供3D模型的，让UI变的更加好看。另外GIMP我现在使用相当熟练了，完全不输PS，甚至更好。
