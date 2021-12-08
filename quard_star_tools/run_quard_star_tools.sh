#!/bin/bash
set -e

SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
$SHELL_FOLDER/gui_tools/quard_star_tools -e $SHELL_FOLDER/output
