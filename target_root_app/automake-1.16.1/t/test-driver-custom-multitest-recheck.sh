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
# that allow multiple testcases in a single test script.  This test not
# only checks implementation details in Automake's custom test drivers
# support, but also serves as a "usability test" for our APIs.
# See also related tests 'test-driver-custom-multitest-recheck2.sh'
# and 'parallel-tests-recheck-override.sh'.
# Keep in sync with 'tap-recheck.sh'.

. test-init.sh

cp "$am_testaux_srcdir"/trivial-test-driver . \
  || fatal_ "failed to fetch auxiliary script trivial-test-driver"

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_LOG_DRIVER = $(SHELL) $(srcdir)/trivial-test-driver
TESTS = a.test b.test c.test d.test
END

cat > a.test << 'END'
#! /bin/sh
echo PASS: aa
echo PASS: AA
: > a.run
END

cat > b.test << 'END'
#! /bin/sh
echo PASS:
if test -f b.ok; then
  echo PASS:
else
  echo ERROR:
fi
: > b.run
END

cat > c.test << 'END'
#! /bin/sh
if test -f c.pass; then
  echo PASS: c0
else
  echo FAIL: c0
fi
if test -f c.xfail; then
  echo XFAIL: c1
else
  echo XPASS: c1
fi
echo XFAIL: c2
: > c.run
END

cat > d.test << 'END'
#! /bin/sh
echo SKIP: who cares ...
(. ./d.extra) || echo FAIL: d.extra failed
: > d.run
END

chmod a+x *.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE

do_recheck ()
{
  case $* in
    --fail) status=FAIL;;
    --pass) status=0;;
         *) fatal_ "invalid usage of function 'do_recheck'";;
  esac
  rm -f *.run
  run_make -O -e $status recheck || { ls -l; exit 1; }
  ls -l
}

for vpath in : false; do
  if $vpath; then
    mkdir build
    cd build
    srcdir=..
  else
    srcdir=.
  fi

  $srcdir/configure

  : A "make recheck" in a clean tree should run no tests.
  using_gmake || $sleep # Required by BSD make.
  do_recheck --pass
  cat test-suite.log
  test ! -e a.run
  test ! -e a.log
  test ! -e b.run
  test ! -e b.log
  test ! -e c.run
  test ! -e c.log
  test ! -e d.run
  test ! -e d.log
  count_test_results total=0 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=0

  : Run the tests for the first time.
  run_make -O -e FAIL check
  ls -l
  # All the test scripts should have run.
  test -f a.run
  test -f b.run
  test -f c.run
  test -f d.run
  count_test_results total=9 pass=3 fail=2 xpass=1 xfail=1 skip=1 error=1

  : Let us make b.test pass.
  using_gmake || $sleep # Required by BSD make.
  echo OK > b.ok
  do_recheck --fail
  # a.test has been successful the first time, so no need to re-run it.
  # Similar considerations apply to similar checks, below.
  test ! -e a.run
  test -f b.run
  test -f c.run
  test -f d.run
  count_test_results total=7 pass=2 fail=2 xpass=1 xfail=1 skip=1 error=0

  : Let us make the first part of c.test pass.
  using_gmake || $sleep # Required by BSD make.
  echo OK > c.pass
  do_recheck --fail
  test ! -e a.run
  test ! -e b.run
  test -f c.run
  test -f d.run
  count_test_results total=5 pass=1 fail=1 xpass=1 xfail=1 skip=1 error=0

  : Let us make also the second part of c.test pass.
  using_gmake || $sleep # Required by BSD make.
  echo KO > c.xfail
  do_recheck --fail
  test ! -e a.run
  test ! -e b.run
  test -f c.run
  test -f d.run
  count_test_results total=5 pass=1 fail=1 xpass=0 xfail=2 skip=1 error=0

  : Nothing changed, so only d.test should be run.
  for i in 1 2; do
    using_gmake || $sleep # Required by BSD make.
    do_recheck --fail
    test ! -e a.run
    test ! -e b.run
    test ! -e c.run
    test -f d.run
    count_test_results total=2 pass=0 fail=1 xpass=0 xfail=0 skip=1 error=0
  done

  : Let us make d.test run more testcases, and experience _more_ failures.
  using_gmake || $sleep # Required by BSD make.
  unindent > d.extra <<'END'
    echo SKIP: s
    echo FAIL: f 1
    echo PASS: p 1
    echo FAIL: f 2
    echo XPASS: xp
    echo FAIL: f 3
    echo FAIL: f 4
    echo ERROR: e 1
    echo PASS: p 2
    echo ERROR: e 2
END
  do_recheck --fail
  test ! -e a.run
  test ! -e b.run
  test ! -e c.run
  test -f d.run
  count_test_results total=11 pass=2 fail=4 xpass=1 xfail=0 skip=2 error=2

  : Let us finally make d.test pass.
  using_gmake || $sleep # Required by BSD make.
  echo : > d.extra
  do_recheck --pass
  test ! -e a.run
  test ! -e b.run
  test ! -e c.run
  test -f d.run
  count_test_results total=1 pass=0 fail=0 xpass=0 xfail=0 skip=1 error=0

  : All tests have been successful or skipped, nothing should be re-run.
  using_gmake || $sleep # Required by BSD make.
  do_recheck --pass
  test ! -e a.run
  test ! -e b.run
  test ! -e c.run
  test ! -e d.run
  count_test_results total=0 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=0

  cd $srcdir

done

:
