#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that aclocal does not create a non-existent local m4 directory
# if the '--install' option is not given.

am_create_testdir=empty
. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
MY_MACRO
END

mkdir sys-acdir
cat > sys-acdir/my-defs.m4 <<END
AC_DEFUN([MY_MACRO], [:])
END

$ACLOCAL -I foo --system-acdir=sys-acdir && exit 1
test ! -e foo

:
