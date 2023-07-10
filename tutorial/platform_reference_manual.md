# Platform Reference Manual

## Universal Asynchronous Receiver/Transmitter (UART)

### Overview

### Instances in SOC

| Instance Number | Address    |
| --------------- | ---------- |
| 0               | 0x10000000 |
| 1               | 0x10001000 |
| 2               | 0x10002000 |

### Registers

| Offset  | Name | Description |
| ------- | ---- | ----------- |
| 0x00	  | reg0 | Registers 0 |
| 0x04	  | reg1 | Registers 1 |
| 0x08	  | reg2 | Registers 2 |


#### reg0

| Bits    | Field Name | Attr. | Rst. | Description                                      |
| ------- | ---------- | ----- | ---- | ------------------------------------------------ |
| [7:0]   | data       | RW    | X    | Transmit/Receiver data                           |
| 8       | erbfi      | RW    | X    | Enable Received Data  Available Interrupt        |
| 9       | etbei      | RW    | X    | Enable Transmit Holding Register Empty Interrupt |
| 10      | elsi       | RW    | X    | Enable Receiver Line Status Interrupt            |
| 11      | edssi      | RW    | X    | Enable MODEM Status Interrupt                    |
| [15:12] | Reserved   |       |      |                                                  |
| 16      | ip         | RO    | X    | interrupt pending                                |
| 16      | fe         | WO    | X    | FIFO Enable                                      |
| [19:17] | iid        | RO    | X    | interrupt ID[2:0]                                |
| 17      | rfr        | WO    | X    | RCVR FIFO Reset                                  |
| 18      | xfr        | WO    | X    | XMIT FIFO Reset                                  |
| 19      | dmas       | WO    | X    | DMA Mode Select                                  |
| [21:20] | Reserved   |       |      |                                                  |
| [23:22] | fstate     | RO    | X    | FIFO State                                       |
| [23:22] | rrt        | WO    | X    | RCVR Trigger                                     |
| [25:24] | wls        | RW    | X    | Word Length Select                               |
| 26      | stb        | RW    | X    | Stop Bit Select                                  |
| 27      | pen        | RW    | X    | Parity Enable                                    |
| 28      | eps        | RW    | X    | Even Parity Select                               |
| 29      | sp         | RW    | X    | Stick Parity                                     |
| 30      | sb         | RW    | X    | Set Break                                        |
| 31      | dlab       | RW    | X    | Divisor Latch Access Bit                         |

#### reg1

| Bits    | Field Name | Attr. | Rst. | Description                                      |
| ------- | ---------- | ----- | ---- | ------------------------------------------------ |
| 0       | dtr        | RW    | X    | Data Terminal Ready                              |
| 1       | rts        | RW    | X    | Request To Send                                  |
| 2       | out1       | RW    | X    | Output1                                          |
| 3       | out2       | RW    | X    | Output2                                          |
| 4       | loop       | RW    | X    | Loopback                                         |
| [7:5]   | Reserved   |       |      |                                                  |
| 8       | dr         | RW    | X    | Data Ready                                       |
| 9       | oe         | RW    | X    | Overrun Error                                    |
| 10      | pe         | RW    | X    | Parity Error                                     |
| 11      | fe         | RW    | X    | Framing Error                                    |
| 12      | bi         | RW    | X    | Break Interrupt                                  |
| 13      | thre       | RW    | X    | Transmitter Holding Register                     |
| 14      | temt       | RW    | X    | Transmitter Empty                                |
| 15      | fifoerr    | RW    | X    | FIFO Error                                       |
| 16      | dcts       | RW    | X    | Delta Clear To Send                              |
| 17      | ddsr       | RW    | X    | Delta Data Set Ready                             |
| 18      | teri       | RW    | X    | Trailing Edge Ring Indicator                     |
| 19      | ddcd       | RW    | X    | Delta Data Carrier Detect                        |
| 20      | cts        | RW    | X    | Clear To Send                                    |
| 21      | dsr        | RW    | X    | Data Set Ready                                   |
| 22      | ri         | RW    | X    | Ring Indicator                                   |
| 23      | dcd        | RW    | X    | Data Carrier Detect                              |
| [32:24] | scr        | RW    | X    | Scratch                                          |

#### reg2

| Bits    | Field Name | Attr. | Rst. | Description                                      |
| ------- | ---------- | ----- | ---- | ------------------------------------------------ |
| [15:0]  | dl         | RW    | X    | Divisor Latch                                    |
| [31:16] | Reserved   |       |      |                                                  |

## Pulse Width Modulator (PWM)

### Overview

### Instances in SOC

| Instance Number | Address    | ncmp | cmpwidth |
| --------------- | ---------- | ---- | -------- |
| 0               | 0x1000d000 | 4    | 16       |

### Registers

| Offset | Name     | Description                |
| ------ | -------- | -------------------------- |
| 0x00   | pwmcfg   | PWM configuration register |
| 0x04   | Reserved |                            |
| 0x08   | pwmcount | PWM count register         |
| 0x0c   | Reserved |                            |
| 0x10   | pwms     | Scaled PWM count register  |
| 0x14   | Reserved |                            |
| 0x18   | Reserved |                            |
| 0x1c   | Reserved |                            |
| 0x20   | pwmcmp0  | PWM compare register 0     |
| 0x24   | pwmcmp1  | PWM compare register 1     |
| 0x28   | pwmcmp2  | PWM compare register 2     |
| 0x2c   | pwmcmp3  | PWM compare register 3     |


#### pwmcfg

| Bits    | Field Name    | Attr. | Rst. | Description                                       |
| ------- | ------------- | ----- | ---- | ------------------------------------------------- |
| [3:0]   | pwmscale      | RW    | X    | PWM Counter scale                                 |
| [7:4]   | Reserved      |       |      |                                                   |
| 8       | pwmsticky     | RW    | X    | PWM Sticky - disallow clearing pwmcmpXip bits     |
| 9       | pwmzerocmp    | RW    | X    | PWM Zero - counter resets to zero after match     | 
| 10      | pwmdeglitch   | RW    | X    | PWM Deglitch -  latch pwmcmp ip within same cycle |
| 11      | Reserved      |       |      |                                                   |
| 12      | pwmenalways   | RW    | 0x0  | PWM enable always - run continuously              |
| 13      | pwmenoneshot  | RW    | 0x0  | PWM enable one shot - run one cycle               |
| [15:14] | Reserved      |       |      |                                                   |
| 16      | pwmcmp0center | RW    | X    | PWM0 Compare Center                               |
| 17      | pwmcmp1center | RW    | X    | PWM1 Compare Center                               |
| 18      | pwmcmp2center | RW    | X    | PWM2 Compare Center                               |
| 19      | pwmcmp3center | RW    | X    | PWM3 Compare Center                               |
| [23:20] | Reserved      |       |      |                                                   |
| 24      | pwmcmp0gang   | RW    | X    | PWM0/PWM1 Compare Gang                            |
| 25      | pwmcmp1gang   | RW    | X    | PWM1/PWM2 Compare Gang                            |
| 26      | pwmcmp2gang   | RW    | X    | PWM2/PWM3 Compare Gang                            |
| 27      | pwmcmp3gang   | RW    | X    | PWM3/PWM0 Compare Gang                            |
| 28      | pwmcmp0ip     | RW    | X    | PWM0 Interrupt Pending                            |
| 29      | pwmcmp1ip     | RW    | X    | PWM1 Interrupt Pending                            |
| 30      | pwmcmp2ip     | RW    | X    | PWM2 Interrupt Pending                            |
| 31      | pwmcmp3ip     | RW    | X    | PWM3 Interrupt Pending                            |

#### pwmcount

| Bits   | Field Name | Attr. | Rst. | Description                                  |
| ------ | ---------- | ----- | ---- | -------------------------------------------- |
| [30:0] | pwmcount   | RW    | X    | PWM count register. cmpwidth + 15 bits wide. |
| [31]   | Reserved   |       |      |                                              |

#### pwms

| Bits   | Field Name | Attr. | Rst. | Description                                    |
| ------ | ---------- | ----- | ---- | ---------------------------------------------- |
| [15:0] | pwms       | RW    | X    | Scaled PWM count register. cmpwidth bits wide. |
| [31:16]| Reserved   |       |      |                                                |

#### pwmcmpX (x = 0-3)

| Bits   | Field Name | Attr. | Rst. | Description         |
| ------ | ---------- | ----- | ---- | ------------------- |
| [15:0] | pwmcmpX    | RW    | X    | PWM X compare Value |
| [31:16]| Reserved   |       |      |                     |

## Serial Peripheral Interface (SPI)

### Overview

### Instances in SOC

| Instance Number | Flash Controller | Address    | cs_width | div_width |
| --------------- | ---------------- | ---------- | -------- | --------- |
| 0               | Y                | 0x10007000 | 1        | 16        |

### Registers

| Offset | Name     | Description                 |
| ------ | -------- | --------------------------- |
| 0x00   | sckdiv   | Serial clock divisor        |
| 0x04   | sckmode  | Serial clock mode           |
| 0x08   | Reserved |                             |
| 0x0C   | Reserved |                             |
| 0x10   | csid     | Chip select ID              |
| 0x14   | csdef    | Chip select default         |
| 0x18   | csmode   | Chip select mode            |
| 0x1C   | Reserved |                             |
| 0x20   | Reserved |                             |
| 0x24   | Reserved |                             |
| 0x28   | delay0   | Delay control 0             |
| 0x2C   | delay1   | Delay control 1             |
| 0x30   | Reserved |                             |
| 0x34   | Reserved |                             |
| 0x38   | Reserved |                             |
| 0x3C   | Reserved |                             |
| 0x40   | fmt      | Frame format                |
| 0x44   | Reserved |                             |
| 0x48   | txdata   | Tx FIFO Data                |
| 0x4C   | rxdata   | Rx FIFO data                |
| 0x50   | txmark   | Tx FIFO watermark           |
| 0x54   | rxmark   | Rx FIFO watermark           |
| 0x58   | Reserved |                             |
| 0x5C   | Reserved |                             |
| 0x60   | fctrl    | SPI flash interface control |
| 0x64   | ffmt     | SPI flash instruction format|
| 0x68   | Reserved |                             |
| 0x6C   | Reserved |                             |
| 0x70   | ie       | SPI interrupt enable        |
| 0x74   | ip       | SPI interrupt pending       |

#### sckdiv

| Bits   | Field Name | Attr. | Rst. | Description                                    |
| ------ | ---------- | ----- | ---- | ---------------------------------------------- |
| [11:0] | div        | RW    | 0x3  | Divisor for serial clock. div_width bits wide. |
| [31:12]| Reserved   |       |      |                                                |
