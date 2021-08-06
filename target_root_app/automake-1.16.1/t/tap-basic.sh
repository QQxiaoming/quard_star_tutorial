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

# Basic TAP support:
#  - LOG_COMPILER support;
#  - basic support for TODO and SKIP directives, and "Bail out!" magic;
#  - testsuite progress output on console;
#  - runtime overriding of TESTS and TEST_LOGS;
#  - correct counts of test results (both in summary and in progress
#    output on console).
# Note that some of the features checked here are checked in other
# test cases too, usually in a more thorough and detailed way.

. test-init.sh

fetch_tap_driver

cat >> configure.ac <<END
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_LOG_DRIVER = $(srcdir)/tap-driver
## Defining LOG_COMPILER should work and not intefere with the
## tap-driver script.
TEST_LOG_COMPILER = cat
TESTS = success.test

ok.test:
	echo '1..3' > $@-t
	echo 'ok 1' >> $@-t
	echo 'not ok 2 # TODO' >>$@-t
	echo 'ok 3 # SKIP' >>$@-t
	cat $@-t ;: For debugging.
	mv -f $@-t $@
END

cat > success.test << 'END'
1..20
ok 1
ok 2 two
ok 3 - three
ok 4 four four
not ok 5
not ok 6 six
not ok 7 - seven
not ok 8 eight eight
ok 9 # TODO
ok 10 ten # TODO
ok 11 - eleven # TODO
ok 12 twelve twelve # TODO
not ok 13 # TODO
not ok 14 fourteen # TODO
not ok 15 - fifteen # TODO
not ok 16 sixteen sixteen # TODO
ok 17 # SKIP
ok 18 eighteen # SKIP
ok 19 - nineteen # SKIP
ok 20 twenty twenty # SKIP
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

# Basilar usage and testsuite progress output.

run_make -O -e FAIL check
count_test_results total=20 pass=4 fail=4 xpass=4 xfail=4 skip=4 error=0

test -f success.log
test -f test-suite.log

cat > exp << 'END'
PASS: success.test 1
PASS: success.test 2 two
PASS: success.test 3 - three
PASS: success.test 4 four four
FAIL: success.test 5
FAIL: success.test 6 six
FAIL: success.test 7 - seven
FAIL: success.test 8 eight eight
XPASS: success.test 9 # TODO
XPASS: success.test 10 ten # TODO
XPASS: success.test 11 - eleven # TODO
XPASS: success.test 12 twelve twelve # TODO
XFAIL: success.test 13 # TODO
XFAIL: success.test 14 fourteen # TODO
XFAIL: success.test 15 - fifteen # TODO
XFAIL: success.test 16 sixteen sixteen # TODO
SKIP: success.test 17 # SKIP
SKIP: success.test 18 eighteen # SKIP
SKIP: success.test 19 - nineteen # SKIP
SKIP: success.test 20 twenty twenty # SKIP
END

$FGREP ': success.test' stdout > got

cat exp
cat got
diff exp got

# Override TESTS from the command line.

rm -f *.log *.test

cat > bail.test <<'END'
1..1
Bail out!
ok 1
END

run_make -O -e FAIL check TESTS=bail.test
count_test_results total=1 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=1

test ! -e success.log
test -f bail.log
test -f test-suite.log

grep '^ERROR: bail\.test - Bail out!' stdout
grep '^PASS:' stdout && exit 1
test $($FGREP -c ': bail.test' stdout) -eq 1
$FGREP 'success.test' stdout && exit 1

# Override TEST_LOGS from the command line, making it point to a test
# (ok.test) that has to be generated at make time.

rm -f *.log *.test

run_make -O check TEST_LOGS=ok.log
count_test_results total=3 pass=1 fail=0 xpass=0 xfail=1 skip=1 error=0

test -f ok.test
test -f ok.log
test ! -e success.log
test ! -e bail.log
test -f test-suite.log

$EGREP '(bail|success)\.test' stdout && exit 1

cat > exp << 'END'
PASS: ok.test 1
XFAIL: ok.test 2 # TODO
SKIP: ok.test 3 # SKIP
END

$FGREP ': ok.test' stdout > got

cat exp
cat got
diff exp got

:
