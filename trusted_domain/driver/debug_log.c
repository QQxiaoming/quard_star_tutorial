#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "quard_star.h"
#include "ns16550.h"
#include "debug_log.h"

#define UART_LOG_BUFF_SIZE 1024

static char printk_string[UART_LOG_BUFF_SIZE] = {0};
static SemaphoreHandle_t xMutex = NULL;

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
	        vOutNS16550( NS16550_ADDR, '\r' );
	    vOutNS16550( NS16550_ADDR, *str++ );
        counter++;
    }
    return counter;
}

void debug_log_init(void)
{
    xMutex = xSemaphoreCreateMutex();
}

int debug_log(char *fmt, ...)
{
    if(xMutex)
        xSemaphoreTake(xMutex, portMAX_DELAY);

    va_list args;
    int plen;
    va_start(args, fmt);
    plen = vsnprintf(printk_string, sizeof(printk_string)/sizeof(char) - 1, fmt, args);
    _puts(printk_string);
    va_end(args);

    if(xMutex)
        xSemaphoreGive(xMutex);    

    return plen;
}


