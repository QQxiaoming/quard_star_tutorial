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
#  - creation and removal of '.trs' auxiliary files
#  - check some internals regarding the use of '.trs' files.

. test-init.sh

cat >> configure.ac <<END
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_EXTENSIONS = .sh .test
TESTS = foo.test bar.sh sub/zardoz.test
TEST_LOG_COMPILER = $(SHELL)
SH_LOG_COMPILER = $(SHELL)

## Used to check some internal details.  And yes, the quotes around
## '$bases' are deliberate: they check for whitespace normalization.
tb:
	$(am__set_TESTS_bases); echo "$$bases" > $@
END

cat > foo.test << 'END'
#! /bin/sh
exit $FOO_STATUS
END
: > bar.sh
mkdir sub
: > sub/zardoz.test

FOO_STATUS=0; export FOO_STATUS

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a


#
# Check some internal details first.
#

for vpath in : false; do

  if $vpath; then
    srcdir=..
    mkdir build
    cd build
  else
    srcdir=.
  fi

  $srcdir/configure

  $MAKE tb
  test x"$(cat tb)" = x"foo bar sub/zardoz"
  rm -f tb
  # Please don't change the order of the stuff in TESTS, below.
  run_make TESTS='foo.test foo2.sh foo-log foolog.test a.log.b.sh 0.exe' tb
  test x"$(cat tb)" = x"foo foo2 foo-log foolog a.log.b 0.exe"
  rm -f tb

  cd $srcdir

done

#
# The 'test-suite.stamp' file and the '.trs' files get created by
# "make check" and removed by "make clean" and "make mostlyclean".
#

: > unrelated.trs
: > sub/foo.trs

$MAKE check
test -f foo.trs
test -f bar.trs
test -f sub/zardoz.trs
$MAKE clean
test ! -e foo.trs
test ! -e bar.trs
test ! -e sub/zardoz.trs
# Unrelated '.trs' files shouldn't be removed.
test -f unrelated.trs
test -f sub/foo.trs

# The files should be properly created in case of testsuite failure too.
FOO_STATUS=1 $MAKE check && exit 1
test -f foo.trs
test -f bar.trs
test -f sub/zardoz.trs
$MAKE mostlyclean
test ! -e foo.trs
test ! -e bar.trs
test ! -e sub/zardoz.trs
# Unrelated '.trs' files shouldn't be removed.
test -f unrelated.trs
test -f sub/foo.trs

#
# Try with a subset of TESTS.
#

run_make TESTS=foo.test check
test -f foo.trs
test ! -e bar.trs
test ! -e sub/zardoz.trs
$MAKE clean
test ! -e foo.trs
run_make TESTS='foo.test bar.sh' check
test -f foo.trs
test -f bar.trs
test ! -e sub/zardoz.trs
# "make clean" shouldn't remove '.trs' files for tests not in $(TESTS).
run_make TESTS=bar.sh clean
test -f foo.trs
test ! -e bar.trs

$MAKE clean

#
# Try with a subset of TEST_LOGS.
#

run_make TEST_LOGS=sub/zardoz.log check
test ! -e foo.trs
test ! -e bar.trs
test -f sub/zardoz.trs
$MAKE clean
test ! -e sub/zardoz.trs
run_make TEST_LOGS='foo.log bar.log' check
test -f foo.trs
test -f bar.trs
test ! -e sub/zardoz.trs
# "make clean" shouldn't remove '.trs' files for tests whose log
# is not in $(TEST_LOGS).
run_make TEST_LOGS=foo.log clean
test ! -e foo.trs
test -f bar.trs
test ! -e sub/zardoz.trs

:
