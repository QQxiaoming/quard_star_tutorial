#!/bin/bash

USAGE="usage $0 [mount | umount] [LOOPDEV(/dev/disk4)] [DIR(./output/rootfs)]"

if [ $# != 3 ] ; then
    echo $USAGE
    exit 1
fi

LOOPDEV=$2
TARGET_DIR=$3

mount_fs()
{
    hdiutil attach -nomount $TARGET_DIR/rootfs.img
    if [ -d "$TARGET_DIR/target" ]; then  
    rm -rf $TARGET_DIR/target
    fi
    mkdir $TARGET_DIR/target
    mkdir $TARGET_DIR/target/bootfs
    mkdir $TARGET_DIR/target/rootfs
    mount -t msdos "$LOOPDEV"s1 $TARGET_DIR/target/bootfs 
    ext4fuse "$LOOPDEV"s2 $TARGET_DIR/target/rootfs
}

umount_fs()
{
    umount $TARGET_DIR/target/bootfs 
    umount $TARGET_DIR/target/rootfs
    hdiutil detach $LOOPDEV
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