# Update Tools

update_tools目录存放配合quard star soc的Mask ROM和updater code用来升级firmware的工具代码，代码基于python3开发。

## 环境搭建

目前依赖的组件如下：

telnetlib3==1.0.4
pyqt=5.12.3
zlib=1.2.11

## 功能开发

Update Tools对接maskrom已可以将大小256KB以内的二进制bin文件，写入0x20000地址并跳转执行。

目前lowlevelboot.bin可以在此地址运行，因此暂时充当update.bin，后续更多功能待完善。
