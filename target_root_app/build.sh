SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
CROSS_COMPILE_DIR=/opt/riscv64--glibc--bleeding-edge-2020.08-1
CROSS_PREFIX=$CROSS_COMPILE_DIR/bin/riscv64-linux

case "$2" in
skip)
    CONFIGURE=echo
    ;;
*)
    CONFIGURE=./configure
	;;
esac

build_bash()
{
    # 编译bash
    cd $SHELL_FOLDER/bash-5.1.8
    $CONFIGURE --host=riscv64 --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j16
    make install
}

build_make()
{
    # 编译make
    cd $SHELL_FOLDER/make-4.3
    $CONFIGURE --host=riscv64 --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j16
    make install
}

build_ncurses()
{
    # 编译ncurses
    cd $SHELL_FOLDER/ncurses-6.2
    $CONFIGURE --host=riscv64 --disable-stripping CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j16
    #make install.progs
    #make install.data
}

build_sudo()
{
    # 编译sudo
    cd $SHELL_FOLDER/sudo-SUDO_1_9_7p1
    $CONFIGURE --host=riscv CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j16
    #make install-binaries
}

build_screenfetch()
{
    # 安装screenFetch
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
    cd $SHELL_FOLDER/tree-1.8.0
    make prefix=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc -j16
    make prefix=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc install
}

build_libevent()
{
    # 编译libevent
    cd $SHELL_FOLDER/libevent-2.1.12-stable
    $CONFIGURE --host=riscv --disable-openssl --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j16
    make install
}

build_screen()
{
    # 编译screen
    cd $SHELL_FOLDER/screen-4.8.0
    $CONFIGURE --host=riscv64 CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
    make -j16
    #make install
}

build_cu()
{
    # 编译cu
    cd $SHELL_FOLDER/cu
    make prefix=$SHELL_FOLDER/output LIBEVENTDIR=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc -j16
    make prefix=$SHELL_FOLDER/output LIBEVENTDIR=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc install
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
all)
    build_bash
    build_make
    build_ncurses
    build_sudo
    build_screenfetch
    build_tree
    build_libevent
    build_screen
    build_cu
    ;;
*)
    echo "Please enter the built package name or use \"all\" !"
	;;
esac
