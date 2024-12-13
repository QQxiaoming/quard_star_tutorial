/*
 * Startup glue code for parisc firmware
 *
 *   (C) 2017-2024 Helge Deller <deller@gmx.de>
 */

#include "parisc/hppa.h"
#include "parisc/hppa_hardware.h"
#include "autoconf.h"
#include "autoversion.h"

	/* load 32-bit 'value' into 'reg' compensating for the ldil
	 * sign-extension when running in wide mode. */
	.macro	load32 value, reg
	ldil	L%\value, \reg
	ldo	R%\value(\reg), \reg
#ifdef CONFIG_64BIT
	depdi   0, 31, 32, \reg
#endif
	.endm

	/* set upper 32-bits of firmware address */
	.macro	load_fw_upper32 reg
#ifdef CONFIG_64BIT
	depdi   FIRMWARE_HIGH, 31, 32, \reg
#endif
	.endm

;! set the W bit
#define set_PSW_W	.level 2.0 !	ssm PSW_W_SM, %r0 !	.level LEVEL

;! nuke the W bit
#define clear_PSW_W	.level 2.0 !	rsm PSW_W_SM, %r0 !	.level LEVEL

#define ENTRY(name) \
	.export name !\
	.align 4 !\
name:

#define END(name) \
	.size name, .-name

#define ENDPROC(name) \
	.type name, @function !\
	END(name)

#define BOOTADDR(x)	(x)

	.macro loadgp
#ifdef CONFIG_64BIT
	ldil		L%__gp, %r27
	ldo		R%__gp(%r27), %r27
	load_fw_upper32	%r27
#else
	ldil		L%$global$, %r27
	ldo		R%$global$(%r27), %r27
#endif
	.endm

#ifdef CONFIG_64BIT
#define LEVEL	2.0w
#define LDREG	ldd
#define STREG	std
#define LDREGX  ldd,s
#define LDREGM	ldd,mb
#define STREGM	std,ma
#define SHRREG	shrd
#define SHLREG	shld
#define ANDCM   andcm,*
#define	COND(x)	* ## x
#define FRAME_SIZE	128
#define CALLEE_REG_FRAME_SIZE	144
#define ASM_ULONG_INSN	.dword
#define WORD_LEN 8
#define INT_LEN 4
#else	/* CONFIG_64BIT */
#define LEVEL	1.1
#define LDREG	ldw
#define STREG	stw
#define LDREGX  ldwx,s
#define LDREGM	ldwm
#define STREGM	stwm
#define SHRREG	shr
#define SHLREG	shlw
#define ANDCM   andcm
#define COND(x)	x
#define FRAME_SIZE	64
#define CALLEE_REG_FRAME_SIZE	128
#define ASM_ULONG_INSN	.word
#define WORD_LEN 4
#define INT_LEN 4
#endif

/* various control register and irq bits */
#define PSW_I		1
#define PSW_Q		8
#define CR_EIRR		23
#define CR_IVA		14
#define CR_EIEM		15
#define PSW_W_SM	0x200
#define PSW_W_BIT       36

	.import	$global$
	.section ".head.text","ax"
	 .level LEVEL

	/* On HPMC, the CPUs will start here at 0xf0000000 */
hpmc_entry:
        b,n toc_asm_entry       /* TOC and HPMC */

reset_entry:
	/* at reset, the CPU begins fetching instructions from address 0xf0000004. */
	b,n startup

marker:
	/* file identification, useful for strings command on binary file to see version. */
	.stringz "PA-RISC/HPPA PDC Firmware " SEABIOS_HPPA_VERSION_STR " (SeaBIOS fork)"
	.stringz "https://github.com/hdeller/seabios-hppa"
	.stringz BUILD_VERSION

/*******************************************************
	Firmware startup code
 *******************************************************/

        .align 0x80
ENTRY(startup)
	rsm	PSW_I, %r0	/* disable local irqs */
	ssm	PSW_Q, %r0	/* enable PSW_Q flag */

#ifdef CONFIG_64BIT
	/* check if we really run on 64-bit CPU */
	ldo	-1(%r0), %r1
	mtctl	%r1,%cr11
	mfctl,w %cr11, %r1
	ldo	-31(%r1),%r1
	cmpib,<>,n 0,%r1,1f
	.word 0xfffdead0	/* immediately halt the emulator */
1:
	/* we now know we run on a 64-bit CPU. */
	/* next step: turn on the PSW.W flag (enable 64-bit mode) */
	load32		2f,%r11
	load_fw_upper32 %r11

	mtctl           %r0,%cr17       /* Clear IIASQ tail */
	mtctl           %r0,%cr17       /* Clear IIASQ head */

	/* Load RFI target into PC queue */
	mtctl           %r11,%cr18      /* IIAOQ head */
	ldo             4(%r11),%r11
	mtctl           %r11,%cr18      /* IIAOQ tail */

	load32          (0x08000000 | PSW_Q),%r10 /* PSW.W=1 */
	mtctl           %r10,%ipsw

	/* Jump through hyperspace to enable PSW.W */
	rfi
	nop
2:
#else
	/* clear any PSW.W on 32-bit firmware */
	clear_PSW_W
#endif

	/* Make sure space registers are set to zero */
	mtsp    %r0,%sr0
	mtsp    %r0,%sr1
	mtsp    %r0,%sr2
	mtsp    %r0,%sr3
	mtsp    %r0,%sr4
	mtsp    %r0,%sr5
	mtsp    %r0,%sr6
	mtsp    %r0,%sr7

	/* If CPU HPA is already set in CPU_HPA_CR_REG then the
	 * CPU is already initialized and the machine was only reset */
	mfctl   CPU_HPA_CR_REG, %r1
	comib,= 0,%r1,$startup_fresh_booted
	nop

$startup_just_rebooted:
	/* Get current CPU HPA. It was stored there at initial bootup. */
	mfctl   CPU_HPA_CR_REG, %r5

	/* branch if this is the monarch cpu */
	load32 CPU_HPA, %r1
	comb,=,n %r5,%r1,$is_monarch_cpu_reboot

	b,n	enter_smp_idle_loop

$startup_fresh_booted:
	/* Here the machine was booted from scratch: */

	/* Save CPU HPA in cr7, hopefully HP-UX will not use that register. */
	mtctl   %r5, CPU_HPA_CR_REG /* store CPU HPA */

	/* branch if this is the monarch cpu */
	load32 CPU_HPA,%r1
	comb,= %r5,%r1,$is_monarch_cpu
	nop

ENTRY(enter_smp_idle_loop)
	/* IDLE LOOP for SMP CPUs - wait for rendenzvous. */
	mfctl   CPU_HPA_CR_REG, %r25 /* get CPU HPA from cr7 */

	rsm	PSW_I, %r0	/* disable local irqs */
	mtctl	%r0, CR_EIEM	/* disable all external irqs */

	/* EIRR : clear all pending external intr */
	load32	-1,%r1
	mtctl	%r1, CR_EIRR
	mfctl	CR_EIRR, %r0
	mtctl	%r0, CR_EIRR

	/* Load IVT for SMT tiny loop exit */
	load32	BOOTADDR(smp_ivt),%r1
	mtctl	%r1, CR_IVA

	/* enable CPU local interrupts */
	load32	1<<31, %r1	/* allow IRQ0 (Timer) */
	mtctl	%r1, CR_EIEM
	ssm	PSW_I, %r0	/* enable local irqs */

	/* endless idle loop for secondary CPUs. Exits to $smp_exit_loop by IRQ only */
$smp_idle_loop:
	b $smp_idle_loop
	or %r10,%r10,%r10	/* qemu sleep instruction */

$smp_exit_loop:
	rsm	PSW_I, %r0	/* disable local irqs */
	mtctl	%r0, CR_EIEM

	/* provide PDCE_PROC entry in arg0 (required on 64-bit) */
	load32	MEM_PDC_ENTRY, %arg0

	/* jump to rendevouz */
	ldw	0x10(%r0),%r3	/* MEM_RENDEZ */
	/* ldw	0x28(%r0),%r0	MEM_RENDEZ_HI - assume addr < 4GB */
	cmpb,=,n  %r0,%r3,enter_smp_idle_loop
	nop			/* failed backward branch is nullified */
	load32	startup, %rp
	load_fw_upper32 %rp
	bv,n	0(%r3)

$is_monarch_cpu:
	/* Save boot_args in PAGE0->pad608[]. Only monarch CPU does this once. */
        load32          BOOTADDR(0x608),%r1
        STREGM          %r26,WORD_LEN(%r1)
        STREGM          %r25,WORD_LEN(%r1)
        STREGM          %r24,WORD_LEN(%r1)
        STREGM          %r23,WORD_LEN(%r1)
        STREGM          %r22,WORD_LEN(%r1)
        STREGM          %r21,WORD_LEN(%r1)
        STREGM          %r20,WORD_LEN(%r1)
        STREGM          %r19,WORD_LEN(%r1)

$is_monarch_cpu_reboot:
	/* Initialize stack pointer */
	load32	BOOTADDR(parisc_stack),%r1
	load_fw_upper32	%r1
	ldo	FRAME_SIZE(%r1),%sp

	/* Initialize the global data pointer */
	loadgp

	/* Clear BSS on monarch CPU */
	.import _bss,data
	.import _ebss,data

	load32	BOOTADDR(_bss),%r3
	load32	BOOTADDR(_ebss),%r4
$bss_loop:
	cmpb,<<,n %r3,%r4,$bss_loop
	STREGM	%r0,WORD_LEN(%r3)

	load32	BOOTADDR(start_parisc_firmware),%r3
	load_fw_upper32 %r3
	bv	0(%r3)
	copy	%r0,%r2
END(startup)

/*******************************************************
  TOC handler
  Write all GRs, CRs, SRs and the iaoq_back and iasq_back registers (in
  r24/r25) into PIM area (when it's not filled yet). This is done by trashing the
  shadow registers.
  In a second step call the arificial getshadowregs asm instruction to restore
  the shadow registers to their real values and store them in PIM as well.  Then
  call the C-code.
 *******************************************************/

/* uses the shadow registers: 1,8,9,16,17,24,25 */
#define PIM_PTR         %r1
#define TEMP            %r8
#define TEMP2           %r9
#define PIM_SAVE        %r16
#define IASQ_BACK       %r24    /* provided by qemu */
#define IAOQ_BACK       %r25    /* provided by qemu */

        .import pim_toc_data, data
ENTRY(toc_asm_entry)
        /* serialize CPUs on entry */
        load32  BOOTADDR(toc_lock),TEMP
0:      ldcw,co 0(TEMP),TEMP2
        cmpib,= 0,TEMP2,0b
        nop

        mfctl   CPU_HPA_CR_REG, TEMP2   /* get CPU HPA from cr7 */
        extru   TEMP2,31-12,4, TEMP     /* extract cpu id */

        load32  BOOTADDR(pim_toc_data), PIM_PTR

1:      comib,= 0,TEMP,2f
        ldo     -1(TEMP),TEMP
        ldo     (PIM_STORAGE_SIZE)(PIM_PTR), PIM_PTR /* find PIM entry */
        b 1b

2:      copy    PIM_PTR, PIM_SAVE
        mtsp    %r0,%sr0

        /* save registers into PIM only if cpu_state field is empty */
        ldw     ((32+32+8+2)*WORD_LEN + 1*INT_LEN)(PIM_SAVE), TEMP
        comib,<>,n 0, TEMP, 5f

        /* save all general registers */
        .set loop,0
        .rept 32
        copy    loop,TEMP
        STREGM  TEMP, WORD_LEN(PIM_PTR)
        .set loop,loop+1
        .endr

        /* save all control registers */
        .set loop,0
        .rept 32
        mfctl   loop,TEMP
        STREGM  TEMP, WORD_LEN(PIM_PTR)
        .set loop,loop+1
        .endr

        /* save all space registers */
        .set loop,0
        .rept 8
        mfsp    loop,TEMP
        STREGM  TEMP, WORD_LEN(PIM_PTR)
        .set loop,loop+1
        .endr

        /* save IASQ_back and IAOQ_back as provided by qemu */
        STREG   IASQ_BACK, ((32+32+8+0)*WORD_LEN)(PIM_SAVE)
        STREG   IAOQ_BACK, ((32+32+8+1)*WORD_LEN)(PIM_SAVE)

        /* restore shadow registers, can not use PIM_SAVE reg for this */
        copy    PIM_SAVE, %r26
        .word 0xfffdead2 /* qemu artificial getshadowregs asm instruction */
        STREG   %r1, (1*WORD_LEN)(%r26)
        STREG   %r8, (8*WORD_LEN)(%r26)
        STREG   %r9, (9*WORD_LEN)(%r26)
        STREG   %r16, (16*WORD_LEN)(%r26)
        STREG   %r17, (17*WORD_LEN)(%r26)
        STREG   %r24, (24*WORD_LEN)(%r26)
        STREG   %r25, (25*WORD_LEN)(%r26)

#ifdef CONFIG_64BIT
        /* cr11 (sar) is a funny one.  5 bits on PA1.1 and 6 bit on PA2.0
         * For PA2.0 mtsar or mtctl always write 6 bits, but mfctl only
         * reads 5 bits.  Use mfctl,w to read all six bits.  Otherwise
         * we lose the 6th bit on a save/restore. */
        mfctl,w %cr11, TEMP
        STREG   TEMP, ((32+11)*WORD_LEN)(%r26)
#endif

5:      /* call the "C" toc_handler in SeaBIOS */
        loadgp
        load32  BOOTADDR(parisc_stack), %sp
        b,l     toc_handler, %r2
        ldo     FRAME_SIZE(%sp),%sp

        /* call OS handler, in case it returns reset the system */
        load32  BOOTADDR(reset), %rp
        bv,n    0(%ret0)
END(toc_asm_entry)


/*******************************************************
	SMP Interrupt vector table (IVT)
 *******************************************************/

	.macro  DEF_IVA_ENTRY
	.align 32
	load32 BOOTADDR($smp_exit_loop),%r1
	bv	0(%r1)
	nop
	.endm

	.align 32	/* should be 4k aligned but qemu does not check */
ENTRY(smp_ivt)
	.rept 32
	DEF_IVA_ENTRY
	.endr
END(smp_ivt)


/*******************************************************
	PDC and IODC entry
 *******************************************************/

/* XXX: memory location in page0: vec_pad0[], temporary storage for PSW.W */
#define PSW_OFFSET1	24
#define PSW_OFFSET2	28

ENTRY(pdc_entry)
	stw %rp,-20(%sp)
	stw %dp,-32(%sp)
	stw %arg0,-36(%sp)
	stw %arg1,-40(%sp)
	stw %arg2,-44(%sp)
	stw %arg3,-48(%sp)
	stw %r31,PSW_OFFSET1(%r0)	/* store PSW_W */
	ldo -FRAME_SIZE(%sp),%arg0

	loadgp
	b,l parisc_pdc_entry, %rp
	ldo FRAME_SIZE(%sp),%sp

	ldo -FRAME_SIZE(%sp),%sp
	ldw -20(%sp),%rp
	ldw -32(%sp),%dp
	ldw PSW_OFFSET1(%r0),%r31
	load32 0x8000000,%r1	/* mask PSW_W */
	and,= %r31,%r1,%r0
	bv %r0(%rp)		/* return without setting PSW_W */
	nop
	bv %r0(%rp)		/* return with setting PSW_W */
	.level 2.0
	nop // ssm PSW_W_SM,%r0
	.level LEVEL
END(pdc_entry)

/* pdc_entry_table will be copied into low memory. */
ENTRY(pdc_entry_table)
	load32 pdc_entry,%r1
	clear_PSW_W
	bv,n %r0(%r1)
END(pdc_entry_table)

ENTRY(iodc_entry_table)
	load32 parisc_iodc_ENTRY_INIT,   %r1
	load32 parisc_iodc_ENTRY_IO,     %r1
	load32 parisc_iodc_ENTRY_SPA,    %r1
	load32 parisc_iodc_ENTRY_CONFIG, %r1
	load32 hlt,			 %r1 /* obsolete */
	load32 parisc_iodc_ENTRY_TEST,   %r1
	load32 parisc_iodc_ENTRY_TLB,    %r1
END(iodc_entry_table)

ENTRY(iodc_entry)
	load32 parisc_iodc_ENTRY_IO, %r1

	stw %rp,-20(%sp)
	stw %dp,-32(%sp)
	stw %arg0,-36(%sp)
	stw %arg1,-40(%sp)
	stw %arg2,-44(%sp)
	stw %arg3,-48(%sp)
	clear_PSW_W
	stw %r31,PSW_OFFSET2(%r0)	/* store PSW_W */
	ldo -FRAME_SIZE(%sp),%arg0

	loadgp
	load32 .iodc_ret, %rp
	bv %r0(%r1)
	ldo FRAME_SIZE(%sp),%sp
.iodc_ret:
	ldo -FRAME_SIZE(%sp),%sp
	ldw -20(%sp),%rp
	ldw -32(%sp),%dp

// .word 0xfffdead0
	ldw PSW_OFFSET2(%r0),%r31
	load32 0x8000000,%r1	/* mask PSW_W */
	and,= %r31,%r1,%r0
	bv %r0(%rp)		/* return without setting PSW_W */
	nop
	bv %r0(%rp)		/* return with setting PSW_W */
	.level 2.0
	nop // ssm PSW_W_SM,%r0
	.level LEVEL
END(iodc_entry)

/****************************************************************
 * Rom Header for VGA / STI
 ****************************************************************/

#if 0 // def CONFIG_BUILD_VGABIOS

        .section .rom.header
        .global _rom_header, _rom_header_size, _rom_header_checksum
_rom_header:
        .word 0xaa55
_rom_header_size:
        .byte 0
_rom_header_entry:
        .word _optionrom_entry  // b,n ?
_rom_header_checksum:
        .byte 0
_rom_header_other:
        .space 17
_rom_header_pcidata:
#if CONFIG_VGA_PCI == 1
        .word rom_pci_data
#else
        .word 0
#endif
_rom_header_pnpdata:
        .word 0
_rom_header_other2:
        .word 0
_rom_header_signature:
        .asciz "IBM"


ENTRY(_optionrom_entry)
	.import vga_post
	load32 BOOTADDR(vga_post), %r1
	bv,n %r0(%r1)
END(_optionrom_entry)

#endif /* CONFIG_BUILD_VGABIOS */
