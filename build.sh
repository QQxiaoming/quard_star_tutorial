SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
PROCESSORS=`cat /proc/cpuinfo |grep "processor"|wc -l`
REBUILD_ROOTFS=$1
GLIB_ELF_CROSS_COMPILE_DIR=/opt/gcc-riscv64-unknown-linux-gnu
GLIB_ELF_CROSS_PREFIX=$GLIB_ELF_CROSS_COMPILE_DIR/bin/riscv64-unknown-linux-gnu
GLIB_ELF_CROSS_PREFIX_SYSROOT_DIR=$GLIB_ELF_CROSS_COMPILE_DIR/sysroot
NEWLIB_ELF_CROSS_COMPILE_DIR=/opt/gcc-riscv64-unknown-elf
NEWLIB_ELF_CROSS_PREFIX=$NEWLIB_ELF_CROSS_COMPILE_DIR/bin/riscv64-unknown-elf

# 编译qemu
echo "\033[1;4;41;32m编译qemu\033[0m"
cd $SHELL_FOLDER/qemu-6.0.0
if [ ! -d "$SHELL_FOLDER/output/qemu" ]; then  
./configure --prefix=$SHELL_FOLDER/output/qemu  --target-list=riscv64-softmmu --enable-gtk  --enable-virtfs --disable-gio
fi  
make -j$PROCESSORS
make install

# 编译lowlevelboot
echo "\033[1;4;41;32m编译lowlevelboot\033[0m"
if [ ! -d "$SHELL_FOLDER/output/lowlevelboot" ]; then  
mkdir $SHELL_FOLDER/output/lowlevelboot
fi  
cd $SHELL_FOLDER/lowlevelboot
$NEWLIB_ELF_CROSS_PREFIX-gcc -x assembler-with-cpp -c startup.s -o $SHELL_FOLDER/output/lowlevelboot/startup.o
$NEWLIB_ELF_CROSS_PREFIX-gcc -nostartfiles -T./boot.lds -Wl,-Map=$SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.map -Wl,--gc-sections $SHELL_FOLDER/output/lowlevelboot/startup.o -o $SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.elf
$NEWLIB_ELF_CROSS_PREFIX-objcopy -O binary -S $SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.elf $SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.bin
$NEWLIB_ELF_CROSS_PREFIX-objdump --source --demangle --disassemble --reloc --wide $SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.elf > $SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.lst

# 编译opensbi
echo "\033[1;4;41;32m编译opensbi\033[0m"
if [ ! -d "$SHELL_FOLDER/output/opensbi" ]; then  
mkdir $SHELL_FOLDER/output/opensbi
fi  
cd $SHELL_FOLDER/opensbi-0.9
make CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- PLATFORM=quard_star
cp -r $SHELL_FOLDER/opensbi-0.9/build/platform/quard_star/firmware/fw_jump.bin $SHELL_FOLDER/output/opensbi/fw_jump.bin
cp -r $SHELL_FOLDER/opensbi-0.9/build/platform/quard_star/firmware/fw_jump.elf $SHELL_FOLDER/output/opensbi/fw_jump.elf
$GLIB_ELF_CROSS_PREFIX-objdump --source --demangle --disassemble --reloc --wide $SHELL_FOLDER/output/opensbi/fw_jump.elf > $SHELL_FOLDER/output/opensbi/fw_jump.lst

# 生成sbi.dtb
echo "\033[1;4;41;32m生成sbi.dtb\033[0m"
cd $SHELL_FOLDER/dts
dtc -I dts -O dtb -o $SHELL_FOLDER/output/opensbi/quard_star_sbi.dtb quard_star_sbi.dts

# 编译trusted_domain
echo "\033[1;4;41;32m编译trusted_domain\033[0m"
if [ ! -d "$SHELL_FOLDER/output/trusted_domain" ]; then  
mkdir $SHELL_FOLDER/output/trusted_domain
fi  
cd $SHELL_FOLDER/trusted_domain
make CROSS_COMPILE=$NEWLIB_ELF_CROSS_PREFIX- clean
make CROSS_COMPILE=$NEWLIB_ELF_CROSS_PREFIX- -j$PROCESSORS
cp ./build/trusted_fw.* $SHELL_FOLDER/output/trusted_domain/

# 编译uboot
echo "\033[1;4;41;32m编译uboot\033[0m"
if [ ! -d "$SHELL_FOLDER/output/uboot" ]; then  
mkdir $SHELL_FOLDER/output/uboot
fi  
cd $SHELL_FOLDER/u-boot-2021.07
make CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- qemu-quard-star_defconfig
make CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- -j$PROCESSORS
cp $SHELL_FOLDER/u-boot-2021.07/u-boot $SHELL_FOLDER/output/uboot/u-boot.elf
cp $SHELL_FOLDER/u-boot-2021.07/u-boot.map $SHELL_FOLDER/output/uboot/u-boot.map
cp $SHELL_FOLDER/u-boot-2021.07/u-boot.bin $SHELL_FOLDER/output/uboot/u-boot.bin
$GLIB_ELF_CROSS_PREFIX-objdump --source --demangle --disassemble --reloc --wide $SHELL_FOLDER/output/uboot/u-boot.elf > $SHELL_FOLDER/output/uboot/u-boot.lst

# 生成uboot.dtb
echo "\033[1;4;41;32m生成uboot.dtb\033[0m"
cd $SHELL_FOLDER/dts
dtc -I dts -O dtb -o $SHELL_FOLDER/output/uboot/quard_star_uboot.dtb quard_star_uboot.dts

# 合成firmware固件
echo "\033[1;4;41;32m合成firmware固件\033[0m"
if [ ! -d "$SHELL_FOLDER/output/fw" ]; then  
mkdir $SHELL_FOLDER/output/fw
fi  
cd $SHELL_FOLDER/output/fw
rm -rf fw.bin
dd of=fw.bin bs=1k count=32k if=/dev/zero
dd of=fw.bin bs=1k conv=notrunc seek=0 if=$SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.bin
dd of=fw.bin bs=1k conv=notrunc seek=512 if=$SHELL_FOLDER/output/opensbi/quard_star_sbi.dtb
dd of=fw.bin bs=1k conv=notrunc seek=1K if=$SHELL_FOLDER/output/uboot/quard_star_uboot.dtb
dd of=fw.bin bs=1k conv=notrunc seek=2K if=$SHELL_FOLDER/output/opensbi/fw_jump.bin
dd of=fw.bin bs=1k conv=notrunc seek=4K if=$SHELL_FOLDER/output/trusted_domain/trusted_fw.bin
dd of=fw.bin bs=1k conv=notrunc seek=8K if=$SHELL_FOLDER/output/uboot/u-boot.bin

# 编译linux kernel
echo "\033[1;4;41;32m编译linux kernel\033[0m"
if [ ! -d "$SHELL_FOLDER/output/linux_kernel" ]; then  
mkdir $SHELL_FOLDER/output/linux_kernel
fi  
cd $SHELL_FOLDER/linux-5.10.42
make ARCH=riscv CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- quard_star_defconfig
make ARCH=riscv CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- -j$PROCESSORS
cp $SHELL_FOLDER/linux-5.10.42/arch/riscv/boot/Image $SHELL_FOLDER/output/linux_kernel/Image

# 编译busybox
echo "\033[1;4;41;32m编译busybox\033[0m"
if [ ! -d "$SHELL_FOLDER/output/busybox" ]; then  
mkdir $SHELL_FOLDER/output/busybox
fi  
cd $SHELL_FOLDER/busybox-1.33.1
make ARCH=riscv CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- quard_star_defconfig
make ARCH=riscv CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- -j$PROCESSORS
make ARCH=riscv CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- install

# 合成文件系统映像
echo "\033[1;4;41;32m合成文件系统映像\033[0m"
MAKE_ROOTFS_DIR=$SHELL_FOLDER/output/rootfs
TARGET_ROOTFS_DIR=$MAKE_ROOTFS_DIR/rootfs
TARGET_BOOTFS_DIR=$MAKE_ROOTFS_DIR/bootfs
if [ ! -d "$MAKE_ROOTFS_DIR" ]; then
mkdir $MAKE_ROOTFS_DIR
REBUILD_ROOTFS="all"
fi
if [ ! -f "$MAKE_ROOTFS_DIR/rootfs.img" ]; then  
REBUILD_ROOTFS="all"
fi
case "$REBUILD_ROOTFS" in
all)
    if [ ! -d "$MAKE_ROOTFS_DIR" ]; then  
    mkdir $MAKE_ROOTFS_DIR
    fi
    if [ ! -d "$TARGET_ROOTFS_DIR" ]; then  
    mkdir $TARGET_ROOTFS_DIR
    fi
    if [ ! -d "$TARGET_BOOTFS_DIR" ]; then  
    mkdir $TARGET_BOOTFS_DIR
    fi
    cd $MAKE_ROOTFS_DIR
    if [ ! -f "$MAKE_ROOTFS_DIR/rootfs.img" ]; then  
    dd if=/dev/zero of=rootfs.img bs=1M count=1024
    pkexec $SHELL_FOLDER/build_rootfs/generate_rootfs.sh $MAKE_ROOTFS_DIR/rootfs.img $SHELL_FOLDER/build_rootfs/sfdisk
    fi
    cp $SHELL_FOLDER/output/linux_kernel/Image $TARGET_BOOTFS_DIR/Image
    cp $SHELL_FOLDER/output/uboot/quard_star_uboot.dtb $TARGET_BOOTFS_DIR/quard_star.dtb
    $SHELL_FOLDER/u-boot-2021.07/tools/mkimage -A riscv -O linux -T script -C none -a 0 -e 0 -n "Distro Boot Script" -d $SHELL_FOLDER/dts/quard_star_uboot.cmd $TARGET_BOOTFS_DIR/boot.scr
    cp -r $SHELL_FOLDER/output/busybox/* $TARGET_ROOTFS_DIR/
    cp -r $SHELL_FOLDER/target_root_script/* $TARGET_ROOTFS_DIR/
    if [ ! -d "$TARGET_ROOTFS_DIR/proc" ]; then  
    mkdir $TARGET_ROOTFS_DIR/proc
    fi
    if [ ! -d "$TARGET_ROOTFS_DIR/sys" ]; then  
    mkdir $TARGET_ROOTFS_DIR/sys
    fi
    if [ ! -d "$TARGET_ROOTFS_DIR/dev" ]; then  
    mkdir $TARGET_ROOTFS_DIR/dev
    fi
    if [ ! -d "$TARGET_ROOTFS_DIR/tmp" ]; then  
    mkdir $TARGET_ROOTFS_DIR/tmp
    fi
    if [ ! -d "$TARGET_ROOTFS_DIR/mnt" ]; then  
    mkdir $TARGET_ROOTFS_DIR/mnt
    fi
    if [ ! -d "$TARGET_ROOTFS_DIR/lib" ]; then  
    mkdir $TARGET_ROOTFS_DIR/lib
    cd $TARGET_ROOTFS_DIR
    ln -s ./lib ./lib64
    cd $MAKE_ROOTFS_DIR
    fi
    cp -r $GLIB_ELF_CROSS_PREFIX_SYSROOT_DIR/lib/* $TARGET_ROOTFS_DIR/lib/
    cp -r $GLIB_ELF_CROSS_PREFIX_SYSROOT_DIR/usr/bin/* $TARGET_ROOTFS_DIR/usr/bin/
    pkexec $SHELL_FOLDER/build_rootfs/build.sh $MAKE_ROOTFS_DIR
    ;;
bootfs)
    cp $SHELL_FOLDER/output/linux_kernel/Image $TARGET_BOOTFS_DIR/Image
    cp $SHELL_FOLDER/output/uboot/quard_star_uboot.dtb $TARGET_BOOTFS_DIR/quard_star.dtb
    $SHELL_FOLDER/u-boot-2021.07/tools/mkimage -A riscv -O linux -T script -C none -a 0 -e 0 -n "Distro Boot Script" -d $SHELL_FOLDER/dts/quard_star_uboot.cmd $TARGET_BOOTFS_DIR/boot.scr
    pkexec $SHELL_FOLDER/build_rootfs/build_only_bootfs.sh $MAKE_ROOTFS_DIR
    ;;
*)
    echo "skip build rootfs.img!"
	;;
esac

echo "\033[1;4;41;32m完成\033[0m"
