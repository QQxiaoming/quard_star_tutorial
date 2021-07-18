#include <FreeRTOS.h>
#include <task.h>
#include "debug_log.h"
#include "sbi.h"
#include "riscv_asm.h"

void task1(void *p_arg)
{ 
    int time = 0;
    for(;;)
    {
        _printf("task1 0x%x\n",time++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task2(void *p_arg)
{ 
    int time = 0;
    for(;;)
    {
        _printf("task2 0x%x\n",time++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void vTaskCreate (void *p_arg)
{ 
	_printf( "vTaskCreate\n");

    xTaskCreate(task1,"task1",2048,NULL,4,NULL);
    xTaskCreate(task2,"task2",2048,NULL,4,NULL);

    vTaskDelete(NULL);
}

int main( void )
{
	_printf( "Hello FreeRTOS!\n");
    
    xTaskCreate(vTaskCreate,"task creat",256,NULL,4,NULL);

	vTaskStartScheduler();
	return 0;
}


