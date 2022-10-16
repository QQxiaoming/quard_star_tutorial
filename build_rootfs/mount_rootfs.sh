#!/bin/bash

USAGE="usage $0 [mount | umount] [LOOPDEV(/dev/loop70)] [DIR(./output/rootfs)]"

if [ $# != 3 ] ; then
    echo $USAGE
    exit 1
fi

LOOPDEV=$2
TARGET_DIR=$3

mount_fs()
{
    losetup -o 0 --sizelimit 2147483648 $LOOPDEV $TARGET_DIR/rootfs.img -P
    if [ -d "$TARGET_DIR/target" ]; then  
    rm -rf $TARGET_DIR/target
    fi
    mkdir $TARGET_DIR/target
    mkdir $TARGET_DIR/target/bootfs
    mkdir $TARGET_DIR/target/rootfs
    mount "$LOOPDEV"p1 $TARGET_DIR/target/bootfs 
    mount "$LOOPDEV"p2 $TARGET_DIR/target/rootfs
}

umount_fs()
{
    umount $TARGET_DIR/target/bootfs 
    umount $TARGET_DIR/target/rootfs
    losetup -d $LOOPDEV
}

case "$1" in
mount)
    mount_fs
	;;
umount)
    umount_fs
    ;;
*)
	echo $USAGE
	exit 1	
	;;
esac