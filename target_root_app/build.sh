#!/bin/bash
set -e

SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
PROCESSORS=$(< /proc/cpuinfo grep "processor" | wc -l)
CROSS_COMPILE_DIR=/opt/gcc-riscv64-unknown-linux-gnu
CROSS_PREFIX=$CROSS_COMPILE_DIR/bin/riscv64-unknown-linux-gnu

build_make()
{
    # 编译make
    echo "------------------------------ 编译make ------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf make-4.3.tar.gz
    cd $SHELL_FOLDER/make-4.3
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/make-4.3
}

build_ncurses()
{
    # 编译ncurses
    echo "----------------------------- 编译ncurses -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf ncurses-6.2.tar.gz
    cd $SHELL_FOLDER/ncurses-6.2
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=/ \
        --with-shared \
        --without-normal \
        --without-debug \
        --disable-stripping \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install DESTDIR=$SHELL_FOLDER/output
    rm -rf $SHELL_FOLDER/output/lib/libncurses++.a
    rm -rf $SHELL_FOLDER/ncurses-6.2
}

build_bash()
{
    # 编译bash
    echo "------------------------------ 编译bash ------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf bash-5.1.8.tar.gz
    cd $SHELL_FOLDER/bash-5.1.8
    ./configure \
        --host=riscv64 \
        --prefix=$SHELL_FOLDER/output \
        CCFLAGS=-I$SHELL_FOLDER/output/include \
        LDFLAGS=-L$SHELL_FOLDER/output/lib \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/bash-5.1.8
}

build_sudo()
{
    # 编译sudo
    echo "------------------------------ 编译sudo ------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf sudo-SUDO_1_9_7p1.tar.gz
    cd $SHELL_FOLDER/sudo-SUDO_1_9_7p1
    ./configure \
        --host=riscv64-linux-gnu \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    # FIXME:sudo的install比较特殊,目前是在目标系统内执行make install
    #make install
    #rm -rf $SHELL_FOLDER/sudo-SUDO_1_9_7p1
}

build_screenfetch()
{
    # 编译screenFetch
    echo "--------------------------- 编译screenFetch ---------------------------"
    cd $SHELL_FOLDER/screenFetch-3.9.1
    if [ ! -d "$SHELL_FOLDER/output" ]; then  
    mkdir $SHELL_FOLDER/output
    mkdir $SHELL_FOLDER/output/bin
    fi  
    if [ ! -d "$SHELL_FOLDER/output/bin" ]; then  
    mkdir $SHELL_FOLDER/output/bin
    fi 
    cp screenfetch-dev $SHELL_FOLDER/output/bin/screenfetch
}

build_tree()
{
    # 编译tree
    echo "------------------------------ 编译tree ------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf tree-1.8.0.tgz
    cd $SHELL_FOLDER/tree-1.8.0
    make prefix=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc -j$PROCESSORS
    make prefix=$SHELL_FOLDER/output MANDIR=$SHELL_FOLDER/output/share/man/man1 CC=$CROSS_PREFIX-gcc install
    rm -rf $SHELL_FOLDER/tree-1.8.0
}

build_libevent()
{
    # 编译libevent
    echo "---------------------------- 编译libevent ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf libevent-2.1.12-stable.tar.gz
    cd $SHELL_FOLDER/libevent-2.1.12-stable
    ./configure \
        --host=riscv64-linux-gnu \
        --disable-openssl \
        --disable-static \
        --prefix=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/libevent-2.1.12-stable
}

build_screen()
{
    # 编译screen
    echo "----------------------------- 编译screen -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf screen-4.8.0.tar.gz
    cd $SHELL_FOLDER/screen-4.8.0
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=/ \
        CCFLAGS=-I$SHELL_FOLDER/output/include \
        LDFLAGS=-L$SHELL_FOLDER/output/lib \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install DESTDIR=$SHELL_FOLDER/output
    rm -rf $SHELL_FOLDER/screen-4.8.0
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
	./configure \
        -opensource \
        -confirm-license \
        -release \
        -optimize-size \
        -strip \
        -ltcg \
        -silent \
        -qpa linuxfb \
        -no-opengl \
        -skip webengine \
        -nomake tools \
        -nomake tests \
        -nomake examples \
        -xplatform linux-riscv64-gnu-g++ \
        -prefix /opt/Qt-5.12.11 \
        -extprefix $SHELL_FOLDER/host_output
	make -j$PROCESSORS
	make install
	export PATH=$TEMP_PATH
    if [ ! -d "$SHELL_FOLDER/output/opt" ]; then
        mkdir $SHELL_FOLDER/output/opt
    fi
    if [ ! -d "$SHELL_FOLDER/output/opt/Qt-5.12.11" ]; then
        mkdir $SHELL_FOLDER/output/opt/Qt-5.12.11
    fi
    cp -r $SHELL_FOLDER/host_output/lib $SHELL_FOLDER/output/opt/Qt-5.12.11/lib
    cp -r $SHELL_FOLDER/host_output/plugins $SHELL_FOLDER/output/opt/Qt-5.12.11/plugins
    cp -r $SHELL_FOLDER/host_output/translations $SHELL_FOLDER/output/opt/Qt-5.12.11/translations
    rm -rf $SHELL_FOLDER/output/opt/Qt-5.12.11/lib/cmake
    rm -rf $SHELL_FOLDER/output/opt/Qt-5.12.11/lib/pkgconfig
    rm -rf $SHELL_FOLDER/output/opt/Qt-5.12.11/lib/*.prl
    rm -rf $SHELL_FOLDER/output/opt/Qt-5.12.11/lib/*.a
    rm -rf $SHELL_FOLDER/output/opt/Qt-5.12.11/lib/*.la
}

build_libmnl()
{
    # 编译libmnl
    echo "----------------------------- 编译libmnl -----------------------------"
    cd $SHELL_FOLDER
    tar -jxvf libmnl-1.0.4.tar.bz2
    cd $SHELL_FOLDER/libmnl-1.0.4
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/libmnl-1.0.4
}

build_ethtool()
{
    # 编译ethtool
    echo "----------------------------- 编译ethtool -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf ethtool-5.13.tar.gz
    cd $SHELL_FOLDER/ethtool-5.13
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        MNL_CFLAGS=-I$SHELL_FOLDER/output/include \
        MNL_LIBS="-L$SHELL_FOLDER/output/lib -lmnl" \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/ethtool-5.13
}

build_openssl()
{
    # 编译openssl
    echo "----------------------------- 编译openssl -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf openssl-1.1.1j.tar.gz
    cd $SHELL_FOLDER/openssl-1.1.1j
	./Configure \
        linux-generic64 \
        no-asm \
        --prefix=$SHELL_FOLDER/output \
        --cross-compile-prefix=$CROSS_PREFIX-
	make -j$PROCESSORS
    make install_sw
    rm $SHELL_FOLDER/output/lib/libssl.a
    rm $SHELL_FOLDER/output/lib/libcrypto.a
    rm -rf $SHELL_FOLDER/openssl-1.1.1j
}

build_zlib()
{
    # 编译zlib
    echo "------------------------------ 编译zlib ------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf zlib-1.2.11.tar.gz
    cd $SHELL_FOLDER/zlib-1.2.11
	export CC=$CROSS_PREFIX-gcc 
    ./configure \
        --prefix=$SHELL_FOLDER/output
	make -j$PROCESSORS
    make install
    rm $SHELL_FOLDER/output/lib/libz.a
    unset CC
    rm -rf $SHELL_FOLDER/zlib-1.2.11
}

build_openssh()
{
    # 编译openssh
    echo "----------------------------- 编译openssh -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf openssh-8.6p1.tar.gz
    cd $SHELL_FOLDER/openssh-8.6p1
    ./configure \
        --host=riscv64-linux-gnu \
        --disable-strip \
        --with-openssl=$SHELL_FOLDER/output \
        --with-zlib=$SHELL_FOLDER/output \
        --prefix=/ \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    # FIXME:在host上使用nokeys安装，目标系统内需自行添加keys
    make install-nokeys DESTDIR=$SHELL_FOLDER/output
    rm -rf $SHELL_FOLDER/openssh-8.6p1
}

build_libpng()
{
    # 编译libpng
    echo "----------------------------- 编译libpng -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf libpng-1.6.34.tar.gz
    cd $SHELL_FOLDER/libpng-1.6.34
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        CPPFLAGS=-I$SHELL_FOLDER/output/include \
        LDFLAGS=-L$SHELL_FOLDER/output/lib \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/libpng-1.6.34
}

build_freetype()
{
    # 编译freetype
    echo "---------------------------- 编译freetype ----------------------------"
    cd $SHELL_FOLDER
    tar -xvJf freetype-2.11.0.tar.xz
    cd $SHELL_FOLDER/freetype-2.11.0
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        --with-harfbuzz=no \
        --with-zlib=$SHELL_FOLDER/output \
        --with-png=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/freetype-2.11.0
}

build_libx11()
{
    # 编译libx11
    echo "------------------------------- 编译x11 -------------------------------"

    echo "--------------------------- 编译util-macros ---------------------------"
    cd $SHELL_FOLDER/libX11/util-macros-1.19.3
    autoreconf -f -i 
    ./configure \
        --prefix=$SHELL_FOLDER/host_output
	make -j$PROCESSORS
    make install
    export ACLOCAL_PATH=$SHELL_FOLDER/host_output/share/aclocal

    echo "----------------------------- 编译xproto -----------------------------"
    cd $SHELL_FOLDER/libX11/xproto-7.0.31
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "---------------------------- 编译xextproto ----------------------------"
    cd $SHELL_FOLDER/libX11/xextproto-7.3.0
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译inputproto ---------------------------"
    cd $SHELL_FOLDER/libX11/inputproto-2.3.2
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译kbproto -----------------------------"
    cd $SHELL_FOLDER/libX11/kbproto-1.0.7
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译util-macros ---------------------------"
    cd $SHELL_FOLDER/libX11/util-macros-1.19.3
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译xtrans -----------------------------"
    cd $SHELL_FOLDER/libX11/xtrans-1.4.0
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "---------------------------- 编译xcb-proto ----------------------------"
    cd $SHELL_FOLDER/libX11/xcb-proto-1.14
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译libXau -----------------------------"
    cd $SHELL_FOLDER/libX11/libXau-1.0.9
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译libxcb -----------------------------"
    cd $SHELL_FOLDER/libX11/libxcb-1.14
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译libX11 -----------------------------"
    cd $SHELL_FOLDER/libX11/libX11-1.7.2
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        --enable-malloc0returnsnull \
        --disable-xf86bigfont \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译libXext -----------------------------"
    cd $SHELL_FOLDER/libX11/libXext-1.3.1
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译libICE -----------------------------"
    cd $SHELL_FOLDER/libX11/libICE-1.0.10
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "------------------------------ 编译libSM ------------------------------"
    cd $SHELL_FOLDER/libX11/libSM-1.2.3
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "------------------------------ 编译libXt ------------------------------"
    cd $SHELL_FOLDER/libX11/libXt-1.1.3
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译recordproto ---------------------------"
    cd $SHELL_FOLDER/libX11/recordproto-1.14.2
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译renderproto ---------------------------"
    cd $SHELL_FOLDER/libX11/renderproto-0.11.1
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译fixesproto ---------------------------"
    cd $SHELL_FOLDER/libX11/fixesproto-5.0
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "---------------------------- 编译libXfixes ----------------------------"
    cd $SHELL_FOLDER/libX11/libXfixes-5.0.3
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "------------------------------ 编译libXi ------------------------------"
    cd $SHELL_FOLDER/libX11/libXi-1.7.10
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        --enable-malloc0returnsnull \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "----------------------------- 编译libXtst -----------------------------"
    cd $SHELL_FOLDER/libX11/libXtst-1.2.1
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译libXrender ---------------------------"
    cd $SHELL_FOLDER/libX11/libXrender-0.9.7
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "--------------------------- 编译randrproto ---------------------------"
    cd $SHELL_FOLDER/libX11/randrproto-1.5.0
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    echo "---------------------------- 编译libXrandr ----------------------------"
    cd $SHELL_FOLDER/libX11/libXrandr-1.3.2
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install

    unset ACLOCAL_PATH
}

build_cups()
{
    # 编译cups
    echo "------------------------------ 编译cups ------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf cups-2.3.1-source.tar.gz
    cd $SHELL_FOLDER/cups-2.3.1
    export STRIPPROG=$CROSS_PREFIX-strip
    patch -p1 < ../cups-2.3.1-source.patch
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        --disable-libusb \
        --disable-dbus \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install-headers install-libs install-exec
    unset STRIPPROG
    rm -rf $SHELL_FOLDER/cups-2.3.1
}

build_libxml2()
{
    # 编译libxml2
    echo "----------------------------- 编译libxml2 -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf libxml2-2.9.12.tar.gz
    cd $SHELL_FOLDER/libxml2-2.9.12
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        --without-python \
        --with-sax1 \
        --with-zlib=$SHELL_FOLDER/output \
        --without-lzma \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/libxml2-2.9.12
}

build_fontconfig()
{
    # 编译fontconfig
    echo "--------------------------- 编译fontconfig ---------------------------"
    cd $SHELL_FOLDER
    tar -xzvf fontconfig-2.13.94.tar.gz
    cd $SHELL_FOLDER/fontconfig-2.13.94
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --enable-libxml2 \
        --disable-static \
        FREETYPE_CFLAGS=-I$SHELL_FOLDER/output/include/freetype2 \
        FREETYPE_LIBS="-L$SHELL_FOLDER/output/lib -lfreetype" \
        LIBXML2_CFLAGS=-I$SHELL_FOLDER/output/include/libxml2 \
        LIBXML2_LIBS="-L$SHELL_FOLDER/output/lib -lxml2" \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/fontconfig-2.13.94
}

build_libffi()
{
    # 编译libffi
    echo "----------------------------- 编译libffi -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf libffi-3.4.2.tar.gz
    cd $SHELL_FOLDER/libffi-3.4.2
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/libffi-3.4.2
}

build_alsa_lib()
{
    # 编译alsa-lib
    echo "---------------------------- 编译alsa-lib ----------------------------"
    cd $SHELL_FOLDER
    tar -jxvf alsa-lib-1.2.5.tar.bz2
    cd $SHELL_FOLDER/alsa-lib-1.2.5
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=/ \
        --disable-static \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install DESTDIR=$SHELL_FOLDER/output
    rm -rf $SHELL_FOLDER/alsa-lib-1.2.5
}

build_alsa_utils()
{
    # 编译alsa-utils
    echo "--------------------------- 编译alsa-utils ---------------------------"
    cd $SHELL_FOLDER
    tar -jxvf alsa-utils-1.2.5.1.tar.bz2
    cd $SHELL_FOLDER/alsa-utils-1.2.5.1
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=/ \
        --with-alsa-inc-prefix=$SHELL_FOLDER/output/include \
        --with-alsa-prefix=$SHELL_FOLDER/output/lib \
        --disable-alsamixer \
        --disable-xmlto \
        --disable-nls \
        --disable-bat \
        --with-udev-rules-dir=/lib/udev \
        --with-asound-state-dir=/var/lib/alsa \
        --disable-alsaconf \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install DESTDIR=$SHELL_FOLDER/output
    rm -rf $SHELL_FOLDER/alsa-utils-1.2.5.1
}

build_openjdk_zero()
{
    echo "----------------------------- 编译openjdk -----------------------------"
    if [ ! -d "$SHELL_FOLDER/host_output/opt/jdk-10" ]; then
        if [ ! -d "$SHELL_FOLDER/host_output" ]; then
            mkdir $SHELL_FOLDER/host_output
        fi            
        if [ ! -d "$SHELL_FOLDER/host_output/opt" ]; then
            mkdir $SHELL_FOLDER/host_output/opt
        fi
        cd $SHELL_FOLDER/host_output/opt
        wget https://download.java.net/openjdk/jdk10/ri/jdk-10_linux-x64_bin_ri.tar.gz
        tar -xzvf jdk-10_linux-x64_bin_ri.tar.gz
        rm -rf jdk-10_linux-x64_bin_ri.tar.gz
    fi
    cd $SHELL_FOLDER/jdk11u-dev-113c646a33d2
    TEMP_PATH=$PATH
	export PATH=$PATH:$CROSS_COMPILE_DIR/bin
    bash configure \
        --with-boot-jdk=$SHELL_FOLDER/host_output/opt/jdk-10 \
        --with-jvm-variants=zero \
        --openjdk-target=riscv64-unknown-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --enable-headless-only=yes \
        --disable-warnings-as-errors \
        --with-alsa=$SHELL_FOLDER/output \
        --with-x=$SHELL_FOLDER/output \
        --with-cups=$SHELL_FOLDER/output \
        --with-fontconfig=$SHELL_FOLDER/output \
        --with-libffi=$SHELL_FOLDER/output \
        --with-freetype-include=$SHELL_FOLDER/output/include/freetype2 \
        --with-freetype-lib=$SHELL_FOLDER/output/lib
    make
	export PATH=$TEMP_PATH
    if [ ! -d "$SHELL_FOLDER/output/opt" ]; then
        mkdir $SHELL_FOLDER/output/opt
    fi
    mv ./build/linux-riscv64-normal-zero-release/jdk $SHELL_FOLDER/output/opt/open-jdk11
}

build_libuuid()
{
    # 编译libuuid
    echo "----------------------------- 编译libuuid -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf libuuid-1.0.3.tar.gz
    cd $SHELL_FOLDER/libuuid-1.0.3
    patch -p1 < ../libuuid-1.0.3.patch
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/libuuid-1.0.3
}

build_lzo()
{
    # 编译lzo
    echo "------------------------------- 编译lzo -------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf lzo-2.10.tar.gz
    cd $SHELL_FOLDER/lzo-2.10
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --enable-shared \
        --disable-static \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/lzo-2.10
}

build_attr()
{
    # 编译attr
    echo "------------------------------ 编译attr ------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf attr-2.5.1.tar.gz
    cd $SHELL_FOLDER/attr-2.5.1
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/attr-2.5.1
}

build_dtc()
{
    # 编译dtc-1.6.1
    echo "------------------------------- 编译dtc -------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf dtc-1.6.1.tar.gz
    cd $SHELL_FOLDER/dtc-1.6.1
	make CC=${CROSS_PREFIX}-gcc PREFIX=$SHELL_FOLDER/output NO_PYTHON=1 libfdt -j$PROCESSORS
    make CC=${CROSS_PREFIX}-gcc PREFIX=$SHELL_FOLDER/output NO_PYTHON=1 install-lib install-includes
    rm  $SHELL_FOLDER/output/lib/libfdt.a
    rm -rf $SHELL_FOLDER/dtc-1.6.1
}

build_kvmtool()
{
    # 编译kvmtool
    cd $SHELL_FOLDER/kvmtool
	make ARCH=riscv CROSS_COMPILE=$CROSS_PREFIX- LIBFDT_PATH=$SHELL_FOLDER/output lkvm -j$PROCESSORS
	cp lkvm $SHELL_FOLDER/output/bin/lkvm
}

build_gdb()
{
    # 编译gdb-8.2.1
    echo "------------------------------- 编译gdb -------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf gdb-10.1.tar.gz
    cd $SHELL_FOLDER/gdb-10.1
    TEMP_PATH=$PATH
	export PATH=$PATH:$CROSS_COMPILE_DIR/bin
    export CXX=$CROSS_PREFIX-g++ 
    export CC=$CROSS_PREFIX-gcc 
    export AR=$CROSS_PREFIX-ar
    ./configure \
        --host=riscv64-linux-gnu \
        --disable-ld \
        --disable-gas \
        --disable-sim \
        --disable-gprofng \
        --prefix=$SHELL_FOLDER/output/opt/gdb
    make -j$PROCESSORS
    make install-gdb install-gdbserver
	export PATH=$TEMP_PATH
    rm -rf $SHELL_FOLDER/gdb-10.1
}

build_trace_cmd()
{
    # 编译trace-cmd-v2.9.5
    echo "---------------------------- 编译trace-cmd ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf trace-cmd-v2.9.5.tar.gz
    cd $SHELL_FOLDER/trace-cmd-v2.9.5
	make CROSS_COMPILE=${CROSS_PREFIX}- CC=${CROSS_PREFIX}-gcc AR=${CROSS_PREFIX}-ar prefix=/ libdir_relative=lib -j$PROCESSORS
	make CROSS_COMPILE=${CROSS_PREFIX}- CC=${CROSS_PREFIX}-gcc AR=${CROSS_PREFIX}-ar prefix=$SHELL_FOLDER/output etcdir=$SHELL_FOLDER/output/etc libdir_relative=lib install
    rm -rf $SHELL_FOLDER/output/lib/libtraceevent.a
    rm -rf $SHELL_FOLDER/trace-cmd-v2.9.5
}

build_lrzsz()
{
    # 编译lrzsz-0.12.20
    echo "------------------------------ 编译lrzsz ------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf lrzsz-0.12.20.tar.gz
    cd $SHELL_FOLDER/lrzsz-0.12.20
    export CXX=$CROSS_PREFIX-g++
    export CC=$CROSS_PREFIX-gcc 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --mandir=$SHELL_FOLDER/output/share/man
    make -j$PROCESSORS
    make install
    unset CXX
    unset CC
    rm -rf $SHELL_FOLDER/lrzsz-0.12.20
}

build_libexpat()
{
    echo "---------------------------- 编译libexpat ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf expat-2.4.3.tar.gz
    cd $SHELL_FOLDER/expat-2.4.3
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/expat-2.4.3
}

build_libdaemon()
{
    echo "---------------------------- 编译libdaemon ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf libdaemon-0.14.tar.gz
    cd $SHELL_FOLDER/libdaemon-0.14
    autoreconf -f -i 
    echo "ac_cv_func_setpgrp_void=yes" > config.cache
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        --config-cache \
        --disable-lynx \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/libdaemon-0.14
}

build_avahi()
{
    # 编译avahi
    echo "------------------------------ 编译avahi ------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf avahi-0.7.tar.gz
    cd $SHELL_FOLDER/avahi-0.7
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=/ \
        --disable-static \
        --with-xml=expat \
        --enable-libdaemon \
        --with-distro=none \
        --disable-glib \
        --disable-gobject \
        --disable-qt3 \
        --disable-qt4 \
        --disable-gtk \
        --disable-gtk3 \
        --disable-dbus \
        --disable-gdbm \
        --disable-python \
        --disable-pygtk \
        --disable-python-dbus \
        --disable-mono \
        --disable-monodoc \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc \
        CPPFLAGS="-I$SHELL_FOLDER/output/include" \
        LDFLAGS="-L$SHELL_FOLDER/output/lib" \
        LIBDAEMON_CFLAGS="-I$SHELL_FOLDER/output/include" \
        LIBDAEMON_LIBS="-L$SHELL_FOLDER/output/lib -ldaemon"
    make -j$PROCESSORS
    make install DESTDIR=$SHELL_FOLDER/output
    rm -rf $SHELL_FOLDER/avahi-0.7
}

build_iperf3()
{
    # 编译iperf3
    echo "----------------------------- 编译iperf3 -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf iperf-3.10.1.tar.gz
    cd $SHELL_FOLDER/iperf-3.10.1
    patch -p1 < ../iperf-3.10.1.patch
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --enable-static-bin \
        --without-openssl \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc
    make -j$PROCESSORS
    make install
    rm  $SHELL_FOLDER/output/lib/libiperf.a
    rm -rf $SHELL_FOLDER/iperf-3.10.1
}

build_util_linux()
{
    # 编译util-linux
    echo "--------------------------- 编译util-linux ---------------------------"
    cd $SHELL_FOLDER
    tar -xzvf util-linux-2.38.tar.gz
    cd $SHELL_FOLDER/util-linux-2.38
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=/ \
        --with-bashcompletiondir=/share/bash-completion/completions \
        --disable-static \
        --disable-libuuid \
        --without-libz \
        --without-tinfo \
        --without-python \
        --disable-makeinstall-chown \
        CCFLAGS=-I$SHELL_FOLDER/output/include \
        LDFLAGS=-L$SHELL_FOLDER/output/lib \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install DESTDIR=$SHELL_FOLDER/output
    rm -rf $SHELL_FOLDER/util-linux-2.38
}

build_iproute2()
{
    # 编译iproute2
    echo "---------------------------- 编译iproute2 ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf iproute2-5.9.0.tar.gz
    cd $SHELL_FOLDER/iproute2-5.9.0
    patch -p1 < ../iproute2-5.9.0.patch
    PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig CC=$CROSS_PREFIX-gcc AR=$CROSS_PREFIX-ar ./configure
    make -j$PROCESSORS
    PREFIX=/ make install DESTDIR=$SHELL_FOLDER/output
    rm -rf $SHELL_FOLDER/iproute2-5.9.0
}

build_can_utils()
{
    # 编译can-utils
    echo "---------------------------- 编译can-utils ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf can-utils-2021.08.0.tar.gz
    cd $SHELL_FOLDER/can-utils-2021.08.0
    mkdir build
    cd build
    CC=$CROSS_PREFIX-gcc cmake -DCMAKE_INSTALL_PREFIX=$SHELL_FOLDER/output ..
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/can-utils-2021.08.0
}

build_confuse()
{
    # 编译confuse
    echo "----------------------------- 编译confuse -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf confuse-3.3.tar.gz
    cd $SHELL_FOLDER/confuse-3.3
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/confuse-3.3
}

build_libite()
{
    # 编译libite
    echo "----------------------------- 编译libite -----------------------------"    
    cd $SHELL_FOLDER
    tar -xzvf libite-2.5.2.tar.gz
    cd $SHELL_FOLDER/libite-2.5.2
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        --without-symlink \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/libite-2.5.2
}

build_libuev()
{
    # 编译libuev
    echo "----------------------------- 编译libuev -----------------------------"    
    cd $SHELL_FOLDER
    tar -xzvf libuev-2.4.0.tar.gz
    cd $SHELL_FOLDER/libuev-2.4.0
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/libuev-2.4.0
}

build_watchdogd()
{
    # 编译watchdogd
    echo "---------------------------- 编译watchdogd ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf watchdogd-3.5.tar.gz
    cd $SHELL_FOLDER/watchdogd-3.5
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=/ \
        --disable-static \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install DESTDIR=$SHELL_FOLDER/output
    rm -rf $SHELL_FOLDER/watchdogd-3.5
}

build_mtd_utils()
{
    # 编译mtd-utils
    echo "---------------------------- 编译mtd-utils ----------------------------"
    cd $SHELL_FOLDER
    tar -jxvf mtd-utils-2.1.2.tar.bz2
    cd $SHELL_FOLDER/mtd-utils-2.1.2
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --with-jffs \
        --with-ubifs \
        --without-zstd \
        --enable-install-tests \
        CFLAGS="-I$SHELL_FOLDER/output/include -I$SHELL_FOLDER/output/include/uuid" \
        LIBS="-L$SHELL_FOLDER/output/lib" \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig:$SHELL_FOLDER/output/share/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc
	make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/mtd-utils-2.1.2
}

build_coreutils()
{
    # 编译coreutils
    echo "---------------------------- 编译coreutils ----------------------------"
    cd $SHELL_FOLDER
    tar -xvJf coreutils-9.1.tar.xz
    cd $SHELL_FOLDER/coreutils-9.1
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=/ \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install DESTDIR=$SHELL_FOLDER/output
    rm -rf $SHELL_FOLDER/coreutils-9.1
}

build_i2c_tools()
{
    # 编译i2c_tools
    echo "---------------------------- 编译i2c_tools ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf i2c-tools-3.0.2.tar.gz
    cd $SHELL_FOLDER/i2c-tools-3.0.2
    make prefix=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc -j$PROCESSORS
    make prefix=$SHELL_FOLDER/output install 
    rm -rf $SHELL_FOLDER/i2c-tools-3.0.2
}

build_libgpiod()
{
    # 编译libgpiod
    echo "---------------------------- 编译libgpiod ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf libgpiod-1.6.3.tar.gz
    cd $SHELL_FOLDER/libgpiod-1.6.3
    ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes ./autogen.sh \
        --enable-tools=yes \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/libgpiod-1.6.3
}

build_libusb()
{
    # 编译libusb
    echo "----------------------------- 编译libusb -----------------------------"
    cd $SHELL_FOLDER
    tar -jxvf libusb-1.0.24.tar.bz2
    cd $SHELL_FOLDER/libusb-1.0.24
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        --disable-udev \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/libusb-1.0.24
}

build_strace()
{
    # 编译strace
    echo "----------------------------- 编译strace -----------------------------"
    cd $SHELL_FOLDER
    tar -xvJf strace-5.13.tar.xz
    cd $SHELL_FOLDER/strace-5.13
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/strace-5.13
}

build_libnl()
{
    # 编译libnl
    echo "------------------------------ 编译libnl ------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf libnl-3.2.25.tar.gz
    cd $SHELL_FOLDER/libnl-3.2.25
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/libnl-3.2.25
}

build_readline()
{
    # 编译readline
    echo "---------------------------- 编译readline ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf readline-8.1.2.tar.gz
    cd $SHELL_FOLDER/readline-8.1.2
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/readline-8.1.2
}

build_libpcap()
{
    # 编译libpcap
    echo "----------------------------- 编译libpcap -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf libpcap-1.10.1.tar.gz
    cd $SHELL_FOLDER/libpcap-1.10.1
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --disable-static \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/output/lib/libpcap.a
    rm -rf $SHELL_FOLDER/libpcap-1.10.1
}

build_dropwatch()
{
    # 编译dropwatch
    echo "---------------------------- 编译dropwatch ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf dropwatch-1.5.4.tar.gz
    cd $SHELL_FOLDER/dropwatch-1.5.4
    ./autogen.sh 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        --without-bfd \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig \
        READLINE_CFLAGS="-I$SHELL_FOLDER/output/include" \
        READLINE_LIBS="-L$SHELL_FOLDER/output/lib -lreadline -lncurses" \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
	make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/dropwatch-1.5.4
}

build_tcpdump()
{
    # 编译tcpdump
    echo "----------------------------- 编译tcpdump -----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf tcpdump-4.99.1.tar.gz
    cd $SHELL_FOLDER/tcpdump-4.99.1
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        PKG_CONFIG_PATH=$SHELL_FOLDER/output/lib/pkgconfig \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/tcpdump-4.99.1
}

build_spi_tools()
{
    # 编译spi-tools
    echo "---------------------------- 编译spi-tools ----------------------------"
    cd $SHELL_FOLDER
    tar -xzvf spi-tools-1.0.1.tar.gz
    cd $SHELL_FOLDER/spi-tools-1.0.1
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/spi-tools-1.0.1
}

build_ell()
{
    # 编译ell
    echo "------------------------------- 编译ell -------------------------------"
    cd $SHELL_FOLDER
    tar -xzvf ell-0.5.1.tar.gz
    cd $SHELL_FOLDER/ell-0.5.1
    autoreconf -f -i 
    ./configure \
        --host=riscv64-linux-gnu \
        --prefix=$SHELL_FOLDER/output \
        CXX=$CROSS_PREFIX-g++ \
        CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
    rm -rf $SHELL_FOLDER/ell-0.5.1
}

case "$1" in
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
libuuid)
    build_libuuid
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
alsa_lib)
    build_alsa_lib
    ;;
alsa_utils)
    build_alsa_utils
    ;;
openjdk_zero)
    build_openjdk_zero
    ;;
lzo)
    build_lzo
    ;;
attr)
    build_attr
    ;;
dtc)
    build_dtc
    ;;
kvmtool)
    build_kvmtool
    ;;
gdb)
    build_gdb
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
util_linux)
    build_util_linux
    ;;
iproute2)
    build_iproute2
    ;;
can-utils)
    build_can_utils
    ;;
libuev)
    build_libuev
    ;;
libite)
    build_libite
    ;;
confuse)
    build_confuse
    ;;
watchdogd)
    build_watchdogd
    ;;
mtd_utils)
    build_mtd_utils
    ;;
coreutils)
    build_coreutils
    ;;
i2c_tools)
    build_i2c_tools
    ;;
libgpiod)
    build_libgpiod
    ;;
libusb)
    build_libusb
    ;;
strace)
    build_strace
    ;;
libnl)
    build_libnl
    ;;
readline)
    build_readline
    ;;
libpcap)
    build_libpcap
    ;;
dropwatch)
    build_dropwatch
    ;;
tcpdump)
    build_tcpdump
    ;;
spi_tools)
    build_spi_tools
    ;;
ell)
    build_ell
    ;;
all)
    build_make
    build_ncurses
    build_bash
    build_sudo
    build_screenfetch
    build_tree
    build_libevent
    build_screen
    build_cu
	build_libmnl
	build_ethtool
	build_zlib
	build_openssl
    build_openssh
	build_libpng
	build_freetype
    build_libuuid
    build_libx11
    build_cups
	build_libxml2
	build_fontconfig
	build_libffi
	build_alsa_lib
    build_alsa_utils
    build_openjdk_zero
    build_lzo
    build_attr
    build_dtc
    build_kvmtool
    build_gdb
    build_trace_cmd
    build_lrzsz
    build_libexpat
    build_libdaemon
    build_avahi
    build_iperf3
    build_util_linux
    build_iproute2
    build_can_utils
    build_libuev
    build_libite
    build_confuse
    build_watchdogd
    build_mtd_utils
    build_coreutils
    build_i2c_tools
    build_libgpiod
    build_libusb
    build_strace
    build_libnl
    build_readline
    build_libpcap
    build_dropwatch
    build_tcpdump
    build_spi_tools
    build_ell
	build_qt
    ;;
*)
    echo "Please enter the built package name or use \"all\" !"
    exit 1
	;;
esac

echo "----------------------------------- 完成 -----------------------------------"
