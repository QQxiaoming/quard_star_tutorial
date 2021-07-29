#!/usr/bin/env bash
set -ex
$@
env QT_WAYLAND_SHELL_INTEGRATION=wl-shell $@
env QT_WAYLAND_SHELL_INTEGRATION=ivi-shell $@
env QT_WAYLAND_SHELL_INTEGRATION=xdg-shell-v6 $@
