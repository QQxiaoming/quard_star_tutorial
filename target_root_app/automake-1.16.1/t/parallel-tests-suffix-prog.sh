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
# - suffix rules, with PROGRAMS involved
# See also sister test 'parallel-tests-suffix.sh'.

required='cc native'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
## Note that automake should not match the '/test' part of 'sub/test' as
## '.test' suffix, nor the '/chk' part of 'sub/chk' as '.chk' suffix.
TESTS = $(dist_TESTS) $(check_PROGRAMS)
dist_TESTS = foo.chk bar.test sub/test
check_PROGRAMS = baz bla.test bli.suff sub/chk
TEST_EXTENSIONS = .chk .test
EXTRA_DIST = $(dist_TESTS)
END

mkdir sub

cat >foo.chk << 'END'
#! /bin/sh
exit 0
END
chmod a+x foo.chk
cp foo.chk bar.test
cp foo.chk sub/test

cat >baz.c << 'END'
int main (void)
{
  return 0;
}
END
cp baz.c bla.c
cp baz.c bli.c
cp baz.c sub/chk.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE check
ls -l . sub
test -f foo.log
test -f bar.log
test -f baz.log
test -f bla.log
test -f bli.suff.log
test -f sub/chk.log
test -f sub/test.log

$MAKE distcheck

:
