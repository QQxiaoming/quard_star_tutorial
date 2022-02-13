#!/bin/bash
set -e

MODE=\
"config_tap | \
release_tap"

USER_NAME=$(whoami)
USAGE="usage $0 [$MODE] [<ETH_NAME>] "

if [ $# == 2 ] ; then
	ETH_NAME=$1
else
    ETH_NAME=enp2s0f0 #eth0
fi

config_tap()
{
    brctl addbr br0
    ip addr flush dev $ETH_NAME
    brctl addif br0 $ETH_NAME
    tunctl -t tap0 -u $USER_NAME
    brctl addif br0 tap0
    ifconfig $ETH_NAME up
    ifconfig tap0 up
    ifconfig br0 up
    ip addr flush dev br0
    ip addr flush dev tap0
    ip addr flush dev $ETH_NAME

    # static
    ip addr add 169.254.105.176/16 broadcast 169.254.255.255 dev br0
    # dynamic
    #dhclient -v br0
}

release_tap()
{
    brctl delif br0 tap0
    tunctl -d tap0
    brctl delif br0 $ETH_NAME
    ifconfig br0 down
    brctl delbr br0
    ifconfig $ETH_NAME up

    # static
    ip addr add 169.254.105.176/16 broadcast 169.254.255.255 dev $ETH_NAME
    # dynamic
    #dhclient -v $ETH_NAME
}

case "$1" in
config_tap)
    config_tap
	;;
release_tap)
    release_tap
	;;
--help)
	echo $USAGE
	exit 0
	;;
*)
	echo $USAGE
	exit 1	
	;;
esac
