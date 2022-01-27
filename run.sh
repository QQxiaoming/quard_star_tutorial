#!/bin/bash
set -e

SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)

GDB_START=""
#GDB_START="gdb --args" #use this need build qemu with --enable-debug option

DEBUG_PARAM=""
#DEBUG_PARAM="-d help -D qemu.log"

PLUGINS_PARAM=""
#PLUGINS_PATH=$SHELL_FOLDER/qemu-6.0.0/build/contrib/plugins/libhotblocks.so
#PLUGINS_PATH=$SHELL_FOLDER/qemu-6.0.0/build/tests/plugin/libsyscall.so
#PLUGINS_PARAM="-plugin $PLUGINS_PATH -d plugin"

NETDEV_PARAM=user,net=192.168.31.0/24,host=192.168.31.2,hostname=qemu,dns=192.168.31.56,tftp=$SHELL_FOLDER/output,bootfile=/linux_kernel/Image,dhcpstart=192.168.31.100,hostfwd=tcp::3522-:22,hostfwd=tcp::3580-:80,id=net0
#NETDEV_PARAM=tap,ifname=tap0,script=no,downscript=no,id=net0

VC=\
"full-screen | \
1920x1080 | \
1600x900 | \
1280x960 | \
1280x720 | \
1024x768 | \
960x640 | \
960x540 | \
800x600 | \
800x480 | \
640x480 | \
640x340"

MODE=\
"graphic | \
nographic | \
customize1 | \
customize2 | \
customize3"

BOOT=\
"pflash | \
spi | \
sd"

USAGE="usage $0 [$MODE] [$VC] [$BOOT]"

if [ $# != 1 ] ; then
	if [ $# != 2 ] ; then
		if [ $# != 3 ] ; then
		echo $USAGE
		exit 1
		fi
	fi
fi

if [ $# == 1 ] ; then
	DEFAULT_VC="1280x720"
	DBOOTCFG="sd"
else
	case "$2" in
	full-screen)
		DEFAULT_VC="$(xrandr -q 2>/dev/null | awk '/*/{print $1}')"
		FULL_SCREEN=-full-screen
		;;
	default)
		DEFAULT_VC="1280x720"
		;;
	*)
		DEFAULT_VC=$2
		;;
	esac
	if [ $# == 2 ] ; then
		DBOOTCFG="sd"
	else
		if [ $# == 3 ] ; then
			DBOOTCFG=$3
		fi
	fi
fi

WIDTH="$(echo $DEFAULT_VC | sed 's/\(.*\)x\(.*\)/\1/g')"
HEIGHT="$(echo $DEFAULT_VC | sed 's/\(.*\)x\(.*\)/\2/g')"

case "$1" in
graphic)
	GRAPHIC_PARAM="--display gtk,zoom-to-fit=false --serial vc:$DEFAULT_VC --serial vc:$DEFAULT_VC --serial vc:$DEFAULT_VC --monitor vc:$DEFAULT_VC --parallel none"
	ROWS="$(echo $WIDTH / 8 |bc)"
	COLS="$(echo $HEIGHT / 16 |bc)"
	DEFAULT_V=":vn:$COLS""x""$ROWS:"
	;;
nographic)
	DEFAULT_VN="$(stty size | sed '/ \+/s//x/g')" 
    GRAPHIC_PARAM="-nographic --parallel none"
	DEFAULT_V=":vn:$DEFAULT_VN:"
    ;;
customize1)
	GRAPHIC_PARAM="--display gtk,zoom-to-fit=false --serial vc:$DEFAULT_VC --serial vc:$DEFAULT_VC --serial vc:$DEFAULT_VC --monitor stdio --parallel none"
	ROWS="$(echo $WIDTH / 8 |bc)"
	COLS="$(echo $HEIGHT / 16 |bc)"
	DEFAULT_V=":vn:$COLS""x""$ROWS:"
	;;
customize2)
	GRAPHIC_PARAM="--display gtk,zoom-to-fit=false --serial telnet:127.0.0.1:3441,server,nowait --serial telnet:127.0.0.1:3442,server,nowait --serial telnet:127.0.0.1:3443,server,nowait --monitor stdio --parallel none"
	DEFAULT_V=":vn:24x80:"
	;;
customize3)
	GRAPHIC_PARAM="--display gtk,zoom-to-fit=false --serial telnet:127.0.0.1:3441,server,nowait --serial telnet:127.0.0.1:3442,server,nowait --serial telnet:127.0.0.1:3443,server,nowait --monitor none --parallel none"
	DEFAULT_V=":vn:24x80:"
	;;
customize4)
	GRAPHIC_PARAM="-nographic --serial telnet:127.0.0.1:3441,server,nowait --serial telnet:127.0.0.1:3442,server,nowait --serial telnet:127.0.0.1:3443,server,nowait --monitor none --parallel none"
	DEFAULT_V=":vn:24x80:"
	;;
customize5)
	# vnc base port is 5900, so this 1 is 5901
	GRAPHIC_PARAM="-display vnc=127.0.0.1:1 --serial telnet:127.0.0.1:3441,server,nowait --serial telnet:127.0.0.1:3442,server,nowait --serial telnet:127.0.0.1:3443,server,nowait --monitor telnet:127.0.0.1:3430,server,nowait --parallel none"
	DEFAULT_V=":vn:24x80:"
	;;
update_test)
	GRAPHIC_PARAM="-nographic --serial telnet:127.0.0.1:3441,server,wait --serial telnet:127.0.0.1:3442,server,nowait --serial telnet:127.0.0.1:3443,server,nowait --monitor stdio --parallel none"
	DEFAULT_V=":vn:24x80:"
	;;
--help)
	echo $USAGE
	exit 0
	;;
*)
	echo $USAGE
	exit 1	
	;;
esac

$GDB_START $SHELL_FOLDER/output/qemu/bin/qemu-system-riscv64 \
-M quard-star,mask-rom-path="$SHELL_FOLDER/output/mask_rom/mask_rom.bin" \
-m 1G \
-smp 8 \
-drive if=pflash,bus=0,unit=0,format=raw,file=$SHELL_FOLDER/output/fw/fw.bin,id=mtd0 \
-drive if=mtd,format=raw,file=$SHELL_FOLDER/output/fw/norflash.img,id=mtd1 \
-drive if=none,format=raw,file=$SHELL_FOLDER/output/fw/usb.img,id=usb0 \
-drive if=sd,format=raw,file=$SHELL_FOLDER/output/fw/sd.img,id=sd0 \
-drive if=none,format=raw,file=$SHELL_FOLDER/output/rootfs/rootfs.img,id=disk0 \
-chardev socket,telnet=on,host=127.0.0.1,port=3450,server=on,wait=off,id=usb1 \
-fsdev local,security_model=mapped-xattr,path=$SHELL_FOLDER,id=fsdev0 \
-netdev $NETDEV_PARAM \
-audiodev sdl,id=audio0 \
-global virtio-mmio.force-legacy=false \
-device virtio-blk-device,drive=disk0,id=hd0 \
-device virtio-gpu-device,xres=$WIDTH,yres=$HEIGHT,id=video0 \
-device virtio-mouse-device,id=input0 \
-device virtio-keyboard-device,id=input1 \
-device virtio-9p-device,fsdev=fsdev0,mount_tag=hostshare,id=fs0 \
-device virtio-net-device,netdev=net0 \
-device usb-storage,drive=usb0 \
-device usb-serial,always-plugged=true,chardev=usb1 \
-device wm8750,audiodev=audio0 \
-fw_cfg name="opt/qemu_cmdline",string="qemu_vc=$DEFAULT_V" \
-global quard-star-syscon.boot-cfg="$DBOOTCFG" \
$GRAPHIC_PARAM $FULL_SCREEN $DEBUG_PARAM $PLUGINS_PARAM
