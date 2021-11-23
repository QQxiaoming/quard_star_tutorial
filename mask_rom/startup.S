#include "riscv_encoding.h"
#include "quard_star.h"

	.extern secondary_start_flag
	.section .init
	.globl _start
	.type _start,@function
_start:
	.cfi_startproc
	.cfi_undefined ra
.option push
.option norelax
	la  gp, __global_pointer$
.option pop

	// FIXME:fix qemu pmp not init 0x0 addr tlb_size is 1 issue
  	csrw pmpcfg0, 0
  	csrw pmpcfg1, 0
  	csrw pmpcfg2, 0
  	csrw pmpcfg3, 0

	// Continue primary hart
	li   a1, PRIM_HART
    csrr a0, mhartid
	bne  a0, a1, secondary
	
	la		t0, copy_finsh
	sb		zero, 0(t0)

  	csrw sie, 0
  	csrw sip, 0
	/* set to disable FPU */
	li t0, SSTATUS_FS
	csrc sstatus, t0
#ifdef __riscv_flen
	/* set to init FPU */
	li t0, (0x1 << 13)
	csrs sstatus, t0
#endif
	li t0, SSTATUS_SUM // SUM in sstatus
	csrs sstatus, t0

	// Primary hart
	la sp, _sp
	csrw sscratch, sp

	// Load data section
	la a0, _data_lma
	la a1, _data
	la a2, _edata
	bgeu a1, a2, 2f
1:
	LOAD t0, (a0)
	STOR t0, (a1)
	addi a0, a0, REGSIZE
	addi a1, a1, REGSIZE
	bltu a1, a2, 1b
2:

	// Clear bss section
	la a0, __bss_start
	la a1, __bss_end
	bgeu a0, a1, 2f
1:
	STOR zero, (a0)
	addi a0, a0, REGSIZE
	bltu a0, a1, 1b
2:

	/* Call global constructors */
	la a0, __libc_fini_array
	call atexit
	call __libc_init_array
	
	// argc, argv, envp is 0
	li  a0, 0
	li  a1, 0
	li  a2, 0
	jal main
1:
	wfi
	j 1b

ecall_err:
	wfi
	j ecall_err
	
secondary:
	li t0, 0x80
    csrr t1, mhartid
	mul t0, t0, t1
	la t1, heap_end
	sub	sp,t1,t0
	jal secondary_main
	.cfi_endproc


