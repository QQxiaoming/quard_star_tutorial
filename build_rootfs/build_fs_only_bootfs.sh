#!/bin/bash

run_linux()
{
    LOOPDEV="$(losetup -f)"
    losetup -o 0 --sizelimit 4294967296 $LOOPDEV $1/rootfs.img -P
    if [ -d "$1/target" ]; then  
    rm -rf $1/target
    fi
    mkdir $1/target
    mkdir $1/target/bootfs
    mount "$LOOPDEV"p1 $1/target/bootfs 

    cp -r $1/bootfs/* $1/target/bootfs/
    sync
    echo "please wait 5s"
    sleep 5

    umount $1/target/bootfs 
    losetup -d $LOOPDEV
    sync
}

run_macos()
{
    LOOPDEV="$(hdiutil attach -nomount $1/rootfs.img | awk -F ' ' '{ print $1 }' | awk 'NR==1')"
    if [ -d "$1/target" ]; then  
    rm -rf $1/target
    fi
    mkdir $1/target
    mkdir $1/target/bootfs
    mount -t msdos "$LOOPDEV"s1 $1/target/bootfs 

    cp -r $1/bootfs/* $1/target/bootfs/
    sync
    echo "please wait 5s"
    sleep 5

    umount $1/target/bootfs 
    hdiutil detach $LOOPDEV
    sync
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

