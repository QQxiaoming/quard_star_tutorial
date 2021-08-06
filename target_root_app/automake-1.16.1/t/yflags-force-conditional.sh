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

# Check that the user can force automake to use *_YFLAGS variables
# which have conditional content.

. test-init.sh

cat >> configure.ac <<'END'
AC_SUBST([CC], [false])
AC_PROG_YACC
AM_CONDITIONAL([COND], [test x"$cond" = x"yes"])
AC_OUTPUT
END

mkdir bin
cat > bin/fake-yacc <<'END'
#!/bin/sh
echo "/* $* */" > y.tab.c
echo 'extern int dummy;' >> y.tab.c
END
chmod a+x bin/fake-yacc
PATH=$(pwd)/bin$PATH_SEPARATOR$PATH; export PATH
YACC=fake-yacc; export YACC

cat > Makefile.am <<'END'
AUTOMAKE_OPTIONS = no-dependencies
bin_PROGRAMS = foo bar
foo_SOURCES = foo.y main.c
bar_SOURCES = $(foo_SOURCES)
bar_YFLAGS = $(bar_yflags2)
if COND
AM_YFLAGS = __am_cond_yes__
bar_YFLAGS += __bar_cond_yes__
else !COND
AM_YFLAGS = __am_cond_no__
bar_yflags2 = __bar_cond_no__
endif !COND
END

: > foo.y

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a -Wno-unsupported

$EGREP '(YFLAGS|yflags|am__append)' Makefile.in # For debugging.

./configure cond=yes
$MAKE foo.c bar-foo.c

cat foo.c
cat bar-foo.c

$FGREP ' __am_cond_yes__ ' foo.c
$FGREP ' __bar_cond_yes__ ' bar-foo.c
$FGREP 'cond_no' foo.c bar-foo.c && exit 1

$MAKE maintainer-clean
ls -l

./configure cond=no
$MAKE foo.c bar-foo.c

cat foo.c
cat bar-foo.c

$FGREP ' __am_cond_no__ ' foo.c
$FGREP ' __bar_cond_no__ ' bar-foo.c
$FGREP 'cond_yes' foo.c bar-foo.c && exit 1

:
