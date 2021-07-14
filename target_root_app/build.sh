SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
CROSS_COMPILE_DIR=/opt/riscv64--glibc--bleeding-edge-2020.08-1
CROSS_PREFIX=$CROSS_COMPILE_DIR/bin/riscv64-linux

case "$1" in
skip)
    CONFIGURE=echo
    ;;
*)
    CONFIGURE=./configure
	;;
esac

# 编译bash
cd $SHELL_FOLDER/bash-5.1.8
$CONFIGURE --host=riscv64 --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
make install

# 编译make
cd $SHELL_FOLDER/make-4.3
$CONFIGURE --host=riscv64 --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
make install

# 编译ncurses
cd $SHELL_FOLDER/ncurses-6.2
$CONFIGURE --host=riscv64 --disable-stripping CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
#make install.progs
#make install.data
