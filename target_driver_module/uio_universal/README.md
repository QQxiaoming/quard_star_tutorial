[![CI](https://github.com/QQxiaoming/uio_universal/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/QQxiaoming/uio_universal/actions/workflows/ci.yml)
[![CodeFactor](https://www.codefactor.io/repository/github/qqxiaoming/uio_universal/badge)](https://www.codefactor.io/repository/github/qqxiaoming/uio_universal)
[![License](https://img.shields.io/github/license/qqxiaoming/uio_universal.svg?colorB=f48041&style=flat-square)](https://github.com/QQxiaoming/uio_universal)

# uio_universal

English | [简体中文](./README_zh_CN.md)

## Introduction

This is a universal device driver using uio. After loading the driver, a /dev/uioX device will be generated. Users can write corresponding device drivers through user mode applications.

## Install

```shell
make
make load
```

## Uninstall

```shell
make unload
```

# Example dts

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

## More

The current code is still in the initial stage and has not yet reached a stable and usable level.
