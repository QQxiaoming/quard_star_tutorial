.section .rodata.splash.init,"a"
.balign 16
.global __splash_u_boot_logo_begin
__splash_u_boot_logo_begin:
.incbin "drivers/video/u_boot_logo.bmp" 
__splash_u_boot_logo_end:
.global __splash_u_boot_logo_end
.balign 16
