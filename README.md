[![Linux ci](https://img.shields.io/github/workflow/status/qqxiaoming/quard_star_tutorial/linux?logo=linux)](https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/linux.yml)
[![Windows ci](https://img.shields.io/github/workflow/status/qqxiaoming/quard_star_tutorial/windows?logo=windows)](https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/windows.yml)
[![Macos ci](https://img.shields.io/github/workflow/status/qqxiaoming/quard_star_tutorial/macos?logo=apple)](https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/macos.yml)
[![Documentation Status](https://readthedocs.org/projects/quard-star-tutorial/badge/?version=latest)](https://quard-star-tutorial.readthedocs.io/zh_CN/latest/?badge=latest)
[![CodeFactor](https://www.codefactor.io/repository/github/qqxiaoming/quard_star_tutorial/badge)](https://www.codefactor.io/repository/github/qqxiaoming/quard_star_tutorial)
[![GitHub tag (latest SemVer)](https://img.shields.io/github/tag/QQxiaoming/quard_star_tutorial.svg)](https://github.com/QQxiaoming/quard_star_tutorial/releases)
[![GitHub All Releases](https://img.shields.io/github/downloads/QQxiaoming/quard_star_tutorial/total.svg)](https://github.com/QQxiaoming/quard_star_tutorial/releases)
[![GitHub stars](https://img.shields.io/github/stars/QQxiaoming/quard_star_tutorial.svg)](https://github.com/QQxiaoming/quard_star_tutorial)
[![GitHub forks](https://img.shields.io/github/forks/QQxiaoming/quard_star_tutorial.svg)](https://github.com/QQxiaoming/quard_star_tutorial)
[![Gitee stars](https://gitee.com/QQxiaoming/quard_star_tutorial/badge/star.svg?theme=dark)](https://gitee.com/QQxiaoming/quard_star_tutorial)
[![Gitee forks](https://gitee.com/QQxiaoming/quard_star_tutorial/badge/fork.svg?theme=dark)](https://gitee.com/QQxiaoming/quard_star_tutorial)

# Building Embedded Linux System based on QEMU

English | [简体中文](./README_zh_CN.md)

![Logo](./tutorial/img/img6.gif)

[video.webm](https://user-images.githubusercontent.com/27486515/201457240-ebede26a-0d15-4deb-9a23-1b11fb91018c.webm)

## Introduction

This project aims to build an Embedded Linux System. In order to analyze the chip from power-on to execute the first instruction to the entire system running, related application services are started, so the real board on the market is not used, based on the qemu simulator Development board, and does not use the fast loading elf file method provided by qemu, so we need to download the qemu source code, write our own (or use qemu's own) hardware ip-core simulation code to customize our own hardware board. This project is synchronously produced [Blog](https://blog.csdn.net/weixin_39871788/category_11180842.html)*(Temporarily Only Chinese)*(2021.8.11: Now I start building the [documentation](https://quard-star-tutorial.readthedocs.io/zh_CN/latest/index.html) using readthedocs, the content is the same as the blog), so the project should try to ensure that it is submitted every time a feature is added, so that the index can track the development process and correspond to the specific chapters of the blog.

## Environment Setup

Ubuntu20.04 needs to use apt to install the package:

```shell
sudo apt install ninja-build pkg-config libglib2.0-dev libpixman-1-dev libgtk-3-dev libcap-ng-dev libattr1-dev libsdl2-dev device-tree-compiler bison flex gperf intltool mtd-utils libpulse-dev libalsa-ocaml-dev libsdl2-dev libslirp-dev
```

## Hardware Architecture

The figure below shows the current virtual hardware architecture, which will be continuously updated as the project progresses.

![Hardware Architecture](./tutorial/img/img3.png)

## Software Flow

The following figure shows the current software flow, which will be continuously updated as the project progresses.

![Software Flow](./tutorial/img/img4.png)

## Memory Distribution

The following figure shows the current memory distribution, which will be continuously updated as the project progresses.

![Memory Distribution](./tutorial/img/img5.png)

## Development Note

[Develop Note](./DEVELOPNOTE.md) *(Temporarily Only Chinese)*

## More

The main branch of the project is currently a stable branch, which is used for supporting tutorial documents. The next-kernel branch is used to learn the latest functions on the experimental kernel. Periodically sync with [linux-next](https://git.kernel.org/pub/scm/linux/kernel/git/next/linux-next).
