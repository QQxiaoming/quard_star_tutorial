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

# Check that ':test-results:' directives in test scripts' output doesn't
# originate spurious results in the testsuite summary.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = foo.test bar.test
END

cat > foo.test <<'END'
#! /bin/sh
echo :test-result:XFAIL
echo :test-result: SKIP
echo :test-result:ERROR
exit 0
END
cat > bar.test <<'END'
#! /bin/sh
echo :test-result: ERROR
echo :test-result:FAIL
echo :test-result: XPASS
exit 0
END
chmod a+x foo.test bar.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

run_make -O -e IGNORE check
cat test-suite.log
cat foo.log
cat bar.log
test $am_make_rc -eq 0

grep '^:test-result:XFAIL$'  foo.log
grep '^:test-result: SKIP$'  foo.log
grep '^:test-result:FAIL$'   bar.log
grep '^:test-result: XPASS$' bar.log

count_test_results total=2 pass=2 fail=0 skip=0 xfail=0 xpass=0 error=0

:
