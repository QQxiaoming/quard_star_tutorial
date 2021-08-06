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

# Custom test drivers: check that we can easily support test protocols
# that allow multiple testcases in a single test script.  This test not
# only checks implementation details in Automake's custom test drivers
# support, but also serves as a "usability test" for our APIs.

. test-init.sh

cp "$am_testaux_srcdir"/trivial-test-driver . \
  || fatal_ "failed to fetch auxiliary script trivial-test-driver"

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_EXTENSIONS = .t
T_LOG_DRIVER = $(SHELL) $(srcdir)/trivial-test-driver

TESTS = \
  pass.t \
  fail.t \
  fail2.t \
  pass-fail.t \
  pass4-skip.t \
  pass3-skip2-xfail.t \
  pass-xpass-fail-xfail-skip-error.t
END

cat > pass.t << 'END'
echo %% pass %%
echo PASS: pass
END

cat > fail.t << 'END'
echo %% fail %%
echo FAIL: fail
END

cat > fail2.t << 'END'
echo %% fail2 %%
echo FAIL: stdout >&1
echo FAIL: stderr >&2
echo :PASS: this should be ignored
END

cat > pass-fail.t << 'END'
echo %% pass-fail %%
echo 'FAIL: this fails :-('
echo 'some randome message'
echo 'some randome warning' >&2
echo 'PASS: this passes :-)'
echo 'INFO: blah'
echo 'WARNING: blah blah' >&2
END

cat > pass4-skip.t << 'END'
echo %% pass4-skip %%
echo PASS: on stdout >&1
echo PASS: on stderr >&2
echo PASS: 3
echo PASS: 4
echo SKIP: 1
echo this FAIL: should be ignored
echo FAIL as should this
exit 99
END

cat > pass3-skip2-xfail.t << 'END'
echo %% pass4-skip2-xfail %%
echo 'PASS: -v'
echo 'PASS: --verbose'
echo 'SKIP: Oops, unsupported system.'
echo 'PASS: -#-#-#-'
cp || echo "SKIP: cp cannot read users' mind" >&2
mv || echo "XFAIL: mv cannot read users' mind yet"
exit 127
END

cat > pass-xpass-fail-xfail-skip-error.t << 'END'
echo PASS:
echo FAIL:
echo XFAIL:
echo XPASS:
echo SKIP:
echo ERROR:
echo %% pass-xpass-fail-xfail-skip-error %%
END

chmod a+x *.t

$ACLOCAL
$AUTOCONF
$AUTOMAKE

for vpath in : false; do
  if $vpath; then
    mkdir build
    cd build
    srcdir=..
  else
    srcdir=.
  fi

  $srcdir/configure

  run_make -O -e FAIL check || { cat test-suite.log; exit 1; }
  cat test-suite.log
  # Couple of sanity checks.  These might need to be updated if the
  # 'trivial-test-driver' script is changed.
  $FGREP INVALID.NAME stdout test-suite.log && exit 1
  test -f BAD.LOG && exit 1
  test -f BAD.TRS && exit 1
  # These log files must all have been created by the testsuite.
  cat pass.log
  cat fail.log
  cat fail2.log
  cat pass-fail.log
  cat pass4-skip.log
  cat pass3-skip2-xfail.log
  cat pass-xpass-fail-xfail-skip-error.log

  count_test_results total=23 pass=10 fail=5 skip=4 xfail=2 xpass=1 error=1

  tst=pass-xpass-fail-xfail-skip-error
  grep  "^PASS: $tst\.t, testcase 1" stdout
  grep  "^FAIL: $tst\.t, testcase 2" stdout
  grep "^XFAIL: $tst\.t, testcase 3" stdout
  grep "^XPASS: $tst\.t, testcase 4" stdout
  grep  "^SKIP: $tst\.t, testcase 5" stdout
  grep "^ERROR: $tst\.t, testcase 6" stdout

  # Check that the content of, and only of, the test logs with at least
  # one failing test case has been copied into 'test-suite.log'.  Note
  # that test logs containing skipped or xfailed test cases are *not*
  # copied into 'test-suite.log' -- a behaviour that deliberately differs
  # from the one of the built-in Automake test drivers.
  grep '%%' test-suite.log # For debugging.
  grep '%% fail %%' test-suite.log
  grep '%% fail2 %%' test-suite.log
  grep '%% pass-fail %%' test-suite.log
  grep '%% pass-xpass-fail-xfail-skip-error %%' test-suite.log
  test $(grep -c '%% ' test-suite.log) -eq 4

  run_make -O TESTS='pass.t pass3-skip2-xfail.t' check
  cat test-suite.log
  count_test_results total=7 pass=4 fail=0 skip=2 xfail=1 xpass=0 error=0

  cd $srcdir

done

:
