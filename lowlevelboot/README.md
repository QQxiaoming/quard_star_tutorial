# lowlevelboot

lowlevelboot目录存放quard star soc的lowlevelboot代码。该代码作为板级低阶引导代码的示例，主要完成ddr初始化，并且加载opensbi、uboot等上层代码。同时这份代码也作为update升级代理代码示例，在另一个分支下执行升级firmware的功能。

## 功能描述

- boot引导，根据syscon寄存器内读取boot引脚的上电电平，加载由opensbi、uboot打包的firmware，跳转引导。

- update升级，根据syscon寄存器内读取maskrom指示，选择是否进入升级模式（该功能还未完全完成）。
