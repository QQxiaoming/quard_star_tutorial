#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Test to make sure include of include detects missing macros

am_create_testdir=empty
. test-init.sh

cat >> configure.ac <<END
AC_INIT([$me], [1.0])
GNOME_X_CHECKS
END

mkdir macros

cat > macros/gnome.m4 << 'END'
AC_DEFUN([GNOME_X_CHECKS], [
# Use a macro that cannot be installed in a third-party system-wide m4
# file; otherwise, this test might fail under "make installcheck".
	AM__PATH__GTK(0.99.5,,AC_MSG_ERROR(GTK not installed, or gtk-config not in path))
])
END

$ACLOCAL -I macros 2>stderr && { cat stderr >&2; exit 1; }
cat stderr
grep '^macros/gnome\.m4:4:.*AM__PATH__GTK.*not found' stderr

:
