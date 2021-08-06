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

# Test for bug in PR 9.
# "make dist" incorrect when AC_CONFIG_AUX_DIR is present.
#
# == Report ==
# When AC_CONFIG_AUX_DIR is set (in my case to 'support'), make dist
# no longer automatically includes config.guess, config.sub, install-sh,
# ltconfig, ltmain.sh, mdate-sh, missing, and mkinstalldirs.  In fact,
# the entire 'support/' directory is omitted.

. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([support])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

: > Makefile.am

mkdir support

$ACLOCAL

$AUTOCONF
$AUTOMAKE -a -c
./configure
$MAKE
$MAKE distcheck

gzip -d $distdir.tar.gz

tar tf $distdir.tar | $FGREP support

tar xf $distdir.tar
ls -l . $distdir # For debugging.
test -d $distdir/support
ls -l $distdir/support # For debugging.
test -f $distdir/support/install-sh

:
