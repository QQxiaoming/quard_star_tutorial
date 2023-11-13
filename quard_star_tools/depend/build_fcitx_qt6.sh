#!/bin/bash
set -e

# ubuntu20.04 need install 
# sudo apt install fcitx-libs-dev qtbase5-private-dev libxkbcommon-dev extra-cmake-modules
# export PATH="~Qt/6.2.0/gcc_64/bin":$PATH
# export PATH="~/Qt/Tools/CMake/bin":$PATH
# export Qt6GuiTools_DIR=/home/qqm/Qt/6.2.0/gcc_64
SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
tar -xzvf fcitx-qt5-1.2.7.tar.gz
cd $SHELL_FOLDER/fcitx-qt5-1.2.7
mkdir build
cd build
cmake .. -DENABLE_QT5=OFF -DENABLE_QT6=ON
make -j4
