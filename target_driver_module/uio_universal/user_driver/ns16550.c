#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <poll.h>
#include <stdarg.h>

/*
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

    | Bits    | Field Name | Attr. | Rst. | Description                                      |
    | ------- | ---------- | ----- | ---- | ------------------------------------------------ |
    | [15:0]  | dl         | RW    | X    | Divisor Latch                                    |
    | [31:16] | Reserved   |       |      |                                                  |
*/

typedef union __attribute__((packed)) ns16550
{
	volatile uint32_t reg[3];
    struct {
        volatile struct
        {
            uint8_t data      : 8;  // [7:0]
            uint8_t erbfi     : 1;  // 8
            uint8_t etbei     : 1;  // 9
            uint8_t elsi      : 1;  // 10
            uint8_t edssi     : 1;  // 11
            uint8_t Reserved0 : 4;  // [15:12]
            union {
                struct {
                    uint8_t ip        : 1;  // 16 
                    uint8_t iid       : 3;  // [17:19]
                    uint8_t Reserved1 : 2;  // [21:20]
                    uint8_t fstate    : 2;  // [23:22]
                };
                struct {
                    uint8_t fe        : 1;  // 16
                    uint8_t iid_rfr   : 1;  // 17
                    uint8_t iid_xfr   : 1;  // 18
                    uint8_t iid_dmas  : 1;  // 19
                    uint8_t Reserved2 : 2;  // [21:20]
                    uint8_t rrt       : 2;  // [23:22]
                };
            };
            uint8_t wls       : 2;  // [25:24]
            uint8_t stb       : 1;  // 26
            uint8_t pen       : 1;  // 27
            uint8_t eps       : 1;  // 28
            uint8_t sp        : 1;  // 29
            uint8_t sb        : 1;  // 30
            uint8_t dlab      : 1;  // 31
        } reg0;  
        volatile struct 
        {
            uint8_t dtr       : 1;  // 0
            uint8_t rts       : 1;  // 1
            uint8_t out1      : 1;  // 2
            uint8_t out2      : 1;  // 3
            uint8_t loop      : 1;  // 4
            uint8_t Reserved0 : 3;  // [7:5]
            uint8_t dr        : 1;  // 8
            uint8_t oe        : 1;  // 9
            uint8_t pe        : 1;  // 10
            uint8_t fe        : 1;  // 11
            uint8_t bi        : 1;  // 12
            uint8_t thre      : 1;  // 13
            uint8_t temt      : 1;  // 14
            uint8_t fifoerr   : 1;  // 15
            uint8_t dcts      : 1;  // 16
            uint8_t ddsr      : 1;  // 17
            uint8_t teri      : 1;  // 18
            uint8_t ddcd      : 1;  // 19
            uint8_t cts       : 1;  // 20
            uint8_t dsr       : 1;  // 21
            uint8_t ri        : 1;  // 22
            uint8_t dcd       : 1;  // 23
            uint8_t Reserved1 : 1;  // 24
            uint8_t Reserved2 : 8;  // [32:25]
        }reg1;
        volatile struct 
        {
            uint16_t dl        : 16; // [15:0]
            uint16_t Reserved0 : 16; // [31:16]
        }reg2;
    };
}ns16550_t;

void ns16550_tx(uintptr_t addr, unsigned char c)
{
	ns16550_t *reg = (ns16550_t *)addr;

	while (reg->reg1.thre == 0) ;
    reg->reg0.data = c;
}

int ns16550_rx(uintptr_t addr, unsigned char *c, uint32_t timeout)
{
	ns16550_t *reg = (ns16550_t *)addr;
	
	while (reg->reg1.dr == 0){
		if(timeout != 0xFFFFFFFF) {
			if(timeout == 0) {
				return -1;
			}
			timeout--;
		}
	}
	if(c) {
		*c = reg->reg0.data;
	}
	return 0;
}

#define UART_LOG_BUFF_SIZE 1024
static uint8_t *NS16550_ADDR = NULL;
static char printk_string[UART_LOG_BUFF_SIZE] = {0};

static int _puts(char *str)
{
    int counter = 0;
    if (!str)
    {
        return 0;
    }
    while (*str && (counter < UART_LOG_BUFF_SIZE))
    {
        if(*str == '\n')
	        ns16550_tx((uintptr_t)NS16550_ADDR, '\r');
	    ns16550_tx((uintptr_t)NS16550_ADDR, *str++);
        counter++;
    }
    return counter;
}

int debug_log(char *fmt, ...)
{
    va_list args;
    int plen;
    va_start(args, fmt);
    plen = vsnprintf(printk_string, sizeof(printk_string)/sizeof(char) - 1, fmt, args);
    _puts(printk_string);
    va_end(args);

    return plen;
}

