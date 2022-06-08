#!/bin/bash
set -e

MODE=\
"config_vcan | \
release_vcan"

USER_NAME=$(whoami)
USAGE="usage $0 [$MODE] [<CAN_NAME>] "

if [ $# == 2 ] ; then
	CAN_NAME=$1
else
    CAN_NAME=vcan0
fi

config_vcan()
{
    modprobe vcan
    ip link add dev $CAN_NAME type vcan
    ip link set up $CAN_NAME
}

release_vcan()
{
    ip link set down $CAN_NAME
    ip link delete dev $CAN_NAME
}

case "$1" in
config_vcan)
    config_vcan
	;;
release_vcan)
    release_vcan
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
