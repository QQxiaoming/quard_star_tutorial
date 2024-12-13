#!/bin/bash
###############################################################################
# This file is part of the quard_star_tutorial project.                       #
# Copyright (C) 2021 Quard <2014500726@smail.xtu.edu.cn>                      #
###############################################################################

################################## env param ##################################
set -e
SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
UNAMEOUT="$(uname -s)"
case "${UNAMEOUT}" in
Linux*)     
	QEMU_PATHNAME="qemu" 
	QEMU_DISPLAY="gtk,zoom-to-fit=false,show-tabs=on,show-cursor=off"
	QEMU_AUDIO="sdl"
	;;
Darwin*)    
	QEMU_PATHNAME="qemu_macos" 
	QEMU_DISPLAY="cocoa"
	QEMU_AUDIO="coreaudio"
	;;
esac
###############################################################################

################################# user param ##################################
if [ -z "$HOST_GDB_PARAM" ]; then
HOST_GDB_PARAM=""
#HOST_GDB_PARAM="gdb --args" #use this need build qemu with --enable-debug option
fi

if [ -z "$TARGET_GDB_PARAM" ]; then
TARGET_GDB_PARAM=""
#TARGET_GDB_PARAM="-s -S"
fi

if [ -z "$DEBUG_PARAM" ]; then
DEBUG_PARAM=""
#DEBUG_PARAM="-d help -D qemu.log"
fi

if [ -z "$PLUGINS_PARAM" ]; then
PLUGINS_PARAM=""
#PLUGINS_PATH=$SHELL_FOLDER/qemu-8.2.8/build/contrib/plugins/libhotblocks.so
#PLUGINS_PATH=$SHELL_FOLDER/qemu-8.2.8/build/tests/plugin/libsyscall.so
#PLUGINS_PARAM="-plugin $PLUGINS_PATH -d plugin"
fi

if [ -z "$NETDEV0_PARAM" ]; then
NETDEV0_PARAM="-netdev user,net=192.168.31.0/24,host=192.168.31.2,hostname=qemu_net0,dns=192.168.31.56,tftp=$SHELL_FOLDER/output,bootfile=/linux_kernel/Image,dhcpstart=192.168.31.100,hostfwd=tcp::3522-:22,hostfwd=tcp::3580-:80,id=net0"
#NETDEV0_PARAM="-netdev tap,ifname=tap0,script=no,downscript=no,id=net0"
#NETDEV0_PARAM=""
fi

if [ -z "$NETDEV1_PARAM" ]; then
NETDEV1_PARAM="-netdev user,net=192.168.32.0/24,host=192.168.32.2,hostname=qemu_net1,dns=192.168.32.56,dhcpstart=192.168.32.100,id=net1"
#NETDEV1_PARAM="-netdev tap,ifname=tap0,script=no,downscript=no,id=net1"
#NETDEV1_PARAM=""
fi

if [ -z "$AUDIO_PARAM" ]; then
AUDIO_PARAM="-audio $QEMU_AUDIO,id=audio0"
#AUDIO_PARAM="-audio none,id=audio0"
fi

if [ -z "$HOST_VCAN_PARAM" ]; then
HOST_VCAN_PARAM=""
#HOST_VCAN_PARAM="-object can-host-socketcan,id=socketcan0,if=vcan0,canbus=canbus0"
fi

if [ -z "$UPDATECFG" ]; then
UPDATECFG="false"
#UPDATECFG="true"
fi


###############################################################################

################################## cli param ##################################
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
	BOOTCFG="sd"
else
	case "$2" in
	full-screen)
		case "${UNAMEOUT}" in
		Linux*)     
			DEFAULT_VC="$(xrandr -q 2>/dev/null | awk '/*/{print $1}')"
			;;
		Darwin*)    
			DEFAULT_VC="$(displayplacer list | sed -n '/^Resolution: /p' | awk '/ /{print $2}')"
			;;
		esac
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
		BOOTCFG="sd"
	else
		if [ $# == 3 ] ; then
			BOOTCFG=$3
		fi
	fi
fi

WIDTH="$(echo $DEFAULT_VC | sed 's/\(.*\)x\(.*\)/\1/g')"
HEIGHT="$(echo $DEFAULT_VC | sed 's/\(.*\)x\(.*\)/\2/g')"

USE_VIRTIO_GPU="1"

case "$1" in
graphic)
	GRAPHIC_PARAM="--display $QEMU_DISPLAY --serial vc:$DEFAULT_VC --serial vc:$DEFAULT_VC --serial vc:$DEFAULT_VC --monitor vc:$DEFAULT_VC --parallel none"
	ROWS="$(echo $WIDTH / 8 |bc)"
	COLS="$(echo $HEIGHT / 16 |bc)"
	DEFAULT_V=":vn:$COLS""x""$ROWS:"
	;;
nographic)
	case "${UNAMEOUT}" in
    Linux*)     
		DEFAULT_VN="$(stty size | sed '/ \+/s//x/g')" 
        ;;
    Darwin*)    
		DEFAULT_VN="$(stty size | sed '/ /s//x/g')" 
        ;;
	esac
    GRAPHIC_PARAM="-nographic --parallel none"
	DEFAULT_V=":vn:$DEFAULT_VN:"
    ;;
customize1)
	GRAPHIC_PARAM="--display $QEMU_DISPLAY --serial vc:$DEFAULT_VC --serial vc:$DEFAULT_VC --serial vc:$DEFAULT_VC --monitor stdio --parallel none"
	ROWS="$(echo $WIDTH / 8 |bc)"
	COLS="$(echo $HEIGHT / 16 |bc)"
	DEFAULT_V=":vn:$COLS""x""$ROWS:"
	;;
customize2)
	GRAPHIC_PARAM="--display $QEMU_DISPLAY --serial telnet:127.0.0.1:3441,server,nowait --serial telnet:127.0.0.1:3442,server,nowait --serial telnet:127.0.0.1:3443,server,nowait --monitor stdio --parallel none"
	DEFAULT_V=":vn:24x80:"
	;;
customize3)
	GRAPHIC_PARAM="--display $QEMU_DISPLAY --serial telnet:127.0.0.1:3441,server,nowait --serial telnet:127.0.0.1:3442,server,nowait --serial telnet:127.0.0.1:3443,server,nowait --monitor none --parallel none"
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
server)
	GRAPHIC_PARAM="-display vnc=0.0.0.0:10001 --serial telnet:0.0.0.0:13441,server,nowait --serial telnet:0.0.0.0:13442,server,nowait --serial telnet:0.0.0.0:13443,server,nowait --monitor telnet:0.0.0.0:13430,server,nowait --parallel none"
	DEFAULT_V=":vn:24x80:"
	USE_VIRTIO_GPU="0"
	;;
server_websock)
	GRAPHIC_PARAM="-display vnc=0.0.0.0:1,websocket=0.0.0.0:15901 --serial telnet:0.0.0.0:13441,websocket=on,server,nowait --serial telnet:0.0.0.0:13442,websocket=on,server,nowait --serial telnet:0.0.0.0:13443,websocket=on,server,nowait --monitor telnet:0.0.0.0:13430,websocket=on,server,nowait --parallel none"
	DEFAULT_V=":vn:24x80:"
	USE_VIRTIO_GPU="0"
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

case "$USE_VIRTIO_GPU" in
1)
	VIRTIO_GPU="-device virtio-gpu-device,xres=$WIDTH,yres=$HEIGHT,id=video0"
	;;
0)
	VIRTIO_GPU=""
	;;
esac

###############################################################################

################################## run qemu ##################################
$HOST_GDB_PARAM $SHELL_FOLDER/output/$QEMU_PATHNAME/bin/qemu-system-riscv64 \
-M quard-star,mask-rom-path="$SHELL_FOLDER/output/mask_rom/mask_rom.bin",canbus=canbus0 \
-m 1G \
-smp 8 \
-global quard-star-syscon.boot-cfg="$BOOTCFG" \
-global quard-star-syscon.update-cfg="$UPDATECFG" \
-drive if=pflash,bus=0,unit=0,format=raw,file=$SHELL_FOLDER/output/fw/pflash.img,id=mtd0 \
-drive if=mtd,bus=0,unit=0,format=raw,file=$SHELL_FOLDER/output/fw/norflash.img,id=mtd1 \
-drive if=mtd,bus=1,unit=0,format=raw,file=$SHELL_FOLDER/output/fw/nandflash.img,id=mtd2 \
-drive if=none,format=raw,file=$SHELL_FOLDER/output/fw/usb.img,id=usb0 \
-drive if=sd,format=raw,file=$SHELL_FOLDER/output/fw/sd.img,id=sd0 \
-drive if=none,format=raw,file=$SHELL_FOLDER/output/rootfs/rootfs.img,id=disk0 \
-chardev socket,telnet=on,host=127.0.0.1,port=3450,server=on,wait=off,id=usb1 \
-object can-bus,id=canbus0 $HOST_VCAN_PARAM \
$NETDEV0_PARAM $NETDEV1_PARAM \
$AUDIO_PARAM \
-net nic,netdev=net0 \
-device usb-storage,drive=usb0 \
-device usb-serial,always-plugged=true,chardev=usb1 \
-device wm8750,audiodev=audio0 \
-fw_cfg name="opt/qemu_cmdline",string="qemu_vc=$DEFAULT_V" \
-fsdev local,security_model=mapped-xattr,path=$SHELL_FOLDER,id=fsdev0 \
-global virtio-mmio.force-legacy=false \
-device virtio-blk-device,drive=disk0,id=hd0 \
-device virtio-9p-device,fsdev=fsdev0,mount_tag=hostshare,id=fs0 \
-device virtio-net-device,netdev=net1 \
$VIRTIO_GPU \
-device virtio-mouse-device,id=input0 \
-device virtio-keyboard-device,id=input1 \
$GRAPHIC_PARAM $FULL_SCREEN $DEBUG_PARAM $PLUGINS_PARAM $TARGET_GDB_PARAM

###############################################################################
