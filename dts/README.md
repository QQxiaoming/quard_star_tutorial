# DTS

dts目录存放quard star soc的设备树源文件和distro boot script文件。

## 文件说明

- quard_star.dtsi：soc设备树文件。

- quard_star_sbi.dts：opensbi所需的设备树文件，主要添加domain的描述。

- quard_star_uboot.dts：uboot和kernel所需的设备树文件，soc集成ip和外部挂载的设备描述（i2c/spi总线挂载的设备等）。

- quard_star_uboot.cmd：distro boot script文件，uboot启动加载。
