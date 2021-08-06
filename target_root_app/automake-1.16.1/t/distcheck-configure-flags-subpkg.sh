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

# Check AM_DISTCHECK_CONFIGURE_FLAGS and DISTCHECK_CONFIGURE_FLAGS
# are not honored in a subpackage Makefile.am, but the flags from
# AM_DISTCHECK_CONFIGURE_FLAGS and DISTCHECK_CONFIGURE_FLAGS are
# passed down to the configure script of the subpackage.  This is
# explicitly documented in the manual.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_SUBDIRS([subpkg])
if test $dc != ok || test $am_dc != ok; then
  AC_MSG_ERROR([dc=$dc am_dc=$dc])
fi
AC_OUTPUT
END

mkdir subpkg

sed <configure.ac >subpkg/configure.ac \
    -e 's/^AC_INIT.*/AC_INIT([subpkg], [2.0])/' \
    -e '/^AC_CONFIG_SUBDIRS/d' \

cat configure.ac
cat subpkg/configure.ac

dc=KO am_dc=KO; export dc am_dc

cat > Makefile.am << 'END'
AM_DISTCHECK_CONFIGURE_FLAGS = am_dc=ok
DISTCHECK_CONFIGURE_FLAGS = dc=ok
END

: > subpkg/Makefile.am

$ACLOCAL
$AUTOMAKE
$AUTOCONF

cd subpkg
$ACLOCAL
$AUTOMAKE
$AUTOCONF
cd ..

# For debugging.
$FGREP 'DISTCHECK_CONFIGURE_FLAGS' Makefile.in subpkg/Makefile.in

grep '^AM_DISTCHECK_CONFIGURE_FLAGS =' Makefile.in
grep '^DISTCHECK_CONFIGURE_FLAGS =' Makefile.in
$EGREP '^(AM_)?DISTCHECK_CONFIGURE_FLAGS' subpkg/Makefile.in && exit 1

./configure dc=ok am_dc=ok

# Flags in $(DISTCHECK_CONFIGURE_FLAGS) and $(AM_DISTCHECK_CONFIGURE_FLAGS)
# should be passed down to the subpackage configure when "make distcheck"
# is run from the top-level package ...
$MAKE distcheck

# ... but not when "make distcheck" is run from the subpackage.
cd subpkg
run_make -e FAIL -M distcheck
grep '^configure:.* dc=KO am_dc=KO' output

:
