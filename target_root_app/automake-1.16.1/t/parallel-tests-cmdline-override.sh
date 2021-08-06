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

# Check that we can use indirections when overriding TESTS and
# TEST_LOGS from the command line.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_EXTENSIONS = .test .t
TEST_LOG_COMPILER = cat
T_LOG_COMPILER = cat
TESTS = bad.test
var1 = b.test $(var2)
var2 = c.test
var3 = d.d
var4 = e
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
rm -f config.log # Do not create false positives below.

cat > exp-log <<'END'
a.log
b.log
c.log
d.log
e.log
test-suite.log
END

cat > exp-out <<'END'
PASS: a.t
PASS: b.test
PASS: c.test
PASS: d.t
PASS: e.test
END

do_check ()
{
  run_make -O -- "$@" check
  grep '^PASS:' stdout | LC_ALL=C sort > got-out
  cat got-out
  ls . | grep '\.log$' | LC_ALL=C sort > got-log
  cat got-log
  st=0
  diff exp-out got-out || st=1
  diff exp-log got-log || st=1
  return $st
}

tests='a.t $(var1) $(var3:.d=.t) $(var4:=.test)'
test_logs='a.log $(var1:.test=.log) $(var3:.d=.log) $(var4:=.log)'

touch a.t b.test c.test d.t e.test

do_check TESTS="$tests"
do_check TEST_LOGS="$test_logs"

:
