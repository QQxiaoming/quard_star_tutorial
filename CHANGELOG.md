# Change Log

## [Unrelease]

- qemu更新到7.2.0
- kernel更新到新的LTS版本6.1.11
- opensbi更新到1.2
- GUI tools开始使用qt6构建（意味着开始支持apple silicon平台的mac系统，放弃windows7及以下的旧版windows系统支持）
- GUI tools工具增加网络物理接口设置连接与断开模拟
- GUI tools工具增加boot选项配置
- GUI tools工具修复版本号显示错误

## [0.1.0] - 2022-11-05

- 优化大量脚本提高可读性和易用性
- 完善GUI tools工具，基础功能完善

## [0.0.3] - 2022-07-12

- rootfs更新为2G空间
- 添加onenand flash emulator（uboot、kernel驱动支持）
- 添加can controller emulator（kernel驱动支持）
- 添加pwm/timer/adc emulator（kernel驱动支持）
- 添加eth emulator （uboot、kernel驱动支持）
- 添加lcdc emulator （uboot、kernel驱动支持）
- 添加watchdog emulator （kernel驱动支持）

## [0.0.2] - 2022-04-25

- busybox更新到1.33.2
- kernel更新到5.10.108
- opensbi更新到1.0
- qemu更新到7.0.0
- 添加macos二进制发布版本
- 添加i2s硬件
- 启动脚本添加avahi
- 添加tap模拟网卡支持

## [0.0.1] - 2021-12-08

- 首次发布
- qemu和quard stat tools的二进制发布版本，包含linux版本和windus x64版本，win64为实验性支持，强烈建议以使用linux版本在ubuntu2004版本使用，其他os若无法使用，建议clone源码自行编译，编译方法参考项目CI脚本
