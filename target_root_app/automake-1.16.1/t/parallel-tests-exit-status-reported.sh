#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# The exit status of a test should be reported in the test logs, so
# that one can see at a glance whether the test has succeeded or failed,
# without having to look also into the corresponding .trs file.
# See automake bug#11814.

. test-init.sh

echo AC_OUTPUT >> configure.ac

echo XFAIL_TESTS = t3.test t00.test > Makefile.am
echo TESTS = t00.test >> Makefile.am
for s in 0 1 2 3 5 77 78 99 100 126 127; do
  echo "TESTS += t${s}.test" >> Makefile.am
  cat > t${s}.test <<END
#!/bin/sh
printf "%s\\n%s\\n" 'random' 'will exit with status $s'
exit $s
END
done
cp t0.test t00.test

chmod a+x *.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
run_make -e FAIL check
ls -l  # For debugging.

match_result ()
{
  cat "$1.log"  # For debugging.
  test $(wc -l <"$1.log") -eq 3
  sed -n '$p' "$1.log" | grep "^$2 $1\\.test (exit status: $3)$"
}

match_result  t0   PASS  0
match_result  t00  XPASS 0
match_result  t1   FAIL  1
match_result  t2   FAIL  2
match_result  t3   XFAIL 3
match_result  t5   FAIL  5
match_result  t77  SKIP  77
match_result  t78  FAIL  78
match_result  t99  ERROR 99
match_result  t100 FAIL  100
match_result  t126 FAIL  126
match_result  t127 FAIL  127

:
