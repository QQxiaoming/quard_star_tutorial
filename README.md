# 基于qemu从0开始构建嵌入式linux系统

## 引言

本项目旨在真正从0开始构建嵌入式linux系统，为了剖析芯片从上电开始执行第一条指令到整个系统运行，相关应用服务启动，因此不使用市面上真实的板子，基于qemu定制模拟器开发板，且不使用qemu提供的快速加载elf的文件方式，因而我们需要下载qemu-6.0.0源码，自己编写(或使用qemu自带的)硬件ip相关模拟代码定制属于自己的硬件板卡。本项目同步制作博客专栏（暂时只发送到本人的博客），因此项目尽量保证每增加一个feature就提交一次，方便索引跟踪开发过程以方便对应到博客的具体章节。

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

