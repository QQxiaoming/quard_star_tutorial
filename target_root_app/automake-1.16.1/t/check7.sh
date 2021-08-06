#! /bin/sh
# Copyright (C) 2007-2018 Free Software Foundation, Inc.
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

# Check EXEEXT extension for XFAIL_TESTS.

# For gen-testsuite-part: ==> try-with-serial-tests <==
required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = $(XFAIL_TESTS)
XFAIL_TESTS = a b c d
check_PROGRAMS = a c d
check_SCRIPTS = b
EXTRA_DIST = $(check_SCRIPTS)

expect-xfail-tests:
	is $(XFAIL_TESTS) == a$(EXEEXT) b c$(EXEEXT) d$(EXEEXT)
END

cat > b <<'END'
#! /bin/sh
exit 1
END
chmod a+x b

cat > a.c <<'END'
#include <stdlib.h>
int main (void)
{
  return EXIT_FAILURE;
}
END

cp a.c c.c
cp a.c d.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE check
run_make expect-xfail-tests
run_make expect-xfail-tests EXEEXT=.bin

$MAKE distcheck

:
