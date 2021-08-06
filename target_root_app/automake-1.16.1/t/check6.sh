#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# Test for PR 400: XFAIL_TESTS delimited by TABs.
# Also test that Solaris make VPATH rewriting is honored

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = $(XFAIL_TESTS)
XFAIL_TESTS = a	b	c
END

cat >>a <<'END'
#! /bin/sh
exit 1
END

cp a b
cp a c

chmod a+x a b c

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE check
$MAKE distclean
mkdir build
cd build
../configure
$MAKE check

:
