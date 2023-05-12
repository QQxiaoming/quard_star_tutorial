[![Linux ci](https://img.shields.io/github/actions/workflow/status/qqxiaoming/quard_star_tutorial/linux.yml?branch=main&logo=linux)](https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/linux.yml)
[![Windows ci](https://img.shields.io/github/actions/workflow/status/qqxiaoming/quard_star_tutorial/windows.yml?branch=main&logo=windows)](https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/windows.yml)
[![Macos ci](https://img.shields.io/github/actions/workflow/status/qqxiaoming/quard_star_tutorial/macos.yml?branch=main&logo=apple)](https://github.com/QQxiaoming/quard_star_tutorial/actions/workflows/macos.yml)
[![Documentation Status](https://img.shields.io/readthedocs/quard-star-tutorial.svg?logo=readthedocs)](https://quard-star-tutorial.readthedocs.io/zh_CN/latest/?badge=latest)
[![CodeFactor](https://img.shields.io/codefactor/grade/github/qqxiaoming/quard_star_tutorial.svg?logo=codefactor)](https://www.codefactor.io/repository/github/qqxiaoming/quard_star_tutorial)
[![GitHub tag (latest SemVer)](https://img.shields.io/github/tag/QQxiaoming/quard_star_tutorial.svg?logo=git)](https://github.com/QQxiaoming/quard_star_tutorial/releases)
[![GitHub All Releases](https://img.shields.io/github/downloads/QQxiaoming/quard_star_tutorial/total.svg?logo=pinboard)](https://github.com/QQxiaoming/quard_star_tutorial/releases)
[![GitHub stars](https://img.shields.io/github/stars/QQxiaoming/quard_star_tutorial.svg?logo=github)](https://github.com/QQxiaoming/quard_star_tutorial)
[![GitHub forks](https://img.shields.io/github/forks/QQxiaoming/quard_star_tutorial.svg?logo=github)](https://github.com/QQxiaoming/quard_star_tutorial)
[![Gitee stars](https://gitee.com/QQxiaoming/quard_star_tutorial/badge/star.svg?theme=dark)](https://gitee.com/QQxiaoming/quard_star_tutorial)
[![Gitee forks](https://gitee.com/QQxiaoming/quard_star_tutorial/badge/fork.svg?theme=dark)](https://gitee.com/QQxiaoming/quard_star_tutorial)

# Building Embedded Linux System based on QEMU

English | [简体中文](./README_zh_CN.md)

![Logo](./tutorial/img/img6.gif)

[video.webm](https://user-images.githubusercontent.com/27486515/201457240-ebede26a-0d15-4deb-9a23-1b11fb91018c.webm)

This project aims to build an Embedded Linux System, in order to analyze the chip from the power-on execution of the first instruction to the entire system running, based on a customized qemu simulator development board.

## What is Quard Star?

Quard Star is the name of the hardware board that we simulate using qemu. It is a RISC-V processor with various peripherals and controllers. We will use this board to learn how to build and run an embedded Linux system from scratch.

### Hardware Architecture


![Hardware Architecture](./tutorial/img/img3.png)

### Software Flow

![Software Flow](./tutorial/img/img4.png)

### Memory Distribution

![Memory Distribution](./tutorial/img/img5.png)

## Features

- Customized qemu simulator with Quard Star board support
- Low-level boot code and mask ROM
- OpenSBI and U-Boot bootloader
- Linux kernel and device tree
- Busybox-based root filesystem
- Qt-based graphical user interface
- Trusted domain and secure boot
- Debugging tools and scripts

## Prerequisites

To build and run this project, you will need the following tools:

- gcc and g++ compilers
- make and cmake utilities
- git version control system
- qemu emulator
- dtc device tree compiler
- python3 interpreter

You can install them using your package manager. For example, on Ubuntu 20.04, you can run:

```shell
sudo apt-get update
sudo apt install ninja-build pkg-config libglib2.0-dev libpixman-1-dev libgtk-3-dev libcap-ng-dev libattr1-dev libsdl2-dev device-tree-compiler bison flex gperf intltool mtd-utils libpulse-dev libalsa-ocaml-dev libsdl2-dev libslirp-dev
```

## Installation

To install this project, you can clone this repository using git:

```shell
git clone https://github.com/QQxiaoming/quard_star_tutorial.git
```

Then, you can run the build.sh script to compile the qemu simulator and other components:

```shell
cd quard_star_tutorial
./build.sh all all
```

This will generate the output directory with all the binaries and files needed to run the project.

## Usage

To run the project, you can use the run.sh script:

```shell
./run.sh graphic
```

This will launch the qemu simulator with the Quard Star board and load the Linux system. You should see a graphical user interface on a new window.

You can also use some command-line options to customize your run. For example:

```shell
./run.sh nographic
```

For more details on how to use these options, please refer to the tutorial files.

## Tutorial

This project comes with a series of tutorial files that explain each step of building and running an embedded Linux system based on QEMU. You can find them in the tutorial directory. They are available in Chinese languages.

The tutorial files cover the following topics:

- Setting up the QEMU environment
- Writing low-level boot code and mask ROM
- Using OpenSBI and U-Boot bootloader
- Compiling Linux kernel and device tree
- Creating root filesystem and boot image
- Developing graphical user interface with Qt
- Implementing trusted domain and secure boot
- Debugging with gdb and kgdb

You can also access the tutorial online on my [blog](https://blog.csdn.net/weixin_39871788/category_11180842.html) or on [read the docs](https://quard-star-tutorial.readthedocs.io/zh_CN/latest/index.html).

## Contributing

This project is open source and welcomes contributions from anyone who is interested in learning or teaching embedded Linux system development. If you want to contribute, please follow these steps:

1. Fork this repository on GitHub.
2. Create a new branch for your feature or bugfix.
3. Make your changes and commit them with clear messages.
4. Push your branch to your forked repository.
5. Create a pull request from your branch to the main repository.
6. Wait for review and feedback.

Please make sure your code follows the coding style and conventions of this project. Also, please write comments and documentation for your code.

## License

This project uses some open source projects as components or references. I would like to thank and acknowledge their authors and contributors for their great work. so the newly added codes of each part respect the original project license. The rest of the original code open source license is BSD-3-Clause License. See the LICENSE file for more details.

## Contact

If you have any questions, suggestions, or feedback about this project, please feel free to contact me at 2014500726@smail.xtu.edu.cn or leave a comment on my blog.

## Further Reading

If you want to read more about the development process and challenges of this project, you can check out my development note file DEVELOPNOTE.md. It contains some insights and tips on how I built and debugged this project.😎

Note: This project also has an experimental branch called next-kernel that uses the latest Linux kernel from [linux-next](https://git.kernel.org/pub/scm/linux/kernel/git/next/linux-next). It is used to learn and test new features and functions. It might be unstable or incompatible with some components. You can switch to this branch using git checkout next-kernel if you are interested, but please be careful and report any issues or bugs you encounter.😉

## Thanks

I would like to acknowledge AI tool (New Bing) for assisting me with writing part of this readme file. 
