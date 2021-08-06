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

# Check interactions between the parallel test harness and "make -n".
# See also sister test 'parallel-tests-dry-run-1.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am <<'END'
TESTS = foo.test bar.test
TEST_LOG_COMPILER = $(SHELL)
END

$ACLOCAL
$AUTOMAKE -a
$AUTOCONF

./configure

make_n_ ()
{
  st=0
  run_make -M -- -n "$@" || { ls -l; exit 1; }
  # Look out for possible errors from common tools used by recipes.
  $EGREP -i ' (exist|permission|denied|no .*(such|file))' output && exit 1
  $EGREP '(mv|cp|rm|cat|grep|sed|awk): ' output && exit 1
  :
}

: > output

files='foo.log bar.log foo.trs bar.trs'

echo 'exit 0' > foo.test
echo 'exit 1' > bar.test

for target in check recheck test-suite.log; do
  make_n_ $target
  test ! -e foo.log
  test ! -e foo.trs
  test ! -e bar.log
  test ! -e bar.trs
  test ! -e test-suite.log
done

$MAKE check && exit 1

chmod a-w .

make_n_ clean
test -f foo.log
test -f foo.trs
test -f foo.log
test -f bar.trs

cat > foo.test <<END
echo this is bad
exit 1
END
echo 'exit 0' > bar.test

for target in check recheck test-suite.log; do
  make_n_ $target
  grep '^:test-result: *FAIL' bar.trs
  grep 'this is bad' foo.log test-suite.log && exit 1
  : For shells with busted 'set -e'.
done

chmod a-rw $files
if test -r foo.log; then
  : You can still read unreadable files!  Skip these checks.
else
  for target in check recheck test-suite.log; do
    make_n_ $target
    for f in $files; do
      test -f $f && test ! -r $f || exit 1
    done
  done
fi
chmod u+r $files

chmod u+w .
rm -f foo.log bar.trs
chmod a-w .
for target in check recheck test-suite.log $files; do
  make_n_ $target
  test ! -e foo.log
  test -f foo.trs
  test ! -e bar.trs
  test -f bar.log
done

:
