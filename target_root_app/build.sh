SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
PROCESSORS=$(< /proc/cpuinfo grep "processor" | wc -l)
CROSS_COMPILE_DIR=/opt/gcc-riscv64-unknown-linux-gnu
CROSS_PREFIX=$CROSS_COMPILE_DIR/bin/riscv64-unknown-linux-gnu

case "$1" in
hosttool)
    echo "skip export!"
	;;
all)
    echo "lazy export!"
	;;
*)
    export PATH=$SHELL_FOLDER/host_output/bin:$PATH
    export ACLOCAL_PATH=$SHELL_FOLDER/host_output/share/aclocal
	;;
esac

build_hosttool()
{
    # 编译automake
    echo "---------------------------- 编译automake ----------------------------"
    cd $SHELL_FOLDER/automake-1.16.1
    autoreconf -f -i 
    ./configure --prefix=$SHELL_FOLDER/host_output
    make -j$PROCESSORS
    make install
    # 编译pkgconfig
    echo "---------------------------- 编译pkgconfig ----------------------------"
    cd $SHELL_FOLDER/pkg-config-0.29.2
    ./configure --prefix=$SHELL_FOLDER/host_output
    make -j$PROCESSORS
    make install
}

build_make()
{
    # 编译make
    echo "------------------------------ 编译make ------------------------------"
    cd $SHELL_FOLDER/make-4.3
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
}

build_ncurses()
{
    # 编译ncurses
    echo "----------------------------- 编译ncurses -----------------------------"
    cd $SHELL_FOLDER/ncurses-6.2
    ./configure --host=riscv64-linux-gnu --with-shared --without-normal --without-debug CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make  install.libs DESTDIR=$SHELL_FOLDER/output
    #make install.progs
    #make install.data
}

build_bash()
{
    # 编译bash
    echo "------------------------------ 编译bash ------------------------------"
    cd $SHELL_FOLDER/bash-5.1.8
    ./configure --host=riscv64 --prefix=$SHELL_FOLDER/output CCFLAGS=-I$SHELL_FOLDER/output/usr/include LDFLAGS=-L$SHELL_FOLDER/output/usr/lib CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
}

build_sudo()
{
    # 编译sudo
    echo "------------------------------ 编译sudo ------------------------------"
    cd $SHELL_FOLDER/sudo-SUDO_1_9_7p1
    ./configure --host=riscv64-linux-gnu CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    #make install-binaries
}

build_screenfetch()
{
    # 编译screenFetch
    echo "--------------------------- 编译screenFetch ---------------------------"
    cd $SHELL_FOLDER/screenFetch-3.9.1
    if [ ! -d "$SHELL_FOLDER/output/usr" ]; then  
    mkdir $SHELL_FOLDER/output/usr
    mkdir $SHELL_FOLDER/output/usr/bin
    fi  
    if [ ! -d "$SHELL_FOLDER/output/usr/bin" ]; then  
    mkdir $SHELL_FOLDER/output/usr/bin
    fi 
    cp screenfetch-dev $SHELL_FOLDER/output/usr/bin/screenfetch
}

build_tree()
{
    # 编译tree
    echo "------------------------------ 编译tree ------------------------------"
    cd $SHELL_FOLDER/tree-1.8.0
    make prefix=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc -j$PROCESSORS
    make prefix=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc install
}

build_libevent()
{
    # 编译libevent
    echo "---------------------------- 编译libevent ----------------------------"
    cd $SHELL_FOLDER/libevent-2.1.12-stable
    ./configure --host=riscv64-linux-gnu --disable-openssl --disable-static --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
}

build_screen()
{
    # 编译screen
    echo "----------------------------- 编译screen -----------------------------"
    cd $SHELL_FOLDER/screen-4.8.0
    ./configure --host=riscv64-linux-gnu CCFLAGS=-I$SHELL_FOLDER/output/usr/include LDFLAGS=-L$SHELL_FOLDER/output/usr/lib CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    #make install
}

build_cu()
{
    # 编译cu
    echo "------------------------------- 编译cu -------------------------------"
    cd $SHELL_FOLDER/cu
    make prefix=$SHELL_FOLDER/output LIBEVENTDIR=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc -j$PROCESSORS
    make prefix=$SHELL_FOLDER/output LIBEVENTDIR=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc install
}

build_qt()
{
    # 编译qt
    echo "------------------------------- 编译qt -------------------------------"
    cd $SHELL_FOLDER/qt-everywhere-src-5.12.11
    TEMP_PATH=$PATH
	export PATH=$PATH:$CROSS_COMPILE_DIR/bin
	./configure -opensource -confirm-license -release -optimize-size -strip -ltcg -silent -qpa linuxfb -no-opengl -skip webengine -nomake tools -nomake tests -nomake examples -xplatform linux-riscv64-gnu-g++ -prefix /opt/Qt-5.12.11 -extprefix $SHELL_FOLDER/host_output
	make -j$PROCESSORS
	make install
	export PATH=$TEMP_PATH
}

build_libmnl()
{
    # 编译libmnl
    echo "----------------------------- 编译libmnl -----------------------------"
    cd $SHELL_FOLDER/libmnl-1.0.4
    autoreconf -f -i 
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
}

build_ethtool()
{
    # 编译ethtool
    echo "----------------------------- 编译ethtool -----------------------------"
    cd $SHELL_FOLDER/ethtool-5.13
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output MNL_CFLAGS=-I$SHELL_FOLDER/output/include MNL_LIBS="-L$SHELL_FOLDER/output/lib -lmnl" CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
}

build_openssl()
{
    # 编译openssl
    echo "----------------------------- 编译openssl -----------------------------"
    cd $SHELL_FOLDER/openssl-1.1.1j
	./Configure linux-generic64 no-asm --prefix=$SHELL_FOLDER/output --cross-compile-prefix=$CROSS_PREFIX-
	make -j$PROCESSORS
    make install_sw
}

build_iperf()
{
    # 编译iperf
    echo "------------------------------ 编译iperf ------------------------------"
    cd $SHELL_FOLDER/iperf-3.10.1
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --with-openssl=$SHELL_FOLDER/output --disable-static CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
}

build_zlib()
{
    # 编译zlib
    echo "------------------------------ 编译zlib ------------------------------"
    cd $SHELL_FOLDER/zlib-1.2.11
	export CC=$CROSS_PREFIX-gcc 
    ./configure --prefix=$SHELL_FOLDER/output
	make -j$PROCESSORS
    make install
    unset CC
}

build_openssh()
{
    # 编译openssh
    echo "----------------------------- 编译openssh -----------------------------"
    cd $SHELL_FOLDER/openssh-8.6p1
    ./configure --host=riscv64-linux-gnu --with-openssl=$SHELL_FOLDER/output --with-zlib=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    #make install
}

build_libpng()
{
    # 编译libpng
    echo "----------------------------- 编译libpng -----------------------------"
    cd $SHELL_FOLDER/libpng-1.6.34
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static CPPFLAGS=-I$SHELL_FOLDER/output/include LDFLAGS=-L$SHELL_FOLDER/output/lib CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
}

build_freetype()
{
    # 编译freetype
    echo "---------------------------- 编译freetype ----------------------------"
    cd $SHELL_FOLDER/freetype-2.11.0
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static --with-zlib=$SHELL_FOLDER/output --with-png=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
}

build_libx11()
{
    # 编译libx11
    echo "------------------------------- 编译x11 -------------------------------"

    echo "--------------------------- 编译util-macros ---------------------------"
    cd $SHELL_FOLDER/libX11/util-macros-1.19.3
    ./configure --prefix=$SHELL_FOLDER/host_output
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译xproto -----------------------------"
    cd $SHELL_FOLDER/libX11/xproto-7.0.31
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "---------------------------- 编译xextproto ----------------------------"
    cd $SHELL_FOLDER/libX11/xextproto-7.3.0
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译inputproto ---------------------------"
    cd $SHELL_FOLDER/libX11/inputproto-2.3.2
    autoreconf -f -i 
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译kbproto -----------------------------"
    cd $SHELL_FOLDER/libX11/kbproto-1.0.7
    autoreconf -f -i 
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译util-macros ---------------------------"
    cd $SHELL_FOLDER/libX11/util-macros-1.19.3
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译xtrans -----------------------------"
    cd $SHELL_FOLDER/libX11/xtrans-1.4.0
    autoreconf -f -i 
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "---------------------------- 编译xcb-proto ----------------------------"
    cd $SHELL_FOLDER/libX11/xcb-proto-1.14
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译libXau -----------------------------"
    cd $SHELL_FOLDER/libX11/libXau-1.0.9
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译libxcb -----------------------------"
    cd $SHELL_FOLDER/libX11/libxcb-1.14
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译libX11 -----------------------------"
    cd $SHELL_FOLDER/libX11/libX11-1.7.2
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static --enable-malloc0returnsnull PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译libXext -----------------------------"
    cd $SHELL_FOLDER/libX11/libXext-1.3.1
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译libICE -----------------------------"
    cd $SHELL_FOLDER/libX11/libICE-1.0.10
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "------------------------------ 编译libSM ------------------------------"
    cd $SHELL_FOLDER/libX11/libSM-1.2.3
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "------------------------------ 编译libXt ------------------------------"
    cd $SHELL_FOLDER/libX11/libXt-1.1.3
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译recordproto ---------------------------"
    cd $SHELL_FOLDER/libX11/recordproto-1.14.2
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译renderproto ---------------------------"
    cd $SHELL_FOLDER/libX11/renderproto-0.11.1
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译fixesproto ---------------------------"
    cd $SHELL_FOLDER/libX11/fixesproto-5.0
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "---------------------------- 编译libXfixes ----------------------------"
    cd $SHELL_FOLDER/libX11/libXfixes-5.0.3
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "------------------------------ 编译libXi ------------------------------"
    cd $SHELL_FOLDER/libX11/libXi-1.7.10
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static --enable-malloc0returnsnull PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译libXtst -----------------------------"
    cd $SHELL_FOLDER/libX11/libXtst-1.2.1
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译libXrender ---------------------------"
    cd $SHELL_FOLDER/libX11/libXrender-0.9.7
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译randrproto ---------------------------"
    cd $SHELL_FOLDER/libX11/randrproto-1.5.0
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "---------------------------- 编译libXrandr ----------------------------"
    cd $SHELL_FOLDER/libX11/libXrandr-1.3.2
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
}

build_cups()
{
    # 编译cups
    echo "------------------------------ 编译cups ------------------------------"
    cd $SHELL_FOLDER/cups-2.3.1
    export STRIPPROG=$CROSS_PREFIX-strip
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install-headers install-libs install-exec
    unset STRIPPROG
}

build_libxml2()
{
    # 编译libxml2
    echo "----------------------------- 编译libxml2 -----------------------------"
    cd $SHELL_FOLDER/libxml2-2.9.12
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static --without-python --with-sax1 CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
}

build_fontconfig()
{
    # 编译fontconfig
    echo "\033[1;4;41;32m编译\033[0m"
    echo "--------------------------- 编译fontconfig ---------------------------"
    cd $SHELL_FOLDER/fontconfig-2.13.94
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --enable-libxml2 --disable-static FREETYPE_CFLAGS=-I$SHELL_FOLDER/output/include/freetype2 FREETYPE_LIBS="-L$SHELL_FOLDER/output/lib -lfreetype" LIBXML2_CFLAGS=-I$SHELL_FOLDER/output/include/libxml2 LIBXML2_LIBS="-L$SHELL_FOLDER/output/lib -lxml2" CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
}

build_libffi()
{
    # 编译libffi
    echo "----------------------------- 编译libffi -----------------------------"
    cd $SHELL_FOLDER/libffi-3.4.2
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static  CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
}

build_alsa()
{
    # 编译alsa
    echo "------------------------------ 编译alsa ------------------------------"
    cd $SHELL_FOLDER/alsa-lib-1.2.5
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --disable-static  CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
}

build_openjdk_zero()
{
    echo "----------------------------- 编译openjdk -----------------------------"
    cd $SHELL_FOLDER/jdk11u-dev-113c646a33d2
    TEMP_PATH=$PATH
	export PATH=$PATH:$CROSS_COMPILE_DIR/bin
    bash configure --with-boot-jdk=$SHELL_FOLDER/host_output/opt/jdk-10 --with-jvm-variants=zero --openjdk-target=riscv64-unknown-linux-gnu --prefix=$SHELL_FOLDER/output --enable-headless-only=yes --disable-warnings-as-errors --with-alsa=$SHELL_FOLDER/output --with-x=$SHELL_FOLDER/output --with-cups=$SHELL_FOLDER/output --with-fontconfig=$SHELL_FOLDER/output --with-libffi=$SHELL_FOLDER/output --with-freetype-include=$SHELL_FOLDER/output/include/freetype2 --with-freetype-lib=$SHELL_FOLDER/output/lib
    make
	export PATH=$TEMP_PATH
}

build_libuuid()
{
    # 编译libuuid
    echo "----------------------------- 编译libuuid -----------------------------"
    cd $SHELL_FOLDER/libuuid-1.0.3
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
}

build_lzo()
{
    # 编译lzo
    echo "------------------------------- 编译lzo -------------------------------"
    cd $SHELL_FOLDER/lzo-2.10
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
}

build_attr()
{
    # 编译attr
    echo "------------------------------ 编译attr ------------------------------"
    cd $SHELL_FOLDER/attr-2.5.1
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
}

build_mtd_utils()
{
    # 编译mtd-utils
    echo "---------------------------- 编译mtd-utils ----------------------------"
    cd $SHELL_FOLDER/mtd-utils-2.1.2
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --without-jffs --without-ubifs PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
}

build_dtc()
{
    # 编译dtc-1.6.1
    echo "\033[1;4;41;32m\033[0m"
    echo "------------------------------- 编译dtc -------------------------------"
    cd $SHELL_FOLDER/dtc-1.6.1
	make CC=${CROSS_PREFIX}-gcc PREFIX=$SHELL_FOLDER/output libfdt -j$PROCESSORS
    make CC=${CROSS_PREFIX}-gcc PREFIX=$SHELL_FOLDER/output install-lib install-includes
}

build_trace_cmd()
{
    # 编译trace-cmd-v2.9.5
    echo "---------------------------- 编译trace-cmd ----------------------------"
    cd $SHELL_FOLDER/trace-cmd-v2.9.5
	make CROSS_COMPILE=${CROSS_PREFIX}- CC=${CROSS_PREFIX}-gcc AR=${CROSS_PREFIX}-ar prefix=/ libdir_relative=lib -j$PROCESSORS
	make CROSS_COMPILE=${CROSS_PREFIX}- CC=${CROSS_PREFIX}-gcc AR=${CROSS_PREFIX}-ar prefix=$SHELL_FOLDER/output etcdir=$SHELL_FOLDER/output/etc libdir_relative=lib install
}

build_lrzsz()
{
    # 编译lrzsz-0.12.20
    echo "------------------------------ 编译lrzsz ------------------------------"
    cd $SHELL_FOLDER/lrzsz-0.12.20
    export CXX=$CROSS_PREFIX-g++
    export CC=$CROSS_PREFIX-gcc 
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output 
    make -j$PROCESSORS
    make install
    unset CXX
    unset CC
}

build_libexpat()
{
    echo "---------------------------- 编译libexpat ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf expat-2.4.3.tar.gz
    cd $SHELL_FOLDER/expat-2.4.3
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output  CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
}

build_libdaemon()
{
    echo "---------------------------- 编译libdaemon ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf libdaemon-0.14.tar.gz
    cd $SHELL_FOLDER/libdaemon-0.14
    autoreconf -f -i 
    echo "ac_cv_func_setpgrp_void=yes" > config.cache
    ./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --config-cache --disable-lynx CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
}

build_avahi()
{
    # 编译avahi
    echo "------------------------------ 编译avahi ------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf avahi-0.7.tar.gz
    cd $SHELL_FOLDER/avahi-0.7
    ./configure --host=riscv64-linux-gnu --prefix=/usr/ --with-xml=expat --enable-libdaemon --with-distro=none --disable-glib --disable-gobject --disable-qt3 --disable-qt4 --disable-gtk --disable-gtk3 --disable-dbus --disable-gdbm --disable-python --disable-pygtk --disable-python-dbus --disable-mono --disable-monodoc CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc CPPFLAGS="-I$SHELL_FOLDER/output/include" LDFLAGS="-L$SHELL_FOLDER/output/lib" LIBDAEMON_CFLAGS="-I$SHELL_FOLDER/output/include" LIBDAEMON_LIBS="-L$SHELL_FOLDER/output/lib -ldaemon"
    make -j$PROCESSORS
    make install DESTDIR=$SHELL_FOLDER/output
}

build_iperf3()
{
    # 编译iperf3
    echo "----------------------------- 编译iperf3 -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf iperf-3.10.1.tar.gz
    cd $SHELL_FOLDER/iperf-3.10.1
    ./configure --enable-static-bin --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --without-openssl CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc
    make -j$PROCESSORS
    make install
}

case "$1" in
hosttool)
    build_hosttool
    ;;
make)
    build_make
    ;;
ncurses)
    build_ncurses
    ;;
bash)
    build_bash
    ;;
sudo)
    build_sudo
    ;;
screenfetch)
    build_screenfetch
    ;;
tree)
    build_tree
    ;;
libevent)
    build_libevent
    ;;
screen)
    build_screen
    ;;
cu)
    build_cu
    ;;
qt)
    build_qt
    ;;
libmnl)
    build_libmnl
    ;;
ethtool)
    build_ethtool
    ;;
openssl)
    build_openssl
    ;;
iperf)
    build_iperf
    ;;
zlib)
    build_zlib
    ;;
openssh)
    build_openssh
    ;;
libpng)
    build_libpng
    ;;
freetype)
    build_freetype
    ;;
libx11)
    build_libx11
    ;;
cups)
    build_cups
    ;;
libxml2)
    build_libxml2
    ;;
fontconfig)
    build_fontconfig
    ;;
libffi)
    build_libffi
    ;;
alsa)
    build_alsa
    ;;
openjdk_zero)
    build_openjdk_zero
    ;;
libuuid)
    build_libuuid
    ;;
lzo)
    build_lzo
    ;;
attr)
    build_attr
    ;;
mtd_utils)
    build_mtd_utils
    ;;
dtc)
    build_dtc
    ;;
trace_cmd)
    build_trace_cmd
    ;;
lrzsz)
    build_lrzsz
    ;;
libexpat)
    build_libexpat
    ;;
libdaemon)
    build_libdaemon
    ;;
avahi)
    build_avahi
    ;;
iperf3)
    build_iperf3
    ;;
all)
    build_hosttool
    export PATH=$SHELL_FOLDER/host_output/bin:$PATH
    export ACLOCAL_PATH=$SHELL_FOLDER/host_output/share/aclocal
    build_make
    build_ncurses
    build_bash
    build_sudo
    build_screenfetch
    build_tree
    build_libevent
    build_screen
    build_cu
	build_qt
	build_libmnl
	build_ethtool
	build_zlib
	build_openssl
	build_libpng
	build_freetype
    build_libx11
    build_cups
	build_libxml2
	build_fontconfig
	build_libffi
	build_alsa
    build_openjdk_zero
    build_libuuid
    build_lzo
    build_attr
    build_mtd_utils
    build_dtc
    build_trace_cmd
    build_lrzsz
    build_libexpat
    build_libdaemon
    build_avahi
    build_iperf3
    ;;
*)
    echo "Please enter the built package name or use \"all\" !"
    exit 1
	;;
esac

echo "----------------------------------- 完成 -----------------------------------"
