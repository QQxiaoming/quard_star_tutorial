#!/bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Build either as CONFIG_FILE or as PROGRAM.

required=cc
. test-init.sh

mkdir sub

cat >>configure.ac <<'END'
AC_PROG_CC
AM_CONDITIONAL([COND], [test "$COND" = true])
AM_COND_IF([COND], [],
	   [AC_CONFIG_FILES([prog1], [chmod 755 prog1])
	    AC_CONFIG_FILES([sub/prog2], [chmod 755 sub/prog2])])
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat >Makefile.am <<'END'
SUBDIRS = sub
if COND
bin_PROGRAMS = prog1
prog1_SOURCES = prog.c
else
bin_SCRIPTS = prog1
CLEANFILES = prog1
endif

sure-exist:
	test -f prog1 || test -f prog1$(EXEEXT)
	test -f sub/prog2 || test -f sub/prog2$(EXEEXT)

sure-not-exist:
	test ! -f prog1 && test ! -f prog1$(EXEEXT)
	test ! -f sub/prog2 && test ! -f sub/prog2$(EXEEXT)
END

cat >sub/Makefile.am <<'END'
if COND
bin_PROGRAMS = prog2
prog2_SOURCES = prog.c
else
bin_SCRIPTS = prog2
CLEANFILES = prog2
endif
END

cat >prog.c <<'END'
int main () { return 42; }
END

cat >prog1.in <<'END'
#! /bin/sh
bindir='@bindir@'
echo "hi, this is $0, and bindir is $bindir"
END

cp prog.c sub
cp prog1.in sub/prog2.in

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure COND=true
run_make -E
grep 'overriding commands' stderr && exit 1
$MAKE sure-exist
./prog1 && exit 1
./sub/prog2 && exit 1
$MAKE clean
$MAKE sure-not-exist
$MAKE
$MAKE sure-exist
./prog1 && exit 1
./sub/prog2 && exit 1
$MAKE distclean

./configure COND=false
run_make -E
grep 'overriding commands' stderr && exit 1
./prog1
./sub/prog2
$MAKE clean
$MAKE sure-not-exist
$MAKE
./prog1
./sub/prog2

:
