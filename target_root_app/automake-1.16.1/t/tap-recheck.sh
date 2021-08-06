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

# Test the 'recheck' target for TAP test protocol.
# Keep in sync with 'test-driver-custom-multitest-recheck.sh'.

. test-init.sh

fetch_tap_driver

cat >> configure.ac <<END
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_LOG_DRIVER = $(srcdir)/tap-driver
TESTS = a.test b.test c.test d.test
END

cat > a.test << 'END'
#! /bin/sh
echo 1..2
echo ok 1
echo ok 2
: > a.run
END

cat > b.test << 'END'
#! /bin/sh
: > b.run
echo 1..2
echo ok 1
if test -f b.ok; then
  echo ok 2
else
  echo 'Bail out!'
fi
END

cat > c.test << 'END'
#! /bin/sh
echo 1..3
if test -f c.pass; then
  echo 'ok - c is ok :-)'
else
  echo 'not ok - c is ko :-('
fi
if test -f c.xfail; then
  echo 'not ok 2 # TODO'
else
  echo 'ok 2 # TODO'
fi
echo 'not ok 3 - blah blah # TODO need better diagnostic'
: > c.run
END

cat > d.test << 'END'
#! /bin/sh
test -f ./d.count && . ./d.count
echo 1..${test_count-2}
echo ok 1 '# SKIP: who cares ...'
(. ./d.extra) || echo 'not ok 2 - d.extra failed'
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
  echo 'test_count=9' > d.count
  unindent > d.extra <<'END'
    echo 'ok # SKIP s'
    echo 'not ok - f 1'
    echo 'ok - p 1'
    echo 'not ok - f 2'
    echo 'ok # TODO xp'
    echo 'not ok - f 3'
    echo 'not ok - f 4'
    echo 'ok - p 2'
    echo 'ok' # Extra test.
    echo 'Bail out!'
END
  do_recheck --fail
  test ! -e a.run
  test ! -e b.run
  test ! -e c.run
  test -f d.run
  count_test_results total=11 pass=2 fail=4 xpass=1 xfail=0 skip=2 error=2

  : Let us finally make d.test pass.
  using_gmake || $sleep # Required by BSD make.
  echo 'test_count=1' > d.count
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
