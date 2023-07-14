#!/bin/bash
set -e

SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)

export QT_HOME=$SHELL_FOLDER/gui_tools/quard_star_tools
export QT_PLUGIN_PATH=$QT_HOME/plugins
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$QT_HOME/lib" 

$SHELL_FOLDER/gui_tools/quard_star_tools/bin/quard_star_tools -e $SHELL_FOLDER/output
