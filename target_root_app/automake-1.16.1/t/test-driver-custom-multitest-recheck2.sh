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

# Custom test drivers: try the "recheck" functionality with test protocols
# that allow multiple testcases in a single test script.  In particular,
# check that this still works when we override $(TESTS) and $(TEST_LOGS)
# at make runtime.
# See also related tests 'test-driver-custom-multitest-recheck.sh' and
# 'parallel-tests-recheck-override.sh'.

. test-init.sh

cp "$am_testaux_srcdir"/trivial-test-driver . \
  || fatal_ "failed to fetch auxiliary script trivial-test-driver"

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_LOG_DRIVER = $(SHELL) $(srcdir)/trivial-test-driver
TESTS = a.test b.test c.test
END

cat > a.test << 'END'
#! /bin/sh
echo PASS: 1
echo PASS: 2
: > a.run
END

cat > b.test << 'END'
#! /bin/sh
echo SKIP: b0
if test -f b.ok; then
  echo XFAIL: b1
else
  echo FAIL: b2
fi
: > b.run
END

cat > c.test << 'END'
#! /bin/sh
if test -f c.err; then
  echo ERROR: xxx
elif test -f c.ok; then
  echo PASS: ok
else
  echo XPASS: xp
fi
: > c.run
END

chmod a+x *.test

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

  : Run the tests for the first time.
  run_make -O -e FAIL check
  # All the test scripts should have run.
  test -f a.run
  test -f b.run
  test -f c.run
  count_test_results total=5 pass=2 fail=1 xpass=1 xfail=0 skip=1 error=0

  rm -f *.run

  : An empty '$(TESTS)' or '$(TEST_LOGS)' means that no test should be run.
  for var in TESTS TEST_LOGS; do
    run_make -O "$var=" recheck
    count_test_results total=0 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=0
    test ! -e a.run
    test ! -e b.run
    test ! -e c.run
  done
  unset var

  : a.test was successful the first time, no need to re-run it.
  using_gmake || $sleep # Required by BSD make.
  run_make -O TESTS=a.test recheck
  count_test_results total=0 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=0
  test ! -e a.run
  test ! -e b.run
  test ! -e c.run

  : b.test failed, it should be re-run.  And make it pass this time.
  using_gmake || $sleep # Required by BSD make.
  echo OK > b.ok
  run_make -O TEST_LOGS=b.log recheck
  test ! -e a.run
  test -f b.run
  test ! -e c.run
  count_test_results total=2 pass=0 fail=0 xpass=0 xfail=1 skip=1 error=0

  rm -f *.run

  : No need to re-run a.test or b.test anymore.
  using_gmake || $sleep # Required by BSD make.
  run_make -O TEST_LOGS=b.log recheck
  count_test_results total=0 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=0
  test ! -e a.run
  test ! -e b.run
  test ! -e c.run
  using_gmake || $sleep # Required by BSD make.
  run_make -O TESTS='a.test b.test' recheck
  count_test_results total=0 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=0
  test ! -e a.run
  test ! -e b.run
  test ! -e c.run

  : No need to re-run a.test anymore, but c.test should be rerun,
  : as it contained an XPASS.  And this time, make it fail with
  : an hard error.
  echo dummy > c.err
  run_make -e FAIL -O TEST_LOGS='a.log c.log' recheck
  count_test_results total=1 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=1
  test ! -e a.run
  test ! -e b.run
  test -f c.run

  rm -f *.run *.err

  : c.test contained and hard error the last time, so it should be re-run.
  : This time, make it pass
  # Use 'echo', not ':'; see comments above for why.
  using_gmake || $sleep # Required by BSD make.
  echo dummy > c.ok
  run_make -O TESTS='c.test a.test' recheck
  count_test_results total=1 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=0
  test ! -e a.run
  test ! -e b.run
  test -f c.run

  rm -f *.run *.err *.ok

  : Nothing should be rerun anymore, as all tests have been eventually
  : successful.
  using_gmake || $sleep # Required by BSD make.
  run_make -O recheck
  count_test_results total=0 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=0
  test ! -e a.run
  test ! -e b.run
  test ! -e c.run

  cd $srcdir

done

:
