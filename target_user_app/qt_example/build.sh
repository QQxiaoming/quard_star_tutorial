SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
PROCESSORS=`cat /proc/cpuinfo |grep "processor"|wc -l`
CROSS_COMPILE_DIR=/opt/gcc-riscv64-unknown-linux-gnu
CROSS_PREFIX=$CROSS_COMPILE_DIR/bin/riscv64-unknown-linux-gnu
CROSS_QT_TOOLS_DIR=$SHELL_FOLDER/../../target_root_app/host_output
export PATH=$PATH:$CROSS_COMPILE_DIR/bin

cd $SHELL_FOLDER/analogclock
$CROSS_QT_TOOLS_DIR/bin/qmake -makefile
make -j$PROCESSORS

cd $SHELL_FOLDER/rasterwindow
$CROSS_QT_TOOLS_DIR/bin/qmake -makefile
make -j$PROCESSORS
