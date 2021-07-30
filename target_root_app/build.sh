SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
PROCESSORS=`cat /proc/cpuinfo |grep "processor"|wc -l`
CROSS_COMPILE_DIR=/opt/gcc-riscv64-unknown-linux-gnu
CROSS_PREFIX=$CROSS_COMPILE_DIR/bin/riscv64-unknown-linux-gnu

case "$2" in
skip)
    CONFIGURE=echo
    ;;
*)
    CONFIGURE=./configure
	;;
esac

build_make()
{
    # 编译make
    echo "\033[1;4;41;32m编译make\033[0m"
    cd $SHELL_FOLDER/make-4.3
    $CONFIGURE --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
}

build_ncurses()
{
    # 编译ncurses
    echo "\033[1;4;41;32m编译ncurses\033[0m"
    cd $SHELL_FOLDER/ncurses-6.2
    $CONFIGURE --host=riscv64-linux-gnu --with-shared --without-normal --without-debug CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make  install.libs DESTDIR=$SHELL_FOLDER/output
    #make install.progs
    #make install.data
}

build_bash()
{
    # 编译bash
    echo "\033[1;4;41;32m编译bash\033[0m"
    cd $SHELL_FOLDER/bash-5.1.8
    $CONFIGURE --host=riscv64 --prefix=$SHELL_FOLDER/output CCFLAGS=-I$SHELL_FOLDER/output/usr/include LDFLAGS=-L$SHELL_FOLDER/output/usr/lib CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
}

build_sudo()
{
    # 编译sudo
    echo "\033[1;4;41;32m编译sudo\033[0m"
    cd $SHELL_FOLDER/sudo-SUDO_1_9_7p1
    $CONFIGURE --host=riscv64-linux-gnu CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    #make install-binaries
}

build_screenfetch()
{
    # 编译screenFetch
    echo "\033[1;4;41;32m编译screenFetch\033[0m"
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
    echo "\033[1;4;41;32m编译tree\033[0m"
    cd $SHELL_FOLDER/tree-1.8.0
    make prefix=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc -j$PROCESSORS
    make prefix=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc install
}

build_libevent()
{
    # 编译libevent
    echo "\033[1;4;41;32m编译libevent\033[0m"
    cd $SHELL_FOLDER/libevent-2.1.12-stable
    $CONFIGURE --host=riscv64-linux-gnu --disable-openssl --disable-static --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    make install
}

build_screen()
{
    # 编译screen
    echo "\033[1;4;41;32m编译screen\033[0m"
    cd $SHELL_FOLDER/screen-4.8.0
    $CONFIGURE --host=riscv64-linux-gnu CCFLAGS=-I$SHELL_FOLDER/output/usr/include LDFLAGS=-L$SHELL_FOLDER/output/usr/lib CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j$PROCESSORS
    #make install
}

build_cu()
{
    # 编译cu
    echo "\033[1;4;41;32m编译cu\033[0m"
    cd $SHELL_FOLDER/cu
    make prefix=$SHELL_FOLDER/output LIBEVENTDIR=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc -j$PROCESSORS
    make prefix=$SHELL_FOLDER/output LIBEVENTDIR=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc install
}

build_qt()
{
    # 编译qt
    echo "\033[1;4;41;32m编译qt\033[0m"
    cd $SHELL_FOLDER/qt-everywhere-src-5.12.11
	export PATH=$PATH:$CROSS_COMPILE_DIR/bin
	$CONFIGURE -opensource -confirm-license -release -optimize-size -strip -ltcg -silent -qpa linuxfb -no-opengl -skip webengine -nomake tools -nomake tests -nomake examples -xplatform linux-riscv64-gnu-g++ -prefix /opt/Qt-5.12.11 -extprefix $SHELL_FOLDER/host_output
	make -j$PROCESSORS
	make install
}

case "$1" in
bash)
    build_bash
    ;;
make)
    build_make
    ;;
ncurses)
    build_ncurses
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
	build_qt
    ;;
*)
    echo "Please enter the built package name or use \"all\" !"
    exit 1
	;;
esac

echo "\033[1;4;41;32m完成\033[0m"
