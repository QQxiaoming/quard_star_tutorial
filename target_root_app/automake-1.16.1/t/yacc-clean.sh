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

# Check that .c and .h files derived from non-distributed .y sources
# are cleaned by "make clean", while .c and .h files derived from
# distributed .y sources are cleaned by "make maintainer-clean".
# See also sister test 'yacc-cxx-clean.sh'.

required='cc yacc'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_YACC
AC_CONFIG_FILES([sub1/Makefile sub2/Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
# Use two subdirectories, one to test with '-d' in YFLAGS, the
# other one to test with empty YFLAGS.
SUBDIRS = sub1 sub2
END

mkdir sub1 sub2

cat > sub1/Makefile.am << 'END'
bin_PROGRAMS = foo bar baz qux

foo_SOURCES = main.c parse.y

bar_SOURCES = main.c parse.y
bar_YFLAGS = $(AM_YFLAGS)

baz_SOURCES = main.c
nodist_baz_SOURCES = baz.y

qux_SOURCES = main.c
nodist_qux_SOURCES = baz.y
qux_YFLAGS = $(AM_YFLAGS)

baz.y:
	cp $(srcdir)/parse.y $@

CLEANFILES = baz.y
END

cat > sub2/Makefile.am << 'END'
include $(top_srcdir)/sub1/Makefile.am
AM_YFLAGS = -d
END

cat > sub1/parse.y << 'END'
%{
int yylex () { return (getchar ()); }
void yyerror (char *s) {}
%}
%%
x : 'x' { };
END
cp sub1/parse.y sub2/parse.y

cat > sub1/main.c << 'END'
int main ()
{
  return yyparse ();
}
END
cp sub1/main.c sub2/main.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

cp config.status config.sav

$MAKE
ls -l . sub1 sub2
# Sanity checks.
test -f sub1/parse.y
test -f sub1/parse.c
test -f sub1/bar-parse.c
test -f sub1/baz.y
test -f sub1/baz.c
test -f sub1/qux-baz.c
test -f sub2/parse.y
test -f sub2/parse.c
test -f sub2/parse.h
test -f sub2/bar-parse.c
test -f sub2/bar-parse.h
test -f sub2/baz.y
test -f sub2/baz.c
test -f sub2/baz.h
test -f sub2/qux-baz.c
test -f sub2/qux-baz.h

for target in clean distclean; do
  $MAKE $target
  ls -l . sub1 sub2
  test -f sub1/parse.y
  test -f sub1/parse.c
  test -f sub1/bar-parse.c
  test ! -e sub1/baz.y
  test ! -e sub1/baz.c
  test ! -e sub1/qux-baz.c
  test -f sub2/parse.y
  test -f sub2/parse.c
  test -f sub2/parse.h
  test -f sub2/bar-parse.c
  test -f sub2/bar-parse.h
  test ! -e sub2/baz.y
  test ! -e sub2/baz.c
  test ! -e sub2/baz.h
  test ! -e sub2/qux-baz.c
  test ! -e sub2/qux-baz.h
done

cp config.sav config.status
./config.status # Re-create 'Makefile'.

$MAKE maintainer-clean
ls -l . sub1 sub2
test -f sub1/parse.y
test ! -e sub1/parse.c
test ! -e sub1/bar-parse.c
test -f sub2/parse.y
test ! -e sub2/parse.c
test ! -e sub2/parse.h
test ! -e sub2/bar-parse.c
test ! -e sub2/bar-parse.h

:
