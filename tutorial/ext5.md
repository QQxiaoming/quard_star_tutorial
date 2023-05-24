# QEMU CAN总线

在我的博客《[qemu常用参数选项说明](https://blog.csdn.net/weixin_39871788/article/details/123250595)》中我介绍的一些常用的qemu参数配置，而对于嵌入式开发往往还会涉及到更多形形色色的系统总线和硬件，本文来讲述下使用qemu can总线的用法。

## qemu参数配置

qemu支持can模拟的硬件不多，如果是官网下载的qemu我这里推荐使用xlnx zynqmp这个board，但是本文这里直接选择我的系列博客《[基于qemu-riscv从0开始构建嵌入式linux系统](https://blog.csdn.net/weixin_39871788/article/details/118469061)》中自制的quard-star板，目前开源在[github](https://github.com/QQxiaoming/quard_star_tutorial)和[gitee](https://gitee.com/QQxiaoming/quard_star_tutorial)上的代码均已支持CAN模拟。

qemu启动参数如下：

```
-M quard-star,canbus=canbus0
-object can-bus,id=canbus0 
-object can-host-socketcan,id=socketcan0,if=vcan0,canbus=canbus0
```

这里参数设置是将host主机上的can设备与qemu模拟的can设备相连接，如果我们host上没有can设备，则需要创建虚拟的vcan，此处vcan0即为我们需要创建的vcan。

## vcan0创建

创建config_vcan.sh脚本，内容如下：

```shell
#!/bin/bash
set -e

MODE=\
"config_vcan | \
release_vcan"

USER_NAME=$(whoami)
USAGE="usage $0 [$MODE] [<CAN_NAME>] "

if [ $# == 2 ] ; then
	CAN_NAME=$1
else
    CAN_NAME=vcan0
fi

config_vcan()
{
    modprobe vcan
    ip link add dev $CAN_NAME type vcan
    ip link set up $CAN_NAME
}

release_vcan()
{
    ip link set down $CAN_NAME
    ip link delete dev $CAN_NAME
}

case "$1" in
config_vcan)
    config_vcan
	;;
release_vcan)
    release_vcan
	;;
--help)
	echo $USAGE
	exit 0
	;;
*)
	echo $USAGE
	exit 1	
	;;
esac
```

执行sudo ./config_vcan.sh config_vcan就可以成功创建vcan0，如果你想删除这个vcan则只需sudo ./config_vcan.sh release_vcan。

这里稍微解释下，vcan是linux kernel内的一个driver，提供vcan的设备，因此只需要加载该驱动，就可以使用ip link相关命令来配置vcan设备，如同真实物理can设备一样。

完成vcan配置后，输入ifconfig，一切正常就会显示存在以下设备，此时我们就可以启动qemu了。

```shell
vcan0: flags=193<UP,RUNNING,NOARP>  mtu 72
        unspec 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  txqueuelen 1000  (未指定)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```

## qemu xlnx-can ip fix

原本此时我们配置好驱动就可以在qemu内的linux系统上使用can总线了，当然你可以裸机自己手写can驱动进行开发，但是我这里实测时发现直接使用linux kerenl的xlnx-can驱动，设备枚举一切正常，但数据会产生错误，经过debug分析最终确认我使用的qemu 7.0.0（截止发文前目前qemu上游mainline依然存在该问题）版本的xlnx-can ip模拟代码可能存在问题，因此这里需要手动修复（如果你使用quard-star则不需要，因为我已经修复它了）。这里描述问题如下：

在测试host上使用vcan与guest上的can通信，结果发现数据包字节序似乎不正确，因此对比kernel内的驱动代码以及qemu的代码发现xlnx的can格式和socket can格式是不匹配的，需要转换，这里阅读《ug1085-zynq-ultrascale-trm.pdf》的Table 20‐3: CAN Message Format就能发现这个问题，奇怪的是qemu-7.0.0/hw/net/can/xlnx-zynqmp-can.c内实现的似乎没有转换xlnx CAN Message Format而是把Socket CAN Format直接转发了，因此导致了错误。我这里添加或修改以下函数，问题得到解决（这里我没有向qemu上游发送patch，因为我不确定是否真实的zynq硬件情况）。

```c
#define XCAN_IDR_IDE_MASK		0x00080000U
#define XCAN_IDR_ID1_MASK		0xFFE00000U
#define XCAN_IDR_ID2_MASK		0x0007FFFEU
#define XCAN_IDR_RTR_MASK		0x00000001U
#define XCAN_IDR_SRR_MASK		0x00100000U
#define XCAN_IDR_ID1_SHIFT		21
#define XCAN_IDR_ID2_SHIFT		1
#define CAN_SFF_ID_BITS		    11
#define CAN_EFF_ID_BITS		    29

static uint32_t id_xcan2can(uint32_t id)
{
    uint32_t ret_id = 0; 
    /* Change Xilinx CAN ID format to socketCAN ID format */
    if (id & XCAN_IDR_IDE_MASK) {
        /* The received frame is an Extended format frame */
        ret_id = (id & XCAN_IDR_ID1_MASK) >> 3;
        ret_id |= (id & XCAN_IDR_ID2_MASK) >>
                XCAN_IDR_ID2_SHIFT;
        ret_id |= QEMU_CAN_EFF_FLAG;
        if (id & XCAN_IDR_RTR_MASK)
            ret_id |= QEMU_CAN_RTR_FLAG;
    } else {
        /* The received frame is a standard format frame */
        ret_id = (id & XCAN_IDR_ID1_MASK) >>
                XCAN_IDR_ID1_SHIFT;
        if (id & XCAN_IDR_SRR_MASK)
            ret_id |= QEMU_CAN_RTR_FLAG;
    }
    return ret_id;
}

static uint32_t id_can2xcan(uint32_t id)
{
    uint32_t ret_id = 0;
    if (id & QEMU_CAN_EFF_FLAG) {
        /* Extended CAN ID format */
        ret_id = ((id & QEMU_CAN_EFF_MASK) << XCAN_IDR_ID2_SHIFT) &
            XCAN_IDR_ID2_MASK;
        ret_id |= (((id & QEMU_CAN_EFF_MASK) >>
            (CAN_EFF_ID_BITS - CAN_SFF_ID_BITS)) <<
            XCAN_IDR_ID1_SHIFT) & XCAN_IDR_ID1_MASK;
        ret_id |= XCAN_IDR_IDE_MASK | XCAN_IDR_SRR_MASK;
        if (id & QEMU_CAN_RTR_FLAG)
            ret_id |= XCAN_IDR_RTR_MASK;
    } else {
        /* Standard CAN ID format */
        ret_id = ((id & QEMU_CAN_SFF_MASK) << XCAN_IDR_ID1_SHIFT) &
            XCAN_IDR_ID1_MASK;
        if (id & QEMU_CAN_RTR_FLAG)
            ret_id |= XCAN_IDR_SRR_MASK;
    }
    return ret_id;
}

static void generate_frame(qemu_can_frame *frame, uint32_t *data)
{
    frame->can_id = id_xcan2can(data[0]);
    frame->can_dlc = FIELD_EX32(data[1], TXFIFO_DLC, DLC);

    frame->data[0] = FIELD_EX32(data[2], TXFIFO_DATA1, DB0);
    frame->data[1] = FIELD_EX32(data[2], TXFIFO_DATA1, DB1);
    frame->data[2] = FIELD_EX32(data[2], TXFIFO_DATA1, DB2);
    frame->data[3] = FIELD_EX32(data[2], TXFIFO_DATA1, DB3);

    frame->data[4] = FIELD_EX32(data[3], TXFIFO_DATA2, DB4);
    frame->data[5] = FIELD_EX32(data[3], TXFIFO_DATA2, DB5);
    frame->data[6] = FIELD_EX32(data[3], TXFIFO_DATA2, DB6);
    frame->data[7] = FIELD_EX32(data[3], TXFIFO_DATA2, DB7);
}

static void update_rx_fifo(XlnxZynqMPCANState *s, const qemu_can_frame *frame)
{
    bool filter_pass = false;
    uint16_t timestamp = 0;

    /* If no filter is enabled. Message will be stored in FIFO. */
    if (!((ARRAY_FIELD_EX32(s->regs, AFR, UAF1)) |
    (ARRAY_FIELD_EX32(s->regs, AFR, UAF2)) |
    (ARRAY_FIELD_EX32(s->regs, AFR, UAF3)) |
    (ARRAY_FIELD_EX32(s->regs, AFR, UAF4)))) {
        filter_pass = true;
    }

    /*
    * Messages that pass any of the acceptance filters will be stored in
    * the RX FIFO.
    */
    if (ARRAY_FIELD_EX32(s->regs, AFR, UAF1)) {
        uint32_t id_masked = s->regs[R_AFMR1] & frame->can_id;
        uint32_t filter_id_masked = s->regs[R_AFMR1] & s->regs[R_AFIR1];

        if (filter_id_masked == id_masked) {
            filter_pass = true;
        }
    }

    if (ARRAY_FIELD_EX32(s->regs, AFR, UAF2)) {
        uint32_t id_masked = s->regs[R_AFMR2] & frame->can_id;
        uint32_t filter_id_masked = s->regs[R_AFMR2] & s->regs[R_AFIR2];

        if (filter_id_masked == id_masked) {
            filter_pass = true;
        }
    }

    if (ARRAY_FIELD_EX32(s->regs, AFR, UAF3)) {
        uint32_t id_masked = s->regs[R_AFMR3] & frame->can_id;
        uint32_t filter_id_masked = s->regs[R_AFMR3] & s->regs[R_AFIR3];

        if (filter_id_masked == id_masked) {
            filter_pass = true;
        }
    }

    if (ARRAY_FIELD_EX32(s->regs, AFR, UAF4)) {
        uint32_t id_masked = s->regs[R_AFMR4] & frame->can_id;
        uint32_t filter_id_masked = s->regs[R_AFMR4] & s->regs[R_AFIR4];

        if (filter_id_masked == id_masked) {
            filter_pass = true;
        }
    }

    if (!filter_pass) {
        trace_xlnx_can_rx_fifo_filter_reject(frame->can_id, frame->can_dlc);
        return;
    }

    /* Store the message in fifo if it passed through any of the filters. */
    if (filter_pass && frame->can_dlc <= MAX_DLC) {

        if (fifo32_is_full(&s->rx_fifo)) {
            ARRAY_FIELD_DP32(s->regs, INTERRUPT_STATUS_REGISTER, RXOFLW, 1);
        } else {
            timestamp = CAN_TIMER_MAX - ptimer_get_count(s->can_timer);

            fifo32_push(&s->rx_fifo, id_can2xcan(frame->can_id));

            fifo32_push(&s->rx_fifo, deposit32(0, R_RXFIFO_DLC_DLC_SHIFT,
                                            R_RXFIFO_DLC_DLC_LENGTH,
                                            frame->can_dlc) |
                                    deposit32(0, R_RXFIFO_DLC_RXT_SHIFT,
                                            R_RXFIFO_DLC_RXT_LENGTH,
                                            timestamp));

            /* First 32 bit of the data. */
            fifo32_push(&s->rx_fifo, deposit32(0, R_TXFIFO_DATA1_DB0_SHIFT,
                                            R_TXFIFO_DATA1_DB0_LENGTH,
                                            frame->data[0]) |
                                    deposit32(0, R_TXFIFO_DATA1_DB1_SHIFT,
                                            R_TXFIFO_DATA1_DB1_LENGTH,
                                            frame->data[1]) |
                                    deposit32(0, R_TXFIFO_DATA1_DB2_SHIFT,
                                            R_TXFIFO_DATA1_DB2_LENGTH,
                                            frame->data[2]) |
                                    deposit32(0, R_TXFIFO_DATA1_DB3_SHIFT,
                                            R_TXFIFO_DATA1_DB3_LENGTH,
                                            frame->data[3]));
            /* Last 32 bit of the data. */
            fifo32_push(&s->rx_fifo, deposit32(0, R_TXFIFO_DATA2_DB4_SHIFT,
                                            R_TXFIFO_DATA2_DB4_LENGTH,
                                            frame->data[4]) |
                                    deposit32(0, R_TXFIFO_DATA2_DB5_SHIFT,
                                            R_TXFIFO_DATA2_DB5_LENGTH,
                                            frame->data[5]) |
                                    deposit32(0, R_TXFIFO_DATA2_DB6_SHIFT,
                                            R_TXFIFO_DATA2_DB6_LENGTH,
                                            frame->data[6]) |
                                    deposit32(0, R_TXFIFO_DATA2_DB7_SHIFT,
                                            R_TXFIFO_DATA2_DB7_LENGTH,
                                            frame->data[7]));

            ARRAY_FIELD_DP32(s->regs, INTERRUPT_STATUS_REGISTER, RXOK, 1);
            trace_xlnx_can_rx_data(frame->can_id, frame->can_dlc,
                                frame->data[0], frame->data[1],
                                frame->data[2], frame->data[3],
                                frame->data[4], frame->data[5],
                                frame->data[6], frame->data[7]);
        }

        can_update_irq(s);
    }
}
```

## 运行qemu

配置即将在qemu内运行的系统设备树

```
can: can@1000c000 {
    compatible = "xlnx,zynq-can-1.0";
    status = "disabled";
    clock-names = "can_clk", "pclk";
    clocks = <&can_clk>, <&pclk>;
    reg = <0x0 0x1000c000 0x0 0x1000>;
    interrupt-parent = <&plic>;
    interrupts = <23>;
    tx-fifo-depth = <0x40>;
    rx-fifo-depth = <0x40>;
  };
```

进入系统can初始化成功，然后我们通过终端配置can设备（busybox的ip命令不支持can网络相关设置，需要使用iproute2来交叉编译完整版的ip，在我的项目的target_root_app/build.sh里交叉编译该软件包的脚本，target_root_app/iproute2-5.9.0.tar.gz也在项目内有提供）。

```shell
/sbin/ip link set can0 type can bitrate 1000000 sample-point 0.750
/sbin/ifconfig can0 up
```

在qemu内系统输入ifconfig，一切正常则会显示

```shell
can0      Link encap:UNSPEC  HWaddr 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  
          UP RUNNING NOARP  MTU:16  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:10 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)
          Interrupt:41 
```

现在让我们使用[can-utils](https://github.com/linux-can/can-utils)工具包进行调试，我们在host和guest上均安装该工具，guest上自己需要交叉编译,host可以用apt安装。

此时先在host执行

```shell
candump vcan0
```

然后在guest执行

```shell
cansend can0 1807EC0B#1122334455667788
cansend can0 5A1#11.22.33.44.55.66.77.88
```

查看host上输出

```shell
  vcan0  1807EC0B  [08]  11 22 33 44 55 66 77 88
  vcan0       5A1  [08]  11 22 33 44 55 66 77 88
```

交换host和guest上的命令，可以得到同样的结果，到这里can设备在qemu上的模拟就完成了，此时你可以使用标准的linux网络框架通过can总线在host和guest上交换数据了。当然嵌入式工程师也可以不使用linux kernel的驱动，通过裸机开发驱动发送数据到host上。
