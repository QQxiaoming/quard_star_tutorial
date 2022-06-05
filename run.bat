@echo off

set "WIDTH=1280"
set "HEIGHT=720"
set "DEFAULT_V=:vn:24x80:"
set "DEFAULT_VC=vc:%WIDTH%x%HEIGHT%"
set "DBOOTCFG=sd"
set "GRAPHIC_PARAM=--display gtk,zoom-to-fit=false --serial %DEFAULT_VC% --serial %DEFAULT_VC% --serial %DEFAULT_VC% --monitor %DEFAULT_VC% --parallel none"

%cd%\output\qemu_w64\qemu-system-riscv64w.exe ^
-M quard-star,mask-rom-path=./output/mask_rom/mask_rom.bin ^
-m 1G ^
-smp 8 ^
-drive if=pflash,bus=0,unit=0,format=raw,file=./output/fw/pflash.img,id=mtd0 ^
-drive if=mtd,bus=0,unit=0,format=raw,file=./output/fw/norflash.img,id=mtd1 ^
-drive if=mtd,bus=1,unit=0,format=raw,file=./output/fw/nandflash.img,id=mtd2 ^
-drive if=none,format=raw,file=./output/fw/usb.img,id=usb0 ^
-drive if=sd,format=raw,file=./output/fw/sd.img,id=sd0 ^
-drive if=none,format=raw,file=./output/rootfs/rootfs.img,id=disk0 ^
-chardev socket,telnet=on,host=127.0.0.1,port=3450,server=on,wait=off,id=usb1 ^
-netdev user,net=192.168.31.0/24,host=192.168.31.2,hostname=qemu,dns=192.168.31.56,tftp=./output,bootfile=/linux_kernel/Image,dhcpstart=192.168.31.100,hostfwd=tcp::3522-:22,hostfwd=tcp::3580-:80,id=net0 ^
-audiodev dsound,id=audio0 ^
-global virtio-mmio.force-legacy=false ^
-device virtio-blk-device,drive=disk0,id=hd0 ^
-device virtio-gpu-device,xres=%WIDTH%,yres=%HEIGHT%,id=video0 ^
-device virtio-mouse-device,id=input0 ^
-device virtio-keyboard-device,id=input1 ^
-device virtio-net-device,netdev=net0 ^
-device usb-storage,drive=usb0 ^
-device usb-serial,always-plugged=true,chardev=usb1 ^
-device wm8750,audiodev=audio0 ^
-fw_cfg name="opt/qemu_cmdline",string="qemu_vc=%DEFAULT_V%" ^
-global quard-star-syscon.boot-cfg=%DBOOTCFG% ^
%GRAPHIC_PARAM%
