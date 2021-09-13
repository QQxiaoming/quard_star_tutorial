SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
GDB_DIR=/opt/gcc-riscv64-unknown-linux-gnu/bin

case "$1" in
qemu)
	$SHELL_FOLDER/run.sh kgdb
	;;
klog)
	telnet localhost 3441
    ;;
kgdb)
	$GDB_DIR/riscv64-unknown-linux-gnu-gdb -x $SHELL_FOLDER/gdb.script $SHELL_FOLDER/linux-5.10.42/vmlinux 
	;;
--help)
	echo "usage qemu klog kgdb"
	exit 0
	;;
*)
	echo "usage qemu klog kgdb"
	exit 1
	;;
esac







