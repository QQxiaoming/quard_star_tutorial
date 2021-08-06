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
# - per-extension "test runners" a.k.a. "log compilers" (xxx_LOG_COMPILER
#   and xxx_LOG_FLAGS), when some tests are PROGRAMS.
# See also related test 'parallel-tests-log-compiler-1.sh'.

required='cc native'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
## Note that automake should not match the '/test' part
## of 'sub/test' as '.test' suffix.
TESTS = foo.chk bar.test $(check_PROGRAMS) sub/test
check_PROGRAMS = baz bla.test bli.suff
TEST_EXTENSIONS = .chk .test
CHK_LOG_COMPILER = ./chk-compiler
TEST_LOG_COMPILER = ./test-compiler
LOG_COMPILER = ./noext-compiler
AM_CHK_LOG_FLAGS = 1
CHK_LOG_FLAGS = 2
AM_TEST_LOG_FLAGS = 3
TEST_LOG_FLAGS = 4
AM_LOG_FLAGS = 5
LOG_FLAGS = 6
END

mkdir sub

cat >chk-compiler <<'END'
#! /bin/sh
echo $0 "$@"
shift
shift
exec "$@"
exit 127
END
chmod a+x chk-compiler
cp chk-compiler test-compiler
cp chk-compiler noext-compiler

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

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE
$MAKE check || { cat test-suite.log; exit 1; }
ls -l . sub
cat test-suite.log
grep 'chk-compiler  *1  *2' foo.log
grep 'test-compiler  *3  *4' bar.log
grep 'noext-compiler  *5  *6' baz.log
grep 'test-compiler  *3  *4' bla.log
grep 'noext-compiler  *5  *6' bli.suff.log
grep 'noext-compiler  *5  *6' sub/test.log

:
