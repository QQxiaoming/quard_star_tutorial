#include <FreeRTOS.h>
#include <task.h>
#include "debug_log.h"
#include "sbi.h"
#include "riscv_asm.h"

static void task1(void *p_arg)
{ 
    int time = 0;
    for(;;)
    {
        debug_log("task1 0x%x\n",time++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void task2(void *p_arg)
{ 
    int time = 0;
    for(;;)
    {
        debug_log("task2 0x%x\n",time++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void vTaskCreate(void *p_arg)
{ 
	debug_log("vTaskCreate\n");

    xTaskCreate(task1,"task1",2048,NULL,4,NULL);
    xTaskCreate(task2,"task2",2048,NULL,4,NULL);

    vTaskDelete(NULL);
}

int main(void)
{
	debug_log("Hello FreeRTOS!\n");
    
    debug_log_init();
    
    xTaskCreate(vTaskCreate,"task creat",256,NULL,4,NULL);

	vTaskStartScheduler();
	return 0;
}
