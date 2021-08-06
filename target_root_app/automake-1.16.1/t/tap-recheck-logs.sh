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

# TAP support:
# - RECHECK_LOGS

. test-init.sh

cat > Makefile.am << 'END'
TEST_LOG_COMPILER = cat
TESTS = foo.test bar.test baz.test
baz.log: zardoz
END

. tap-setup.sh

: > zardoz

cat > foo.test <<'END'
1..2
ok 1
ok 2
END

cat > bar.test <<'END'
1..1
not ok 1
END

cat > baz.test <<'END'
1..1
Bail out!
END

# Even the tests that are not re-run should contribute to the testsuite
# summary when obtained by "make check RECHECK_LOGS=".
grep_summary ()
{
  grep '^# TOTAL: *4$' stdout
  grep '^# PASS: *2$' stdout
  grep '^# XPASS: *0$' stdout
  grep '^# FAIL: *1$' stdout
  grep '^# XFAIL: *0$' stdout
  grep '^# SKIP: *0$' stdout
  grep '^# ERROR: *1$' stdout
}

run_make -e FAIL check
test -f foo.log
test -f bar.log
test -f baz.log

rm -f foo.log bar.log

run_make -O -e FAIL check RECHECK_LOGS=
test -f foo.log
test -f bar.log
grep '^PASS: foo\.test 1$' stdout
grep '^PASS: foo\.test 2$' stdout
grep '^FAIL: bar\.test 1$' stdout
grep 'baz\.test' stdout && exit 1
grep_summary

$sleep
touch foo.test
# We re-run only a successful test, but the tests that failed in the
# previous run should still be taken into account, and cause an overall
# failure.
run_make -O -e FAIL check RECHECK_LOGS=
grep '^PASS: foo\.test 1$' stdout
grep '^PASS: foo\.test 2$' stdout
grep 'ba[rz]\.test' stdout && exit 1
is_newest foo.log foo.test
grep_summary

$sleep
touch zardoz
run_make -O -e FAIL check RECHECK_LOGS=
grep '^ERROR: baz\.test' stdout
$EGREP '(foo|bar)\.test' stdout && exit 1
is_newest baz.log zardoz
grep_summary

# Now, explicitly retry with all test logs already updated, and ensure
# that the summary is still displayed.
run_make -O -e FAIL check RECHECK_LOGS=
$EGREP '(foo|bar|baz)\.test' stdout && exit 1
grep_summary

# The following should re-run foo.test (and only foo.test), even if its
# log file is up-to-date.
: > older
run_make -O -e FAIL check RECHECK_LOGS=foo.log
grep '^PASS: foo\.test 1$' stdout
grep '^PASS: foo\.test 2$' stdout
grep 'ba[rz]\.test' stdout && exit 1
is_newest foo.log older
grep_summary

:
