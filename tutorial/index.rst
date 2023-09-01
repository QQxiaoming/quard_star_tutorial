主页
----------------------------------

.. image:: https://img.shields.io/github/actions/workflow/status/qqxiaoming/quard_star_tutorial/linux.yml?branch=main&logo=linux
   :target: https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/linux.yml
   :alt: Linux ci
.. image:: https://img.shields.io/github/actions/workflow/status/qqxiaoming/quard_star_tutorial/windows.yml?branch=main&logo=windows
   :target: https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/windows.yml
   :alt: Windows ci
.. image:: https://img.shields.io/github/actions/workflow/status/qqxiaoming/quard_star_tutorial/macos.yml?branch=main&logo=apple
   :target: https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/macos.yml
   :alt: Macos ci
.. image:: https://img.shields.io/github/actions/workflow/status/qqxiaoming/quard_star_tutorial/android.yml?branch=main&logo=android
   :target: https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/android.yml
   :alt: Android ci
.. image:: https://img.shields.io/github/actions/workflow/status/qqxiaoming/quard_star_tutorial/ios.yml?branch=main&logo=appstore
   :target: https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/ios.yml
   :alt: IOS ci
.. image:: https://img.shields.io/readthedocs/quard-star-tutorial.svg?logo=readthedocs
   :target: https://quard-star-tutorial.readthedocs.io/zh_CN/latest/?badge=latest
   :alt: Documentation Status
.. image:: https://img.shields.io/codefactor/grade/github/qqxiaoming/quard_star_tutorial.svg?logo=codefactor
   :target: https://www.codefactor.io/repository/github/qqxiaoming/quard_star_tutorial
   :alt: CodeFactor
.. image:: https://img.shields.io/github/tag/QQxiaoming/quard_star_tutorial.svg?logo=git
   :target: https://github.com/QQxiaoming/quard_star_tutorial/releases
   :alt: GitHub tag (latest SemVer)
.. image:: https://img.shields.io/github/downloads/QQxiaoming/quard_star_tutorial/total.svg?logo=pinboard
   :target: https://github.com/QQxiaoming/quard_star_tutorial/releases
   :alt: GitHub All Releases
.. image:: https://img.shields.io/github/stars/QQxiaoming/quard_star_tutorial.svg?logo=github
   :target: https://github.com/QQxiaoming/quard_star_tutorial
   :alt: GitHub stars
.. image:: https://img.shields.io/github/forks/QQxiaoming/quard_star_tutorial.svg?logo=github
   :target: https://github.com/QQxiaoming/quard_star_tutorial
   :alt: GitHub forks
.. image:: https://gitee.com/QQxiaoming/quard_star_tutorial/badge/star.svg?theme=dark
   :target: https://gitee.com/QQxiaoming/quard_star_tutorial
   :alt: Gitee stars
.. image:: https://gitee.com/QQxiaoming/quard_star_tutorial/badge/fork.svg?theme=dark
   :target: https://gitee.com/QQxiaoming/quard_star_tutorial
   :alt: Gitee forks

.. image:: ./img/img6.gif

********
硬件架构
********
.. image:: ./img/img3.png

********
软件流程
********
.. image:: ./img/img4_zh_CN.png

********
内存分布
********
.. image:: ./img/img5.png

.. toctree::
   :maxdepth: 3
   :caption: 正文

   ch0.引言<ch0.md>
   ch1.搭建qemu环境<ch1.md>
   ch2.添加qemu仿真板——Quard-Star板<ch2.md>
   ch3.添加中断控制器、串口、pflash支持<ch3.md>
   ch4.制作测试固件验证串口打印<ch4.md>
   ch5-1.什么是多级BootLoader与opensbi(上)<ch5-1.md>
   ch5-2.什么是多级BootLoader与opensbi(下)<ch5-2.md>
   ch6.设备树文件与opensbi加载运行<ch6.md>
   ch7.OpenSBI Domain<ch7.md>
   ch8.U-Boot<ch8.md>
   ch9.U-Boot platform添加与内存分配重规划<ch9.md>
   ch10.qemu之virtio与fw_cfg<ch10.md>
   ch11-1.向着linux Kernel出发！——加载引导<ch11-1.md>
   ch11-2.向着linux Kernel出发！——完成启动<ch11-2.md>
   ch12.busybox<ch12.md>
   ch13.最小根文件系统完成<ch13.md>
   ch14.动态链接——elf文件的加载<ch14.md>
   ch15.linux多用户管理<ch15.md>
   ch16.Framebuffer显示设备<ch16.md>
   ch17.bash——交叉编译应用<ch17.md>
   ch18.RTC设备与系统控制设备<ch18.md>
   ch19.交叉编译sudo<ch19.md>
   ch20.交叉编译screen、tree、cu<ch20.md>
   ch21.实时操作系统FreeRTOS移植RISCV-S模式<ch21.md>
   ch22.编译器与C/C++标准库<ch22.md>
   ch23.linux FB应用——Qt库移植<ch23.md>
   ch24.qemu网卡/linux内核网络配置<ch24.md>
   ch25.sshd服务配置<ch25.md>

.. toctree::
   :maxdepth: 3
   :caption: 扩展文章

   kgdb调试linux内核以及驱动模块<ext1.md>
   gcc编译选项pg在嵌入式profile中应用<ext2.md>
   qemu常用参数选项说明<ext3.md>
   使用qemu的gdbstub调试<ext4.md>
   QEMU CAN总线<ext5.md>
   嵌入式Linux上ifpulgd的使用配置与qemu模拟验证<ext6.md>
