SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)

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

USAGE="usage $0 [$MODE] [$VC]"

if [ $# != 1 ] ; then
	if [ $# != 2 ] ; then
		echo $USAGE
		exit 1
	fi
fi

if [ $# != 2 ] ; then
	DEFAULT_VC="1280x720"
else
	case "$2" in
	full-screen)
		DEFAULT_VC="$(xrandr -q 2>/dev/null | awk '/*/{print $1}')"
		FULL_SCREEN=-full-screen
		;;
	*)
		DEFAULT_VC=$2
		;;
	esac
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
	GRAPHIC_PARAM="--display gtk,zoom-to-fit=false --serial telnet::3441,server,nowait --serial telnet::3442,server,nowait --serial telnet::3443,server,nowait --monitor stdio --parallel none"
	DEFAULT_V=":vn:24x80:"
	;;
customize3)
	GRAPHIC_PARAM="--display gtk,zoom-to-fit=false --serial telnet::3441,server,nowait --serial telnet::3442,server,nowait --serial telnet::3443,server,nowait --monitor none --parallel none"
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

$SHELL_FOLDER/output/qemu/bin/qemu-system-riscv64 \
-M quard-star \
-m 1G \
-smp 8 \
-drive if=pflash,bus=0,unit=0,format=raw,file=$SHELL_FOLDER/output/fw/fw.bin \
-drive file=$SHELL_FOLDER/output/rootfs/rootfs.img,format=raw,id=drive0 \
-fsdev local,security_model=mapped-xattr,id=fsdev0,path=$SHELL_FOLDER \
-global virtio-mmio.force-legacy=false \
-device virtio-blk-device,id=hd0,drive=drive0 \
-device virtio-gpu-device,id=video0,xres=$WIDTH,yres=$HEIGHT \
-device virtio-mouse-device,id=input0 \
-device virtio-keyboard-device,id=input1 \
-device virtio-9p-device,id=fs0,fsdev=fsdev0,mount_tag=hostshare \
-fw_cfg name="opt/qemu_cmdline",string="qemu_vc="$DEFAULT_V"" \
$GRAPHIC_PARAM $FULL_SCREEN
