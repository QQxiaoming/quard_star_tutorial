# Mask ROM Code

mask_rom目录存放quard star soc的Mask ROM代码。Mask ROM力求简单可靠安全低rom/ram开销以及通用扩展。

## 功能描述

- boot引导，根据syscon寄存器内读取boot引脚的上电电平，选择不同的器件引导系统，目前支持pflash、spi nor flash、sd卡。

- update升级，根据syscon寄存器内读取boot引脚的上电电平，选择是否进入升级模式，升级模式下，通过串口xmodem协议将一段代码写入sram中，跳转运行，由这段代码完成后续升级。
