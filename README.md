# 基于qemu从0开始构建嵌入式linux系统

## 引言

本项目旨在真正从0开始构建嵌入式linux系统，为了剖析芯片从上电开始执行第一条指令到整个系统运行，相关应用服务启动，因此不使用市面上真实的板子，基于qemu定制模拟器开发板，且不使用qemu提供的快速加载elf的文件方式，因而我们需要下载qemu-6.0.0源码，自己编写(或使用qemu自带的)硬件ip相关模拟代码定制属于自己的硬件板卡。本项目同步制作[博客专栏](https://blog.csdn.net/weixin_39871788/category_11180842.html)（暂时只发送到本人的博客），因此项目尽量保证每增加一个feature就提交一次，方便索引跟踪开发过程以方便对应到博客的具体章节。

## 环境搭建

ubuntu18.04需要使用apt安装的包:

```shell
sudo apt install ninja-build pkg-config libglib2.0-dev libpixman-1-dev libgtk-3-dev libcap-ng-dev libattr1-dev device-tree-compiler bison flex
```

## 开发记录

- 2021.07.04(下午):增加了Quard Star开发板定义，8核支持，增加中断控制器、串口、pflash。添加低阶启动代码固化在pflash运行。
- 2021.07.05(晚上):增加了opensbi源码并添加platform支持代码，低阶启动代码添加加载sbi功能。
- 2021.07.09(晚上):增加了opensbi设备树中domain定义，并增加安全域的工程代码，使用安全域串口3打印；增加u-boot代码并引导进入非安全域执行，使用串口0打印。
- 2021.07.10(中午):增加uboot的platform定义以及配置等代码。
- 2021.07.10(下午):使用之前制作的根文件系统及内核映像尝试引导正确，设备树文件有所改动，且发现了点问题，1.当前版本的opensbi fix下一级设备树文件时会添加不可用的domain到设备树的reserved-memory节点，但存在重叠的情况，这在u-boot中解析设备树时是会报warring的，因此这里将域修改为分块的域，并不优雅但暂时就这么解决。2.u-boot在启动bl1阶段会拷贝代码到ddr高地址，而如果我们将高地址配置为了不可访问的安全domain时，当前u-boot的riscv实现中没有检查是否是reserved-memory区域，这里我们为了简单期间直接在我们的board platform代码层添加hook函数修改gd->ram_top来实现，其实这种方式最简单高效，如果都从设备树里解析未免过度设计了，这里本就只是启动引导的一小段环节而已，但是看起来uboot的趋势越来越向解析设备树方向靠近了。3.另外发现linux5.10.42在qemu使用-device virtio-gpu-device驱动工作不正常，内核oops了，目前不清楚是qemu-6.0.0的问题还是内核的代码，这个我有空去查一下。
- 2021.07.11(上午):添加linux kernel的编译，busybox的简易编译，编写根文件系统映像的打包脚本，添加简易etc配置文件，终于shell运行成功，后面要开始完善一些系统配置，用户态程序以及增加新显示驱动设备，我们打算移植Qt界面到系统内。另外安全域内的固件打算移植一些RTOS系统，看是移植FreeRTOS还是RTThread比较好呢(⊙o⊙)?
- 2021.07.12(晚上):使用-device virtio-gpu-device选项添加virtio显示设备时，内核出现了oops，地址跑飞的问题，我稍微花了点时间查找了一下，直接错误出现在linux-5.10.42/drivers/gpu/drm/virtio/virtgpu_kms.c:242行此处vgdev为空指针，直接访问会出错。为什么会是空指针呢，原来是因为110行处初始化时检查VIRTIO_F_VERSION_1不通过，因此驱动并没有正常被加载，而后release时出现空指针。明确了这个问题我们就要去qemu源码里查找一下为什么不支持这个VIRTIO_F_VERSION_1这个feature，经过查找终于发现在qemu-6.0.0/hw/virtio/virtio-mmio.c:712处，只有定义了proxy->legacy为false时才能添加这个feature，而722行给出默认配置为true，因此我们在运行qemu选项添加-global virtio-mmio.force-legacy=false选项后，终于一切正常了。这里我们参考一下这篇[qemu开发邮件](https://www.mail-archive.com/search?l=qemu-devel@nongnu.org&q=subject:%22Re%5C%3A+%5C%5BQemu%5C-devel%5C%5D+%5C%5BPATCH+v3%5C%5D+virtio%5C-mmio%5C%3A+implement+modern+%5C%28v2%5C%29%2C+personality+%5C%28virtio%5C-1%5C%29%22&o=newest&f=1)不难发现，force-legacy定义为false才是更新版本的定义，但可能是考虑兼容性的问题qemu默认还是给出了true才用传统的virtio行为。解决了这个问题，我们重新添加内核参数配置，以及内核定义一个我们的quard_star_defconfigs文件，添加显示linux启动logo的配置，再次启动qemu，ok！以下为上文部分描述中代码片段：

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

- 2021.07.13(晚上):将启动使用的shell从busybox提供的sh改为标准的bash，以方便后续开发。不过最麻烦的就是ncurses库的编译安装了，动态库可以从交叉编译工具链中拷贝，但是一些终端相关的配置文件和数据就要去编译ncurses库来安装了，好在基本上搞定了。另外注意从这里开始，我们新增给系统roofs.img中添加的内容都是通过目标虚拟机挂载共享目录然后安装的，而不是最处由脚本构建打包的，因此务必注意除非img被破坏，否则不要重新构建img，尽可能使用虚拟机本身去修改img内的文件。
- 2021.07.14(晚上):添加rtc和syscon到系统内，解决了虚拟机内时间戳不准确问题，解决了reboot不能复位系统，另外之前内核resetvec字段没有定义到rom的基地址，而是默认的0x1000，虽没有产生错误，但是cpu启动时确认读入0x1000地址，与我们的期望不符，这次reboot可以看到其错误，修改后问题解决。关于上述cpu执行启动地址问题你可以添加qemu启动选项-d in_asm -D qemu_run_asm.log来观察行为。另外目前的实现reboot和poweroff这两个配置在kernel的设备树中其实是不太优雅的设计，我们使用的linux内核版本中sbi已经支持了poweroff，所以syscon-poweroff注册会失败，而syscon-reboot会注册成功，因为sbi还没有实现reboot，鉴于opensbi还在快速开发阶段，相信这个问题已经在逐渐完善了，感兴趣的朋友可以关注下这些项目的主线开发分支情况，这里我们就不去纠结这个设计了。
- 2021.07.15(晚上):将sudo和screenfetch移植到系统内，screenfetch稍微修改了一点，毕竟我们是busybox但是换了标准的bash，检测有的错误。另外今天在@zhiliao007(wenqing.li@aliyun.com)的帮助下确认了开启这个项目的所需要使用apt安装的包，由于我启动项目时使用的pc已经安装了很多开发环境了，因此一直没能确认如果是新的Ubuntu主机需要安装什么基础库，这里提出感谢。
- 2021.07.17(下午):这几天工作有点忙，终于周末有空继续这个项目了，首先是运行qemu时使用gtk显示界面是目前看起来最理想的方式，为了使用更大分辨率建议添加配置使其自动缩放启动参数如下--display gtk,zoom-to-fit=false。然后我将我一直在用的一些比较好用的工具继续向系统移植tree,screen,cu这三个工具，尤其是screen，毕竟只有终端的日子可能还要持续很久，那么一个漂亮的多终端工具就非常重要了，这些工具的移植配置还算是容易，但是终端窗口大小从主机测动态传入就比较麻烦了，这时候我们之前的fw_cfg就起到了大作用，我们启动脚本计算了终端窗口大小通过fw_cfg传入目标机，然后目标机通过cat /sys/firmware/qemu_fw_cfg/by_name/opt/qemu_cmdline/raw就可以读取到。不过注意了，由于qemu是在6.0版本才给riscv架构添加fw_cfg，而linux5.10.42还没有更新到，因此我们这里手动稍微修改下内核里的cofing平台过滤配置就可以使用啦。相信最新版主线上linux内核已经更新了这个功能，所以如果想体验新内核也可以更新最新内核来尝鲜。下面放张图看看开发到当前的效果。

![img0](./doc/img/img0.png)

