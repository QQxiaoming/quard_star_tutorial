#!/bin/bash
losetup -o 0 --sizelimit 1073741824 /dev/loop70 $1/rootfs.img -P
if [ -d "$1/target" ]; then  
rm -rf $1/target
fi
mkdir $1/target
mkdir $1/target/bootfs
mkdir $1/target/rootfs
mount /dev/loop70p1 $1/target/bootfs 
mount /dev/loop70p2 $1/target/rootfs

cp -r $1/bootfs/* $1/target/bootfs/
cp -r $1/rootfs/* $1/target/rootfs/
sync

umount $1/target/bootfs 
umount $1/target/rootfs
losetup -d /dev/loop70  
sync