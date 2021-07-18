/*
 * FreeRTOS Kernel V10.4.3
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 * 1 tab == 4 spaces!
 */

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the RISC-V RV32 port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "riscv_asm.h"
#include "sbi.h"
#include "ns16550.h"

/* Standard includes. */
#include "string.h"

/* Let the user override the pre-loading of the initial LR with the address of
prvTaskExitError() in case it messes up unwinding of the stack in the
debugger. */
#ifdef configTASK_RETURN_ADDRESS
	#define portTASK_RETURN_ADDRESS	configTASK_RETURN_ADDRESS
#else
	#define portTASK_RETURN_ADDRESS	prvTaskExitError
#endif

/* The stack used by interrupt service routines.  Set configISR_STACK_SIZE_WORDS
to use a statically allocated array as the interrupt stack.  Alternative leave
configISR_STACK_SIZE_WORDS undefined and update the linker script so that a
linker variable names __freertos_irq_stack_top has the same value as the top
of the stack used by main.  Using the linker script method will repurpose the
stack that was used by main before the scheduler was started for use as the
interrupt stack after the scheduler has started. */
#ifdef configISR_STACK_SIZE_WORDS
	static __attribute__ ((aligned(16))) StackType_t xISRStack[ configISR_STACK_SIZE_WORDS ] = { 0 };
	const StackType_t xISRStackTop = ( StackType_t ) &( xISRStack[ configISR_STACK_SIZE_WORDS & ~portBYTE_ALIGNMENT_MASK ] );

	/* Don't use 0xa5 as the stack fill bytes as that is used by the kernerl for
	the task stacks, and so will legitimately appear in many positions within
	the ISR stack. */
	#define portISR_STACK_FILL_BYTE	0xee
#else
	extern const uint32_t __freertos_irq_stack_top[];
	const StackType_t xISRStackTop = ( StackType_t ) __freertos_irq_stack_top;
#endif

/*
 * Setup the timer to generate the tick interrupts.  The implementation in this
 * file is weak to allow application writers to change the timer used to
 * generate the tick interrupt.
 */
void vPortSetupTimerInterrupt( void ) __attribute__(( weak ));

/*-----------------------------------------------------------*/

/* Tick时间间隔 */
const size_t uxTimerIncrementsForOneTick = ( size_t ) ( ( configCPU_CLOCK_HZ ) / ( configTICK_RATE_HZ ) ); /* Assumes increment won't go over 32-bits. */

/* Set configCHECK_FOR_STACK_OVERFLOW to 3 to add ISR stack checking to task
stack checking.  A problem in the ISR stack will trigger an assert, not call the
stack overflow hook function (because the stack overflow hook is specific to a
task stack, not the ISR stack). */
#if defined( configISR_STACK_SIZE_WORDS ) && ( configCHECK_FOR_STACK_OVERFLOW > 2 )
	#warning This path not tested, or even compiled yet.

	static const uint8_t ucExpectedStackBytes[] = {
									portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,		\
									portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,		\
									portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,		\
									portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,		\
									portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE };	\

	#define portCHECK_ISR_STACK() configASSERT( ( memcmp( ( void * ) xISRStack, ( void * ) ucExpectedStackBytes, sizeof( ucExpectedStackBytes ) ) == 0 ) )
#else
	/* Define the function away. */
	#define portCHECK_ISR_STACK()
#endif /* configCHECK_FOR_STACK_OVERFLOW > 2 */

/*-----------------------------------------------------------*/

static uint64_t get_ticks()
{
    static volatile uint64_t time_elapsed = 0;
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

/*-----------------------------------------------------------*/

void vPortSetupTimerInterrupt( void )
{
	/* 通过sbi设置下次tick中断 */
	sbi_set_timer(get_ticks() + uxTimerIncrementsForOneTick);
}

/*-----------------------------------------------------------*/

void vPortClearIpiInterrupt( void )
{
	/* 通过sbi清除软中断，在软中断服务函数中调用 */
	extern void sbi_clear_ipi(void);
	sbi_clear_ipi();
}

/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
extern void xPortStartFirstTask( void );

	#if( configASSERT_DEFINED == 1 )
	{
		volatile uint32_t stvec = 0;

		/* Check the least significant two bits of mtvec are 00 - indicating
		single vector mode. */
		__asm volatile( "csrr %0, stvec" : "=r"( stvec ) );
		configASSERT( ( stvec & 0x03UL ) == 0 );

		/* Check alignment of the interrupt stack - which is the same as the
		stack that was being used by main() prior to the scheduler being
		started. */
		configASSERT( ( xISRStackTop & portBYTE_ALIGNMENT_MASK ) == 0 );

		#ifdef configISR_STACK_SIZE_WORDS
		{
			memset( ( void * ) xISRStack, portISR_STACK_FILL_BYTE, sizeof( xISRStack ) );
		}
		#endif	 /* configISR_STACK_SIZE_WORDS */
	}
	#endif /* configASSERT_DEFINED */

	/* 通过sbi设置Timer为滴答时钟 */
	vPortSetupTimerInterrupt();

    /* 使能SIE中S模式Timer中断和Soft中断，注意此处使能并不会立即响应
	xPortStartFirstTask中将打开全局使能 */
    csr_set(CSR_SIE, SIP_STIP);
    csr_set(CSR_SIE, SIP_SSIP);

	xPortStartFirstTask();

	/* Should not get here as after calling xPortStartFirstTask() only tasks
	should be executing. */
	return pdFAIL;
}

/*-----------------------------------------------------------*/

void prvTaskExitError( void )
{
	/* A function that implements a task must not exit or attempt to return to
	its caller as there is nothing to return to.  If a task wants to exit it
	should instead call vTaskDelete( NULL ).

	Artificially force an assert() to be triggered if configASSERT() is
	defined, then stop here so application writers can catch the error. */
	configASSERT( ulPortInterruptNesting == ~0UL );
	portDISABLE_INTERRUPTS();
	for( ;; );
}

/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* Not implemented. */
	for( ;; );
}





