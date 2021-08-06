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

# Check against automake bug#9578: "make uninstall" issued before
# "make install" or after a mere "make install-data" or a mere
# "make install-exec" failed spuriously.
#
# FIXME: this test only deal with DATA and script primaries; maybe we
# need sister tests for other primaries too?  E.g., PROGRAMS, LISP,
# PYTHON, etc...

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

: > foo
: > bar

cat > Makefile.am << 'END'
bin_SCRIPTS = foo
data_DATA = bar
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure --prefix="$(pwd)/inst"

$MAKE uninstall
test ! -e inst

rm -rf inst

$MAKE install-exec
test -f inst/bin/foo || exit 99 # Sanity check.
$MAKE uninstall
test ! -e inst/bin/foo

$MAKE install-data
test -f inst/share/bar || exit 99 # Sanity check.
$MAKE uninstall
test ! -e inst/share/bar

rm -rf inst

$MAKE install-exec
test -f inst/bin/foo || exit 99 # Sanity check.
$MAKE uninstall
test ! -e inst/bin/foo

:
