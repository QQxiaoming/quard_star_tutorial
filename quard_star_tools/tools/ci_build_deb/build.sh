#!/bin/sh

./linuxdeployqt ../../release/out/quard_star_tools_temp -appimage
rm -rf ../../release/out/default.desktop
rm -rf ../../release/out/default.png
rm -rf ../../release/out/quard_star_tools_temp
rm -rf ../../release/out/AppRun
rm -rf ../../release/out/doc

