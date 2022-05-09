# try use tftp load boot.cfg
mw.l ${kernel_addr_r} 0x0 8
dhcp ${kernel_addr_r} /boot.cfg

# load bootfile according to boot.cfg
if itest.s *${kernel_addr_r} == "tftp-kgdb";
then 
    echo tftpkgdb...
    dhcp ${fdt_addr_r} /uboot/quard_star_uboot_kgdb.dtb
    dhcp ${kernel_addr_r} /linux_kernel_next/Image
else
    if itest.s *${kernel_addr_r} == "tftp-boot";
    then
        echo tftpboot...
        dhcp ${fdt_addr_r} /uboot/quard_star_uboot.dtb
        dhcp ${kernel_addr_r} /linux_kernel_next/Image
    else
        echo virtioboot...
        load virtio 0:1 ${fdt_addr_r} /quard_star.dtb
        load virtio 0:1 ${kernel_addr_r} /Image
    fi
fi

# boot kernel
booti ${kernel_addr_r} - ${fdt_addr_r}
