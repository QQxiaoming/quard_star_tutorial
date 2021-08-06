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

# Check that the testsuite driver can find test in the srcdir as
# well as in builddir, and that is prefers those in the builddir.

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = foo.test bar.test
EXTRA_DIST = $(TESTS)
END

cat > foo.test << 'END'
#! /bin/sh
exit ${FOO_EXIT_STATUS-0}
END
chmod a+x foo.test

unset FOO_EXIT_STATUS

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

mkdir build
cd build

../configure

cat > bar.test << 'END'
#! /bin/sh
exit 0
END
chmod a+x bar.test

run_make -M check
# The serial test driver does not strip VPATH components from
# the name of the test, but the parallel driver should.
if test x"$am_serial_tests" = x"yes"; then
  grep '^PASS: .*foo\.test *$' output
else
  grep '\.\./foo' output && exit 1
  grep '^PASS: foo\.test *$' output
fi
grep '^PASS: bar\.test *$' output

rm -f test-suite.log foo.log bar.log

run_make -M -e FAIL FOO_EXIT_STATUS=1 check
# The serial test driver does not strip VPATH components from
# the name of the test, but the parallel driver should.
if test x"$am_serial_tests" = x"yes"; then
  grep '^FAIL: .*foo\.test *$' output
else
  grep '\.\./foo' output && exit 1
  grep '^FAIL: foo\.test *$' output
fi
grep '^PASS: bar\.test *$' output

rm -f test-suite.log foo.log bar.log

# Check that if the same test is present in srcdir and builddir,
# the one in builddir is preferred.
cp bar.test foo.test
run_make -M FOO_EXIT_STATUS=1 check
grep '^PASS: foo\.test *$' output
grep '^PASS: bar\.test *$' output

# The tests in the builddir must be preferred also by "make dist".
FOO_EXIT_STATUS=1 $MAKE distcheck

:
