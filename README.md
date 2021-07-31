# 基于qemu从0开始构建嵌入式linux系统

## 引言

本项目旨在真正从0开始构建嵌入式linux系统，为了剖析芯片从上电开始执行第一条指令到整个系统运行，相关应用服务启动，因此不使用市面上真实的板子，基于qemu定制模拟器开发板，且不使用qemu提供的快速加载elf的文件方式，因而我们需要下载qemu-6.0.0源码，自己编写(或使用qemu自带的)硬件ip相关模拟代码定制属于自己的硬件板卡。本项目同步制作[博客专栏](https://blog.csdn.net/weixin_39871788/category_11180842.html)（暂时只发送到本人的博客），因此项目尽量保证每增加一个feature就提交一次，方便索引跟踪开发过程以方便对应到博客的具体章节。

## 环境搭建

ubuntu18.04需要使用apt安装的包:

```shell
sudo apt install ninja-build pkg-config libglib2.0-dev libpixman-1-dev libgtk-3-dev libcap-ng-dev libattr1-dev device-tree-compiler bison flex
```

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
    
    但是要注意到[内核文档>devices.rst](linux-5.10.42/Documentation/admin-guide/devices.rst)第155行提到了要将devpts挂载到/dev/pts这样linux风格的pty设备才能使用，原来是这样( ^_^ )！我们在目标文件系统配置文件/etc/fstab使用mdev的方式生成/dev目录，但是并没有给我们创建/dev/pts目录，因此不能直接在/etc/fstab添加挂载设备信息，那我们就还是在/etc/init.d/rcS启动脚本中添加，在/sbin/mdev -s后添加
    
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
 