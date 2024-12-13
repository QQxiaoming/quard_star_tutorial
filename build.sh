#!/bin/bash
set -e

PERMISSION_TOOL=""
UNAMEOUT="$(uname -s)"
case "${UNAMEOUT}" in
    Linux*)    
        HOST_USER_NAME="$(whoami)" 
        case "${HOST_USER_NAME}" in
        root)
            PERMISSION_TOOL=""
            ;;
        *)
            PERMISSION_TOOL="pkexec"
            ;;
        esac
        PROCESSORS=$(< /proc/cpuinfo grep "processor" | wc -l)
        ;;
    Darwin*)    
        PROCESSORS=$(sysctl -n machdep.cpu.thread_count)
        ;;
    *)
        PROCESSORS=4
        ;;
esac
SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
if [ -z $TOOLCHAIN_ROOT_PATH ]; then
TOOLCHAIN_ROOT_PATH=/opt
fi
GLIB_ELF_CROSS_COMPILE_DIR=$TOOLCHAIN_ROOT_PATH/gcc-riscv64-unknown-linux-gnu
GLIB_ELF_CROSS_PREFIX=$GLIB_ELF_CROSS_COMPILE_DIR/bin/riscv64-unknown-linux-gnu
GLIB_ELF_CROSS_PREFIX_SYSROOT_DIR=$GLIB_ELF_CROSS_COMPILE_DIR/sysroot
NEWLIB_ELF_CROSS_COMPILE_DIR=$TOOLCHAIN_ROOT_PATH/gcc-riscv64-unknown-elf
NEWLIB_ELF_CROSS_PREFIX=$NEWLIB_ELF_CROSS_COMPILE_DIR/bin/riscv64-unknown-elf

BUILD_TARGET=$1
BUILD_ROOTFS_OPT=$2

if [ ! -d "$SHELL_FOLDER/output" ]; then  
mkdir $SHELL_FOLDER/output
fi  

build_qemu()
{
    echo "------------------------------ 编译qemu ------------------------------"
    cd $SHELL_FOLDER/qemu-8.2.8
    if [ ! -d "$SHELL_FOLDER/output/qemu" ]; then  
    ./configure --prefix=$SHELL_FOLDER/output/qemu --target-list=riscv64-softmmu --enable-gtk  --enable-virtfs --disable-gio --enable-plugins --audio-drv-list=pa,alsa,sdl,oss
    fi  
    make -j$PROCESSORS
    make install
}

build_qemu_w64()
{
    echo "---------------------------- 编译qemu_w64 ----------------------------"
    cd $SHELL_FOLDER/qemu-8.2.8
    if [ ! -d "$SHELL_FOLDER/output/qemu_w64" ]; then  
    ./configure --prefix=$SHELL_FOLDER/output/qemu_w64 --cross-prefix=x86_64-w64-mingw32- --target-list=riscv64-softmmu --enable-gtk --disable-gio
    fi  
    make -j$PROCESSORS
    make install
}

build_qemu_macos()
{
    echo "---------------------------- 编译qemu_macos ----------------------------"
    cd $SHELL_FOLDER/qemu-8.2.8
    if [ ! -d "$SHELL_FOLDER/output/qemu_macos" ]; then  
    ./configure --prefix=$SHELL_FOLDER/output/qemu_macos --target-list=riscv64-softmmu --enable-virtfs
    fi  
    make -j$PROCESSORS
    make install
}

build_qemu_android()
{
    echo "--------------------------- 编译qemu_android ---------------------------"
    cd $SHELL_FOLDER/qemu-8.2.8
    export NDK=/home/qqm/Android/Sdk/ndk/25.1.8937393
    export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
    export NDK_CROSS=$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin
    export TARGET=x86_64-linux-android
    export API=21
    export AR=$TOOLCHAIN/bin/llvm-ar
    export CC=$TOOLCHAIN/bin/$TARGET$API-clang
    export AS=$CC
    export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
    export LD=$TOOLCHAIN/bin/ld
    export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
    export STRIP=$TOOLCHAIN/bin/llvm-strip
    export SYSROOT=$NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot
    export PKG_CONFIG_PATH=$SYSROOT/python3/lib/pkgconfig
    export PKG_CONFIG_LIBDIR=$NDK_CROSS
    export PATH=$PATH:$NDK:$NDK_CROSS
    if [ ! -d "$SHELL_FOLDER/output/qemu_android" ]; then  
    ./configure \
        --cc=$CC \
        --cxx=$CXX \
        --prefix=$SHELL_FOLDER/output/qemu_android \
        --target-list=riscv64-softmmu --enable-virtfs
    fi  
    make -j$PROCESSORS
    make install
}

