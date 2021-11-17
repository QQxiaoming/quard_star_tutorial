	.global lowlevel_init

	.section .init.lowlevel_init
lowlevel_init:
    //init ddr, but in qemu no need to do this
	li		a0,	0x0
	ret

    .end