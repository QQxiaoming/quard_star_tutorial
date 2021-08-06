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

# Check parallel-tests features: normal and special exit statuses
# in the test scripts.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

# $failure_statuses should be defined to the list of all integers between
# 1 and 255 (inclusive), excluded 77 and 99.
failure_statuses=$(seq_ 1 255 | $EGREP -v '^(77|99)$' | tr "$nl" ' ')
# For debugging.
echo "failure_statuses: $failure_statuses"
# Sanity check.
test $(for st in $failure_statuses; do echo $st; done | wc -l) -eq 253 \
  || fatal_ "initializing list of exit statuses for simple failures"

cat > Makefile.am <<END
LOG_COMPILER = $AM_TEST_RUNNER_SHELL ./do-exit
fail_tests = $failure_statuses
TESTS = 0 77 99 $failure_statuses
\$(TESTS):
END

cat > do-exit <<'END'
#!/bin/sh
echo "$0: $1"
case $1 in
  [0-9]|[0-9][0-9]|[0-9][0-9][0-9]) st=$1;;
  */[0-9]|*/[0-9][0-9]|*/[0-9][0-9][0-9]) st=${1##*/};;
  *) st=99;;
esac
exit $st
END
chmod a+x do-exit

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

{
  echo PASS: 0
  echo SKIP: 77
  echo ERROR: 99
  for st in $failure_statuses; do
    echo "FAIL: $st"
  done
} | LC_ALL=C sort > exp-0

sed 's/^FAIL:/XFAIL:/' exp-0 | LC_ALL=C sort > exp-1
sed '/^ERROR:/d' exp-1 > exp-2

./configure

mk_ ()
{
  n=$1; shift
  unset am_make_rc
  run_make -e IGNORE -O -- ${1+"$@"} check
  cat test-suite.log
  LC_ALL=C grep '^[A-Z][A-Z]*:' stdout | LC_ALL=C sort > got-$n
  cat exp-$n
  cat got-$n
  diff exp-$n got-$n
}

mk_ 0
test $am_make_rc -gt 0
mk_ 1 XFAIL_TESTS="$failure_statuses 99"
test $am_make_rc -gt 0
mk_ 2 XFAIL_TESTS="$failure_statuses" TESTS="0 77 $failure_statuses"
test $am_make_rc -eq 0

:
