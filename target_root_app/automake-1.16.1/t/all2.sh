#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Test to make sure all-local and other -local targets work correctly.

. test-init.sh

$ACLOCAL

targets='all install-exec install-data uninstall'
for target in $targets; do
  : Doing $target
  echo "${target}-local:" > Makefile.am
  $AUTOMAKE
  grep "${target}-local ${target}-local" Makefile.in && exit 1
  grep "${target}-am:.*${target}-local" Makefile.in
done

:
