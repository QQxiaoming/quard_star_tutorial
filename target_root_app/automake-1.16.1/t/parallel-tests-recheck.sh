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

# Check parallel-tests features:
# - recheck

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_SUITE_LOG = mylog.log
TESTS = foo.test bar.test baz.test
check_SCRIPTS = bla
CLEANFILES = bla
bla:
	echo bla > $@
END

cat > foo.test <<'END'
#! /bin/sh
echo "this is $0"
test -f bla || exit 1
exit 0
END
cat > bar.test <<'END'
#! /bin/sh
echo "this is $0"
exit 99
END
cat > baz.test <<'END'
#! /bin/sh
echo "this is $0"
exit 1
END
chmod a+x foo.test bar.test baz.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
run_make -O -e FAIL check
count_test_results total=3 pass=1 fail=1 skip=0 xfail=0 xpass=0 error=1

# Running this two times in a row should produce the same results the
# second time.
for i in 1 2; do
  using_gmake || $sleep # Required by BSD make.
  run_make -O -e FAIL recheck
  count_test_results total=2 pass=0 fail=1 skip=0 xfail=0 xpass=0 error=1
  grep 'foo\.test' stdout && exit 1
  grep '^ERROR: bar\.test$' stdout
  grep '^FAIL: baz\.test$' stdout
done

# Ensure that recheck builds check_SCRIPTS, and that
# recheck reruns nothing if check has not been run.
$MAKE clean
$MAKE recheck
test -f bla
test ! -e foo.log
test ! -e bar.log
test ! -e baz.log
test -f mylog.log

:
