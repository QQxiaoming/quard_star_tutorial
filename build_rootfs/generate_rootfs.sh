#!/bin/bash
LOOPDEV="$(losetup -f)"

echo "label: dos"                                           > sfdisk
echo "label-id: 0x32a47f23"                                >> sfdisk
echo "device: $LOOPDEV"                                    >> sfdisk
echo "unit: sectors"                                       >> sfdisk
echo -n "$LOOPDEV"                                         >> sfdisk
echo "p1 : start=        2048, size=      196608, type=c"  >> sfdisk
echo -n "$LOOPDEV"                                         >> sfdisk
echo "p2 : start=      198656, size=     3995648, type=83" >> sfdisk

losetup -P -o 0 --sizelimit 2147483648 $LOOPDEV $1
echo -e "I\nsfdisk\nw\n" | fdisk $LOOPDEV
losetup -d $LOOPDEV
rm sfdisk
sync
echo "please wait 5s"
sleep 5
losetup -P -o 0 --sizelimit 2147483648 $LOOPDEV $1
mkfs.vfat "$LOOPDEV"p1
mkfs.ext4 "$LOOPDEV"p2
losetup -d $LOOPDEV 
sync
