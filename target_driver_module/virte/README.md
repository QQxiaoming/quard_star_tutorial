[![CI](https://github.com/QQxiaoming/virte/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/QQxiaoming/virte/actions/workflows/ci.yml)

# virte

## 介绍

这是一个软件模拟的ETH网卡设备驱动，加载驱动后，会产生eth(X)和eth(X+1)两个设备，不同的应用程序分别打开这两个网卡设备可以互相通信(注意要使用linux的网络命名空间隔离两个设备以免进入本地lo网络设备)。

## 安装

```shell
make
make load
```
## 配置

使用网络命名空间分别配置两个网卡，注意替换eth2/eth3为你的网卡名称

```shell
ip netns add eth2spaces
ip link set eth2 netns eth2spaces
ip netns exec eth2spaces ifconfig eth2 192.168.1.100 netmask 255.255.255.0 up

ip netns add eth3spaces
ip link set eth3 netns eth3spaces
ip netns exec eth3spaces ifconfig eth3 192.168.1.101 netmask 255.255.255.0 up
```

## 示例

以下为ping包测试示例

```shell
ip netns exec eth2spaces ifconfig
ip netns exec eth3spaces ifconfig
ip netns exec eth3spaces ping 192.168.1.100
ip netns exec eth2spaces ping 192.168.1.101
```
