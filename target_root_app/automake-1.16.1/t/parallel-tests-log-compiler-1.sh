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
#   and xxx_LOG_FLAGS), also with AC_SUBST'd stuff.
# See also related test 'parallel-tests-log-compiler-2.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_SUBST([acsubst_compiler], [t-compiler])
AC_CONFIG_LINKS([chk-compiler:chk-compiler])
AC_OUTPUT
END

cat > Makefile.am << 'END'
# Note that automake should not match the '/test' part of 'sub/test'
# as '.test' suffix.
TESTS = foo.chk bar.test baz bla.t bli.suff sub/test

TEST_EXTENSIONS = .chk .test .t

CHK_LOG_COMPILER = ./chk-compiler
TEST_LOG_COMPILER = $(SHELL) $(srcdir)/test-compiler
T_LOG_COMPILER = @acsubst_compiler@
LOG_COMPILER = @SHELL@ -x $(srcdir)/noext-compiler

CHK_LOG_FLAGS = --chk

AM_TEST_LOG_FLAGS = --am-test

T_LOG_FLAGS = --t
AM_T_LOG_FLAGS = --am-t

AM_LOG_FLAGS = -a
LOG_FLAGS = -b

# 't-compiler' is deliberately not distributed; it should be available
# in PATH also from "make distcheck".
EXTRA_DIST = $(TESTS) chk-compiler test-compiler noext-compiler
END

mkdir sub

cat > chk-compiler <<'END'
#! /bin/sh
test x"$1" = x"--chk" || {
  echo "$0: invalid command line: $*" >&2
  exit 1
}
shift
exec "$@"
exit 127
END
sed 's/--chk/--am-test/' chk-compiler > test-compiler

mkdir bin
PATH=$(pwd)/bin$PATH_SEPARATOR$PATH; export PATH

cat > bin/t-compiler <<'END'
#! /bin/sh
if test x"$1,$2" != x"--am-t,--t"; then
  echo "$0: invalid command line: $*" >&2
  exit 1
fi
shift; shift
exec "$@"
exit 127
END
sed 's/--am-t,--t/-a,-b/' bin/t-compiler > noext-compiler

chmod a+x chk-compiler chk-compiler test-compiler bin/t-compiler noext-compiler

cat >foo.chk << 'END'
#! /bin/sh
exit 0
END
chmod a+x foo.chk
cp foo.chk bar.test
cp foo.chk baz
cp foo.chk bla.t
cp foo.chk bli.suff
cp foo.chk sub/test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE check || { cat test-suite.log; exit 1; }
ls -l . sub
cat test-suite.log
test -f foo.log
test -f bar.log
test -f baz.log
test -f bla.log
test -f bli.suff.log
test -f sub/test.log

run_make -e FAIL T_LOG_FLAGS=--bad check
cat test-suite.log
cat bla.log
# With the above flag overridden, bla.t should fail ...
$EGREP '(^ *|/)t-compiler:.* invalid .*--bad' bla.log
# ... but no other test should.
grep -v '^FAIL: bla\.t ' bla.log | grep 'FAIL:' && exit 1

# Try also with a VPATH build.
$MAKE distcheck

:
