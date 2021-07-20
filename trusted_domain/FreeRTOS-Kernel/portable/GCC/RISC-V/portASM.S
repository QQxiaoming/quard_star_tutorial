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

/*
 * The FreeRTOS kernel's RISC-V port is split between the the code that is
 * common across all currently supported RISC-V chips (implementations of the
 * RISC-V ISA), and code which tailors the port to a specific RISC-V chip:
 *
 * + The code that is common to all RISC-V chips is implemented in
 *   FreeRTOS\Source\portable\GCC\RISC-V-RV32\portASM.S.  There is only one
 *   portASM.S file because the same file is used no matter which RISC-V chip is
 *   in use.
 *
 * + The code that tailors the kernel's RISC-V port to a specific RISC-V
 *   chip is implemented in freertos_risc_v_chip_specific_extensions.h.  There
 *   is one freertos_risc_v_chip_specific_extensions.h that can be used with any
 *   RISC-V chip that both includes a standard CLINT and does not add to the
 *   base set of RISC-V registers.  There are additional
 *   freertos_risc_v_chip_specific_extensions.h files for RISC-V implementations
 *   that do not include a standard CLINT or do add to the base set of RISC-V
 *   registers.
 *
 * CARE MUST BE TAKEN TO INCLDUE THE CORRECT
 * freertos_risc_v_chip_specific_extensions.h HEADER FILE FOR THE CHIP
 * IN USE.  To include the correct freertos_risc_v_chip_specific_extensions.h
 * header file ensure the path to the correct header file is in the assembler's
 * include path.
 *
 * This freertos_risc_v_chip_specific_extensions.h is for use on RISC-V chips
 * that include a standard CLINT and do not add to the base set of RISC-V
 * registers.
 *
 */
#include "riscv_encoding.h"

#if __riscv_xlen == 64
	#define portWORD_SIZE 8
	#define store_x sd
	#define load_x ld
#elif __riscv_xlen == 32
	#define portWORD_SIZE 4
	#define store_x sw
	#define load_x lw
#else
	#error Assembler did not define __riscv_xlen
#endif

/* Only the standard core registers are stored by default. */
#if __riscv_flen == 64
	#define portCONTEXT_SIZE ( 62 * portWORD_SIZE )
	#define store_fx fsd
	#define load_fx fld
#elif __riscv_xlen == 32
	#define portCONTEXT_SIZE ( 62 * portWORD_SIZE )
	#define store_fx fsw
	#define load_fx flw
#else
	#define portCONTEXT_SIZE ( 30 * portWORD_SIZE )
#endif

.global xPortStartFirstTask
.global freertos_risc_v_trap_handler
.global pxPortInitialiseStack
.extern pxCurrentTCB
.extern ulPortTrapHandler
.extern vTaskSwitchContext
.extern xTaskIncrementTick
.extern Timer_IRQHandler
.extern pullMachineTimerCompareRegister
.extern pullNextTime
.extern uxTimerIncrementsForOneTick /* size_t type so 32-bit on 32-bit core and 64-bits on 64-bit core. */
.extern xISRStackTop
.extern prvTaskExitError

/*-----------------------------------------------------------*/

.align 8
.func
freertos_risc_v_trap_handler:
	addi sp, sp, -portCONTEXT_SIZE
	store_x x1, 1 * portWORD_SIZE( sp )
	store_x x5, 2 * portWORD_SIZE( sp )
	store_x x6, 3 * portWORD_SIZE( sp )
	store_x x7, 4 * portWORD_SIZE( sp )
	store_x x8, 5 * portWORD_SIZE( sp )
	store_x x9, 6 * portWORD_SIZE( sp )
	store_x x10, 7 * portWORD_SIZE( sp )
	store_x x11, 8 * portWORD_SIZE( sp )
	store_x x12, 9 * portWORD_SIZE( sp )
	store_x x13, 10 * portWORD_SIZE( sp )
	store_x x14, 11 * portWORD_SIZE( sp )
	store_x x15, 12 * portWORD_SIZE( sp )
	store_x x16, 13 * portWORD_SIZE( sp )
	store_x x17, 14 * portWORD_SIZE( sp )
	store_x x18, 15 * portWORD_SIZE( sp )
	store_x x19, 16 * portWORD_SIZE( sp )
	store_x x20, 17 * portWORD_SIZE( sp )
	store_x x21, 18 * portWORD_SIZE( sp )
	store_x x22, 19 * portWORD_SIZE( sp )
	store_x x23, 20 * portWORD_SIZE( sp )
	store_x x24, 21 * portWORD_SIZE( sp )
	store_x x25, 22 * portWORD_SIZE( sp )
	store_x x26, 23 * portWORD_SIZE( sp )
	store_x x27, 24 * portWORD_SIZE( sp )
	store_x x28, 25 * portWORD_SIZE( sp )
	store_x x29, 26 * portWORD_SIZE( sp )
	store_x x30, 27 * portWORD_SIZE( sp )
	store_x x31, 28 * portWORD_SIZE( sp )

	csrr t0, sstatus					/* Required for SPIE bit. */
	store_x t0, 29 * portWORD_SIZE( sp )

#ifdef __riscv_flen
	csrr t2, sstatus
	li t0, SSTATUS_FS
	and t1,t2,t0
	bne t1,t0,1f

	store_fx f0, 30 * portWORD_SIZE( sp )
	store_fx f1, 31 * portWORD_SIZE( sp )
	store_fx f2, 32 * portWORD_SIZE( sp )
	store_fx f3, 33 * portWORD_SIZE( sp )
	store_fx f4, 34 * portWORD_SIZE( sp )
	store_fx f5, 35 * portWORD_SIZE( sp )
	store_fx f6, 36 * portWORD_SIZE( sp )
	store_fx f7, 37 * portWORD_SIZE( sp )
	store_fx f8, 38 * portWORD_SIZE( sp )
	store_fx f9, 39 * portWORD_SIZE( sp )
	store_fx f10, 40 * portWORD_SIZE( sp )
	store_fx f11, 41 * portWORD_SIZE( sp )
	store_fx f12, 42 * portWORD_SIZE( sp )
	store_fx f13, 43 * portWORD_SIZE( sp )
	store_fx f14, 44 * portWORD_SIZE( sp )
	store_fx f15, 45 * portWORD_SIZE( sp )
	store_fx f16, 46 * portWORD_SIZE( sp )
	store_fx f17, 47 * portWORD_SIZE( sp )
	store_fx f18, 48 * portWORD_SIZE( sp )
	store_fx f19, 49 * portWORD_SIZE( sp )
	store_fx f20, 50 * portWORD_SIZE( sp )
	store_fx f21, 51 * portWORD_SIZE( sp )
	store_fx f22, 52 * portWORD_SIZE( sp )
	store_fx f23, 53 * portWORD_SIZE( sp )
	store_fx f24, 54 * portWORD_SIZE( sp )
	store_fx f25, 55 * portWORD_SIZE( sp )
	store_fx f26, 56 * portWORD_SIZE( sp )
	store_fx f27, 57 * portWORD_SIZE( sp )
	store_fx f28, 58 * portWORD_SIZE( sp )
	store_fx f29, 59 * portWORD_SIZE( sp )
	store_fx f30, 60 * portWORD_SIZE( sp )
	store_fx f31, 61 * portWORD_SIZE( sp )
1:	
	li t0, (0x1 << 14)
	csrs sstatus,t0
	li t0, (0x1 << 13)
	csrc sstatus,t0   
#endif

	csrr t0, sepc
	store_x t0, 0 * portWORD_SIZE( sp )

	load_x  t0, pxCurrentTCB			/* Load pxCurrentTCB. */
	store_x  sp, 0( t0 )				/* Write sp to first TCB member. */

	csrr a0, scause
	csrr a1, sepc

test_if_asynchronous:
	srli a2, a0, __riscv_xlen - 1		/* MSB of mcause is 1 if handing an asynchronous interrupt - shift to LSB to clear other bits. */
	beq a2, x0, handle_synchronous		/* Branch past interrupt handing if not asynchronous. */
	store_x a1, 0( sp )					/* Asynch so save unmodified exception return address. */

handle_asynchronous:

test_if_ipi:
	addi t0, x0, 1

	slli t0, t0, __riscv_xlen - 1   /* LSB is already set, shift into MSB.  Shift 31 on 32-bit or 63 on 64-bit cores. */
	addi t1, t0, 1					/* 0x8000[]0001 == Supervisor ipi interrupt. */
	bne a0, t1, test_if_mtimer

	load_x sp, xISRStackTop			/* Switch to ISR stack before function call. */
	jal vPortClearIpiInterrupt
	jal vTaskSwitchContext
	j processed_source

test_if_mtimer:						/* If there is a CLINT then the mtimer is used to generate the tick interrupt. */
	addi t1, t1, 4					/* 0x80000001 + 4 = 0x80000005 == Supervisor timer interrupt. */
	bne a0, t1, test_if_external_interrupt

	load_x sp, xISRStackTop			/* Switch to ISR stack before function call. */
	jal vPortSetupTimerInterrupt
	jal xTaskIncrementTick
	beqz a0, processed_source		/* Don't switch context if incrementing tick didn't unblock a task. */
	jal vTaskSwitchContext
	j processed_source

test_if_external_interrupt:			/* If there is a CLINT and the mtimer interrupt is not pending then check to see if an external interrupt is pending. */
	addi t1, t1, 4					/* 0x80000005 + 4 = 0x80000009 == Supervisor external interrupt. */
	bne a0, t1, processed_trap   	/* Something as yet unhandled. */

	load_x sp, xISRStackTop			/* Switch to ISR stack before function call. */
	jal handle_interrupt	        /* Jump to the interrupt handler if there is no CLINT or if there is a CLINT and it has been determined that an external interrupt is pending. */
	j processed_source

handle_synchronous:
	addi a1, a1, 4					/* Synchronous so updated exception return address to the instruction after the instruction that generated the exeption. */
	store_x a1, 0( sp )				/* Save updated exception return address. */

processed_trap:
	csrr a0, scause
	csrr a1, sepc
	csrr a2, stval
	mv   a4, sp
	load_x sp, xISRStackTop			/* Switch to ISR stack before function call. */
	jal handle_trap	                /* Jump to the interrupt handler if there is no CLINT or if there is a CLINT and it has been determined that an external interrupt is pending. */
	j processed_source

processed_source:
	load_x  t1, pxCurrentTCB		/* Load pxCurrentTCB. */
	load_x  sp, 0( t1 )				/* Read sp from first TCB member. */

	/* Load sret with the address of the next instruction in the task to run next. */
	load_x t0, 0 * portWORD_SIZE( sp )
	csrw sepc, t0

#ifdef __riscv_flen
	csrr t2, sstatus
#endif

	/* Load mstatus with the interrupt enable bits used by the task. */
	load_x  t0, 29 * portWORD_SIZE( sp )
	csrw sstatus, t0				/* Required for MPIE bit. */

#ifdef __riscv_flen
	li t0, SSTATUS_FS
	and t1,t2,t0
	bne t1,t0,1f

	load_fx f0, 30 * portWORD_SIZE( sp )
	load_fx f1, 31 * portWORD_SIZE( sp )
	load_fx f2, 32 * portWORD_SIZE( sp )
	load_fx f3, 33 * portWORD_SIZE( sp )
	load_fx f4, 34 * portWORD_SIZE( sp )
	load_fx f5, 35 * portWORD_SIZE( sp )
	load_fx f6, 36 * portWORD_SIZE( sp )
	load_fx f7, 37 * portWORD_SIZE( sp )
	load_fx f8, 38 * portWORD_SIZE( sp )
	load_fx f9, 39 * portWORD_SIZE( sp )
	load_fx f10, 40 * portWORD_SIZE( sp )
	load_fx f11, 41 * portWORD_SIZE( sp )
	load_fx f12, 42 * portWORD_SIZE( sp )
	load_fx f13, 43 * portWORD_SIZE( sp )
	load_fx f14, 44 * portWORD_SIZE( sp )
	load_fx f15, 45 * portWORD_SIZE( sp )
	load_fx f16, 46 * portWORD_SIZE( sp )
	load_fx f17, 47 * portWORD_SIZE( sp )
	load_fx f18, 48 * portWORD_SIZE( sp )
	load_fx f19, 49 * portWORD_SIZE( sp )
	load_fx f20, 50 * portWORD_SIZE( sp )
	load_fx f21, 51 * portWORD_SIZE( sp )
	load_fx f22, 52 * portWORD_SIZE( sp )
	load_fx f23, 53 * portWORD_SIZE( sp )
	load_fx f24, 54 * portWORD_SIZE( sp )
	load_fx f25, 55 * portWORD_SIZE( sp )
	load_fx f26, 56 * portWORD_SIZE( sp )
	load_fx f27, 57 * portWORD_SIZE( sp )
	load_fx f28, 58 * portWORD_SIZE( sp )
	load_fx f29, 59 * portWORD_SIZE( sp )
	load_fx f30, 60 * portWORD_SIZE( sp )
	load_fx f31, 61 * portWORD_SIZE( sp )
1:	
#endif

	load_x  x1, 1 * portWORD_SIZE( sp )
	load_x  x5, 2 * portWORD_SIZE( sp )		/* t0 */
	load_x  x6, 3 * portWORD_SIZE( sp )		/* t1 */
	load_x  x7, 4 * portWORD_SIZE( sp )		/* t2 */
	load_x  x8, 5 * portWORD_SIZE( sp )		/* s0/fp */
	load_x  x9, 6 * portWORD_SIZE( sp )		/* s1 */
	load_x  x10, 7 * portWORD_SIZE( sp )	/* a0 */
	load_x  x11, 8 * portWORD_SIZE( sp )	/* a1 */
	load_x  x12, 9 * portWORD_SIZE( sp )	/* a2 */
	load_x  x13, 10 * portWORD_SIZE( sp )	/* a3 */
	load_x  x14, 11 * portWORD_SIZE( sp )	/* a4 */
	load_x  x15, 12 * portWORD_SIZE( sp )	/* a5 */
	load_x  x16, 13 * portWORD_SIZE( sp )	/* a6 */
	load_x  x17, 14 * portWORD_SIZE( sp )	/* a7 */
	load_x  x18, 15 * portWORD_SIZE( sp )	/* s2 */
	load_x  x19, 16 * portWORD_SIZE( sp )	/* s3 */
	load_x  x20, 17 * portWORD_SIZE( sp )	/* s4 */
	load_x  x21, 18 * portWORD_SIZE( sp )	/* s5 */
	load_x  x22, 19 * portWORD_SIZE( sp )	/* s6 */
	load_x  x23, 20 * portWORD_SIZE( sp )	/* s7 */
	load_x  x24, 21 * portWORD_SIZE( sp )	/* s8 */
	load_x  x25, 22 * portWORD_SIZE( sp )	/* s9 */
	load_x  x26, 23 * portWORD_SIZE( sp )	/* s10 */
	load_x  x27, 24 * portWORD_SIZE( sp )	/* s11 */
	load_x  x28, 25 * portWORD_SIZE( sp )	/* t3 */
	load_x  x29, 26 * portWORD_SIZE( sp )	/* t4 */
	load_x  x30, 27 * portWORD_SIZE( sp )	/* t5 */
	load_x  x31, 28 * portWORD_SIZE( sp )	/* t6 */
	addi sp, sp, portCONTEXT_SIZE

	sret
	.endfunc
/*-----------------------------------------------------------*/

.align 8
.weak handle_trap
.func
handle_trap:
	j handle_trap
	.endfunc
/*-----------------------------------------------------------*/

.align 8
.weak handle_interrupt
.func
handle_interrupt:
	j handle_interrupt
	ret
	.endfunc
/*-----------------------------------------------------------*/

.align 8
.func
xPortStartFirstTask:
	/* If there is a clint then interrupts can branch directly to the FreeRTOS
	trap handler.  Otherwise the interrupt controller will need to be configured
	outside of this file. */
	la t0, freertos_risc_v_trap_handler
	csrw stvec, t0

	load_x  sp, pxCurrentTCB			/* Load pxCurrentTCB. */
	load_x  sp, 0( sp )				 	/* Read sp from first TCB member. */

	load_x  x1, 0( sp ) /* Note for starting the scheduler the exception return address is used as the function return address. */

#ifdef __riscv_flen
	load_fx f0, 30 * portWORD_SIZE( sp )
	load_fx f1, 31 * portWORD_SIZE( sp )
	load_fx f2, 32 * portWORD_SIZE( sp )
	load_fx f3, 33 * portWORD_SIZE( sp )
	load_fx f4, 34 * portWORD_SIZE( sp )
	load_fx f5, 35 * portWORD_SIZE( sp )
	load_fx f6, 36 * portWORD_SIZE( sp )
	load_fx f7, 37 * portWORD_SIZE( sp )
	load_fx f8, 38 * portWORD_SIZE( sp )
	load_fx f9, 39 * portWORD_SIZE( sp )
	load_fx f10, 40 * portWORD_SIZE( sp )
	load_fx f11, 41 * portWORD_SIZE( sp )
	load_fx f12, 42 * portWORD_SIZE( sp )
	load_fx f13, 43 * portWORD_SIZE( sp )
	load_fx f14, 44 * portWORD_SIZE( sp )
	load_fx f15, 45 * portWORD_SIZE( sp )
	load_fx f16, 46 * portWORD_SIZE( sp )
	load_fx f17, 47 * portWORD_SIZE( sp )
	load_fx f18, 48 * portWORD_SIZE( sp )
	load_fx f19, 49 * portWORD_SIZE( sp )
	load_fx f20, 50 * portWORD_SIZE( sp )
	load_fx f21, 51 * portWORD_SIZE( sp )
	load_fx f22, 52 * portWORD_SIZE( sp )
	load_fx f23, 53 * portWORD_SIZE( sp )
	load_fx f24, 54 * portWORD_SIZE( sp )
	load_fx f25, 55 * portWORD_SIZE( sp )
	load_fx f26, 56 * portWORD_SIZE( sp )
	load_fx f27, 57 * portWORD_SIZE( sp )
	load_fx f28, 58 * portWORD_SIZE( sp )
	load_fx f29, 59 * portWORD_SIZE( sp )
	load_fx f30, 60 * portWORD_SIZE( sp )
	load_fx f31, 61 * portWORD_SIZE( sp )
#endif

	load_x  x6, 3 * portWORD_SIZE( sp )		/* t1 */
	load_x  x7, 4 * portWORD_SIZE( sp )		/* t2 */
	load_x  x8, 5 * portWORD_SIZE( sp )		/* s0/fp */
	load_x  x9, 6 * portWORD_SIZE( sp )		/* s1 */
	load_x  x10, 7 * portWORD_SIZE( sp )	/* a0 */
	load_x  x11, 8 * portWORD_SIZE( sp )	/* a1 */
	load_x  x12, 9 * portWORD_SIZE( sp )	/* a2 */
	load_x  x13, 10 * portWORD_SIZE( sp )	/* a3 */
	load_x  x14, 11 * portWORD_SIZE( sp )	/* a4 */
	load_x  x15, 12 * portWORD_SIZE( sp )	/* a5 */
	load_x  x16, 13 * portWORD_SIZE( sp )	/* a6 */
	load_x  x17, 14 * portWORD_SIZE( sp )	/* a7 */
	load_x  x18, 15 * portWORD_SIZE( sp )	/* s2 */
	load_x  x19, 16 * portWORD_SIZE( sp )	/* s3 */
	load_x  x20, 17 * portWORD_SIZE( sp )	/* s4 */
	load_x  x21, 18 * portWORD_SIZE( sp )	/* s5 */
	load_x  x22, 19 * portWORD_SIZE( sp )	/* s6 */
	load_x  x23, 20 * portWORD_SIZE( sp )	/* s7 */
	load_x  x24, 21 * portWORD_SIZE( sp )	/* s8 */
	load_x  x25, 22 * portWORD_SIZE( sp )	/* s9 */
	load_x  x26, 23 * portWORD_SIZE( sp )	/* s10 */
	load_x  x27, 24 * portWORD_SIZE( sp )	/* s11 */
	load_x  x28, 25 * portWORD_SIZE( sp )	/* t3 */
	load_x  x29, 26 * portWORD_SIZE( sp )	/* t4 */
	load_x  x30, 27 * portWORD_SIZE( sp )	/* t5 */
	load_x  x31, 28 * portWORD_SIZE( sp )	/* t6 */

	load_x  x5, 29 * portWORD_SIZE( sp )	/* Initial mstatus into x5 (t0) */
	ori x5, x5, 0x2					    	/* Set SIE bit so the first task starts with interrupts enabled - required as returns with ret not eret. */
	csrrw  x0, sstatus, x5					/* Interrupts enabled from here! */
	load_x  x5, 2 * portWORD_SIZE( sp )		/* Initial x5 (t0) value. */

	addi	sp, sp, portCONTEXT_SIZE
	ret
	.endfunc
