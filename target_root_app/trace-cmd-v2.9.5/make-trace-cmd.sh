#!/bin/bash

if [ -z "$INSTALL_PATH" ]; then
	echo
	echo 'Error: No $INSTALL_PATH defined'
	echo
	echo "   usage: [PREFIX=prefix][BUILD_PATH=/path/to/build] INSTALL_PATH=/path/to/install make-trace-cmd.sh install|install_libs|clean|uninstall"
	echo
	echo "     Used to create a self contained directory to copy to other machines."
	echo
	echo "   Please read PACKAGING for more information."
	echo
	exit
fi

if [ ! -d $INSTALL_PATH ]; then
	mkdir $INSTALL_PATH
fi

if [ ! -z "$BUILD_PATH" ]; then
	if [ ! -d $BUILD_PATH ]; then
		mkdir $BUILD_PATH
	fi
	O_PATH="O=$BUILD_PATH"
fi

if [ -z "$PREFIX" ]; then
	PREFIX="/usr"
fi

PKG_PATH=`pkg-config --variable pc_path pkg-config | tr ":" " " | cut -d' ' -f1`

WITH_PATH=""
# If pkg-config supports --with-path, use that as well
if pkg-config --with-path=/tmp --variable pc_path pkg-config &> /dev/null ; then
	WITH_PATH="--with-path=$INSTALL_PATH$PKG_PATH"
fi

PKG_CONFIG_PATH="$INSTALL_PATH/$PKG_PATH" PKG_CONFIG="pkg-config $WITH_PATH --define-variable=prefix=$INSTALL_PATH/$PREFIX" CFLAGS="-g -Wall -I$INSTALL_PATH/$PREFIX/include" make DESTDIR=$INSTALL_PATH  $O_PATH prefix=$PREFIX $@
