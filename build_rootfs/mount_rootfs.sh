#!/bin/bash

UNAMEOUT="$(uname -s)"
USAGE="usage $0 [mount | umount] [LOOPDEV(Auto or Linux:/dev/loop70 or macOS:/dev/disk4)] [DIR(./output/rootfs)]"

if [ $# != 3 ] ; then
    echo $USAGE
    exit 1
fi

LOOPDEV=$2
TARGET_DIR=$3

mount_fs()
{
    case "${UNAMEOUT}" in
        Linux*)    
            LOOPDEV="$(losetup -f)"
            losetup -o 0 --sizelimit 2147483648 $LOOPDEV $TARGET_DIR/rootfs.img -P
            ;;
        Darwin*)    
            LOOPDEV="$(hdiutil attach -nomount $TARGET_DIR/rootfs.img | awk -F ' ' '{ print $1 }' | awk 'NR==1')"
            ;;
    esac
    if [ -d "$TARGET_DIR/target" ]; then  
    rm -rf $TARGET_DIR/target
    fi
    mkdir $TARGET_DIR/target
    mkdir $TARGET_DIR/target/bootfs
    mkdir $TARGET_DIR/target/rootfs
    case "${UNAMEOUT}" in
        Linux*)    
            mount "$LOOPDEV"p1 $TARGET_DIR/target/bootfs 
            mount "$LOOPDEV"p2 $TARGET_DIR/target/rootfs
            ;;
        Darwin*)    
            mount -t msdos "$LOOPDEV"s1 $TARGET_DIR/target/bootfs 
            ext4fuse "$LOOPDEV"s2 $TARGET_DIR/target/rootfs
            ;;
    esac
    echo "lo setup $LOOPDEV success!"
}

umount_fs()
{
    umount $TARGET_DIR/target/bootfs 
    umount $TARGET_DIR/target/rootfs
    case "${UNAMEOUT}" in
        Linux*)    
            losetup -d $LOOPDEV
            ;;
        Darwin*)    
            hdiutil detach $LOOPDEV
            ;;
    esac
    echo "lo setdown $LOOPDEV success!"
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