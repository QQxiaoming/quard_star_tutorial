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

# parallel-tests support: redirection of file descriptors with
# AM_TESTS_FD_REDIRECT, for tests which are binary executables
# We use some tricks to ensure that all code paths in 'lib/am/check2.am'
# are covered, even on platforms where $(EXEEXT) would be naturally empty.
# See also the more generic test 'check-fd-redirect.sh', and
# sister test 'parallel-tests-fd-redirect.sh'.

required='cc native'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
# Calls like "write(9, ...)" are unlikely to work for MinGW-compiled
# programs.  We must skip this test if this is the case.
am__ok=no
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([[#include <unistd.h>]],
                     [[write (9, "foobar\n", 7); return 0;]])],
    [AM_RUN_LOG([./conftest$EXEEXT 9>&1]) \
dnl Leading ":;" required to avoid having two nested subshells starting
dnl with '((' in the generated configure: that is unportable and could
dnl confuse some shells (e.g., NetBSD 5.1 /bin/ksh) into thinking we are
dnl trying to perform an arithmetic operation.
       && AM_RUN_LOG([:; (./conftest$EXEEXT 9>&1) | grep "^foobar"]) \
       && am__ok=yes])
test $am__ok = yes || AS_EXIT([63])
AM_CONDITIONAL([real_EXEEXT], [test -n "$EXEEXT"])
test -n "$EXEEXT" || EXEEXT=.bin
AC_OUTPUT
END

cat > Makefile.am << 'END'
AM_TESTS_FD_REDIRECT = 9>&1
TESTS = $(check_PROGRAMS)
check_PROGRAMS = baz qux.test
qux_test_SOURCES = zardoz.c

## Sanity check.
if !real_EXEEXT
check-local:
	test -f baz.bin
	test -f qux.test.bin
endif
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

cat > baz.c <<'END'
#include <stdio.h>
#include <unistd.h>
int main (void)
{
  ssize_t res = write (9, " bazbazbaz\n", 11);
  if (res < 0)
    perror("write failed");
  return res != 11;
}
END

cat > zardoz.c <<'END'
#include <stdio.h>
#include <unistd.h>
int main (void)
{
  ssize_t res = write (9, " quxquxqux\n", 11);
  if (res < 0)
    perror("write failed");
  return res != 11;
}
END

st=0; ./configure || st=$?
cat config.log # For debugging, as we do tricky checks in configure.
if test $st -eq 63; then
  skip_ "fd redirect in compiled program unsupported"
elif test $st -eq 0; then
  : Continue.
else
  fatal_ "unexpected error in ./configure"
fi

# Sanity checks.
st=0
grep '^baz\.log:.*baz\$(EXEEXT)' Makefile || st=1
grep '^\.test\$(EXEEXT)\.log:' Makefile || st=1
grep '^qux\.log:' Makefile && st=1
test $st -eq 0 || fatal_ "doesn't cover expected code paths"

run_make -O -e IGNORE check
cat baz.log
cat qux.log
test $am_make_rc -eq 0
grep "^ bazbazbaz$" stdout
grep "^ quxquxqux$" stdout
$EGREP '(bazbazbaz|quxquxqux)' *.log && exit 1

:
