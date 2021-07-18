SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
REBUILD_ROOTFS=$1
CROSS_COMPILE_DIR=/opt/riscv64--glibc--bleeding-edge-2020.08-1
CROSS_PREFIX=$CROSS_COMPILE_DIR/bin/riscv64-linux

# 编译qemu
cd $SHELL_FOLDER/qemu-6.0.0
if [ ! -d "$SHELL_FOLDER/output/qemu" ]; then  
./configure --prefix=$SHELL_FOLDER/output/qemu  --target-list=riscv64-softmmu --enable-gtk  --enable-virtfs --disable-gio
fi  
make -j16
make install

# 编译lowlevelboot
if [ ! -d "$SHELL_FOLDER/output/lowlevelboot" ]; then  
mkdir $SHELL_FOLDER/output/lowlevelboot
fi  
cd $SHELL_FOLDER/lowlevelboot
$CROSS_PREFIX-gcc -x assembler-with-cpp -c startup.s -o $SHELL_FOLDER/output/lowlevelboot/startup.o
$CROSS_PREFIX-gcc -nostartfiles -T./boot.lds -Wl,-Map=$SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.map -Wl,--gc-sections $SHELL_FOLDER/output/lowlevelboot/startup.o -o $SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.elf
$CROSS_PREFIX-objcopy -O binary -S $SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.elf $SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.bin
$CROSS_PREFIX-objdump --source --demangle --disassemble --reloc --wide $SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.elf > $SHELL_FOLDER/output/lowlevelboot/lowlevel_fw.lst

# 编译opensbi
if [ ! -d "$SHELL_FOLDER/output/opensbi" ]; then  
mkdir $SHELL_FOLDER/output/opensbi
fi  
cd $SHELL_FOLDER/opensbi-0.9
make CROSS_COMPILE=$CROSS_PREFIX- PLATFORM=quard_star
cp -r $SHELL_FOLDER/opensbi-0.9/build/platform/quard_star/firmware/fw_jump.bin $SHELL_FOLDER/output/opensbi/fw_jump.bin
cp -r $SHELL_FOLDER/opensbi-0.9/build/platform/quard_star/firmware/fw_jump.elf $SHELL_FOLDER/output/opensbi/fw_jump.elf
$CROSS_PREFIX-objdump --source --demangle --disassemble --reloc --wide $SHELL_FOLDER/output/opensbi/fw_jump.elf > $SHELL_FOLDER/output/opensbi/fw_jump.lst

# 生成sbi.dtb
cd $SHELL_FOLDER/dts
dtc -I dts -O dtb -o $SHELL_FOLDER/output/opensbi/quard_star_sbi.dtb quard_star_sbi.dts

# 编译trusted_domain
if [ ! -d "$SHELL_FOLDER/output/trusted_domain" ]; then  
mkdir $SHELL_FOLDER/output/trusted_domain
fi  
cd $SHELL_FOLDER/trusted_domain
make CROSS_COMPILE=$CROSS_PREFIX- clean
make CROSS_COMPILE=$CROSS_PREFIX- -j16
cp ./build/trusted_fw.* $SHELL_FOLDER/output/trusted_domain/

# 编译uboot
if [ ! -d "$SHELL_FOLDER/output/uboot" ]; then  
mkdir $SHELL_FOLDER/output/uboot
fi  
cd $SHELL_FOLDER/u-boot-2021.07
make CROSS_COMPILE=$CROSS_PREFIX- qemu-quard-star_defconfig
make CROSS_COMPILE=$CROSS_PREFIX- -j16
cp $SHELL_FOLDER/u-boot-2021.07/u-boot $SHELL_FOLDER/output/uboot/u-boot.elf
cp $SHELL_FOLDER/u-boot-2021.07/u-boot.map $SHELL_FOLDER/output/uboot/u-boot.map
cp $SHELL_FOLDER/u-boot-2021.07/u-boot.bin $SHELL_FOLDER/output/uboot/u-boot.bin
$CROSS_PREFIX-objdump --source --demangle --disassemble --reloc --wide $SHELL_FOLDER/output/uboot/u-boot.elf > $SHELL_FOLDER/output/uboot/u-boot.lst

# 生成uboot.dtb
cd $SHELL_FOLDER/dts
dtc -I dts -O dtb -o $SHELL_FOLDER/output/uboot/quard_star_uboot.dtb quard_star_uboot.dts

# 合成firmware固件
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
if [ ! -d "$SHELL_FOLDER/output/linux_kernel" ]; then  
mkdir $SHELL_FOLDER/output/linux_kernel
fi  
cd $SHELL_FOLDER/linux-5.10.42
make ARCH=riscv CROSS_COMPILE=$CROSS_PREFIX- quard_star_defconfig
make ARCH=riscv CROSS_COMPILE=$CROSS_PREFIX- -j16
cp $SHELL_FOLDER/linux-5.10.42/arch/riscv/boot/Image $SHELL_FOLDER/output/linux_kernel/Image

# 编译busybox-1.33.1
if [ ! -d "$SHELL_FOLDER/output/busybox" ]; then  
mkdir $SHELL_FOLDER/output/busybox
fi  
cd $SHELL_FOLDER/busybox-1.33.1
make ARCH=riscv CROSS_COMPILE=$CROSS_PREFIX- quard_star_defconfig
make ARCH=riscv CROSS_COMPILE=$CROSS_PREFIX- -j16
make ARCH=riscv CROSS_COMPILE=$CROSS_PREFIX- install

# 合成文件系统映像
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
    cp $CROSS_COMPILE_DIR/riscv64-buildroot-linux-gnu/sysroot/lib/* $TARGET_ROOTFS_DIR/lib/
    cp $CROSS_COMPILE_DIR/riscv64-buildroot-linux-gnu/sysroot/usr/bin/* $TARGET_ROOTFS_DIR/usr/bin/
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

cd $SHELL_FOLDER