/*-----------------------------------------------------------*/

/*
 * Unlike other ports pxPortInitialiseStack() is written in assembly code as it
 * needs access to the portasmADDITIONAL_CONTEXT_SIZE constant.  The prototype
 * for the function is as per the other ports:
 * StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters );
 *
 * As per the standard RISC-V ABI pxTopcOfStack is passed in in a0, pxCode in
 * a1, and pvParameters in a2.  The new top of stack is passed out in a0.
 *
 * RISC-V maps registers to ABI names as follows (X1 to X31 integer registers
 * for the 'I' profile, X1 to X15 for the 'E' profile, currently I assumed).
 *
 * Register		ABI Name	Description						Saver
 * x0			zero		Hard-wired zero					-
 * x1			ra			Return address					Caller
 * x2			sp			Stack pointer					Callee
 * x3			gp			Global pointer					-
 * x4			tp			Thread pointer					-
 * x5-7			t0-2		Temporaries						Caller
 * x8			s0/fp		Saved register/Frame pointer	Callee
 * x9			s1			Saved register					Callee
 * x10-11		a0-1		Function Arguments/return values Caller
 * x12-17		a2-7		Function arguments				Caller
 * x18-27		s2-11		Saved registers					Callee
 * x28-31		t3-6		Temporaries						Caller
 *
 * The RISC-V context is saved t FreeRTOS tasks in the following stack frame,
 * where the global and thread pointers are currently assumed to be constant so
 * are not saved:
 *
 * mstatus
 * x31
 * x30
 * x29
 * x28
 * x27
 * x26
 * x25
 * x24
 * x23
 * x22
 * x21
 * x20
 * x19
 * x18
 * x17
 * x16
 * x15
 * x14
 * x13
 * x12
 * x11
 * pvParameters
 * x9
 * x8
 * x7
 * x6
 * x5
 * portTASK_RETURN_ADDRESS
 * [chip specific registers go here]
 * pxCode
 */
.align 8
.func
pxPortInitialiseStack:
	csrr t0, sstatus					/* Obtain current sstatus value. */
	la t1, sstatus_init
	lw t2, 0(t1)
	or t0, t0, t2
#ifdef __riscv_flen
	lw t2, 4(t1)
	and t0, t0, t2
#endif
	andi t0, t0, ~0x2					/* Ensure interrupts are disabled when the stack is restored within an ISR.  Required when a task is created after the schedulre has been started, otherwise interrupts would be disabled anyway. */
	addi t1, x0, 0x120					/* Generate the value 0x120, which are the SPIE and SPP bits to set in mstatus. */
	or t0, t0, t1						/* Set SPIE and SPP bits in sstatus value. */

#ifdef __riscv_flen
	addi a0, a0, -(32 * portWORD_SIZE)	/* Space for registers x11-x31. */
#endif
	addi a0, a0, -portWORD_SIZE
	store_x t0, 0(a0)					/* sstatus onto the stack. */
	addi a0, a0, -(22 * portWORD_SIZE)	/* Space for registers x11-x31. */
	store_x a2, 0(a0)					/* Task parameters (pvParameters parameter) goes into register X10/a0 on the stack. */
	addi a0, a0, -(6 * portWORD_SIZE)	/* Space for registers x5-x9. */
	la t0, prvTaskExitError
	store_x t0, 0(a0)					/* Return address onto the stack, could be portTASK_RETURN_ADDRESS */

	addi a0, a0, -portWORD_SIZE
	store_x a1, 0(a0)					/* mret value (pxCode parameter) onto the stack. */
	ret
sstatus_init:
	.word (~SSTATUS_FS)
#ifdef __riscv_flen
	.word (0x1 << 13)
#endif
	.endfunc
/*-----------------------------------------------------------*/
