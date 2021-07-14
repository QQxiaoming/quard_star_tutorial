SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)

VC=\
"1920x1080 | \
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

if [ $# != 1 ] ; then
	if [ $# != 2 ] ; then
		echo "usage $0 [graphic | nographic] [$VC]"
		exit 1
	fi
fi

case "$1" in
graphic)
	DEFAULT_VC=$2
	if [ $# != 2 ] ; then
		DEFAULT_VC="1280x720"
	fi
	GRAPHIC_PARAM="--display gtk --serial vc:$DEFAULT_VC --serial vc:$DEFAULT_VC --serial vc:$DEFAULT_VC --parallel none --monitor vc:$DEFAULT_VC"
	
	WIDTH="$(echo $DEFAULT_VC | sed 's/\(.*\)x\(.*\)/\1/g')"
	HEIGHT="$(echo $DEFAULT_VC | sed 's/\(.*\)x\(.*\)/\2/g')"
	ROWS="$(echo $WIDTH / 8 |bc)"
	COLS="$(echo $HEIGHT / 16 |bc)"
	DEFAULT_V=":vn:$COLS""x""$ROWS:"
	;;
nographic)
	DEFAULT_VN="$(stty size | sed '/ \+/s//x/g')" 
    GRAPHIC_PARAM="-nographic --parallel none"
	DEFAULT_V=":vn:$DEFAULT_VN:"
    ;;
--help)
	echo "usage $0 [graphic | nographic] [$VC]"
	exit 0
	;;
*)
	echo "usage $0 [graphic | nographic] [$VC]"
	exit 1	
	;;
esac

$SHELL_FOLDER/output/qemu/bin/qemu-system-riscv64 \
-M quard-star \
-m 1G \
-smp 8 \
-drive if=pflash,bus=0,unit=0,format=raw,file=$SHELL_FOLDER/output/fw/fw.bin \
-drive file=$SHELL_FOLDER/output/rootfs/rootfs.img,format=raw,id=hd0 \
-global virtio-mmio.force-legacy=false \
-device virtio-blk-device,drive=hd0 \
-device virtio-gpu-device,id=video0,xres=1280,yres=720 \
-device virtio-mouse-device \
-device virtio-keyboard-device \
-fsdev local,security_model=passthrough,id=fsdev0,path=$SHELL_FOLDER/target_root_app \
-device virtio-9p-device,id=fs0,fsdev=fsdev0,mount_tag=hostshare \
-fw_cfg name="opt/qemu_cmdline",string="qemu_vc="$DEFAULT_V"" \
$GRAPHIC_PARAM
