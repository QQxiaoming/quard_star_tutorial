# 基于qemu-riscv从0开始构建嵌入式linux系统ch0.引言

## 引言

本项目旨在真正从0开始构建嵌入式linux系统，为了剖析芯片从上电开始执行第一条指令到整个系统运行，相关应用服务启动，因此不使用市面上真实的板子，基于qemu定制模拟器开发板，且不使用qemu提供的快速加载elf的文件方式，因而我们需要下载qemu-6.0.0源码，自己编写(或使用qemu自带的)硬件ip相关模拟代码定制属于自己的硬件板卡。本项目同步制作博客专栏（暂时只发送到本人的博客），因此项目尽量保证每增加一个feature就提交一次，方便索引跟踪开发过程以方便对应到博客的具体章节。

ubuntu18.04需要使用apt安装的包:

```shell
sudo apt install ninja-build pkg-config libglib2.0-dev libpixman-1-dev libgtk-3-dev libcap-ng-dev libattr1-dev device-tree-compiler bison flex
```

> 本教程的<br>github仓库：https://github.com/QQxiaoming/quard_star_tutorial<br>gitee仓库：https://gitee.com/QQxiaoming/quard_star_tutorial（由于仓库体积过大，gitee已无法同步github仓库内容更新，建议可以先用gitee克隆仓库，然后修改远端地址为gitehub地址再进行同步）<br>readthedocs文档：https://quard-star-tutorial.readthedocs.io/zh_CN/latest/?badge=latest<br>仓库代码更新快于博客更新的，因此如果想提前了解后续内容可以前往github仓库查看。