int read_uio_vsync(int fd_dev, int timeout_ms) {
    int ret;
    int32_t irq = 0;
    struct pollfd fds = {
        .fd = fd_dev,
        .events = POLLIN,
    };
    ret = poll(&fds, 1, timeout_ms);
    if (ret >= 1) {
        ret = read(fd_dev, &irq, sizeof(irq));
        if (ret == (ssize_t)sizeof(irq)) {
            return irq;
        }
    } 
    return -1;
}

int detect_dev(int dev_num, const char *name)
{
    int fd_sysfs,sysfs_len;
    char sysfs_buf[256];
    char sysfs_path[256];

    sprintf(sysfs_path, "/sys/class/uio/uio%d/name", dev_num);

    fd_sysfs = open(sysfs_path, O_RDONLY);
    if (fd_sysfs < 0) {
        printf("open sysfs failed\n");
        return -1;
    }
    sysfs_len = read(fd_sysfs, sysfs_buf, sizeof(sysfs_buf));
    if (sysfs_len < 0) {
        printf("read sysfs failed\n");
        close(fd_sysfs);
        return -1;
    }
    sysfs_buf[sysfs_len] = '\0';
    if (strncmp(sysfs_buf, "uio_universal", strlen("uio_universal")) != 0) {
        printf("device name not match\n");
        close(fd_sysfs);
        return -1;
    }
    close(fd_sysfs);

    sprintf(sysfs_path, "/sys/class/uio/uio%d/maps/map0/name", dev_num);

    fd_sysfs = open(sysfs_path, O_RDONLY);
    if (fd_sysfs < 0) {
        printf("open sysfs failed\n");
        return -1;
    }
    sysfs_len = read(fd_sysfs, sysfs_buf, sizeof(sysfs_buf));
    if (sysfs_len < 0) {
        printf("read sysfs failed\n");
        close(fd_sysfs);
        return -1;
    }
    sysfs_buf[sysfs_len] = '\0';
    if (strncmp(sysfs_buf, name, strlen(name)) != 0) {
        printf("device name not match\n");
        close(fd_sysfs);
        return -1;
    }
    close(fd_sysfs);

    return 0;
}

int get_dev_map_size(int dev_num, uint32_t *map_size) 
{
    char sysfs_path[256];
    sprintf(sysfs_path, "/sys/class/uio/uio%d/maps/map0/size", dev_num);

    int fd_sysfs = open(sysfs_path, O_RDONLY);
    if (fd_sysfs < 0) {
        printf("open sysfs failed\n");
        return -1;
    }
    char sysfs_buf[256];
    int sysfs_len = read(fd_sysfs, sysfs_buf, sizeof(sysfs_buf));
    if (sysfs_len < 0) {
        printf("read sysfs failed\n");
        return -1;
    }
    sysfs_buf[sysfs_len] = '\0';
    *map_size = strtoul(sysfs_buf, NULL, 0);
    close(fd_sysfs);

    return 0;
}

int open_dev(int dev_num, uint32_t map_size)
{
    char dev_path[256];
    sprintf(dev_path, "/dev/uio%d", dev_num);
    int fd_dev = open(dev_path, O_RDWR);
    if (fd_dev < 0) {
        printf("open device failed\n");
        return -1;
    }

    NS16550_ADDR = (uint8_t *)mmap(
        NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev, 0);
    if(NS16550_ADDR == MAP_FAILED) {
        printf("mmap failed\n");
        close(fd_dev);
        return -1;
    }

    return fd_dev;
}

void close_dev(int fd_dev, uint32_t map_size)
{
    munmap(NS16550_ADDR, map_size);
    close(fd_dev);
}

#define DEV_NUM  (0)
#define DEV_NAME "ns16550a"

int main(int argc, char **argv)
{
    int dev_num = DEV_NUM;
    const char *dev_name = DEV_NAME;
    uint32_t map_size = 0;
    int dev_fd = -1;

    for(int k = 1; k < argc; k++) {
        if(strcmp(argv[k], "-dev_num") == 0){
            dev_num = atoi(argv[++k]);
            printf("sbm_ch %d\n", dev_num);
        }
        if(strcmp(argv[k], "-dev_name") == 0){
            dev_name = argv[++k];
            printf("dev_name %s\n", dev_name);
        }
        if(strcmp(argv[k], "-h") == 0){
            printf("Usage: %s [-dev_num <dev_num>] [-dev_name <dev_name>]\n", argv[0]);
            return 0;
        }
    }

    if(detect_dev(dev_num, dev_name) < 0) {
        printf("detect device failed\n");
        return -1;
    }

    if(get_dev_map_size(dev_num, &map_size) < 0) {
        printf("get device map size failed\n");
        return -1;
    }

    if(dev_fd = open_dev(dev_num, map_size) < 0) {
        printf("open device failed\n");
        return -1;
    }

    debug_log("hello world\n");

    close_dev(dev_fd, map_size);

    return 0;
}
