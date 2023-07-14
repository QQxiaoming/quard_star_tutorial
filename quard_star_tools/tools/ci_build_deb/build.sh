#!/bin/sh

./linuxdeploy-x86_64.AppImage --executable=../../release/out/quard_star_tools_temp --appdir=../../release/out --plugin=qt
rm -rf ../../release/out/apprun-hooks
