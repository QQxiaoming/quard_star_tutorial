#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Make sure "aclocal --install" install files also in directories
# passed to '-I' as absolute paths.  Now that we support the
# ACLOCAL_PATH variable, it is clear that this is the right thing
# to do.  See also automake bug#8407.

. test-init.sh

mkdir loc sys

echo 'AM_DUMMY_MACRO' >> configure.ac
echo 'AC_DEFUN([AM_DUMMY_MACRO], [:])' >> sys/foo.m4

cwd=$(pwd) || fatal_ "cannot get current working directory"
env ACLOCAL_PATH="$cwd/sys" $ACLOCAL --verbose --install -I "$cwd/loc"
diff sys/foo.m4 loc/foo.m4

:
