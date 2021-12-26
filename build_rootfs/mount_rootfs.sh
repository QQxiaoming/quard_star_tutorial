#!/bin/bash

USAGE="usage $0 [mount | umount] [DIR(./output/rootfs)]"

if [ $# != 2 ] ; then
    echo $USAGE
    exit 1
fi

TARGET_DIR=$2

mount_fs()
{
    losetup -o 0 --sizelimit 1073741824 /dev/loop70 $TARGET_DIR/rootfs.img -P
    if [ -d "$TARGET_DIR/target" ]; then  
    rm -rf $TARGET_DIR/target
    fi
    mkdir $TARGET_DIR/target
    mkdir $TARGET_DIR/target/bootfs
    mkdir $TARGET_DIR/target/rootfs
    mount /dev/loop70p1 $TARGET_DIR/target/bootfs 
    mount /dev/loop70p2 $TARGET_DIR/target/rootfs
}

umount_fs()
{
    umount $TARGET_DIR/target/bootfs 
    umount $TARGET_DIR/target/rootfs
    losetup -d /dev/loop70  
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