build_mask_rom()
{
    echo "---------------------------- 编译mask_rom ----------------------------"
    if [ ! -d "$SHELL_FOLDER/output/mask_rom" ]; then  
    mkdir $SHELL_FOLDER/output/mask_rom
    fi  
    cd $SHELL_FOLDER/mask_rom
    make CROSS_COMPILE=$NEWLIB_ELF_CROSS_PREFIX- clean
    make CROSS_COMPILE=$NEWLIB_ELF_CROSS_PREFIX- -j$PROCESSORS
    cp ./build/mask_rom.* $SHELL_FOLDER/output/mask_rom/
}

build_lowlevelboot()
{
    echo "-------------------------- 编译lowlevelboot --------------------------"
    if [ ! -d "$SHELL_FOLDER/output/lowlevelboot" ]; then  
    mkdir $SHELL_FOLDER/output/lowlevelboot
    fi  
    cd $SHELL_FOLDER/lowlevelboot
    make CROSS_COMPILE=$NEWLIB_ELF_CROSS_PREFIX- clean
    make CROSS_COMPILE=$NEWLIB_ELF_CROSS_PREFIX- -j$PROCESSORS
    cp ./build/lowlevelboot.* $SHELL_FOLDER/output/lowlevelboot/
}

build_opensbi()
{
    echo "---------------------------- 编译opensbi -----------------------------"
    if [ ! -d "$SHELL_FOLDER/output/opensbi" ]; then  
    mkdir $SHELL_FOLDER/output/opensbi
    fi  
    cd $SHELL_FOLDER/opensbi-1.2
    make CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- PLATFORM=quard_star
    cp -r $SHELL_FOLDER/opensbi-1.2/build/platform/quard_star/firmware/fw_jump.bin $SHELL_FOLDER/output/opensbi/fw_jump.bin
    cp -r $SHELL_FOLDER/opensbi-1.2/build/platform/quard_star/firmware/fw_jump.elf $SHELL_FOLDER/output/opensbi/fw_jump.elf
    $GLIB_ELF_CROSS_PREFIX-objdump --source --demangle --disassemble --reloc --wide $SHELL_FOLDER/output/opensbi/fw_jump.elf > $SHELL_FOLDER/output/opensbi/fw_jump.lst
}

build_sbi_dtb()
{
    echo "---------------------------- 生成sbi.dtb -----------------------------"
    cd $SHELL_FOLDER/dts
    cpp -nostdinc -I include -undef -x assembler-with-cpp quard_star_sbi.dts > quard_star_sbi.dtb.dts.tmp
    dtc -i $SHELL_FOLDER/dts -I dts -O dtb -o $SHELL_FOLDER/output/opensbi/quard_star_sbi.dtb quard_star_sbi.dtb.dts.tmp 
}

build_trusted_domain()
{
    echo "------------------------- 编译trusted_domain -------------------------"
    if [ ! -d "$SHELL_FOLDER/output/trusted_domain" ]; then  
    mkdir $SHELL_FOLDER/output/trusted_domain
    fi  
    cd $SHELL_FOLDER/trusted_domain
    make CROSS_COMPILE=$NEWLIB_ELF_CROSS_PREFIX- clean
    make CROSS_COMPILE=$NEWLIB_ELF_CROSS_PREFIX- -j$PROCESSORS
    cp ./build/trusted_fw.* $SHELL_FOLDER/output/trusted_domain/
}

build_uboot_dtb()
{
    echo "--------------------------- 生成uboot.dtb ----------------------------"
    if [ ! -d "$SHELL_FOLDER/output/uboot" ]; then  
    mkdir $SHELL_FOLDER/output/uboot
    fi  
    cd $SHELL_FOLDER/dts
    cpp -nostdinc -I include -undef -x assembler-with-cpp quard_star_uboot.dts > quard_star_uboot.dtb.dts.tmp
    dtc -I dts -O dtb -o $SHELL_FOLDER/output/uboot/quard_star_uboot.dtb quard_star_uboot.dtb.dts.tmp
    cpp -nostdinc -I include -undef -x assembler-with-cpp quard_star_uboot_kgdb.dts > quard_star_uboot_kgdb.dtb.dts.tmp
    dtc -I dts -O dtb -o $SHELL_FOLDER/output/uboot/quard_star_uboot_kgdb.dtb quard_star_uboot_kgdb.dtb.dts.tmp
    if [ -f "$SHELL_FOLDER/u-boot-2023.04/tools/mkimage" ]; then  
    $SHELL_FOLDER/u-boot-2023.04/tools/mkimage -A riscv -O linux -T script -C none -a 0 -e 0 -n "Distro Boot Script" -d $SHELL_FOLDER/dts/quard_star_uboot.cmd $SHELL_FOLDER/output/uboot/boot.scr
    fi  
}

build_uboot()
{
    echo "----------------------------- 编译uboot ------------------------------"
    if [ ! -d "$SHELL_FOLDER/output/uboot" ]; then  
    mkdir $SHELL_FOLDER/output/uboot
    fi  
    cd $SHELL_FOLDER/u-boot-2023.04
    make CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- qemu-quard-star_defconfig
    make CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- -j$PROCESSORS DEVICE_TREE=../../../../output/uboot/quard_star_uboot
    cp $SHELL_FOLDER/u-boot-2023.04/u-boot $SHELL_FOLDER/output/uboot/u-boot.elf
    cp $SHELL_FOLDER/u-boot-2023.04/u-boot.map $SHELL_FOLDER/output/uboot/u-boot.map
    cp $SHELL_FOLDER/u-boot-2023.04/u-boot.bin $SHELL_FOLDER/output/uboot/u-boot.bin
    $GLIB_ELF_CROSS_PREFIX-objdump --source --demangle --disassemble --reloc --wide $SHELL_FOLDER/output/uboot/u-boot.elf > $SHELL_FOLDER/output/uboot/u-boot.lst
    cd $SHELL_FOLDER/dts
    $SHELL_FOLDER/u-boot-2023.04/tools/mkimage -A riscv -O linux -T script -C none -a 0 -e 0 -n "Distro Boot Script" -d $SHELL_FOLDER/dts/quard_star_uboot.cmd $SHELL_FOLDER/output/uboot/boot.scr
}

build_firmware()
{
    echo "--------------------------- 合成firmware固件 ---------------------------"
    if [ ! -f "$SHELL_FOLDER/output/lowlevelboot/lowlevelboot.bin" ]; then  
        echo "not found lowlevelboot.bin, please ./build.sh lowlevelboot"
        exit 255
    fi
    if [ ! -f "$SHELL_FOLDER/output/opensbi/quard_star_sbi.dtb" ]; then  
        echo "not found quard_star_sbi.dtb, please ./build.sh sbi_dtb"
        exit 255
    fi
    if [ ! -f "$SHELL_FOLDER/output/uboot/quard_star_uboot.dtb" ]; then  
        echo "not found quard_star_uboot.dtb, please ./build.sh uboot_dtb"
        exit 255
    fi
    if [ ! -f "$SHELL_FOLDER/output/opensbi/fw_jump.bin" ]; then  
        echo "not found fw_jump.bin, please ./build.sh opensbi"
        exit 255
    fi
    if [ ! -f "$SHELL_FOLDER/output/trusted_domain/trusted_fw.bin" ]; then  
        echo "not found trusted_fw.bin, please ./build.sh trusted_domain"
        exit 255
    fi
    if [ ! -f "$SHELL_FOLDER/output/uboot/u-boot.bin" ]; then  
        echo "not found u-boot.bin, please ./build.sh uboot"
        exit 255
    fi
    if [ ! -d "$SHELL_FOLDER/output/fw" ]; then  
    mkdir $SHELL_FOLDER/output/fw
    fi  
    cd $SHELL_FOLDER/output/fw
    rm -rf fw.bin
    dd of=fw.bin bs=1k count=12k if=/dev/zero
    dd of=fw.bin bs=1k conv=notrunc seek=0 if=$SHELL_FOLDER/output/lowlevelboot/lowlevelboot.bin
    dd of=fw.bin bs=1k conv=notrunc seek=512 if=$SHELL_FOLDER/output/opensbi/quard_star_sbi.dtb
    dd of=fw.bin bs=1k conv=notrunc seek=1K if=$SHELL_FOLDER/output/uboot/quard_star_uboot.dtb
    dd of=fw.bin bs=1k conv=notrunc seek=2K if=$SHELL_FOLDER/output/opensbi/fw_jump.bin
    dd of=fw.bin bs=1k conv=notrunc seek=4K if=$SHELL_FOLDER/output/trusted_domain/trusted_fw.bin
    dd of=fw.bin bs=1k conv=notrunc seek=8K if=$SHELL_FOLDER/output/uboot/u-boot.bin
    
    rm -rf pflash.img
    dd bs=1k count=32k if=/dev/zero | tr '\000' '\377' > pflash.img
    dd of=pflash.img bs=1k conv=notrunc seek=0 if=fw.bin

    if [ ! -f "$SHELL_FOLDER/output/fw/norflash.img" ]; then  
        dd bs=1k count=32k if=/dev/zero | tr '\000' '\377' > norflash.img
        rm -rf jff2s_dir jffs2.bin
        mkdir jff2s_dir
        mkfs.jffs2 -lqn -e64 -s320 -p0x1400000 -r jff2s_dir -o jffs2.bin
        dd of=norflash.img bs=1k conv=notrunc seek=12K if=jffs2.bin
        rm -rf jff2s_dir jffs2.bin
    fi
    dd of=norflash.img bs=1k conv=notrunc seek=0 if=fw.bin

    if [ ! -f "$SHELL_FOLDER/output/fw/sd.img" ]; then  
        dd bs=1k count=32k if=/dev/zero | tr '\000' '\377' > sd.img
    fi
    dd of=sd.img bs=1k conv=notrunc seek=0 if=fw.bin

    if [ ! -f "$SHELL_FOLDER/output/fw/usb.img" ]; then  
        dd bs=1k count=32k if=/dev/zero | tr '\000' '\377' > usb.img
    fi

    if [ ! -f "$SHELL_FOLDER/output/fw/nandflash.img" ]; then  
        # 256 + 8 = 264
        dd bs=1k count=264k if=/dev/zero | tr '\000' '\377' > nandflash.img
    fi
}

build_kernel()
{
    echo "-------------------------- 编译linux kernel --------------------------"
    if [ ! -d "$SHELL_FOLDER/output/linux_kernel" ]; then  
    mkdir $SHELL_FOLDER/output/linux_kernel
    fi  
    cd $SHELL_FOLDER/linux-6.1.11
    make ARCH=riscv CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- quard_star_defconfig
    make ARCH=riscv CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- -j$PROCESSORS
    #make ARCH=riscv CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- tools/perf -j$PROCESSORS
    cp $SHELL_FOLDER/linux-6.1.11/arch/riscv/boot/Image $SHELL_FOLDER/output/linux_kernel/Image
}

build_busybox()
{
    echo "---------------------------- 编译busybox -----------------------------"
    if [ ! -d "$SHELL_FOLDER/output/busybox" ]; then  
    mkdir $SHELL_FOLDER/output/busybox
    fi  
    cd $SHELL_FOLDER/busybox-1.33.2
    make ARCH=riscv CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- quard_star_defconfig
    make ARCH=riscv CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- -j$PROCESSORS
    make ARCH=riscv CROSS_COMPILE=$GLIB_ELF_CROSS_PREFIX- install
}

build_rootfs()
{
    echo "----------------------------- 合成文件系统映像 -----------------------------"
    if [ ! -f "$SHELL_FOLDER/output/linux_kernel/Image" ]; then  
        echo "not found Image, please ./build.sh kernel"
        exit 255
    fi
    if [ ! -f "$SHELL_FOLDER/output/uboot/quard_star_uboot.dtb" ]; then  
        echo "not found quard_star_uboot.dtb, please ./build.sh uboot_dtb"
        exit 255
    fi
    if [ ! -d "$SHELL_FOLDER/output/busybox" ]; then  
        echo "not found busybox, please ./build.sh busybox"
        exit 255
    fi
    MAKE_ROOTFS_DIR=$SHELL_FOLDER/output/rootfs
    TARGET_ROOTFS_DIR=$MAKE_ROOTFS_DIR/rootfs
    TARGET_BOOTFS_DIR=$MAKE_ROOTFS_DIR/bootfs
    if [ ! -d "$MAKE_ROOTFS_DIR" ]; then
    BUILD_ROOTFS_OPT="all"
    fi
    if [ ! -f "$MAKE_ROOTFS_DIR/rootfs.img" ]; then  
    BUILD_ROOTFS_OPT="all"
    fi
    case "$BUILD_ROOTFS_OPT" in
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
        dd if=/dev/zero of=rootfs.img bs=1M count=4096
        $PERMISSION_TOOL $SHELL_FOLDER/build_rootfs/generate_rootfs.sh $MAKE_ROOTFS_DIR/rootfs.img $SHELL_FOLDER/build_rootfs/sfdisk
        fi
        cp $SHELL_FOLDER/output/linux_kernel/Image $TARGET_BOOTFS_DIR/Image
        cp $SHELL_FOLDER/output/uboot/quard_star_uboot.dtb $TARGET_BOOTFS_DIR/quard_star.dtb
        $SHELL_FOLDER/u-boot-2023.04/tools/mkimage -A riscv -O linux -T script -C none -a 0 -e 0 -n "Distro Boot Script" -d $SHELL_FOLDER/dts/quard_star_uboot.cmd $TARGET_BOOTFS_DIR/boot.scr
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
        if [ ! -d "$TARGET_ROOTFS_DIR/var" ]; then  
        mkdir $TARGET_ROOTFS_DIR/var
        mkdir $TARGET_ROOTFS_DIR/var/log
        mkdir $TARGET_ROOTFS_DIR/var/run
        fi
        if [ ! -d "$TARGET_ROOTFS_DIR/lib" ]; then  
        mkdir $TARGET_ROOTFS_DIR/lib
        cd $TARGET_ROOTFS_DIR
        ln -s ./lib ./lib64
        cd $MAKE_ROOTFS_DIR
        fi
        cp -r $GLIB_ELF_CROSS_PREFIX_SYSROOT_DIR/lib/* $TARGET_ROOTFS_DIR/lib/
        cp -r $GLIB_ELF_CROSS_PREFIX_SYSROOT_DIR/usr/bin/* $TARGET_ROOTFS_DIR/usr/bin/
        $SHELL_FOLDER/build_rootfs/clean_gitkeep.sh $TARGET_BOOTFS_DIR
        $SHELL_FOLDER/build_rootfs/clean_gitkeep.sh $TARGET_ROOTFS_DIR
        $PERMISSION_TOOL $SHELL_FOLDER/build_rootfs/build_fs.sh $MAKE_ROOTFS_DIR
        ;;
    bootfs)
        if [ ! -d "$TARGET_BOOTFS_DIR" ]; then  
        mkdir $TARGET_ROOTFS_DIR
        fi
        cp $SHELL_FOLDER/output/linux_kernel/Image $TARGET_BOOTFS_DIR/Image
        cp $SHELL_FOLDER/output/uboot/quard_star_uboot.dtb $TARGET_BOOTFS_DIR/quard_star.dtb
        cp $SHELL_FOLDER/output/uboot/boot.scr $TARGET_BOOTFS_DIR/boot.scr
        $SHELL_FOLDER/build_rootfs/clean_gitkeep.sh $TARGET_BOOTFS_DIR
        $PERMISSION_TOOL $SHELL_FOLDER/build_rootfs/build_fs_only_bootfs.sh $MAKE_ROOTFS_DIR
        ;;
    *)
        echo "skip build rootfs.img!"
        ;;
    esac
}

build_all()
{
    build_qemu
    build_mask_rom
    build_lowlevelboot
    build_opensbi
    build_sbi_dtb
    build_trusted_domain
    build_uboot_dtb
    build_uboot
    build_firmware
    build_kernel
    build_busybox
    build_rootfs
}

echo_usage()
{
    TARGET="qemu|mask_rom|lowlevelboot|opensbi|sbi_dtb|trusted_domain|uboot|uboot_dtb|firmware|kernel|busybox|rootfs|all"
    ROOTFS_OPT="all|bootfs"
    USAGE="usage $0 [$TARGET] [$ROOTFS_OPT]"
	echo $USAGE
}

case "$BUILD_TARGET" in
--help)
    echo_usage
	exit 0
	;;
qemu)
    build_qemu
    ;;
qemu_w64)
    build_qemu_w64
    ;;
qemu_macos)
    build_qemu_macos
    ;;
qemu_android)
    build_qemu_android
    ;;
mask_rom)
    build_mask_rom
    ;;
lowlevelboot)
    build_lowlevelboot
    ;;
opensbi)
    build_opensbi
    ;;
sbi_dtb)
    build_sbi_dtb
    ;;
trusted_domain)
    build_trusted_domain
    ;;
uboot)
    build_uboot
    ;;
uboot_dtb)
    build_uboot_dtb
    ;;
firmware)
    build_firmware
    ;;
kernel)
    build_kernel
    ;;
busybox)
    build_busybox
    ;;
rootfs)
    build_rootfs
    ;;
all)
    build_all
    ;;
*)
    echo_usage
	exit 255
	;;
esac

echo "----------------------------------- 完成 -----------------------------------"
