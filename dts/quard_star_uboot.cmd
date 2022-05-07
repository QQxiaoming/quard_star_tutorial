# try use tftp load boot.cfg
mw.l ${kernel_addr_r} 0x0
dhcp ${kernel_addr_r} /boot.cfg

# load bootfile according to boot.cfg
if itest.l *${kernel_addr_r} == 0x70746674; 
then 
    echo tftpboot...
    dhcp ${kernel_addr_r} /linux_kernel_next/Image
    dhcp ${fdt_addr_r} /uboot/quard_star_uboot.dtb
else
    echo virtioboot...
    load virtio 0:1 ${kernel_addr_r} /Image
    load virtio 0:1 ${fdt_addr_r} /quard_star.dtb
fi

# boot kernel
booti ${kernel_addr_r} - ${fdt_addr_r}
