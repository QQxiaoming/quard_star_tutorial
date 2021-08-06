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

# With old serial testsuite driver, TESTS_ENVIRONMENT can be used to
# define the "test runner", i.e. the program that the test scripts must
# be run by (with the parallel test harness one should use LOG_COMPILER
# for this).  The behaviour tested here is also documented in the manual.

am_serial_tests=yes
. test-init.sh

cat >> configure.ac <<END
AC_SUBST([PERL], ['$PERL'])
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS_ENVIRONMENT = $(PERL) -Mstrict -w
TESTS = foo.pl bar.pl baz.pl
XFAIL_TESTS = baz.pl
EXTRA_DIST = $(TESTS)
END

echo 'exit (0);' > foo.pl
echo 'exit (0);' > bar.pl

cat > baz.pl << 'END'
# With "use strict" enacted, this will cause an error, since the
# variable '$x' is not declared with 'my' nor specified with an
# explicit package name.
$x = 0;
exit ($x);
END

chmod a+x *.pl

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE check
$MAKE distcheck

:
