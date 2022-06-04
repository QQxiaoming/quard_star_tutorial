/*
 * QEMU RISC-V Quard Star Board
 *
 * Copyright (c) 2021 qiao qiming
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HW_RISCV_QUARD_STAR__H
#define HW_RISCV_QUARD_STAR__H

#include "hw/cpu/cluster.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/sysbus.h"
#include "hw/block/flash.h"
#include "qom/object.h"
#include "hw/register.h"
#include "hw/i2c/i2c.h"
#include "hw/i2c/imx_i2c.h"
#include "hw/ssi/ssi.h"
#include "hw/ssi/sifive_spi.h"
#include "hw/usb/hcd-dwc3.h"
#include "hw/gpio/sifive_gpio.h"
#include "hw/dma/sifive_pdma.h"
#include "hw/sd/cadence_sdhci.h"

#define QUARD_STAR_MANAGEMENT_CPU_COUNT    1
#define QUARD_STAR_COMPUTE_CPU_COUNT       7

#define TYPE_RISCV_QUARD_STAR_MACHINE MACHINE_TYPE_NAME("quard-star")
typedef struct QuardStarState QuardStarState;
DECLARE_INSTANCE_CHECKER(QuardStarState, RISCV_VIRT_MACHINE,
                         TYPE_RISCV_QUARD_STAR_MACHINE)

struct QuardStarState {
    /*< private >*/
    MachineState parent;

    /*< public >*/
    CPUClusterState r_cluster;
    CPUClusterState c_cluster;
    RISCVHartArrayState r_cpus;
    RISCVHartArrayState c_cpus;
    DeviceState *plic;
    PFlashCFI01 *flash;
    IMXI2CState i2c[3];
    SiFiveSPIState spi[2];
    FWCfgState *fw_cfg;
    USBDWC3 usb;
    SIFIVEGPIOState gpio;
    SiFivePDMAState dma;
    CadenceSDHCIState sdhost;
    DeviceState *i2s;
    DeviceState *nand;
    
    I2CSlave *at24c_dev;
    I2CSlave *wm8750_dev;

    char *mask_rom_path;
};

enum {
    QUARD_STAR_MROM,
    QUARD_STAR_SRAM,
    QUARD_STAR_CLINT,
    QUARD_STAR_PLIC,
    QUARD_STAR_UART0,
    QUARD_STAR_UART1,
    QUARD_STAR_UART2,
    QUARD_STAR_RTC,
    QUARD_STAR_I2C0,
    QUARD_STAR_I2C1,
    QUARD_STAR_I2C2,
    QUARD_STAR_SPI0,
    QUARD_STAR_SPI1,
    QUARD_STAR_GPIO,
    QUARD_STAR_SDIO,
    QUARD_STAR_I2S,
    QUARD_STAR_NAND,
    QUARD_STAR_SYSCTL,
    QUARD_STAR_VIRTIO0,
    QUARD_STAR_VIRTIO1,
    QUARD_STAR_VIRTIO2,
    QUARD_STAR_VIRTIO3,
    QUARD_STAR_VIRTIO4,
    QUARD_STAR_VIRTIO5,
    QUARD_STAR_VIRTIO6,
    QUARD_STAR_VIRTIO7,
    QUARD_STAR_FW_CFG,
    QUARD_STAR_USB,
    QUARD_STAR_DMA,
    QUARD_STAR_FLASH,
    QUARD_STAR_DRAM,
};

enum {
    QUARD_STAR_VIRTIO0_IRQ = 1,
    QUARD_STAR_VIRTIO1_IRQ = 2,
    QUARD_STAR_VIRTIO2_IRQ = 3,
    QUARD_STAR_VIRTIO3_IRQ = 4,
    QUARD_STAR_VIRTIO4_IRQ = 5,
    QUARD_STAR_VIRTIO5_IRQ = 6,
    QUARD_STAR_VIRTIO6_IRQ = 7,
    QUARD_STAR_VIRTIO7_IRQ = 8,

    QUARD_STAR_UART0_IRQ = 10,
    QUARD_STAR_UART1_IRQ = 11,
    QUARD_STAR_UART2_IRQ = 12,
    QUARD_STAR_RTC_IRQ = 13,
    QUARD_STAR_I2C0_IRQ = 14,
    QUARD_STAR_I2C1_IRQ = 15,
    QUARD_STAR_I2C2_IRQ = 16,
    QUARD_STAR_SPI0_IRQ = 17,
    QUARD_STAR_SPI1_IRQ = 18,
    QUARD_STAR_USB_IRQ = 19,
    QUARD_STAR_SDIO_IRQ = 20,
    QUARD_STAR_I2S_IRQ = 21,
    QUARD_STAR_NAND_IRQ = 22,

    QUARD_STAR_GPIO_IRQ = 32, /* 32-47 */
    QUARD_STAR_DMA_IRQ = 48,  /* 48-55 */
};

#define QUARD_STAR_PLIC_NUM_SOURCES    127
#define QUARD_STAR_PLIC_NUM_PRIORITIES 7
#define QUARD_STAR_PLIC_PRIORITY_BASE  0x04
#define QUARD_STAR_PLIC_PENDING_BASE   0x1000
#define QUARD_STAR_PLIC_ENABLE_BASE    0x2000
#define QUARD_STAR_PLIC_ENABLE_STRIDE  0x80
#define QUARD_STAR_PLIC_CONTEXT_BASE   0x200000
#define QUARD_STAR_PLIC_CONTEXT_STRIDE 0x1000

#endif
