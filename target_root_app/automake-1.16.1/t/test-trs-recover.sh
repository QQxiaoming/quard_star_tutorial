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

# Check parallel harness features:
#  - recovery from deleted '.trs' files, in various scenarios
# This test is complex and tricky, but that's acceptable since we are
# testing semantics that are potentially complex and tricky.

. test-init.sh

cat >> configure.ac <<END
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = foo.test bar.test baz.test
TEST_LOG_COMPILER = $(SHELL)
END

echo 'exit $TEST_STATUS' > foo.test
echo 'exit $TEST_STATUS' > bar.test
: > baz.test

TEST_STATUS=0; export TEST_STATUS

# Slower and possible overkill in some situations, but also clearer
# and safer.
update_stamp () { $sleep && touch stamp && $sleep; }

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

: Create the required log files.
$MAKE check

: Recreate by hand.
rm -f foo.trs bar.trs baz.trs
$MAKE foo.trs
test -f foo.trs
test ! -e bar.trs
test ! -e baz.trs

: Recreate by hand, several at the same time.
rm -f foo.trs bar.trs baz.trs
$MAKE foo.trs bar.trs
test -f foo.trs
test -f bar.trs
test ! -e baz.trs

: Recreate by hand, with a failing test.
rm -f foo.trs bar.trs
run_make -O TEST_STATUS=1 bar.trs baz.trs
test ! -e foo.trs
test -f bar.trs
test -f baz.trs
grep '^FAIL: bar\.test' stdout
$EGREP '^(baz|foo)\.test' stdout && exit 1

: Recreate with a sweeping "make check", and ensure that also up-to-date
: '.trs' files are remade.
update_stamp
rm -f foo.trs bar.trs
$MAKE check
test -f foo.trs
test -f bar.trs
is_newest baz.trs stamp

: Recreate with a sweeping "make check" with failing tests.  Again,
: ensure that also up-to-date '.trs' files are remade -- this time we
: grep the "make check" output verify that.
rm -f foo.trs bar.trs
run_make -O -e FAIL TEST_STATUS=1 check
test -f foo.trs
test -f bar.trs
grep '^FAIL: foo\.test' stdout
grep '^FAIL: bar\.test' stdout
grep '^PASS: baz\.test' stdout

: Recreate with a "make check" with redefined TESTS.
rm -f foo.trs bar.trs baz.trs
run_make TESTS=foo.test check
test -f foo.trs
test ! -e bar.trs
test ! -e baz.trs

: Recreate with a "make check" with redefined TEST_LOGS.
rm -f foo.trs bar.trs baz.trs
run_make TEST_LOGS=bar.log check
test ! -e foo.trs
test -f bar.trs
test ! -e baz.trs

: Interactions with "make recheck" are OK.
rm -f foo.trs bar.trs baz.log baz.trs
run_make -O recheck
test -f foo.trs
test -f bar.trs
test ! -e baz.trs
test ! -e baz.log
grep '^PASS: foo\.test' stdout
grep '^PASS: bar\.test' stdout
grep 'baz\.test' stdout && exit 1
count_test_results total=2 pass=2 fail=0 xpass=0 xfail=0 skip=0 error=0

: Setup for the next check.
$MAKE check
test -f foo.trs
test -f bar.trs
test -f baz.trs

: Recreate by remaking the global test log, and ensure that up-to-date
: '.trs' files are *not* remade.
update_stamp
rm -f foo.trs bar.trs test-suite.log
run_make -O test-suite.log
grep '^PASS: foo\.test' stdout
grep '^PASS: bar\.test' stdout
grep 'baz\.test' stdout && exit 1
stat *.trs *.log stamp || : # For debugging.
# Check that make has updated what it needed to, but no more.
test -f foo.trs
test -f bar.trs
is_newest stamp baz.trs
is_newest test-suite.log foo.trs bar.trs

: Setup for the next check.
$MAKE check
test -f foo.trs
test -f bar.trs
test -f baz.trs

: Interactions with lazy test reruns are OK.
rm -f foo.trs
update_stamp
touch bar.test
run_make -O RECHECK_LOGS= check
# Check that make has updated what it needed to, but no more.
test -f foo.trs
is_newest bar.trs bar.test
is_newest stamp baz.trs
grep '^PASS: foo\.test' stdout
grep '^PASS: bar\.test' stdout
grep 'baz\.test' stdout && exit 1

:
