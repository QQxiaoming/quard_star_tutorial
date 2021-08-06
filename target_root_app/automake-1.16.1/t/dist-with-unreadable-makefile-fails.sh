#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Test for bug in 'make dist'
# From Pavel Roskin.

am_create_testdir=empty
. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
dnl Prevent automake from looking in .. and ../..
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = .
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

chmod 000 Makefile.am

# On some systems (like DOS and Windows), files are always readable.
test ! -r Makefile.am || skip_ "cannot drop file read permissions"

./configure

# 'dist' should fail because we can't copy Makefile.am.
if $MAKE dist; then
  exit 1
else
  exit 0
fi
