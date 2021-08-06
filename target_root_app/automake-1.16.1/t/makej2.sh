#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Test to make sure known BSD 'make -jN' issues are fixed:
# without -B, it may reuse the same shell for separate commands in a
# rule, which can lead to interesting results.

. test-init.sh

cat >>configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
test-distdir-removed:
	test ! -d $(distdir)
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

mkdir build
cd build
../configure --prefix="$(pwd)/inst"

$MAKE -j2 || skip_ "$MAKE failed to run with two parallel jobs"
$MAKE -j2 distcheck
$MAKE test-distdir-removed

:
