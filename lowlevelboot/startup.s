

	.macro loop,cunt
    li		t1,	0xffff
    li		t2,	\cunt
1:
	nop
	addi    t1, t1, -1
	bne		t1, x0, 1b
    li		t1,	0xffff
	addi    t2, t2, -1
	bne		t2, x0, 1b
	.endm

	.macro load_data,_src_start,_dst_start,_dst_end
	bgeu	\_dst_start, \_dst_end, 2f
1:
	lw      t0, (\_src_start)
	sw      t0, (\_dst_start)
	addi    \_src_start, \_src_start, 4
	addi    \_dst_start, \_dst_start, 4
	bltu    \_dst_start, \_dst_end, 1b
2:
	.endm

	.section .data
	.globl  _pen
	.type	_pen,%object
_pen:
	.word   1

	.section .text
	.globl _start
	.type _start,@function
_start:
    csrr    a0, mhartid
	beq		a0, zero, _no_wait
_loop:
	loop	0x1000
	la		t0, _pen
	lw		t0, 0(t0)
	beq		t0, zero, _run
	j       _loop
_no_wait:
	la		t0, _pen
	la		t1, 1
	sw		t1, 0(t0)
	//load opensbi_fw.bin 
	//[0x20200000:0x20400000] --> [0xBFF80000:0xC0000000]
    li		a0,	0x202
	slli	a0,	a0, 20
    li		a1,	0xbff
	slli	a1,	a1, 20
	li		a2,	0x800
	slli	a2,	a2, 8
	add 	a1,	a1, a2
	add 	a2,	a1, a2
	load_data a0,a1,a2

	//load qemu_sbi.dtb
	//[0x20080000:0x20100000] --> [0xBFF00000:0xBFF80000]
    li		a0,	0x2008
	slli	a0,	a0, 16
    li		a1,	0xbff
	slli	a1,	a1, 20
	li		a2,	0x800
	slli	a2,	a2, 4
	add 	a2,	a1, a2
	load_data a0,a1,a2

	//load trusted_fw.bin
	//[0x20400000:0x20800000] --> [0xBF800000:0xBFC00000]
    li		a0,	0x204
	slli	a0,	a0, 20
    li		a1,	0xbf8
	slli	a1,	a1, 20
    li		a2,	0xbfc
	slli	a2,	a2, 20
	load_data a0,a1,a2

	//load qemu_uboot.dtb
	//[0x20100000:0x20180000] --> [0xB0000000:0xB0080000]
    li		a0,	0x201
	slli	a0,	a0, 20
	li		a1,	0xb00
	slli	a1,	a1, 20
    li		a2,	0x800
	slli	a2,	a2, 8
	add 	a2,	a1, a2
	load_data a0,a1,a2

	//load u-boot.bin
	//[0x20800000:0x20C00000] --> [0xB0200000:0xB0600000]
    li		a0,	0x208
	slli	a0,	a0, 20
    li		a1,	0xb02
	slli	a1,	a1, 20
    li		a2,	0xb06
	slli	a2,	a2, 20
	load_data a0,a1,a2

_run:
    csrr    a0, mhartid
    li		a1,	0xbff
	slli	a1,	a1, 20
	li		t0,	0x800
	slli	t0,	t0, 8
	add 	t0,	a1, t0
	la		t1, _pen
	sw		zero, 0(t1)
    jr      t0


    .end