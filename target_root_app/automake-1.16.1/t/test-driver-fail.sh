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

# Custom test drivers: what happens when a test driver fails?  Well,
# "make check" should at least fail too, and the test-suite.log
# shouldn't be created.  Unfortunately, we cannot truly control also
# the (non-)creation of individual test logs, since those are expected
# to be created by the drivers themselves, and an ill-behaved driver
# (like our dummy one in this test) might leave around a test log even
# in case of internal failures.

. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am <<'END'
TEST_LOG_DRIVER = ./oops
TESTS = foo.test
END

cat > foo.test <<'END'
#! /bin/sh
exit 0
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

# The testsuite driver does not exist.
$MAKE check && exit 1
test ! -e test-suite.log

# The testsuite driver exists and create the test log files, but fails.

cat > oops <<'END'
#!/bin/sh
: > foo.log
echo 'Oops, I fail!' >&2
exit 1
END
chmod a+x oops

$MAKE check && exit 1
test ! -e test-suite.log

:
