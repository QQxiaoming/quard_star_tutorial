#!/bin/bash

run_linux()
{
    LOOPDEV="$(losetup -f)"
    losetup -o 0 --sizelimit 2147483648 $LOOPDEV $1/rootfs.img -P
    if [ -d "$1/target" ]; then  
    rm -rf $1/target
    fi
    mkdir $1/target
    mkdir $1/target/bootfs
    mkdir $1/target/rootfs
    mount "$LOOPDEV"p1 $1/target/bootfs 
    mount "$LOOPDEV"p2 $1/target/rootfs

    cp -r $1/bootfs/* $1/target/bootfs/
    cp -r $1/rootfs/* $1/target/rootfs/
    sync
    echo "please wait 5s"
    sleep 5

    umount $1/target/bootfs 
    umount $1/target/rootfs
    losetup -d $LOOPDEV
    sync
}

run_macos()
{
    echo "MacOS do not support ext4!"
}

UNAMEOUT="$(uname -s)"
case "${UNAMEOUT}" in
    Linux*)    
        run_linux $1
        ;;
    Darwin*)    
        run_macos $1
        ;;
esac
