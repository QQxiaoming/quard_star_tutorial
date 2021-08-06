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

# "Simple Tests" testsuite driver: check TESTS_ENVIRONMENT support.

am_serial_tests=yes
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = foo.test
EXTRA_DIST = $(TESTS)
END

cat > foo.test << 'END'
#! /bin/sh
test x"$FOO" = x"ok"
END
chmod a+x foo.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

FOO=bad TESTS_ENVIRONMENT='FOO=ok'  $MAKE check
FOO=ok  TESTS_ENVIRONMENT='FOO=bad' $MAKE check && exit 1

:
