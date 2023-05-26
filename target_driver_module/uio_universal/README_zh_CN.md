[![CI](https://github.com/QQxiaoming/uio_universal/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/QQxiaoming/uio_universal/actions/workflows/ci.yml)
[![CodeFactor](https://www.codefactor.io/repository/github/qqxiaoming/uio_universal/badge)](https://www.codefactor.io/repository/github/qqxiaoming/uio_universal)
[![License](https://img.shields.io/github/license/qqxiaoming/uio_universal.svg?colorB=f48041&style=flat-square)](https://github.com/QQxiaoming/uio_universal)

# uio_universal

[English](./README.md) | 简体中文

## 介绍

这是一个使用uio的通用设备驱动，加载驱动后，会产生一个/dev/uioX设备，用户可以通过用户态应用程序编写相应的设备驱动。

## 安装

```shell
make
make load
```

## 卸载

```shell
make unload
```

# 示例 dts

```dts
    uart1:uart1@10001000 {
        interrupts = <11>;
        interrupt-parent = <&plic>;
        clock-frequency = <0x384000>;
        reg = <0x0 0x10001000 0x0 0x100>;
        compatible = "uio-universal";
        uio-name = "ns16550a";
    };
```

# 更多

当前代码仍属于最初阶段，尚未达到稳定可用的程度。
