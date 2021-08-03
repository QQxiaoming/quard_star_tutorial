SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
PROCESSORS=`cat /proc/cpuinfo |grep "processor"|wc -l`

install_make()
{
    cp $SHELL_FOLDER/output/bin/make /bin/make
}

install_ncurses()
{
    cd $SHELL_FOLDER/ncurses-6.2
    make install.progs
    make install.data
    cd /usr/lib
    cp $SHELL_FOLDER/output/usr/lib/libpanel.so.6.2 /usr/lib/libpanel.so.6.2
    cp $SHELL_FOLDER/output/usr/lib/libncurses.so.6.2 /usr/lib/libncurses.so.6.2
    cp $SHELL_FOLDER/output/usr/lib/libmenu.so.6.2 /usr/lib/libmenu.so.6.2
    cp $SHELL_FOLDER/output/usr/lib/libform.so.6.2 /usr/lib/libform.so.6.2
    ln -s libpanel.so.6.2 libpanel.so.6
    ln -s libpanel.so.6 libpanel.so
    ln -s libncurses.so.6.2 libncurses.so.6
    ln -s libncurses.so.6 libncurses.so
    ln -s libmenu.so.6.2 libmenu.so.6
    ln -s libmenu.so.6 libmenu.so
    ln -s libform.so.6.2 libform.so.6
    ln -s libform.so.6 libform.so
    ln -s libncurses.so.6.2 libcurses.so
}

install_bash()
{
    cp $SHELL_FOLDER/output/bin/bash /bin/bash
}

install_sudo()
{
    cd $SHELL_FOLDER/sudo-SUDO_1_9_7p1
    make install-binaries
}

install_screenfetch()
{
    cp $SHELL_FOLDER/output/usr/bin/screenfetch /usr/bin/screenfetch
}

install_tree()
{
    cp $SHELL_FOLDER/output/bin/tree /usr/bin/tree
}

install_libevent()
{
    cd /usr/lib
    cp $SHELL_FOLDER/output/lib/libevent-2.1.so.7.0.1 /usr/lib/libevent-2.1.so.7.0.1
    cp $SHELL_FOLDER/output/lib/libevent_core-2.1.so.7.0.1 /usr/lib/libevent_core-2.1.so.7.0.1
    cp $SHELL_FOLDER/output/lib/libevent_extra-2.1.so.7.0.1 /usr/lib/libevent_extra-2.1.so.7.0.1
    cp $SHELL_FOLDER/output/lib/libevent_pthreads-2.1.so.7.0.1 /usr/lib/libevent_pthreads-2.1.so.7.0.1
    ln -s libevent-2.1.so.7.0.1 libevent-2.1.so.7
    ln -s libevent-2.1.so.7.0.1 libevent.so
    ln -s libevent_core-2.1.so.7.0.1 libevent_core-2.1.so.7
    ln -s libevent_core-2.1.so.7.0.1 libevent_core.so
    ln -s libevent_extra-2.1.so.7.0.1 libevent_extra-2.1.so.7
    ln -s libevent_extra-2.1.so.7.0.1 libevent_extra.so
    ln -s libevent_pthreads-2.1.so.7.0.1 libevent_pthreads-2.1.so.7
    ln -s libevent_pthreads-2.1.so.7.0.1 libevent_pthreads.so
}

install_screen()
{
    cd $SHELL_FOLDER/screen-4.8.0
    make install
}

install_cu()
{
    cp $SHELL_FOLDER/output/bin/cu /usr/bin/cu
}

install_qt()
{
    if [ ! -d "/opt" ]; then  
        mkdir /opt
    fi
    rm -rf /opt/Qt-5.12.11
    mkdir /opt/Qt-5.12.11
    mkdir /opt/Qt-5.12.11/lib
    mkdir /opt/Qt-5.12.11/plugins
    #cp -r $SHELL_FOLDER/host_output/plugins /opt/Qt-5.12.11/
    cp -r $SHELL_FOLDER/host_output/plugins/platforms /opt/Qt-5.12.11/plugins/
    cp -r $SHELL_FOLDER/host_output/plugins/generic /opt/Qt-5.12.11/plugins/
    cp $SHELL_FOLDER/host_output/lib/*.so.5.12.11 /opt/Qt-5.12.11/lib/
    cd /opt/Qt-5.12.11/lib
    for file in `ls /opt/Qt-5.12.11/lib`
    do
        if [ -f $file ]; then  
            filename0=${file%.*.*.*}
            filename1=${file%.*.*}
            filename2=${file%.*}
            ln -s $file $filename2
            ln -s $filename2 $filename1
            ln -s $filename1 $filename0
        fi
    done
}


install_libmnl()
{
	cd /lib
	cp $SHELL_FOLDER/output/lib/libmnl.so.0.2.0 /lib/libmnl.so.0.2.0
	ln -s libmnl.so.0.2.0 libmnl.so.0
	ln -s libmnl.so.0 libmnl.so
}

install_ethtool()
{
	cp $SHELL_FOLDER/output/sbin/ethtool /sbin/ethtool
}

install_openssl()
{
	cd /lib
	cp $SHELL_FOLDER/output/bin/openssl /bin/openssl
	cp $SHELL_FOLDER/output/lib/libssl.so.1.1 /lib/libssl.so.1.1
	cp $SHELL_FOLDER/output/lib/libcrypto.so.1.1 /lib/libcrypto.so.1.1
	ln -s libssl.so.1.1 libssl.so
	ln -s libcrypto.so.1.1 libcrypto.so
}

install_iperf()
{
	cd /lib
	cp $SHELL_FOLDER/output/bin/iperf3 /bin/iperf3
	cp $SHELL_FOLDER/output/lib/libiperf.so.0.0.0 /lib/libiperf.so.0.0.0
	ln -s libiperf.so.0.0.0 libiperf.so.0
	ln -s libiperf.so.0 libiperf.so
}

install_zlib()
{
	cd /lib
	cp $SHELL_FOLDER/output/lib/libz.so.1.2.11 /lib/libz.so.1.2.11
	ln -s libz.so.1.2.11 libz.so.1
	ln -s libz.so.1 libz.so
}

install_openssh()
{
    cd $SHELL_FOLDER/openssh-8.6p1
    make install
}

case "$1" in
bash)
    install_bash
    ;;
make)
    install_make
    ;;
ncurses)
    install_ncurses
    ;;
sudo)
    install_sudo
    ;;
screenfetch)
    install_screenfetch
    ;;
tree)
    install_tree
    ;;
libevent)
    install_libevent
    ;;
screen)
    install_screen
    ;;
cu)
    install_cu
    ;;
qt)
    install_qt
    ;;
libmnl)
    install_libmnl
    ;;
ethtool)
    install_ethtool
    ;;
openssl)
    install_openssl
    ;;
iperf)
    install_iperf
    ;;
zlib)
    install_zlib
    ;;
openssh)
    install_openssh
    ;;
all)
    install_make
    install_ncurses
    install_bash
    install_sudo
    install_screenfetch
    install_tree
    install_libevent
    install_screen
    install_cu
	install_qt
    install_libmnl
    install_ethtool
    install_openssl
    install_iperf
	install_zlib
	install_openssh
    ;;
*)
    echo "Please enter the built package name or use \"all\" !"
    exit 1
	;;
esac
